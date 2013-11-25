#!/usr/bin/env python
# --*-- coding:gbk --*--

import sys
import os
import math
import scitemplate
import srsweb

#import math
#import re 

# 模板文件
SCI3D = 'sci3d.sci3d' # 三维影像缓存

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
KEY_VALUE = '[value]'

VER40 = 1
VER31 = 2 # 使用新的缓存图片生成方案的 iServer 2.0 缓存
VER30 = 3 # iServer 2.0 缓存
VER21 = 4 
VER20 = 5

MAP_HASHCODE = "8B185AB8FIX"


# =============================================================================

class smSci(object):
    ''' SuperMap缓存配置文件生成 ''' 

    def __init__(self):
        self.mapName = 'Untitled'
        self.sciTmpFile = "sci.sci"
        self.idxBnd = list()
        self.mapBnd = list()
        self.lines = scitemplate.sci40 # sic文件内容
        self.sci_scale = scitemplate.sci40_scale
        self.scales = [] # 比例尺数组
        self.proj_lines = []
        self.sciver = VER31
	self.file_format = 'JPG'

    def setParams(self, mapName, mapBnd, idxBnd, sciVer):
        ''' 设置生成SCI文件的参数
         mapName 必选,地图名称 
         mapBnd 必选,切片范围
         idxBnd 必选,切片索引范围 
         sciVer 可选方法,sci文件的版本
        '''
        self.mapName=mapName
        self.mapBnd=mapBnd
        self.idxBnd=idxBnd
        if sciVer == VER40:
            self.lines = scitemplate.sci40 
            self.sci_scale = scitemplate.sci40_scale
            self.sciver = VER40
        elif sciVer == VER31:
            self.lines = scitemplate.sci31 
            self.sci_scale = scitemplate.sci31_scale
            self.sciver = VER31
        elif sciVer == VER30:
            self.lines = scitemplate.sci30 
            self.sci_scale = scitemplate.sci31_scale
            self.sciver = VER30
        elif sciVer == VER21:
            self.lines = scitemplate.sci31 
            self.sci_scale = scitemplate.sci31_scale
            self.sciver = VER21

    def setFileFormat(self, ext):
	ext = ext[-3:].lower()
	if ext=='jpg':
	    self.file_format='JPG'
	elif ext=='png':
	    self.file_format='PNG'

    def setWidthHeight(self, w, h):
        ''' 影像缓存最高分辨率图像的宽高(最清晰一层)的总尺寸,单位为像素 '''
        self.width, self.height=w,h

    def setScales(self, scales):
        self.scales.extend(scales)

    def setProj(self, projs):
        self.proj_lines.extend(projs)
    
    def isValid(self):
        if len(self.idxBnd)!=4: return False
        if len(self.mapBnd)!=4: return False
        return True
    
    def loadTemplate(self):

        try:
            dirName = os.path.dirname(os.path.abspath(__file__))
        except:
            dirName = os.path.dirname(os.path.abspath(sys.argv[0]))
        srcSciPath=os.path.join(dirName, 'smSci', self.sciTmpFile)
        # 打包后目录发生变化,需要去掉library.zip目录
        srcSciPath=srcSciPath.replace('library.zip','')
        srcSciPath=os.path.abspath(srcSciPath)
        if not os.path.isfile(srcSciPath):return 
        '''
        f=open('d:\\txt.txt','w')
        f.write(srcSciPath)
        f.close()
        '''
        f=open(srcSciPath)
        lines=f.readlines()
        f.close()
        for line in lines:
            line=line.strip()
            if line=='':continue
            self.lines.append(line)

        
    def write(self, sciPath):
        if self.sciver in (VER31,VER30):
            fileName = self.mapName + self.sciTmpFile[self.sciTmpFile.find('.'):]
            folder = "%s_256x256" % self.mapName
	elif self.sciver in (VER21,VER20,VER40):
            fileName = self.mapName + self.sciTmpFile[self.sciTmpFile.find('.'):]
	    folder = "%s_100x100" % self.mapName
        desSciPath = os.path.join(sciPath, folder, fileName)
        outPath = os.path.dirname(desSciPath)
        if not os.path.exists(outPath): 
	    os.makedirs(outPath)
        
        f=open(desSciPath, 'w+')
        for line in self.lines:
            f.write(line+'\n')
        f.close()

    def replaceText(self, lines, tag, k, v):
        for i in xrange(len(lines)):
            line = lines[i]
            if tag in line:
                line = line.replace(k, v)
                lines[i]=line
                break
        return True

    def replaceInt(self, lines, k, v):
        for i in xrange(len(lines)):
            line = lines[i]
            if k in line:
                line = line.replace(KEY_VALUE, ('%d' % v))
                lines[i]=line
                break
        return True

    def replaceDouble(self, lines, k, v, lens=9):
        for i in xrange(len(lines)):
            line = lines[i]
            if k in line:
                fmt = '%%.%df' % lens
                line = line.replace(KEY_VALUE, (fmt % v))
                lines[i]=line
                break
        return True

    def replaceCacheName(self):
        self.replaceText(self.lines, "CacheName",KEY_VALUE, self.mapName)
        self.replaceText(self.lines, "<sml:MapName>",KEY_VALUE, self.mapName)
        self.replaceText(self.lines, "<sml:ImageFormat>",KEY_VALUE, self.file_format)
        return True

    def replaceWidthHeight(self):
        self.replaceInt(self.lines, "ImageWidth", self.width)
        self.replaceInt(self.lines, "ImageHeight", self.height)
        return True

    def replaceBnd(self):
        self.replaceDouble(self.lines, 'MapLeft', self.idxBnd[0])
        self.replaceDouble(self.lines, 'MapTop', self.idxBnd[1])
        self.replaceDouble(self.lines, 'MapRight', self.idxBnd[2])
        self.replaceDouble(self.lines, 'MapBottom', self.idxBnd[3])

        self.replaceDouble(self.lines, 'ImageLeft', self.mapBnd[0])
        self.replaceDouble(self.lines, 'ImageTop', self.mapBnd[1])
        self.replaceDouble(self.lines, 'ImageRight', self.mapBnd[2])
        self.replaceDouble(self.lines, 'ImageBottom', self.mapBnd[3])

    def replaceHash(self, lines):
        for i in xrange(len(lines)):
            line = lines[i]
            if '<sml:HashCode' in line:
                line = line.replace('<sml:HashCode/', '<sml:HashCode>'+MAP_HASHCODE+'</sml:HashCode')
                lines[i]=line
                break
        return True

    def replaceScales(self):
        if not self.scales:
            return 

        self.scales.sort()
        self.scales.reverse()
        self.replaceDouble(self.lines, 'BaseScale', max(self.scales), 20)

        pos = 0
        for i in range(len(self.lines)):
            if '<MapCache>' in self.lines[i]:
                pos = i+1
                break

        for scale in self.scales:
            tmpscale = self.sci_scale[:]
            self.replaceDouble(tmpscale, '<sml:Scale>', scale, 20)
	    scale_int = int(1/scale)
	    if self.sciver==VER21 or self.sciver==VER20:
		scale_int = int(0.5+1/scale)
		self.replaceHash(tmpscale)
            self.replaceInt(tmpscale, '<sml:ScaleFolder>', scale_int)
            for i in range(len(tmpscale)-1, -1, -1):
                self.lines.insert(pos, tmpscale[i])

    def replaceProj(self):
        pos = 0
        for i in range(len(self.lines)):
            if '</sml:MapBounds>' in self.lines[i]:
                pos = i+1
                break

        for i in range(len(self.proj_lines)-1, -1, -1):
            self.lines.insert(pos, self.proj_lines[i])

    def replaceMapCacheStrategy(self):
	if self.sciver in (VER21, VER20):
	    self.replaceText(self.lines, '<sml:MapCacheStrategy', '2', '0')
	elif self.sciver == VER40:
	    self.replaceText(self.lines, '<sml:MapCacheStrategy', '2', '1')


    def replace(self):
        self.replaceCacheName()
        self.replaceBnd()
        self.replaceProj()
        self.replaceWidthHeight()
        self.replaceScales()
	self.replaceMapCacheStrategy()

    def saveSciFile(self, sciPath):
        ''' 生成SM格式切片SCI文件 '''
        if not self.isValid():return False

        #self.loadTemplate()
        self.replace()
        self.write(sciPath)
        return True

    @staticmethod
    def calcTileName(root, mapname, scale, row, col, ext, ver):
	if len(ext)>3:
	    ext = ext[-3:].lower()

        if ver == VER31 or ver==VER30:
	    """
	    iServer20 地图缓存文件路径规则：
	    地图名_图片宽度x图片高度（10进制表示，以像素为单位）作为一级目录；
	    比例尺或者比例尺倒数作为二级目录；
	    图片在 IndexBounds 中的列索引作为三级目录；
	    图片名称为图片在 IndexBounds 中的行索引.后缀；
	    """
            filename = "%d.%s" % (row, ext) 
            mapname = "%s_256x256" % mapname
            strscale = "%d" % int(1/scale)
            strcol = "%d"% col
            filename = os.path.join(root, mapname, strscale, strcol, filename)
            return filename
	elif ver == VER21 or ver==VER20:
	    """
	    地图名 + 图片尺寸作为一级子目录；
	    比例尺或者比例尺倒数作为二级子目录；
	    中心点所在区块索引作为三级子目录；
	    中心点近似值 + 地图散列值 + 后缀作为文件名
	    """
	    glbmkt = srsweb.GlobalMercator()
	    col_idx, row_idx = glbmkt.calcSmCellIndex(col, row, scale)
            mapname = "%s_100x100" % mapname
            strscale = "%d" % int(0.5+1/scale)
	    maphashcode = MAP_HASHCODE 
	    stridx = "%dx%d" % (col_idx, row_idx)
            filename = "%dx%d_%s.%s" % (col, row, maphashcode, ext) 
            filename = os.path.join(root, mapname, strscale, stridx, filename)
	    return filename
	elif ver == VER40:
	    """
	    // 地图名_图片宽度x图片高度（16进制表示，以像素为单位）作为一级目录；
	    // 比例尺或者比例尺倒数作为二级目录；
	    // 中心点所在 region 在 IndexBounds 中的列索引作为三级目录；
	    // 中心点所在 region 在 IndexBounds 中的行索引作为四级目录；
	    // 图片在所在 region 中的行索引x列索引_地图散列值.后缀作为文件名。
	    // 例如：
	    // Path = China400_100x100/18750000/2/0/0001x0002_C782F04CFIX.png
	    // {MapName}_{HexPicWidth}x{HexPicHeight}/{Scale}/{RegionYIndex}/{RegionXIndex}/{PicYIndex}x{PicXIndex}_{HashCode}.{ExtName}
	    // Region 为矩形的多个图片的集合。Region 的索引以 IndexBounds 作为参考进行计算。
	    """
            mapname = "%s_100x100" % mapname
            strscale = "%d" % int(1/scale)
	    pass

