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

try:
    from agw import advancedsplash as AS
except ImportError: # if it's not there locally, try the wxPython lib.
    import wx.lib.agw.advancedsplash as AS


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

	self.txtIn=txtin = wx.TextCtrl(panel, -1, "", size=(520,-1),style=wx.TE_READONLY)
        btnFile = wx.Button(panel, wx.ID_ANY, "�ļ�")
        btnDir = wx.Button(panel, wx.ID_ANY, "Ŀ¼")
	label=wx.StaticText(panel, -1, "����:")
	box=wx.BoxSizer(wx.HORIZONTAL)
	box.Add(label,0, wx.ALL|wx.ALIGN_LEFT|wx.ALIGN_BOTTOM, 5) 
	box.Add(txtin,0, wx.ALL|wx.ALIGN_LEFT|wx.ALIGN_BOTTOM, 5) 
	box.Add(btnFile,0, wx.ALL|wx.ALIGN_LEFT|wx.ALIGN_BOTTOM, 5) 
	box.Add(btnDir,0, wx.ALL|wx.ALIGN_LEFT|wx.ALIGN_BOTTOM, 5) 
	sizerIn.Add(box,0, wx.ALL|wx.ALIGN_LEFT|wx.ALIGN_BOTTOM, 5) 

	box=wx.StaticBox(panel,-1,"")
	boxSiezer=wx.StaticBoxSizer(box, wx.HORIZONTAL)
	self.txtLvlBeg=txtbeg=wx.TextCtrl(panel, -1, "1", size=(25,-1),style=wx.TE_READONLY)
	self.txtLvlEnd=txtend=wx.TextCtrl(panel, -1, "20", size=(25,-1),style=wx.TE_READONLY)
        self.spinLvlBeg = wx.SpinButton(panel, -1, (-1,-1), (-1,-1), wx.SP_VERTICAL)
        self.spinLvlEnd = wx.SpinButton(panel, -1, (-1,-1), (-1,-1), wx.SP_VERTICAL)
        self.spinLvlBeg.SetRange(1, 20)
        self.spinLvlEnd.SetRange(1, 20)
        self.spinLvlBeg.SetValue(1)
        self.spinLvlEnd.SetValue(20)

        self.Bind(wx.EVT_SPIN, self.OnSpinBeg, self.spinLvlBeg)
        self.Bind(wx.EVT_SPIN, self.OnSpinEnd, self.spinLvlEnd)

	label=wx.StaticText(panel, -1, "��ʼ�㼶:")
	box=wx.BoxSizer(wx.HORIZONTAL)
	box.Add(label,0, wx.ALL|wx.ALIGN_LEFT|wx.ALIGN_BOTTOM, 5) 
	box.Add(self.txtLvlBeg,0, wx.ALL|wx.ALIGN_LEFT|wx.ALIGN_BOTTOM, 5) 
	box.Add(self.spinLvlBeg,0, wx.ALL|wx.ALIGN_LEFT|wx.ALIGN_BOTTOM, 5) 
	boxSiezer.Add(box,0, wx.ALL|wx.ALIGN_LEFT|wx.ALIGN_BOTTOM, 5) 

	label=wx.StaticText(panel, -1, "��ֹ�㼶:")
	box=wx.BoxSizer(wx.HORIZONTAL)
	box.Add(label,0, wx.ALL|wx.ALIGN_LEFT|wx.ALIGN_BOTTOM, 5) 
	box.Add(self.txtLvlEnd,0, wx.ALL|wx.ALIGN_LEFT|wx.ALIGN_BOTTOM, 5) 
	box.Add(self.spinLvlEnd,0, wx.ALL|wx.ALIGN_LEFT|wx.ALIGN_BOTTOM, 5) 
	boxSiezer.Add(box,0, wx.ALL|wx.ALIGN_LEFT|wx.ALIGN_BOTTOM, 5) 


	label=wx.StaticText(panel, -1, "���:")
	self.txtOut=textOutPath= wx.TextCtrl(panel, -1, "", size=(520,-1))
        btnOutPath = wx.Button(panel, wx.ID_ANY, "Ŀ¼")
	box=wx.BoxSizer(wx.HORIZONTAL)
	box.Add(label,0, wx.ALL|wx.ALIGN_LEFT|wx.ALIGN_BOTTOM, 5) 
	box.Add(textOutPath,0, wx.ALL|wx.ALIGN_LEFT|wx.ALIGN_BOTTOM, 5) 
	box.Add(btnOutPath,0, wx.ALL|wx.ALIGN_LEFT|wx.ALIGN_BOTTOM, 5) 
	sizerIn.Add(box,0, wx.ALL|wx.ALIGN_LEFT|wx.ALIGN_BOTTOM, 5) 

        line = wx.StaticLine(panel, -1, size=(750,-1), style=wx.LI_HORIZONTAL)
	sizerIn.Add(line,0, wx.ALL|wx.ALIGN_LEFT|wx.ALIGN_BOTTOM, 5) 

	labelName = wx.StaticText(panel, -1, "��������:")
	self.txtName=textName=wx.TextCtrl(panel, -1, "", size=(200,-1))
	self.rb=rb= wx.RadioBox(panel, -1, "��Ƭ����", wx.DefaultPosition, wx.DefaultSize,
                ['png','jpg'], 2, wx.RA_SPECIFY_COLS)
	button=wx.Button(panel, -1, "����")
	self.Bind(wx.EVT_BUTTON, self.OnButtonRun, button)

	box=wx.BoxSizer(wx.HORIZONTAL)
	box.Add(labelName,0, wx.ALL|wx.ALIGN_LEFT|wx.ALIGN_BOTTOM, 5) 
	box.Add(textName,0, wx.ALL|wx.ALIGN_LEFT|wx.ALIGN_BOTTOM, 5) 
	box.Add(rb,0, wx.ALL|wx.ALIGN_LEFT|wx.ALIGN_BOTTOM, 5) 
	box.Add(boxSiezer,0, wx.ALL|wx.ALIGN_LEFT|wx.ALIGN_BOTTOM, 5) 
	box.Add(button,0, wx.ALL|wx.ALIGN_LEFT|wx.ALIGN_BOTTOM, 5) 
	sizerIn.Add(box,0, wx.ALL|wx.ALIGN_LEFT|wx.ALIGN_BOTTOM, 5) 


        self.Bind(wx.EVT_RADIOBOX, self.EvtRadioBox, rb)

	
	self.log=logText = wx.TextCtrl(panel,-1,"",size=(750,220),style=wx.TE_READONLY|wx.TE_RICH|wx.TE_MULTILINE|wx.EXPAND)
	logBox = wx.StaticBox(panel, -1, "��־:")
	sizerLog = wx.StaticBoxSizer(logBox, wx.HORIZONTAL)
	sizerLog.Add(logText,0, wx.ALL|wx.EXPAND, 5) 

	self.psizer=sizer = wx.BoxSizer(wx.VERTICAL)
	sizer.Add(sizerIn, 0, wx.EXPAND|wx.ALL, 25)
	sizer.Add(sizerLog, 0, wx.EXPAND|wx.LEFT|wx.RIGHT, 25)
        line = wx.StaticLine(panel, -1, size=(750,-1), style=wx.LI_HORIZONTAL)
	sizer.Add(line, 0, wx.EXPAND|wx.LEFT|wx.RIGHT, 25)

	self.uiButtonOK()

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
    
    def uiButtonOK(self):
        btnsizer = wx.StdDialogButtonSizer()
        if wx.Platform != "__WXMSW__":
            btn = wx.ContextHelpButton(self)
            btnsizer.AddButton(btn)
        
        btn = wx.Button(self.panel, wx.ID_OK, "���")
        btn.SetHelpText("The OK button completes the dialog")
        btn.SetDefault()
	self.Bind(wx.EVT_BUTTON, self.OnButtonClear, btn)
        btnsizer.AddButton(btn)

        btn = wx.Button(self.panel, wx.ID_CANCEL, "�ر�")
        btn.SetHelpText("The Cancel button cancels the dialog.")
        btnsizer.AddButton(btn)
	self.Bind(wx.EVT_BUTTON, self.OnCloseMe, btn)

        btn = wx.Button(self.panel, wx.ID_HELP)
        btn.SetHelpText("The Cancel button cancels the dialog.")
        btnsizer.AddButton(btn)
        btnsizer.Realize()
	self.Bind(wx.EVT_BUTTON, self.OnButtonHelp, btn)

        self.psizer.Add(btnsizer, 0, wx.ALIGN_RIGHT|wx.ALL, 15)

    def uiSplash(self):
	try:
	    dirName = os.path.dirname(os.path.abspath(__file__))
	except:
	    dirName = os.path.dirname(os.path.abspath(sys.argv[0]))
	pn =os.path.join(dirName, 'logo.png')
	if not os.path.exists(pn):return 
        bitmap = wx.Bitmap(pn, wx.BITMAP_TYPE_PNG)
        shadow = wx.WHITE
        frame = AS.AdvancedSplash(self, bitmap=bitmap, timeout=3000,
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
        info.Name = cm.APPNAME 
        info.Version = cm.VERSION 
        info.Copyright = "(C) 2006-2013 www.atolin.net ��������Ȩ��.\n\n"
	strdes="������άӰ�񻺴�.\n\n�Զ�ƴ��,���������������ص�.\n\n"#.decode('gb2312')
	strdes+="��ֱ�ӽ�Ӱ���зֳɵ���άӰ�񻺴��ļ�.\n\n"#.decode('gb2312')
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
	self.checkPrj()
	self.defaultLevels()

    def checkPrj(self):
	''' ȷ����������Ϊ��γ������ '''
	wgsList=[]
	for f in self.fileList:
	    ifile=imf.ImageFile(f)
	    if ifile.isGeographic():
		wgsList.append(f)
	    else:
		self.printLog(('��WGS84����: %s' % f))
	self.fileList=wgsList

    def defaultLevels(self):
	''' �����������ݵ�Ĭ����ʼ��ֹ�㼶  '''
	if len(self.fileList)==0: return
	l,t,r,b, xres, yres = imf.calcBoundary(self.fileList)
	mapbnd=l,t,r,b
	endl=smSci.smSci3d.calcEndLevel(xres)
	startl=smSci.smSci3d.calcStartLevel(l,t,r,b,xres,endl)
        self.spinLvlBeg.SetRange(startl, endl-1)
        self.spinLvlEnd.SetRange(startl+1, endl)
        self.txtLvlBeg.SetValue(str(startl))
        self.txtLvlEnd.SetValue(str(endl))
	#self.printLog('start:'+str(startl)+',end:'+str(endl))

    def OnButtonClear(self, event):
	''' ������־��Ϣ '''
	self.log.Clear()
	
    def EvtRadioBox(self, event):
	pass
        #self.log.AppendText('EvtRadioBox: %d\n' % event.GetInt())

    def OnSpinBeg(self, event):
        self.txtLvlBeg.SetValue(str(event.GetPosition()))

    def OnSpinEnd(self, event):
        self.txtLvlEnd.SetValue(str(event.GetPosition()))
	
	
    def printLog(self, msg, newline=True):
	strtime=time.strftime("%Y-%m-%d %H:%M:%S> ")
	strlog=strtime+msg
	if newline: 
	    self.log.AppendText(strlog+"\n")
	else:
	    self.log.AppendText(strlog+"\r")

    def check(self):
	if self.txtIn.GetValue()=="":
	    self.printLog("����·��Ϊ��.")
	    return False
	if self.txtOut.GetValue()=="":
	    self.printLog("���·��Ϊ��.")
	    return False
	if self.txtName.GetValue()=="":
	    self.printLog("��������Ϊ��.")
	    return False
	return True


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
	for i in xrange(endl, startl-1, -1):
	    self.printLog(('��ʼ�����%d������...' % i))
	    imgtile.toTiles(self.fileList, i, outPath)
	self.printLog('All done.')
	del imgtile,sci 

#---------------------------------------------------------------------------


#---------------------------------------------------------------------------
def main():
    app = wx.App(False)  # Create a new app, don't redirect stdout/stderr to a window.
    frame = MyFrame(None, wx.ID_ANY, cm.APPNAME+"-Sci3d-������άӰ�񻺴�-"+cm.VERSION) # A Frame is a top-level window.
    frame.Show(True)     # Show the frame.
    app.MainLoop()


if __name__ == '__main__':
    main()
