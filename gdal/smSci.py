#!/usr/bin/env python
# --*-- coding:utf-8 --*--

import sys
import os
import math

#import math
#import re 

# ģ���ļ�
CACHE_40_JPG = 'cache_40_jpg.sci' # ��ͼ���������ļ�_4.0_jpg 
CACHE_40_PNG = 'cache_40_png.sci' # ��ͼ���������ļ�_4.0_png
CACHE_50_JPG_LOCAL = 'cache_50_jpg_local.sci' # ��ͼ���������ļ�_5.0_jpg_png_�����ʷ� 
CACHE_50_JPG_PNG_GLOBAL = 'cache_50_jpg_png_global.sci' # ��ͼ���������ļ�_5.0_jpg_png_ȫ���ʷ� 
CACHE_50_JPG_PNG_LOCAL = 'cache_50_jpg_png_local.sci' # ��ͼ���������ļ�_5.0_jpg_�����ʷ�
SCI3D = 'sci3d.sci3d' # ��άӰ�񻺴�

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
KEY_IDXBND_LEFT='[idxBndLeft]'
KEY_IDXBND_TOP='[idxBndTop]'
KEY_IDXBND_RIGHT='[idxBndRight]'
KEY_IDXBND_BOTTOM='[idxBndBottom]'

KEY_MAP_LEFT='[mapBndLeft]'
KEY_MAP_TOP='[mapBndTop]'
KEY_MAP_RIGHT='[mapBndRight]'
KEY_MAP_BOTTOM='[mapBndBottom]'

KEY_LEFT='[Left]'
KEY_TOP='[Top]'
KEY_RIGHT='[Right]'
KEY_BOTTOM='[Bottom]'

KEY_SCALE_VALUE='[scaleValue]'
KEY_SCALE_CAPTION='[scaleCaption]'

class smSci(object):
    ''' SuperMap���������ļ����� ''' 

    def __init__(self):
	self.mapName='Untitled'
	self.sciTmpFile=CACHE_50_JPG_LOCAL
	self.idxBnd=list()
	self.mapBnd=list()
	self.scaleVal=0.0
	self.scaleCap=''
	self.lines=[] # sic�ļ�����

    def setParams(self, mapName, mapBnd, idxBnd, sciVer):
	''' ��������SCI�ļ��Ĳ���
	 mapName ��ѡ,��ͼ���� 
	 mapBnd ��ѡ,��Ƭ��Χ
	 idxBnd ��ѡ,��Ƭ������Χ 
	 sciVer ��ѡ����,sci�ļ��İ汾
	'''
	self.mapName=mapName
	self.mapBnd=mapBnd
	self.idxBnd=idxBnd
	if DICTVERSION.has_key(sciVer):
	    self.sciTmpFile=DICTVERSION[sciVer]
    
    def isValid(self):
	if len(self.idxBnd)!=4: return False
	if len(self.mapBnd)!=4: return False
	return True
    
    def loadTemplate(self):
	srcSciPath = os.path.join(os.getcwd(), 'smSci', self.sciTmpFile)
	f=open(srcSciPath)
	lines=f.readlines()
	f.close()
	for line in lines:
	    line=line.strip()
	    if line=='':continue
	    self.lines.append(line)

	
    def write(self, sciPath):
	fileName=self.mapName+self.sciTmpFile[self.sciTmpFile.find('.'):]
	desSciPath=os.path.join(sciPath, fileName)
	f=open(desSciPath, 'w+')
	for line in self.lines:
	    f.write(line+'\n')
	f.close()

    def replaceCacheName(self):
	for i in xrange(len(self.lines)):
	    line=self.lines[i]
	    if KEY_CACHE_NAME in line:
		line = line.replace(KEY_CACHE_NAME, self.mapName)
		self.lines[i]=line
		break
	return True

    def relpaceBnd(self):
	for i in xrange(len(self.lines)):
	    line=self.lines[i]
	    if KEY_IDXBND_LEFT in line:
		line = line.replace(KEY_IDXBND_LEFT, self.idxBnd[0])
	    if KEY_IDXBND_TOP in line:
		line = line.replace(KEY_IDXBND_TOP, self.idxBnd[1])
	    if KEY_IDXBND_RIGHT in line:
		line = line.replace(KEY_IDXBND_RIGHT, self.idxBnd[2])
	    if KEY_IDXBND_BOTTOM in line:
		line = line.replace(KEY_IDXBND_BOTTOM, self.idxBnd[3])

	    if KEY_MAP_LEFT in line:
		line = line.replace(KEY_MAP_LEFT, self.mapBnd[0])
	    if KEY_MAP_TOP in line:
		line = line.replace(KEY_MAP_TOP, self.mapBnd[1])
	    if KEY_MAP_RIGHT in line:
		line = line.replace(KEY_MAP_RIGHT, self.mapBnd[2])
	    if KEY_MAP_BOTTOM in line:
		line = line.replace(KEY_MAP_BOTTOM, self.mapBnd[3])
	    self.lines[i] = line

    def replace(self):
	self.replaceCacheName()
	self.replaceBnd()
	for i in xrange(len(self.lines)):
	    if KEY_SCALE_VALUE in line:
		line = line.replace(KEY_SCALE_VALUE, self.scaleVal)
	    if KEY_SCALE_CAPTION in line:
		line = line.replace(KEY_SCALE_CAPTION, self.scaleCap)

    def saveSciFile(self, sciPath):
	''' ����SM��ʽ��ƬSCI�ļ� '''
	if not self.isValid():return False

	self.loadTemplate()
	self.replace()
	self.write(sciPath)
	return True

