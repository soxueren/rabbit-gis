#!/usr/bin/env python
# --*-- coding:gbk --*--

import os
import sys
import urllib2
import random
import multiprocessing as mp
import logging
import time
import argparse

import numpy
from array import array

try:
    from osgeo import gdal
    #from osgeo import ogr
    #from osgeo import osr
except:
    import gdal
    #import ogr
    print("""You are using "old gen" bindings. gdal2tiles needs "new gen" bindings.""")
    sys.exit(1)

import tileserver
from tileserver.common import watermark as wmk
from tileserver.common import sci as smsci
from tileserver.common import srsweb
from tileserver.common import comm as cm
from tileserver.common import license as lic

import g2s

logger = logging.getLogger("g2s3d")
GOOGLE_TILE_LOCAL_NAME = "level%d_row%d_col%d.png"
#---------------------------------------------------------------------------

def tile_in_google(row, col, level):
    """ 指定瓦片真的是谷歌的吗 """
    size = 2**level
    if row<size and col<size:
        return True
    return False

#---------------------------------------------------------------------------
def load_local_tiles(tiles, outPath):
    """ 将本地瓦片加载到内存 """
    mem_tiles = {}
    gm = srsweb.GlobalMercator(256)
    for level, row, col in tiles:
        if not tile_in_google(row, col, level):
            return {}
        outfile = GOOGLE_TILE_LOCAL_NAME % (level, row, col) 
        fp = os.path.join(outPath, outfile)
        if not os.path.isfile(fp):
            return {}
        tmp_ds = gdal.Open(fp, gdal.GA_ReadOnly)
        if not tmp_ds:
            return {}
        r,g,b = [tmp_ds.GetRasterBand(i).ReadAsArray(0, 0, 256, 256, 256, 256) for i in (1,2,3)]
        #tx, ty = gm.TMSTile(col, row, level)
        mem_tiles[(row,col)] = (r,g,b) 
        del tmp_ds
    return mem_tiles

#---------------------------------------------------------------------------

#---------------------------------------------------------------------------
def download_tile(tiles, outPath, strurl):
    """ 下载瓦片到本地 """
    def _download_one_tile(url):
        """ 一次下载 """
        try:
            f = urllib2.urlopen(url)
        except urllib2.URLError, e:
            #print url, e.reason
            return None
        return f.read()

    for level, row, col in tiles:
        if not tile_in_google(row, col, level):
            continue

        url = strurl % (random.randint(0,3), col, row, level)
        outfile = GOOGLE_TILE_LOCAL_NAME % (level, row, col) 
        fp = os.path.join(outPath, outfile)
        if os.path.isfile(fp):
            continue

        data = _download_one_tile(url)
        if not data or len(data)<1024:
            continue

        '''
        if data is None:
            data = _download_one_tile(url) # 第2次下载
        if data is None:
            data = _download_one_tile(url) # 第3次下载
        if data is None:
            #print url
            continue # 放弃吧

        if len(data)<1024:
            data = _download_one_tile(url) # 第2次下载
        if len(data)<1024:
            data = _download_one_tile(url) # 第3次下载

        if len(data)<1024:
            #print url, len(data)
            continue # 放弃吧
        '''

        if not os.path.exists(os.path.dirname(fp)):
            os.makedirs(os.path.dirname(fp))
        with open(fp, "wb") as myfile:
            myfile.write(data)
            myfile.close()

