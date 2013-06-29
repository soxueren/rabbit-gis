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

buildOptions = dict(
        compressed=True,
	includes=['numpy'],
	#zip_includes=['smSci/sci3d.sci3d','smSci/sci3d.sci3d'],
	include_files=['smSci/sci3d.sci3d','smSci/sci3d.sci3d'],
	)

setup(
        name = "ts4Sci3d",
        version = cm.VERSION,
        description = "Build Sci3d tile files.",
	author = 'linwenyu',
        author_email = 'wenyulin.lin@gmail.com',
        maintainer = 'linwenyu',
        url = 'www.atolin.net',
	options = dict(build_exe = buildOptions),
        executables = [Executable("ts4Sci3d.py", targetName='TileServer4Sci3d.exe',icon='icon.ico'), ])

