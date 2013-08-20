#!/usr/bin/env python
# --*-- coding:gbk --*--

import sys

try:
    from osgeo import gdal
    #from osgeo import ogr
    from osgeo import osr
except:
    import gdal
    import ogr
    print('You are using "old gen" bindings. gdal2tiles needs "new gen" bindings.')
    sys.exit(1)

import numpy
import os
from array import array
#import math
#import re 
import watermark as wmk

class ImageFile(object):
    ''' 图像信息 '''

    def __init__(self, fileName, lic=False):
	self.fileName=fileName
	self.srsWkt=None
	self.xSize=None
	self.ySize=None
	self.dMinX=None
	self.dMinY=None
	self.dMaxX=None
	self.dMaxY=None
	self.xRes=None
	self.yRes=None
	self.ibandCount=None
	self.ds=None
	self.ct = None # CoordinateTransformation坐标转换器
	self.srs = None # 影像坐标系
	self.license = lic
	self.process()

    def getWidthHeight(self):
	return self.xSize, self.ySize
    
    def getResolution(self):
	return (self.xRes, self.xRes)

    def getBBox(self):
	return (self.dMinX, self.dMaxY, self.dMaxX, self.dMinY)

    def resetBBox(self):
	''' 投影坐标范围转成经纬度坐标范围 '''
	if self.ct is None: return None
	l,t,r,b = self.dMinX, self.dMaxY, self.dMaxX, self.dMinY 

	try:
	    (self.dMinX, self.dMaxY, height) = self.ct.TransformPoint(l, t)
	    (self.dMaxX, self.dMinY, height) = self.ct.TransformPoint(r, b)
	    self.xRes = (self.dMaxX-self.dMinX)/self.xSize
	    self.yRes = (self.dMaxY-self.dMinY)/self.ySize
	except:
	    print 'resetBBox::无法进行坐标转换'
	
    
    def getProjection(self):
	return self.srsWkt

    def isInGeographic(self):
	''' 文件坐标是否在(-180,90,180,-90)范围内 '''
	l,t,r,b = self.getBBox()
	if l>180.0 or t<-90.0 or r<-180.0 or b>90:
	    return False
	return True

    def isGeographic(self):
	if self.srsWkt=='': return False
	srs=osr.SpatialReference()
        srs.ImportFromWkt(self.srsWkt)
	return srs.IsGeographic()

    def canbeGeographic(self):
	''' 投影坐标是否可以转为经纬度坐标 '''
	return True if self.ct is not None else False

    def process(self):
	if not os.path.isfile(self.fileName):
	    print 'file[%s] not exits.' % self.fileName
	    return False

	self.ds = gdal.Open(self.fileName, gdal.GA_ReadOnly)
	if self.ds is None:
	    print "unable to open '%s'." % self.fileName
	    return False
	
	#/* -------------------------------------------------------------------- */
	#/*      Report projection.                                              */
	#/* -------------------------------------------------------------------- */
	self.srsWkt = self.ds.GetProjectionRef()
	self.srs = srs = osr.SpatialReference()
	srs.ImportFromWkt(self.srsWkt)
	srsLatLong = srs.CloneGeogCS()
	try:
	    self.ct = osr.CoordinateTransformation(srs, srsLatLong)
	    #print 'hassattr,TransformPoint->', hasattr(self.ct, 'TransformPoint') 
	    #print callable(getattr(self.ct,'TransformPoint', None))
	except:
	    print self.ct==None

	self.xSize, self.ySize = self.ds.RasterXSize, self.ds.RasterYSize

	adfGeoTransform = self.ds.GetGeoTransform(can_return_null = True)
	if adfGeoTransform is None:
	    return False

        self.dMinX = adfGeoTransform[0] + adfGeoTransform[1] * 0 + adfGeoTransform[2] * 0 
        self.dMaxY = adfGeoTransform[3] + adfGeoTransform[4] * 0 + adfGeoTransform[5] * 0 
        self.dMaxX = adfGeoTransform[0] + adfGeoTransform[1] * self.xSize + adfGeoTransform[2] * self.ySize
        self.dMinY = adfGeoTransform[3] + adfGeoTransform[4] * self.xSize + adfGeoTransform[5] * self.ySize

	self.xRes = (self.dMaxX-self.dMinX)/self.xSize
	self.yRes = (self.dMaxY-self.dMinY)/self.ySize
	self.ibandCount = self.ds.RasterCount
	
	'''
	print ds.RasterXSize, ds.RasterYSize, ds.RasterCount
	hBand = ds.GetRasterBand(1)
	print gdal.GetDataTypeName(hBand.DataType)
        print gdal.GetColorInterpretationName(hBand.GetRasterColorInterpretation()) 
	'''

	return True

    def cut(self, l, t, r, b, ts, fp, isBil=False, logs=None):
	''' 按照指定范围切割影像
	l, t, r, b 要切割的地理范围
	ts 切割后的影像宽度高度(像素单位)
	fp 切割后影像文件全路径
	isBil 是否输出为bil地形
	logs 日志信息数组
	'''
	if l>self.dMaxX or t<self.dMinY or r<self.dMinX or b>self.dMaxY:
	    return None
	
	if isBil:
	    self.cut4Bil(l, t, r, b, ts, fp, logs)
	else:
	    self.cut4Image(l, t, r, b, ts, fp, logs)
	
    def cut4Image(self,l, t, r, b, ts, fp, logs=None):
	''' 切分为影像文件 '''

	(bOutOfFile, (ry, ry2, rx,rx2),(wy,wy2,wx,wx2))=self.fixTilePos(l, t, r, b, ts)

	mem_drv = gdal.GetDriverByName('MEM')
	mem_ds=mem_drv.Create('', ts, ts, 3)# 输出固定为24位png或jpg
	if mem_ds is None: return 
	#print fp

	tmp_ds = None
	if bOutOfFile and os.path.exists(fp):
	    tmp_ds = gdal.Open(fp, gdal.GA_ReadOnly) # 假定输入数据为24位png或jpg

	rysize = ry2-ry
	rxsize = rx2-rx
	wysize = wy2-wy
	wxsize = wx2-wx

	if self.ibandCount==1:
	    data = self.ds.GetRasterBand(1).ReadAsArray(rx, ry, rxsize, rysize, wxsize, wysize)
	    # 跨边界的瓦片需要进行瓦片合并
	    if bOutOfFile and os.path.exists(fp) and tmp_ds is not None:
		for i in range(1, 4):
		    tile_data = tmp_ds.GetRasterBand(i).ReadAsArray(0,0,ts,ts,ts,ts) 
		    if tile_data is not None:
			tile_data[wy:wy2,wx:wx2] = data
			mem_ds.GetRasterBand(i).WriteArray(tile_data)
			del tile_data
	    else:
		tile_data = numpy.zeros((ts, ts), numpy.uint8)
		tile_data[wy:wy2,wx:wx2]=data
		for i in range(1, 4):
		    mem_ds.GetRasterBand(i).WriteArray(tile_data)
		del tile_data

	elif self.ibandCount==3:
	    for i in range(1, 4):
		tile_data = numpy.zeros((ts, ts), numpy.uint8)
		# 跨边界的瓦片需要进行瓦片合并
		if bOutOfFile and os.path.exists(fp) and tmp_ds is not None:
		    tile_data = tmp_ds.GetRasterBand(i).ReadAsArray(0,0,ts,ts,ts,ts) 
		band = self.ds.GetRasterBand(i)
		data = band.ReadAsArray(rx, ry, rxsize, rysize, wxsize, wysize)
		if data is not None and tile_data is not None:
		    tile_data[wy:wy2,wx:wx2] = data
		    mem_ds.GetRasterBand(i).WriteArray(tile_data)
		else:
		    logs.append('read tile fialed. rx:%d,ry:%d,rxsize:%d, rysize:%d' % (rx, ry, rxsize, rysize))
		del tile_data
	else:
	    if logs is not None:
		logs.append('Unsupport band count %d' % self.ibandCount)
		return
	
	if not self.license:
	    self.watermark(mem_ds)

	out_drv=gdal.GetDriverByName(self.getDriverName(fp))
	'''
	# for test
	if bOutOfFile and os.path.exists(fp):
	    fp = fp[:-4]+'_2'+fp[-4:]
	    logs.append('file is exists. new file %s' % fp)
	'''
	del tmp_ds
	out_drv.CreateCopy(fp, mem_ds, strict=0)
	del out_drv, mem_drv

    def watermark(self, mem_ds):
	tx, ty = mem_ds.RasterXSize, mem_ds.RasterYSize
	tile_data = mem_ds.GetRasterBand(1).ReadAsArray(0,0,tx,ty,tx,ty) 
	wmk.printWatermark(tile_data, wmk.buf_xsize, wmk.buf_ysize, wmk.buf_tile_server)
	mem_ds.GetRasterBand(1).WriteArray(tile_data)

    def cut4Bil(self,l, t, r, b, ts, fp, logs=None):
	''' 切分为地形文件 '''

	(bOutOfFile,(ry,ry2,rx,rx2),(wy,wy2,wx,wx2)) = self.fixTilePos(l, t, r, b, ts)

	tilebands = self.ibandCount if self.ibandCount<=3 else 3

	rysize=ry2-ry
	rxsize=rx2-rx
	wysize=wy2-wy
	wxsize=wx2-wx

	tile_data = numpy.zeros((ts, ts), numpy.int16)
	if bOutOfFile and os.path.exists(fp):
	    self.loadFromBil(tile_data, fp)
	    #pass

	if self.ibandCount==1:
	    data = self.ds.GetRasterBand(1).ReadAsArray(rx, ry, rxsize, rysize, wxsize, wysize)
	    tile_data[wy:wy2,wx:wx2]=data
	elif self.ibandCount==3:
	    tile1 = numpy.zeros((wysize,wxsize), numpy.int32)
	    tile2 = numpy.zeros((wysize,wxsize), numpy.int16)
	    tile3 = numpy.zeros((wysize,wxsize), numpy.int8)

	    data1 = self.ds.GetRasterBand(1).ReadAsArray(rx, ry, rxsize, rysize, wxsize, wysize)
	    data2 = self.ds.GetRasterBand(2).ReadAsArray(rx, ry, rxsize, rysize, wxsize, wysize)
	    data3 = self.ds.GetRasterBand(3).ReadAsArray(rx, ry, rxsize, rysize, wxsize, wysize)
	    tile1[:], tile2[:], tile3[:] = data1[:], data2[:], data3[:]
	    #print tile1.dtype

	    tile_data[wy:wy2,wx:wx2] = (tile1<<16) + tile2 + tile1
	    del tile1, tile2, tile3, data1,data2,data3
	else:
	    if logs is not None:
		logs.append('Unsupport band count %d' % self.ibandCount)
		del tile_data
		return

	'''
	# for test
	if bOutOfFile and os.path.exists(fp):
	    fp = fp[:-4]+'_2'+fp[-4:]
	    logs.append('file is exists. new file %s' % fp)
	'''
	if not self.license:
	    wmk.printWatermark(tile_data, wmk.buf_xsize, wmk.buf_ysize, wmk.buf_tile_server)

	#print tile_data.shape#, tile_data
	f=open(fp, 'wb')
	for i in xrange(ts):
	    line=array('h', tile_data[i])
	    line.write(f)
	f.close()
	del tile_data

    def loadFromBil(self, tile_data, fp):
	''' bil文件内容加载到内存,bil文件固定为256x256大小 '''
	data = numpy.fromfile(fp, dtype=numpy.int16)
	for i in xrange(256):
	    tile_data[i] = data[i*256:i*256+256]

    def fixTilePos(self, l, t, r, b, ts):
	''' 定位瓦片在影像中的像素范围
	l, t, r, b 要切割的地理范围
	ts 切割后的影像宽度高度(像素单位)
	'''
	tres = (r-l)/ts # 切片分辨率
	rres = min(self.xRes, self.yRes)

	bOutOfFile = False # 是否超出文件范围

	# 影像的地理范围
	rl,rt,rr,rb=self.dMinX, self.dMaxY, self.dMaxX, self.dMinY
	ry, wy=0,0 # 影像文件行号(起始),切分结果文件行号(起始)

	if t>rt:
	    bOutOfFile=True
	    wy=int((t-rt)/tres)
	else:
	    ry=int((rt-t)/rres)

	# 影像文件行号(终止),切分结果文件行号(终止)
	ry2, wy2=self.ySize, ts 
	if b<rb:
	    bOutOfFile=True
	    wy2=int((t-rb)/tres)
	else:
	    ry2=int((rt-b)/rres)

	rx, wx=0,0 # 影像列号(起始),切分结果文件列号(起始)
	if l<rl:
	    bOutOfFile=True
	    wx=int((rl-l)/tres)
	else:
	    rx=int((l-rl)/rres)

	# 影像列号(终止),切分结果文件列号(终止)
	rx2, wx2=self.xSize,ts
	if r>rr:
	    bOutOfFile=True
	    wx2=int((rr-l)/tres)
	else:
	    rx2=int((r-rl)/rres)
	
	# 确保影像行列号不超范围
	if rx2>self.xSize: rx2=self.xSize
	if ry2>self.ySize: ry2-self.ySize

	return (bOutOfFile, (ry,ry2,rx,rx2), (wy,wy2,wx,wx2))

    def getDriverName(self, fPath):
	ext = fPath[-3:]
	if ext.upper()=='PNG':return 'PNG'
	elif ext.upper()=='JPG':return 'JPEG'
	elif ext.upper()=='GIF':return 'GIF'
	else: return 'PNG'

