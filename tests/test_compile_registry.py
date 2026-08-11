import json
from dataclasses import replace
from pathlib import Path
import re
import subprocess
import sys
import tempfile
import tomllib
import unittest

from build_text_archives import (
    LINE_BREAK,
    RECORD_END,
    build_archive,
    encode_description,
    encode_name,
    encode_text,
    load_package_text,
)
from compile_registry import (
    PackageError,
    discover_packages,
    emit_attack_tables,
    emit_chip_records,
    emit_text_archives,
    fixed_width_entry_count,
    generate,
    generate_c_values,
    generate_linker_values,
    generate_text_manifest,
    load_config,
    load_metadata,
    text_archive_entry_count,
    validate_and_allocate,
)
from extract_assets import ASSETS


ROOT = Path(__file__).resolve().parents[1]


class PackageCompilerTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls) -> None:
        cls.temporary = tempfile.TemporaryDirectory()
        cls.fixture_root = Path(cls.temporary.name)
        (cls.fixture_root / "build").mkdir()
        extracted_assets = {asset.output: asset for asset in ASSETS}
        cls.config_paths = {}
        for variant in ("gregar", "falzar"):
            source = ROOT / f"config.{variant}.toml"
            config_path = cls.fixture_root / source.name
            config_text = source.read_text()
            config_path.write_text(config_text)
            cls.config_paths[variant] = config_path
            raw = tomllib.loads(config_text)
            fixed_tables = (
                [item["native_table"] for item in raw["object_classes"]]
                + [item["native_table"] for item in raw["sprite_groups"]]
                + [raw["songs"]["native_table"]]
                + [item["native_table"] for item in raw["attack_pools"].values()]
            )
            for relative in fixed_tables:
                asset = extracted_assets[Path(relative).name]
                (cls.fixture_root / relative).write_bytes(bytes(asset.length))
            for archive in raw["text"]["archives"]:
                entry_count = 0x100 if archive["source_index"] == 0 else 0xA8
                (cls.fixture_root / archive["binary"]).write_bytes(
                    build_archive([b"\xE6"] * entry_count)
                )

        cls.metadata = {}
        for variant, falzar in (("gregar", 0), ("falzar", 1)):
            metadata_path = Path(cls.temporary.name) / f"metadata-{variant}.json"
            subprocess.run(
                [
                    sys.executable,
                    str(ROOT / "compile_c_metadata.py"),
                    "--define",
                    f"FALZAR={falzar}",
                    "--output",
                    str(metadata_path),
                ],
                check=True,
            )
            cls.metadata[variant] = load_metadata(metadata_path)

    def test_runtime_sources_are_not_packages(self) -> None:
        for metadata in self.metadata.values():
            self.assertNotIn("abi", metadata)
            self.assertNotIn("runtime", metadata)

    @classmethod
    def tearDownClass(cls) -> None:
        cls.temporary.cleanup()

    def config(self, variant="gregar"):
        return replace(load_config(self.config_paths[variant]), root=ROOT)

    def packages(self, variant="gregar"):
        config = self.config(variant)
        return config, discover_packages(config, self.metadata[variant])

    def test_config_is_loaded_independently(self) -> None:
        for variant in ("gregar", "falzar"):
            raw = tomllib.loads((ROOT / f"config.{variant}.toml").read_text())
            config = self.config(variant)

            self.assertNotIn("manifest_globs", raw)
            self.assertEqual(config.variant, raw["variant"])
            self.assertEqual(config.songs.references, tuple(raw["songs"]["references"]))
            self.assertEqual(
                config.text.folder_edit_skip,
                (
                    raw["text"]["folder_edit_skip_address"],
                    raw["text"]["folder_edit_skip_target"],
                ),
            )
            self.assertEqual(config.chips.table_address, raw["chips"]["table_address"])
            for item in raw["object_classes"]:
                self.assertNotIn("native_entries", item)
                self.assertEqual(
                    config.object_classes[item["number"]].native_entries,
                    (self.fixture_root / item["native_table"]).stat().st_size // 4,
                )
            for group in raw["sprite_groups"]:
                self.assertIn("references", group)
                self.assertNotIn("pointer_address", group)
                self.assertNotIn("native_entries", group)
                self.assertEqual(
                    config.sprite_groups[group["number"]].native_entries,
                    (self.fixture_root / group["native_table"]).stat().st_size // 4,
                )
            self.assertNotIn("native_entries", raw["songs"])
            self.assertEqual(
                config.songs.native_entries,
                (self.fixture_root / raw["songs"]["native_table"]).stat().st_size
                // 8,
            )
            for kind, pool in config.attack_pools.items():
                self.assertEqual(pool.family, raw["attack_pools"][kind]["family"])
                self.assertNotIn("native_entries", raw["attack_pools"][kind])
                self.assertEqual(
                    pool.native_entries,
                    (
                        self.fixture_root
                        / raw["attack_pools"][kind]["native_table"]
                    ).stat().st_size
                    // 4,
                )
            archives = {
                archive.name: archive
                for group in config.text.groups
                for archive in group.archives
            }
            for item in raw["text"]["archives"]:
                self.assertNotIn("native_entries", item)
                binary = (self.fixture_root / item["binary"]).read_bytes()
                self.assertEqual(
                    archives[item["name"]].native_entries,
                    int.from_bytes(binary[:2], "little") // 2,
                )
            self.assertEqual(archives["chip-names-1"].native_entries, 0xA8)
            self.assertEqual(archives["chip-descriptions-1"].native_entries, 0xA8)

    def test_binary_entry_counts_reject_malformed_tables(self) -> None:
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            (root / "fixed.bin").write_bytes(b"12345")
            with self.assertRaisesRegex(PackageError, "multiple of 4 bytes"):
                fixed_width_entry_count(root, "fixed.bin", 4, 0x100, "table")

            (root / "text.bin").write_bytes(b"\x05\x00payload")
            with self.assertRaisesRegex(PackageError, "invalid offset-table size"):
                text_archive_entry_count(root, "text.bin", "archive")

    def test_package_text_records_end_with_the_record_delimiter(self) -> None:
        manifest = {
            "archives": [
                {
                    "name": "names",
                    "source": "names",
                    "source_index": 0,
                    "encoding": "name",
                },
                {
                    "name": "descriptions",
                    "source": "descriptions",
                    "source_index": 0,
                    "encoding": "description",
                },
            ],
            "entries": [
                {
                    "package": "test",
                    "archive": "names",
                    "index": 0,
                    "value": "BugCharg",
                },
                {
                    "package": "test",
                    "archive": "descriptions",
                    "index": 0,
                    "value": "All your\nbugs will\nattack!",
                },
            ],
        }
        with tempfile.TemporaryDirectory() as directory:
            path = Path(directory) / "text.json"
            path.write_text(json.dumps(manifest))
            package_text = load_package_text(path)

        name = package_text.changes["names"][0]
        description = package_text.changes["descriptions"][0]
        self.assertNotEqual(encode_name("BugCharg")[-1], RECORD_END)
        self.assertNotEqual(
            encode_description("All your\nbugs will\nattack!")[-1], RECORD_END
        )
        self.assertEqual(name[-1], RECORD_END)
        self.assertEqual(description[-1], RECORD_END)
        self.assertEqual(
            encode_text("bugs\nattack"),
            encode_text("bugs") + bytes((LINE_BREAK,)) + encode_text("attack"),
        )
        self.assertEqual(description.count(bytes((LINE_BREAK,))), 2)

    def test_text_encoder_uses_longest_token_match(self) -> None:
        self.assertEqual(
            encode_text("[B][BX][bat]"),
            bytes((0xB5, 0x41, 0xA0)),
        )

    def test_registry_compiler_is_target_agnostic(self) -> None:
        compiler = (ROOT / "compile_registry.py").read_text().lower()
        self.assertNotIn("gregar", compiler)
        self.assertNotIn("falzar", compiler)

        assemblies = {}
        for variant in ("gregar", "falzar"):
            config, packages = self.packages(variant)
            assemblies[variant] = generate(
                config,
                packages,
                validate_and_allocate(config, packages),
            )
            self.assertNotIn(".if falzar", assemblies[variant])

        self.assertIn(".org 0x080EACD0", assemblies["gregar"])
        self.assertNotIn(".org 0x080E9990", assemblies["gregar"])
        self.assertIn(".org 0x080E9990", assemblies["falzar"])
        self.assertNotIn(".org 0x080EACD0", assemblies["falzar"])

    def test_text_archive_layout_comes_from_config(self) -> None:
        config, packages = self.packages()
        assembly = "\n".join(emit_text_archives(config))
        manifest = json.loads(generate_text_manifest(config, packages))

        configured_names = set()
        for group in config.text.groups:
            for archive in group.archives:
                configured_names.add(archive.name)
                self.assertIn(f"{archive.symbol}:", assembly)
                self.assertIn(f'.incbin "{archive.binary}"', assembly)
                for address in archive.references:
                    self.assertIn(f".org 0x{address:08X}", assembly)
                    self.assertIn(f".dw {archive.symbol}", assembly)
        self.assertEqual(
            configured_names,
            {archive["name"] for archive in manifest["archives"]},
        )
        for entry in manifest["entries"]:
            self.assertEqual(set(entry), {"package", "archive", "index", "value"})
            self.assertIn(entry["archive"], configured_names)

    def test_conventions_and_shared_resource_deduplication(self) -> None:
        config, packages = self.packages()
        allocations = validate_and_allocate(config, packages)
        assembly = generate(config, packages, allocations)
        linker = generate_linker_values(packages, allocations)

        self.assertNotIn('.include "packages/', assembly)
        self.assertIn(".dw searchman_reticle_main + 1", assembly)
        self.assertIn("__bn67_object_id_searchman_reticle_main =", linker)
        self.assertIn("__bn67_sprite_id_searchman_reticle_sprite =", linker)
        self.assertIn("__bn67_sprite_group_searchman_reticle_sprite =", linker)
        self.assertIn(".dw searchman_reticle_sprite", assembly)
        self.assertIn(
            f"__bn67_song_id_common_navi_summon_song = "
            f"0x{allocations.songs['common_navi_summon_song']:X};",
            linker,
        )
        self.assertIn(
            f"__bn67_song_group_common_navi_summon_song = "
            f"0x{allocations.song_players['common_navi_summon_song']:X};",
            linker,
        )
        self.assertNotIn("LASERMAN_SUMMON_SONG", assembly)
        self.assertNotIn("ROLLARROW_SUMMON_SONG", assembly)
        self.assertIn(
            "exe6_sound_req(BN67_SONG_ID(common_navi_summon_song))",
            (ROOT / "src/chips/laserman.c").read_text(),
        )
        self.assertIn(
            "BN67_SONG_ID(common_navi_summon_song)",
            (ROOT / "src/chips/rollarrow.c").read_text(),
        )
        self.assertNotIn(".definelabel", assembly)

    def test_sources_and_text_definition_files(self) -> None:
        self.assertFalse((ROOT / "packages").exists())
        self.assertFalse(list((ROOT / "src").rglob("manifest.toml")))
        self.assertFalse(list((ROOT / "src").rglob("*.defs.toml")))
        configured_archives = {
            archive.name
            for group in self.config().text.groups
            for archive in group.archives
        }
        for path in (ROOT / "src").rglob("*.text.toml"):
            definitions = tomllib.loads(path.read_text())
            self.assertTrue(definitions, path)
            self.assertLessEqual(set(definitions), configured_archives, path)
            for entries in definitions.values():
                self.assertIsInstance(entries, dict, path)
        for source in (ROOT / "src").rglob("*.c"):
            text = source.read_text()
            self.assertNotIn(".generated.h", text, source)
            self.assertNotIn("BN67_PCM_SONG", text, source)

    def test_explicit_battle_sprite_priorities_stay_behind_hud(self) -> None:
        arguments = []
        for source in (ROOT / "src/chips").rglob("*.c"):
            arguments.extend(
                argument.strip()
                for argument in re.findall(
                    r"exe6_obj_prio_set\(\s*([^()]+?)\s*\)",
                    source.read_text(),
                )
            )

        self.assertTrue(arguments)
        self.assertEqual(set(arguments), {"EXE6_OBJ_PRIORITY_BATTLE"})

    def test_chip_records_are_linked_c_resources(self) -> None:
        gregar_config, gregar_packages = self.packages("gregar")
        falzar_config, falzar_packages = self.packages("falzar")
        allocations = validate_and_allocate(gregar_config, gregar_packages)
        assembly = "\n".join(emit_chip_records(gregar_config, gregar_packages))
        falzar_assembly = "\n".join(
            emit_chip_records(falzar_config, falzar_packages)
        )

        bugcharge = next(
            chip
            for package in gregar_packages if package.name == "bugcharge"
            for chip in package.chips
        )
        self.assertEqual(bugcharge.chip_id, 0x131)
        self.assertEqual(bugcharge.record, "bn67_chip_record_0x131")
        self.assertIn(
            0x0BA,
            {
                chip.chip_id
                for package in gregar_packages
                for chip in package.chips
            },
        )

        bugcharge_package = next(
            package for package in gregar_packages if package.name == "bugcharge"
        )
        text_entries = {
            (entry.archive, entry.index): entry.value
            for entry in bugcharge_package.text
        }
        self.assertEqual(text_entries[("chip-names-1", 0x31)], "BugCharg")
        self.assertEqual(
            text_entries[("chip-descriptions-1", 0x31)],
            "All your\nbugs will\nattack!",
        )

        self.assertIn("// bugcharge: chip 0x131", assembly)
        self.assertIn(
            "copy_c_data 0x08025214,bn67_chip_record_0x131,0x2C",
            assembly,
        )
        self.assertIn("bn67_chip_record_0x131", falzar_assembly)
        bugcharge_attack = allocations.attacks["bugcharge_attack_main"]
        c_values = generate_c_values(allocations)
        self.assertIn(
            f"#define bn67_attack_family_bugcharge_attack_main "
            f"0x{bugcharge_attack.family:02X}u",
            c_values,
        )
        self.assertIn(
            f"#define bn67_attack_subfamily_bugcharge_attack_main "
            f"0x{bugcharge_attack.subfamily:02X}u",
            c_values,
        )
        bugcharge_source = (ROOT / "src/chips/bugcharge.c").read_text()
        self.assertIn("BN67_CHIP_RECORD(0x131)", bugcharge_source)
        self.assertIn(".counter_settings = 0x8B", bugcharge_source)
        self.assertIn(
            ".family = BN67_ATTACK_FAMILY(bugcharge_attack_main)",
            bugcharge_source,
        )

    def test_attack_entries_are_compiler_allocated(self) -> None:
        config, packages = self.packages()
        allocations = validate_and_allocate(config, packages)
        assembly = "\n".join(emit_attack_tables(config, packages, allocations))

        attacks = [package.attack for package in packages if package.attack is not None]
        self.assertEqual(len(allocations.attacks), len(attacks))
        self.assertEqual(
            len({(slot.family, slot.subfamily) for slot in allocations.attacks.values()}),
            len(attacks),
        )
        for kind, pool in config.attack_pools.items():
            ordered = sorted(
                (attack for attack in attacks if attack.kind == kind),
                key=lambda attack: (attack.chip_id, attack.package, attack.main),
            )
            self.assertEqual(
                [allocations.attacks[attack.main].subfamily for attack in ordered],
                list(range(pool.native_entries, pool.native_entries + len(ordered))),
            )
        for attack in attacks:
            slot = allocations.attacks[attack.main]
            package = next(
                package for package in packages if package.name == attack.package
            )
            self.assertIn(attack.chip_id, {chip.chip_id for chip in package.chips})
            self.assertIn(
                f".dw {attack.main} + 1 // 0x{slot.subfamily:02X} {attack.package}",
                assembly,
            )

        self.assertNotIn("attack_route", assembly)
        self.assertNotIn("cmp r0", assembly)
        self.assertNotIn("unassigned_attack_main", assembly)
        self.assertEqual(
            sum(
                1
                for line in assembly.splitlines()
                if line.startswith("    .dw ") and " // 0x" in line
            ),
            len(attacks),
        )
        for pool in config.attack_pools.values():
            allocated = [
                allocation.subfamily
                for allocation in allocations.attacks.values()
                if allocation.family == pool.family
            ]
            if allocated:
                self.assertNotIn(f"// 0x{max(allocated) + 1:02X}", assembly)
        for pool in config.attack_pools.values():
            label = f"attack_family_{pool.family:02x}_table"
            self.assertIn(f"{label}:", assembly)
            self.assertIn(f"{label}_end:", assembly)
            for address in pool.references:
                self.assertIn(f".org 0x{address:08X}", assembly)

        for package in packages:
            source = (ROOT / package.source).read_text()
            self.assertNotRegex(source, r"(?m)^\.org 0x0802C(?:CD|D[A-F0-9])")
        searchman = next(package for package in packages if package.name == "searchman")
        self.assertIsNotNone(searchman.attack)
        self.assertEqual(searchman.attack.kind, "summon_attack")
        signalred = next(package for package in packages if package.name == "signalred")
        self.assertIsNotNone(signalred.attack)
        self.assertEqual(signalred.attack.kind, "persistent_attack")
        folderback = next(package for package in packages if package.name == "folderback")
        self.assertIsNotNone(folderback.attack)
        self.assertEqual(folderback.attack.kind, "persistent_attack")

    def test_ephemeral_attack_pool_follows_the_native_table(self) -> None:
        expected_references = {
            "gregar": (0x080ED72C,),
            "falzar": (0x080EC3EC,),
        }
        for variant in ("gregar", "falzar"):
            config, packages = self.packages(variant)
            ephemeral_pool = config.attack_pools["ephemeral_attack"]
            self.assertEqual(ephemeral_pool.family, 0x1C)
            self.assertEqual(ephemeral_pool.native_entries, 0x17)
            self.assertEqual(
                ephemeral_pool.references,
                expected_references[variant],
            )
            signalred = next(
                package for package in packages if package.name == "signalred"
            )
            self.assertIsNotNone(signalred.attack)
            ephemeral_signalred = replace(
                signalred,
                attack=replace(signalred.attack, kind="ephemeral_attack"),
            )
            packages = [
                ephemeral_signalred if package.name == "signalred" else package
                for package in packages
            ]
            allocations = validate_and_allocate(config, packages)
            slot = allocations.attacks["signalred_attack_main"]
            self.assertEqual(slot.family, 0x1C)
            self.assertEqual(slot.subfamily, 0x17)
            assembly = "\n".join(
                emit_attack_tables(config, packages, allocations)
            )
            self.assertIn("attack_family_1c_table:", assembly)
            self.assertIn(
                f'.incbin "{ephemeral_pool.native_table}"',
                assembly,
            )
            self.assertIn(
                ".dw signalred_attack_main + 1 // 0x17 signalred",
                assembly,
            )

    def test_custom_summons_follow_delta_ray(self) -> None:
        for variant in ("gregar", "falzar"):
            config, packages = self.packages(variant)
            allocations = validate_and_allocate(config, packages)
            summon_pool = config.attack_pools["summon_attack"]
            self.assertEqual(summon_pool.family, 0x1B)
            self.assertEqual(summon_pool.native_entries, 0x1D)
            self.assertEqual(
                allocations.attacks["rollarrow_attack_main"].subfamily,
                0x1D,
            )

    def test_object_ids_are_compiler_allocated(self) -> None:
        config, packages = self.packages()
        allocations = validate_and_allocate(config, packages)
        assembly = generate(config, packages, allocations)

        self.assertNotIn("custom_type_", assembly)
        self.assertNotIn("custom_object_kind", assembly)
        self.assertNotIn("unassigned_object_main", assembly)
        for number, namespace in allocations.objects.items():
            first_custom_id = config.object_classes[number].native_entries
            self.assertEqual(min(namespace.values()), first_custom_id)
            self.assertEqual(len(namespace), len(set(namespace.values())))
            for main, object_id in namespace.items():
                self.assertIn(
                    f".dw {main} + 1 // 0x{object_id:02X} {main}",
                    assembly,
                )
            if namespace:
                self.assertNotIn(
                    f"// 0x{max(namespace.values()) + 1:02X}", assembly
                )
            label = f"object_class_{number}_table"
            self.assertIn(f"{label}:", assembly)
            self.assertIn(f"{label}_end:", assembly)
            object_class = config.object_classes[number]
            self.assertIn(f'.incbin "{object_class.native_table}"', assembly)
            for address in object_class.references:
                self.assertIn(f".org 0x{address:08X}", assembly)

        self.assertNotIn("object_class_1_dispatch_table", assembly)
        self.assertNotIn("object_dispatch_interceptor_main:", assembly)
        self.assertIn("// Package-declared fixed section patches.", assembly)
        self.assertIn(
            ".org 0x080031FA\n"
            "    push {r1}\n"
            "    bl section_patch_folderback_dispatch_main_relay\n"
            ".org 0x08003C9C\n"
            "section_patch_folderback_dispatch_main_relay:",
            assembly,
        )
        self.assertIn("ldr r1,=folderback_dispatch_main + 1", assembly)
        self.assertIn(
            ".org 0x08012646\n"
            "    push {r1}\n"
            "    bl section_patch_blackweapon_attack_level_dispatch_relay\n"
            ".org 0x0801264C\n"
            "section_patch_blackweapon_attack_level_dispatch_relay:",
            assembly,
        )
        self.assertIn(
            "ldr r1,=blackweapon_attack_level_dispatch + 1",
            assembly,
        )
        self.assertIn(
            ".org 0x080117E0\n"
            "    .dw blackweapon_beast_buster_id3_dispatch + 1",
            assembly,
        )
        self.assertIn(
            ".org 0x080117E4\n"
            "    .dw blackweapon_beast_buster_id4_dispatch + 1",
            assembly,
        )
        folderback = (ROOT / "src/chips/folderback.c").read_text()
        self.assertIn(
            "BN67_PATCH_SECTION(0x080031FA, 0x08003C9C, folderback_dispatch_main)",
            folderback,
        )
        self.assertNotIn("BN67_PATCH_POINTER(0x08003224", folderback)
        self.assertNotIn("__bn67_object_kind", folderback)
        self.assertIn("BN67_OBJ_ID(folderback_controller_main)", folderback)
        self.assertIn("folderback_object_should_pause", folderback)
        self.assertEqual(folderback.count("exe6_battle_chip_set();"), 1)
        self.assertNotIn("exe6_deck_shuffle_sub(", folderback)
        self.assertIn("selection->active_chip_index = 0;", folderback)
        self.assertIn("selection->loaded_chip_count = 0;", folderback)
        self.assertNotIn("sizeof(*selection)", folderback)
        self.assertIn("const Exe6ObjectSlot *slots = EXE6_EFFECT_POOL_HEAD", folderback)
        self.assertIn("EXE6_POOL_SLOT_COUNT", folderback)
        self.assertIn("object->object_class", folderback)
        self.assertIn("EXE6_OBJECT_CLASS_ENEMY", folderback)
        self.assertIn("EXE6_OBJECT_CLASS_SHELL", folderback)
        self.assertNotIn("EXE6_SHELL_TYPE_2", folderback)
        self.assertNotIn("locked_opponents", folderback)
        self.assertNotIn("object->parent", folderback)
        self.assertNotIn("object->owner", folderback)
        self.assertIn('"bl folderback_object_should_pause\\n"', folderback)
        self.assertNotIn('"ldr r6,=0x02036870\\n"', folderback)
        self.assertIn('"pop {r1}\\n"', folderback)
        self.assertIn('"push {r0,r1,r2,r3,r4,r6,r7}\\n"', folderback)
        self.assertEqual(
            folderback.count('"pop {r0,r1,r2,r3,r4,r6,r7}\\n"'),
            2,
        )
        self.assertEqual(folderback.count('"lsrs r1,r1,#1\\n"'), 2)
        self.assertEqual(folderback.count('"lsls r1,r1,#1\\n"'), 2)
        for instruction in (
            "push {r0}",
            "ldr r0,=0x08003200 + 1",
            "mov lr,r0",
            "pop {r0}",
            "mov pc,lr",
        ):
            self.assertIn(f'"{instruction}\\n"', folderback)
        self.assertNotIn("0x08003206", folderback)
        self.assertNotIn("0x0800372A", folderback)
        runtime = (ROOT / "src/runtime.h").read_text()
        abi = (ROOT / "src/abi.h").read_text()
        self.assertIn("struct Exe6ObjectSlotFields", abi)
        self.assertIn("sizeof(Exe6ObjectSlot) == 0xC8", abi)
        self.assertIn("EXE6_ENEMY_POOL_HEAD", abi)
        self.assertIn("EXE6_SHELL_POOL_HEAD", abi)
        self.assertIn("EXE6_EFFECT_POOL_HEAD", abi)
        self.assertIn("0x0203A9B0u", abi)
        self.assertIn("0x0203CFE0u", abi)
        self.assertIn("0x02036870u", abi)
        self.assertIn("sizeof(Exe6EnemyObjectSlot) == 0xD8", abi)
        self.assertIn("sizeof(Exe6ShellObjectSlot) == 0xD8", abi)
        self.assertNotIn("BN67_POINTER_PATCH", runtime)
        self.assertIn("BN67_PATCH_POINTER", runtime)
        self.assertIn("BN67_PATCH_THUMB_POINTER", runtime)
        self.assertIn("BN67_PATCH_SECTION", runtime)
        self.assertNotRegex(runtime, r"(?m)^#define EXE6_")

    def test_out_of_range_c_chip_id_is_rejected(self) -> None:
        metadata = dict(self.metadata["gregar"])
        metadata["antinavi"] = ["__bn67_meta__chip__0x13a"]
        config = self.config()
        packages = discover_packages(config, metadata)
        with self.assertRaisesRegex(PackageError, "chip ID must be between"):
            validate_and_allocate(config, packages)

    def test_unknown_text_archive_is_rejected(self) -> None:
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            source = root / "src" / "invalid.c"
            source.parent.mkdir(parents=True)
            source.write_text("")
            source.with_suffix(".text.toml").write_text('[unknown]\n"0" = "bad"\n')
            config = replace(self.config(), root=root.resolve())
            with self.assertRaisesRegex(PackageError, "unknown text archive"):
                discover_packages(config, {"invalid": []})

    def test_object_spawn_parameters_are_named_c_fields(self) -> None:
        searchman = (ROOT / "src/chips/searchman.c").read_text()
        self.assertIn(".object_spawn = { .variant = 3 }", searchman)
        abi = (ROOT / "src/abi.h").read_text()
        self.assertIn("sizeof(Exe6ChipRecord) == 0x2C", abi)
        self.assertIn("offsetof(Exe6ChipRecord, behavior) == 0x09", abi)

    def test_discovery_and_output_are_deterministic(self) -> None:
        config, first = self.packages()
        second = discover_packages(config, self.metadata[config.variant])
        self.assertEqual([package.name for package in first], [package.name for package in second])
        self.assertEqual(
            generate(config, first, validate_and_allocate(config, first)),
            generate(config, second, validate_and_allocate(config, second)),
        )
        self.assertEqual(
            generate_text_manifest(config, first),
            generate_text_manifest(config, second),
        )
        self.assertEqual(
            generate_linker_values(first, validate_and_allocate(config, first)),
            generate_linker_values(second, validate_and_allocate(config, second)),
        )

    def test_linker_symbols_match_allocations(self) -> None:
        config, packages = self.packages()
        allocations = validate_and_allocate(config, packages)
        linker = generate_linker_values(packages, allocations)
        self.assertIn(
            f"__bn67_object_id_signalred_controller_main = "
            f"0x{allocations.objects[4]['signalred_controller_main']:X};",
            linker,
        )
        group, sprite_id = allocations.sprites["signalred_battle_sprite"]
        self.assertIn(f"__bn67_sprite_group_signalred_battle_sprite = 0x{group:X};", linker)
        self.assertIn(f"__bn67_sprite_id_signalred_battle_sprite = 0x{sprite_id:X};", linker)
        self.assertIn(
            f"__bn67_song_id_signalred_spawn_song = "
            f"0x{allocations.songs['signalred_spawn_song']:X};",
            linker,
        )

    def test_generated_assembly_symbols_are_snake_case(self) -> None:
        config, packages = self.packages()
        assembly = generate(config, packages, validate_and_allocate(config, packages))
        labels = re.findall(r"(?m)^([A-Za-z_][A-Za-z0-9_]*):$", assembly)
        self.assertTrue(labels)
        for label in labels:
            self.assertRegex(label, r"^[a-z][a-z0-9]*(?:_[a-z0-9]+)*$")

if __name__ == "__main__":
    unittest.main()
