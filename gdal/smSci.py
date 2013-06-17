#!/usr/bin/env python
# --*-- coding:utf-8 --*--

import sys
import os
#import math
#import re 

CACHE_40_JPG = 'cache_40_jpg.sci' # 地图缓存配置文件_4.0_jpg 
CACHE_40_PNG = 'cache_40_png.sci' # 地图缓存配置文件_4.0_png
CACHE_50_JPG_LOCAL = 'cache_50_jpg_local.sci' # 地图缓存配置文件_5.0_jpg_png_本地剖分 
CACHE_50_JPG_PNG_GLOBAL = 'cache_50_jpg_png_global.sci' # 地图缓存配置文件_5.0_jpg_png_全球剖分 
CACHE_50_JPG_PNG_LOCAL = 'cache_50_jpg_png_local.sci' # 地图缓存配置文件_5.0_jpg_本地剖分

SCIVERSION401 = 401
SCIVERSION402 = 402
SCIVERSION501 = 502
SCIVERSION502 = 502
SCIVERSION503 = 503

DICTVERSION = {SCIVERSION401:CACHE_40_JPG,
SCIVERSION402:CACHE_40_PNG,
SCIVERSION501:CACHE_50_JPG_LOCAL,
SCIVERSION502:CACHE_50_JPG_PNG_GLOBAL,
SCIVERSION503:CACHE_50_JPG_PNG_LOCAL}


class smSci(object):
    ''' SuperMap缓存配置文件生成 ''' 

    def __init__(self, filePath=None):
	self.filePath=filePath
	self.mapName='Untitled'
	self.sciTmpFile=CACHE_50_JPG_LOCAL
    
    def setPath(self, filePath):
	self.filePath=filePath

    def setMapName(self, mapName):
	self.mapName=mapName

    def setSciVer(self, ver):
	if DICTVERSION.has_key(ver):
	    self.sciVersion=DICTVERSION[ver]

    def write(self):
	pass
	

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

