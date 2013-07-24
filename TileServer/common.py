#!/usr/bin/env python
# --*-- coding:utf-8 --*--

import os, sys

VERSION = "0.3.8.94"
APPNAME = "TileServer"
APPTITLE = "�ߵ�"

# Ӱ�񻺴湤��
TITLESCI3D = APPTITLE + "-Sci3d-������άӰ�񻺴�-" + VERSION

# ���λ��湤��
TITLESCT = APPTITLE + "-Sct-������ά���λ���-" + VERSION

#---------------------------------------------------------------#
APPID_SCI3D = 0x01
APPID_SCT = 0x02

#---------------------------------------------------------------#

class iniFile(object):
    ''' ini�ļ����� '''
    def __init__(self):
	self.mpcnt=0
	self.parser()

    def parser(self):
	try:
	    dirName = os.path.dirname(os.path.abspath(__file__))
	except:
	    dirName = os.path.dirname(os.path.abspath(sys.argv[0]))

	iniPath=os.path.join(dirName, 'config.ini')
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
	
