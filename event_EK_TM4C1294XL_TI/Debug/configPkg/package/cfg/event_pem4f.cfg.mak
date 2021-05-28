# invoke SourceDir generated makefile for event.pem4f
event.pem4f: .libraries,event.pem4f
.libraries,event.pem4f: package/cfg/event_pem4f.xdl
	$(MAKE) -f C:\Users\Ben\DOCUME~1\UNISTU~1\EGH456\Project\event_EK_TM4C1294XL_TI/src/makefile.libs

clean::
	$(MAKE) -f C:\Users\Ben\DOCUME~1\UNISTU~1\EGH456\Project\event_EK_TM4C1294XL_TI/src/makefile.libs clean

