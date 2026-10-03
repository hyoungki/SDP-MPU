#
# Makefile for CLI-S2 (design/cli-argv-command-dispatch.md 포팅본)
#

# Include common and platform-dependent definitions
include ../common.make

TARGET = CLI-S2
TARGET_INSTALL = $(NFS_HOME)/$(TARGET)

# 2026-09-03 : History.Bin(FRAM 대체 파일, design/history-fram-to-file.md)을
# SDP 프로세스 없이도 직접 읽어보는 독립 실행 조회 도구. CLI-S2 본체와는
# 별개의 실행파일이라 TARGET 과 나란히 두 번째 링크 타깃으로 추가한다.
HISTVIEW = histView
HISTVIEW_INSTALL = $(NFS_HOME)/$(HISTVIEW)

#
INCDIRS = $(INCLUDEDIRS)
CFLAGS = $(FLAGS_APP)
LFLAGS =

LIBS     = $(USER_LIB)/$(LIBLOCALNAME).$(LIB_EXTENSION) -lpthread
LIBRARYS = $(USER_LIB)/$(LIBLOCALNAME).$(LIB_EXTENSION)

# Define objects used in the executable
OBJS = cliMain.o cliDrv.o cliCmds.o cliDisplay.o cliDebug.o
HISTVIEW_OBJS = histView.o
SRCS = $(OBJS:.o=.c) $(HISTVIEW_OBJS:.o=.c)

all: depend $(LIBLOCALNAME) $(TARGET) $(HISTVIEW)

$(TARGET) : $(OBJS) $(LIBRARYS)
	@echo ""
	@echo "Linking $(TARGET)..."
	if [ -f $@ ]; then $(MV) $@ $@~; fi
	$(CC) -o $(TARGET) $(OBJS) $(LIBS) -lreadline
	$(CP) -f $(TARGET) $(TARGET_INSTALL)

# histView 는 readline 이 필요없는 단순 조회 도구라 -lreadline 없이 링크한다.
$(HISTVIEW) : $(HISTVIEW_OBJS) $(LIBRARYS)
	@echo ""
	@echo "Linking $(HISTVIEW)..."
	if [ -f $@ ]; then $(MV) $@ $@~; fi
	$(CC) -o $(HISTVIEW) $(HISTVIEW_OBJS) $(LIBS)
	$(CP) -f $(HISTVIEW) $(HISTVIEW_INSTALL)

install: $(LIBLOCALNAME) $(TARGET) $(HISTVIEW)
	@echo ""
	@echo "Installing $(TARGET) to $(TARGET_INSTALL)..."
	if [ -f $(TARGET_INSTALL) ]; then $(MV) $(TARGET_INSTALL) $(TARGET_INSTALL)~; fi
	$(CP) -f $(TARGET) $(TARGET_INSTALL)
	$(CP) -f $(TARGET) $(USER_BIN)
	@echo "Installing $(HISTVIEW) to $(HISTVIEW_INSTALL)..."
	if [ -f $(HISTVIEW_INSTALL) ]; then $(MV) $(HISTVIEW_INSTALL) $(HISTVIEW_INSTALL)~; fi
	$(CP) -f $(HISTVIEW) $(HISTVIEW_INSTALL)
	$(CP) -f $(HISTVIEW) $(USER_BIN)
	sync

uninstall:
	@echo ""
	@echo "Uninstalling $(TARGET_INSTALL)..."
	if [ -f $(TARGET_INSTALL) ]; then $(MV) $(TARGET_INSTALL) $(TARGET_INSTALL)~; fi
	$(RM) -f $(TARGET_INSTALL)
	@echo "Uninstalling $(HISTVIEW_INSTALL)..."
	if [ -f $(HISTVIEW_INSTALL) ]; then $(MV) $(HISTVIEW_INSTALL) $(HISTVIEW_INSTALL)~; fi
	$(RM) -f $(HISTVIEW_INSTALL)

clean:
	@echo ""
	@echo "Cleaning $(TARGET)..."
	$(RM) -f $(OBJS) $(TARGET) $(TARGET)~ $(HISTVIEW_OBJS) $(HISTVIEW) $(HISTVIEW)~ $(TEMP_FILES) .depend

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