def calcBoundary(imgList):
    ''' 计算一组影像地理范围及分辨率 '''
    dls, dts, drs, dbs, dxs, dys=[],[],[],[],[],[]
    for fName in imgList:
	imgfile = ImageFile(fName)
	dl, dt, dr, db = imgfile.getBBox()
	dx, dy = imgfile.getResolution()
	dls.append(dl)
	dts.append(dt)
	drs.append(dr)
	dbs.append(db)
	dxs.append(dx)
	dys.append(dy)

    l,t,r,b = min(dls), max(dts), max(drs), min(dbs)
    xres, yres = min(dxs), min(dys)
    return l,t,r,b,xres,yres

def calcGeographicBoundary(imgList):
    ''' 计算一组影像地理范围及分辨率(经纬度下) '''
    dls, dts, drs, dbs, dxs, dys=[],[],[],[],[],[]
    for fName in imgList:
	imgfile = ImageFile(fName)
	if not imgfile.isGeographic():
	    if imgfile.canbeGeographic():
		imgfile.resetBBox()
	    else:
		continue
	dl, dt, dr, db = imgfile.getBBox()
	dx, dy = imgfile.getResolution()
	dls.append(dl)
	dts.append(dt)
	drs.append(dr)
	dbs.append(db)
	dxs.append(dx)
	dys.append(dy)

    l,t,r,b = min(dls), max(dts), max(drs), min(dbs)
    xres, yres = min(dxs), min(dys)
    return l,t,r,b,xres,yres


