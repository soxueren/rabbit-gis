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
	self.callbackfun=None
	pass
    
    def hook(self, callback):
	self.callbackfun=callback

    def printLog(self, msg, newline=True):
	if self.callbackfun is None:return
	self.callbackfun(msg, newline)
    
    def toTiles(self, imgList, level, outPath):
	imgbound={}
	for fName in imgList:
	    fPath = os.path.join(self.filePath, fName)
	    oneImg = img.ImageFile(fPath)
	    self.printLog(("begin file:%s" % fPath))
	    dl, dt, dr, db = oneImg.getBound()
	    rs,re,cs,ce=smSci.smSci3d.calcRowCol(dl,dt,dr,db,level) 
	    for row in xrange(rs, re+1):
		for col in xrange(cs, ce+1):
		    l,t,r,b=smSci.smSci3d.calcBndByRowCol(row,col,level)
		    fp = smSci.smSci3d.calcTileName(level,row,col,outPath)+'.jpg'
		    if not os.path.exists(os.path.dirname(fp)):
			os.makedirs(os.path.dirname(fp))
		    oneImg.cut(l,t,r,b,TILESIZE256, fp)
		    self.printLog(("added tiles:(%d/%d)" % ((row-rs)*(ce-cs)+col-cs,(re-rs+1)*(ce-cs+1))), False)
	    del oneImg
	return True

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
    filePath=r'E:\2013\2013-06\2013-06-17'
    outPath=r'E:\2013\2013-06\2013-06-17\out'
    mapname, ext='out', 'jpg'
    imgtile = Image2Tiles(filePath) 
    imgList = imgtile.listDir(filePath, 'tif')
    #imgList = imgList[:1]
    l,t,r,b, xres, yres = img.calcBoundary(imgList)
    mapbnd=l,t,r,b

    endl=smSci.smSci3d.calcEndLevel(xres)
    startl=smSci.smSci3d.calcStartLevel(l,t,r,b,xres,endl)
    w,h=smSci.smSci3d.calcWidthHeight(l,t,r,b,endl)
    
    sci = smSci.smSci3d()
    sci.setParams(mapname, mapbnd, mapbnd, '')
    sci.setLevels(startl, endl)
    sci.setExtName(ext)
    sci.setWidthHeight(w,h)
    sci.saveSciFile(outPath)
    
    for i in xrange(endl, startl, -1):
	imgtile.toTiles(imgList, i, outPath)

if __name__=='__main__':
    #argv = gdal.GeneralCmdLineprocessor( sys.argv )
    #if len(argv)==1: sys.exit(1) 
    unitTest()

