#!/usr/bin/env python
# --*-- coding:gbk --*--

import os
import sys
import shutil
import zipfile

APPNAME = "九九下载-谷歌地图转超图缓存"

def copyfiles(app_name):
    pf = os.path.join(os.getcwd(), 'build', 'exe.win32-2.7')
    pt = os.path.join(os.getcwd(), app_name)
    if not os.path.exists(pt):
	os.makedirs(pt)
    for root, dirs, files in os.walk(pf):
        for name in files:
            src = os.path.join(root, name)
            dst = src.replace(pf,pt)
            if not os.path.exists(os.path.dirname(dst)):
                os.makedirs(os.path.app_name(dst))
            print 'copied ', src 
            shutil.copy2(src,dst)

def zipfiles(app_name, tozipfile):
    pf = os.path.join(os.getcwd(), app_name)
    if not os.path.exists(pf): 
	return
    with zipfile.ZipFile(tozipfile, 'w') as myzip:
        for root, dirs, files in os.walk(pf):
            for name in files:
                src = os.path.join(root, name)
                dst = src[src.find(app_name):]
                print 'zipped ', src
                myzip.write(dst)

def movefiles(app_name, tozipfile):
    pf = os.path.join(os.getcwd(), app_name)
    pt = os.path.join(os.getcwd(), 'dist', app_name)
    if os.path.exists(pt):
	shutil.rmtree(pt)
    shutil.move(pf, pt)
    pt = os.path.join(os.getcwd(), 'dist',tozipfile)
    if os.path.isfile(pt):
	os.remove(pt)
    shutil.move(tozipfile,pt)

if __name__=='__main__':
    import version 
    ver = version.__version__
    if len(sys.argv)==2:
	ver = sys.argv[1].strip().lower()
    app_name = "%s-v%s" % (APPNAME, ver)
    tozipfile = app_name+'.zip'

    copyfiles(app_name)
    zipfiles(app_name, tozipfile)
    movefiles(app_name, tozipfile)

