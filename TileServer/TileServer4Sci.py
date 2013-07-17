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
#import  wx.lib.mixins.listctrl  as  listmix

try:
    from agw import advancedsplash as AS
except ImportError: # if it's not there locally, try the wxPython lib.
    import wx.lib.agw.advancedsplash as AS

#---------------------------------------------------------------------------

class MyListCtrl(wx.ListCtrl):
    '''
                   listmix.ListCtrlAutoWidthMixin,
                   listmix.TextEditMixin):
    '''

    def __init__(self, parent, ID, pos=wx.DefaultPosition,
                 size=wx.DefaultSize, style=0):
        wx.ListCtrl.__init__(self, parent, ID, pos, size, style)

        #listmix.ListCtrlAutoWidthMixin.__init__(self)
        self.Populate()
        #listmix.TextEditMixin.__init__(self)

    def Populate(self):
        # for normal, simple columns, you can add them like this:
	self.InsertColumn(0, "序号")
        self.InsertColumn(1, "比例尺", wx.LIST_FORMAT_LEFT,200)
        self.InsertColumn(2, "标题")

    def GetScales(self):
	''' 返回比例尺 '''
	cnt = self.GetItemCount()
	scales=[]
	for i in range(cnt):
	    strs = self.GetItem(i, 2).GetText()
	    if strs!="" and strs.isdigit(): 
		scales.append(int(strs))
	return scales
    
    def SetScales(self, scales):
	self.DeleteAllItems()
        for i in xrange(len(scales)):
            index = self.InsertStringItem(sys.maxint, str(i+1))
	    scale = scales[i]
	    self.SetStringItem(index, 1, ("1/%d" % scale))
	    self.SetStringItem(index, 2, ("%d" % scale))
            self.SetItemData(index, i)
	self.RefreshItems(0, len(scales))

    def SetStringItem(self, index, col, data):
        if col in range(3):
            wx.ListCtrl.SetStringItem(self, index, col, data)


#---------------------------------------------------------------------------

g_wildcard = "GeoTiff (*.tif)|*.tif|"     \
           "GeoTiff (*.tiff)|*.tiff|" \
           "Erdas Image File (*.img)|*.img" \