'''=============================================================='''

class smSci3d(smSci):
    ''' ��άӰ�񻺴������ļ� '''

    def __init__(self):
	super(smSci3d, self).__init__()
	self.sciTmpFile=SCI3D
	self.width=1.0
	self.height=1.0
	self.startl=1
	self.endl=2

    def setLevels(self, s, e):
	self.startl, self.endl=s,e

    def setWidthHeight(self, w, h):
	''' Ӱ�񻺴���߷ֱ���ͼ��Ŀ��(������һ��)���ܳߴ�,��λΪ���� '''
	self.width, self.height=w,h


    def replaceBnd(self):
	for i in xrange(len(self.lines)):
	    line=self.lines[i]
	    if KEY_LEFT in line:
		line = line.replace(KEY_LEFT, ('%f' % self.mapBnd[0]))
	    if KEY_TOP in line:
		line = line.replace(KEY_TOP, ('%f' % self.mapBnd[1]))
	    if KEY_RIGHT in line:
		line = line.replace(KEY_RIGHT, ('%f' % self.mapBnd[2]))
	    if KEY_BOTTOM in line:
		line = line.replace(KEY_BOTTOM, ('%f' % self.mapBnd[3]))

	    self.lines[i]=line
	return True
	
    def replaceWidthHeight(self):
	for i in xrange(len(self.lines)):
	    line=self.lines[i]
	    if '[Width]' in line:
		line = line.replace('[Width]', ('%f' % self.width))
	    if '[Height]' in line:
		line = line.replace('[Height]', ('%f' % self.height))

	    self.lines[i]=line
	return True
    
    def replaceLevels(self):
	ils, ile=self.startl, self.endl
	for i in xrange(len(self.lines)):
	    line=self.lines[i]
	    if '[Level]' in line:
		newline = line.replace('[Level]', ('%d' % ils))
		self.lines[i]=newline
		for j in xrange(1, ile-ils+1):
		    newline = line.replace('[Level]', ('%d' % (ils+j)))
		    self.lines.insert(i+j,newline)
		return True
    
    def replace(self):
	self.replaceCacheName()
	self.replaceWidthHeight()
	self.replaceBnd()
	self.replaceLevels()

    @staticmethod
    def calcEndLevel(res):
	''' ������ֹ�㼶  '''
	level0Res = 180.0/256 # 0��ֱ��� 
	tmp=None
	endLevel=0
	for i in xrange(21):
	    leveliRes = level0Res/(1<<i) # i��ֱ���
	    dis = abs(leveliRes-res)
	    if tmp is None:
		tmp=dis
	    elif dis<tmp:
		tmp=dis
		endLevel=i
	return endLevel

    @staticmethod
    def calcStartLevel(l, t, r, b, res, endl):
	w,h = r-l, t-b
	cover = w if w>h else h
	coverPix=cover/res
	startl=endl
	while(coverPix>256):
	    coverPix *= 0.5
	    startl=startl-1
	return startl
    
    @staticmethod
    def calcWidthHeight(l,t,r,b,level):
	''' Ӱ�񻺴���߷ֱ���ͼ��Ŀ��(������һ��)���ܳߴ�,��λΪ���� '''
	level0Res = 180.0/256 # 0��ֱ��� 
	levelRes = level0Res/(1<<level) # i��ֱ���
	w,h = (r-l)/levelRes, (t-b)/levelRes # ���ؿ�ȸ߶�
	return w,h
    
    @staticmethod
    def calcRowCol(l,t,r,b,level):
	''' ����ȫ���ʷ����к� '''
	level0Res = 180.0/256 # 0��ֱ��� 
	levelRes = level0Res/(1<<level) # i��ֱ���
	startCol = math.floor((l-(-180.0)) / levelRes / 256)
	endCol = math.ceil((r-(-180.0)) / levelRes / 256)
	startRow = math.floor((90.0-t) / levelRes / 256)
	endRow = math.ceil((90.0-b) / levelRes / 256)
	return int(startRow), int(endRow), int(startCol), int(endCol)
    
    @staticmethod
    def calcRowColGroup(r,c,level):
	''' ����ȫ���ʷ����к�,��������ļ������к� '''
	irate=int(math.floor((2.0*level+1)/3))
	irowStep=(1<<level) / (1<<irate)
	return r/irowStep, c/irowStep/2
    
    @staticmethod
    def calcBndByRowCol(r,c,level):
	level0Res = 180.0/256 # 0��ֱ��� 
	levelRes = level0Res/(1<<level) # i��ֱ���
	l,t=c*levelRes, r*levelRes
	r,b=l+levelRes, t-levelRes
	return l,t,r,b
	
	

