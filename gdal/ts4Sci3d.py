#!/usr/bin/env python
# --*-- coding:utf-8 --*--

import  wx
import time
import image2tile as i2t
import imageFile as imf
import smSci
#---------------------------------------------------------------------------

g_wildcard = "GeoTiff (*.tif)|*.tif|"     \
           "GeoTiff (*.tiff)|*.tiff|" \
           "Erdas Image File (*.img)|*.img" \

class MyFrame(wx.Frame):
    def __init__(
            self, parent, ID, title, pos=wx.DefaultPosition,
            size=(1024,600), style=wx.RESIZE_BORDER|wx.SYSTEM_MENU|wx.CAPTION|wx.CLOSE_BOX|wx.CLIP_CHILDREN):

        wx.Frame.__init__(self, parent, ID, title, pos, size, style)
	self.fileList=[]
        panel = wx.Panel(self, -1)

	inBox = wx.StaticBox(panel, -1, "")
	sizerIn = wx.StaticBoxSizer(inBox, wx.VERTICAL)

	self.txtIn=txtin = wx.TextCtrl(panel, -1, "", size=(520,-1))
        btnFile = wx.Button(panel, wx.ID_ANY, "文件")
        btnDir = wx.Button(panel, wx.ID_ANY, "目录")
	label=wx.StaticText(panel, -1, "输入:")
	box=wx.BoxSizer(wx.HORIZONTAL)
	box.Add(label,0, wx.ALL|wx.ALIGN_LEFT|wx.ALIGN_BOTTOM, 5) 
	box.Add(txtin,0, wx.ALL|wx.ALIGN_LEFT|wx.ALIGN_BOTTOM, 5) 
	box.Add(btnFile,0, wx.ALL|wx.ALIGN_LEFT|wx.ALIGN_BOTTOM, 5) 
	box.Add(btnDir,0, wx.ALL|wx.ALIGN_LEFT|wx.ALIGN_BOTTOM, 5) 
	sizerIn.Add(box,0, wx.ALL|wx.ALIGN_LEFT|wx.ALIGN_BOTTOM, 5) 

	box=wx.StaticBox(panel,-1,"")
	boxSiezer=wx.StaticBoxSizer(box, wx.HORIZONTAL)
	self.txtLvlBeg=txtbeg=wx.TextCtrl(panel, -1, "1", size=(20,-1),style=wx.TE_READONLY)
	self.txtLvlEnd=txtend=wx.TextCtrl(panel, -1, "20", size=(20,-1),style=wx.TE_READONLY)
        self.spinLvlBeg = wx.SpinButton(panel, -1, (-1,-1), (-1,-1), wx.SP_VERTICAL)
        self.spinLvlEnd = wx.SpinButton(panel, -1, (-1,-1), (-1,-1), wx.SP_VERTICAL)
        self.spinLvlBeg.SetRange(1, 20)
        self.spinLvlEnd.SetRange(1, 20)
        self.spinLvlBeg.SetValue(1)
        self.spinLvlEnd.SetValue(20)

        self.Bind(wx.EVT_SPIN, self.OnSpinBeg, self.spinLvlBeg)
        self.Bind(wx.EVT_SPIN, self.OnSpinEnd, self.spinLvlEnd)

	label=wx.StaticText(panel, -1, "起始层级:")
	box=wx.BoxSizer(wx.HORIZONTAL)
	box.Add(label,0, wx.ALL|wx.ALIGN_LEFT|wx.ALIGN_BOTTOM, 5) 
	box.Add(self.txtLvlBeg,0, wx.ALL|wx.ALIGN_LEFT|wx.ALIGN_BOTTOM, 5) 
	box.Add(self.spinLvlBeg,0, wx.ALL|wx.ALIGN_LEFT|wx.ALIGN_BOTTOM, 5) 
	boxSiezer.Add(box,0, wx.ALL|wx.ALIGN_LEFT|wx.ALIGN_BOTTOM, 5) 

	label=wx.StaticText(panel, -1, "终止层级:")
	box=wx.BoxSizer(wx.HORIZONTAL)
	box.Add(label,0, wx.ALL|wx.ALIGN_LEFT|wx.ALIGN_BOTTOM, 5) 
	box.Add(self.txtLvlEnd,0, wx.ALL|wx.ALIGN_LEFT|wx.ALIGN_BOTTOM, 5) 
	box.Add(self.spinLvlEnd,0, wx.ALL|wx.ALIGN_LEFT|wx.ALIGN_BOTTOM, 5) 
	boxSiezer.Add(box,0, wx.ALL|wx.ALIGN_LEFT|wx.ALIGN_BOTTOM, 5) 


	label=wx.StaticText(panel, -1, "输出:")
	self.txtOut=textOutPath= wx.TextCtrl(panel, -1, "", size=(520,-1))
        btnOutPath = wx.Button(panel, wx.ID_ANY, "目录")
	box=wx.BoxSizer(wx.HORIZONTAL)
	box.Add(label,0, wx.ALL|wx.ALIGN_LEFT|wx.ALIGN_BOTTOM, 5) 
	box.Add(textOutPath,0, wx.ALL|wx.ALIGN_LEFT|wx.ALIGN_BOTTOM, 5) 
	box.Add(btnOutPath,0, wx.ALL|wx.ALIGN_LEFT|wx.ALIGN_BOTTOM, 5) 
	sizerIn.Add(box,0, wx.ALL|wx.ALIGN_LEFT|wx.ALIGN_BOTTOM, 5) 

	labelName = wx.StaticText(panel, -1, "缓存名称:")
	self.txtName=textName=wx.TextCtrl(panel, -1, "", size=(200,-1))
	self.rb=rb= wx.RadioBox(panel, -1, "瓦片类型", wx.DefaultPosition, wx.DefaultSize,
                ['png','jpg'], 2, wx.RA_SPECIFY_COLS)
	button=wx.Button(panel, -1, "运行")
	self.Bind(wx.EVT_BUTTON, self.OnButtonRun, button)

	box=wx.BoxSizer(wx.HORIZONTAL)
	box.Add(labelName,0, wx.ALL|wx.ALIGN_LEFT|wx.ALIGN_BOTTOM, 5) 
	box.Add(textName,0, wx.ALL|wx.ALIGN_LEFT|wx.ALIGN_BOTTOM, 5) 
	box.Add(rb,0, wx.ALL|wx.ALIGN_LEFT|wx.ALIGN_BOTTOM, 5) 
	box.Add(boxSiezer,0, wx.ALL|wx.ALIGN_LEFT|wx.ALIGN_BOTTOM, 5) 
	box.Add(button,0, wx.ALL|wx.ALIGN_LEFT|wx.ALIGN_BOTTOM, 5) 
	sizerIn.Add(box,0, wx.ALL|wx.ALIGN_LEFT|wx.ALIGN_BOTTOM, 5) 


        self.Bind(wx.EVT_RADIOBOX, self.EvtRadioBox, rb)

	
	self.log=logText = wx.TextCtrl(panel,-1,"",size=(880,360),style=wx.TE_RICH|wx.TE_MULTILINE|wx.EXPAND)
	logBox = wx.StaticBox(panel, -1, "日志:")
	sizerLog = wx.StaticBoxSizer(logBox, wx.HORIZONTAL)
	sizerLog.Add(logText,0, wx.ALL|wx.EXPAND, 5) 

	sizer = wx.BoxSizer(wx.VERTICAL)
	sizer.Add(sizerIn, 0, wx.EXPAND|wx.ALL, 25)
	sizer.Add(sizerLog, 0, wx.EXPAND|wx.LEFT|wx.RIGHT, 25)

	self.Bind(wx.EVT_BUTTON, self.OnButtonFile, btnFile)
	self.Bind(wx.EVT_BUTTON, self.OnButtonDir, btnDir)
	self.Bind(wx.EVT_BUTTON, self.OnButtonDirOut, btnOutPath)
	
        self.Bind(wx.EVT_CLOSE, self.OnCloseWindow)
	panel.SetSizer(sizer)

	sizer=wx.BoxSizer(wx.VERTICAL)
	sizer.Add(panel, 0, wx.EXPAND|wx.ALL)
	self.SetSizer(sizer)
	#self.SetAutoLayout(1)
	sizer.Fit(self)
	self.Show()
	


    def OnCloseMe(self, event):
        self.Close(True)

    def OnCloseWindow(self, event):
        self.Destroy()

    def OnButtonDirOut(self, event):
	dlg = wx.DirDialog(self, "Choose a directory:",
                          style=wx.DD_DEFAULT_STYLE
                           #| wx.DD_DIR_MUST_EXIST
                           #| wx.DD_CHANGE_DIR
                           )

        if dlg.ShowModal() == wx.ID_OK:
	    self.txtOut.Clear()
	    self.txtOut.WriteText(dlg.GetPath())
        dlg.Destroy()

    def OnButtonDir(self, event):
	dlg = wx.DirDialog(self, "Choose a directory:",
                          style=wx.DD_DEFAULT_STYLE
                           #| wx.DD_DIR_MUST_EXIST
                           #| wx.DD_CHANGE_DIR
                           )

        if dlg.ShowModal() == wx.ID_OK:
	    self.txtIn.Clear()
	    self.txtIn.WriteText(dlg.GetPath())
        dlg.Destroy()
	self.fillFileList()

    def OnButtonFile(self, event):
	dlg = wx.FileDialog(self, message="Choose a file",
            defaultDir=os.getcwd(), 
            defaultFile="",
            wildcard=g_wildcard,
            style=wx.OPEN | wx.MULTIPLE | wx.CHANGE_DIR)

        if dlg.ShowModal() == wx.ID_OK:
            paths = dlg.GetPaths()
	    self.txtIn.Clear()
	    if len(paths)==1:
		self.txtIn.WriteText(paths[0])
	    else:
		path=','.join(paths)
		self.txtIn.WriteText(path)
        dlg.Destroy()
	self.fillFileList()

    def fillFileList(self):
	txtfiles=self.txtIn.GetValue() 
	self.fileList=[]
	if os.path.isdir(txtfiles):
	    imgList=i2t.Image2Tiles.listDir(txtfiles, 'tif')
	    self.fileList.extend(imgList)
	    imgList=i2t.Image2Tiles.listDir(txtfiles, 'tiff')
	    self.fileList.extend(imgList)
	    imgList=i2t.Image2Tiles.listDir(txtfiles, 'img')
	    self.fileList.extend(imgList)
	elif os.path.isfile(txtfiles):
	    self.fileList.append(txtfiles)
	elif txtfiles.find(',')!=-1:
	    imgList=txtfiles.split(',')
	    self.fileList.extend(imgList)
	#self.logPrint('selected files:'+str(len(self.fileList)))
	self.defaultLevels()

    def defaultLevels(self):
	l,t,r,b, xres, yres = imf.calcBoundary(self.fileList)
	mapbnd=l,t,r,b
	endl=smSci.smSci3d.calcEndLevel(xres)
	startl=smSci.smSci3d.calcStartLevel(l,t,r,b,xres,endl)
        self.spinLvlBeg.SetRange(startl, endl-1)
        self.spinLvlEnd.SetRange(startl+1, endl)
        self.txtLvlBeg.SetValue(str(startl))
        self.txtLvlEnd.SetValue(str(endl))
	#self.logPrint('start:'+str(startl)+',end:'+str(endl))
	
	
    def EvtRadioBox(self, event):
        self.log.WriteText('EvtRadioBox: %d\n' % event.GetInt())

    def OnSpinBeg(self, event):
        self.txtLvlBeg.SetValue(str(event.GetPosition()))

    def OnSpinEnd(self, event):
        self.txtLvlEnd.SetValue(str(event.GetPosition()))
	
	
    def logPrint(self, msg, newline=True):
	strtime=time.strftime("%Y-%m-%d %H:%M:%S>")
	strlog=strtime+" "+msg
	if newline: 
	    self.log.WriteText(strlog+"\n")
	else:
	    self.log.WriteText(strlog+"\r")

    def check(self):
	if self.txtIn.GetValue()=="":
	    self.logPrint("输入路径为空.")
	    return False
	if self.txtOut.GetValue()=="":
	    self.logPrint("输出路径为空.")
	    return False
	if self.txtName.GetValue()=="":
	    self.logPrint("缓存名称为空.")
	    return False
	return True


    def OnButtonRun(self, event):
	if not self.check():return False
	mapname=self.txtName.GetValue()
	ext='png' if self.rb.GetSelection()==1 else 'jpg'
	outPath=self.txtOut.GetValue()
	outPath=os.path.join(outPath, mapname)
	if not os.path.exists(outPath): os.makedirs(outPath)

	l,t,r,b, xres, yres=imf.calcBoundary(self.fileList)
	mapbnd=l,t,r,b

	endl=int(self.txtLvlEnd.GetValue())
	startl=int(self.txtLvlBeg.GetValue())
	w,h=smSci.smSci3d.calcWidthHeight(l,t,r,b,endl)
	
	self.logPrint(('文件数目:%d' % len(self.fileList)))
	self.logPrint(('地理范围:上下左右(%f,%f,%f,%f),分辨率(%f)' % (l,t,r,b,xres)))
	self.logPrint(('起始终止层级:(%d,%d)' % (startl, endl)))

	sci = smSci.smSci3d()
	sci.setParams(mapname, mapbnd, mapbnd, '')
	sci.setLevels(startl, endl)
	sci.setExtName(ext)
	sci.setWidthHeight(w,h)
	sci.saveSciFile(outPath)
    
	imgtile = i2t.Image2Tiles(outPath) 
	imgtile.hook(self.logPrint)
	for i in xrange(endl, startl, -1):
	    imgtile.toTiles(self.fileList, i, outPath)
	self.logPrint('All done.')

#---------------------------------------------------------------------------


#---------------------------------------------------------------------------


if __name__ == '__main__':
    import sys,os
    import common as cm
    app = wx.App(False)  # Create a new app, don't redirect stdout/stderr to a window.
    frame = MyFrame(None, wx.ID_ANY, "TileServer-生成三维影像缓存-"+cm.VERSION) # A Frame is a top-level window.
    frame.Show(True)     # Show the frame.
    app.MainLoop()
