#!/usr/bin/env python
# --*-- coding:utf-8 --*--

import os
import shutil
import common as cm
import zipfile

dirname=cm.APPNAME+'-'+cm.VERSION+'-Win32'

def copyfiles():
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

def zipfiles():
    tozipfile=cm.APPNAME+'-'+cm.VERSION+'-Win32.zip'
    pf=os.path.join(os.getcwd(), dirname)
    if not os.path.exists(pf):return
    with zipfile.ZipFile(tozipfile, 'w') as myzip:
	for root, dirs, files in os.walk(pf):
	    for name in files:
		src=os.path.join(root, name)
		dst=src[src.find(dirname):]
		print 'zipped ', src
		myzip.write(dst)

if __name__=='__main__':
    copyfiles()
    zipfiles()

