#!/usr/bin/env python
# --*-- coding:utf-8 --*--

import os
import shutil
import common as cm
import zipfile

dirname = cm.APPNAME+'-'+cm.VERSION+'-win32'
tozipfile = cm.APPNAME+'-'+cm.VERSION+'-win32.zip'

def copyfiles(dirname):
    pf=os.path.join(os.getcwd(), 'build', 'exe.win32-2.7')
    pt=os.path.join(os.getcwd(), dirname)
    if not os.path.exists(pt): os.makedirs(pt)
    for root, dirs, files in os.walk(pf):
	for name in files:
	    src=os.path.join(root, name)
	    dst=src.replace(pf,pt)
	    if not os.path.exists(os.path.dirname(dst)):
		os.makedirs(os.path.dirname(dst))
	    print 'copied ', src 
	    shutil.copy2(src,dst)

def zipfiles(dirname, tozipfile):
    pf=os.path.join(os.getcwd(), dirname)
    if not os.path.exists(pf):return
    with zipfile.ZipFile(tozipfile, 'w') as myzip:
	for root, dirs, files in os.walk(pf):
	    for name in files:
		src=os.path.join(root, name)
		dst=src[src.find(dirname):]
		print 'zipped ', src
		myzip.write(dst)
def movefiles(dirname, tozipfile):
    pf=os.path.join(os.getcwd(), dirname)
    pt=os.path.join(os.getcwd(), 'dist', dirname)
    if os.path.exists(pt):shutil.rmtree(pt)
    shutil.move(pf, pt)
    pt=os.path.join(os.getcwd(), 'dist',tozipfile)
    if os.path.isfile(pt):os.remove(pt)
    shutil.move(tozipfile,pt)

if __name__=='__main__':
    copyfiles(dirname)
    zipfiles(dirname, tozipfile)
    movefiles(dirname, tozipfile)

