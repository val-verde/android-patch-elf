BINDIR=bin
CC=clang
CFLAGS=-s -DNDEBUG -O3
CP=cp
DSTROOT=/usr
MKDIR=mkdir
OUTDIR=./out
RM=rm
TARGET=android-patch-elf

.PHONY: all
all: $(TARGET)

$(TARGET): main.c
	$(MKDIR) -p ${OUTDIR}
	$(CC) $(CFLAGS) -o $(OUTDIR)/$(TARGET) main.c

.PHONY: install
install: $(TARGET)
	$(MKDIR) -p $(DSTROOT)/$(BINDIR)
	$(CP) -p $(OUTDIR)/$(TARGET) $(DSTROOT)/$(BINDIR)

.PHONY: clean 
clean:
	$(RM) -f $(OUTDIR)/$(TARGET)
