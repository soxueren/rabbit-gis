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
import math
import ConfigParser

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
from tileserver.common import srsweb
from tileserver.common import comm as cm
from tileserver.common import license as lic

import cfg
import gdal_merge

#---------------------------------------------------------------------------
logger = logging.getLogger("g2f")
LICENSE_APP_NAME = "g2f"

GOOGLE_TILE_LOCAL_NAME = "level%d_row%d_col%d.jpg"
TEMP_DIR = "tiles"

#---------------------------------------------------------------------------
def download_g_tiles(tile_list, out_dir, strurl, over_write, print_watermark):
    def _tile_in_google(row, col, level):
        """ 指定瓦片真的是谷歌的吗 """
        size = 2**level
        if row<size and col<size:
            return True
        return False
    
    def _download_one_tile(url):
        """ 下载 """
        try:
            f = urllib2.urlopen(url)
        except urllib2.URLError, e:
            #print url, e.reason
            return None
        return f.read()
    
    def _write_wld(_path, level, row, col):
        _path = _path[:-3]+'wld'
        tms = srsweb.GlobalMercator()
        col,row = tms.TMSTile(col, row, level)
        l,t,r,b = tms.TileBounds(col, row, level)
        res = tms.Resolution(level)
        l_t_c_x = l+0.5*res
        l_t_c_y = t-0.5*res
        str_wld = "%.10lf\n0.0\n0.0\n%.10lf\n%.10lf\n%.10lf\n" % (res, -res, l_t_c_x, l_t_c_y)
        with open(_path,'w') as f:
            f.write(str_wld)
            f.close()

    def _print_water_mark(_path):
        tmp_ds = gdal.Open(_path, gdal.GA_ReadOnly)
        ts  = tmp_ds.RasterXSize

        mem_drv = gdal.GetDriverByName('MEM')
        mem_ds = mem_drv.Create('', ts, ts, 3)# 输出固定为24位png或jpg
        for i in range(1, 4):
            tile_data = tmp_ds.GetRasterBand(i).ReadAsArray(0,0,ts,ts,ts,ts) 
            if i==1:
                wmk.printWatermark(tile_data, wmk.buf_xsize, wmk.buf_ysize, wmk.buf_tile_server)
            mem_ds.GetRasterBand(i).WriteArray(tile_data)

        out_drv = gdal.GetDriverByName("JPEG")
        del tmp_ds
        out_drv.CreateCopy(_path, mem_ds, strict=0)
        del out_drv, mem_drv

    for level, row, col in tile_list:
        if not _tile_in_google(row, col, level):
            continue

        url = strurl % (random.randint(0,3), col, row, level)
        outfile = GOOGLE_TILE_LOCAL_NAME % (level, row, col) 
        fp = os.path.join(out_dir, outfile)

        if os.path.isfile(fp) and os.path.isfile(fp[:-3]+'wld'):
            if over_write:
                os.remove(fp)
            else:
                continue

        data = _download_one_tile(url)
        if not data or len(data)<1024:
            continue

        if not os.path.exists(os.path.dirname(fp)):
            os.makedirs(os.path.dirname(fp))

        with open(fp, "wb") as myfile:
            myfile.write(data)
            myfile.close()
            if level>7 and print_watermark:
                _print_water_mark(fp)
            _write_wld(fp,level,row,col)

