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
def runMP(imgList, startl, endl, outPath, ext, printLog):
    ''' 多进程处理切图  '''
    print startl, endl, outPath, ext
    imgtile = i2t.Image2Tiles(outPath) 
    imgtile.hook(printLog)
    imgtile.setExt(ext)

    for i in xrange(startl, endl+1):
	#printLog(('开始处理第%d层数据...' % i))
	#msg = ("正在处理第%d层数据, 共[%d,%d]层..." % (i,startl, endl))
	imgtile.toTiles(imgList, i, outPath)
	if i==endl: 
	    pass
	
    del imgtile 
#---------------------------------------------------------------------------

g_wildcard = "GeoTiff (*.tif)|*.tif|"     \
           "GeoTiff (*.tiff)|*.tiff|" \
           "Erdas Image File (*.img)|*.img" \

class MyFrame(ts.TileServerFrame):
    def __init__(
            self, parent, ID, title, pos=wx.DefaultPosition,
            size=(1024,600), style=wx.RESIZE_BORDER|wx.SYSTEM_MENU|wx.CAPTION|wx.CLOSE_BOX|wx.CLIP_CHILDREN):

        ts.TileServerFrame.__init__(self, parent, ID, title, pos, size, style)

    def uiCacheName(self, sizer):
	box = wx.BoxSizer(wx.VERTICAL)
	labelName = wx.StaticText(self.panel, -1, "缓存名称:")
	box.Add(labelName,0, wx.ALL|wx.ALIGN_LEFT|wx.ALIGN_BOTTOM, 5) 
	self.txtName=textName=wx.TextCtrl(self.panel, -1, "", size=(180,-1))
	box.Add(textName,0, wx.ALL|wx.ALIGN_LEFT|wx.ALIGN_BOTTOM, 5) 
	sizer.Add(box,0, wx.ALL|wx.ALIGN_LEFT|wx.ALIGN_BOTTOM, 0) 

    def uiTileType(self, sizer):
	self.rb=rb= wx.RadioBox(self.panel, -1, "瓦片类型", wx.DefaultPosition, (-1,65),
                ['png','jpg'], 2, wx.RA_SPECIFY_COLS)
	sizer.Add(self.rb,0, wx.ALL|wx.ALIGN_LEFT|wx.ALIGN_BOTTOM, 5) 
        self.Bind(wx.EVT_RADIOBOX, self.EvtRadioBox, self.rb)

    def OnButtonHelp(self, event):
	# First we create and fill the info object
        info = wx.AboutDialogInfo()
        info.Name = cm.APPTITLE+"-"+cm.APPNAME 
        info.Version = cm.VERSION 
        info.Copyright = "(C) 2013 www.atolin.net 保留所有权利.\n\n"
	strdes="生成三维影像缓存工具.\n\n自动拼接,无需入库是其最大特点.\n\n"#.decode('gb2312')
	strdes+="可直接将影像切分成三维影像缓存文件.\n\n"#.decode('gb2312')
        info.Description = wordwrap(info.Name+strdes, 
            350, wx.ClientDC(self))
        info.WebSite = ("http://www.atolin.net", info.Name)
	info.Developers = ["wenyulin.lin@gmail.com", "qq:42848918"]
	#info.Artists = ["wenyu.lin"]

        #info.License = wordwrap(licenseText, 500, wx.ClientDC(self))
        # Then we call wx.AboutBox giving it that info object
        wx.AboutBox(info)

    def runSingleProcess(self, startl, endl, outPath, ext):
	''' 单进程模式切图 '''
	imgtile = i2t.Image2Tiles(outPath) 
	imgtile.hook(self.printLog)
	imgtile.setExt(ext)

	maxstep=endl-startl+2
	dlg = self.createProgressDialog("生成影像缓存", "生成影像缓存", maxstep)
        keepGoing = True
	
	self.printLine("Start")
	for i in xrange(startl, endl+1):
	    self.printLog(('开始处理第%d层数据...' % i))
	    msg = ("正在处理第%d层数据, 共[%d,%d]层..." % (i,startl, endl))
	    (keepGoing, skip) = dlg.Update(i-startl+1, msg)
	    imgtile.toTiles(self.fileList, i, outPath)
	    if i==endl: 
		dlg.Destroy()
	    
	self.printLine("End, All done.")
	del imgtile 
	dlg.Destroy()
        
    def runMultiProcess(self, startl, endl, outPath, ext):
	''' 多进程模式切图 '''
	
	self.printLine("Start")
	totalLevel = endl-startl+1
	plist = []
	if totalLevel<=4:
	    for i in range(startl, endl+1):
		p = mp.Process(target=runMP, args=(self.fileList, i, i, outPath, ext,None))
		plist.append(p)
	else:
	    p = mp.Process(target=runMP, args=(self.fileList, startl, endl-3, outPath, ext,None))
	    plist.append(p)
	    for i in range(endl-2, endl+1):
		p = mp.Process(target=runMP, args=(self.fileList, i, i, outPath, ext,None))
		plist.append(p)

	for p in plist:
	    p.start()
	for p in plist:
	    p.join()
	    
	self.printLine("End, All done.")
        
    def OnButtonRun(self, event):
	if not self.check():return False
	mapname=self.txtName.GetValue()
	ext='png' if self.rb.GetSelection()==0 else 'jpg'
	outPath=self.txtOut.GetValue()
	outPath=os.path.join(outPath, mapname)
	if not os.path.exists(outPath): os.makedirs(outPath)

	l,t,r,b, xres, yres=imf.calcGeographicBoundary(self.fileList)
	mapbnd=l,t,r,b

	endl=int(self.txtLvlEnd.GetValue())
	startl=int(self.txtLvlBeg.GetValue())
	w,h=smSci.smSci3d.calcWidthHeight(l,t,r,b,endl)
	
	self.printLog(('文件数目:%d' % len(self.fileList)))
	self.printLog(('地理范围:上下左右(%f,%f,%f,%f),分辨率(%f,%f)' % (l,t,r,b,xres,yres)))
	self.printLog(('起始终止层级:(%d,%d)' % (startl, endl)))

	sci = smSci.smSci3d()
	sci.setParams(mapname, mapbnd, mapbnd, '')
	sci.setLevels(startl, endl)
	sci.setExtName(ext)
	sci.setWidthHeight(w,h)
	sci.saveSciFile(outPath)
    
	if ext=='png': ext='.png'
	if ext=='jpg': ext='.jpg'

	#self.runSingleProcess(startl, endl, outPath, ext)
	self.runMultiProcess(startl, endl, outPath, ext)
	del sci
	    

#---------------------------------------------------------------------------
__title__ = cm.TITLESCI3D

def main():
    app = wx.App(False)  # Create a new app, don't redirect stdout/stderr to a window.
    frame = MyFrame(None, wx.ID_ANY, __title__) # A Frame is a top-level window.
    frame.Show(True)     # Show the frame.
    app.MainLoop()

#---------------------------------------------------------------------------

def unitTest():
    app = wx.App(False)  # Create a new app, don't redirect stdout/stderr to a window.
    frame = MyFrame(None, wx.ID_ANY, __title__) # A Frame is a top-level window.
    data = r'E:\新建文件夹'
    fName = "\\before_900913.tif"
    frame.txtOut.AppendText(data)
    frame.txtIn.AppendText(data+fName)
    frame.txtName.AppendText('sci3d')
    frame.fillFileList()
    frame.Show(True)     # Show the frame.
    app.MainLoop()


#---------------------------------------------------------------------------

if __name__ == '__main__':
    #main()
    unitTest()