class MyFrame(wx.Frame):
    def __init__(
            self, parent, ID, title, pos=wx.DefaultPosition,
            size=(1024,600), style=wx.RESIZE_BORDER|wx.SYSTEM_MENU|wx.CAPTION|wx.CLOSE_BOX|wx.CLIP_CHILDREN):

        wx.Frame.__init__(self, parent, ID, title, pos, size, style)
	self.uiSplash()
	self.SetIcon(wx.Icon('icon.ico', wx.BITMAP_TYPE_ICO))
	self.fileList=[]
        self.panel=panel= wx.Panel(self, -1)

	inBox = wx.StaticBox(panel, -1, "")
	sizerIn = wx.StaticBoxSizer(inBox, wx.VERTICAL)

	self.txtIn=txtin = wx.TextCtrl(panel, -1, "", size=(560,-1),style=wx.TE_READONLY)
        btnFile = wx.Button(panel, wx.ID_ANY, "文件")
        btnDir = wx.Button(panel, wx.ID_ANY, "目录")
	label=wx.StaticText(panel, -1, "输入:")
	box=wx.BoxSizer(wx.HORIZONTAL)
	box.Add(label,0, wx.ALL|wx.ALIGN_LEFT|wx.ALIGN_BOTTOM, 5) 
	box.Add(txtin,0, wx.ALL|wx.ALIGN_LEFT|wx.ALIGN_BOTTOM, 5) 
	box.Add(btnFile,0, wx.ALL|wx.ALIGN_LEFT|wx.ALIGN_BOTTOM, 5) 
	box.Add(btnDir,0, wx.ALL|wx.ALIGN_LEFT|wx.ALIGN_BOTTOM, 5) 
	sizerIn.Add(box,0, wx.ALL|wx.ALIGN_LEFT|wx.ALIGN_BOTTOM, 5) 

	box=wx.StaticBox(panel,-1,"比例尺:")
	boxSizer=wx.StaticBoxSizer(box, wx.HORIZONTAL)

        self.listCtrl = MyListCtrl(panel, wx.NewId(),
                                 style=wx.LC_REPORT
                                 #| wx.BORDER_NONE
                                 | wx.LC_SORT_ASCENDING, size=(400,120))
        boxSizer.Add(self.listCtrl, 0, wx.ALL|wx.ALIGN_LEFT|wx.ALIGN_BOTTOM, 5)

	label=wx.StaticText(panel, -1, "输出:")
	self.txtOut=textOutPath= wx.TextCtrl(panel, -1, "", size=(560,-1))
        btnOutPath = wx.Button(panel, wx.ID_ANY, "目录")
	box=wx.BoxSizer(wx.HORIZONTAL)
	box.Add(label,0, wx.ALL|wx.ALIGN_LEFT|wx.ALIGN_BOTTOM, 5) 
	box.Add(textOutPath,0, wx.ALL|wx.ALIGN_LEFT|wx.ALIGN_BOTTOM, 5) 
	box.Add(btnOutPath,0, wx.ALL|wx.ALIGN_LEFT|wx.ALIGN_BOTTOM, 5) 
	sizerIn.Add(box,0, wx.ALL|wx.ALIGN_LEFT|wx.ALIGN_BOTTOM, 5) 

        line = wx.StaticLine(panel, -1, size=(770,-1), style=wx.LI_HORIZONTAL)
	sizerIn.Add(line,0, wx.ALL|wx.ALIGN_LEFT|wx.ALIGN_BOTTOM, 5) 

	box=wx.BoxSizer(wx.VERTICAL)
	self.uiVersion(box)
	self.uiCacheName(box)

	box2=wx.BoxSizer(wx.HORIZONTAL)
	box2.Add(boxSizer,0, wx.ALL|wx.ALIGN_LEFT|wx.ALIGN_BOTTOM, 5) 
	box2.Add(box,0, wx.ALL|wx.ALIGN_LEFT|wx.ALIGN_BOTTOM, 5) 
	self.uiButtonScale(box2)
	sizerIn.Add(box2,0, wx.ALL|wx.ALIGN_LEFT|wx.ALIGN_BOTTOM, 5) 

	
	self.log=logText = wx.TextCtrl(panel,-1,"",size=(770,200),style=wx.TE_READONLY|wx.TE_RICH|wx.TE_MULTILINE|wx.EXPAND)
	logBox = wx.StaticBox(panel, -1, "日志:")
	sizerLog = wx.StaticBoxSizer(logBox, wx.HORIZONTAL)
	sizerLog.Add(logText,0, wx.ALL|wx.EXPAND, 5) 

	self.psizer=sizer = wx.BoxSizer(wx.VERTICAL)
	sizer.Add(sizerIn, 0, wx.EXPAND|wx.ALL, 25)
	sizer.Add(sizerLog, 0, wx.EXPAND|wx.LEFT|wx.RIGHT, 25)
        line = wx.StaticLine(panel, -1, size=(770,-1), style=wx.LI_HORIZONTAL)
	sizer.Add(line, 0, wx.EXPAND|wx.LEFT|wx.RIGHT, 25)

	self.uiButtonOK(sizer)

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

    def uiTileType(self, sizer):
	''' 结果瓦片格式 '''
	self.rb=rb= wx.RadioBox(self.panel, -1, "瓦片类型", wx.DefaultPosition, (-1,-1),
                ['png','jpg','gif'], 2, wx.RA_SPECIFY_COLS)
	sizer.Add(self.rb,0, wx.ALL|wx.ALIGN_LEFT|wx.ALIGN_BOTTOM, 5) 
	pass

    def uiCacheName(self, sizer):
	labelName = wx.StaticText(self.panel, -1, "缓存名称:")
	sizer.Add(labelName,0, wx.ALL|wx.ALIGN_LEFT|wx.ALIGN_BOTTOM, 5) 
	self.txtName=textName=wx.TextCtrl(self.panel, -1, "", size=(160,-1))
	sizer.Add(textName,0, wx.ALL|wx.ALIGN_LEFT|wx.ALIGN_BOTTOM, 5) 

    def uiVersion(self, sizer):
	''' 缓存版本号 '''
        btn = wx.Button(self.panel, wx.NewId(), "添加比例尺", size=(160,-1))
	sizer.Add(btn,0, wx.ALL|wx.ALIGN_LEFT|wx.ALIGN_TOP, 5) 
	self.Bind(wx.EVT_BUTTON, self.OnButtonScaleAdd, btn)

	labelName = wx.StaticText(self.panel, -1, "缓存版本:")
	sizer.Add(labelName,0, wx.ALL|wx.ALIGN_LEFT|wx.ALIGN_BOTTOM, 5) 
	sampleList = ['4.0 (适用于iServer 6R)', '3.1 (适用于iServer 20)', 
		'3.0 (适用于iServer 20)', '2.1 (适用于IS.NET)', ' 2.1(适用于IS.NET)']
	
	# This combobox is created with a preset list of values.
        self.cb = cb = wx.ComboBox(self.panel, 500, sampleList[0], (90, 50), 
                         (160, -1), sampleList,
                         wx.CB_DROPDOWN
                         | wx.TE_PROCESS_ENTER
			 | wx.CB_READONLY
                         #| wx.CB_SORT
                         )
	sizer.Add(cb,0, wx.ALL|wx.ALIGN_LEFT|wx.ALIGN_BOTTOM, 5) 
	pass

    def uiButtonRun(self, sizer):
	button=wx.Button(self.panel, -1, "运行", size=(100,60))
	self.Bind(wx.EVT_BUTTON, self.OnButtonRun, button)
	sizer.Add(button,0, wx.ALL|wx.ALIGN_LEFT|wx.ALIGN_BOTTOM, 5) 


    def uiButtonScale(self, sizer):
	box = wx.BoxSizer(wx.VERTICAL)
	self.uiTileType(box)
	self.uiButtonRun(box)
	sizer.Add(box,0, wx.ALL|wx.ALIGN_LEFT|wx.ALIGN_BOTTOM, 5) 
    
    def uiButtonOK(self, sizer):
        btnsizer = wx.StdDialogButtonSizer()
        if wx.Platform != "__WXMSW__":
            btn = wx.ContextHelpButton(self)
            btnsizer.AddButton(btn)
        
        btn = wx.Button(self.panel, wx.ID_OK, "清除")
        btn.SetHelpText("The OK button completes the dialog")
        btn.SetDefault()
	self.Bind(wx.EVT_BUTTON, self.OnButtonClear, btn)
        btnsizer.AddButton(btn)

        btn = wx.Button(self.panel, wx.ID_CANCEL, "关闭")
        btn.SetHelpText("The Cancel button cancels the dialog.")
        btnsizer.AddButton(btn)
	self.Bind(wx.EVT_BUTTON, self.OnCloseMe, btn)

        btn = wx.Button(self.panel, wx.ID_HELP)
        btn.SetHelpText("The Cancel button cancels the dialog.")
        btnsizer.AddButton(btn)
        btnsizer.Realize()
	self.Bind(wx.EVT_BUTTON, self.OnButtonHelp, btn)

        sizer.Add(btnsizer, 0, wx.ALIGN_RIGHT|wx.ALL, 15)

    def uiSplash(self):
	try:
	    dirName = os.path.dirname(os.path.abspath(__file__))
	except:
	    dirName = os.path.dirname(os.path.abspath(sys.argv[0]))
	pn = os.path.join(dirName, 'logo.png')
	# 打包后目录发生变化,需要去掉library.zip目录
	pn = pn.replace('library.zip','')
	pn=os.path.abspath(pn)
	if not os.path.exists(pn):return 
        bitmap = wx.Bitmap(pn, wx.BITMAP_TYPE_PNG)
        shadow = wx.WHITE
        frame = AS.AdvancedSplash(self, bitmap=bitmap, timeout=1000,
                                  agwStyle=AS.AS_TIMEOUT |
                                  AS.AS_CENTER_ON_PARENT |
                                  AS.AS_SHADOW_BITMAP,
                                  shadowcolour=shadow)
	    

    def OnCloseMe(self, event):
        self.Close(True)

    def OnCloseWindow(self, event):
        self.Destroy()

    def OnButtonHelp(self, event):
	# First we create and fill the info object
        info = wx.AboutDialogInfo()
        info.Name = cm.APPTITLE+"-"+cm.APPNAME 
        info.Version = cm.VERSION 
        info.Copyright = "(C) 2006-2013 www.atolin.net 保留所有权利.\n\n"
	strdes="影像生成二维地图缓存.\n\n自动拼接,无需入库是其最大特点.\n\n"#.decode('gb2312')
	strdes+="可直接将影像切分成二维地图缓存文件.\n\n"#.decode('gb2312')
        info.Description = wordwrap(info.Name+strdes, 
            350, wx.ClientDC(self))
        info.WebSite = ("http://www.atolin.net", info.Name)
	info.Developers = [ "wenyulin.lin@gmail.com","qq:42848918" ]

        #info.License = wordwrap(licenseText, 500, wx.ClientDC(self))
        # Then we call wx.AboutBox giving it that info object
        wx.AboutBox(info)

    def OnButtonDirOut(self, event):
	dlg = wx.DirDialog(self, "Choose a directory:",
                          style=wx.DD_DEFAULT_STYLE
                           #| wx.DD_DIR_MUST_EXIST
                           #| wx.DD_CHANGE_DIR
                           )

        if dlg.ShowModal() == wx.ID_OK:
	    self.txtOut.Clear()
	    self.txtOut.AppendText(dlg.GetPath())
        dlg.Destroy()

    def OnButtonDir(self, event):
	dlg = wx.DirDialog(self, "Choose a directory:",
                          style=wx.DD_DEFAULT_STYLE
                           #| wx.DD_DIR_MUST_EXIST
                           #| wx.DD_CHANGE_DIR
                           )

        if dlg.ShowModal() == wx.ID_OK:
	    self.txtIn.Clear()
	    self.txtIn.AppendText(dlg.GetPath())
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
		self.txtIn.AppendText(paths[0])
	    else:
		path=','.join(paths)
		self.txtIn.AppendText(path)
        dlg.Destroy()
	self.fillFileList()

    def OnButtonScaleAdd(self, event):
	scales = self.listCtrl.GetScales()
	str_scales = [str(s) for s in scales]
	dlg = wx.TextEntryDialog(self, '添加比例尺(分母)', "添加比例尺", ','.join(str_scales))
        if dlg.ShowModal() == wx.ID_OK:
	    txt = dlg.GetValue()
	    dlg.Destroy()
	    if txt!="":
		str_scales = txt.split(',')
		scales=[]
		for s in str_scales:
		    if s!="" and s.isdigit():
			scales.append(int(s))
		if len(scales)>0:
		    self.listCtrl.SetScales(scales)
		else:
		    self.printLog("比例尺%s未添加." % txt)
	    else:
		self.listCtrl.SetScales([])
        dlg.Destroy()


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
	#self.printLog('selected files:'+str(len(self.fileList)))
	self.defaultLevels()

    def defaultLevels(self):
	''' 计算输入数据的默认起始终止层级  '''
	if len(self.fileList)==0: return

	l,t,r,b, xres, yres = imf.calcBoundary(self.fileList)
	mapbnd=l,t,r,b
	print mapbnd
	endl=smSci.smSci3d.calcEndLevel(xres)
	startl=smSci.smSci3d.calcStartLevel(l,t,r,b,xres,endl)
	#self.printLog('start:'+str(startl)+',end:'+str(endl))

    def OnButtonClear(self, event):
	''' 清理日志信息 '''
	self.log.Clear()
	

    def printLog(self, msg, newline=True):
	strtime=time.strftime("%Y-%m-%d %H:%M:%S> ")
	strlog=strtime+msg
	if newline: 
	    self.log.AppendText(strlog+"\n")
	else:
	    self.log.AppendText(strlog+"\r")

    def check(self):
	if self.txtIn.GetValue()=="":
	    self.printLog("输入路径为空.")
	    return False
	if self.txtOut.GetValue()=="":
	    self.printLog("输出路径为空.")
	    return False
	if self.txtName.GetValue()=="":
	    self.printLog("缓存名称为空.")
	    return False
	if len(self.fileList)==0:return False
	return True


    def OnButtonRun(self, event):
	if not self.check():return False

    def createProgressDialog(self, title, msg, maxstep):
	dlg = wx.ProgressDialog(title,
                               msg,
                               maximum = maxstep,
                               #parent=self,
                               style = wx.PD_ELAPSED_TIME
			       | wx.PD_APP_MODAL
                                #| wx.PD_CAN_ABORT
                                #| wx.PD_ELAPSED_TIME
                                | wx.PD_ESTIMATED_TIME
                                | wx.PD_REMAINING_TIME)
	dlg.SetSize((400, 200))
	return dlg

    def printLine(self, msg):
	lineLen=48
	if len(msg)>lineLen:
	    self.printLog(msg)
	else:
	    line=(lineLen-len(msg))/2-1
	    msg="="*line+" "+msg+" "+"="*line
	    self.printLog(msg)


#---------------------------------------------------------------------------
__title__ = cm.TITLESCT


#---------------------------------------------------------------------------
def main():
    app = wx.App(False)  # Create a new app, don't redirect stdout/stderr to a window.
    frame = MyFrame(None, wx.ID_ANY, cm.APPNAME+"-Sct-生成三维地形缓存-"+cm.VERSION) # A Frame is a top-level window.
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
    frame.txtName.AppendText('sct')
    frame.fillFileList()
    frame.Show(True)     # Show the frame.
    app.MainLoop()


if __name__ == '__main__':
    unitTest()
    #main()
