#!/usr/bin/env python
# --*-- coding:utf-8 --*--

import os
import sys
import urllib2
import random
import multiprocessing as mp
import logging
import time
import argparse
import math

try:
    from osgeo import gdal
    #from osgeo import ogr
    #from osgeo import osr
except:
    import gdal
    #import ogr
    print("""You are using "old gen" bindings. gdal2tiles needs "new gen" bindings.""")
    sys.exit(1)

import numpy
from array import array
import watermark as wmk
import smSci as smsci
import srsWeb as srsweb
import common as cm
import license as lic
import scitemplate

#---------------------------------------------------------------------------
def appPath():
    try:
	dirName = os.path.dirname(os.path.abspath(__file__))
    except:
	dirName = os.path.dirname(os.path.abspath(sys.argv[0]))

    # 打包后目录发生变化,需要去掉library.zip目录
    dirName = dirName.replace("library.zip","")
    return dirName

#---------------------------------------------------------------------------
def tileInGoogle(row, col, level):
    """ 指定瓦片真的是谷歌的吗 """
    size = 2**level
    if row<size and col<size:
	return True
    return False

#---------------------------------------------------------------------------
def loadTiles(rs,re,cs,ce, level, outPath):
    """ 将本地瓦片加载到内存 """

    tiles = {}
    gm = srsweb.GlobalMercator(256)
    for row in range(rs, re+1):
	for col in range(cs, ce+1):
	    if not tileInGoogle(row, col, level):
		continue
	    outfile = "%d_%d_%d.png" % (level, row, col) 
	    fp = os.path.join(outPath, outfile)
	    if not os.path.isfile(fp):
		continue

	    tmp_ds = gdal.Open(fp, gdal.GA_ReadOnly)
	    if tmp_ds is None:
		continue

	    r = tmp_ds.GetRasterBand(1).ReadAsArray(0, 0, 256, 256, 256, 256) 
	    g = tmp_ds.GetRasterBand(2).ReadAsArray(0, 0, 256, 256, 256, 256) 
	    b = tmp_ds.GetRasterBand(3).ReadAsArray(0, 0, 256, 256, 256, 256) 
	    tx, ty = gm.TMSTile(col, row, level)
	    tiles[(ty,tx)] = (r,g,b) 
	    del tmp_ds
    return tiles

#---------------------------------------------------------------------------
def downOneTile(url, fp):
    """ 一次下载 """
    try:
	f = urllib2.urlopen(url)
    except urllib2.URLError, e:
	print url 
	print e.reason
	return None
    return f.read()


#---------------------------------------------------------------------------
def downlaodTile(mapname, level, row, col, outPath, strurl, ver, haswatermark):
    """ 下载瓦片到本地 """

    tmp = strurl 
    mkt = srsweb.GlobalMercator()
    scale = mkt.Scale(level) 
    fp = smsci.smSci.calcTileName(outPath, mapname, scale, row, col, '.png', ver) 

    url = tmp % (random.randint(0,3), col, row, level)
    data = downOneTile(url, fp)
    if data is None:
	data = downOneTile(url, fp) # 第2次下载
    if data is None:
	data = downOneTile(url, fp) # 第3次下载
    if data is None:
	print url
	return False # 放弃吧

    if len(data)<1024:
	data = downOneTile(url, fp) # 第2次下载
    if len(data)<1024:
	data = downOneTile(url, fp) # 第3次下载

    if len(data)<1024:
	print url, len(data)
	return False # 放弃吧

    if not os.path.exists(os.path.dirname(fp)):
	os.makedirs(os.path.dirname(fp))
    with open(fp, "wb") as myfile:
	myfile.write(data)
	myfile.close()

