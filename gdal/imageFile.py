#!/usr/bin/env python
# --*-- coding:utf-8 --*--

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
#import math
#import re 

class ImageFile(object):
    ''' 图像信息 '''

    def __init__(self, fileName):
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
	self.iBandNums=None
	self.ds=None
	self.process()

    def getWidthHeight(self):
	return self.xSize, self.ySize
    
    def getResolution(self):
	return (self.xRes, self.xRes)

    def getBound(self):
	return (self.dMinX, self.dMaxY, self.dMaxX, self.dMinY)
    
    def getProjection(self):
	return self.srsWkt

    def isGeographic(self):
	if self.srsWkt=='': return False
	srs=osr.SpatialReference()
        srs.ImportFromWkt(self.srsWkt)
	return srs.IsGeographic()
	

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
	self.iBandNums = self.ds.RasterCount
	
	'''
	print ds.RasterXSize, ds.RasterYSize, ds.RasterCount
	hBand = ds.GetRasterBand(1)
	print gdal.GetDataTypeName(hBand.DataType)
        print gdal.GetColorInterpretationName(hBand.GetRasterColorInterpretation()) 
	'''

	return True

    def cut(self, l, t, r, b, ts, fp):
	''' 按照指定范围切割影像
	l, t, r, b 要切割的地理范围
	ts 切割后的影像宽度高度(像素单位)
	fp 切割后影像文件全路径
	'''
	if l>self.dMaxX or t<self.dMinY or r<self.dMinX or b>self.dMaxY:
	    return None

	((rowInFileStart, rowInFileEnd, colInFileStart,colInFileEnd), \
	    (rowInTileStart, rowInTileEnd, colInTileStart, colInTileEnd)) \
	    = self.posOneTile(l, t, r, b, ts)
	
	tilebands = 3#self.iBandNums
	mem_drv = gdal.GetDriverByName('MEM')
	mem_ds=mem_drv.Create('', ts, ts, tilebands)
	if mem_ds is None: return 
	#print fp

	rowFileSize=rowInFileEnd-rowInFileStart
	colFileSize=colInFileEnd-colInFileStart
	rowTileSize=rowInTileEnd-rowInTileStart
	colTileSize=colInTileEnd-colInTileStart

	if self.iBandNums==1:
	    tile_data = numpy.zeros((ts, ts), numpy.uint8)
	    band = self.ds.GetRasterBand(1)
	    data = band.ReadAsArray(colInFileStart, rowInFileStart,
			    colFileSize, rowFileSize, colTileSize, rowTileSize)
	    tile_data[rowInTileStart:rowInTileEnd,colInTileStart:colInTileEnd]=data
	    mem_ds.GetRasterBand(1).WriteArray(tile_data)
	    mem_ds.GetRasterBand(2).WriteArray(tile_data)
	    mem_ds.GetRasterBand(3).WriteArray(tile_data)
	    del tile_data
	else:
	    for iband in range(1, self.iBandNums+1):
		tile_data = numpy.zeros((ts, ts), numpy.uint8)
		band = self.ds.GetRasterBand(iband)
		data = band.ReadAsArray(colInFileStart, rowInFileStart,
				colFileSize, rowFileSize, colTileSize, rowTileSize)
		#print data.shape, data.dtype
		#print tile_data.shape, tile_data.dtype
		tile_data[rowInTileStart:rowInTileEnd,colInTileStart:colInTileEnd]=data
		mem_ds.GetRasterBand(iband).WriteArray(tile_data)
		del tile_data
	
	out_drv=gdal.GetDriverByName(self.getDriverName(fp))
	out_drv.CreateCopy(fp, mem_ds, strict=0)
	del out_drv
	del mem_drv


    def posOneTile(self, l, t, r, b, ts):
	''' 定位瓦片在影像中的像素范围
	l, t, r, b 要切割的地理范围
	ts 切割后的影像宽度高度(像素单位)
	'''
	tres = (r-l)/ts # 切片分辨率
	rres = min(self.xRes, self.yRes)
	rl,rt,rr,rb=self.dMinX, self.dMaxY, self.dMaxX, self.dMinY
	rowInFileStart, rowInTileStart=0,0
	if t<self.dMaxY:
	    rowInFileStart=int((rt-t)/rres)
	else:
	    rowInTileStart=int((t-rt)/tres)
	
	rowInFileEnd, rowInTileEnd=self.ySize, ts 
	if b>self.dMinY:
	    rowInFileEnd=int((rt-b)/rres)
	else:
	    rowInTileEnd=int((t-rb)/tres)

	colInFileStart, colInTileStart=0,0
	if l>self.dMinX:
	    colInFileStart=int((l-rl)/rres)
	else:
	    colInTileStart=int((rl-l)/tres)

	colInFileEnd, colInTileEnd=self.xSize,ts
	if r<self.dMaxX:
	    colInFileEnd=int((r-rl)/rres)
	else:
	    colInTileEnd=int((rr-l)/tres)

	return ((rowInFileStart, rowInFileEnd, colInFileStart,colInFileEnd), \
	    (rowInTileStart, rowInTileEnd, colInTileStart, colInTileEnd))

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
	oneimg = ImageFile(fName)
	dl, dt, dr, db = oneimg.getBound()
	dx, dy = oneimg.getResolution()
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
    fileName=r'E:\新建文件夹\全市域裁切影像\新建文件夹\1-1.tif'
    imgf=ImageFile(fileName)
    print imgf.getProjection()
    print imgf.isGeographic()
    del imgf

def unitTest():
    import smSci
    fileName=r'E:\2013\2013-06\2013-06-17\srtm_47_01.tif'
    imgf = ImageFile(fileName) 
    dl, dt, dr, db = imgf.getBound()
    print dl, dt, dr, db
    level, ts=10, 256
    rs,re,cs,ce=smSci.smSci3d.calcRowCol(dl,dt,dr,db,level) 
    print rs,re,cs,ce 
    for row in xrange(rs, re+1):
	for col in xrange(cs, ce+1):
	    l,t,r,b=smSci.smSci3d.calcBndByRowCol(row,col,level)
	    #print l,t,r,b
	    fp=os.path.dirname(fileName)
	    fp = os.path.join(fp, 'out', ('%d_%d.png' % (row, col)))
	    if not os.path.exists(os.path.dirname(fp)):
		os.makedirs(os.path.dirname(fp))
	    imgf.cut(l,t,r,b,ts,fp)

if __name__=='__main__':
    #unitTest()
    unitTestProj()