# =============================================================================
class google2file(object):
    """ 下载 """

    def __init__(self, argv):
        self.l, self.t, self.r, self.b = -180.0, 90.0, -180.0, 90.0
        self.out = ""
        self.name = ""
        self.url = "http://mt%d.google.cn/vt/lyrs=s@132&x=%d&y=%d&z=%d" 
        self.file_format = 'tif'
        self.levels = []
        self.mpcnt = 1
        self.haswatermark = not self.__verify_license()
        self.__config_init()
        self.__argparse_init(argv)
        self.__parse_task()

    #########################################################################
    def __argparse_init(self,argv):
        if self.__verify_license():
            msg  =strlic = "\n授权版本 %s." % lic.License.hostName()
        else:
            msg  = "\n免费试用版本."

        msg = tileserver.__author__ + msg
        parser = argparse.ArgumentParser(description="谷歌地图转超图缓存.", epilog=msg)
        parser.add_argument("file", default="g.tsk", help="task file.")
        self.args = parser.parse_args(argv)

    #########################################################################
    def __config_init(self):
        config = ConfigParser.ConfigParser()
        _path = os.path.join(cm.app_path(), "g2f.cfg")
        config.read(_path)
        if config:
            self.mpcnt = config.getint("config", "multiprocess")
            self.mpcnt = max(1, self.mpcnt) # 确保大于1
            self.url = config.get("image", "url")

    #########################################################################
    def __verify_license(self):
        _path = cm.app_path()
        def _find_file(_dir):
            file_list = os.listdir(_dir)
            for fp in file_list:
                if fp.endswith(".lic"):
                    return os.path.join(_dir, fp)
            return None
        _path = _find_file(_path) 
        if not _path:
            return False

        pn = os.path.abspath(_path)
        if os.path.isfile(pn):
            lics = lic.License(pn)
            host = lics.hostName()
            return lics.verify(host, LICENSE_APP_NAME)
        else:
            return False

    #########################################################################
    def __parse_task(self):
        taskfile = self.args.file
        if not os.path.isfile(taskfile):
            taskfile = os.path.join(cm.app_path(), "g.tsk")
        if not os.path.isfile(taskfile):
            logger.error("任务文件未找到,%s" % taskfile)
            return None
        
        _tsk = cfg.from_file(taskfile, 'task')
        if 'bbox' in _tsk:
            bbox = [float(i.strip()) for i in _tsk['bbox'].split(',')]
            if len(bbox)==4:  
                self.l,self.t,self.r,self.b = bbox
        if 'level' in _tsk:
            self.levels = int(_tsk['level'].strip())
        if 'out' in _tsk:
            self.out = _tsk['out'].strip()
        if 'name' in _tsk:
            self.name = _tsk['name']
        if 'format' in _tsk:
            self.file_format = _tsk['format'].strip().lower()
        if 'width' in _tsk:
            self.width = int(_tsk['width'].strip())
        if 'height' in _tsk:
            self.height = int(_tsk['height'].strip())
        if 'overwrite' in _tsk:
            self.over_write = True if _tsk['overwrite'].lower()=='true' else False
    
    #########################################################################
    def download(self):
        def _calc_tiles(l,t,r,b, level):
            tiles = []
            tms = srsweb.GlobalMercator() 
            rs,re,cs,ce = tms.calcRowColByLatLon(l,t,r,b, level) 
            for row in range(rs, re+1):
                for col in range(cs, ce+1):
                    tiles.append((level, row, col))
            return tiles
        
        def _download_single_process(tile_list, out_dir, strurl, over_write, has_water_mark):
            download_g_tiles(tile_list, out_dir, strurl, over_write,has_water_mark)
        
        def _download_multi_process(tile_list, out_dir, strurl, \
                over_wirte,has_water_mark, mpcnt):
            def _split_by_process(tiles, mpcnt):
                """ 根据进程数目,瓦片张数划分合理的任务 """
                mplist = [[] for i in range(mpcnt)]
                for i in xrange(len(tiles)):
                    mplist[i%mpcnt].append(tiles[i])
                return mplist

            mplist = _split_by_process(tile_list, mpcnt)
            plist = []
            m = mp.Manager()
            for i in xrange(len(mplist)):
                bboxs = mplist[i]
                if not bboxs:
                    continue
                p = mp.Process(target=download_g_tiles, args=(bboxs,out_dir, \
                    strurl, over_wirte,has_water_mark))
                plist.append( (p, len(bboxs)) )

            for p, cnt in plist:
                p.start()
            for p, cnt in plist:
                p.join()
                logger.info("子进程(id=%d), 瓦片张数(%d), 已完成." % (p.pid, cnt) )

        l,t,r,b = self.l, self.t, self.r, self.b
        total_tiles = _calc_tiles(l,t,r,b,self.levels)
        logger.info("地理范围:左(%f),上(%f),右(%f),下(%f)" % (l,t,r,b))
        logger.info("层级:(%d), 瓦片总数%d张." % (self.levels, len(total_tiles)))

        tmp_dir = os.path.join(self.out, TEMP_DIR)
        if not os.path.exists(tmp_dir):
            os.makedirs(tmp_dir)

        if self.mpcnt<2:
            _download_single_process(total_tiles, tmp_dir, self.url, \
                    self.over_write,self.haswatermark)
        else:
            _download_multi_process(total_tiles, tmp_dir, \
                    self.url,self.over_write,self.haswatermark,self.mpcnt)

    #########################################################################
    def merge(self):
        l,t,r,b,level = self.l,self.t,self.r,self.b, self.levels
        tms = srsweb.GlobalMercator() 
        rs,re,cs,ce = tms.calcRowColByLatLon(l,t,r,b, level) 
        total_pix_w, total_pix_h = (ce-cs+1)*256, (re-rs+1)*256
        out_w, out_h = self.width, self.height
        fcnt_col, fcnt_row = int(math.ceil(total_pix_w*1.0/out_w)), int(math.ceil(total_pix_h*1.0/out_h))
        ftile_list = [[] for i in range(fcnt_row*fcnt_col)]
        for row in range(rs, re+1):
            _row = (row-rs)*256/out_h
            for col in range(cs, ce+1):
                _col = (col-cs)*256/out_w
                _idx = fcnt_col * _row + _col
                ftile_list[_idx].append((row, col))

        out_dir = self.out
        tmp_dir = os.path.join(out_dir, TEMP_DIR)
        cmd_mgr = os.path.join(cm.app_path(), 'gdal_merge.py')
        ext = 'tif' if self.file_format.lower()=='tif' else 'img'

        for i in range(len(ftile_list)):
            tile_list = ftile_list[i]
            _out_file = os.path.join(out_dir, "level%d_%d.%s" % (level, i, ext))
            _in_files = []

            for (row,col) in tile_list:
                _path = os.path.join(tmp_dir, GOOGLE_TILE_LOCAL_NAME % (level,row,col))
                if os.path.isfile(_path):
                    _in_files.append('%s' % _path)

            if not _in_files:
                logger.info("没有找到要拼接的文件.")
                return None
            cmd_out_file = '%s' % _out_file
            cmd_in_files = ' '.join(_in_files)
            cmd = '%s -o %s %s' % (cmd_mgr, cmd_out_file, cmd_in_files)
            #print cmd
            argv = cmd.split()
            gdal_merge.main(argv) 
            logger.info("第%d个文件拼接完成, %s" % (i+1,cmd_out_file))

    #########################################################################
    def check_tiles(self):
        """ 检查下载瓦片坐标完整性 """
        l,t,r,b,level = self.l,self.t,self.r,self.b, self.levels
        tms = srsweb.GlobalMercator() 
        rs,re,cs,ce = tms.calcRowColByLatLon(l,t,r,b, level) 
        out_dir = self.out
        tmp_dir = os.path.join(out_dir, TEMP_DIR)
        _flag = True
        for row in range(rs, re+1):
            for col in range(cs, ce+1):
                _path = os.path.join(tmp_dir, GOOGLE_TILE_LOCAL_NAME % (level,row,col))
                _jpg = os.path.isfile(_path) 
                _wld = os.path.isfile(_path[:-3]+'wld')
                if _jpg and _wld:
                    continue
                else:
                    if not _jpg: 
                        logger.info("瓦片未找到,%s" % _path)
                    elif not _wld: 
                        logger.info("瓦片未找到,%s" % _path)
                    _flag = False
        return _flag

    #########################################################################
    def run(self):

        logger.info(38 * "-")
        t1 = time.time()
        self.download()
        t2 = time.time()
        h,m,s = google2file.calc_cross_time(t1,t2)

        if self.check_tiles():
            logger.info("下载完成,耗时:%dh, %dm, %.2lfs" % (h,m,s))
        else:
            logger.info("下载瓦片未完成,请继续下载.")

        t3 = time.time()
        self.merge()
        t4 = time.time()
        h,m,s = google2file.calc_cross_time(t3,t4)
        logger.info("拼接完成,耗时:%dh, %dm, %.2lfs" % (h,m,s))

        h,m,s = google2file.calc_cross_time(t1,t4)
        logger.info("全部完成,总耗时:%dh, %dm, %.2lfs" % (h,m,s))
        logger.info(38 * "=")

    @staticmethod
    def calc_cross_time(t1, t2):
        """ t1,t2分别为开始,结束时间 """
        s = t2-t1
        h,m = int(s/60/60), int(s/60)
        m,s = m-h*60, s-h*60-m*60
        return h,m,s

# =============================================================================
def log_init():
    """ 初始化日志 """
    dirName = cm.app_path()
    name = time.strftime("%Y-%m-%d.log")
    logfile = os.path.join(dirName, "log", name)
    logfile = os.path.abspath(logfile)
    if not os.path.exists(os.path.dirname(logfile)):
        os.makedirs(os.path.dirname(logfile))

    logger = logging.getLogger("")
    logger.setLevel(logging.DEBUG)

    fh = logging.FileHandler(logfile)
    fh.setLevel(logging.DEBUG)

    ch = logging.StreamHandler()
    ch.setLevel(logging.DEBUG)

    formatter = logging.Formatter(fmt="%(asctime)s %(message)s", datefmt="%Y-%m-%d %H:%M:%S >")
    fh.setFormatter(formatter)
    ch.setFormatter(formatter)

    logger.addHandler(fh)
    logger.addHandler(ch)


def main():
    log_init()
    google2file(sys.argv[1:]).run()

# =============================================================================
if __name__=="__main__":
    mp.freeze_support()
    main()
