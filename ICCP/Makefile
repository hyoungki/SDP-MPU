#
# Makefile for ICCP
#

# Include common and platform-dependent definitions
include ../common.make

TARGET = ICCP
TARGET_INSTALL = $(NFS_HOME)/$(TARGET)

# ICCP stack
MMSLITEDIR = ../mmslite.iccp
MMSLITEINCDIR=$(MMSLITEDIR)/inc
MMSLITELIBDIR=../mmslite.iccp.lib

# FLAGS_MMSLITE에서  -m32 제거
FLAGS_MMSLITE = -fsigned-char -D_GNU_SOURCE -D_THREAD_SAFE -pthread 
#FLAGS_MMSLITE = -fsigned-char -D_GNU_SOURCE -D_THREAD_SAFE -pthread -m32
FLAGS_MMSLITE += -DDEBUG_SISCO -DMMS_LITE -DMOSI -DLEAN_T \
								-DETHERNET -DICCP_LITE -DMLOG_ENABLE \
								-DSISCO_STACK_CFG
LIBS_MMSLITE  = -lpthread -lrt
# DFLAG
# 		_l  : logging    / no debug
# 		_n  : no logging / no debug
# 		_l : logging    / debug
# 		_nd : no logging / debug
ifeq ($(PLATFORM), PPC)
DFLAG=_l
else
DFLAG=_ld
endif
MMSLITELIBS = $(MMSLITELIBDIR)/mi_plus$(DFLAG).a \
							$(MMSLITELIBDIR)/mvl$(DFLAG).a \
							$(MMSLITELIBDIR)/mlog$(DFLAG).a \
							$(MMSLITELIBDIR)/mmsle$(DFLAG).a \
							$(MMSLITELIBDIR)/mmsl$(DFLAG).a \
							$(MMSLITELIBDIR)/asn1l$(DFLAG).a \
							$(MMSLITELIBDIR)/mem$(DFLAG).a \
							$(MMSLITELIBDIR)/slog$(DFLAG).a \
							$(MMSLITELIBDIR)/util$(DFLAG).a \
							$(MMSLITELIBDIR)/ositcps$(DFLAG).a \
							$(MMSLITELIBDIR)/stackcfg$(DFLAG).a

# Define directories here
SRCDIR     = .
OBJDIR     = .
MMSOPDIR   = .
MMSSRCDIR  = $(MMSLITEDIR)/src
ICCPSRCDIR = $(MMSLITEDIR)/iccp/src
ICCPUSRDIR = .
MVLSRCDIR  = $(MMSLITEDIR)/mvl/src
MVLACSEDIR = $(MMSLITEDIR)/mvl/src/acse
MVLUSRDIR  = $(MMSLITEDIR)/mvl/usr

#
VPATH = $(SRCDIR) $(MVLSRCDIR) $(MVLUSRDIR) $(MMSSRCDIR) $(MMSLITELIBDIR) \
	$(OBJDIR) $(MMSLITEINCDIR) $(ICCPSRCDIR) $(ICCPUSRDIR) $(MVLACSEDIR)

#
INCDIRS = $(INCLUDEDIRS) -I$(MMSLITEINCDIR)
CFLAGS = $(FLAGS_APP) $(FLAGS_MMSLITE)
LFLAGS =

LIBS     = $(MMSLITELIBS) $(LIBS_MMSLITE) \
					 $(USER_LIB)/$(LIBLOCALNAME).$(LIB_EXTENSION) -lm
LIBRARYS = $(MMSLITELIBS) $(USER_LIB)/$(LIBLOCALNAME).$(LIB_EXTENSION) 

# Define objects used in the executable
OBJS = 	$(OBJDIR)/iccpMain.o \
				$(OBJDIR)/event2.o \
				$(OBJDIR)/mi_main.o \
				$(OBJDIR)/mi_wrcfg.o \
				$(OBJDIR)/mic_usr.o \
				$(OBJDIR)/mics_cfg.o \
				$(OBJDIR)/mis_usr.o \
				$(OBJDIR)/miu_icfg.o \
				$(OBJDIR)/miusrobj.o \
				$(OBJDIR)/mmsop_en.o \
				$(OBJDIR)/mvlop_en.o \
				$(OBJDIR)/mvl_acse.o \
				$(OBJDIR)/logcfgx.o \
				$(OBJDIR)/slogipc.o \
				$(OBJDIR)/UserUtil.o
#SRCS = $(OBJS:.o=.c)
SRCS = 	$(SRCDIR)/iccpMain.c \
				$(MVLUSRDIR)/event2.c \
				$(SRCDIR)/mi_main.c \
				$(ICCPSRCDIR)/mi_wrcfg.c \
				$(SRCDIR)/mic_usr.c \
				$(SRCDIR)/mics_cfg.c \
				$(SRCDIR)/mis_usr.c \
				$(SRCDIR)/miu_icfg.c \
				$(SRCDIR)/miusrobj.c \
				$(MMSSRCDIR)/mmsop_en.c \
				$(MVLSRCDIR)/mvlop_en.c \
				$(MVLACSEDIR)/mvl_acse.c \
				$(MVLUSRDIR)/logcfgx.c \
				$(MMSSRCDIR)/slogipc.c \
				$(SRCDIR)/UserUtil.c

all: depend $(LIBLOCALNAME) $(TARGET)

$(TARGET) : $(OBJS) $(LIBRARYS)
	@echo ""
	@echo "Linking $(TARGET)..."
	if [ -f $@ ]; then $(MV) $@ $@~; fi
	$(CC) -o $(TARGET) $(OBJS) $(LIBS) 
	$(CP) -f $(TARGET) $(TARGET_INSTALL)

install: $(LIBLOCALNAME) $(TARGET)
	@echo ""
	@echo "Installing $(TARGET) to $(TARGET_INSTALL)..."
	if [ -f $(TARGET_INSTALL) ]; then $(MV) $(TARGET_INSTALL) $(TARGET_INSTALL)~; fi
	$(CP) -f $(TARGET) $(TARGET_INSTALL)
	$(CP) -f $(TARGET) $(USER_BIN)
	sync

uninstall:
	@echo ""
	@echo "Uninstalling $(TARGET_INSTALL)..."
	if [ -f $(TARGET_INSTALL) ]; then $(MV) $(TARGET_INSTALL) $(TARGET_INSTALL)~; fi
	$(RM) -f $(TARGET_INSTALL)

clean:
	@echo ""
	@echo "Cleaning $(TARGET)..."
	$(RM) -f $(OBJS) $(TARGET) $(TARGET)~ $(TEMP_FILES) .depend

depend: Makefile $(SRCS)
	@echo "make depend $(TARGET)..."
	$(CC) -M $(INCDIRS) $(CFLAGS) $(SRCS) > .$@
-include .depend

tags: Makefile $(SRCS)
	@echo ""
	@echo "Making tags in $(TARGET)..."
	$(CTAGS) $(SRCS) $(USER_LIB)/*.c $(USER_INC)/*.h ./*.h

.c.o : 
	@echo "Compiling $<..."
	$(CC) $(INCDIRS) $(CFLAGS) -c $<

$(LIBLOCALNAME):
	@cd $(USER_LIB); $(MAKE)
