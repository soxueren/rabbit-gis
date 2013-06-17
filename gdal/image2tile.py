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

TILESIZE256 = 256

class Image2Tiles(object):
    ''' 将image文件切割为瓦片 '''

    def __init__(self, fileName):
	self.fileName=fileName
	pass
    
    def Process(self):
	imgList = self.listDir(self.fileName)
	dleft, dtop, dright, dbottom, dxres, dyres = self.calcBoundary(imgList)
	pass

    def calcBoundary(self, imgList):
	dls=[]
	dts=[]
	drs=[]
	dbs=[]
	dxs=[]
	dys=[]
	for fName in imgList:
	    fPath = os.path.join(self.fileName, fName)
	    oneImg = img.ImageFile(fPath)
	    #print oneImg.getWidth()
	    #print oneImg.getHeight()
	    dl, dt, dr, db = oneImg.getBound()
	    dx, dy = oneImg.getResolution()
	    dls.append(dl)
	    dts.append(dt)
	    drs.append(dr)
	    dbs.append(db)
	    dxs.append(dx)
	    dys.append(dy)
	    #print oneImg.getProjection()

	dleft, dtop, dright, dbottom =  min(dls), max(dts), max(drs), min(dbs)
	dxres, dyres = min(dxs), min(dys)
	return dleft, dtop, dright, dbottom, dxres, dyres

    def img2Grid(self, imgList):
	imgbound={}
	for fName in imgList:
	    fPath = os.path.join(self.fileName, fName)
	    oneImg = img.ImageFile(fPath)
	    dl, dt, dr, db = oneImg.getBound()
	    imgbound[fName]=(dl, dt, dr, db)
	
	dleft, dtop, dright, dbottom, dxres, dyres = self.calcBoundary(imgList)
	dTileXSize=TILESIZE256*dxres # 瓦片的地理范围
	dTileYSize=TILESIZE256*dyres
	imgTileBound={}
	for k,v in imgbound.iteritems():
	    dl, dt, dr, db=v
	    print k, v
	    iColStart = int(math.floor( abs(dl-dleft)/dTileXSize ))
	    iColEnd = int(math.ceil( abs(dr-dleft)/dTileXSize ))
	    iRowStart = int(math.floor( abs(dt-dtop)/dTileYSize ))
	    iRowEnd = int(math.ceil( abs(db-dtop)/dTileYSize ))
	    i,j=iRowStart,iColStart
	    imgTileBound[k]=[]
	    while(i<=iRowEnd):
		while(j<=iColEnd):
		    dTileLeft = dleft+j*dTileXSize
		    dTileTop = dtop-i*dTileYSize
		    dTileRight = dTileLeft+dTileXSize
		    dTileBottom = dTileTop-dTileYSize
		    imgTileBound[k].append((i, j, dTileLeft, dTileTop,
			    dTileRight, dTileBottom))
		    j = j+1
		i = i+1

	for k,v in imgTileBound.iteritems():
	    fPath = os.path.join(self.fileName, k)
	    aimg = img.ImageFile(fPath)
	    for i,j,l,t,r,b in v:
		atile = numpy.zeros((TILESIZE256, TILESIZE256),int)
		aimg.cut(l,t,r,b,dxres,TILESIZE256, atile)
		fName = '%d_%d.bmp' % (i,j)
		fName = os.path.join(self.fileName, fName)
		
		driverName = 'BMP'
		drv = gdal.GetDriverByName(driverName)
		print fName
		ds=drv.Create(fName,TILESIZE256,TILESIZE256, 3,
				gdal.GDT_Byte)

		if ds is None: continue
		ds.GetRasterBand(1).WriteArray(atile)
		ds=None
		
	    pass
    
    def caclLevelInfo(self, dLeft, dTop, dRight, dBottom, dResolution, iTileSize):
	''' 计算比例尺,行列号信息
	dLeft 缓存区域的地理范围
	dTop 缓存区域的地理范围
	dRight 缓存区域的地理范围
	dBottom 缓存区域的地理范围 
	dResolution 影像像素分辨率
	iTileSize 瓦片的像素宽高(宽==高)
	'''

	dTileSize = dResolution*iTileSize
	iRowCount = math.ceil( (dTop-dBottom)/dTileSize )
	iColCount = math.ceil( (dRight-dLeft)/dTileSize )

    def createBound(self, dMinX, dMinY, dMaxX, dMaxY):
	''' 创建影像的外包矩形 '''
	shpPath = self.fileName
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
    filePath=r'E:\2013\2013-06\2013-06-14'
    imgtile = Image2Tiles(filePath) 
    imgList = imgtile.listDir(filePath)
    imgtile.img2Grid(imgList)
    #imgtile.Process()
    pass

if __name__=='__main__':
    argv = gdal.GeneralCmdLineProcessor( sys.argv )
    print argv
    #if len(argv)==1: sys.exit(1) 
    unitTest()