# =============================================================================
# =============================================================================
# =============================================================================

def unitTestProj():
    fileName=r'E:\新建文件夹\全市域裁切影像\全市域裁切影像\gttb_2011_09--2011_12_beijing.img'
    imgf=ImageFile(fileName)
    #print imgf.getProjection()
    print 'imgf.isGeographic', imgf.isGeographic()
    print 'imgf.ct', imgf.ct
    print 'imgf.canbeGeographic', imgf.canbeGeographic()
    print 'imgf.getBBox', imgf.getBBox()
    imgf.resetBBox()
    print 'imgf.getBBox', imgf.getBBox()
    #print calcGeographicBoundary([fileName])
    del imgf

def unitTest():
    import smSci
    fileName=r'E:\2013\2013-06\2013-06-17\srtm_47_01.tif'
    imgf = ImageFile(fileName) 
    dl, dt, dr, db = imgf.getBBox()
    print dl, dt, dr, db
    level, ts=10, 256
    rs,re,cs,ce=smSci.smSci3d.calcRowCol(dl,dt,dr,db,level) 
    print rs,re,cs,ce 
    for row in xrange(rs, re+1):
	for col in xrange(cs, ce+1):
	    l,t,r,b=smSci.smSci3d.calcBndByRowCol(row,col,level)
	    #print l,t,r,b
	    fp=os.path.dirname(fileName)
	    fp = os.path.join(fp, 'out', ('%d_%d.bil' % (row, col)))
	    if not os.path.exists(os.path.dirname(fp)):
		os.makedirs(os.path.dirname(fp))
	    imgf.cut(l,t,r,b,ts,fp, True)

if __name__=='__main__':
    #unitTest()
    unitTestProj()

