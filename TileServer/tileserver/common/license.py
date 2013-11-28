# --*-- coding:gbk --*--
import os, sys
import rsa
import argparse

# pub, priv = rsa.newkeys(512) 
n = 10449857015422111869994689290495734599294620868031159468330107986132955704778293082393535355189959482153707721248713281917366331660414312517080591900364657L
e = 65537
d = 2113506183368480291084114416978515374729545136422982723541138308988698411195516702109474430864370621143204620159898463988148949415944078505461421751450513L
p = 6964594242254963495679702082769522046045672759184826654329423936893002925517130429L
q = 49560818395920193444455023592828568369423L


class License(object):
    ''' 许可类 '''
    def __init__(self, lic=""):
        self.lic = lic
        self.host = ""
        self.app_name = ""
        self.licNums = 0
        self.decrypted = ""

    def setHost(self, hostName):
        self.host = hostName

    def setApp(self, app_name):
        self.app_name = app_name
    
    def create(self):
        msg = "hostname=%s;app_name=%s" % (self.host, self.app_name)
        pub_key = rsa.PublicKey(n,e)
        encrypted = rsa.encrypt(msg, pub_key)
        with open(self.lic, "wb") as licfile:
            licfile.write(encrypted)
            licfile.close()
        
    @staticmethod
    def hostName():
        if sys.platform == 'win32':
            hostname = os.getenv('computername')  
            return hostname  
        return "" 

    def verify(self, hostName, app_name):
        global n,e,d,p,q
        priv_key = rsa.PrivateKey(n,e,d,p,q)
        with open(self.lic, "rb") as licfile:
            msg = licfile.read()
            licfile.close()
            try:
                decrypted = rsa.decrypt(msg, priv_key)
                self.decrypted = decrypted 
                #print decrypted
            except rsa.DecryptionError as e:
                print e.read()
                return False
            cons = decrypted.split(";")
            for con in cons:
                lr = con.split("=")
                if len(lr) == 2:
                    l,r =lr[0].strip().lower(), lr[1].strip().lower()
                    if l=="hostname":
                        self.host=r
                    elif l=="app_name":
                        self.app_name=r
            return self.host==hostName.lower() and self.app_name==app_name 
        return False

def argparse_init():
    """ 解析命令行参数 """
    parser = argparse.ArgumentParser(description="生成许可工具.",
            epilog="Author: wenyu.lin@autonavi.com")
    parser.add_argument("app", default=sys.stdin,
            help="app name.")
    parser.add_argument("host", default=sys.stdin,
            help="host name.")
    args = parser.parse_args()
    return args

def creat_license(host, app_name):
    _path = "%s.lic" % host
    lics = License(_path)
    lics.setHost(host)
    lics.setApp(app_name)
    lics.create()

def verify_license(host, app_name):
    _path = "%s.lic" % host
    lics = License(_path)
    if lics.verify(host, app_name):
        print '加密成功,', lics.decrypted
    else:
        print '加密失败.'
    
if __name__=="__main__":
    args = argparse_init()
    host = args.host if args.host else License.hostName()
    creat_license(host, args.app)
    verify_license(host, args.app)
