#!/usr/bin/env python
# --*-- coding: gbk --*--

import os, sys

VERSION = "0.4.7.110"
APPNAME = "TileServer"
APPTITLE = "�ߵ�"
APPNAME_GOOGLE_SCI3D = APPNAME + "-Google2Sic3d"

# Ӱ�񻺴湤��
TITLESCI3D = APPTITLE + "-Sci3d-������άӰ�񻺴�-" + VERSION

# ���λ��湤��
TITLESCT = APPTITLE + "-Sct-������ά���λ���-" + VERSION

#---------------------------------------------------------------#
APPID_SCI3D = 1**2
APPID_SCT = 2**2
APPID_GOOGLE_SIC3D = 3**2

#---------------------------------------------------------------#

class iniFile(object):
    ''' ini�ļ����� '''
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
        # �����Ŀ¼�����仯,��Ҫȥ��library.zipĿ¼
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
        
