SHELL := /bin/sh

OPENORBIS_ROOT ?= $(OO_PS4_TOOLCHAIN)
CC := clang
LD := ld.lld
SDK := $(OPENORBIS_ROOT)
OUT := out
PLATFORM ?= linux
TOOLCHAIN_BIN := $(SDK)/bin/$(PLATFORM)
CREATE_EBOOT := $(TOOLCHAIN_BIN)/create-eboot
CREATE_GP4 := $(TOOLCHAIN_BIN)/create-gp4
PKG_TOOL := $(TOOLCHAIN_BIN)/PkgTool.Core
SDL_ROOT ?= third_party/SDL-PS4
SDL_LIB := $(SDL_ROOT)/lib/libSDL2.a
CFLAGS := --target=x86_64-pc-freebsd12-elf -fPIC -ffreestanding -fno-builtin \
		  -DNULL='((void*)0)' \
		  -I$(SDK)/include -I$(SDL_ROOT)/include -Isrc -O2 -Wall -Wextra -Werror
LDFLAGS := --sysroot=$(SDK) -L$(SDK)/lib -L$(SDL_ROOT)/lib -pie \
		   --script $(SDK)/link.x --eh-frame-hdr
LDLIBS := -lSDL2 -lScePad -lSceUserService -lSceVideoOut -lSceAudioOut -lSceSysmodule -lkernel -lc
SOURCES := src/main.c src/tester.c src/renderer.c
OBJECTS := $(SOURCES:src/%.c=$(OUT)/%.o)

.PHONY: all clean pkg tools-check sdl-check

all: $(OUT)/PS4ControllerTester.elf

$(OUT):
	mkdir -p $(OUT)

$(OUT)/%.o: src/%.c | $(OUT)
	$(CC) $(CFLAGS) -c $< -o $@

tools-check:
	@test -n "$(SDK)" || { echo "OO_PS4_TOOLCHAIN is not set"; exit 1; }
	@command -v $(CC) >/dev/null || { echo "Missing $(CC)"; exit 1; }
	@test -x "$(CREATE_EBOOT)" || { echo "Missing $(CREATE_EBOOT)"; exit 1; }
	@test -x "$(CREATE_GP4)" || { echo "Missing $(CREATE_GP4)"; exit 1; }
	@test -x "$(PKG_TOOL)" || { echo "Missing $(PKG_TOOL)"; exit 1; }

sdl-check:
	@test -f "$(SDL_LIB)" || { echo "Missing $(SDL_LIB); build or install SDL2-PS4 before running make"; exit 1; }

$(OUT)/eboot.bin: $(OUT)/PS4ControllerTester.elf
	$(CREATE_EBOOT) -in=$< -out=$@

$(OUT)/param.sfo: param.sfo.in | $(OUT) tools-check
	$(PKG_TOOL) sfo_new $@
	$(PKG_TOOL) sfo_setentry $@ APP_TYPE --type Integer --maxsize 4 --value 1
	$(PKG_TOOL) sfo_setentry $@ APP_VER --type Utf8 --maxsize 8 --value 01.00
	$(PKG_TOOL) sfo_setentry $@ ATTRIBUTE --type Integer --maxsize 4 --value 0
	$(PKG_TOOL) sfo_setentry $@ CATEGORY --type Utf8 --maxsize 4 --value gd
	$(PKG_TOOL) sfo_setentry $@ CONTENT_ID --type Utf8 --maxsize 48 --value IV0000-CTST00001_00-PS4CONTROLLERTEST
	$(PKG_TOOL) sfo_setentry $@ TITLE --type Utf8 --maxsize 128 --value "PS4 Controller Tester"
	$(PKG_TOOL) sfo_setentry $@ TITLE_ID --type Utf8 --maxsize 12 --value CTST00001
	$(PKG_TOOL) sfo_setentry $@ VERSION --type Utf8 --maxsize 8 --value 01.00

$(OUT)/PS4ControllerTester.elf: sdl-check $(OBJECTS)
	$(LD) $(OBJECTS) $(SDK)/lib/crt1.o -o $@ $(LDFLAGS) $(LDLIBS)

$(OUT)/PS4ControllerTester.gp4: $(OUT)/eboot.bin $(OUT)/param.sfo
	$(CREATE_GP4) -out $@ --content-id=IV0000-CTST00001_00-PS4CONTROLLERTEST --files $^

pkg: tools-check sdl-check $(OUT)/PS4ControllerTester.gp4
	$(PKG_TOOL) pkg_build $(OUT)/PS4ControllerTester.gp4 $(OUT)

clean:
	rm -rf $(OUT)