# =============================================================================

class smSci3d(smSci):
    ''' 三维影像缓存配置文件 '''

    def __init__(self):
        super(smSci3d, self).__init__()
        self.sciTmpFile=SCI3D
        self.width=1.0
        self.height=1.0
        self.startl=1
        self.endl=2
        self.extName='png'
        self.lines=scitemplate.sci3d # sic文件内容

    def setLevels(self, s, e):
        self.startl, self.endl=s,e

    def setExtName(self, ext):
        self.extName=ext

    def setWidthHeight(self, w, h):
        ''' 影像缓存最高分辨率图像的宽高(最清晰一层)的总尺寸,单位为像素 '''
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

    def replaceExtName(self):
        for i in xrange(len(self.lines)):
            line=self.lines[i]
            if '[FileExtentName]' in line:
                newline = line.replace('[FileExtentName]', self.extName)
                self.lines[i]=newline
    
    def replace(self):
        self.replaceCacheName()
        self.replaceWidthHeight()
        self.replaceBnd()
        self.replaceLevels()
        self.replaceExtName()

    def write(self, sciPath):
        fileName = self.mapName+'.sci3d'
        desSciPath = os.path.join(sciPath, fileName)
        f = open(desSciPath, 'w+')
        for line in self.lines:
            f.write(line+'\n')
        f.close()

    @staticmethod
    def calcEndLevel(res):
        ''' 计算终止层级  '''
        level0Res = 180.0/256 # 0层分辨率 
        tmp=None
        endLevel=0
        for i in xrange(21):
            leveliRes = level0Res/(1<<i) # i层分辨率
            dis = abs(leveliRes-res)
            if tmp is None:
                tmp=dis
            elif dis<tmp:
                tmp=dis
                endLevel=i
        return endLevel if endLevel<21 else 20

    @staticmethod
    def calcStartLevel(l, t, r, b, res, endl):
        w,h = r-l, t-b
        cover = w if w>h else h
        coverPix=cover/res
        startl=endl
        while(coverPix>256):
            coverPix *= 0.5
            startl=startl-1
        return startl if startl>-1 else 0
    
    @staticmethod
    def calcWidthHeight(l,t,r,b,level):
        ''' 影像缓存最高分辨率图像的宽高(最清晰一层)的总尺寸,单位为像素 '''
        level0Res = 180.0/256 # 0层分辨率 
        levelRes = level0Res/(1<<level) # i层分辨率
        w,h = (r-l)/levelRes, (t-b)/levelRes # 像素宽度高度
        return w,h
    
    @staticmethod
    def calcRowCol(l,t,r,b,level):
        ''' 计算l,t,r,b范围所覆盖的,全球剖分行列号 '''
        level0Res = 180.0/256 # 0层分辨率 
        levelRes = level0Res/(1<<level) # i层分辨率
	halflevelRes = levelRes*0.5# 挺重要de... 
        '''
        startCol = math.floor((l-(-180.0)) / levelRes / 256)
        endCol = math.ceil((r-(-180.0)) / levelRes / 256)
        startRow = math.floor((90.0-t) / levelRes / 256)
        endRow = math.ceil((90.0-b) / levelRes / 256)
        '''
	l,t,r,b = l+halflevelRes,t-halflevelRes,r-halflevelRes,b+halflevelRes 
        startCol = math.floor((l-(-180.0)) / levelRes / 256)
        endCol = math.floor((r-(-180.0)) / levelRes / 256)
        startRow = math.floor((90.0-t) / levelRes / 256)
        endRow = math.floor((90.0-b) / levelRes / 256)
        return int(startRow), int(endRow), int(startCol), int(endCol)
    
    @staticmethod
    def calcRowColGroup(r,c,level):
        ''' 根据全球剖分行列号,计算分组文件夹行列号 '''
        irate=int(math.floor((2.0*level+1)/3))
        irowStep=(1<<level) / (1<<irate)
        return r/irowStep, c/irowStep/2
    
    @staticmethod
    def calcBndByRowCol(r,c,level):
        level0Res = 180.0/256 # 0层分辨率 
        levelRes = level0Res/(1<<level) # i层分辨率
        l,t=c*levelRes*256+(-180.0), 90-r*levelRes*256
        r,b=l+levelRes*256, t-levelRes*256
        return l,t,r,b

    @staticmethod
    def calcTileName(level, row, col, path):
        fName = '%04d_%04d_0000' % (row,col)
        rg,cg = smSci3d.calcRowColGroup(row,col,level)
        strl, strr, strc = ('%d' % level), ('%04d' % rg), ('%04d' % cg)
        fPath = os.path.join(path, strl, strr, strc, fName)
        return fPath

    @staticmethod
    def calcTileCount(l,t,r,b,level):
        ''' 计算指定层级,指定范围内的瓦片数目 '''
        rs,re,cs,ce=smSci3d.calcRowCol(l,t,r,b,level) 
        totalNums = (re-rs+1)*(ce-cs+1)
        return totalNums 
        
    @staticmethod
    def calcTotalTileCount(l,t,r,b, startl, endl):
        ''' 计算指定层级,指定范围内的瓦片数目 '''
        totalNums = 0
        for i in range(startl, endl+1):
            totalNums += smSci3d.calcTileCount(l,t,r,b,i) 
        return totalNums 
        
