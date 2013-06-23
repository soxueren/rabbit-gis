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
import numpy
import imageFile as img
import smSci

TILESIZE256 = 256

class Image2Tiles(object):
    ''' 将image文件切割为瓦片 '''

    def __init__(self, filePath):
	self.filePath=filePath
	pass
    
    def process(self):
	imgList = self.listDir(self.filePath)
	dleft, dtop, dright, dbottom, dxres, dyres = self.calcBoundary(imgList)
	pass

    def calcBoundary(self, imgList):
	''' 计算影像地理范围及分辨率 '''
	dls, dts, drs, dbs, dxs, dys=[],[],[],[],[],[]
	for fName in imgList:
	    fPath = os.path.join(self.filePath, fName)
	    oneImg = img.ImageFile(fPath)
	    dl, dt, dr, db = oneImg.getBound()
	    dx, dy = oneImg.getResolution()
	    dls.append(dl)
	    dts.append(dt)
	    drs.append(dr)
	    dbs.append(db)
	    dxs.append(dx)
	    dys.append(dy)

	dleft, dtop, dright, dbottom = min(dls), max(dts), max(drs), min(dbs)
	dxres, dyres = min(dxs), min(dys)
	return dleft, dtop, dright, dbottom, dxres, dyres

    def toTiles(self, imgList, level, outPath):
	imgbound={}
	gdal.AllRegister()
	for fName in imgList:
	    fPath = os.path.join(self.filePath, fName)
	    oneImg = img.ImageFile(fPath)
	    dl, dt, dr, db = oneImg.getBound()
	    rs,re,cs,ce=smSci.smSci3d.calcRowCol(dl,dt,dr,db,level) 
	    i,j=rs,cs
	    while(i<re):
		while(j<ce):
		    l,t,r,b=smSci.smSci3d.calcBndByRowCol(i,j,level)
		    atile = numpy.zeros((TILESIZE256, TILESIZE256),int)
		    oneImg.cut(l,t,r,b,TILESIZE256, atile)
		    self.saveOneTile(atile,level, i,j,outPath)
		    j+=1
		i+=1
	    imgbound[fName]=(dl, dt, dr, db)
	
	dleft, dtop, dright, dbottom, dxres, dyres = self.calcBoundary(imgList)
	dTileXSize=TILESIZE256*dxres # 瓦片的地理范围
	dTileYSize=TILESIZE256*dyres
	return

    def saveOneTile(self,atile, level, row, col, path):
	fName = '%04d_%04d_0000.png' % (row,col)
	rg,cg = smSci.smSci3d.calcRowColGroup(row,col,level)
	strl, strr, strc = ('%d' % level), ('%04d' % rg), ('%04d' % cg)
	fPath = os.path.join(path, strl, strr, strc, fName)
	folder=os.path.dirname(fPath)
	if not os.path.exists(folder): os.makedirs(folder)

	driverName = 'PNG'
	out_drv = gdal.GetDriverByName(driverName)
	mem_drv = gdal.GetDriverByName('MEM')
	mem_ds=mem_drv.Create('',TILESIZE256, TILESIZE256, 3, gdal.GDT_Byte)

	if mem_ds is None: return 
	mem_ds.GetRasterBand(1).WriteArray(atile)
	out_drv.CreateCopy(fPath, mem_ds, strict=0)
	mem_ds=None

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

    def listDir(slef, root):
	''' 得到指定目录下文件列表
	'''
	imgList=[]
	if os.path.isdir(root):
	    reTif = '[\d\D]*\.tif$' 
	    reTiff = '[\d\D]*\.tiff$' 
	    for fName in os.listdir(root):
		if re.match(reTif,fName,re.IGNORECASE) is not None \
		    or re.match(reTif,fName,re.IGNORECASE) is not None:
		    #imgList.append(os.path.join(root, fName))
		    imgList.append(fName)

	return imgList

# =============================================================================
# =============================================================================
# =============================================================================

def unitTest():
    filePath=r'E:\2013\2013-06\2013-06-17'
    outPath=r'E:\2013\2013-06\2013-06-17\out'
    imgtile = Image2Tiles(filePath) 
    imgList = imgtile.listDir(filePath)
    imgtile.toTiles(imgList, 10, outPath)
    '''
    icnt=gdal.GetDriverCount()
    for i in xrange(icnt):
	print gdal.GetDriver(i).ShortName, gdal.GetDriver(i).LongName
    '''
    #imgtile.process()
    pass

if __name__=='__main__':
    #argv = gdal.GeneralCmdLineprocessor( sys.argv )
    #if len(argv)==1: sys.exit(1) 
    unitTest()

