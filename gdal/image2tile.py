#!/usr/bin/env python
# --*-- coding:utf-8 --*--

import sys

try:
    from osgeo import gdal
    from osgeo import osr
except:
    import gdal
    print('You are using "old gen" bindings. gdal2tiles needs "new gen" bindings.')
    sys.exit(1)

import os
import math

class Image2Tiles(object):
	''' 将image文件切割为瓦片 
	'''

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

		print hDataset.RasterXSize, hDataset.RasterYSize
		adfGeoTransform = hDataset.GetGeoTransform(can_return_null = True)
		print adfGeoTransform 
		pass


# =============================================================================
# =============================================================================
# =============================================================================

if __name__=='__main__':
	argv = gdal.GeneralCmdLineProcessor( sys.argv )
	print argv
	if len(argv)==1: sys.exit(1) 
	img = Image2Tiles(argv[1]) 
	img.Process()

