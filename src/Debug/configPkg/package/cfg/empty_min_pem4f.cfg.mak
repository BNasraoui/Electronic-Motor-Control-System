# invoke SourceDir generated makefile for empty_min.pem4f
empty_min.pem4f: .libraries,empty_min.pem4f
.libraries,empty_min.pem4f: package/cfg/empty_min_pem4f.xdl
	$(MAKE) -f C:\Users\jesse\Dropbox\Sem1_2021\Embedded\CCS_Workspace\EGH456_GROUP6\src/src/makefile.libs

clean::
	$(MAKE) -f C:\Users\jesse\Dropbox\Sem1_2021\Embedded\CCS_Workspace\EGH456_GROUP6\src/src/makefile.libs clean

