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

class MyFrame(ts.TileServerFrame):
    def __init__(
            self, parent, ID, title, pos=wx.DefaultPosition,
            size=(1024,600), style=wx.RESIZE_BORDER|wx.SYSTEM_MENU|wx.CAPTION|wx.CLOSE_BOX|wx.CLIP_CHILDREN):

        ts.TileServerFrame.__init__(self, parent, ID, title, pos, size, style)

    def uiCacheName(self, sizer):
	self.txtName=textName=wx.TextCtrl(self.panel, -1, "", size=(205,-1))
	sizer.Add(textName,0, wx.ALL|wx.ALIGN_LEFT|wx.ALIGN_BOTTOM, 5) 

    def uiTileType(self, sizer):
	self.rb=rb= wx.RadioBox(self.panel, -1, "��Ƭ����", wx.DefaultPosition, wx.DefaultSize,
                ['png','jpg'], 2, wx.RA_SPECIFY_COLS)
	sizer.Add(self.rb,0, wx.ALL|wx.ALIGN_LEFT|wx.ALIGN_BOTTOM, 5) 
        self.Bind(wx.EVT_RADIOBOX, self.EvtRadioBox, self.rb)

    def OnButtonHelp(self, event):
	# First we create and fill the info object
        info = wx.AboutDialogInfo()
        info.Name = cm.APPTITLE+"-"+cm.APPNAME 
        info.Version = cm.VERSION 
        info.Copyright = "(C) 2006-2013 www.atolin.net ��������Ȩ��.\n\n"
	strdes="������άӰ�񻺴湤��.\n\n�Զ�ƴ��,���������������ص�.\n\n"#.decode('gb2312')
	strdes+="��ֱ�ӽ�Ӱ���зֳ���άӰ�񻺴��ļ�.\n\n"#.decode('gb2312')
        info.Description = wordwrap(info.Name+strdes, 
            350, wx.ClientDC(self))
        info.WebSite = ("http://www.atolin.net", info.Name)
	info.Developers = [ "wenyulin.lin@gmail.com","qq:42848918" ]

        #info.License = wordwrap(licenseText, 500, wx.ClientDC(self))
        # Then we call wx.AboutBox giving it that info object
        wx.AboutBox(info)
        
    def OnButtonRun(self, event):
	if not self.check():return False
	mapname=self.txtName.GetValue()
	ext='png' if self.rb.GetSelection()==0 else 'jpg'
	outPath=self.txtOut.GetValue()
	outPath=os.path.join(outPath, mapname)
	if not os.path.exists(outPath): os.makedirs(outPath)

	l,t,r,b, xres, yres=imf.calcBoundary(self.fileList)
	mapbnd=l,t,r,b

	endl=int(self.txtLvlEnd.GetValue())
	startl=int(self.txtLvlBeg.GetValue())
	w,h=smSci.smSci3d.calcWidthHeight(l,t,r,b,endl)
	
	self.printLog(('�ļ���Ŀ:%d' % len(self.fileList)))
	self.printLog(('����Χ:��������(%f,%f,%f,%f),�ֱ���(%f)' % (l,t,r,b,xres)))
	self.printLog(('��ʼ��ֹ�㼶:(%d,%d)' % (startl, endl)))

	sci = smSci.smSci3d()
	sci.setParams(mapname, mapbnd, mapbnd, '')
	sci.setLevels(startl, endl)
	sci.setExtName(ext)
	sci.setWidthHeight(w,h)
	sci.saveSciFile(outPath)
    
	if ext=='png': ext='.png'
	if ext=='jpg': ext='.jpg'
	imgtile = i2t.Image2Tiles(outPath) 
	imgtile.hook(self.printLog)
	imgtile.setExt(ext)

	maxstep=endl-startl+1
	dlg = self.createProgressDialog("����Ӱ�񻺴�", "����Ӱ�񻺴�", maxstep)
        keepGoing = True
	
	self.printLine("Start")
	for i in xrange(endl, startl-1, -1):
	    self.printLog(('��ʼ�����%d������...' % i))
	    (keepGoing, skip) = dlg.Update(maxstep-(i-startl)-1,
			    ("���ڴ����%d������..." % i))
	    imgtile.toTiles(self.fileList, i, outPath)
	    if i==startl: 
		dlg.Destroy()
	    
	self.printLine("End, All done.")
	del imgtile,sci 
	dlg.Destroy()

#---------------------------------------------------------------------------
def main():
    app = wx.App(False)  # Create a new app, don't redirect stdout/stderr to a window.
    title = cm.APPTITLE + "-" +cm.APPNAME+"-Sci3d-������άӰ�񻺴�-"+cm.VERSION
    frame = MyFrame(None, wx.ID_ANY, title) # A Frame is a top-level window.
    frame.Show(True)     # Show the frame.
    app.MainLoop()

#---------------------------------------------------------------------------

def unitTest():
    app = wx.App(False)  # Create a new app, don't redirect stdout/stderr to a window.
    title = cm.APPTITLE + "-" +cm.APPNAME+"-Sci3d-������άӰ�񻺴�-"+cm.VERSION
    frame = MyFrame(None, wx.ID_ANY, title) # A Frame is a top-level window.
    data = r'E:\2013\2013-06\2013-06-14'
    fName = ""
    frame.txtOut.AppendText(data)
    frame.txtIn.AppendText(data+fName)
    frame.txtName.AppendText('abc')
    frame.fillFileList()
    frame.Show(True)     # Show the frame.
    app.MainLoop()


#---------------------------------------------------------------------------

if __name__ == '__main__':
    main()
    #unitTest()
