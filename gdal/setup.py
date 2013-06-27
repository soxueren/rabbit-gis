# -*- coding:utf-8 -*-

from cx_Freeze import setup, Executable

buildOptions = dict(
        compressed = True)

setup(
        name = "ts4Sci3d",
        version = "0.1.0",
        description = "build cache for SuperMap.",
	author = 'linwenyu',
        author_email = 'wenyulin.lin@gmail.com',
        maintainer = 'linwenyu',
        url = 'www.atolin.net',
	options = dict(build_exe = buildOptions),
        executables = [Executable("ts4Sci3d.py")])

