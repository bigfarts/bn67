SHELL := /bin/sh

PATCH_DIR := $(patsubst %/,%,$(dir $(abspath $(lastword $(MAKEFILE_LIST)))))
BUILD_DIR := $(PATCH_DIR)/build
DIST_DIR := $(PATCH_DIR)/dist
TANGOPATCH_SRC := $(PATCH_DIR)/tangopatch

PYTHON ?= python3
ARMIPS ?= armips
ARM_GCC ?= arm-none-eabi-gcc
ARM_OBJCOPY ?= arm-none-eabi-objcopy
ARM_NM ?= arm-none-eabi-nm
FLIPS ?= flips
TANGO_PATCH ?= tango-patch

BN5_COLONEL_ROM ?=
BN5_PROTOMAN_ROM ?=
BN6_GREGAR_ROM ?=
BN6_FALZAR_ROM ?=
BN4_BLUE_MOON_ROM ?=
BN3_BLUE_ROM ?=

PACKAGE_DEFS := $(wildcard $(PATCH_DIR)/src/*.defs.toml)
RUNTIME_SOURCES := $(PATCH_DIR)/src/abi.c $(PATCH_DIR)/src/runtime.c
PACKAGE_SOURCES := $(filter-out $(RUNTIME_SOURCES),$(wildcard $(PATCH_DIR)/src/*.c))
ASM_SOURCES := $(wildcard $(PATCH_DIR)/src/*.asm)
C_SOURCES := $(wildcard $(PATCH_DIR)/src/*.c)
C_HEADERS := $(wildcard $(PATCH_DIR)/src/*.h)
C_LINKER_SCRIPT := $(PATCH_DIR)/src/link.ld
ASSET_STAMP := $(BUILD_DIR)/.assets.stamp

CFLAGS := -std=c11 -Os -mthumb -march=armv4t -mthumb-interwork \
	-ffreestanding -fno-builtin -fno-common -fno-pic -fno-pie \
	-fomit-frame-pointer -ffixed-r5 -ffunction-sections -fdata-sections \
	-fno-unwind-tables -fno-asynchronous-unwind-tables \
	-Wall -Wextra -Werror -I"$(PATCH_DIR)/src" -I"$(BUILD_DIR)"
CLDFLAGS := -nostdlib -nostartfiles -Wl,-T,"$(C_LINKER_SCRIPT)" \
	-Wl,--build-id=none

.DEFAULT_GOAL := all
.DELETE_ON_ERROR:

.PHONY: all build patches compile-commands check-roms clean help

# ROM validation is order-only so it runs on every invocation without forcing
# every generated target to be rebuilt.
check-roms:
	@set -eu; \
	if [ -z "$(BN5_PROTOMAN_ROM)" ] || \
	   [ -z "$(BN6_GREGAR_ROM)" ] || \
	   [ -z "$(BN6_FALZAR_ROM)" ] || \
	   [ -z "$(BN4_BLUE_MOON_ROM)" ] || \
	   [ -z "$(BN3_BLUE_ROM)" ]; then \
		echo "usage: make BN5_PROTOMAN_ROM=... BN6_GREGAR_ROM=... BN6_FALZAR_ROM=... BN4_BLUE_MOON_ROM=... BN3_BLUE_ROM=... [BN5_COLONEL_ROM=...]" >&2; \
		exit 2; \
	fi; \
	if ! command -v "$(ARMIPS)" >/dev/null 2>&1; then \
		echo "armips was not found. Set ARMIPS=/absolute/path/to/armips." >&2; \
		exit 1; \
	fi; \
	check_sha256() { \
		path=$$1; \
		expected=$$2; \
		actual=$$(shasum -a 256 "$$path" | awk '{print $$1}'); \
		if [ "$$actual" != "$$expected" ]; then \
			echo "Unsupported ROM: $$path" >&2; \
			echo "Expected SHA-256: $$expected" >&2; \
			echo "Actual SHA-256:   $$actual" >&2; \
			exit 1; \
		fi; \
	}; \
	check_sha256 "$(BN5_PROTOMAN_ROM)" b35f5890f54784c9d90a896dc5ac4831d43acc9f94e8c42816742fcfa6b41a7b; \
	check_sha256 "$(BN5_COLONEL_ROM)" d4b7aefc3918c9f801c84cfd1322c2cdbb9d13c2e3271b3c3f8f9927480f2633; \
	check_sha256 "$(BN6_GREGAR_ROM)" 572e113eeb53bb29cd9ff8acb9db265cfd48c5e509c8d0e6420b58e71e442cf2; \
	check_sha256 "$(BN6_FALZAR_ROM)" a37c1028adb72082b51e142321fa437967bc54b6f46730a53f6581ad455ad670; \
	check_sha256 "$(BN4_BLUE_MOON_ROM)" 63ea187c792f4bfcd077f92c3a509fa09ed422993aee9480c39dfdf6a561c5c1; \
	check_sha256 "$(BN3_BLUE_ROM)" 8c6767788f99dc9e2af0c9d75513b227c7c42d6d452d6165c8e08850af78e273

$(BUILD_DIR) $(DIST_DIR):
	@mkdir -p "$@"

$(PATCH_DIR)/compile_commands.json: $(PATCH_DIR)/generate_compile_commands.py Makefile $(C_SOURCES) $(C_HEADERS)
	$(PYTHON) "$(PATCH_DIR)/generate_compile_commands.py" \
		--cc "$(ARM_GCC)" --cflags='$(CFLAGS)' \
		--directory "$(PATCH_DIR)" --output "$@" $(C_SOURCES)

$(ASSET_STAMP): $(PATCH_DIR)/extract_assets.py | check-roms $(BUILD_DIR)
	$(PYTHON) "$(PATCH_DIR)/extract_assets.py" \
		--output-dir "$(BUILD_DIR)" \
		--bn5-protoman "$(BN5_PROTOMAN_ROM)" \
		--bn5-colonel "$(BN5_COLONEL_ROM)" \
		--bn6-gregar "$(BN6_GREGAR_ROM)" \
		--bn6-falzar "$(BN6_FALZAR_ROM)" \
		--bn4-blue-moon "$(BN4_BLUE_MOON_ROM)" \
		--bn3-blue "$(BN3_BLUE_ROM)"
	@touch "$@"

EDITIONS := gregar falzar
EDITION_ROM_gregar = $(BN6_GREGAR_ROM)
EDITION_ROM_falzar = $(BN6_FALZAR_ROM)
EDITION_DEFINE_gregar := 0
EDITION_DEFINE_falzar := 1
EDITION_TEXT_OFFSET_gregar := 0x42038
EDITION_TEXT_OFFSET_falzar := 0x42068

edition_targets = $(foreach edition,$(EDITIONS),$(BUILD_DIR)/$(1)$(edition)$(2))
EDITION_REGISTRY_METADATA := $(call edition_targets,registry-metadata-,.generated.json)
EDITION_REGISTRY_STAMPS := $(call edition_targets,.registry-,.stamp)
EDITION_ELFS := $(call edition_targets,gameplay-,.elf)
EDITION_BINARIES := $(call edition_targets,gameplay-,.bin)
EDITION_C_SYMBOLS := $(call edition_targets,c-symbols-,.generated.asm)
EDITION_TITLE_STAMPS := $(call edition_targets,.title-,.stamp)
EDITION_TEXT_STAMPS := $(call edition_targets,.text-,.stamp)
EDITION_ROMS := $(call edition_targets,bn67-,.gba)

$(EDITION_REGISTRY_METADATA): $(BUILD_DIR)/registry-metadata-%.generated.json: \
	$(PATCH_DIR)/compile_c_metadata.py $(PACKAGE_SOURCES) $(C_HEADERS) | $(BUILD_DIR)
	$(PYTHON) "$(PATCH_DIR)/compile_c_metadata.py" \
		--cc "$(ARM_GCC)" --nm "$(ARM_NM)" --define FALZAR=$(EDITION_DEFINE_$*) --output "$@"

$(EDITION_REGISTRY_STAMPS): $(BUILD_DIR)/.registry-%.stamp: \
	$(PATCH_DIR)/compile_registry.py $(PATCH_DIR)/config.%.toml $(PACKAGE_DEFS) \
	$(BUILD_DIR)/registry-metadata-%.generated.json | $(BUILD_DIR)
	$(PYTHON) "$(PATCH_DIR)/compile_registry.py" \
		"$(PATCH_DIR)/config.$*.toml" \
		--metadata "$(BUILD_DIR)/registry-metadata-$*.generated.json" \
		--output "$(BUILD_DIR)/registry-$*.generated.asm" \
		--linker-output "$(BUILD_DIR)/registry-values-$*.generated.ld" \
		--text-output "$(BUILD_DIR)/text-replacements-$*.generated.json"
	@touch "$@"

$(EDITION_ELFS): $(BUILD_DIR)/gameplay-%.elf: \
	$(BUILD_DIR)/.registry-%.stamp $(ASSET_STAMP) $(C_SOURCES) $(C_HEADERS) \
	$(C_LINKER_SCRIPT) | $(BUILD_DIR)
	$(ARM_GCC) $(CFLAGS) -DFALZAR=$(EDITION_DEFINE_$*) $(C_SOURCES) $(CLDFLAGS) -Wl,-T,"$(BUILD_DIR)/registry-values-$*.generated.ld" -lgcc -o "$@"

$(EDITION_BINARIES): $(BUILD_DIR)/gameplay-%.bin: $(BUILD_DIR)/gameplay-%.elf
	$(ARM_OBJCOPY) -O binary "$<" "$@"

$(EDITION_C_SYMBOLS): $(BUILD_DIR)/c-symbols-%.generated.asm: \
	$(BUILD_DIR)/gameplay-%.elf $(PATCH_DIR)/export_c_symbols.py
	$(PYTHON) "$(PATCH_DIR)/export_c_symbols.py" --nm "$(ARM_NM)" \
		"$<" "$@"

$(EDITION_TITLE_STAMPS): $(BUILD_DIR)/.title-%.stamp: \
	$(PATCH_DIR)/build_title_screen.py $(PATCH_DIR)/assets/title-screen-overlay.png \
	| check-roms $(BUILD_DIR)
	$(PYTHON) "$(PATCH_DIR)/build_title_screen.py" \
		$* "$(EDITION_ROM_$*)" "$(BUILD_DIR)/title-67-$*.bin" \
		"$(BUILD_DIR)/title-map-$*.bin"
	@touch "$@"

$(EDITION_TEXT_STAMPS): $(BUILD_DIR)/.text-%.stamp: \
	$(PATCH_DIR)/build_text_archives.py $(BUILD_DIR)/.registry-%.stamp | $(BUILD_DIR)
	$(PYTHON) "$(PATCH_DIR)/build_text_archives.py" \
		"$(EDITION_ROM_$*)" $(EDITION_TEXT_OFFSET_$*) 0x27D50 \
		"$(BUILD_DIR)/chip-names-0-$*.bin" "$(BUILD_DIR)/chip-names-1-$*.bin" \
		"$(BUILD_DIR)/chip-descriptions-0-$*.bin" "$(BUILD_DIR)/chip-descriptions-1-$*.bin" \
		--package-text "$(BUILD_DIR)/text-replacements-$*.generated.json"
	@touch "$@"

$(EDITION_ROMS): $(BUILD_DIR)/bn67-%.gba: \
	$(BUILD_DIR)/.title-%.stamp $(ASSET_STAMP) $(BUILD_DIR)/.text-%.stamp \
	$(BUILD_DIR)/gameplay-%.bin $(BUILD_DIR)/c-symbols-%.generated.asm \
	$(ASM_SOURCES) $(PATCH_DIR)/config.%.toml \
	$(PATCH_DIR)/reorder_chip_sort.py | $(BUILD_DIR)
	cp "$(EDITION_ROM_$*)" "$@"
	cd "$(PATCH_DIR)" && "$(ARMIPS)" -root "$(PATCH_DIR)" -erroronwarning -sym "$(BUILD_DIR)/$*.sym" src/$*.asm
	$(PYTHON) "$(PATCH_DIR)/reorder_chip_sort.py" \
		"$@" "$(BUILD_DIR)/$*.sym" "$(EDITION_ROM_$*)"

all: patches

build: $(EDITION_ROMS)
	@echo "Patched ROMs written to $(BUILD_DIR)"

compile-commands: $(PATCH_DIR)/compile_commands.json

patches: build | $(DIST_DIR)
	@set -eu; \
	if command -v "$(FLIPS)" >/dev/null 2>&1; then \
		"$(FLIPS)" --create --bps "$(BN6_GREGAR_ROM)" "$(BUILD_DIR)/bn67-gregar.gba" "$(DIST_DIR)/bn67-gregar.bps"; \
		"$(FLIPS)" --create --bps "$(BN6_FALZAR_ROM)" "$(BUILD_DIR)/bn67-falzar.gba" "$(DIST_DIR)/bn67-falzar.bps"; \
		echo "BPS patches written to $(DIST_DIR)"; \
		if command -v "$(TANGO_PATCH)" >/dev/null 2>&1; then \
			mkdir -p "$(TANGOPATCH_SRC)/roms"; \
			cp "$(DIST_DIR)/bn67-gregar.bps" "$(TANGOPATCH_SRC)/roms/BR5E_00.bps"; \
			cp "$(DIST_DIR)/bn67-falzar.bps" "$(TANGOPATCH_SRC)/roms/BR6E_00.bps"; \
			"$(TANGO_PATCH)" validate "$(TANGOPATCH_SRC)"; \
			"$(TANGO_PATCH)" pack --out "$(DIST_DIR)" "$(TANGOPATCH_SRC)"; \
		else \
			echo "tango-patch was not found; .tangopatch packaging was skipped." >&2; \
		fi; \
	else \
		echo "flips was not found; BPS and .tangopatch packaging were skipped." >&2; \
	fi

clean:
	rm -rf "$(BUILD_DIR)" "$(DIST_DIR)" "$(TANGOPATCH_SRC)/roms"

help:
	@echo "make BN5_PROTOMAN_ROM=... BN5_COLONEL_ROM=... BN6_GREGAR_ROM=... BN6_FALZAR_ROM=... BN4_BLUE_MOON_ROM=... BN3_BLUE_ROM=..."
	@echo ""
	@echo "Targets:"
	@echo "  all      Build and create optional patch packages (default)"
	@echo "  build    Build the patched ROMs"
	@echo "  patches  Build and create optional patch packages"
	@echo "  compile-commands  Generate compile_commands.json for both editions"
	@echo "  clean    Remove generated build and distribution files"
