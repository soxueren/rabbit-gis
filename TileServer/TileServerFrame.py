#!/usr/bin/env python
# --*-- coding:gbk --*--

import sys,os
import multiprocessing as mp
import wx
import time
import image2tile as i2t
import imageFile as imf
import smSci
import common as cm
from wx.lib.wordwrap import wordwrap
import license as lic

try:
    from agw import advancedsplash as AS
except ImportError: # if it's not there locally, try the wxPython lib.
    import wx.lib.agw.advancedsplash as AS

#---------------------------------------------------------------------------
def runMP(imgList, outPath, ext, bil, bboxs, q, pindex, license):
    ''' ����̴�����ͼ  '''
    #print startl, endl, outPath, ext
    ppid = os.getpid()
    imgtile = i2t.Image2Tiles(outPath, license) 
    imgtile.setExt(ext)

    #printLog(('��ʼ�����%d������...' % i))
    #msg = ("���ڴ����%d������, ��[%d,%d]��..." % (i,startl, endl))
    imgtile.toTilesByBoxs(imgList, bboxs, outPath, bil, True)
        
    msg = "�ӽ���%d, id:%d, " % (pindex, ppid)
    logs=[]
    for log in imgtile.logs:
        logs.append(msg+log)
    q.put(logs)
    del imgtile 


#---------------------------------------------------------------------------

g_wildcard = "GeoTiff (*.tif)|*.tif|"     \
           "GeoTiff (*.tiff)|*.tiff|" \
           "Erdas Image File (*.img)|*.img" \

