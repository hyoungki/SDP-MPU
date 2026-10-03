#
# Makefile for all task
#
# 2026-02-13 오후 3:53:21  CLI-ReadLine 추가


include common.make

# hkkim add below two lines
LOCAL_HOME = $(shell pwd)
NFS_HOME = $(shell pwd)/NFS





export LOCAL_HOME NFS_HOME  
  
.PHONY : lib WDT CLI SCAN HOST LINK SCU SIM ICCP EXIT  LIB  CLI_S2


all:
	@if [ -d lib ]; then $(CD) lib; $(MAKE) all; fi
	@if [ -d WDT ]; then $(CD) WDT; $(MAKE) all; fi
	@if [ -d CLI-ReadLine ]; then $(CD) CLI-ReadLine; $(MAKE) all; fi
	@if [ -d SCAN ]; then $(CD) SCAN; $(MAKE) all; fi
	@if [ -d HOST ]; then $(CD) HOST; $(MAKE) all; fi
	@if [ -d LINK ]; then $(CD) LINK; $(MAKE) all; fi
	@if [ -d SCU ]; then $(CD) SCU; $(MAKE) all; fi
	@if [ -d SIM ]; then $(CD) SIM; $(MAKE) all; fi
	@if [ -d ICCP ]; then $(CD) ICCP; $(MAKE) all; fi
	@if [ -d EXIT ]; then $(CD) EXIT; $(MAKE) all; fi
	
install:
	@if [ -d lib ]; then $(CD) lib; $(MAKE) install; fi
	@if [ -d WDT ]; then $(CD) WDT; $(MAKE) install; fi
	@if [ -d CLI-ReadLine ]; then $(CD) CLI-ReadLine; $(MAKE) install; fi
	@if [ -d SCAN ]; then $(CD) SCAN; $(MAKE) install; fi
	@if [ -d HOST ]; then $(CD) HOST; $(MAKE) install; fi
	@if [ -d LINK ]; then $(CD) LINK; $(MAKE) install; fi
	@if [ -d SCU ]; then $(CD) SCU; $(MAKE) install; fi
	@if [ -d SIM ]; then $(CD) SIM; $(MAKE) install; fi
	@if [ -d ICCP ]; then $(CD) ICCP; $(MAKE) install; fi

uninstall:
	@if [ -d lib ]; then $(CD) lib; $(MAKE) uninstall; fi
	@if [ -d WDT ]; then $(CD) WDT; $(MAKE) uninstall; fi
	@if [ -d CLI-ReadLine ]; then $(CD) CLI-ReadLine; $(MAKE) uninstall; fi
	@if [ -d SCAN ]; then $(CD) SCAN; $(MAKE) uninstall; fi
	@if [ -d HOST ]; then $(CD) HOST; $(MAKE) uninstall; fi
	@if [ -d LINK ]; then $(CD) LINK; $(MAKE) uninstall; fi
	@if [ -d SCU ]; then $(CD) SCU; $(MAKE) uninstall; fi
	@if [ -d SIM ]; then $(CD) SIM; $(MAKE) uninstall; fi
	@if [ -d ICCP ]; then $(CD) ICCP; $(MAKE) uninstall; fi

#clean:
#	@if [ -d lib ]; then $(CD) lib; $(MAKE) clean; fi
#	@if [ -d WDT ]; then $(CD) WDT; $(MAKE) clean; fi
#	@if [ -d CLI-ReadLine ]; then $(CD) CLI-ReadLine; $(MAKE) clean; fi
#	@if [ -d SCAN ]; then $(CD) SCAN; $(MAKE) clean; fi
#	@if [ -d HOST ]; then $(CD) HOST; $(MAKE) clean; fi
#	@if [ -d LINK ]; then $(CD) LINK; $(MAKE) clean; fi
#	@if [ -d SCU ]; then $(CD) SCU; $(MAKE) clean; fi
#	@if [ -d SIM ]; then $(CD) SIM; $(MAKE) clean; fi
#	@if [ -d ICCP ]; then $(CD) ICCP; $(MAKE) clean; fi

clean:
	@echo "clean subdir" 
	for i in lib WDT CLI-ReadLine SCAN HOST LINK SCU SIM ICCP EXIT ; do \
		cd $$i ; \
		$(MAKE) clean;\
		cd ..;\
	 done


tags:
	@if [ -d lib ]; then $(CD) lib; $(MAKE) tags; fi
	@if [ -d WDT ]; then $(CD) WDT; $(MAKE) tags; fi
	@if [ -d CLI-ReadLine ]; then $(CD) CLI-ReadLine; $(MAKE) tags; fi
	@if [ -d SCAN ]; then $(CD) SCAN; $(MAKE) tags; fi
	@if [ -d HOST ]; then $(CD) HOST; $(MAKE) tags; fi
	@if [ -d LINK ]; then $(CD) LINK; $(MAKE) tags; fi
	@if [ -d SCU ]; then $(CD) SCU; $(MAKE) tags; fi
	@if [ -d SIM ]; then $(CD) SIM; $(MAKE) tags; fi
	@if [ -d ICCP ]; then $(CD) ICCP; $(MAKE) tags; fi


depend:
	@if [ -d lib ]; then $(CD) lib; $(MAKE) depend; fi
	@if [ -d WDT ]; then $(CD) WDT; $(MAKE) depend; fi
	@if [ -d CLI-ReadLine ]; then $(CD) CLI-ReadLine; $(MAKE) depend; fi
	@if [ -d SCAN ]; then $(CD) SCAN; $(MAKE) depend; fi
	@if [ -d HOST ]; then $(CD) HOST; $(MAKE) depend; fi
	@if [ -d LINK ]; then $(CD) LINK; $(MAKE) depend; fi
	@if [ -d SCU ]; then $(CD) SCU; $(MAKE) depend; fi
	@if [ -d SIM ]; then $(CD) SIM; $(MAKE) depend; fi
	@if [ -d ICCP ]; then $(CD) ICCP; $(MAKE) depend; fi

LIB :
	$(CD) lib; $(MAKE) 


CLI_S2 :
	$(CD) CLI-S2; $(MAKE) 

CLI :
	$(CD) CLI-ReadLine; $(MAKE) 

EXIT :
	$(CD) EXIT; $(MAKE) 
	
CEXIT :
	$(CD) EXIT; $(MAKE) clean

CCLI :
	$(CD) CLI-ReadLine; $(MAKE) clean

ICCP :
	$(CD) ICCP; $(MAKE) clean
	$(CD) ICCP; $(MAKE) all


SCAN :
	$(CD) SCAN; $(MAKE) 


CSCAN :
	$(CD) SCAN; $(MAKE) clean


lib :
	$(CD) lib; $(MAKE) 



WDT : lib
	$(CD) WDT; $(MAKE) 


HOST  : lib
	$(CD) HOST; $(MAKE) clean  ; $(MAKE) 




