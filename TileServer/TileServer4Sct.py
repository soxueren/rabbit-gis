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

    def OnButtonRun(self, event):
	if not self.check():return False
	mapname=self.txtName.GetValue()
	ext='bil'
	outPath=self.txtOut.GetValue()
	outPath=os.path.join(outPath, mapname)
	if not os.path.exists(outPath): os.makedirs(outPath)

	l,t,r,b, xres, yres=imf.calcBoundary(self.fileList)
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
	imgtile = i2t.Image2Tiles(outPath) 
	imgtile.hook(self.printLog)
	imgtile.setExt(ext)
    
	maxstep=endl-startl+1
	dlg = self.createProcessDlg("生成地形缓存", "生成地形缓存", maxstep)
        keepGoing = True

	for i in xrange(endl, startl-1, -1):
	    self.printLog(('开始处理第%d层数据...' % i))
	    (keepGoing, skip) = dlg.Update(maxstep-(i-startl), ('正在处理第%d层数据' % i))
	    #imgtile.toTiles(self.fileList, i, outPath, True)
	    if i==startl: dlg.Destroy()
	self.printLog('All done.')
	del imgtile,sci 

#---------------------------------------------------------------------------


#---------------------------------------------------------------------------
def main():
    app = wx.App(False)  # Create a new app, don't redirect stdout/stderr to a window.
    frame = SctFrame(None, wx.ID_ANY, cm.APPNAME+"-Sct-生成三维地形缓存-"+cm.VERSION) # A Frame is a top-level window.
    frame.Show(True)     # Show the frame.
    app.MainLoop()


if __name__ == '__main__':
    main()
