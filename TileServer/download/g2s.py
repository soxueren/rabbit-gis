#!/usr/bin/env python
# --*-- coding:gbk --*--

import os
import sys
import urllib2
import random
import multiprocessing as mp
import logging
import time
import argparse
import math
import ConfigParser

import numpy
from array import array

try:
    from osgeo import gdal
    #from osgeo import ogr
    #from osgeo import osr
except:
    import gdal
    #import ogr
    print("""You are using "old gen" bindings. gdal2tiles needs "new gen" bindings.""")
    sys.exit(1)

import tileserver
from tileserver.common import watermark as wmk
from tileserver.common import sci as smsci
from tileserver.common import srsweb
from tileserver.common import comm as cm
from tileserver.common import license as lic

import tsk
import scitemplate

#---------------------------------------------------------------------------
logger = logging.getLogger("g2s")
LICENSE_APP_NAME = "g2s"

#---------------------------------------------------------------------------
def downOneTile(url, fp):
    """ 一次下载 """
    try:
        f = urllib2.urlopen(url)
    except urllib2.URLError, e:
        print url, e.reason
        return None
    return f.read()


#---------------------------------------------------------------------------
def download_tile(mapname, level, row, col, outPath, ext, strurl, ver, haswatermark):
    """ 下载瓦片到本地 """

    tmp = strurl 
    mkt = srsweb.GlobalMercator()
    scale = mkt.Scale(level) 
    fp = smsci.smSci.calcTileName(outPath, mapname, scale, row, col, ext, ver) 

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

    if haswatermark: # 加水印
	tmp_ds = gdal.Open(fp, gdal.GA_ReadOnly) # 假定输入数据为24位png或jpg
	ts  = tmp_ds.RasterXSize

        mem_drv = gdal.GetDriverByName('MEM')
        mem_ds = mem_drv.Create('', ts, ts, 3)# 输出固定为24位png或jpg
	for i in range(1, 4):
	    tile_data = tmp_ds.GetRasterBand(i).ReadAsArray(0,0,ts,ts,ts,ts) 
	    if i==1:
		wmk.printWatermark(tile_data, wmk.buf_xsize, wmk.buf_ysize, wmk.buf_tile_server)
	    mem_ds.GetRasterBand(i).WriteArray(tile_data)

	ext = ext[-3:].lower() 
	if ext == 'jpg':
	    dirv = 'JPEG'
	elif ext == 'png':
	    dirv = 'PNG'

        out_drv=gdal.GetDriverByName(dirv)
        del tmp_ds
        out_drv.CreateCopy(fp, mem_ds, strict=0)
        del out_drv, mem_drv

#---------------------------------------------------------------------------
def multi_process_fun(bboxs, mapname, outPath, ext, pindex, ver, url,haswatermark):
    """ 多进程处理切图  """
    for level, row, col in bboxs:
        download_tile(mapname, level, row, col, outPath,ext, url, ver, haswatermark)

