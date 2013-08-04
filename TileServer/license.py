# --*-- coding:utf-8 --*--
import os, sys
import rsa
import common as cm

# pub, priv = rsa.newkeys(512) 
n = 10449857015422111869994689290495734599294620868031159468330107986132955704778293082393535355189959482153707721248713281917366331660414312517080591900364657L
e = 65537
d = 2113506183368480291084114416978515374729545136422982723541138308988698411195516702109474430864370621143204620159898463988148949415944078505461421751450513L
p = 6964594242254963495679702082769522046045672759184826654329423936893002925517130429L
q = 49560818395920193444455023592828568369423L


class License(object):
    ''' –Ìø…¿‡ '''
    def __init__(self, lic=""):
	self.lic = lic
	self.host = ""
	self.licNums = 0
	self.decrypted = ""

    def setHost(self, hostName):
	self.host = hostName

    def addApp(self, appid):
	appids = (cm.APPID_SCI3D, cm.APPID_SCT)
	if appid in appids:
	    self.licNums |= appid
    
    def create(self):
	msg = "hostname=%s;apps=%d" % (self.host, self.licNums)
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

    def verify(self, hostName, appid):
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
		    l=lr[0].strip().lower()
		    r=lr[1].strip().lower()
		    if l=="hostname":
			self.host=r
		    elif l=="apps":
			if r.isdigit():
			    self.licNums=int(r)
	    return self.host==hostName.lower() and self.licNums&appid 
	return False

def unitLicense(host):
    #host = License.hostName()
    licFile = "%s.lic" % host
    lics = License(licFile)
    lics.setHost(host)
    lics.addApp(cm.APPID_SCI3D)
    lics.addApp(cm.APPID_SCT)
    lics.addApp(cm.APPID_GOOGLE_SIC3D)
    lics.create()

def unitVerify(host):
    licFile = "%s.lic" % host
    lics = License(licFile)
    host = License.hostName()
    appid = cm.APPID_SCI3D
    lics.verify(host, appid)
    print 'encrypted is:',lics.decrypted
    
if __name__=="__main__":
    host = License.hostName()
    if len(sys.argv)==2:
	host = sys.argv[1]
    print 'host name is:', host
    unitLicense(host)
    unitVerify(host)
