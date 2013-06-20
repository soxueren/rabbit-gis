#!/usr/bin/env python
# --*-- coding:utf-8 --*--

import sys
import os

#import math
#import re 

# ģ���ļ�
CACHE_40_JPG = 'cache_40_jpg.sci' # ��ͼ���������ļ�_4.0_jpg 
CACHE_40_PNG = 'cache_40_png.sci' # ��ͼ���������ļ�_4.0_png
CACHE_50_JPG_LOCAL = 'cache_50_jpg_local.sci' # ��ͼ���������ļ�_5.0_jpg_png_�����ʷ� 
CACHE_50_JPG_PNG_GLOBAL = 'cache_50_jpg_png_global.sci' # ��ͼ���������ļ�_5.0_jpg_png_ȫ���ʷ� 
CACHE_50_JPG_PNG_LOCAL = 'cache_50_jpg_png_local.sci' # ��ͼ���������ļ�_5.0_jpg_�����ʷ�

# �汾��
SCIVERSION401 = 401
SCIVERSION402 = 402
SCIVERSION501 = 502
SCIVERSION502 = 502
SCIVERSION503 = 503

# �汾��ģ��ӳ��
DICTVERSION = {SCIVERSION401:CACHE_40_JPG,
SCIVERSION402:CACHE_40_PNG,
SCIVERSION501:CACHE_50_JPG_LOCAL,
SCIVERSION502:CACHE_50_JPG_PNG_GLOBAL,
SCIVERSION503:CACHE_50_JPG_PNG_LOCAL}

KEY_CACHE_NAME='[CacheName]'
KEY_MAP_NAME='[MapName]'

class smSci(object):
    ''' SuperMap���������ļ����� ''' 

    def __init__(self, sciPath=None):
	self.sciPath=sciPath # Ŀ��SCI�ļ�
	self.mapName='Untitled'
	self.sciTmpFile=CACHE_50_JPG_LOCAL
	self.indexBnd=list()
	self.mapBnd=list()
    
    # ��ѡ����,sci���·��
    def setPath(self, sciPath):
	self.sciPath=sciPath

    # ��ѡ����,��ͼ����
    def setMapName(self, mapName):
	self.mapName=mapName

    def setMapBound(self, bnd):
	self.mapBnd=[]
	self.extend(bnd)

    def setIndexBound(self, bnd):
	self.indexBnd=[]
	self.indexBnd(bnd)

    # ��ѡ����,sic�ļ��İ汾
    def setSciVer(self, ver):
	if DICTVERSION.has_key(ver):
	    self.sciTmpFile=DICTVERSION[ver]

    def write(self):
	srcSciPath = os.path.join(os.getcwd(), 'smSci', self.sciTmpFile)
	f=open(srcSciPath)
	lines=f.readlines()
	f.close()
	outlines=[]
	for line in lines:
	    line=line.strip()
	    if line=='':continue
	    if KEY_CACHE_NAME in line:
		line = line.replace(KEY_CACHE_NAME, self.mapName)
	    if KEY_MAP_NAME in line:
		line = line.replace(KEY_MAP_NAME, self.mapName)
	    outlines.append(line)
	
	desSciPath=os.path.join(self.sciPath, self.mapName+'.sci')
	f=open(desSciPath, 'w+')
	for line in outlines:
	    f.write(line+'\n')
	f.close()
	pass
	

# =============================================================================
# =============================================================================
# =============================================================================

def unitTest():
    sciPath=r'E:\Work\2013\2013-06\2013-06-17'
    sci = smSci(sciPath)
    sci.write()
    pass

if __name__=='__main__':
    unitTest()

