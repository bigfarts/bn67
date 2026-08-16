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
    build_compressed_archive,
    encode_description,
    encode_name,
    encode_ncp_description,
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
                + [raw["dust_sprites"]["native_table"]]
                + [raw["field_objects"]["native_table"]]
                + [raw["songs"]["native_table"]]
                + [item["native_table"] for item in raw["attack_pools"].values()]
            )
            for relative in fixed_tables:
                asset = extracted_assets[Path(relative).name]
                (cls.fixture_root / relative).write_bytes(bytes(asset.length))
            for archive in raw["text"]["archives"]:
                entry_counts = {
                    "names": (0x100, 0xA8),
                    "descriptions": (0x100, 0xA8),
                    "ncp_names": (75,),
                    "ncp_descriptions": (48,),
                }
                entry_count = entry_counts[archive["source"]][
                    archive["source_index"]
                ]
                entries = [b"\xE6"] * entry_count
                contents = (
                    build_compressed_archive(entries)
                    if archive.get("compression") == "lz77"
                    else build_archive(entries)
                )
                (cls.fixture_root / archive["binary"]).write_bytes(contents)

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
            self.assertNotIn("native_entries", raw["dust_sprites"])
            self.assertEqual(
                config.dust_sprites.native_entries,
                (
                    self.fixture_root
                    / raw["dust_sprites"]["native_table"]
                ).stat().st_size
                // 2,
            )
            self.assertNotIn("native_entries", raw["field_objects"])
            self.assertEqual(config.field_objects.base_id, 0xCD)
            self.assertEqual(config.field_objects.max_id, 0xFF)
            self.assertEqual(
                config.field_objects.native_entries,
                (
                    self.fixture_root
                    / raw["field_objects"]["native_table"]
                ).stat().st_size
                // 5,
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
                self.assertEqual(
                    archives[item["name"]].native_entries,
                    text_archive_entry_count(
                        self.fixture_root,
                        item["binary"],
                        "archive",
                        item.get("compression"),
                    ),
                )
            self.assertEqual(archives["chip-names-1"].native_entries, 0xA8)
            self.assertEqual(archives["chip-descriptions-1"].native_entries, 0xA8)
            self.assertEqual(archives["ncp-names"].native_entries, 75)
            self.assertEqual(archives["ncp-descriptions"].native_entries, 48)

    def test_binary_entry_counts_reject_malformed_tables(self) -> None:
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            (root / "fixed.bin").write_bytes(b"12345")
            with self.assertRaisesRegex(PackageError, "multiple of 4 bytes"):
                fixed_width_entry_count(root, "fixed.bin", 4, 0x100, "table")

            (root / "text.bin").write_bytes(b"\x05\x00payload")
            with self.assertRaisesRegex(PackageError, "invalid offset-table size"):
                text_archive_entry_count(root, "text.bin", "archive")

            (root / "text.lz").write_bytes(
                build_compressed_archive([b"\xE6"] * 48)
            )
            self.assertEqual(
                text_archive_entry_count(root, "text.lz", "archive", "lz77"),
                48,
            )

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
                {
                    "name": "ncp-descriptions",
                    "source": "ncp_descriptions",
                    "source_index": 0,
                    "encoding": "ncp_description",
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
                {
                    "package": "test",
                    "archive": "ncp-descriptions",
                    "index": 28,
                    "value": "Prevents\nstatus\nproblems",
                },
            ],
        }
        with tempfile.TemporaryDirectory() as directory:
            path = Path(directory) / "text.json"
            path.write_text(json.dumps(manifest))
            package_text = load_package_text(path)

        name = package_text.changes["names"][0]
        description = package_text.changes["descriptions"][0]
        ncp_description = package_text.changes["ncp-descriptions"][28]
        self.assertNotEqual(encode_name("BugCharg")[-1], RECORD_END)
        self.assertNotEqual(
            encode_description("All your\nbugs will\nattack!")[-1], RECORD_END
        )
        self.assertEqual(name[-1], RECORD_END)
        self.assertEqual(description[-1], RECORD_END)
        self.assertEqual(ncp_description[-1], RECORD_END)
        self.assertEqual(
            ncp_description[:-1],
            encode_ncp_description("Prevents\nstatus\nproblems"),
        )
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
                for address in archive.compressed_references:
                    self.assertIn(f".org 0x{address:08X}", assembly)
                    self.assertIn(
                        f".dw {archive.symbol}+0x80000000", assembly
                    )
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
            ".dw count_battle_sprite + 0x80000000 // 0x16 count_battle_sprite",
            assembly,
        )
        group_08 = assembly.split("imported_sprite_group_08_table:", 1)[1].split(
            "imported_sprite_group_08_table_end:", 1
        )[0]
        self.assertIn("count_battle_sprite", group_08)
        group_0c = assembly.split("imported_sprite_group_0c_table:", 1)[1].split(
            "imported_sprite_group_0c_table_end:", 1
        )[0]
        self.assertNotIn("count_battle_sprite", group_0c)
        self.assertIn(
            ".dw otenko_battle_sprite // 0x49 otenko_battle_sprite",
            assembly,
        )
        self.assertIn(
            ".dw falzar_battle_sprite + 0x80000000 // 0x66 falzar_battle_sprite",
            assembly,
        )
        self.assertIn(
            ".dw gregar_battle_sprite + 0x80000000 // 0x68 gregar_battle_sprite",
            assembly,
        )
        self.assertIn(
            ".dw gregar_controller_main + 1 // 0x7A gregar_controller_main",
            assembly,
        )
        self.assertIn(
            ".dw falzar_controller_main + 1 // 0x7B falzar_controller_main",
            assembly,
        )
        self.assertIn(
            ".dw gregar_shared_aux_main + 1 // 0x7D gregar_shared_aux_main",
            assembly,
        )
        self.assertIn(
            ".dw count_lance_main + 1 // 0x0D count_lance_main",
            assembly,
        )

        otenko_patch = (ROOT / "src/chips/otenko.asm").read_text()
        self.assertIn(".org 0x080DCB60", otenko_patch)
        self.assertIn(".org 0x080DB2F0", otenko_patch)
        self.assertIn(".db 0x49", otenko_patch)
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
        self.assertIn(
            "__bn67_dust_kind_signalred_battle_sprite = 0xF;",
            linker,
        )
        self.assertIn(
            "__bn67_dust_kind_rook_battle_sprite = 0xB;",
            linker,
        )
        self.assertIn(
            "__bn67_dust_kind_otenko_battle_sprite = 0xE;",
            linker,
        )
        self.assertIn("dust_sprite_table:", assembly)
        self.assertIn(
            '.incbin "build/dust-sprite-table-gregar.bin"',
            assembly,
        )
        signalred_group, signalred_id = allocations.sprites[
            "signalred_battle_sprite"
        ]
        rook_group, rook_id = allocations.sprites["rook_battle_sprite"]
        self.assertIn(
            f".byte 0x{signalred_group:02X},0x{signalred_id:02X} "
            "// 0x0F signalred_battle_sprite",
            assembly,
        )
        self.assertIn(
            f".byte 0x{rook_group:02X},0x{rook_id:02X} "
            "// 0x0B rook_battle_sprite",
            assembly,
        )
        self.assertIn(
            ".byte 0x0C,0x49 // 0x0E otenko_battle_sprite",
            assembly,
        )
        self.assertIn(".org 0x080D9DF8\n    mov r0,0x04", assembly)
        self.assertNotIn(".org 0x080DCAE2", assembly)
        self.assertNotIn("mov r1,0x1F", assembly)
        self.assertNotIn("LASERMAN_SUMMON_SONG", assembly)
        self.assertIn(
            "exe6_sound_req(BN67_SONG_ID(common_navi_summon_song))",
            (ROOT / "src/chips/laserman.c").read_text(),
        )
        self.assertNotIn(".definelabel", assembly)

    def test_dust_sprite_kinds_are_compiler_allocated(self) -> None:
        config, packages = self.packages()
        allocations = validate_and_allocate(config, packages)

        self.assertIn(
            "__bn67_meta__dust_sprite__rook_battle_sprite",
            self.metadata["gregar"]["rook"],
        )
        self.assertIn(
            "__bn67_meta__dust_sprite__signalred_battle_sprite",
            self.metadata["gregar"]["signalred"],
        )
        self.assertIn(
            "__bn67_meta__fixed_dust_sprite__0x0E__otenko_battle_sprite",
            self.metadata["gregar"]["otenko"],
        )
        self.assertFalse(
            any(
                symbol.startswith("__bn67_meta__dust_sprite__0x")
                for symbols in self.metadata["gregar"].values()
                for symbol in symbols
            )
        )
        self.assertEqual(
            allocations.dust_sprites["otenko_battle_sprite"],
            0x0E,
        )
        self.assertEqual(allocations.dust_sprites["rook_battle_sprite"], 0x0B)
        self.assertEqual(
            allocations.dust_sprites["signalred_battle_sprite"],
            0x0F,
        )

    def test_field_object_ids_are_compiler_allocated(self) -> None:
        config, packages = self.packages()
        allocations = validate_and_allocate(config, packages)
        assembly = generate(config, packages, allocations)
        linker = generate_linker_values(packages, allocations)

        self.assertIn(
            "__bn67_meta__field_object__rook_battle_sprite__4__0__1",
            self.metadata["gregar"]["rook"],
        )
        self.assertIn(
            "__bn67_meta__field_object__signalred_battle_sprite__0__0__1",
            self.metadata["gregar"]["signalred"],
        )
        self.assertEqual(allocations.field_objects["rook_battle_sprite"], 0xEC)
        self.assertEqual(
            allocations.field_objects["signalred_battle_sprite"],
            0xED,
        )
        self.assertIn(".org 0x0800F4D4\n    .dw field_object_sprite_table", assembly)
        self.assertIn(
            "0x04,0x00,0x01 // 0xEC rook_battle_sprite",
            assembly,
        )
        self.assertIn(
            "0x00,0x00,0x01 // 0xED signalred_battle_sprite",
            assembly,
        )
        self.assertIn(
            "__bn67_field_object_id_rook_battle_sprite = 0xEC;",
            linker,
        )
        self.assertIn(
            "__bn67_field_object_id_signalred_battle_sprite = 0xED;",
            linker,
        )

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

    def test_explicit_battle_sprite_priorities_are_intentional(self) -> None:
        arguments = []
        for source in (ROOT / "src/chips").rglob("*.c"):
            arguments.extend(
                (source.name, argument.strip())
                for argument in re.findall(
                    r"exe6_obj_prio_set\(\s*([^()]+?)\s*\)",
                    source.read_text(),
                )
            )

        self.assertTrue(arguments)
        foreground = [item for item in arguments if item[1] != "EXE6_OBJ_PRIORITY_BATTLE"]
        self.assertEqual(foreground, [("numberman.c", "DIE_PRIORITY")])

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
        fixed_attacks = [
            item for package in packages for item in package.fixed_attacks
        ]
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
        for attack in fixed_attacks:
            self.assertIn(
                f".dw {attack.main} + 1 // 0x{attack.subfamily:02X} "
                f"{attack.main}",
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
            len(attacks) + len(fixed_attacks),
        )
        for pool in config.attack_pools.values():
            allocated = [
                allocation.subfamily
                for allocation in allocations.attacks.values()
                if allocation.family == pool.family
            ]
            if allocated:
                next_id = max(allocated) + 1
                fixed_ids = {
                    attack.subfamily
                    for attack in fixed_attacks
                    if attack.family == pool.family
                }
                if next_id not in fixed_ids:
                    label = f"attack_family_{pool.family:02x}_table"
                    section = assembly.split(f"{label}:", 1)[1].split(
                        f"{label}_end:", 1
                    )[0]
                    self.assertNotIn(f"// 0x{next_id:02X}", section)
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
        numberman = next(package for package in packages if package.name == "numberman")
        self.assertIsNotNone(numberman.attack)
        self.assertEqual(numberman.attack.kind, "summon_attack")
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

    def test_blackweapon_replaces_delta_ray_slot(self) -> None:
        source = (ROOT / "src/chips/blackweapon.c").read_text()
        self.assertIn("BN67_CHIP_RECORD(0x12f)", source)
        self.assertIn(
            "BN67_PERSISTENT_ATTACK(0x12f, blackweapon_attack_main)",
            source,
        )
        self.assertIn(".library_number = 3", source)
        self.assertIn(".library_sort_order = 0x012f", source)
        self.assertNotIn("BN67_CHIP_RECORD(0x12d)", source)

        for variant in ("gregar", "falzar"):
            _, packages = self.packages(variant)
            blackweapon = next(
                package for package in packages if package.name == "blackweapon"
            )
            self.assertEqual([chip.chip_id for chip in blackweapon.chips], [0x12F])
            self.assertEqual(blackweapon.attack.chip_id, 0x12F)
            text_entries = {
                (entry.archive, entry.index): entry.value
                for entry in blackweapon.text
            }
            self.assertEqual(text_entries[("chip-names-1", 0x2F)], "BlakWeap")
            self.assertIn(("chip-descriptions-1", 0x2F), text_entries)

    def test_giga_replacements_preserve_library_slots(self) -> None:
        chaoslord = (ROOT / "src/chips/chaoslord.c").read_text()
        self.assertIn("BN67_CHIP_RECORD(0x12e)", chaoslord)
        self.assertIn(".library_number = 0x02", chaoslord)
        self.assertIn(".library_flags = 0x18", chaoslord)
        self.assertIn(".library_lock_on_type = 0x00", chaoslord)

        deathphoenix = (ROOT / "src/chips/deathphoenix.c").read_text()
        self.assertIn("BN67_CHIP_RECORD(0x134)", deathphoenix)
        self.assertIn(".library_number = 0x03", deathphoenix)
        self.assertIn(".library_flags = 0x18", deathphoenix)
        self.assertIn(".library_lock_on_type = 0x00", deathphoenix)

    def test_folderback_replaces_color_point_slot(self) -> None:
        source = (ROOT / "src/chips/folderback.c").read_text()
        self.assertIn("BN67_CHIP_RECORD(0x0c2)", source)
        self.assertIn(
            "BN67_PERSISTENT_ATTACK(0x0c2, folderback_attack_main)",
            source,
        )
        self.assertNotIn("BN67_CHIP_RECORD(0x0c6)", source)
        self.assertIn(".library_number = 0xC7", source)
        self.assertIn(".library_flags = 0x00", source)
        self.assertIn(".library_sort_order = 0x00C7", source)

        for variant in ("gregar", "falzar"):
            _, packages = self.packages(variant)
            folderback = next(
                package for package in packages if package.name == "folderback"
            )
            self.assertEqual([chip.chip_id for chip in folderback.chips], [0x0C2])
            self.assertEqual(folderback.attack.chip_id, 0x0C2)
            text_entries = {
                (entry.archive, entry.index): entry.value
                for entry in folderback.text
            }
            self.assertEqual(text_entries[("chip-names-0", 0xC2)], "FoldrBak")
            self.assertIn(("chip-descriptions-0", 0xC2), text_entries)

    def test_numberman_replaces_chargeman_slots(self) -> None:
        source = (ROOT / "src/chips/numberman.c").read_text()
        self.assertEqual(source.count("BN67_CHIP_RECORD("), 3)
        self.assertIn("BN67_CHIP_RECORD(0x0ef)", source)
        self.assertIn("BN67_CHIP_RECORD(0x0f0)", source)
        self.assertIn("BN67_CHIP_RECORD(0x0f1)", source)
        self.assertIn(
            "BN67_SUMMON_ATTACK(0x0ef, numberman_attack_main)",
            source,
        )
        self.assertIn("static const uint8_t DIE_ROLLS[16]", source)
        self.assertIn("EXE6_HIT_REGION_CENTERED_3X3", source)
        self.assertIn("EXE6_HIT_REGION_CURRENT_BLOCK", source)
        self.assertIn("die->attack = actor->attack", source)
        self.assertIn(
            "multiply_attack(self->attack, self->variant)",
            source,
        )
        self.assertIn("numberman_explosion_song", source)
        self.assertIn(".short 0x7EF4,0x75E9,0x44C1", source)
        self.assertIn("static const uint16_t FLIGHT_FRAMES = 43", source)
        self.assertIn("static const uint16_t APPEAR_FRAMES = 4", source)
        self.assertIn("static const int32_t DIE_INITIAL_Z_VELOCITY = 4 << 16", source)
        self.assertIn("static const int32_t DIE_GRAVITY = -0x3000", source)
        self.assertIn("exe6_obj_prio_set(DIE_PRIORITY)", source)
        self.assertIn("BN67_SPRITE_GROUP(numberman_battle_sprite)", source)

        for variant in ("gregar", "falzar"):
            _, packages = self.packages(variant)
            numberman = next(
                package for package in packages if package.name == "numberman"
            )
            self.assertEqual(
                [chip.chip_id for chip in numberman.chips],
                [0x0EF, 0x0F0, 0x0F1],
            )
            self.assertEqual(numberman.attack.chip_id, 0x0EF)
            text_entries = {
                (entry.archive, entry.index): entry.value
                for entry in numberman.text
            }
            self.assertEqual(
                text_entries[("chip-names-0", 0xEF)],
                "NumbrMan",
            )
            self.assertEqual(
                text_entries[("chip-descriptions-0", 0xEF)],
                "Bomb 3\nahead!\nHits 9sq",
            )

    def test_django_restores_bn5_base_attack_in_missing_slots(self) -> None:
        source = (ROOT / "src/chips/django.c").read_text()
        self.assertIn(
            "BN67_SUMMON_ATTACK(0x116, django_attack_main)",
            source,
        )
        self.assertIn("EXE6_HIT_REGION_CENTERED_3X3", source)
        self.assertIn("find_target_in_row", source)
        self.assertIn("django_coffin_sprite", source)
        self.assertIn("django_sun_sprite", source)
        self.assertIn(
            "BN67_PATCH_SPRITE_LOAD(0x080BF4AA, django_battle_sprite)",
            source,
        )
        self.assertIn(
            "BN67_PATCH_SPRITE_LOAD(0x080BDC3A, django_battle_sprite)",
            source,
        )
        self.assertNotIn("BN67_PATCH_SPRITE_LOAD(0x080BFD6C", source)
        self.assertNotIn("BN67_PATCH_SPRITE_LOAD(0x080BE4FC", source)
        self.assertIn("LIGHT_VARIANT_CHARGE", source)
        self.assertIn("LIGHT_VARIANT_SUN_CONTROLLER", source)
        self.assertIn("SUNLIGHT_ORBIT_STEP = 8", source)
        self.assertIn("BUILDUP_LAST_FRAME = 60", source)
        self.assertIn("ROCK_SPAWN_FRAME = 20", source)
        self.assertIn("CHARGE_SOUND_PERIOD = 16", source)
        self.assertIn("exe6_camera_quake_set(2, 30)", source)
        self.assertIn("exe6_sound_req(0xE5)", source)
        self.assertIn("exe6_sound_req(0x149)", source)
        self.assertIn("{-1, 0}, {1, 0}, {0, -1}, {0, 1}", source)
        self.assertIn("exe6_obj_shadow_set()", source)
        self.assertIn("exe6_obj_spawn_with_variant(2)", source)
        self.assertIn("django2_palette", source)
        self.assertIn("django3_palette", source)
        self.assertIn(".short 0x6BF7,0x5BAD,0x4743", source)
        self.assertIn("self->z = 35 << 16", source)
        self.assertIn("self->z = 24 << 16", source)
        self.assertIn("GREGAR_ROCK_SPRITE_GROUP = 0x10", source)
        self.assertIn("GREGAR_ROCK_SPRITE_ID = 0x05", source)
        self.assertIn("EXE6_HIT_TYPE_17", source)
        self.assertIn("hide_target", source)
        self.assertIn(".object_spawn = {0}", source)
        self.assertNotIn("EXE6_CHIP_EFFECT_FLAG_DYNAMIC_POWER", source)
        self.assertIn(
            "0x116, EXE6_CHIP_CODE_ASTERISK, 2, 30, 130, django_palette",
            source,
        )
        self.assertIn(
            "0x117, EXE6_CHIP_CODE_NONE, 3, 70, 180, django2_palette",
            source,
        )
        self.assertIn(
            "0x118, EXE6_CHIP_CODE_NONE, 4, 90, 260, django3_palette",
            source,
        )

        for variant in ("gregar", "falzar"):
            config, packages = self.packages(variant)
            django = next(
                package for package in packages if package.name == "django"
            )
            self.assertEqual(
                [chip.chip_id for chip in django.chips],
                [0x116, 0x117, 0x118],
            )
            self.assertEqual(django.attack.chip_id, 0x116)
            expected_address = (
                0x080BDC3A if variant == "falzar" else 0x080BF4AA
            )
            self.assertEqual(
                [
                    (patch.address, patch.archive)
                    for patch in django.sprite_load_patches
                ],
                [(expected_address, "django_battle_sprite")],
            )
            allocations = validate_and_allocate(config, packages)
            group, sprite_id = allocations.sprites["django_battle_sprite"]
            assembly = generate(config, packages, allocations)
            self.assertIn(
                f".org 0x{expected_address:08X}\n"
                f"    mov r1,0x{group:X}\n"
                f"    mov r2,0x{sprite_id:X}",
                assembly,
            )
            text_entries = {
                (entry.archive, entry.index): entry.value
                for entry in django.text
            }
            self.assertEqual(
                text_entries[("chip-names-1", 0x16)],
                "Django",
            )
            self.assertEqual(
                text_entries[("chip-descriptions-1", 0x16)],
                "Burns\nwith\nsunlight",
            )

    def test_protoman_uses_native_delta_ray(self) -> None:
        source = (ROOT / "src/chips/protoman.c").read_text()
        self.assertEqual(source.count("BN67_CHIP_RECORD("), 3)
        self.assertIn("BN67_CHIP_RECORD(0x0e0)", source)
        self.assertIn("BN67_CHIP_RECORD(0x0e1)", source)
        self.assertIn("BN67_CHIP_RECORD(0x0e2)", source)
        self.assertEqual(source.count(".family = DELTARAY_FAMILY"), 3)
        self.assertEqual(source.count(".subfamily = DELTARAY_SUBFAMILY"), 3)
        self.assertRegex(source, r"(?m)^    \.power = 80,$")
        self.assertRegex(source, r"(?m)^    \.power = 100,$")
        self.assertRegex(source, r"(?m)^    \.power = 200,$")
        self.assertEqual(
            source.count(
                ".object_spawn = { .animation_state = "
                "DELTARAY_ANIMATION_STATE }"
            ),
            3,
        )

        for variant in ("gregar", "falzar"):
            config, packages = self.packages(variant)
            protoman = next(
                package for package in packages if package.name == "protoman"
            )
            self.assertEqual(
                [chip.chip_id for chip in protoman.chips],
                [0x0E0, 0x0E1, 0x0E2],
            )
            self.assertEqual(
                {
                    (entry.archive, entry.index, entry.value)
                    for entry in protoman.text
                },
                {
                    ("chip-descriptions-0", index, "A button\npower up\nby 3 swrd")
                    for index in range(0xE0, 0xE3)
                }
                | {
                    ("chip-names-0", 0xE1, "ProtoMn2"),
                    ("chip-names-0", 0xE2, "ProtoMn3"),
                },
            )
            assembly = "\n".join(emit_chip_records(config, packages))
            for chip_id in range(0x0E0, 0x0E3):
                self.assertIn(
                    f"bn67_chip_record_0x{chip_id:03x}",
                    assembly,
                )

    def test_colonel_uses_native_cross_divide(self) -> None:
        source = (ROOT / "src/chips/colonel.c").read_text()
        self.assertEqual(source.count("BN67_CHIP_RECORD("), 3)
        self.assertIn("BN67_CHIP_RECORD(0x110)", source)
        self.assertIn("BN67_CHIP_RECORD(0x111)", source)
        self.assertIn("BN67_CHIP_RECORD(0x112)", source)
        self.assertEqual(source.count(".family = FAMILY"), 3)
        self.assertEqual(source.count(".subfamily = SUBFAMILY"), 3)
        self.assertEqual(
            source.count(
                ".object_spawn = { .animation_state = "
                "CROSSDIVIDE_ANIMATION_STATE }"
            ),
            3,
        )

        for variant in ("gregar", "falzar"):
            config, packages = self.packages(variant)
            colonel = next(
                package for package in packages if package.name == "colonel"
            )
            self.assertEqual(
                [chip.chip_id for chip in colonel.chips],
                [0x110, 0x111, 0x112],
            )
            self.assertEqual(
                {
                    (entry.archive, entry.index, entry.value)
                    for entry in colonel.text
                },
                {
                    ("chip-descriptions-1", index, "Cross-\nslice!")
                    for index in range(0x10, 0x13)
                }
                | {
                    ("chip-names-1", 0x11, "Colonel2"),
                    ("chip-names-1", 0x12, "Colonel3"),
                },
            )
            assembly = "\n".join(emit_chip_records(config, packages))
            for chip_id in range(0x110, 0x113):
                self.assertIn(
                    f"bn67_chip_record_0x{chip_id:03x}",
                    assembly,
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
            label = f"object_class_{number}_table"
            object_table = assembly.split(f"{label}:", 1)[1].split(
                f"{label}_end:", 1
            )[0]
            if namespace:
                self.assertNotIn(
                    f"// 0x{max(namespace.values()) + 1:02X}", object_table
                )
            self.assertIn(f"{label}:", assembly)
            self.assertIn(f"{label}_end:", assembly)
            object_class = config.object_classes[number]
            self.assertIn(f'.incbin "{object_class.native_table}"', assembly)
            for address in object_class.references:
                self.assertIn(f".org 0x{address:08X}", assembly)

        fixed_objects = [
            item for package in packages for item in package.fixed_objects
        ]
        self.assertEqual(len(fixed_objects), 8)
        for item in fixed_objects:
            self.assertIn(
                f".dw {item.main} + 1 // 0x{item.object_id:02X} {item.main}",
                assembly,
            )

        self.assertNotIn("object_class_1_dispatch_table", assembly)
        self.assertNotIn("object_dispatch_interceptor_main:", assembly)
        self.assertIn("// Package-declared fixed section patches.", assembly)
        self.assertIn(
            ".org falzar_controller_main + 0x302\n"
            "    bl falzar_strike_feather_spawn_with_bonus",
            assembly,
        )
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
            ".org 0x0800819A\n"
            "    push {r1}\n"
            "    bl section_patch_folderback_custom_transition_dispatch_relay\n"
            ".org 0x080E42C8\n"
            "section_patch_folderback_custom_transition_dispatch_relay:",
            assembly,
        )
        self.assertIn(
            "ldr r1,=folderback_custom_transition_dispatch + 1",
            assembly,
        )
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
        self.assertIn("clear_chip_use_counts();", folderback)
        self.assertIn(
            "*EXE6_USED_CHIP_CLASS_COUNTS = (Exe6ChipClassUseCounts){0};",
            folderback,
        )
        self.assertNotIn("sizeof(*selection)", folderback)
        self.assertIn("const Exe6ObjectSlot *slots = EXE6_EFFECT_POOL_HEAD", folderback)
        self.assertIn("EXE6_POOL_SLOT_COUNT", folderback)
        self.assertIn("object->object_class", folderback)
        self.assertIn("EXE6_OBJECT_CLASS_ENEMY", folderback)
        self.assertIn("EXE6_OBJECT_CLASS_SHELL", folderback)
        self.assertNotIn("exe6_navi_status_work_adrs_get", folderback)
        self.assertNotIn("EXE6_NAVI_TRANSFORMATION", folderback)
        self.assertNotIn("exe6_event_chip_state_get", folderback)
        self.assertNotIn("EXE6_HIT_STATUS_FLAG_BEAST_OVER", folderback)
        self.assertNotIn("exe6_event_chip_state_reset(owner ^ 1u)", folderback)
        self.assertIn("folderback_coalesce_native_custom", folderback)
        self.assertIn("restore_folder_once(controller);", folderback)
        self.assertIn("delete_controller(controller);", folderback)
        self.assertIn("->folder_restored = false;", folderback)
        self.assertIn(
            "BN67_PATCH_SECTION(0x0800819A, 0x080E42C8,",
            folderback,
        )
        controller_start = folderback.index(
            "BN67_EFFECT(folderback_controller_main)"
        )
        controller_end = folderback.index(
            "BN67_PERSISTENT_ATTACK", controller_start
        )
        controller = folderback[controller_start:controller_end]
        self.assertNotIn("exe6_cockpit_pause_set();", controller)
        self.assertIn("hold_local_custom_gauge(self);", controller)
        held_gauge_capture = "exe6_cockpit_get_custom_gauge_value();"
        self.assertIn(held_gauge_capture, controller)
        self.assertNotIn("local_operation_work", folderback)
        self.assertNotIn("exe6_op_work_adrs_get", folderback)
        self.assertLess(
            controller.index(held_gauge_capture),
            controller.index("hold_local_custom_gauge(self)"),
        )
        self.assertLess(
            controller.index("hold_local_custom_gauge(self);"),
            controller.index("effect_update(self)"),
        )
        self.assertIn("work->held_custom_gauge = FULL_GAUGE;", folderback)
        self.assertNotIn("player + 0x28", folderback)
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
        self.assertIn(
            "uint32_t exe6_cockpit_get_custom_gauge_value(void);", abi
        )
        self.assertNotIn("exe6_op_work_adrs_get", abi)
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
        self.assertIn("BN67_PATCH_LINKED_CALL", runtime)
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
            "__bn67_dust_kind_signalred_battle_sprite = 0xF;",
            linker,
        )
        self.assertIn(
            "__bn67_dust_kind_rook_battle_sprite = 0xB;",
            linker,
        )
        self.assertIn(
            "__bn67_dust_kind_otenko_battle_sprite = 0xE;",
            linker,
        )
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
