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

BN5_PROTOMAN_ROM ?=
BN6_GREGAR_ROM ?=
BN6_FALZAR_ROM ?=
BN4_BLUE_MOON_ROM ?=
BN3_BLUE_ROM ?=

ifeq ($(strip $(BN5_COLONEL_ROM)),)
BN5_COLONEL_ROM := $(dir $(BN5_PROTOMAN_ROM))exe5k_rom_k_e.srl
endif

PACKAGE_DEFS := $(wildcard $(PATCH_DIR)/src/*.defs.toml)
RUNTIME_SOURCES := $(PATCH_DIR)/src/abi.c $(PATCH_DIR)/src/runtime.c
PACKAGE_SOURCES := $(filter-out $(RUNTIME_SOURCES),$(wildcard $(PATCH_DIR)/src/*.c))
ASM_SOURCES := $(wildcard $(PATCH_DIR)/src/*.asm)
C_SOURCES := $(wildcard $(PATCH_DIR)/src/*.c)
C_HEADERS := $(wildcard $(PATCH_DIR)/src/*.h)
C_LINKER_SCRIPT := $(PATCH_DIR)/src/link.ld
GREGAR_REGISTRY_METADATA := $(BUILD_DIR)/registry-metadata-gregar.generated.json
FALZAR_REGISTRY_METADATA := $(BUILD_DIR)/registry-metadata-falzar.generated.json
GREGAR_REGISTRY_ASSEMBLY := $(BUILD_DIR)/registry-gregar.generated.asm
FALZAR_REGISTRY_ASSEMBLY := $(BUILD_DIR)/registry-falzar.generated.asm
GREGAR_REGISTRY_LINKER_VALUES := $(BUILD_DIR)/registry-values-gregar.generated.ld
FALZAR_REGISTRY_LINKER_VALUES := $(BUILD_DIR)/registry-values-falzar.generated.ld
GREGAR_TEXT_REPLACEMENTS := $(BUILD_DIR)/text-replacements-gregar.generated.json
FALZAR_TEXT_REPLACEMENTS := $(BUILD_DIR)/text-replacements-falzar.generated.json
GREGAR_REGISTRY_STAMP := $(BUILD_DIR)/.registry-gregar.stamp
FALZAR_REGISTRY_STAMP := $(BUILD_DIR)/.registry-falzar.stamp
GREGAR_C_STAMP := $(BUILD_DIR)/.c-gregar.stamp
FALZAR_C_STAMP := $(BUILD_DIR)/.c-falzar.stamp
ASSET_STAMP := $(BUILD_DIR)/.assets.stamp
GREGAR_TITLE_STAMP := $(BUILD_DIR)/.title-gregar.stamp
FALZAR_TITLE_STAMP := $(BUILD_DIR)/.title-falzar.stamp
GREGAR_TEXT_STAMP := $(BUILD_DIR)/.text-gregar.stamp
FALZAR_TEXT_STAMP := $(BUILD_DIR)/.text-falzar.stamp
GREGAR_BUILD_STAMP := $(BUILD_DIR)/.gregar.stamp
FALZAR_BUILD_STAMP := $(BUILD_DIR)/.falzar.stamp

.DEFAULT_GOAL := all

.PHONY: all build patches compile-commands check-roms clean help

all: patches

build: $(GREGAR_BUILD_STAMP) $(FALZAR_BUILD_STAMP)

compile-commands: $(PATCH_DIR)/compile_commands.json

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

$(GREGAR_REGISTRY_METADATA): $(PATCH_DIR)/compile_c_metadata.py $(PACKAGE_SOURCES) $(C_HEADERS) | $(BUILD_DIR)
	$(PYTHON) "$(PATCH_DIR)/compile_c_metadata.py" \
		--cc "$(ARM_GCC)" --nm "$(ARM_NM)" --define FALZAR=0 --output "$@"

$(FALZAR_REGISTRY_METADATA): $(PATCH_DIR)/compile_c_metadata.py $(PACKAGE_SOURCES) $(C_HEADERS) | $(BUILD_DIR)
	$(PYTHON) "$(PATCH_DIR)/compile_c_metadata.py" \
		--cc "$(ARM_GCC)" --nm "$(ARM_NM)" --define FALZAR=1 --output "$@"

$(GREGAR_REGISTRY_STAMP): $(PATCH_DIR)/compile_registry.py $(PATCH_DIR)/config.gregar.toml $(PACKAGE_DEFS) $(GREGAR_REGISTRY_METADATA) | $(BUILD_DIR)
	$(PYTHON) "$(PATCH_DIR)/compile_registry.py" \
		"$(PATCH_DIR)/config.gregar.toml" \
		--metadata "$(GREGAR_REGISTRY_METADATA)" \
		--output "$(GREGAR_REGISTRY_ASSEMBLY)" \
		--linker-output "$(GREGAR_REGISTRY_LINKER_VALUES)" \
		--text-output "$(GREGAR_TEXT_REPLACEMENTS)"
	@touch "$@"

$(FALZAR_REGISTRY_STAMP): $(PATCH_DIR)/compile_registry.py $(PATCH_DIR)/config.falzar.toml $(PACKAGE_DEFS) $(FALZAR_REGISTRY_METADATA) | $(BUILD_DIR)
	$(PYTHON) "$(PATCH_DIR)/compile_registry.py" \
		"$(PATCH_DIR)/config.falzar.toml" \
		--metadata "$(FALZAR_REGISTRY_METADATA)" \
		--output "$(FALZAR_REGISTRY_ASSEMBLY)" \
		--linker-output "$(FALZAR_REGISTRY_LINKER_VALUES)" \
		--text-output "$(FALZAR_TEXT_REPLACEMENTS)"
	@touch "$@"

CFLAGS := -std=c11 -Os -mthumb -march=armv4t -mthumb-interwork \
	-ffreestanding -fno-builtin -fno-common -fno-pic -fno-pie \
	-fomit-frame-pointer -ffixed-r5 -ffunction-sections -fdata-sections \
	-fno-unwind-tables -fno-asynchronous-unwind-tables \
	-Wall -Wextra -Werror -I"$(PATCH_DIR)/src" -I"$(BUILD_DIR)"
CLDFLAGS := -nostdlib -nostartfiles -Wl,-T,"$(C_LINKER_SCRIPT)" \
	-Wl,--build-id=none

$(PATCH_DIR)/compile_commands.json: $(PATCH_DIR)/generate_compile_commands.py Makefile $(C_SOURCES) $(C_HEADERS)
	$(PYTHON) "$(PATCH_DIR)/generate_compile_commands.py" \
		--cc "$(ARM_GCC)" --cflags='$(CFLAGS)' \
		--directory "$(PATCH_DIR)" --output "$@" $(C_SOURCES)

$(GREGAR_C_STAMP): $(GREGAR_REGISTRY_STAMP) $(ASSET_STAMP) $(C_SOURCES) $(C_HEADERS) $(C_LINKER_SCRIPT) $(PATCH_DIR)/export_c_symbols.py | $(BUILD_DIR)
	$(ARM_GCC) $(CFLAGS) -DFALZAR=0 $(C_SOURCES) $(CLDFLAGS) -Wl,-T,"$(GREGAR_REGISTRY_LINKER_VALUES)" -lgcc -o "$(BUILD_DIR)/gameplay-gregar.elf"
	$(ARM_OBJCOPY) -O binary "$(BUILD_DIR)/gameplay-gregar.elf" "$(BUILD_DIR)/gameplay-gregar.bin"
	$(PYTHON) "$(PATCH_DIR)/export_c_symbols.py" --nm "$(ARM_NM)" \
		"$(BUILD_DIR)/gameplay-gregar.elf" "$(BUILD_DIR)/c-symbols-gregar.generated.asm"
	@touch "$@"

$(FALZAR_C_STAMP): $(FALZAR_REGISTRY_STAMP) $(ASSET_STAMP) $(C_SOURCES) $(C_HEADERS) $(C_LINKER_SCRIPT) $(PATCH_DIR)/export_c_symbols.py | $(BUILD_DIR)
	$(ARM_GCC) $(CFLAGS) -DFALZAR=1 $(C_SOURCES) $(CLDFLAGS) -Wl,-T,"$(FALZAR_REGISTRY_LINKER_VALUES)" -lgcc -o "$(BUILD_DIR)/gameplay-falzar.elf"
	$(ARM_OBJCOPY) -O binary "$(BUILD_DIR)/gameplay-falzar.elf" "$(BUILD_DIR)/gameplay-falzar.bin"
	$(PYTHON) "$(PATCH_DIR)/export_c_symbols.py" --nm "$(ARM_NM)" \
		"$(BUILD_DIR)/gameplay-falzar.elf" "$(BUILD_DIR)/c-symbols-falzar.generated.asm"
	@touch "$@"

$(GREGAR_TITLE_STAMP): $(PATCH_DIR)/build_title_screen.py $(PATCH_DIR)/assets/title-screen-overlay.png | check-roms $(BUILD_DIR)
	$(PYTHON) "$(PATCH_DIR)/build_title_screen.py" \
		gregar "$(BN6_GREGAR_ROM)" "$(BUILD_DIR)/title-67-gregar.bin" \
		"$(BUILD_DIR)/title-map-gregar.bin"
	@touch "$@"

$(FALZAR_TITLE_STAMP): $(PATCH_DIR)/build_title_screen.py $(PATCH_DIR)/assets/title-screen-overlay.png | check-roms $(BUILD_DIR)
	$(PYTHON) "$(PATCH_DIR)/build_title_screen.py" \
		falzar "$(BN6_FALZAR_ROM)" "$(BUILD_DIR)/title-67-falzar.bin" \
		"$(BUILD_DIR)/title-map-falzar.bin"
	@touch "$@"

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

$(GREGAR_TEXT_STAMP): $(PATCH_DIR)/build_text_archives.py $(GREGAR_REGISTRY_STAMP) | $(BUILD_DIR)
	$(PYTHON) "$(PATCH_DIR)/build_text_archives.py" \
		"$(BN6_GREGAR_ROM)" 0x42038 0x27D50 \
		"$(BUILD_DIR)/chip-names-0-gregar.bin" "$(BUILD_DIR)/chip-names-1-gregar.bin" \
		"$(BUILD_DIR)/chip-descriptions-0-gregar.bin" "$(BUILD_DIR)/chip-descriptions-1-gregar.bin" \
		--package-text "$(GREGAR_TEXT_REPLACEMENTS)"
	@touch "$@"

$(FALZAR_TEXT_STAMP): $(PATCH_DIR)/build_text_archives.py $(FALZAR_REGISTRY_STAMP) | $(BUILD_DIR)
	$(PYTHON) "$(PATCH_DIR)/build_text_archives.py" \
		"$(BN6_FALZAR_ROM)" 0x42068 0x27D50 \
		"$(BUILD_DIR)/chip-names-0-falzar.bin" "$(BUILD_DIR)/chip-names-1-falzar.bin" \
		"$(BUILD_DIR)/chip-descriptions-0-falzar.bin" "$(BUILD_DIR)/chip-descriptions-1-falzar.bin" \
		--package-text "$(FALZAR_TEXT_REPLACEMENTS)"
	@touch "$@"

$(GREGAR_BUILD_STAMP): $(GREGAR_TITLE_STAMP) $(ASSET_STAMP) $(GREGAR_TEXT_STAMP) $(GREGAR_C_STAMP) $(ASM_SOURCES) $(PATCH_DIR)/config.gregar.toml $(PATCH_DIR)/reorder_chip_sort.py | $(BUILD_DIR)
	cp "$(BN6_GREGAR_ROM)" "$(BUILD_DIR)/bn67-gregar.gba"
	cd "$(PATCH_DIR)" && "$(ARMIPS)" -root "$(PATCH_DIR)" -erroronwarning -sym "$(BUILD_DIR)/gregar.sym" src/gregar.asm
	$(PYTHON) "$(PATCH_DIR)/reorder_chip_sort.py" \
		"$(BUILD_DIR)/bn67-gregar.gba" "$(BUILD_DIR)/gregar.sym" "$(BN6_GREGAR_ROM)"
	@touch "$@"

$(FALZAR_BUILD_STAMP): $(FALZAR_TITLE_STAMP) $(ASSET_STAMP) $(FALZAR_TEXT_STAMP) $(FALZAR_C_STAMP) $(ASM_SOURCES) $(PATCH_DIR)/config.falzar.toml $(PATCH_DIR)/reorder_chip_sort.py | $(BUILD_DIR)
	cp "$(BN6_FALZAR_ROM)" "$(BUILD_DIR)/bn67-falzar.gba"
	cd "$(PATCH_DIR)" && "$(ARMIPS)" -root "$(PATCH_DIR)" -erroronwarning -sym "$(BUILD_DIR)/falzar.sym" src/falzar.asm
	$(PYTHON) "$(PATCH_DIR)/reorder_chip_sort.py" \
		"$(BUILD_DIR)/bn67-falzar.gba" "$(BUILD_DIR)/falzar.sym" "$(BN6_FALZAR_ROM)"
	@touch "$@"
	@echo "Patched ROMs written to $(BUILD_DIR)"

patches: $(GREGAR_BUILD_STAMP) $(FALZAR_BUILD_STAMP) | $(DIST_DIR)
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
	@echo "make BN5_PROTOMAN_ROM=... BN6_GREGAR_ROM=... BN6_FALZAR_ROM=... BN4_BLUE_MOON_ROM=... BN3_BLUE_ROM=..."
	@echo ""
	@echo "Targets:"
	@echo "  all      Build and create optional patch packages (default)"
	@echo "  build    Build the patched ROMs"
	@echo "  patches  Build and create optional patch packages"
	@echo "  compile-commands  Generate compile_commands.json for both editions"
	@echo "  clean    Remove generated build and distribution files"