#---------------------------------------------------------------------------
def productSmTile(row, col, level, outPath, tiles, haswatermark):
    """ 生成SuperMap瓦片 """

    mem_drv = gdal.GetDriverByName("MEM")
    mem_ds = mem_drv.Create("", 256, 256, 3)# 输出固定为24位png或jpg
    
    rdata = numpy.zeros((256, 256), numpy.uint8)
    gdata = numpy.zeros((256, 256), numpy.uint8)
    bdata = numpy.zeros((256, 256), numpy.uint8)
    
    gm = srsweb.GlobalMercator(256)
    tmp = os.path.join(outPath, "xxx")
    l,t,r,b = smsci.smSci3d.calcBndByRowCol(row, col, level)

    levelRes = (180.0/256) / (1<<level) # i层分辨率
    halfRes = levelRes*0.5

    for line in range(0, 256):
	for pix in range(0, 256):
	    lat, lon = t-line*levelRes-halfRes, l+pix*levelRes+halfRes
	    x,y = gm.LatLonToMeters(lat, lon)
	    px, py = gm.MetersToPixels(x, y, level+1)
	    tx, ty = gm.PixelsToTile(px, py)
	    gx, gy = gm.GoogleTile(tx, ty, level+1)
	    #print gx,gy
	    if (ty, tx) in tiles:
		trow, tcol = gm.PixelsPosInTile(px,py)
		r,g,b = tiles[(ty, tx)]
		rdata[line][pix] = r[trow][tcol]
		gdata[line][pix] = g[trow][tcol]
		bdata[line][pix] = b[trow][tcol]

    mem_ds.GetRasterBand(1).WriteArray(rdata)
    mem_ds.GetRasterBand(2).WriteArray(gdata)
    mem_ds.GetRasterBand(3).WriteArray(bdata)

    fp = smsci.smSci3d.calcTileName(level, row, col, outPath)+".png"
    if not os.path.exists(os.path.dirname(fp)):
	os.makedirs(os.path.dirname(fp))

    tx, ty = mem_ds.RasterXSize, mem_ds.RasterYSize
    tile_data = mem_ds.GetRasterBand(1).ReadAsArray(0,0,tx,ty,tx,ty) 
    if haswatermark:
	wmk.printWatermark(tile_data, wmk.buf_xsize, wmk.buf_ysize, wmk.buf_tile_server)
    mem_ds.GetRasterBand(1).WriteArray(tile_data)

    out_drv = gdal.GetDriverByName("PNG")
    out_drv.CreateCopy(fp, mem_ds, strict=0)
    del mem_ds, rdata, gdata, bdata 

#---------------------------------------------------------------------------
def runProcess(bboxs, mapname, outPath, q, pindex, ver, haswatermark):
    """ 多进程处理切图  """
    logger = logging.getLogger("")
    gm = srsweb.GlobalMercator(256)

    url = "http://mt%d.google.cn/vt/lyrs=s@132&x=%d&y=%d&z=%d" 
    for level, row, col in bboxs:
	downlaodTile(mapname, level, row, col, outPath, url, ver, haswatermark)
	logger.info("%d,%d" % (row, col))

def verifyLicense():
    dirName = appPath()
    fileList = os.listdir(dirName)
    for fp in fileList:
	if fp.endswith(".lic"):
	    dirName = os.path.join(dirName, fp)
	    break

    pn = os.path.abspath(dirName)
    if os.path.isfile(pn):
	lics = lic.License(pn)
	host = lics.hostName()
	return lics.verify(host, cm.APPID_GOOGLE_SIC3D)
    else:
	return False