#---------------------------------------------------------------------------
def save_as_sm_tile(row, col, level, outPath, ext, tiles, haswatermark):
    """ 生成SuperMap瓦片 """
    mem_drv = gdal.GetDriverByName("MEM")
    mem_ds = mem_drv.Create("", 256, 256, 3)# 输出固定为24位png或jpg
    
    rdata = numpy.zeros((256, 256), numpy.uint8)
    gdata = numpy.zeros((256, 256), numpy.uint8)
    bdata = numpy.zeros((256, 256), numpy.uint8)
    
    gm = srsweb.GlobalMercator(256)
    tmp = os.path.join(outPath, "xxx")
    l,t,r,b = smsci.smSci3d.calcBndByRowCol(row, col, level)

    levelRes = (180.0/256) / (1<<level) # i层分辨率
    halfRes = levelRes*0.5

    for line in range(0, 256):
        for pix in range(0, 256):
            lat, lon = t-line*levelRes-halfRes, l+pix*levelRes+halfRes
            x,y = gm.LatLonToMeters(lat, lon)
            px, py = gm.MetersToPixels(x, y, level+1)
            tx, ty = gm.PixelsToTile(px, py)
            gx, gy = gm.GoogleTile(tx, ty, level+1)
            if (gy, gx) in tiles:
                trow, tcol = gm.PixelsPosInTile(px,py)
                r,g,b = tiles[(gy, gx)]
                rdata[line][pix] = r[trow][tcol]
                gdata[line][pix] = g[trow][tcol]
                bdata[line][pix] = b[trow][tcol]
            else: # 没完全加载谷歌瓦片,舍弃生成瓦片
                return None

    mem_ds.GetRasterBand(1).WriteArray(rdata)
    mem_ds.GetRasterBand(2).WriteArray(gdata)
    mem_ds.GetRasterBand(3).WriteArray(bdata)

    fp = smsci.smSci3d.calcTileName(level, row, col, outPath)+ext
    if not os.path.exists(os.path.dirname(fp)):
        os.makedirs(os.path.dirname(fp))

    tx, ty = mem_ds.RasterXSize, mem_ds.RasterYSize
    tile_data = mem_ds.GetRasterBand(1).ReadAsArray(0,0,tx,ty,tx,ty) 
    if haswatermark:
        wmk.printWatermark(tile_data, wmk.buf_xsize, wmk.buf_ysize, wmk.buf_tile_server)
        mem_ds.GetRasterBand(1).WriteArray(tile_data)
    
    driv_name = 'PNG' if ext.lower()=='.png' else 'JPEG'
    out_drv = gdal.GetDriverByName(driv_name)
    out_drv.CreateCopy(fp, mem_ds, strict=0)
    del mem_ds, rdata, gdata, bdata 

#---------------------------------------------------------------------------
def multi_process_fun(bboxs, outPath, tile_ext, tmpPath, pindex, haswatermark,url, over_write):
    """ 多进程处理切图,bboxs为超图三维瓦片集合  """

    def _calc_g_tile(sml, smr, smc):
        """ 超图瓦片->谷歌瓦片 """
        tiles = []
        l,t,r,b = smsci.smSci3d.calcBndByRowCol(smr,smc,sml)
        gm = srsweb.GlobalMercator(256)
        rs,re,cs,ce = gm.calcRowColByLatLon(l,t,r,b, sml+1)
        for row in range(rs, re+1):
            for col in range(cs, ce+1):
                tiles.append((sml+1, row, col))
        return tiles

    ext = '.png' if tile_ext.lower()=='png' else '.jpg'
    gm = srsweb.GlobalMercator(256)

    for level, row, col in bboxs:
        fp = smsci.smSci3d.calcTileName(level, row, col, outPath)+ext
        if os.path.isfile(fp):
            if over_write:
                os.remove(fp)
            else:
                continue
        g_tiles = _calc_g_tile(level,row,col)
        download_tile(g_tiles, tmpPath, url)
        tiles = load_local_tiles(g_tiles, tmpPath)
        if not tiles:
            continue
        if level<4: #  八级以下不加水印
            save_as_sm_tile(row, col, level, outPath,ext,tiles, False)
        else:
            save_as_sm_tile(row, col, level, outPath,ext,tiles, haswatermark)
        del tiles

