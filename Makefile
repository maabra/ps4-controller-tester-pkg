SHELL := /bin/sh

OPENORBIS_ROOT ?= /opt/OpenOrbis-Toolchain
CC := $(OPENORBIS_ROOT)/llvm/bin/clang
LD := $(OPENORBIS_ROOT)/llvm/bin/ld.lld
SDK := $(OPENORBIS_ROOT)/target
OUT := out
CFLAGS := --target=x86_64-scei-ps4 -fPIC -ffreestanding -fno-builtin \
          -I$(SDK)/include -Isrc -O2 -Wall -Wextra -Werror
LDFLAGS := --sysroot=$(SDK) -L$(SDK)/lib -pie
SOURCES := src/main.c src/tester.c
OBJECTS := $(SOURCES:src/%.c=$(OUT)/%.o)

.PHONY: all clean pkg tools-check

all: $(OUT)/PS4ControllerTester.elf

$(OUT):
	mkdir -p $(OUT)

$(OUT)/%.o: src/%.c | $(OUT)
	$(CC) $(CFLAGS) -c $< -o $@

$(OUT)/PS4ControllerTester.elf: $(OBJECTS)
	$(LD) $(LDFLAGS) $(OBJECTS) -o $@

tools-check:
	@test -x "$(CC)" || { echo "Missing OpenOrbis compiler: $(CC)"; exit 1; }
	@command -v create-fself >/dev/null || { echo "Missing create-fself"; exit 1; }
	@command -v create-param-sfo >/dev/null || { echo "Missing create-param-sfo"; exit 1; }
	@command -v create-gp4 >/dev/null || { echo "Missing create-gp4"; exit 1; }
	@command -v orbis-pub-cmd >/dev/null || { echo "Missing orbis-pub-cmd"; exit 1; }

$(OUT)/eboot.bin: $(OUT)/PS4ControllerTester.elf
	create-fself $< $@

$(OUT)/param.sfo: param.sfo.in | $(OUT)
	create-param-sfo $< $@

pkg: tools-check $(OUT)/eboot.bin $(OUT)/param.sfo
	create-gp4 --param-sfo $(OUT)/param.sfo --eboot $(OUT)/eboot.bin --output $(OUT)/PS4ControllerTester.gp4
	orbis-pub-cmd img_create $(OUT)/PS4ControllerTester.gp4 $(OUT)/PS4ControllerTester.pkg

clean:
	rm -rf $(OUT)