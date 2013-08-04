# -*- coding:utf-8 -*-

import os, sys
from cx_Freeze import setup, Executable
import common as cm

buildPath=os.path.join(os.path.dirname(__file__), 'build')
for root, dirs, files in os.walk(buildPath):
    for name in files:
	os.remove(os.path.join(root, name))
    #for name in dirs:
    #	os.removedirs(os.path.join(root, name))

base = None
if sys.platform == "win32":
    base = "Win32GUI"

buildOpts = dict(
        compressed=True,
	includes=['numpy'],
	#zip_includes=['smSci/sci3d.sci3d'],
	include_files=['logo.png','icon.ico',
	    'config.ini','ReadMe.txt','docs\\TileServer-Sci3d 使用说明.pdf',
	    'docs\\TileServer-Sct 使用说明.pdf'],
	)

msiOpts=dict(
	add_to_path=True,
	target_name=cm.APPNAME+'-'+cm.VERSION
	)

exeTables = [Executable("TileServer4Sci3d.py", \
			targetName=cm.APPNAME+'-4Sci3d.exe',\
			icon='icon.ico',base=base), \
	    Executable("TileServer4Sct.py", \
			targetName=cm.APPNAME+'-4Sct.exe',\
			icon='icon.ico',base=base),]
setup(
        name = "TileServer",
	version = cm.VERSION[:cm.VERSION.rfind('.')],
        description = "Build Sci3d tile files.",
	author = 'wenyulin.lin@gmail.com',
        author_email = 'wenyulin.lin@gmail.com',
        maintainer = 'linwenyu',
        url = 'www.atolin.net',
	options = dict(build_exe=buildOpts, bdist_msi=msiOpts),
        executables = exeTables)

