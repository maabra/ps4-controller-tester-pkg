# Package metadata
TITLE       := PS4 Hello World
VERSION     := 1.00
TITLE_ID    := HELO00001
CONTENT_ID  := IV0000-HELO00001_00-HELLOWORLD000000

# Libraries linked into the ELF
LIBS        := -lc -lkernel -lSceMsgDialog -lSceCommonDialog -lSceSysmodule -lSceVideoOut -lScePad -lSceUserService

# Root vars
TOOLCHAIN   := $(OO_PS4_TOOLCHAIN)
INTDIR      := build

# Define objects to build
CFILES      := $(wildcard src/*.c)
OBJS        := $(patsubst src/%.c, $(INTDIR)/%.o, $(CFILES))

# Define final C/C++ flags
CFLAGS      := --target=x86_64-pc-freebsd12-elf -fPIC -funwind-tables -c -isysroot $(TOOLCHAIN) -isystem $(TOOLCHAIN)/include -Isrc -O2 -Wall -Wextra
LDFLAGS     := -m elf_x86_64 -pie --script $(TOOLCHAIN)/link.x --eh-frame-hdr -L$(TOOLCHAIN)/lib $(LIBS) $(TOOLCHAIN)/lib/crt1.o

PLATFORM    ?= linux
BINDIR      := $(TOOLCHAIN)/bin/$(PLATFORM)

.PHONY: all clean pkg

all: $(CONTENT_ID).pkg

$(INTDIR):
	mkdir -p $(INTDIR)

$(INTDIR)/%.o: src/%.c | $(INTDIR)
	clang $(CFLAGS) -o $@ $<

eboot.bin: $(OBJS)
	ld.lld $(OBJS) -o $(INTDIR)/app.elf $(LDFLAGS)
	$(BINDIR)/create-fself -in=$(INTDIR)/app.elf -out=$(INTDIR)/app.oelf --eboot "$@" --paid 0x3800000000000011

sce_sys/param.sfo: Makefile
	mkdir -p sce_sys
	$(BINDIR)/PkgTool.Core sfo_new $@
	$(BINDIR)/PkgTool.Core sfo_setentry $@ APP_TYPE --type Integer --maxsize 4 --value 1
	$(BINDIR)/PkgTool.Core sfo_setentry $@ APP_VER --type Utf8 --maxsize 8 --value '$(VERSION)'
	$(BINDIR)/PkgTool.Core sfo_setentry $@ ATTRIBUTE --type Integer --maxsize 4 --value 0
	$(BINDIR)/PkgTool.Core sfo_setentry $@ CATEGORY --type Utf8 --maxsize 4 --value gd
	$(BINDIR)/PkgTool.Core sfo_setentry $@ CONTENT_ID --type Utf8 --maxsize 48 --value '$(CONTENT_ID)'
	$(BINDIR)/PkgTool.Core sfo_setentry $@ DOWNLOAD_DATA_SIZE --type Integer --maxsize 4 --value 0
	$(BINDIR)/PkgTool.Core sfo_setentry $@ SYSTEM_VER --type Integer --maxsize 4 --value 0
	$(BINDIR)/PkgTool.Core sfo_setentry $@ TITLE --type Utf8 --maxsize 128 --value '$(TITLE)'
	$(BINDIR)/PkgTool.Core sfo_setentry $@ TITLE_ID --type Utf8 --maxsize 12 --value '$(TITLE_ID)'
	$(BINDIR)/PkgTool.Core sfo_setentry $@ VERSION --type Utf8 --maxsize 8 --value '$(VERSION)'

pkg.gp4: eboot.bin sce_sys/param.sfo sce_sys/icon0.png
	$(BINDIR)/create-gp4 -out $@ --content-id=$(CONTENT_ID) --files "eboot.bin sce_sys/param.sfo sce_sys/icon0.png"

$(CONTENT_ID).pkg: pkg.gp4
	$(BINDIR)/PkgTool.Core pkg_build $< .

pkg: $(CONTENT_ID).pkg

clean:
	rm -rf $(INTDIR) eboot.bin pkg.gp4 sce_sys/param.sfo *.pkg out