# =============================================================================
# =============================================================================
# =============================================================================

def unitTestLevel():
    l,t,r,b =49.956471,54.999583,55.044362,49.999584 
    res=0.0008
    endl=smSci3d.calcEndLevel(res)
    startl=smSci3d.calcStartLevel(l,t,r,b,res,endl)
    w, h=smSci3d.calcWidthHeight(l,t,r,b,6)
    print startl, endl, w,h
    rs,re, cs, ce = smSci3d.calcRowCol(l,t,r,b,endl)
    print smSci3d.calcRowColGroup(rs,cs, endl)

def unitTest():
    sciPath=r'E:\2013\2013-06\2013-06-17'
    mapName='srtm_47_02@img_3'
    l,t,r,b =50.0004166667, 54.9995833333, 55.0004164667,49.9995835333 
    mapBnd=l,t,r,b

    res=0.000833333300000001
    endl=smSci3d.calcEndLevel(res)
    startl=smSci3d.calcStartLevel(l,t,r,b,res,endl)
    w,h=smSci3d.calcWidthHeight(l,t,r,b,endl)

    sci = smSci3d()
    sci.setParams(mapName, mapBnd, mapBnd, '')
    sci.setLevels(startl, endl)
    sci.setWidthHeight(w,h)
    sci.saveSciFile(sciPath)
    pass

if __name__=='__main__':
    #unitTest()
    unitTestLevel()