# =============================================================================
        
class smSct(smSci3d):
    ''' 三维地形缓存配置文件 '''
    def __init__(self):
        super(smSct, self).__init__()
        self.extName='bil'

    def replaceType(self):
        for i in xrange(len(self.lines)):
            line=self.lines[i]
            if 'Image' in line:
                newline = line.replace('Image', 'DEM')
                self.lines[i]=newline
            if 'sci3d' in line:
                newline = line.replace('sci3d', 'sct')
                self.lines[i]=newline
            
    def replaceExtName(self):
        for i in xrange(len(self.lines)):
            line=self.lines[i]
            if '[FileExtentName]' in line:
                newline = line.replace('[FileExtentName]', self.extName)
                self.lines[i]=newline
                self.lines.insert(i, "<sml:CompressType>NONE</sml:CompressType>")

    def replace(self):
        self.replaceCacheName()
        self.replaceWidthHeight()
        self.replaceBnd()
        self.replaceLevels()
        self.replaceExtName()
        self.replaceType()

    def write(self, sciPath):
        fileName=self.mapName+'.sct'
        desSciPath=os.path.join(sciPath, fileName)
        f=open(desSciPath, 'w+')
        for line in self.lines:
            f.write(line+'\n')
        f.close()


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
    sci.setLevels(0, 0)
    sci.setWidthHeight(w,h)
    sci.saveSciFile(sciPath)
    pass

