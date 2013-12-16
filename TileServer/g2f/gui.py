#!/usr/bin/env python
# --*-- coding:gbk --*--

import sys,os
import time
import logging
import multiprocessing as mp
import ConfigParser
import math

import wx
from wx.lib.wordwrap import wordwrap

import tileserver
from tileserver.common import license as lic
from tileserver.common import comm as cm
import version
import cfg
import g2f

logger = logging.getLogger("")

try:
    from agw import advancedsplash as AS
except ImportError: # if it's not there locally, try the wxPython lib.
    import wx.lib.agw.advancedsplash as AS

#---------------------------------------------------------------------------
APPNAME = "九九下载-谷歌地图转GeoTIF/IMG文件"
LICENSE_APP_NAME = "g2f"

#---------------------------------------------------------------------------
class DownloadFrame(wx.Frame):
    def __init__(
            self, parent, ID, title, pos=wx.DefaultPosition,
            size=(1024,600), style=wx.RESIZE_BORDER|wx.SYSTEM_MENU|wx.CAPTION|wx.CLOSE_BOX|wx.CLIP_CHILDREN):

        wx.Frame.__init__(self, parent, ID, title, pos, size, style)
        self.uiSplash()
        self.SetIcon(wx.Icon('icon.ico', wx.BITMAP_TYPE_ICO))
        self.license = False # 是否通过许可验证

        self.panel = panel= wx.Panel(self, -1)

        inBox = wx.StaticBox(panel, -1, "")
        sizerIn = wx.StaticBoxSizer(inBox, wx.VERTICAL)

        _text_size = 680
        _line_size = 690

        self.txt_bbox = txtin = wx.TextCtrl(panel, -1, "", size=(_text_size,-1))
        label=wx.StaticText(panel, -1, "谷歌地图经纬度范围, 格式为(英文逗号隔开):left,top,right,bottom, 单位为度, 例如: -180,90,180,-90")
        box=wx.BoxSizer(wx.VERTICAL)
        box.Add(label,0, wx.ALL|wx.ALIGN_LEFT|wx.ALIGN_BOTTOM, 5) 
        box.Add(txtin,0, wx.ALL|wx.ALIGN_LEFT|wx.ALIGN_BOTTOM, 5) 
        sizerIn.Add(box,0, wx.ALL|wx.ALIGN_LEFT|wx.ALIGN_BOTTOM, 5) 

	def _add_level(panel, sizer):
	    label=wx.StaticText(panel, -1, "谷歌地图层级, 取值为[ 0, 20]:")
	    self.txt_level = txt_level = wx.TextCtrl(panel, -1, "", size=(-1,-1))
	    box=wx.BoxSizer(wx.HORIZONTAL)
	    box.Add(label,0, wx.ALL|wx.ALIGN_LEFT|wx.ALIGN_BOTTOM, 5) 
	    box.Add(txt_level,0, wx.ALL|wx.ALIGN_LEFT|wx.ALIGN_BOTTOM, 5) 
	    sizer.Add(box,0, wx.ALL|wx.ALIGN_LEFT|wx.ALIGN_BOTTOM, 5) 
	_add_level(panel, sizerIn)

        line = wx.StaticLine(panel, -1, size=(_line_size,-1), style=wx.LI_HORIZONTAL)
        sizerIn.Add(line,0, wx.ALL|wx.ALIGN_LEFT|wx.ALIGN_BOTTOM, 5) 

	def _add_out_format(panel, sizer):
	    label=wx.StaticText(panel, -1, "结果文件类型, 取值为[tif,img]:")
	    self.out_format = out_format = wx.TextCtrl(panel, -1, "", size=(-1,-1))
	    box = wx.BoxSizer(wx.HORIZONTAL)
	    box.Add(label,0, wx.ALL|wx.ALIGN_LEFT|wx.ALIGN_BOTTOM, 5) 
	    box.Add(out_format,0, wx.ALL|wx.ALIGN_LEFT|wx.ALIGN_BOTTOM, 5) 
	    sizer.Add(box,0, wx.ALL|wx.ALIGN_LEFT|wx.ALIGN_BOTTOM, 5) 
	_add_out_format(panel, sizerIn)

	def _add_out_w_h(panel, sizer):
	    label=wx.StaticText(panel, -1, "结果文件像素宽度(256整数倍), 256x:")
	    self.txt_out_w = txt_out_w = wx.TextCtrl(panel, -1, "", size=(-1,-1))
	    box = wx.BoxSizer(wx.HORIZONTAL)
	    box.Add(label,0, wx.ALL|wx.ALIGN_LEFT|wx.ALIGN_BOTTOM, 5) 
	    box.Add(txt_out_w,0, wx.ALL|wx.ALIGN_LEFT|wx.ALIGN_BOTTOM, 5) 
	    sizer.Add(box,0, wx.ALL|wx.ALIGN_LEFT|wx.ALIGN_BOTTOM, 5) 

	    label=wx.StaticText(panel, -1, "结果文件像素高度(256整数倍), 256x:")
	    self.txt_out_h = txt_out_h = wx.TextCtrl(panel, -1, "", size=(-1,-1))
	    box = wx.BoxSizer(wx.HORIZONTAL)
	    box.Add(label,0, wx.ALL|wx.ALIGN_LEFT|wx.ALIGN_BOTTOM, 5) 
	    box.Add(txt_out_h,0, wx.ALL|wx.ALIGN_LEFT|wx.ALIGN_BOTTOM, 5) 
	    sizer.Add(box,0, wx.ALL|wx.ALIGN_LEFT|wx.ALIGN_BOTTOM, 5) 
	_add_out_w_h(panel, sizerIn)

        def _add_output_dir(panel, sizer):
            label=wx.StaticText(panel, -1, "输出结果位置目录:")
            self.out_dir=textOutPath= wx.TextCtrl(panel, -1, "", size=(590,-1))
            btn_dir = wx.Button(panel, wx.ID_ANY, "目录")
            box=wx.BoxSizer(wx.VERTICAL)
            box.Add(label,0, wx.ALL|wx.ALIGN_LEFT|wx.ALIGN_BOTTOM, 5) 
            _box=wx.BoxSizer(wx.HORIZONTAL)
            _box.Add(textOutPath,0, wx.ALL|wx.ALIGN_LEFT|wx.ALIGN_BOTTOM, 5) 
            _box.Add(btn_dir,0, wx.ALL|wx.ALIGN_LEFT|wx.ALIGN_BOTTOM, 5) 
            box.Add(_box,0, wx.ALL|wx.ALIGN_LEFT|wx.ALIGN_BOTTOM, 5) 
            cb = self.cb = wx.CheckBox(panel, -1, "是否覆盖已生成瓦片")#, (65, 60), (150, 20), wx.NO_BORDER)
            #cb.SetValue(True)
            box.Add(cb,0, wx.ALL|wx.ALIGN_LEFT|wx.ALIGN_BOTTOM, 5) 
            sizer.Add(box,0, wx.ALL|wx.ALIGN_LEFT|wx.ALIGN_BOTTOM, 5) 
            self.Bind(wx.EVT_BUTTON, self.OnButtonDir, btn_dir)
        _add_output_dir(panel, sizerIn)

        self.psizer = sizer = wx.BoxSizer(wx.VERTICAL)
        sizer.Add(sizerIn, 0, wx.EXPAND|wx.ALL, 25)
        line = wx.StaticLine(panel, -1, size=(_line_size,-1), style=wx.LI_HORIZONTAL)
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
        logger.info(APPNAME) 

        self.load_default_task()
        self.verify_license()
        self.Show()

    def verify_license(self):
        def _find_license_file(dir_name):
            filelist = os.listdir(dir_name)
            for fp in filelist:
                if fp.endswith(".lic"):
                    return os.path.join(dir_name, fp)
            return None

        dirname = cm.app_path()
        pn =  _find_license_file(dirname)
        if pn is None:
            return None

        pn = os.path.abspath(pn)
        if os.path.isfile(pn):
            lics = lic.License(pn)
            self.license = lics.verify(lics.hostName(), LICENSE_APP_NAME)

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
            return None

        bitmap = wx.Bitmap(pn, wx.BITMAP_TYPE_PNG)
        shadow = wx.WHITE
        frame = AS.AdvancedSplash(self, bitmap=bitmap, timeout=1500,
                                  agwStyle=AS.AS_TIMEOUT |
                                  AS.AS_CENTER_ON_PARENT |
                                  AS.AS_SHADOW_BITMAP,
                                  shadowcolour=shadow)

    def OnCloseMe(self, event):
        self.Destroy()
        sys.exit(1)

    def OnCloseWindow(self, event):
        self.Destroy()
        sys.exit(1)

    def OnButtonHelp(self, event):
        def _get_license(_lic=True):
            strlic = "免费试用版本."
            return strlic if not _lic else "授权版本 %s." % lic.License.hostName() 
        info = wx.AboutDialogInfo()
        info.Name = APPNAME 
        info.Version = version.__version__ 
        info.Copyright = version.__copyright__ + ("\n\n%s\n" % _get_license(self.license))

        strdes="将Google地图下载并转存为SuperMap缓存.\n\n"
        info.Description = wordwrap(strdes, 350, wx.ClientDC(self))
        info.WebSite = (version.__website__, info.Name)
        info.Developers = [ version.__author__ ]
        wx.AboutBox(info)

    def check(self):
        if self.txt_bbox.GetValue()=="":
            logger.info("输入路径为空.")
            return False
        if self.out_dir.GetValue()=="":
            logger.info("输出路径为空.")
            return False
        if self.out_format.GetValue()=="":
            logger.info("瓦片格式为空.")
            return False
        return True

    def OnButtonDir(self, event):
        dlg = wx.DirDialog(self, APPNAME, style=wx.DD_DEFAULT_STYLE)
        if dlg.ShowModal() == wx.ID_OK:
            self.out_dir.Clear()
            self.out_dir.AppendText(dlg.GetPath())
        dlg.Destroy()
        return True

    def OnButtonRun(self, event):
        if not self.check():
            return None

        tskpath = self.save_default_task()
	logger.info("开始转换...")
	g2f.google2file([tskpath]).run() 
        
        dlg = wx.MessageDialog(self, '转换完成^_^!', APPNAME, wx.OK|wx.ICON_INFORMATION)
        dlg.ShowModal()
        dlg.Destroy()

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

    def load_default_task(self):
	def _load_tsk(path):
	    _tsk = cfg.from_file(path, 'task')
	    if not _tsk:
		logger.error("文件内容错误, %s" % path)
		return None
	    return _tsk

        path = self.config_path()
        if os.path.isfile(path):
	    _tsk = _load_tsk(path) 

        if not _tsk:
            path = os.path.join(cm.app_path(), 'g.tsk')
	    _tsk = _load_tsk(path) 

        if 'bbox' in _tsk:
            self.txt_bbox.AppendText(_tsk['bbox'].strip())
        if 'level' in _tsk:
            self.txt_level.AppendText(_tsk['level'].strip())
        if 'out' in _tsk:
            self.out_dir.AppendText(_tsk['out'].strip())
        if 'format' in _tsk:
            self.out_format.AppendText(_tsk['format'].strip())
        if 'overwrite' in _tsk:
            self.cb.SetValue( True if _tsk['overwrite'].lower()=='true' else False)
	if 'width' in _tsk:
	    _w = int(_tsk['width'].strip())
	    _w = int(math.ceil(_w/256.0))
            self.txt_out_w.AppendText(str(_w))
	if 'height' in _tsk:
	    _h = int(_tsk['height'].strip())
	    _h = int(math.ceil(_h/256.0))
            self.txt_out_h.AppendText(str(_h))
    
    def save_default_task(self):
        """ 保存以便下次启动加载值 """
        _path = self.out_dir.GetValue()
        if not os.path.exists(_path):
            os.makedirs(_path)
        
        _path = os.path.join(_path, 'g.tsk') 
	def _write_gui_config(tsk_path):
	    _path = os.path.join(cm.app_path(), 'gui.cfg')
	    config = ConfigParser.ConfigParser()
	    config.read(_path)
	    config.set('path','path', tsk_path)
	    with open(_path, 'wb') as configfile:
		config.write(configfile)

	def _write_tsk_config(tsk_path):
	    _tsk = {}
	    _tsk['bbox'] = self.txt_bbox.GetValue()
	    _tsk['level'] = self.txt_level.GetValue()
	    _tsk['format'] = self.out_format.GetValue()
	    _tsk['out'] = self.out_dir.GetValue()
	    _tsk['overwrite'] = 'True' if self.cb.GetValue() else 'False'
	    _strw,_strh = self.txt_out_w.GetValue(), self.txt_out_h.GetValue()
	    _w, _h = 256*int(_strw), 256*int(_strh)
	    _tsk['width'] = str(_w)
	    _tsk['height'] = str(_h)
	    cfg.to_file(tsk_path, 'task', _tsk)
	
	_write_gui_config(_path)
	_write_tsk_config(_path)
	return _path

    def config_path(self):
        path = os.path.join(cm.app_path(), 'gui.cfg')
        config = ConfigParser.ConfigParser()
        config.read(path)
        path = config.get("path", "path")
        return path

#---------------------------------------------------------------------------
def main():
    app = wx.App(True)  # Create a new app, don't redirect stdout/stderr to a window.
    title = "%s v%s" % (APPNAME, version.__version__)
    app.SetOutputWindowAttributes(title,pos=wx.DefaultPosition,size=(580,220))
    frame = DownloadFrame(None, wx.ID_ANY, title) # A Frame is a top-level window.
    frame.Show(True)     # Show the frame.
    app.MainLoop()

#---------------------------------------------------------------------------
if __name__ == '__main__':
    mp.freeze_support()
    main()