# =============================================================================
class Download(g2s.Download):
    """ 图像信息,超图zoom(0)==谷歌zoom(1) """

    def __init__(self, argv):
        super(Download, self).__init__(argv)

    def save_sci_file(self,l,t,r,b):
        """ 生成SuperMap缓存配置文件 """
        outPath = os.path.join(self.out, self.name)
        if not os.path.exists(outPath): 
            os.makedirs(outPath)
        
        levelRes = (180.0/256) / (1<<self.levels[-1]) # i层分辨率
        halfRes = levelRes*0.5
        gm = srsweb.GlobalMercator(256)

        x,y = gm.LatLonToMeters(t-halfRes, l+halfRes)
        lat, lon = gm.MetersToLatLon(x,y)
        t = min(t, lat)

        x,y = gm.LatLonToMeters(b+halfRes, r-halfRes)
        lat, lon = gm.MetersToLatLon(x,y)
        b = max(b, lat)

        mapbnd = l,t,r,b
        w,h = smsci.smSci3d.calcWidthHeight(l,t,r,b, self.levels[-1])

        _sci = smsci.smSci3d()
        _sci.setParams(self.name, mapbnd, mapbnd, "")
        _sci.setLevels(self.levels)
        _sci.setExtName(self.file_format)
        _sci.setWidthHeight(w,h)
        _sci.saveSciFile(outPath)

    def run(self):
        def _calc_sm_tiles(l,t,r,b,l_list):
            tiles = []
            for i in l_list:
                rs,re,cs,ce = smsci.smSci3d.calcRowCol(l,t,r,b,i)
                for row in range(rs, re+1):
                    for col in range(cs, ce+1):
                        tiles.append((i, row, col))
            return tiles

        def _split_by_process(tiles, mpcnt):
            """ 根据进程数目,瓦片张数划分合理的任务 """
            mplist = [[] for i in range(mpcnt)]
            for i in xrange(len(tiles)):
                mplist[i%mpcnt].append(tiles[i])
            return mplist
        
        l,t,r,b = self.l, self.t, self.r, self.b
        self.save_sci_file(l, t, r, b)
        sm_tiles = _calc_sm_tiles(l,t,r,b,self.levels)
        mplist = _split_by_process(sm_tiles, self.mpcnt)

        logger.info(38 * "-")
        logger.info("地理范围:左上右下(%f,%f,%f,%f)" % (l,t,r,b))
        logger.info("下载层级:(%s), 瓦片总数%d张." % (','.join(map(str,self.levels)), len(sm_tiles)))
    
        outPath = os.path.join(self.out, self.name)
        tmp_path = os.path.join(outPath, "xxx") 
        def _create_del_temp(dir_path, is_del=False):
            def __removedirs(root):
                for rt, dirs, files in os.walk(root):
                    for name in files:
                        os.remove(os.path.join(rt, name))
                os.removedirs(dir_path)
            if is_del:
                if os.path.exists(dir_path): 
                    __removedirs(dir_path)
            else:
                if not os.path.exists(dir_path): 
                    os.makedirs(dir_path)

        _create_del_temp(tmp_path,False)

        plist = []
        m = mp.Manager()
        for i in xrange(len(mplist)):
            bboxs = mplist[i]
            if not bboxs:
                continue
            p = mp.Process(target=multi_process_fun, 
                    args=(bboxs, outPath,self.file_format,tmp_path, i+1, self.haswatermark,self.url,self.over_write))
            plist.append( (p, len(bboxs)) )

        for p, cnt in plist:
            p.start()

        for p, cnt in plist:
            p.join()
            logger.info("子进程(id=%d), 瓦片张数(%d), 已完成." % (p.pid, cnt) )

        _create_del_temp(tmp_path, True)
        logger.info("End, All done.")
        logger.info(38 * "=")

# =============================================================================

def main():
    g2s.log_init()
    Download(sys.argv[1:]).run()

# =============================================================================
if __name__=="__main__":
    mp.freeze_support()
    main()

