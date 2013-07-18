#!/usr/bin/env python
# --*-- coding:utf-8 --*--

import sys

try:
    from osgeo import gdal
    from osgeo import ogr
    from osgeo import osr
except:
    import gdal
    import ogr
    print('You are using "old gen" bindings. gdal2tiles needs "new gen" bindings.')
    sys.exit(1)

import os
import math
import re 
import imageFile as img
import smSci

TILESIZE256 = 256

class Image2Tiles(object):
    ''' 将image文件切割为瓦片 '''

    def __init__(self, filePath):
	self.filePath=filePath
	self.callbackfunLog=None
	self.ext='.jpg'
	pass
    
    def hook(self, callback):
	self.callbackfunLog=callback

    def printLog(self, msg):
	''' 记录日志 '''
	if self.callbackfunLog is None: return
	self.callbackfunLog(msg)


    def setExt(self, ext):
	self.ext=ext
    
    def toTiles(self, imgList, level, outPath, isBil=False, forlatlong=True):
	''' 生成瓦片 
	forlatlong 是否转为经纬度坐标   
	'''
	l,t,r,b, xres, yres=img.calcGeographicBoundary(imgList)
	totalNums = smSci.smSci3d.calcTileCount(l,t,r,b,level)
	curNums = 0
	
	for fName in imgList:
	    fPath = os.path.join(self.filePath, fName)
	    imgfile = img.ImageFile(fPath)
	    if forlatlong and not imgfile.isGeographic():
		if imgfile.canbeGeographic():
		    imgfile.resetBBox()
		else:
		    continue

	    self.printLog(("Reading file: %s" % fPath))
	    dl, dt, dr, db = imgfile.getBBox()
	    rs,re,cs,ce=smSci.smSci3d.calcRowCol(dl,dt,dr,db,level) 
	    #totalNums = (re-rs+1)*(ce-cs+1)
	    bStep = False if totalNums<200 else True
	    for row in xrange(rs, re+1):
		for col in xrange(cs, ce+1):
		    l,t,r,b=smSci.smSci3d.calcBndByRowCol(row,col,level)
		    fp = smSci.smSci3d.calcTileName(level,row,col,outPath)+self.ext
		    if not os.path.exists(os.path.dirname(fp)):
			os.makedirs(os.path.dirname(fp))
		    logs=[]
		    imgfile.cut(l,t,r,b,TILESIZE256, fp, isBil, logs)
		    curNums += 1 #(row-rs)*(ce-cs+1)+col-cs+1
		    if curNums<= totalNums:
			self.updateProgress(curNums, totalNums, bStep)
		    for log in logs:
			self.printLog(log)
	    del imgfile
	return True

    def updateProgress(self, curNums, totalNums, bStep=False):
	if bStep:
	    if curNums%50==0 or (totalNums-curNums)==0:
		self.printLog(("已处理%d张,共%d张" % (curNums, totalNums)))
	else:
	    self.printLog(("已处理%d张,共%d张" % (curNums, totalNums)))

    def createBound(self, dMinX, dMinY, dMaxX, dMaxY):
	''' 创建影像的外包矩形 '''
	shpPath = self.filePath
	shpPath = shpPath[:-4]+'.shp'

	driverName = 'ESRI Shapefile'
	drv = ogr.GetDriverByName(driverName)
	if drv is None:
	    print "%s driver not available.\n" % driverName
	    return False

	ds = drv.CreateDataSource(shpPath)
	if ds is None:
	    print "Creation of output file[%s] failed.\n" % shpPath 
	    return False

	lyr = ds.CreateLayer("bound", None, ogr.wkbLineString)
	if lyr is None:
	    print "Layer creation failed.\n"
	    return False

	field_defn = ogr.FieldDefn( "Name", ogr.OFTString )
	field_defn.SetWidth( 32 )

	if lyr.CreateField ( field_defn ) != 0:
	    print "Creating Name field failed.\n"

	feat = ogr.Feature( lyr.GetLayerDefn())
	#feat.SetField( "Name", name )

	lineString = ogr.Geometry(ogr.wkbLineString)
	lineString.AddPoint(dMinX, dMinY)
	lineString.AddPoint(dMinX, dMaxY)
	lineString.AddPoint(dMaxX, dMaxY)
	lineString.AddPoint(dMaxX, dMinY)
	lineString.AddPoint(dMinX, dMinY)
	feat.SetGeometry(lineString)

	if lyr.CreateFeature(feat) != 0:
	    print "Failed to create feature in shapefile.\n"

	feat.Destroy()
	ds = None

    @staticmethod
    def listDir(root, ext):
	''' 得到指定目录下文件列表 '''
	reExt='[\d\D]*\.tif$' 
	if ext.strip('.').upper()=='TIF':
	    reExt='[\d\D]*\.tif$' 
	elif ext.strip('.').upper()=='TIFF':
	    reExt='[\d\D]*\.tiff$' 
	elif ext.strip('.').upper()=='IMG':
	    reExt='[\d\D]*\.img$' 

	imgList=[]
	if os.path.isdir(root):
	    for fName in os.listdir(root):
		if re.match(reExt,fName,re.IGNORECASE) is not None:
		    imgList.append(os.path.join(root, fName))
	return imgList

# =============================================================================
# =============================================================================
# =============================================================================

def unitTest():
    filePath=r'E:\新建文件夹\全市域裁切影像\新建文件夹'
    outPath=r'E:\2013\2013-06\2013-06-17\out'
    mapname, ext='out', 'jpg'
    imgtile = Image2Tiles(filePath) 
    imgList = imgtile.listDir(filePath, 'tif')
    #imgList = imgList[:1]
    l,t,r,b, xres, yres = img.calcBoundary(imgList)
    mapbnd=l,t,r,b
    print mapbnd, xres,yres

    endl=smSci.smSci3d.calcEndLevel(xres)
    startl=smSci.smSci3d.calcStartLevel(l,t,r,b,xres,endl)
    w,h=smSci.smSci3d.calcWidthHeight(l,t,r,b,endl)
    
    sci = smSci.smSci3d()
    sci.setParams(mapname, mapbnd, mapbnd, '')
    sci.setLevels(startl, endl)
    sci.setExtName(ext)
    sci.setWidthHeight(w,h)
    sci.saveSciFile(outPath)
    print startl, endl, w,h

    return
    
    for i in xrange(endl, startl, -1):
	imgtile.toTiles(imgList, i, outPath)

if __name__=='__main__':
    #argv = gdal.GeneralCmdLineprocessor( sys.argv )
    #if len(argv)==1: sys.exit(1) 
    unitTest()

