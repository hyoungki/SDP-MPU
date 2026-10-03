#
# Makefile for SCAN
#

# Include common and platform-dependent definitions
include ../common.make

TARGET = SCAN
TARGET_INSTALL = $(NFS_HOME)/$(TARGET)


SIG_DEBUG = 1

ifeq ($(SIG_DEBUG), 1)	
DEBUG_FLAG= -g -Og -fno-omit-frame-pointer -funwind-tables -rdynamic
else	
DEBUG_FLAG	
endif 
#
INCDIRS = $(INCLUDEDIRS)
CFLAGS = $(FLAGS_APP)
LFLAGS =

LIBS     = $(USER_LIB)/$(LIBLOCALNAME).$(LIB_EXTENSION) -lpthread
LIBRARYS = $(USER_LIB)/$(LIBLOCALNAME).$(LIB_EXTENSION) 

# Define objects used in the executable
OBJS = scanMain.o scanThread.o scanSUB.o
SRCS = $(OBJS:.o=.c)

all: depend $(LIBLOCALNAME) $(TARGET)

$(TARGET) : $(OBJS) $(LIBRARYS)
	@echo ""	
	@echo "Linking $(TARGET)..."
	@echo "CHECK DEBUG_FLAG $(DEBUG_FLAG)"	
	if [ -f $@ ]; then $(MV) $@ $@~; fi
	$(CC) -o $(TARGET) $(OBJS) $(LIBS) $(DEBUG_FLAG)
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
	$(CC) $(INCDIRS) $(CFLAGS)  $(DEBUG_FLAG) -c $<

# 	$(CC) $(INCDIRS) $(CFLAGS)  -g -O0 -fno-omit-frame-pointer -funwind-tables -rdynamic -c $<

$(LIBLOCALNAME):
	@cd $(USER_LIB); $(MAKE)
