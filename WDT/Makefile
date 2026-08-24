#
# Makefile for WDT
#

# Include common and platform-dependent definitions
include ../common.make

TARGET = WDT
TARGET_INSTALL = $(NFS_HOME)/$(TARGET)

#
INCDIRS = $(INCLUDEDIRS)
CFLAGS = $(FLAGS_APP)
LFLAGS =

LIBS     = $(USER_LIB)/$(LIBLOCALNAME).$(LIB_EXTENSION)
LIBRARYS = $(USER_LIB)/$(LIBLOCALNAME).$(LIB_EXTENSION) 

# Define objects used in the executable
#OBJS = wdtMain.o wdtRtc1340.o wdtConfig.o
OBJS = wdtMain.o wdtRtc1340.o wdtConfig.o wdtCalculate.o wdtParser.o wdtNtpClient.o gpio_lan_ctrl.o
#SRCS = $(OBJS:.o=.c)
#SRCS = 
all:  $(LIBLOCALNAME) $(TARGET)

$(TARGET) : $(OBJS) $(LIBRARYS)
	@echo ""
	@echo "Linking $(TARGET)..."
	if [ -f $@ ]; then $(MV) $@ $@~; fi
	echo "Compling $(TARGET)"
	$(CC) -o $(TARGET) $(OBJS) $(LIBS) -lstdc++
	echo "cp $(TARGET) $(TARGET_INSTALL)"
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

#depend: Makefile $(SRCS)
#	@echo "make depend $(TARGET)..."
#	$(CC) -M $(INCDIRS) $(CFLAGS) $(SRCS) > .$@
#-include .depend

tags: Makefile $(SRCS)
	@echo ""
	@echo "Making tags in $(TARGET)..."
	$(CTAGS) $(SRCS) $(USER_LIB)/*.c $(USER_INC)/*.h ./*.h

.c.o : 
	@echo "Compiling $<..."
	$(CC) $(INCDIRS) $(CFLAGS) -c $<

.cpp.o : 
	@echo "Compiling $<..."
	$(CC) $(INCDIRS) $(CFLAGS) -c $<

$(LIBLOCALNAME):
	@cd $(USER_LIB); $(MAKE)
