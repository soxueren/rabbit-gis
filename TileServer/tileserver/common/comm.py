#!/usr/bin/env python
# --*-- coding: gbk --*--

import os, sys

VERSION = "0.4.7.110"
APPNAME = "TileServer"
APPTITLE = "瓦刀"
APPNAME_GOOGLE_SCI3D = APPNAME + "-Google2Sic3d"

# 影像缓存工具
TITLESCI3D = APPTITLE + "-Sci3d-生成三维影像缓存-" + VERSION

# 地形缓存工具
TITLESCT = APPTITLE + "-Sct-生成三维地形缓存-" + VERSION

#---------------------------------------------------------------#
APPID_SCI3D = 1**2
APPID_SCT = 2**2
APPID_GOOGLE_SIC3D = 3**2

#---------------------------------------------------------------#

class iniFile(object):
    ''' ini文件解析 '''
    def __init__(self, name=""):
        self.mpcnt=0
        self.name = name
        self.parser()

    def parser(self):
        try:
            dirName = os.path.dirname(os.path.abspath(__file__))
        except:
            dirName = os.path.dirname(os.path.abspath(sys.argv[0]))

        if self.name=="": 
            self.name = 'config.ini'
        iniPath=os.path.join(dirName, self.name)
        # 打包后目录发生变化,需要去掉library.zip目录
        iniPath = iniPath.replace('library.zip','')
        iniPath = os.path.abspath(iniPath)

        if not os.path.isfile(iniPath):return 
        f = open(iniPath, "r")
        lines = f.readlines()
        f.close()

        for l in lines:
            l = l.strip()
            if l=="" or l[0]=="#":continue
            line = l.split("=")
            if len(line)==2:
                line[0]=line[0].strip()
                line[1]=line[1].strip()
                if line[0].lower()=="multiprocess" and line[1].isdigit():
                    self.mpcnt = int(line[1])
        
