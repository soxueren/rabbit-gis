#!/usr/bin/env python
# --*-- coding:gbk --*--

import sys,os
import time
import logging
import multiprocessing as mp
import ConfigParser

import wx
from wx.lib.wordwrap import wordwrap

import tileserver
from tileserver.common import license as lic
from tileserver.common import comm as cm
import tsk
import g2s


logger = logging.getLogger("")

try:
    from agw import advancedsplash as AS
except ImportError: # if it's not there locally, try the wxPython lib.
    import wx.lib.agw.advancedsplash as AS

#---------------------------------------------------------------------------
APPNAME = "谷歌地图转超图缓存"

#---------------------------------------------------------------------------
class DownloadFrame(wx.Frame):
    def __init__(
            self, parent, ID, title, pos=wx.DefaultPosition,
            size=(1024,600), style=wx.RESIZE_BORDER|wx.SYSTEM_MENU|wx.CAPTION|wx.CLOSE_BOX|wx.CLIP_CHILDREN):

        wx.Frame.__init__(self, parent, ID, title, pos, size, style)
        self.uiSplash()
        self.SetIcon(wx.Icon('icon.ico', wx.BITMAP_TYPE_ICO))
        self.license = False # 是否通过许可验证

        self.appid = 1 
        self.panel = panel= wx.Panel(self, -1)

        inBox = wx.StaticBox(panel, -1, "")
        sizerIn = wx.StaticBoxSizer(inBox, wx.VERTICAL)

        self.txtIn = txtin = wx.TextCtrl(panel, -1, "", size=(540,-1))
	label=wx.StaticText(panel, -1, "出图经纬度范围(英文逗号隔开), 格式为:left,top,right,bottom, 例如: -180,90,180,-90")
        box=wx.BoxSizer(wx.VERTICAL)
        box.Add(label,0, wx.ALL|wx.ALIGN_LEFT|wx.ALIGN_BOTTOM, 5) 
        box.Add(txtin,0, wx.ALL|wx.ALIGN_LEFT|wx.ALIGN_BOTTOM, 5) 
        sizerIn.Add(box,0, wx.ALL|wx.ALIGN_LEFT|wx.ALIGN_BOTTOM, 5) 

        label=wx.StaticText(panel, -1, "级别列表(英文逗号隔开), 取值范围为[0,20]:")
        self.txtLevels = txtLevels = wx.TextCtrl(panel, -1, "", size=(540,-1))
        box=wx.BoxSizer(wx.VERTICAL)
        box.Add(label,0, wx.ALL|wx.ALIGN_LEFT|wx.ALIGN_BOTTOM, 5) 
        box.Add(txtLevels,0, wx.ALL|wx.ALIGN_LEFT|wx.ALIGN_BOTTOM, 5) 
        sizerIn.Add(box,0, wx.ALL|wx.ALIGN_LEFT|wx.ALIGN_BOTTOM, 5) 

        line = wx.StaticLine(panel, -1, size=(550,-1), style=wx.LI_HORIZONTAL)
        sizerIn.Add(line,0, wx.ALL|wx.ALIGN_LEFT|wx.ALIGN_BOTTOM, 5) 

        label=wx.StaticText(panel, -1, "输出结果位置目录:")
        self.txtOut=textOutPath= wx.TextCtrl(panel, -1, "", size=(540,-1))
        box=wx.BoxSizer(wx.VERTICAL)
        box.Add(label,0, wx.ALL|wx.ALIGN_LEFT|wx.ALIGN_BOTTOM, 5) 
        box.Add(textOutPath,0, wx.ALL|wx.ALIGN_LEFT|wx.ALIGN_BOTTOM, 5) 
        sizerIn.Add(box,0, wx.ALL|wx.ALIGN_LEFT|wx.ALIGN_BOTTOM, 5) 

	label=wx.StaticText(panel, -1, "输出结果瓦片类型, 取值范围为[png, jpg]:")
        self.txtTileFormat = txtTileFormat = wx.TextCtrl(panel, -1, "", size=(540,-1))
        box = wx.BoxSizer(wx.VERTICAL)
        box.Add(label,0, wx.ALL|wx.ALIGN_LEFT|wx.ALIGN_BOTTOM, 5) 
        box.Add(txtTileFormat,0, wx.ALL|wx.ALIGN_LEFT|wx.ALIGN_BOTTOM, 5) 
        sizerIn.Add(box,0, wx.ALL|wx.ALIGN_LEFT|wx.ALIGN_BOTTOM, 5) 

	label = wx.StaticText(panel, -1, "缓存类型:")
	cache_list = self.load_cache_type()
        self.ch = choice = wx.Choice(panel, -1, (100, 50), choices = cache_list)
	choice.SetSelection(0)
        self.Bind(wx.EVT_CHOICE, self.EvtChoice, self.ch)
        box = wx.BoxSizer(wx.HORIZONTAL)
        box.Add(label,0, wx.ALL|wx.ALIGN_LEFT|wx.ALIGN_BOTTOM, 5) 
        box.Add(choice,0, wx.ALL|wx.ALIGN_LEFT|wx.ALIGN_BOTTOM, 5) 
        sizerIn.Add(box,0, wx.ALL|wx.ALIGN_LEFT|wx.ALIGN_BOTTOM, 5) 

	label=wx.StaticText(panel, -1, "缓存名称:")
        self.txtName = txtName = wx.TextCtrl(panel, -1, "", size=(140,-1))
        box = wx.BoxSizer(wx.HORIZONTAL)
        box.Add(label,0, wx.ALL|wx.ALIGN_LEFT|wx.ALIGN_BOTTOM, 5) 
        box.Add(txtName,0, wx.ALL|wx.ALIGN_LEFT|wx.ALIGN_BOTTOM, 5) 
        sizerIn.Add(box,0, wx.ALL|wx.ALIGN_LEFT|wx.ALIGN_BOTTOM, 5) 

        self.psizer=sizer = wx.BoxSizer(wx.VERTICAL)
        sizer.Add(sizerIn, 0, wx.EXPAND|wx.ALL, 25)
        line = wx.StaticLine(panel, -1, size=(550,-1), style=wx.LI_HORIZONTAL)
        sizer.Add(line, 0, wx.EXPAND|wx.LEFT|wx.RIGHT, 25)

        self.uiButtonOK(sizer)
        
        self.Bind(wx.EVT_CLOSE, self.OnCloseWindow)
        panel.SetSizer(sizer)

        sizer=wx.BoxSizer(wx.VERTICAL)
        sizer.Add(panel, 0, wx.EXPAND|wx.ALL)
        self.SetSizer(sizer)
        #self.SetAutoLayout(1)
        sizer.Fit(self)
	
	self.logInit()
        logger.info("Google Maps转SuperMap缓存") 

	self.loadDefaultTsk()
        self.Show()

    def load_cache_type(self):
	path = cm.app_path()
	path = os.path.join(path, 'gui.cfg')
	cache_list = []
	if os.path.isfile(path):
	    config = ConfigParser.ConfigParser()
	    config.read(path)
	    cachetypes = config.get("gui", "cachetype")
	    cache_list = cachetypes.split(',')
	    cacheindexs = config.get("gui", "cacheindex")
	    self.index_list = cacheindexs.split(',') 
	return cache_list

    def verifyLicense(self):
	dirName = cm.app_path()
        pn =  self.findLicenseFile(dirName)
        pn = os.path.abspath(pn)
        if os.path.isfile(pn):
            lics = lic.License(pn)
            host = lics.hostName()
            self.license = lics.verify(host, self.appid)

    def findLicenseFile(self, dirName):
        fileList = os.listdir(dirName)
        for fp in fileList:
            if fp.endswith(".lic"):
                return os.path.join(dirName, fp)
        return ""

    def uiButtonOK(self, sizer):
        btnsizer = wx.StdDialogButtonSizer()
        if wx.Platform != "__WXMSW__":
            btn = wx.ContextHelpButton(self)
            btnsizer.AddButton(btn)
 
        btn = wx.Button(self.panel, wx.ID_OK, "运行")
        btnsizer.AddButton(btn)
        self.Bind(wx.EVT_BUTTON, self.OnButtonRun, btn)

        btn = wx.Button(self.panel, wx.ID_CANCEL, "退出")
        btn.SetHelpText("The Cancel button cancels the dialog.")
        btnsizer.AddButton(btn)
        self.Bind(wx.EVT_BUTTON, self.OnCloseMe, btn)

        btn = wx.Button(self.panel, wx.ID_HELP, "关于")
        btn.SetHelpText("The Cancel button cancels the dialog.")
        btnsizer.AddButton(btn)
        btnsizer.Realize()
        self.Bind(wx.EVT_BUTTON, self.OnButtonHelp, btn)

        sizer.Add(btnsizer, 0, wx.ALIGN_RIGHT|wx.ALL, 15)

    def uiSplash(self):
	dirName = cm.app_path()
        pn = os.path.join(dirName, 'logo.png')
        pn = os.path.abspath(pn)
        if not os.path.exists(pn):
	    return 

        bitmap = wx.Bitmap(pn, wx.BITMAP_TYPE_PNG)
        shadow = wx.WHITE
        frame = AS.AdvancedSplash(self, bitmap=bitmap, timeout=1000,
                                  agwStyle=AS.AS_TIMEOUT |
                                  AS.AS_CENTER_ON_PARENT |
                                  AS.AS_SHADOW_BITMAP,
                                  shadowcolour=shadow)
            

    def OnCloseMe(self, event):
        #self.Close(True)
        self.Destroy()
	sys.exit(1)

    def OnCloseWindow(self, event):
        self.Destroy()
	sys.exit(1)

    def GetLicense(self):
        strlic = "免费试用版本."
        if self.license:
            strlic = "授权版本 %s." % lic.License.hostName()
        return strlic

    def OnButtonHelp(self, event):
        # First we create and fill the info object
        info = wx.AboutDialogInfo()
        info.Name = APPNAME 
        info.Version = tileserver.__version__ 
        info.Copyright = tileserver.__copyright__ + ("\n\n%s\n" % self.GetLicense())

        strdes="将Google地图下载并转存为SuperMap缓存.\n\n"
        info.Description = wordwrap(strdes, 350, wx.ClientDC(self))
        info.WebSite = (tileserver.__website__, info.Name)
        info.Developers = [ tileserver.__author__ ]

        #info.License = wordwrap(self.GetLicense(), 500, wx.ClientDC(self))
        # Then we call wx.AboutBox giving it that info object
        wx.AboutBox(info)

    def EvtChoice(self, event):
	pass


    def check(self):
        if self.txtIn.GetValue()=="":
            logger.info("输入路径为空.")
            return False
        if self.txtOut.GetValue()=="":
            logger.info("输出路径为空.")
            return False
        if self.txtName.GetValue()=="":
            logger.info("缓存名称为空.")
            return False
        if self.txtTileFormat.GetValue()=="":
            logger.info("瓦片格式为空.")
            return False
        if self.txtLevels.GetValue()=="":
            logger.info("瓦片格式为空.")
            return False
        return True

    def OnButtonRun(self, event):
        if not self.check():
	    return False

	if 2 == self.ch.GetSelection():
	    logger.info("生成三维场景缓存")
	else:
	    logger.info("生成二维地图缓存")
	    tskpath = self.saveDefaultTsk()
	    g2s.Download([tskpath]).run() 

    def logInit(self):
	""" 初始化日志 """
        dirName = cm.app_path()
        name = time.strftime("%Y-%m-%d.log")
        logfile = os.path.join(dirName, "log", name)
        logfile = os.path.abspath(logfile)
        if not os.path.exists(os.path.dirname(logfile)):
            os.makedirs(os.path.dirname(logfile))

        logger.setLevel(logging.DEBUG)

        fh = logging.FileHandler(logfile)
        fh.setLevel(logging.DEBUG)

        ch = logging.StreamHandler()
        ch.setLevel(logging.DEBUG)

        formatter = logging.Formatter(fmt="%(asctime)s %(message)s", datefmt="%Y-%m-%d %H:%M:%S >")
        fh.setFormatter(formatter)
        ch.setFormatter(formatter)

        logger.addHandler(fh)
        logger.addHandler(ch)

    def loadDefaultTsk(self):
	path = self.config_path()
	if not os.path.isfile(path):
	    path = os.path.join(cm.app_path(), 'g.tsk')
	if not os.path.isfile(path):
	    logger.error("No file found, %s" % path)
	    return None

	f = open(path)
	lines = f.readlines()
	f.close()

	_tsk = tsk.from_lines(lines)
	if _tsk is None:
	    logger.error("Parse file error, %s" % path)
	    return False

	if 'bbox' in _tsk:
	    val = ','.join(map(str,_tsk['bbox'])) 
	    self.txtIn.AppendText(val)
	if 'level' in _tsk:
	    val = ','.join(map(str,_tsk['level'])) 
	    self.txtLevels.AppendText(val)
	if 'out' in _tsk:
	    self.txtOut.AppendText(_tsk['out'])
	if 'name' in _tsk:
	    self.txtName.AppendText(_tsk['name'])
	if 'format' in _tsk:
	    self.txtTileFormat.AppendText(_tsk['format'])
    
    def saveDefaultTsk(self):
	""" 保存以便下次启动加载值 """
	out_path = self.txtOut.GetValue()
	if not os.path.exists(out_path):
	    os.makedirs(out_path)
	
	out_path = os.path.join(out_path, 'g.tsk') 
	cfg_path = os.path.join(cm.app_path(), 'gui.cfg')

	config = ConfigParser.ConfigParser()
	config.read(cfg_path)
	config.set('path','path', out_path)
	with open(cfg_path, 'wb') as configfile:
	    config.write(configfile)

	idx = self.ch.GetSelection()
	idx = int(self.index_list[idx])
	if 31==idx:
	    strver = 'ver31'
	elif 30==idx:
	    strver = 'ver30'
	else:
	    strver = 'ver31'

	_tsk = {'bbox':'','level':'','name':'','format':'','version':'','out':'ver31'}
	_tsk['bbox'] = self.txtIn.GetValue()
	_tsk['level'] = self.txtLevels.GetValue()
	_tsk['format'] = self.txtTileFormat.GetValue()
	_tsk['out'] = self.txtOut.GetValue()
	_tsk['name'] = self.txtName.GetValue()
	_tsk['version'] = strver

	lines = tsk.to_lines(_tsk)
	f = open(out_path, 'w')
	f.writelines(lines)
	f.close()
	return out_path

    def config_path(self):
	path = os.path.join(cm.app_path(), 'gui.cfg')
	config = ConfigParser.ConfigParser()
	config.read(path)
	path = config.get("path", "path")
	return path

#---------------------------------------------------------------------------
def main():
    app = wx.App(True)  # Create a new app, don't redirect stdout/stderr to a window.
    title = APPNAME + "V"+ tileserver.__version__
    frame = DownloadFrame(None, wx.ID_ANY, title) # A Frame is a top-level window.
    frame.Show(True)     # Show the frame.
    app.MainLoop()

#---------------------------------------------------------------------------

if __name__ == '__main__':
    mp.freeze_support()
    main()
