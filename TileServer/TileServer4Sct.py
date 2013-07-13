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

    def uiCacheName(self, sizer):
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
	
	self.printLog(('�ļ���Ŀ:%d' % len(self.fileList)))
	self.printLog(('����Χ:��������(%f,%f,%f,%f),�ֱ���(%f)' % (l,t,r,b,xres)))
	self.printLog(('��ʼ��ֹ�㼶:(%d,%d)' % (startl, endl)))

	sci = smSci.smSct()
	sci.setParams(mapname, mapbnd, mapbnd, '')
	sci.setLevels(startl, endl)
	sci.setWidthHeight(w,h)
	sci.saveSciFile(outPath)
    
	ext='.bil'
	imgtile = i2t.Image2Tiles(outPath) 
	imgtile.hook(self.printLog)
	imgtile.setExt(ext)
    
	maxstep=endl-startl+2
	dlg = self.createProgressDialog("���ɵ��λ���", "���ɵ��λ���", maxstep)
        keepGoing = True

	self.printLine("Start")
	for i in xrange(startl, endl+1):
	    self.printLog(("��ʼ�����%d������..." % i))
	    (keepGoing, skip) = dlg.Update(i-startl+1, ("���ڴ����%d������..." % i))
	    imgtile.toTiles(self.fileList, i, outPath, True)
	    if i==endl: 
		dlg.Destroy()
		#dlg.Update(maxstep, "�������!")

	self.printLine("End, All done.")
	del imgtile,sci 
	dlg.Destroy()


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
    data = r'E:\�½��ļ���'
    fName = "\\before_900913.tif"
    frame.txtOut.AppendText(data)
    frame.txtIn.AppendText(data+fName)
    frame.txtName.AppendText('sct')
    frame.fillFileList()
    frame.Show(True)     # Show the frame.
    app.MainLoop()


if __name__ == '__main__':
    main()
    #unitTest()