class TileServerFrame(wx.Frame):
    def __init__(
            self, parent, ID, title, pos=wx.DefaultPosition,
            size=(1024,600), style=wx.RESIZE_BORDER|wx.SYSTEM_MENU|wx.CAPTION|wx.CLOSE_BOX|wx.CLIP_CHILDREN):

        wx.Frame.__init__(self, parent, ID, title, pos, size, style)
        self.uiSplash()
        self.SetIcon(wx.Icon('icon.ico', wx.BITMAP_TYPE_ICO))
        self.fileList=[]
        self.license = False # �Ƿ�ͨ�������֤
        self.appid = 1 
        self.lock = mp.RLock()
        self.panel=panel= wx.Panel(self, -1)

        inBox = wx.StaticBox(panel, -1, "")
        sizerIn = wx.StaticBoxSizer(inBox, wx.VERTICAL)

        self.txtIn=txtin = wx.TextCtrl(panel, -1, "", size=(540,-1),style=wx.TE_READONLY)
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
        self.txtOut=textOutPath= wx.TextCtrl(panel, -1, "", size=(540,-1))
        btnOutPath = wx.Button(panel, wx.ID_ANY, "Ŀ¼")
        box=wx.BoxSizer(wx.HORIZONTAL)
        box.Add(label,0, wx.ALL|wx.ALIGN_LEFT|wx.ALIGN_BOTTOM, 5) 
        box.Add(textOutPath,0, wx.ALL|wx.ALIGN_LEFT|wx.ALIGN_BOTTOM, 5) 
        box.Add(btnOutPath,0, wx.ALL|wx.ALIGN_LEFT|wx.ALIGN_BOTTOM, 5) 
        sizerIn.Add(box,0, wx.ALL|wx.ALIGN_LEFT|wx.ALIGN_BOTTOM, 5) 

        line = wx.StaticLine(panel, -1, size=(750,-1), style=wx.LI_HORIZONTAL)
        sizerIn.Add(line,0, wx.ALL|wx.ALIGN_LEFT|wx.ALIGN_BOTTOM, 5) 

        button=wx.Button(panel, -1, "����", size=(-1,60))
        self.Bind(wx.EVT_BUTTON, self.OnButtonRun, button)

        box=wx.BoxSizer(wx.HORIZONTAL)
        self.uiCacheName(box)
        self.uiTileType(box)
        box.Add(boxSiezer,0, wx.ALL|wx.ALIGN_LEFT|wx.ALIGN_BOTTOM, 5) 
        box.Add(button,0, wx.ALL|wx.ALIGN_LEFT|wx.ALIGN_BOTTOM, 5) 
        sizerIn.Add(box,0, wx.ALL|wx.ALIGN_LEFT|wx.ALIGN_BOTTOM, 5) 

        
        self.log=logText = wx.TextCtrl(panel,-1,"",size=(750,220),style=wx.TE_READONLY|wx.TE_RICH|wx.TE_MULTILINE|wx.EXPAND)
        logBox = wx.StaticBox(panel, -1, "��־:")
        sizerLog = wx.StaticBoxSizer(logBox, wx.HORIZONTAL)
        sizerLog.Add(logText,0, wx.ALL|wx.EXPAND, 5) 

        self.psizer=sizer = wx.BoxSizer(wx.VERTICAL)
        sizer.Add(sizerIn, 0, wx.EXPAND|wx.ALL, 25)
        sizer.Add(sizerLog, 0, wx.EXPAND|wx.LEFT|wx.RIGHT, 25)
        line = wx.StaticLine(panel, -1, size=(750,-1), style=wx.LI_HORIZONTAL)
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

    def verifyLicense(self):
        try:
            dirName = os.path.dirname(os.path.abspath(__file__))
        except:
            dirName = os.path.dirname(os.path.abspath(sys.argv[0]))
        # �����Ŀ¼�����仯,��Ҫȥ��library.zipĿ¼
        dirName = dirName.replace('library.zip','')
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

    def uiTileType(self, sizer):
        pass

    def uiCacheName(self, sizer):
        pass

    
    def uiButtonOK(self, sizer):
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

        btn = wx.Button(self.panel, wx.ID_HELP, "����")
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
        # �����Ŀ¼�����仯,��Ҫȥ��library.zipĿ¼
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

    def GetLicense(self):
        strlic = "������ð汾."
        if self.license:
            strlic = "��Ȩ�汾 %s." % lic.License.hostName()
        return strlic

    def OnButtonHelp(self, event):
        # First we create and fill the info object
        info = wx.AboutDialogInfo()
        info.Name = cm.APPTITLE+"-"+cm.APPNAME 
        info.Version = cm.VERSION 
        info.Copyright = "(C) 2013 www.atolin.net ��������Ȩ��.\n\n%s\n" % self.GetLicense()
        strdes="������ά���λ��湤��.\n\n�Զ�ƴ��,���������������ص�.\n\n"#.decode('gb2312')
        strdes+="��ֱ�ӽ�Ӱ���зֳ���ά���λ����ļ�.\n\n"#.decode('gb2312')
        info.Description = wordwrap(info.Name+strdes, 350, wx.ClientDC(self))
        info.WebSite = ("http://www.atolin.net", info.Name)
        info.Developers = [ "wenyulin.lin@gmail.com","qq:42848918" ]

        #info.License = wordwrap(self.GetLicense(), 500, wx.ClientDC(self))
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
            if ifile.isGeographic() or ifile.isInGeographic() or ifile.canbeGeographic():
                wgsList.append(f)
            else:
                wkt_srs=ifile.getProjection()
                self.printLog(('��WGS84����: %s' % f))
                if wkt_srs!="":
                    self.printLog(wkt_srs)
        self.fileList=wgsList

    def defaultLevels(self):
        ''' �����������ݵ�Ĭ����ʼ��ֹ�㼶  '''
        if len(self.fileList)==0: return

        l,t,r,b, xres, yres = imf.calcGeographicBoundary(self.fileList)
        mapbnd=l,t,r,b
        endl=smSci.smSci3d.calcEndLevel(xres)
        startl=smSci.smSci3d.calcStartLevel(l,t,r,b,xres,endl)
        self.spinLvlBeg.SetRange(startl, endl)
        self.spinLvlEnd.SetRange(startl, endl)
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
        self.lock.acquire()
        strtime=time.strftime("%Y-%m-%d %H:%M:%S> ")
        strlog=strtime+msg
        if newline: 
            self.log.AppendText(strlog+"\n")
        else:
            self.log.AppendText(strlog+"\r")
        self.log.Refresh()
        self.lock.release()

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

    def runSingleProcess(self, startl, endl, outPath, ext, bil=False):
        ''' ������ģʽ��ͼ '''
        imgtile = i2t.Image2Tiles(outPath, self.license) 
        imgtile.hook(self.printLog)
        imgtile.setExt(ext)

        maxstep=endl-startl+2
        dlg = self.createProgressDialog("���ɻ���", "���ɻ���", maxstep)
        keepGoing = True
        
        self.printLine("Start")
        for i in xrange(startl, endl+1):
            self.printLog(('��ʼ�����%d������...' % i))
            msg = ("���ڴ����%d������, ��[%d,%d]��..." % (i,startl, endl))
            (keepGoing, skip) = dlg.Update(i-startl+1, msg)
            imgtile.toTiles(self.fileList, i, outPath, bil)
            if i==endl: 
                dlg.Destroy()
            
        self.printLine("End, All done.")
        del imgtile 
        dlg.Destroy()

    def splitBox(self, l,t,r,b, startl, endl, mpcnt):
        ''' ���ݽ�����Ŀ,��Ƭ�������ֺ�������� '''
        tasks = []
        for i in range(startl, endl+1):
            rs,re,cs,ce=smSci.smSci3d.calcRowCol(l,t,r,b, i) 
            for row in range(rs, re+1):
                for col in range(cs, ce+1):
                    tasks.append((i, row, col))

        totalNums = len(tasks)
        splitNums = totalNums / mpcnt
        mplist = []
        add = 0
        for i in range(mpcnt):
            mplist.append( tasks[i*splitNums:(i+1)*splitNums] )
            add += splitNums

        if add<totalNums:
            mplist[-1].extend( tasks[add-totalNums:] )
        return mplist
        
    def runMultiProcess(self, startl, endl, outPath, ext, bil=False, mpcnt=4):
        ''' �����ģʽ��ͼ '''
        l,t,r,b, xres, yres=imf.calcGeographicBoundary(self.fileList)
        boxList = self.splitBox(l,t,r,b,startl,endl,mpcnt)
        assert(len(boxList)==mpcnt)

        self.printLine("Start")
        totalLevel = endl-startl+1
        plist = []
        m = mp.Manager()
        q = m.Queue()
        for i in xrange(mpcnt):
            bboxs = boxList[i]
            p = mp.Process(target=runMP, args=(self.fileList, outPath, ext, bil, bboxs, q, i+1, self.license))
            plist.append(p)

        for p in plist:
            p.start()

        for p in plist:
            p.join()
            logs = list() if q.empty() else q.get()
            for log in logs:
                self.printLog(log)
        
        '''
        for p in plist:
            print p.is_alive(), p.exitcode
        '''

        self.printLine("End, All done.")

