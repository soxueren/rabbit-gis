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
	self.pszProjection=None
	self.xSize=None
	self.ySize=None
	self.dMinX=None
	self.dMinY=None
	self.dMaxX=None
	self.dMaxY=None
	self.dResolutionX=None
	self.dResolutionY=None
	self.iBandNums=None
	self.hDataset=None
	self.process()

    def getWidthHeight(self):
	return self.xSize, self.ySize
    
    def getResolution(self):
	return (self.dResolutionX, self.dResolutionX)

    def getBound(self):
	return (self.dMinX, self.dMaxY, self.dMaxX, self.dMinY)
    
    def getProjection(self):
	return self.pszProjection

    def process(self):
	if not os.path.isfile(self.fileName):
	    print 'file[%s] not exits.' % self.fileName
	    return False

	self.hDataset = gdal.Open(self.fileName, gdal.GA_ReadOnly)
	if self.hDataset is None:
	    print "unable to open '%s'." % self.fileName
	    return False
	
	#/* -------------------------------------------------------------------- */
	#/*      Report projection.                                              */
	#/* -------------------------------------------------------------------- */
	self.pszProjection = self.hDataset.GetProjectionRef()

	self.xSize, self.ySize = self.hDataset.RasterXSize, self.hDataset.RasterYSize

	adfGeoTransform = self.hDataset.GetGeoTransform(can_return_null = True)
	if adfGeoTransform is None:
	    return False

        self.dMinX = adfGeoTransform[0] + adfGeoTransform[1] * 0 + adfGeoTransform[2] * 0 
        self.dMaxY = adfGeoTransform[3] + adfGeoTransform[4] * 0 + adfGeoTransform[5] * 0 
        self.dMaxX = adfGeoTransform[0] + adfGeoTransform[1] * self.xSize + adfGeoTransform[2] * self.ySize
        self.dMinY = adfGeoTransform[3] + adfGeoTransform[4] * self.xSize + adfGeoTransform[5] * self.ySize

	self.dResolutionX = (self.dMaxX-self.dMinX)/self.xSize
	self.dResolutionY = (self.dMaxY-self.dMinY)/self.ySize
	self.iBandNums = self.hDataset.RasterCount
	
	'''
	print hDataset.RasterXSize, hDataset.RasterYSize, hDataset.RasterCount
	hBand = hDataset.GetRasterBand(1)
	print gdal.GetDataTypeName(hBand.DataType)
        print gdal.GetColorInterpretationName(hBand.GetRasterColorInterpretation()) 
	'''

	hDataset=None
	return True

    def cut(self, l, t, r, b, ts, tile_data=None):
	if l>self.dMaxX or t<self.dMinY or r<self.dMinX or b>self.dMaxY:
	    return None

	((rowInFileStart, rowInFileEnd, colInFileStart,colInFileEnd), \
	    (rowInTileStart, rowInTileEnd, colInTileStart, colInTileEnd)) \
	    = self._posOneTile(l, t, r, b, ts)

	for iband in range(1, self.iBandNums+1):
	    band = self.hDataset.GetRasterBand(iband)
	    #for i in range(band.YSize-1, -1, -1):
	    print '#'*50
	    rowFileSize=rowInFileEnd-rowInFileStart
	    colFileSize=colInFileEnd-colInFileStart
	    rowTileSize=rowInTileEnd-rowInTileStart
	    colTileSize=colInTileEnd-colInTileStart
	    data = band.ReadAsArray(colInFileStart, rowInFileStart,
			    colFileSize, rowFileSize, colTileSize, rowTileSize)
	    print data.shape, data.dtype
	    print tile_data.shape, tile_data.dtype
	    tile_data[rowInTileStart:rowInTileEnd,colInTileStart:colInTileEnd]=data


    def _posOneTile(self,l, t, r, b, ts):
	''' 定位瓦片在影像中的像素范围 '''
	res = self.dResolutionY
	rowInFileStart, rowInTileStart=0,0
	if t<self.dMaxY:
	    rowInFileStart=int((self.dMaxY-t)/self.dResolutionY)
	else:
	    rowInTileStart=int((t-self.dMaxY)/res)
	
	rowInFileEnd=self.ySize
	rowInTileEnd=ts
	if b>self.dMinY:
	    rowInFileEnd=int((b-self.dMinY)/self.dResolutionY)
	else:
	    rowInTileEnd=int((self.dMinY-b)/res)

	colInFileStart=0
	colInTileStart=0
	if l>self.dMinX:
	    colInFileStart=int((l-self.dMinX)/self.dResolutionX)
	else:
	    colInTileStart=int((self.dMinX-l)/res)

	colInFileEnd=self.xSize
	colInTileEnd=ts
	if r<self.dMaxX:
	    colInFileEnd=int((self.dMaxX-r)/self.dResolutionX)
	else:
	    colInTileEnd=int((r-self.dMaxX)/res)

	return ((rowInFileStart, rowInFileEnd, colInFileStart,colInFileEnd), \
	    (rowInTileStart, rowInTileEnd, colInTileStart, colInTileEnd))

# =============================================================================
# =============================================================================
# =============================================================================

def unitTest():
    filePath=r'E:\2013\2013-06\2013-06-14'
    img = ImageFile(filePath) 
    print img.getWidth()
    pass

if __name__=='__main__':
    argv = gdal.GeneralCmdLineProcessor( sys.argv )
    print argv
    #if len(argv)==1: sys.exit(1) 
    unitTest()

