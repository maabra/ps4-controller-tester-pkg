SHELL := /bin/sh

OPENORBIS_ROOT ?= $(OO_PS4_TOOLCHAIN)
CC ?= clang
LD ?= ld.lld
SDK := $(OPENORBIS_ROOT)
OUT := out
CFLAGS := --target=x86_64-scei-ps4 -fPIC -ffreestanding -fno-builtin \
		  -I$(SDK)/include -Ithird_party/SDL-PS4/include -Isrc -O2 -Wall -Wextra -Werror
LDFLAGS := --sysroot=$(SDK) -L$(SDK)/lib -Lthird_party/SDL-PS4/lib -pie
LDLIBS := -lSDL2 -lScePad -lSceUserService -lSceVideoOut -lSceSysmodule -lkernel -lc
SOURCES := src/main.c src/tester.c src/renderer.c
OBJECTS := $(SOURCES:src/%.c=$(OUT)/%.o)

.PHONY: all clean pkg tools-check

all: $(OUT)/PS4ControllerTester.elf

$(OUT):
	mkdir -p $(OUT)

$(OUT)/%.o: src/%.c | $(OUT)
	$(CC) $(CFLAGS) -c $< -o $@

$(OUT)/PS4ControllerTester.elf: $(OBJECTS)
	$(LD) $(LDFLAGS) $(OBJECTS) $(LDLIBS) -o $@

tools-check:
	@command -v clang >/dev/null || { echo "Missing clang"; exit 1; }
	@command -v create-eboot >/dev/null || { echo "Missing create-eboot"; exit 1; }
	@command -v create-gp4 >/dev/null || { echo "Missing create-gp4"; exit 1; }
	@command -v PkgTool.Core >/dev/null || { echo "Missing PkgTool.Core"; exit 1; }

$(OUT)/eboot.bin: $(OUT)/PS4ControllerTester.elf
	create-eboot $< $@

$(OUT)/param.sfo: param.sfo.in | $(OUT)
	create-param-sfo $< $@

pkg: tools-check $(OUT)/eboot.bin $(OUT)/param.sfo
	create-gp4 --param-sfo $(OUT)/param.sfo --eboot $(OUT)/eboot.bin --output $(OUT)/PS4ControllerTester.gp4
	PkgTool.Core pkg_build $(OUT)/PS4ControllerTester.gp4 $(OUT)

clean:
	rm -rf $(OUT)