# =============================================================================
class Download(object):
    """ 图像信息 """

    def __init__(self, taskFile=None):
	self.task = taskFile
	self.l, self.t, self.r, self.b = -180.0, 90.0, -180.0, 90.0
	self.level = 0
	self.out = ""
	self.tmp = ""
	self.name = ""
	self.url = "http://mt%d.google.cn/vt/lyrs=s@132&x=%d&y=%d&z=%d" 
	self.levels = []
	#logging = logging.getLogger("Download")
	self.haswatermark = False if verifyLicense() else True

    def parser(self):
	if self.task is None: return

	if not os.path.isfile(self.task): return 
	f = open(self.task, "r")
	for line in f:
	    line = line.strip()
	    if line=="" or line[0]=="#":continue
	    lr = line.split("=")
	    if len(lr)==2:
		l,r = lr[0].strip().lower(), lr[1].strip()
		if l=="bbox":
		    bbox = r.split(",")
		    if len(bbox)==4:
			for i in range(4):
			    bbox[i] = bbox[i].strip()

			if bbox[0]: self.l = float(bbox[0]) 
			if bbox[1]: self.t = float(bbox[1]) 
			if bbox[2]: self.r = float(bbox[2]) 
			if bbox[3]: self.b = float(bbox[3]) 
		elif l=="level":
		    levels = r.split(",")
		    for level in levels:
			level = level.strip()
			if not level.isdigit(): continue
			level = int(level)
			if level in self.levels: continue
			self.levels.append(level)
		    if self.levels:
			self.levels.sort()
		elif l=="out":
		    if os.path.isdir(r):
			self.out = r
		    
		    if self.out=="":
			self.out = appPath()

		elif l=="name":
		    self.name = r
	f.close()

    def splitByProcess(self, l,t,r,b, startl, endl, mpcnt):
	""" 根据进程数目,瓦片张数划分合理的任务 """
	tasks = []
	mkt = srsweb.GlobalMercator() 
	for i in range(startl, endl+1):
	    rs,re,cs,ce = mkt.calcRowColByLatLon(l,t,r,b, i) 
	    for row in range(rs, re+1):
		for col in range(cs, ce+1):
		    tasks.append((i, row, col))

	totalNums = len(tasks)
	splitNums = totalNums / mpcnt
	mplist = []
	add = 0
	for i in range(mpcnt):
	    mplist.append( tasks[i*splitNums:(i+1)*splitNums] )
	    add += splitNums

	if add<totalNums:
	    mplist[-1].extend( tasks[add-totalNums:] )
	return mplist

    def saveSciFile(self,l,t,r,b, startl, endl):
	""" 生成SuperMap缓存配置文件 """
	outPath = self.out

	mkt = srsweb.GlobalMercator() 
	l,t = mkt.LatLonToMeters(t, l)
	r,b = mkt.LatLonToMeters(b, r)
	mapBnd = l,t,r,b
	res = mkt.Resolution(endl)
	w,h = (r-l)/res, (t-b)/res 
	w,h = math.ceil(w), math.ceil(h)

	l,t = mkt.LatLonToMeters(90.0, -180.0)
	r,b = mkt.LatLonToMeters(-90, 180.0)
	idxBnd = l,t,r,b

	scales = []
	for level in range(startl, endl+1):
	    scale = mkt.Scale(level)
	    scales.append(scale) 
	    

	sci = smsci.smSci()
	sci.setParams(self.name, mapBnd, mapBnd, smsci.VER31)
	sci.setWidthHeight(w,h)
	sci.setScales(scales)
	sci.setProj(scitemplate.webmkt_prj)
	sci.saveSciFile(outPath)

    def doJob(self, startl, endl):
	l,t,r,b = self.l, self.t, self.r, self.b
	self.saveSciFile(l, t, r, b, startl, endl)

	ini = cm.iniFile()
	mpcnt = max(1, ini.mpcnt)
	mplist = self.splitByProcess(l,t,r,b, startl, endl, mpcnt)
	picNums = 0
	for i in xrange(len(mplist)):
	    picNums += len(mplist[i])

	logging.info("地理范围:左上右下(%f,%f,%f,%f)" % (l,t,r,b))
	logging.info("起始终止层级:(%d,%d), 瓦片总数%d张." % (startl, endl, picNums))

	logging.info("Start.")
	
	plist = []
	m = mp.Manager()
	q = m.Queue()
	for i in xrange(len(mplist)):
	    bboxs = mplist[i]
	    p = mp.Process(target=runProcess, args=(bboxs,self.name, self.out,q,i+1,smsci.VER31,self.haswatermark))
	    plist.append( (p, len(bboxs)) )

	for p, cnt in plist:
	    p.start()

	for p, cnt in plist:
	    p.join()
	    logging.info("子进程(id=%d), 瓦片张数(%d), 已完成." % (p.pid, cnt) )

	logging.info("End, All done.")
	logging.info(38 * "=")

# =============================================================================
def run(taskfile):
    down = Download(taskfile)
    down.parser()
    startl, endl = down.levels[0], down.levels[-1]
    down.doJob(startl, endl)

# =============================================================================

def main(taskfile):
    dirName = appPath()
    #name =  time.strftime("%Y-%m-%d %H-%M-%S.log")
    name = time.strftime("%Y-%m-%d.log")
    logfile = os.path.join(dirName, "log", name)
    logfile = os.path.abspath(logfile)
    if not os.path.exists(os.path.dirname(logfile)):
	os.makedirs(os.path.dirname(logfile))

    # create logger with "spam_application"
    logger = logging.getLogger("")
    logger.setLevel(logging.DEBUG)

    # create file handler which logs even debug messages
    fh = logging.FileHandler(logfile)
    fh.setLevel(logging.DEBUG)

    # create console handler with a higher log level
    ch = logging.StreamHandler()
    ch.setLevel(logging.DEBUG)

    # create formatter and add it to the handlers
    formatter = logging.Formatter(fmt="%(asctime)s %(message)s", datefmt="%Y-%m-%d %H:%M:%S >")
    fh.setFormatter(formatter)
    ch.setFormatter(formatter)

    # add the handlers to the logger
    logger.addHandler(fh)
    logger.addHandler(ch)

    logger.info(cm.APPNAME_GOOGLE_SCI3D) 
    if os.path.isfile(taskfile):
	run(taskfile)
    else:
	logger.error("file not found, %s" % taskfile)

if __name__=="__main__":
    mp.freeze_support()

    if verifyLicense():
	msg  =strlic = "\n授权版本 %s." % lic.License.hostName()
    else:
	msg  = "\n免费试用版本."

    parser = argparse.ArgumentParser(description="Download GoogleMaps to SuperMap tile files",
	    epilog="Author: 42848918@qq.com"+msg)
    parser.add_argument("-f", "--file", default="g.tsk", help="task file.")

    args = parser.parse_args()
    main(args.file)

