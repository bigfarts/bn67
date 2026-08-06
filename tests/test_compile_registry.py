import json
from dataclasses import replace
from pathlib import Path
import subprocess
import sys
import tempfile
import tomllib
import unittest

from compile_registry import (
    PackageError,
    discover_packages,
    emit_attack_routes,
    emit_chip_records,
    emit_text_archives,
    generate,
    generate_linker_values,
    generate_text_manifest,
    load_metadata,
    static_config,
    validate_and_allocate,
)


ROOT = Path(__file__).resolve().parents[1]


class PackageCompilerTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls) -> None:
        cls.temporary = tempfile.TemporaryDirectory()
        cls.metadata_path = Path(cls.temporary.name) / "metadata.json"
        subprocess.run(
            [
                sys.executable,
                str(ROOT / "compile_c_metadata.py"),
                "--output",
                str(cls.metadata_path),
            ],
            check=True,
        )
        cls.metadata = load_metadata(cls.metadata_path)

    def test_runtime_sources_are_not_packages(self) -> None:
        self.assertNotIn("abi", self.metadata)
        self.assertNotIn("runtime", self.metadata)

    @classmethod
    def tearDownClass(cls) -> None:
        cls.temporary.cleanup()

    def packages(self):
        config = static_config(ROOT)
        return config, discover_packages(config, self.metadata)

    def test_static_config_is_loaded_from_edition_toml(self) -> None:
        gregar = tomllib.loads((ROOT / "config.gregar.toml").read_text())
        falzar = tomllib.loads((ROOT / "config.falzar.toml").read_text())
        config = static_config(ROOT)

        self.assertNotIn("manifest_globs", gregar)
        self.assertNotIn("manifest_globs", falzar)
        self.assertEqual(config.songs.gregar_references, tuple(gregar["songs"]["references"]))
        self.assertEqual(config.songs.falzar_references, tuple(falzar["songs"]["references"]))
        self.assertEqual(
            config.text.gregar_folder_edit_skip,
            (gregar["text"]["folder_edit_skip_address"], gregar["text"]["folder_edit_skip_target"]),
        )
        self.assertEqual(
            config.text.falzar_folder_edit_skip,
            (falzar["text"]["folder_edit_skip_address"], falzar["text"]["folder_edit_skip_target"]),
        )
        self.assertEqual(config.chips.table_address, gregar["chips"]["table_address"])
        self.assertEqual(gregar["chips"], falzar["chips"])

    def test_text_archive_layout_comes_from_static_config(self) -> None:
        config, packages = self.packages()
        assembly = "\n".join(emit_text_archives(config))
        manifest = json.loads(generate_text_manifest(config, packages))

        configured_names = set()
        for group in config.text.groups:
            for archive in group.archives:
                configured_names.add(archive.name)
                self.assertIn(f"{archive.symbol}:", assembly)
                self.assertIn(f'.incbin "{archive.binary}"', assembly)
                for address in archive.gregar_references + archive.falzar_references:
                    self.assertIn(f".org 0x{address:08X}", assembly)
                    self.assertIn(f".dw {archive.symbol}", assembly)
        self.assertEqual(
            configured_names,
            {archive["name"] for archive in manifest["archives"]},
        )
        for entry in manifest["entries"]:
            self.assertEqual(set(entry), {"package", "archive", "index", "value"})
            self.assertIn(entry["archive"], configured_names)

    def test_conventions_and_included_package_deduplication(self) -> None:
        config, packages = self.packages()
        allocations = validate_and_allocate(config, packages)
        assembly = generate(config, packages, allocations)
        linker = generate_linker_values(packages, allocations)

        self.assertNotIn('.include "packages/', assembly)
        package_includes = {package.name: package.includes for package in packages}
        self.assertEqual(package_includes["rollarrow"], ("common",))
        self.assertEqual(package_includes["laserman"], ("common",))
        self.assertIn(".dw searchman_reticle_main + 1", assembly)
        self.assertIn("__bn6_object_kind_searchman_reticle_main =", linker)
        self.assertIn("__bn6_sprite_id_searchman_reticle_sprite =", linker)
        self.assertIn("__bn6_sprite_group_searchman_reticle_sprite =", linker)
        self.assertIn(".dw searchman_reticle_sprite", assembly)
        self.assertIn(
            f"__bn6_song_id_CommonNaviSummonSong = "
            f"0x{allocations.songs['CommonNaviSummonSong']:X};",
            linker,
        )
        self.assertIn(
            f"__bn6_song_group_CommonNaviSummonSong = "
            f"0x{allocations.song_players['CommonNaviSummonSong']:X};",
            linker,
        )
        self.assertNotIn("LASERMAN_SUMMON_SONG", assembly)
        self.assertNotIn("ROLLARROW_SUMMON_SONG", assembly)
        self.assertIn(
            "bn6_play_sound(BN6_SONG_ID(CommonNaviSummonSong))",
            (ROOT / "src/laserman.c").read_text(),
        )
        self.assertIn(
            "BN6_SONG_ID(CommonNaviSummonSong)",
            (ROOT / "src/rollarrow.c").read_text(),
        )
        self.assertIn("BN6_INCLUDE(common);", (ROOT / "src/rollarrow.c").read_text())
        self.assertIn("BN6_INCLUDE(common);", (ROOT / "src/laserman.c").read_text())
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
            for archive, entries in definitions.get("text", {}).items():
                self.assertIsInstance(archive, str, path)
                self.assertIsInstance(entries, dict, path)
        for source in ROOT.glob("src/*.c"):
            self.assertNotIn(".generated.h", source.read_text(), source)

    def test_chip_records_are_semantic_definition_resources(self) -> None:
        config, packages = self.packages()
        validate_and_allocate(config, packages)
        assembly = "\n".join(emit_chip_records(config, packages))

        bugcharge = next(
            chip
            for package in packages if package.name == "bugcharge"
            for chip in package.chips
        )
        self.assertEqual(bugcharge.chip_id, 0x131)
        self.assertEqual(dict(bugcharge.common)["codes"], (0x01, 0xFF, 0xFF, 0xFF))
        self.assertEqual(dict(bugcharge.common)["class"], 0x02)
        self.assertEqual(dict(bugcharge.common)["behavior.counter_settings"], 0x8B)
        self.assertEqual(dict(bugcharge.gregar)["behavior.effect_flags"], 0x41)
        self.assertEqual(dict(bugcharge.falzar)["behavior.effect_flags"], 0x01)
        self.assertEqual(dict(bugcharge.gregar)["artwork.icon"], "BugchargeIcon")
        self.assertNotIn("artwork.icon", dict(bugcharge.falzar))

        bugcharge_package = next(package for package in packages if package.name == "bugcharge")
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
        self.assertIn(".dw BugchargeIcon // artwork.icon", assembly)
        self.assertNotIn(".dw 0x0872C594 // artwork.icon", assembly)
        for path in ROOT.glob("src/*.c"):
            self.assertNotIn("CHIP_DATA", path.read_text(), path)

    def test_attack_routes_are_compiler_owned(self) -> None:
        config, packages = self.packages()
        assembly = "\n".join(emit_attack_routes(config, packages))

        self.assertIn(".org 0x0802CD4C", assembly)
        self.assertIn(".dw AttackRoute15_26 + 1", assembly)
        self.assertIn("cmp r0,0x00", assembly)
        self.assertIn("ldr r3,=signalred_attack_main + 1", assembly)
        self.assertIn("cmp r0,0x8B", assembly)
        self.assertIn("ldr r3,=bugcharge_attack_main + 1", assembly)
        self.assertIn("0x080EAADC + 1", assembly)
        self.assertIn("0x080E979C + 1", assembly)

        self.assertIn(".org 0x0802CD94", assembly)
        self.assertIn("cmp r0,0x8A", assembly)
        self.assertIn("ldr r3,=searchman_attack_main + 1", assembly)
        self.assertIn("cmp r0,0x94", assembly)
        self.assertIn("ldr r3,=rollarrow_attack_main + 1", assembly)
        self.assertIn("0x080BBED0 + 1", assembly)
        self.assertIn("0x080BA660 + 1", assembly)

        for package in packages:
            source = (ROOT / package.source).read_text()
            self.assertNotRegex(source, r"(?m)^\.org 0x0802C(?:CD|D[A-F0-9])")
        searchman = next(package for package in packages if package.name == "searchman")
        self.assertIsNotNone(searchman.attack)
        self.assertFalse(hasattr(searchman.attack, "default"))

        without_bugcharge = [package for package in packages if package.name != "bugcharge"]
        single_route_assembly = "\n".join(emit_attack_routes(config, without_bugcharge))
        self.assertIn(".dw AttackRoute15_26 + 1", single_route_assembly)
        self.assertIn("ldr r3,=signalred_attack_main + 1", single_route_assembly)
        self.assertIn("0x080EAADC + 1", single_route_assembly)
        self.assertIn("0x080E979C + 1", single_route_assembly)

    def test_invalid_semantic_chip_code_is_rejected(self) -> None:
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            source = root / "src" / "invalid.c"
            source.parent.mkdir(parents=True)
            source.write_text("")
            source.with_suffix(".defs.toml").write_text(
                '[chips."0x001"]\ncodes = ["AA"]\n'
            )
            config = replace(static_config(ROOT), root=root.resolve())
            with self.assertRaisesRegex(PackageError, "codes must contain"):
                discover_packages(config, {"invalid": []})

    def test_discovery_and_output_are_deterministic(self) -> None:
        config, first = self.packages()
        second = discover_packages(config, self.metadata)
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
            f"__bn6_object_kind_signalred_controller_main = "
            f"0x{allocations.objects[4]['signalred_controller_main']:X};",
            linker,
        )
        group, sprite_id = allocations.sprites["signalred_battle_sprite"]
        self.assertIn(f"__bn6_sprite_group_signalred_battle_sprite = 0x{group:X};", linker)
        self.assertIn(f"__bn6_sprite_id_signalred_battle_sprite = 0x{sprite_id:X};", linker)
        self.assertIn(
            f"__bn6_song_id_SignalredSpawnSong = "
            f"0x{allocations.songs['SignalredSpawnSong']:X};",
            linker,
        )

    def test_include_cycles_are_rejected(self) -> None:
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            for name, dependency in (("a", "b"), ("b", "a")):
                source = root / "src" / f"{name}.c"
                source.parent.mkdir(parents=True, exist_ok=True)
                source.write_text("")
            metadata = {
                "a": ["__bn6_meta__include__b"],
                "b": ["__bn6_meta__include__a"],
            }
            with self.assertRaisesRegex(PackageError, "source include cycle"):
                discover_packages(
                    replace(static_config(ROOT), root=root.resolve()), metadata
                )

if __name__ == "__main__":
    unittest.main()
