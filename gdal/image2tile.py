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

class Image2Tiles(object):
    ''' 将image文件切割为瓦片 '''

    def __init__(self, fileName):
	self.fileName=fileName
	pass
    
    def Process(self):
	if not os.path.isfile(self.fileName):
	    print 'file[%s] not exits.' % self.fileName
	    return False

	hDataset = gdal.Open(self.fileName, gdal.GA_ReadOnly)
	if hDataset is None:
	    print "unable to open '%s'." % self.fileName
	    return False
	
	#/* -------------------------------------------------------------------- */
	#/*      Report projection.                                              */
	#/* -------------------------------------------------------------------- */
	pszProjection = hDataset.GetProjectionRef()
	if pszProjection is not None:
	    hSRS = osr.SpatialReference()
	    if hSRS.ImportFromWkt(pszProjection ) == gdal.CE_None:
		pszPrettyWkt = hSRS.ExportToPrettyWkt(False)
		#print( "Coordinate System is:\n%s" % pszPrettyWkt )
	    else:
		pass
		#print( "Coordinate System is `%s'" % pszProjection )
	

	self.GetInfo(hDataset)
	pass

    def GetInfo(self, hDataset):
	''' 获取影像地理范围等信息 '''

	xSize, ySize = hDataset.RasterXSize, hDataset.RasterYSize
	adfGeoTransform = hDataset.GetGeoTransform(can_return_null = True)
	if adfGeoTransform is None:
	    return False

	print adfGeoTransform 
        dMinX = adfGeoTransform[0] + adfGeoTransform[1] * 0 + adfGeoTransform[2] * 0 
        dMaxY = adfGeoTransform[3] + adfGeoTransform[4] * 0 + adfGeoTransform[5] * 0 
        dMaxX = adfGeoTransform[0] + adfGeoTransform[1] * xSize + adfGeoTransform[2] * ySize
        dMinY = adfGeoTransform[3] + adfGeoTransform[4] * xSize + adfGeoTransform[5] * ySize
	print "(xSize, ySize)", xSize, ySize
	print "(dMinX, dMinY, dMaxX, dMaxY)", dMinX, dMinY, dMaxX, dMaxY
	
	'''
	print hDataset.RasterXSize, hDataset.RasterYSize, hDataset.RasterCount
	hBand = hDataset.GetRasterBand(1)
	print gdal.GetDataTypeName(hBand.DataType)
        print gdal.GetColorInterpretationName(hBand.GetRasterColorInterpretation()) 
	'''

	self.createBound(dMinX, dMinY, dMaxX, dMaxY)

	return True
    
    def createBound(self, dMinX, dMinY, dMaxX, dMaxY):
	''' 创建影像的外包矩形 '''
	shpPath = self.fileName
	shpPath = shpPath[:-4]+'.shp'

	driverName = 'ESRI Shaplefile'
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

# =============================================================================
# =============================================================================
# =============================================================================

def unitTest():
    filePath=r'E:\2013\2013-06\2013-06-14\srtm_47_01.tif'
    img = Image2Tiles(filePath) 
    img.Process()
    pass

if __name__=='__main__':
    argv = gdal.GeneralCmdLineProcessor( sys.argv )
    print argv
    #if len(argv)==1: sys.exit(1) 
    unitTest()

