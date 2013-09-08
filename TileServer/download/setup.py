# -*- coding:gbk -*-

import os, sys
from cx_Freeze import setup, Executable
import tileserver

buildPath=os.path.join(os.path.dirname(__file__), 'build')
for root, dirs, files in os.walk(buildPath):
    for name in files:
        os.remove(os.path.join(root, name))
    #for name in dirs:
    #   os.removedirs(os.path.join(root, name))

base = None
if sys.platform == "win32":
    base = "Win32GUI"

buildOpts = dict(
        compressed=True,
        includes=['numpy'],
        excludes=['Tkinter','tcl'],
        #zip_includes=['smSci/sci3d.sci3d'],
        include_files=['logo.png','icon.ico','g.tsk'],
        )

exeTables = [Executable("g2s.py", \
                        targetName='g2s.exe',\
                        icon='icon.ico',base=None), \
            Executable("gui.py", \
                        targetName='gui.exe',\
                        icon='icon.ico',base=base),]
setup(
        name = "TileServer",
        version = tileserver.__version__[:tileserver.__version__.rfind('.')],
        description = "Download Google tile files.",
        author = 'wenyulin.lin@gmail.com',
        author_email = 'wenyulin.lin@gmail.com',
        maintainer = 'linwenyu',
        url = 'www.atolin.net',
        options = dict(build_exe=buildOpts,),
        executables = exeTables)

