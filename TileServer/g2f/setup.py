# -*- coding:gbk -*-

import os, sys
from cx_Freeze import setup, Executable
import version

buildPath=os.path.join(os.path.dirname(__file__), 'build')
for root, dirs, files in os.walk(buildPath):
    for name in files:
        os.remove(os.path.join(root, name))

base = None
if sys.platform == "win32":
    base = "Win32GUI"

buildOpts = dict(
        compressed=True,
        includes=['numpy'],
        excludes=['Tkinter'],
        #zip_includes=['smSci/sci3d.sci3d'],
        include_files=['logo.png','icon.ico','g.tsk','g2f.cfg','gui.cfg','使用说明.pdf'],
        )

exeTables = [Executable("gui.py", targetName='g2f.exe', icon='icon.ico',base=base),]
setup(
        name = "TileServer",
        version = version.__version__[:version.__version__.rfind('.')],
        description = "Download Google tile files.",
        author = 'wenyulin.lin@gmail.com',
        author_email = 'wenyulin.lin@gmail.com',
        maintainer = 'linwenyu',
        url = 'www.atolin.net',
        options = dict(build_exe=buildOpts,),
        executables = exeTables)

