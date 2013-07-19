#!/usr/bin/env python
# --*-- coding:utf-8 --*--

import sys,os
import wx
import time
import image2tile as i2t
import imageFile as imf
import smSci
import common as cm
from wx.lib.wordwrap import wordwrap
import TileServerFrame as ts
import multiprocessing as mp

try:
    from agw import advancedsplash as AS
except ImportError: # if it's not there locally, try the wxPython lib.
    import wx.lib.agw.advancedsplash as AS


#---------------------------------------------------------------------------

g_wildcard = "GeoTiff (*.tif)|*.tif|"     \
           "GeoTiff (*.tiff)|*.tiff|" \
           "Erdas Image File (*.img)|*.img" \

class SctFrame(ts.TileServerFrame):
    def __init__(
            self, parent, ID, title, pos=wx.DefaultPosition,
            size=(1024,600), style=wx.RESIZE_BORDER|wx.SYSTEM_MENU|wx.CAPTION|wx.CLOSE_BOX|wx.CLIP_CHILDREN):

        ts.TileServerFrame.__init__(self, parent, ID, title, pos, size, style)

    def uiCacheName(self, sizer):
	labelName = wx.StaticText(self.panel, -1, "缓存名称:")
	sizer.Add(labelName,0, wx.ALL|wx.ALIGN_LEFT|wx.ALIGN_BOTTOM, 5) 
	self.txtName=textName=wx.TextCtrl(self.panel, -1, "", size=(225,-1))
	sizer.Add(textName,0, wx.ALL|wx.ALIGN_LEFT|wx.ALIGN_BOTTOM, 5) 

    def OnButtonRun(self, event):
	if not self.check():return False
	mapname=self.txtName.GetValue()
	ext='bil'
	outPath=self.txtOut.GetValue()
	outPath=os.path.join(outPath, mapname)
	if not os.path.exists(outPath): os.makedirs(outPath)

	l,t,r,b, xres, yres=imf.calcGeographicBoundary(self.fileList)
	mapbnd=l,t,r,b

	endl=int(self.txtLvlEnd.GetValue())
	startl=int(self.txtLvlBeg.GetValue())
	w,h=smSci.smSci3d.calcWidthHeight(l,t,r,b,endl)
	
	self.printLog(('文件数目:%d' % len(self.fileList)))
	self.printLog(('地理范围:上下左右(%f,%f,%f,%f),分辨率(%f)' % (l,t,r,b,xres)))
	self.printLog(('起始终止层级:(%d,%d)' % (startl, endl)))

	sci = smSci.smSct()
	sci.setParams(mapname, mapbnd, mapbnd, '')
	sci.setLevels(startl, endl)
	sci.setWidthHeight(w,h)
	sci.saveSciFile(outPath)
    
	ext='.bil'

	#self.runSingleProcess(startl, endl, outPath, ext, True)
	self.runMultiProcess(startl, endl, outPath, ext, True)
	del sci


#---------------------------------------------------------------------------
__title__ = cm.TITLESCT

def main():
    app = wx.App(False)  # Create a new app, don't redirect stdout/stderr to a window.
    frame = SctFrame(None, wx.ID_ANY, __title__) # A Frame is a top-level window.
    frame.Show(True)     # Show the frame.
    app.MainLoop()

#---------------------------------------------------------------------------
def unitTest():
    app = wx.App(False)  # Create a new app, don't redirect stdout/stderr to a window.
    frame = SctFrame(None, wx.ID_ANY, __title__) # A Frame is a top-level window.
    data = r'E:\新建文件夹'
    fName = "\\before_900913.tif"
    frame.txtOut.AppendText(data)
    frame.txtIn.AppendText(data+fName)
    frame.txtName.AppendText('sct')
    frame.fillFileList()
    frame.Show(True)     # Show the frame.
    app.MainLoop()


if __name__ == '__main__':
    mp.freeze_support()
    main()
    #unitTest()
