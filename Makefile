CC=clang
DSTROOT=/usr/bin
TARGET=android-patch-elf

.PHONY: all
all: $(TARGET)

$(TARGET): main.c
	$(CC) -o $(TARGET) main.c

.PHONY: install
install:
	mkdir -p $(DSTROOT)
	cp -p android-patch-elf $(DSTROOT)

.PHONY: clean 
clean:
	rm -f $(TARGET)
