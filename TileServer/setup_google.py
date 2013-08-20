# -*- coding:gbk -*-

import os, sys
from cx_Freeze import setup, Executable
import common as cm
import cpyzip

buildPath=os.path.join(os.path.dirname(__file__), 'build')
for root, dirs, files in os.walk(buildPath):
    for name in files:
        os.remove(os.path.join(root, name))
    #for name in dirs:
    #   os.removedirs(os.path.join(root, name))

appname = cm.APPNAME+"-Google2Sci"

buildOpts = dict(
        compressed=True,
        includes=['numpy'],
        excludes = ['Tkinter'],
        #zip_includes=['smSci/sci3d.sci3d'],
        include_files=['icon.ico',
            'config.ini',
            'g.tsk',
            'docs/TileServer-Google2Sci3d 使用说明.pdf'],
        )

exeTables = [Executable("TileServer4Download.py", \
                        targetName = appname+'3d.exe',\
                        icon='icon.ico',base=None),] 
setup(
        name = "TileServer",
        version = cm.VERSION[:cm.VERSION.rfind('.')],
        description = "Google Maps to SuperMap tile files.",
        author = 'wenyulin.lin@gmail.com',
        author_email = 'wenyulin.lin@gmail.com',
        maintainer = 'linwenyu',
        url = 'www.atolin.net',
        options = dict(build_exe=buildOpts),
        executables = exeTables)

dirname = appname+'-'+cm.VERSION+'-win32'
tozipfile = appname+'-'+cm.VERSION+'-win32.zip'

cpyzip.copyfiles(dirname)
cpyzip.zipfiles(dirname, tozipfile)
cpyzip.movefiles(dirname, tozipfile)

