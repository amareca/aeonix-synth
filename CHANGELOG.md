# Changelog

All notable changes to AEONIX are documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.1.0/),
and this project adheres to [Semantic Versioning](https://semver.org/).

## [0.2.0] - Unreleased

> [!IMPORTANT]
> This section is in preparation. The project version in the source tree is
> still **0.1.0**; `CMakeLists.txt`, `src/Main.cpp`, the README and the website
> are updated only when 0.2.0 is released.

### Added

- **Factory library v1**: 36 presets across seven families, five per family
  plus `Default`:
  - Pads: Aurora, Nocturne, Air, Drift, Frost.
  - Plucks: Glass, Wood, Bell, Muted, Wide.
  - Leads: Copper, Hollow, Velvet, Twin, Edge.
  - Bass: Sub Terra, Round, Acid Line, Growl, Pulse.
  - Brass: Soft, Bright, Big, Muted, Stab (synthetic brass).
  - Piano: Felt, Bell, Soft, Bright, Lo-fi (e-piano, not acoustic).
  - Strings: Warm, Wide, Soft, Bright, Cinematic.
- **Preset audition**: a persistent `Load on select` toggle in the preset
  manager. Selecting a preset loads it without closing the manager. The
  preference is stored in `config.json` (`presets.audition`) and starts off.
- **Folder shortcut in the sidebar**: the preset folder is now a 26×26 folder
  icon next to `+ New preset`, matching the main preset bar.
- **Focused keyboard access** for `+ New preset` and the folder icon.
- **Preset sorting**: a 120×22 sort button in the manager header opens a menu
  with `Name A–Z`, `Name Z–A`, `Category`, `Origin` and `Last used`. The choice
  is stored in `config.json` (`presets.sort`) and `Origin` is the default.
- **Pluck and Brass categories** (`pluck`, `metales`): the library now shows
  five plucks and five brass presets in their own rows.

### Changed

- The previous 14 prototype presets were replaced by the 36-preset factory
  library.
- The preset manager footer is now 32 px high, with a top rule, a 120 px
  `Esc to close` label and a 190×24 audition toggle aligned with the preset
  list.
- The end of the preset list fades over 12 px, with a 6 px cushion, so rows no
  longer collide with the footer. The fade does not cover the scrollbar.
- `Abrir carpeta` / `Open folder` was removed from the footer; the sidebar icon
  is the only entry point.
- Factory categories are derived from the family suffix
  (`Pad`, `Pluck`, `Lead`, `Bass`, `Brass`, `Piano`, `Strings`). The factory
  category is always taken from the built-in table, so older `library.json`
  files do not need to be migrated.
- Empty categories are no longer drawn: `FX` and `Other` only appear when a
  preset uses them. The sidebar shows eight rows with the current library.
- The list no longer has an `Origin` column. The name gained 36 px and only
  user presets carry a 6 px diamond at the end of the row; the accessible name
  of every row still reports factory or user.
- If the saved `preset.last` no longer exists, AEONIX falls back to `Default`
  instead of starting without a loaded preset.

### Fixed

- Cancelling the unsaved-changes guard from `Load` no longer closes the preset
  manager.
- Auditioning a preset no longer adds it to Recent or writes `preset.last` on
  every selection; the last auditioned preset is saved once when the manager
  closes.
- Opening `Save copy…` from the guard no longer resets the manager's search and
  filter.
- The preset manager's keyboard cycle now reaches `+ New preset` and the folder
  icon, with a visible focus ring.

### Verification

- `verify`: 84 tests.
- `presets`: Copper Lead and Warm Strings smoke cases.
- `ui`: 48/48 combinations, 64 main controls, 0 overlaps, 0 out-of-bounds,
  0 clipped labels.
- PRO Release build: `AEONIX.exe`.
- `docs-check`: 27 notes, 191 wiki links and 24 images.

### Notes

- No DSP, audio engine or main-surface control changes.
- AEONIX remains a standalone Windows application; VST3 is planned but not
  available.
- Release notes, ZIP names, tag and SHA-256 checksums are pending.

## [0.1.0] - 2026-11-11

First public release.

- Standalone subtractive synthesizer for Windows 10/11, 64-bit, built with
  C++20 and JUCE 8.
- One oscillator, resonant low-pass filter, amp and filter ADSR, LFO to cutoff,
  unison, stereo delay, reverb, pitch bend and mod wheel.
- 14 factory presets, user presets in JSON, 4 themes, Spanish and English, and
  three text sizes.
- Static runtime: no Visual C++ Redistributable required.
- AGPL-3.0.
- Unsigned executable with SHA-256 checksums and a SmartScreen warning.

[0.2.0]: https://github.com/amareca/aeonix-synth/compare/v0.1.0...HEAD
[0.1.0]: https://github.com/amareca/aeonix-synth/releases/tag/v0.1.0