def unitTestSct():
    sciPath=r'E:\2013\2013-06\2013-06-17'
    mapName='srtm_47_02@img_3'
    l,t,r,b =50.0004166667, 54.9995833333, 55.0004164667,49.9995835333 
    mapBnd=l,t,r,b

    res=0.000833333300000001
    endl=smSci3d.calcEndLevel(res)
    startl=smSci3d.calcStartLevel(l,t,r,b,res,endl)
    w,h=smSci3d.calcWidthHeight(l,t,r,b,endl)

    sci = smSct()
    sci.setParams(mapName, mapBnd, mapBnd, '')
    sci.setLevels(startl, endl)
    sci.setWidthHeight(w,h)
    sci.saveSciFile(sciPath)

def unitTestRowCol():
    l,t,r,b = 117.718910,34.239982,118.365360,33.554232
    level = 9 
    rs,re, cs, ce = smSci3d.calcRowCol(l,t,r,b,level)
    for row in range(rs, re+1):
        for col in range(cs, ce+1):
            print row, col, smSci3d.calcBndByRowCol(row,col,level)

def unitTestSci():
    sciPath = r'E:\2013\2013-07\2013-07-24'
    mapName = 'test'
    l,t,r,b = 50.0004166667, 54.9995833333, 55.0004164667,49.9995835333 

    mkt = srsweb.GlobalMercator() 
    l,t = mkt.LatLonToMeters(t, l)
    r,b = mkt.LatLonToMeters(b, r)
    mapBnd = l,t,r,b

    l,t = mkt.LatLonToMeters(90.0, -180.0)
    r,b = mkt.LatLonToMeters(-90, 180.0)
    idxBnd = l,t,r,b

    scales = [0.1, 0.01]
    w,h = 256, 2560

    sci = smSci()
    sci.setParams(mapName, mapBnd, idxBnd, VER31)
    sci.setWidthHeight(w,h)
    sci.setScales(scales)
    sci.setProj(scitemplate.webmkt_prj)
    sci.saveSciFile(sciPath)

if __name__=='__main__':
    unitTest()
    #unitTestLevel()
    #unitTestSct()
    #unitTestSci()
    #unitTestRowCol()