#---------------------------------------------------------------------------
def unitTest():
    app = wx.App(False)  # Create a new app, don't redirect stdout/stderr to a window.
    frame = TileServerFrame(None, wx.ID_ANY, cm.APPNAME+"-Sct-������ά���λ���-"+cm.VERSION) # A Frame is a top-level window.
    data = r'E:\�½��ļ���\ȫ�������Ӱ��\�½��ļ���'
    fName = "\\1-1.tif"
    frame.txtOut.AppendText(data)
    frame.txtIn.AppendText(data+fName)
    frame.fillFileList()
    l,t,r,b, xres, yres = imf.calcGeographicBoundary(frame.fileList)
    endl=smSci.smSci3d.calcEndLevel(xres)
    boxlist = frame.splitBox(l,t,r,b, 8,14, 4)
    frame.runMultiProcess(8, 14, "", "", bil=False, mpcnt=4)
#---------------------------------------------------------------------------
def main():
    app = wx.App(False)  # Create a new app, don't redirect stdout/stderr to a window.
    frame = TileServerFrame(None, wx.ID_ANY, cm.APPNAME+"-Sct-������ά���λ���-"+cm.VERSION) # A Frame is a top-level window.
    frame.Show(True)     # Show the frame.
    app.MainLoop()


if __name__ == '__main__':
    #main()
    unitTest()
