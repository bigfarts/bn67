import json
from dataclasses import replace
from pathlib import Path
import re
import subprocess
import sys
import tempfile
import tomllib
import unittest

from build_text_archives import build_archive
from compile_registry import (
    PackageError,
    discover_packages,
    emit_attack_tables,
    emit_chip_records,
    emit_object_tables,
    emit_text_archives,
    fixed_width_entry_count,
    generate,
    generate_linker_values,
    generate_text_manifest,
    load_config,
    load_metadata,
    parse_chip_record,
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
        self.assertIn("__exe6_object_id_searchman_reticle_main =", linker)
        self.assertIn("__exe6_sprite_id_searchman_reticle_sprite =", linker)
        self.assertIn("__exe6_sprite_group_searchman_reticle_sprite =", linker)
        self.assertIn(".dw searchman_reticle_sprite", assembly)
        self.assertIn(
            f"__exe6_song_id_common_navi_summon_song = "
            f"0x{allocations.songs['common_navi_summon_song']:X};",
            linker,
        )
        self.assertIn(
            f"__exe6_song_group_common_navi_summon_song = "
            f"0x{allocations.song_players['common_navi_summon_song']:X};",
            linker,
        )
        self.assertNotIn("LASERMAN_SUMMON_SONG", assembly)
        self.assertNotIn("ROLLARROW_SUMMON_SONG", assembly)
        self.assertIn(
            "exe6_sound_req(EXE6_SONG_ID(common_navi_summon_song))",
            (ROOT / "src/laserman.c").read_text(),
        )
        self.assertIn(
            "EXE6_SONG_ID(common_navi_summon_song)",
            (ROOT / "src/rollarrow.c").read_text(),
        )
        self.assertNotIn(".definelabel", assembly)

    def test_flat_sources_and_chip_definition_files(self) -> None:
        self.assertFalse((ROOT / "packages").exists())
        self.assertFalse(list((ROOT / "src").glob("manifest.toml")))
        for path in ROOT.glob("src/*.defs.toml"):
            definitions = tomllib.loads(path.read_text())
            self.assertTrue(definitions, path)
            self.assertLessEqual(set(definitions), {"chips", "text"}, path)
            for chip_id, chip in definitions.get("chips", {}).items():
                self.assertTrue(chip_id.startswith("0x"), path)
                self.assertIsInstance(chip, dict, path)
                behavior = chip.get("behavior", {})
                self.assertNotIn("family", behavior, path)
                self.assertNotIn("subfamily", behavior, path)
            for archive, entries in definitions.get("text", {}).items():
                self.assertIsInstance(archive, str, path)
                self.assertIsInstance(entries, dict, path)
        for source in ROOT.glob("src/*.c"):
            text = source.read_text()
            self.assertNotIn(".generated.h", text, source)
            self.assertNotIn("EXE6_PCM_SONG", text, source)

    def test_chip_records_are_semantic_definition_resources(self) -> None:
        gregar_config, gregar_packages = self.packages("gregar")
        falzar_config, falzar_packages = self.packages("falzar")
        allocations = validate_and_allocate(gregar_config, gregar_packages)
        assembly = "\n".join(
            emit_chip_records(gregar_config, gregar_packages, allocations)
        )
        falzar_assembly = "\n".join(
            emit_chip_records(
                falzar_config,
                falzar_packages,
                validate_and_allocate(falzar_config, falzar_packages),
            )
        )

        bugcharge = next(
            chip
            for package in gregar_packages if package.name == "bugcharge"
            for chip in package.chips
        )
        falzar_bugcharge = next(
            chip
            for package in falzar_packages if package.name == "bugcharge"
            for chip in package.chips
        )
        self.assertEqual(bugcharge.chip_id, 0x131)
        self.assertEqual(dict(bugcharge.common)["codes"], (0x01, 0xFF, 0xFF, 0xFF))
        self.assertEqual(dict(bugcharge.common)["class"], 0x02)
        self.assertEqual(dict(bugcharge.common)["behavior.counter_settings"], 0x8B)
        self.assertEqual(dict(bugcharge.override)["behavior.effect_flags"], 0x41)
        self.assertEqual(
            dict(falzar_bugcharge.override)["behavior.effect_flags"], 0x01
        )
        self.assertEqual(dict(bugcharge.override)["artwork.icon"], "bugcharge_icon")
        self.assertNotIn("artwork.icon", dict(falzar_bugcharge.override))

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
            ("All your", "bugs will", "attack!"),
        )

        self.assertIn("// bugcharge: chip 0x131", assembly)
        self.assertIn(".db 0x8B // behavior.counter_settings", assembly)
        bugcharge_attack = allocations.attacks["bugcharge_attack_main"]
        self.assertIn(f".db 0x{bugcharge_attack.family:02X} // behavior.family", assembly)
        self.assertIn(
            f".db 0x{bugcharge_attack.subfamily:02X} // behavior.subfamily",
            assembly,
        )
        searchman_ex = next(
            chip
            for package in gregar_packages
            if package.name == "searchman"
            for chip in package.chips
            if chip.chip_id == 0x108
        )
        self.assertEqual(
            dict(searchman_ex.common)["behavior.object_spawn"],
            (3, 0, 0, 0),
        )
        self.assertIn(
            ".db 0x03,0x00,0x00,0x00 // behavior.object_spawn",
            assembly,
        )
        self.assertEqual(assembly.count(".dw 0x08729D50 // artwork.icon"), 6)
        self.assertEqual(falzar_assembly.count(".dw 0x0872BE14 // artwork.icon"), 6)
        self.assertIn(".dw bugcharge_icon // artwork.icon", assembly)
        self.assertNotIn(".dw 0x0872C594 // artwork.icon", assembly)
        for path in ROOT.glob("src/*.c"):
            self.assertNotIn("CHIP_DATA", path.read_text(), path)

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
        assembly = "\n".join(emit_object_tables(config, packages, allocations))

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
                self.assertNotIn(f"// 0x{max(namespace.values()) + 1:02X}", assembly)
            label = f"object_class_{number}_table"
            self.assertIn(f"{label}:", assembly)
            self.assertIn(f"{label}_end:", assembly)
            object_class = config.object_classes[number]
            self.assertIn(f'.incbin "{object_class.native_table}"', assembly)
            for address in object_class.references:
                self.assertIn(f".org 0x{address:08X}", assembly)

        self.assertNotIn("object_class_1_dispatch_table", assembly)
        self.assertIn("object_dispatch_interceptor_main:", assembly)
        self.assertIn("ldr r1,=folderback_type_1_main + 1", assembly)
        self.assertIn(
            f".org 0x{config.object_dispatch.hook_address:08X}", assembly
        )
        folderback = (ROOT / "src/folderback.c").read_text()
        self.assertNotIn("EXE6_POINTER_PATCH(0x08003224", folderback)
        self.assertNotIn("__exe6_object_kind", folderback)
        self.assertIn("__exe6_object_id_folderback_controller_main", folderback)
        self.assertNotIn("0x08003C9C", folderback)

    def test_invalid_semantic_chip_code_is_rejected(self) -> None:
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            source = root / "src" / "invalid.c"
            source.parent.mkdir(parents=True)
            source.write_text("")
            source.with_suffix(".defs.toml").write_text(
                '[chips."0x001"]\ncodes = ["AA"]\n'
            )
            config = replace(self.config(), root=root.resolve())
            with self.assertRaisesRegex(PackageError, "codes must contain"):
                discover_packages(config, {"invalid": []})

    def test_object_spawn_parameters_are_named_fields(self) -> None:
        fields = dict(
            parse_chip_record(
                {
                    "behavior": {
                        "object_spawn": {
                            "variant": 3,
                            "animation_state": 5,
                        }
                    }
                },
                "test chip",
            )
        )
        self.assertEqual(fields["behavior.object_spawn"], (3, 0, 5, 0))

        with self.assertRaisesRegex(PackageError, r"unknown field\(s\): parameters"):
            parse_chip_record(
                {"behavior": {"parameters": [3, 0, 0, 0]}},
                "test chip",
            )

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
            f"__exe6_object_id_signalred_controller_main = "
            f"0x{allocations.objects[4]['signalred_controller_main']:X};",
            linker,
        )
        group, sprite_id = allocations.sprites["signalred_battle_sprite"]
        self.assertIn(f"__exe6_sprite_group_signalred_battle_sprite = 0x{group:X};", linker)
        self.assertIn(f"__exe6_sprite_id_signalred_battle_sprite = 0x{sprite_id:X};", linker)
        self.assertIn(
            f"__exe6_song_id_signalred_spawn_song = "
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