# =============================================================================
class Download(object):
    """ 图像信息 """

    def __init__(self, argv):
        self.l, self.t, self.r, self.b = -180.0, 90.0, -180.0, 90.0
        self.out = ""
        self.tmp = ""
        self.name = ""
        self.url = "http://mt%d.google.cn/vt/lyrs=s@132&x=%d&y=%d&z=%d" 
	self.sm_cache_ver = smsci.VER31
	self.file_format = 'jpg'
        self.levels = []
        self.haswatermark = False if self.verifyLicense() else True
        self.args = None
        self.argparse_init(argv)
        self.parse_task()
	self.mpcnt = 1
	self.config_init()

    def argparse_init(self, argv):
        if self.verifyLicense():
            msg  =strlic = "\n授权版本 %s." % lic.License.hostName()
        else:
            msg  = "\n免费试用版本."

	msg = tileserver.__author__ + msg
        parser = argparse.ArgumentParser(description="谷歌地图转超图缓存.",
                epilog=msg)

        parser.add_argument("file", default="g.tsk", help="task file.")
        self.args = parser.parse_args(argv)

    def verifyLicense(self):
        dirName = cm.app_path()
        fileList = os.listdir(dirName)
        for fp in fileList:
            if fp.endswith(".lic"):
                dirName = os.path.join(dirName, fp)
                break

        pn = os.path.abspath(dirName)
        if os.path.isfile(pn):
            lics = lic.License(pn)
            host = lics.hostName()
            return lics.verify(host, LICENSE_APP_NAME)
        else:
            return False

    def parse_task(self):
        taskfile = self.args.file
        if not os.path.isfile(taskfile):
            taskfile = os.path.join(cm.app_path(), "g.tsk")
        if not os.path.isfile(taskfile):
            logger.error("任务文件未找到,%s" % taskfile)
            return None

        f = open(taskfile, "r")
	lines = f.readlines()
        f.close()
	
	_tsk = tsk.from_lines(lines)
	if 'bbox' in _tsk:
	    self.l,self.t,self.r,self.b =  _tsk['bbox']
	if 'level' in _tsk:
	    self.levels = _tsk['level']
	if 'out' in _tsk:
	    self.out = _tsk['out']
	if 'name' in _tsk:
	    self.name = _tsk['name']
	if 'format' in _tsk:
	    self.file_format= _tsk['format']
	if 'version' in _tsk:
	    r = _tsk['version'].lower()
	    if r=="ver31":
		self.sm_cache_ver = smsci.VER31
	    elif r=="ver30":
		self.sm_cache_ver = smsci.VER30
	    elif r=="ver21":
		self.sm_cache_ver = smsci.VER21
	    elif r=="ver20":
		self.sm_cache_ver = smsci.VER20
	if 'overwrite' in _tsk:
	    self.over_write = True if _tsk['overwrite'].lower()=='true' else False

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

        _sci = smsci.smSci()
        _sci.setParams(self.name, mapBnd, idxBnd, self.sm_cache_ver)
        _sci.setWidthHeight(w,h)
        _sci.setScales(scales)
        _sci.setProj(scitemplate.webmkt_prj)
	_sci.setFileFormat(self.file_format)
        _sci.saveSciFile(outPath)

    def run(self):
        startl, endl = self.levels[0], self.levels[-1]
        l,t,r,b = self.l, self.t, self.r, self.b
        self.saveSciFile(l, t, r, b, startl, endl)

        mplist = self.splitByProcess(l,t,r,b, startl, endl, self.mpcnt)
        picNums = 0
        for i in xrange(len(mplist)):
            picNums += len(mplist[i])

	logger.info("TaskFile:%s" % self.args.file)
        logger.info("地理范围:左上右下(%f,%f,%f,%f)" % (l,t,r,b))
        logger.info("起始终止层级:(%d,%d), 瓦片总数%d张." % (startl, endl, picNums))

        logger.info("Start.")
        
        plist = []
        m = mp.Manager()

        for i in xrange(len(mplist)):
            bboxs = mplist[i]
            p = mp.Process(target=multi_process_fun, args=(bboxs,self.name, self.out, \
		self.file_format,i+1, self.sm_cache_ver,self.url,self.haswatermark))
            plist.append( (p, len(bboxs)) )

        for p, cnt in plist:
            p.start()

        for p, cnt in plist:
            p.join()
            logger.info("子进程(id=%d), 瓦片张数(%d), 已完成." % (p.pid, cnt) )

        logger.info("End, All done.")
        logger.info(38 * "=")

    def config_init(self):
        config = ConfigParser.ConfigParser()
	cfg_path = os.path.join(cm.app_path(), "g2s.cfg")
        config.read(cfg_path)
        if config is not None:
            self.mpcnt = config.getint("config", "multiprocess")
            self.mpcnt = max(1, self.mpcnt) # 确保大于1
            self.url = config.get("image", "url")

# =============================================================================
def log_init():
    """ 初始化日志 """
    dirName = cm.app_path()
    name = time.strftime("%Y-%m-%d.log")
    logfile = os.path.join(dirName, "log", name)
    logfile = os.path.abspath(logfile)
    if not os.path.exists(os.path.dirname(logfile)):
	os.makedirs(os.path.dirname(logfile))

    logger = logging.getLogger("")
    logger.setLevel(logging.DEBUG)

    fh = logging.FileHandler(logfile)
    fh.setLevel(logging.DEBUG)

    ch = logging.StreamHandler()
    ch.setLevel(logging.DEBUG)

    formatter = logging.Formatter(fmt="%(asctime)s %(message)s", datefmt="%Y-%m-%d %H:%M:%S >")
    fh.setFormatter(formatter)
    ch.setFormatter(formatter)

    logger.addHandler(fh)
    logger.addHandler(ch)


def main():
    log_init()
    Download(sys.argv[1:]).run()

# =============================================================================
if __name__=="__main__":
    mp.freeze_support()
    main()

