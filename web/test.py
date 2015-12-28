import os

f=open("config.txt")
fl=f.readlines()
for i in range(0,10000):
	for file in fl:
		cmd = "./web.exe  5 %s 192.1.3.55 8769 \"http://123.57.26.24:8080/saveData/airmessage/messMgr.do\" 9" % (file[0:7])
		os.system(cmd)
		cmd = "./web.exe  6 %s 192.1.3.55 8769 \"http://123.57.26.24:8080/saveData/airmessage/messMgr.do\" 9" % (file[0:7])
		os.system(cmd)
		cmd = "./web.exe  2 %s 192.1.3.55 8769 60 132.6 90 92 \"2015-12-02%%2009:02\" \"http://123.57.26.24:8080/saveData/airmessage/messMgr.do\" 9" % (file[0:7])
		os.system(cmd)
		cmd = "./web.exe  4 %s 192.168.1.123 1234 \"http://123.57.26.24:8080/saveData/airmessage/messMgr.do\" 9 92 \"2th sensor possible error\"" % (file[0:7])
		os.system(cmd)