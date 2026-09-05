SHELL := /bin/sh

OPENORBIS_ROOT ?= $(OO_PS4_TOOLCHAIN)
SDK := $(OPENORBIS_ROOT)
OUT := out
PLATFORM ?= linux
TOOLCHAIN_BIN := $(SDK)/bin/$(PLATFORM)

CC := clang
LD := ld.lld
CREATE_FSELF := $(TOOLCHAIN_BIN)/create-fself
CREATE_GP4 := $(TOOLCHAIN_BIN)/create-gp4
PKG_TOOL := $(TOOLCHAIN_BIN)/PkgTool.Core
ICON_SOURCE ?= package-assets/icon0.png

CONTENT_ID := IV0000-HELO00001_00-HELLOWORLD000000
TITLE := PS4 Hello World
TITLE_ID := HELO00001
VERSION := 01.00

CFLAGS := --target=x86_64-pc-freebsd12-elf -fPIC -funwind-tables -c \
		  -isysroot $(SDK) -isystem $(SDK)/include -Isrc -O2 -Wall -Wextra
LDFLAGS := -m elf_x86_64 -pie --script $(SDK)/link.x --eh-frame-hdr \
		   -L$(SDK)/lib
LDLIBS := -lSceVideoOut -lScePad -lSceUserService -lkernel -lc
SOURCES := src/main.c
OBJECTS := $(SOURCES:src/%.c=$(OUT)/%.o)

.PHONY: all clean pkg tools-check

all: $(OUT)/eboot.bin

$(OUT):
	mkdir -p $(OUT) $(OUT)/sce_sys

$(OUT)/%.o: src/%.c | $(OUT)
	$(CC) $(CFLAGS) $< -o $@

tools-check:
	@test -n "$(SDK)" || { echo "OO_PS4_TOOLCHAIN is not set"; exit 1; }
	@test "$$(printf '%s' '$(CONTENT_ID)' | wc -c)" -eq 36 || { echo "CONTENT_ID must be 36 characters"; exit 1; }
	@command -v $(CC) >/dev/null || { echo "Missing $(CC)"; exit 1; }
	@test -x "$(CREATE_FSELF)" || test -x "$(TOOLCHAIN_BIN)/create-eboot" || { echo "Missing create-fself"; exit 1; }
	@test -x "$(CREATE_GP4)" || { echo "Missing $(CREATE_GP4)"; exit 1; }
	@test -x "$(PKG_TOOL)" || { echo "Missing $(PKG_TOOL)"; exit 1; }

$(OUT)/PS4HelloWorld.elf: $(OBJECTS)
	$(LD) $(OBJECTS) $(SDK)/lib/crt1.o -o $@ $(LDFLAGS) $(LDLIBS)

$(OUT)/eboot.bin: $(OUT)/PS4HelloWorld.elf
	if [ -x "$(CREATE_FSELF)" ]; then \
		$(CREATE_FSELF) -in "$<" --out "$(OUT)/PS4HelloWorld.oelf" --eboot "$@" --paid 0x3800000000000011 || \
		$(CREATE_FSELF) "$<" "$@" --paid 0x3800000000000011; \
	elif [ -x "$(TOOLCHAIN_BIN)/create-eboot" ]; then \
		$(TOOLCHAIN_BIN)/create-eboot "$<" "$@"; \
	fi

$(OUT)/sce_sys/param.sfo: param.sfo.in | $(OUT) tools-check
	mkdir -p $(dir $@)
	$(PKG_TOOL) sfo_new $@
	$(PKG_TOOL) sfo_setentry $@ APP_TYPE --type Integer --maxsize 4 --value 1
	$(PKG_TOOL) sfo_setentry $@ APP_VER --type Utf8 --maxsize 8 --value $(VERSION)
	$(PKG_TOOL) sfo_setentry $@ ATTRIBUTE --type Integer --maxsize 4 --value 0
	$(PKG_TOOL) sfo_setentry $@ CATEGORY --type Utf8 --maxsize 4 --value gd
	$(PKG_TOOL) sfo_setentry $@ CONTENT_ID --type Utf8 --maxsize 48 --value $(CONTENT_ID)
	$(PKG_TOOL) sfo_setentry $@ DOWNLOAD_DATA_SIZE --type Integer --maxsize 4 --value 0
	$(PKG_TOOL) sfo_setentry $@ SYSTEM_VER --type Integer --maxsize 4 --value 0
	$(PKG_TOOL) sfo_setentry $@ TITLE --type Utf8 --maxsize 128 --value "$(TITLE)"
	$(PKG_TOOL) sfo_setentry $@ TITLE_ID --type Utf8 --maxsize 12 --value $(TITLE_ID)
	$(PKG_TOOL) sfo_setentry $@ VERSION --type Utf8 --maxsize 8 --value $(VERSION)

$(OUT)/sce_sys/icon0.png: $(ICON_SOURCE) | $(OUT)
	mkdir -p $(dir $@)
	cp $(ICON_SOURCE) $@

$(OUT)/PS4HelloWorld.gp4: $(OUT)/eboot.bin $(OUT)/sce_sys/param.sfo $(OUT)/sce_sys/icon0.png
	cd $(OUT) && $(CREATE_GP4) -out PS4HelloWorld.gp4 --content-id=$(CONTENT_ID) --files "eboot.bin sce_sys/param.sfo sce_sys/icon0.png"

pkg: tools-check $(OUT)/PS4HelloWorld.gp4
	$(PKG_TOOL) pkg_build $(OUT)/PS4HelloWorld.gp4 $(OUT)

clean:
	rm -rf $(OUT)