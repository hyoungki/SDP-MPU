#
# Project dependent make variables
#
# PRODUCT(S)  : KR SDP(Smart Digital Processor) Development
#
# MODIFICATION LOG :
#   Date      Who    Rev                       Comments
# ---------- ------  ----  -------------------------------------------
# 2011.11.07 ChoiBC   01   create
# 2015.12.10 ChoiBC   03   copy from pmu
#
#	TARGET_PLATFORM : LINUX, PPC_SAEIL
#
#TARGET_PLATFORM = LINUX
#TARGET_PLATFORM = PPC_SAEIL
TARGET_PLATFORM = PPC_SANE



# 2026-05-18 ���� 5:19:24 
# select ESIO_ARCH   arm or pppc

# ARM_ARCH = 1  2026.08. 24 replace below code (by claude)
  

ifeq ($(origin ARM_ARCH), undefined)
  ifneq ($(findstring arm-,$(CC)),)
    ARM_ARCH = 1
  else
    ARM_ARCH = 0
  endif
endif


ifeq ($(ARM_ARCH), 1)	
FLAGS_APP = -g  -D_REENTRANT -Wall $(PROJ_CFLAGS) -D__ARM_ARCH__
else	
FLAGS_APP = -g  -D_REENTRANT -Wall $(PROJ_CFLAGS) -D__PPC_ARCH__
endif 









#	-D_INTEL_TARGET : for linux
ifeq ($(TARGET_PLATFORM), LINUX)
PROJ_CFLAGS = -D_INTEL_TARGET -D_PLATFORM_LINUX
else
PROJ_CFLAGS = -D_PLATFORM_PPC
endif

#	-D_SDP : SDP Project, if not defined then Editor or Monitor 
PROJ_CFLAGS += -D_SDP 

#
# Environment
#
ifeq ($(TARGET_PLATFORM), PPC_SANE)
PLATFORM      = PPC
# hkkim add below two lines
#NFS_HOME      = /home/freescale/mpu_home
#LOCAL_HOME    = /media/sf_shared-SDP/ACE-SDP-MPU-V3

endif
ifeq ($(TARGET_PLATFORM), PPC_SAEIL)
PLATFORM      = PPC
NFS_HOME      =  /home/freescale/mpu_home
LOCAL_HOME    = /media/sf_Shared/ACE-SDP-MPU-V3
endif
ifeq ($(TARGET_PLATFORM), LINUX)
PLATFORM      = LINUX
NFS_HOME      = /home/freescale/bin
LOCAL_HOME    = /media/sf_Shared/ACE-SDP-MPU-V3
endif



#
# Variables
#
MAKE        = make
CD          = cd
RM          = rm
MV          = mv
CP          = cp
#CC          = $(CROSS_COMPILE)gcc
#LD          = $(CROSS_COMPILE)ld
#AR          = $(CROSS_COMPILE)ar
ARFLAG 	    = rcv
#RANLIB      = $(CROSS_COMPILE)ranlib
#STRIP       = $(CROSS_COMPILE)strip
#OBJCOPY     = $(CROSS_COMPILE)objcopy
#OBJDUMP     = $(CROSS_COMPILE)objdump
CTAGS       = ctags
FIND        = find

#
# User
#
USER_INC = $(LOCAL_HOME)/inc
USER_LIB = $(LOCAL_HOME)/lib
USER_BIN = $(LOCAL_HOME)/bin

INCLUDEDIRS += -I. -I$(USER_INC)

#
# Static or Shared Library
#
#LIB_EXTENSION = so
LIB_EXTENSION = a

#
# libLocal
#
LIBLOCALNAME = libLocal

# files that deleted by make clean
TEMP_FILES = *.bak tags core

#
# Application
#
#FLAGS_APP = $(PROJ_CFLAGS) -O2 -D_REENTRANT -Wall -Werror -D_CHECK_LOGS


#
# 61850 Library
#
# DFLAG
# 		_l  : logging    / no debug
# 		_n  : no logging / no debug
# 		_ld : logging    / debug
# 		_nd : no logging / debug
#
ifeq ($(PLATFORM), PPC)
DFLAG=_l
DEFS=-DDEBUG_SISCO
else
DFLAG=_ld
DEFS=-DDEBUG_SISCO
endif

#
# IEC61850
#
MMSLITEDIR = ../mmslite
MMSLITEINCDIR = $(MMSLITEDIR)/inc
MMSLITELIBDIR = ../mmslite.lib

MMSLITELIBS = $(MMSLITELIBDIR)/mvlu$(DFLAG).a\
			  			$(MMSLITELIBDIR)/mlogl$(DFLAG).a\
			  			$(MMSLITELIBDIR)/mmsle$(DFLAG).a\
			  			$(MMSLITELIBDIR)/mmsl$(DFLAG).a\
			  			$(MMSLITELIBDIR)/asn1l$(DFLAG).a\
			  			$(MMSLITELIBDIR)/mem$(DFLAG).a\
			  			$(MMSLITELIBDIR)/slog$(DFLAG).a\
			  			$(MMSLITELIBDIR)/util$(DFLAG).a \
			  			$(MMSLITELIBDIR)/ositcps$(DFLAG).a

LIBS_MMSLITE  = -lpthread -lrt

#FLAGS_MMSLITE = -fsigned-char -D_GNU_SOURCE -D_THREAD_SAFE -pthread -m32
FLAGS_MMSLITE  = -D_GNU_SOURCE -D_THREAD_SAFE -pthread -m32
FLAGS_MMSLITE += $(DEFS)
FLAGS_MMSLITE += -DMMS_LITE -DMOSI -DLEAN_T -DMLOG_ENABLE -DUSE_DIB_LIST -DMVL_UCA 

#
# 61850 lib
#
DIRlib61850 = $(LOCAL_HOME)/61850Lib
LIBNAME61850  = lib61850


#
# ICCP
#
DIRiccp = $(UTARGETDIR)/bin/ICCP_LIB/$(PLATFORM)
INC_ICCP = -I$(PROJ_SRC)/mmslite.iccp/inc
LIB_ICCP =	$(DIRiccp)/mi_plus$(DFLAG).a \
			$(DIRiccp)/mvl$(DFLAG).a \
			$(DIRiccp)/mlog$(DFLAG).a \
			$(DIRiccp)/mmsle$(DFLAG).a \
			$(DIRiccp)/mmsl$(DFLAG).a \
			$(DIRiccp)/asn1l$(DFLAG).a \
			$(DIRiccp)/mem$(DFLAG).a \
			$(DIRiccp)/slog$(DFLAG).a \
			$(DIRiccp)/util$(DFLAG).a \
			$(DIRiccp)/ositcps$(DFLAG).a \
			$(DIRiccp)/stackcfg$(DFLAG).a \
			$(PLATFORM_LIBS)

FLAGS_ICCP = $(PLATFORM_CFLAGS) $(DEFS)\
			-DMMS_LITE -DMOSI -DLEAN_T -DETHERNET -DICCP_LITE -DMLOG_ENABLE \
			-DSISCO_STACK_CFG \
			-DMUI_REDUNDANCY_SUPPORT

#
# Mini-Xml Library
#
#DIRmxml = $(PROJ_SRC)/mxml
#LIB_MXML = $(DIRmxml)/libmxml.$(PLATFORM).a
ifeq ($(PLATFORM), PPC)
LIB_MXML = -lmxml.PPC
else
LIB_MXML = -lmxml
endif
# END
