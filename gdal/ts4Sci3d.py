#!/usr/bin/env python
# --*-- coding:utf-8 --*--

import  wx

#---------------------------------------------------------------------------

class MyFrame(wx.Frame):
    def __init__(
            self, parent, ID, title, pos=wx.DefaultPosition,
            size=(1024,600), style=wx.RESIZE_BORDER|wx.SYSTEM_MENU|wx.CAPTION|wx.CLOSE_BOX|wx.CLIP_CHILDREN):

        wx.Frame.__init__(self, parent, ID, title, pos, size, style)
        panel = wx.Panel(self, -1)

	inBox = wx.StaticBox(panel, -1, "输入:")
	text = wx.TextCtrl(panel, -1, "", size=(420,-1))
        button = wx.Button(panel, 100, "文件")
        btnDir = wx.Button(panel, 100, "目录")

	sizerIn = wx.StaticBoxSizer(inBox, wx.HORIZONTAL)
	sizerIn.Add(text,0, wx.ALL|wx.ALIGN_LEFT|wx.ALIGN_BOTTOM, 5) 
	sizerIn.Add(button,0, wx.ALL|wx.ALIGN_LEFT|wx.ALIGN_BOTTOM, 5) 
	sizerIn.Add(btnDir,0, wx.ALL|wx.ALIGN_LEFT|wx.ALIGN_BOTTOM, 5) 

	outBox = wx.StaticBox(panel, -1, "输出:")
	textOutPath = wx.TextCtrl(panel, -1, "", size=(420,-1))
        btnOutPath = wx.Button(panel, 100, "目录")
	
	labelFormat = wx.StaticText(panel, -1, "瓦片类型:")
	labelName = wx.StaticText(panel, -1, "缓存名称:")
	textName = wx.TextCtrl(panel, -1, "", size=(120,-1))
	sampleList=['png','jpg']
	chFormat = wx.Choice(panel, -1, (100, 50), choices = sampleList)

	sizerOut = wx.StaticBoxSizer(outBox, wx.HORIZONTAL)
	sizerOut.Add(textOutPath,0, wx.ALL|wx.ALIGN_LEFT|wx.ALIGN_BOTTOM, 5) 
	sizerOut.Add(btnOutPath,0, wx.ALL|wx.ALIGN_LEFT|wx.ALIGN_BOTTOM, 5) 
	sizerOut.Add(labelFormat,0, wx.ALL|wx.ALIGN_LEFT|wx.ALIGN_BOTTOM, 5) 
	sizerOut.Add(chFormat,0, wx.ALL|wx.ALIGN_LEFT|wx.ALIGN_BOTTOM, 5) 
	sizerOut.Add(labelName,0, wx.ALL|wx.ALIGN_LEFT|wx.ALIGN_BOTTOM, 5) 
	sizerOut.Add(textName,0, wx.ALL|wx.ALIGN_LEFT|wx.ALIGN_BOTTOM, 5) 

	
	self.log = logText = wx.TextCtrl(panel,-1,"",size=(600,480),style=wx.TE_RICH|wx.TE_MULTILINE|wx.EXPAND)
	logBox = wx.StaticBox(panel, -1, "日志:")
	sizerLog = wx.StaticBoxSizer(logBox, wx.HORIZONTAL)
	sizerLog.Add(logText,0, wx.ALL|wx.EXPAND, 5) 

	sizer = wx.BoxSizer(wx.VERTICAL)
	sizer.Add(sizerIn, 0, wx.EXPAND|wx.ALL, 25)
	sizer.Add(sizerOut, 0, wx.EXPAND|wx.LEFT|wx.RIGHT, 25)
	sizer.Add(sizerLog, 0, wx.EXPAND|wx.LEFT|wx.RIGHT, 25)

        #self.Bind(wx.EVT_BUTTON, self.OnCloseMe, button)
	self.Bind(wx.EVT_BUTTON, self.OnButtonFile, button)
	self.Bind(wx.EVT_BUTTON, self.OnButtonDir, btnDir)
        self.Bind(wx.EVT_CLOSE, self.OnCloseWindow)
	panel.SetSizer(sizer)
	
	#self.SetSizer(sizer)
	#self.SetAutoLayout(1)
	#sizer.Fit(self)
	self.Show()
	


    def OnCloseMe(self, event):
        self.Close(True)

    def OnCloseWindow(self, event):
        self.Destroy()

    def OnButtonDir(self, event):
	dlg = wx.DirDialog(self, "Choose a directory:",
                          style=wx.DD_DEFAULT_STYLE
                           #| wx.DD_DIR_MUST_EXIST
                           #| wx.DD_CHANGE_DIR
                           )

        if dlg.ShowModal() == wx.ID_OK:
            self.log.WriteText('You selected: %s\n' % dlg.GetPath())
        dlg.Destroy()

    def OnButtonFile(self, event):
	wildcard = "GeoTiff (*.tif)|*.tif|"     \
           "GeoTiff (*.tiff)|*.tiff|" \
           "Erdas Image File (*.img)|*.img" \
	    
	dlg = wx.FileDialog(self, message="Choose a file",
            defaultDir=os.getcwd(), 
            defaultFile="",
            wildcard=wildcard,
            style=wx.OPEN | wx.MULTIPLE | wx.CHANGE_DIR)

        if dlg.ShowModal() == wx.ID_OK:
            paths = dlg.GetPaths()
            self.log.WriteText('You selected %d files:' % len(paths))
            for path in paths:
                self.log.WriteText('           %s\n' % path)

        self.log.WriteText("CWD: %s\n" % os.getcwd())
        dlg.Destroy()
	
	

#---------------------------------------------------------------------------


#---------------------------------------------------------------------------


if __name__ == '__main__':
    import sys,os
    app = wx.App(False)  # Create a new app, don't redirect stdout/stderr to a window.
    frame = MyFrame(None, wx.ID_ANY, "生成三维影像缓存") # A Frame is a top-level window.
    frame.Show(True)     # Show the frame.
    app.MainLoop()
