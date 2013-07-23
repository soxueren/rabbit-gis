import os, sys
import rsa

# pub, priv = rsa.newkeys(512) 
n = 10449857015422111869994689290495734599294620868031159468330107986132955704778293082393535355189959482153707721248713281917366331660414312517080591900364657L
e = 65537 
d = 2113506183368480291084114416978515374729545136422982723541138308988698411195516702109474430864370621143204620159898463988148949415944078505461421751450513L
p = 6964594242254963495679702082769522046045672759184826654329423936893002925517130429L
q = 49560818395920193444455023592828568369423L

def createLicense(msg, lic):
    pub_key = rsa.PublicKey(n,e)
    encrypted = rsa.encrypt(msg, pub_key)
    print len(encrypted)
    with open(lic, "wb") as licfile:
	licfile.write(encrypted)
	licfile.close()

def hostName():
    if sys.platform == 'win32':
	hostname = os.getenv('computername')  
	return hostname  
    return "" 

def verify(lic, hostName, appId):
    priv_key = rsa.PrivateKey(n,e,d,p,q)
    with open(lic, "rb") as licfile:
	msg = licfile.read()
	licfile.close()
	try:
	    decrypted = rsa.decrypt(msg, priv_key)
	except rsa.DecryptionError as e:
	    print e.read()
	    print len(decrypted), decrypted 
	return True
    return False
    
if __name__=="__main__":
    '''
    msg = "mac=test;"
    lic = "license.lic"
    create_license(msg, lic)
    verify(lic)
    '''
    print hostName()
