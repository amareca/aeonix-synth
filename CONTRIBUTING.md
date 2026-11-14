# Contributing to AEONIX

English → [Español](#español)

A quick guide to working on the project.

## Requirements

- Windows 10/11.
- Visual Studio 2022 Build Tools with the **C++** workload.
- CMake 3.24+ (the one bundled with Visual Studio is fine).
- Git, with the JUCE submodule initialised:

```powershell
git submodule update --init --recursive
```

## First run

```powershell
.\dev.ps1 doctor     # checks the environment
.\dev.ps1 build      # builds Release
.\dev.ps1 test       # runs the tests
```

If PowerShell blocks script execution:

```powershell
powershell -ExecutionPolicy Bypass -File .\dev.ps1 build
```

The development executable lands in `build\Aeonix_artefacts\Release\AEONIX-dev.exe`
(PRO: `build-pro\Aeonix_artefacts\Release\AEONIX.exe`).

## Commands

| Command | What it does |
|---|---|
| `.\dev.ps1 doctor` | Environment diagnostics |
| `.\dev.ps1 build` | Build Release (`-Configuration Debug` for Debug) |
| `.\dev.ps1 test` | Tests through CTest (`-Tag engine` for a single tag) |
| `.\dev.ps1 verify` | Configure + build + tests |
| `.\dev.ps1 ui` | Visual F11/F12 matrix (48 combinations; `-Quick` while iterating) |
| `.\dev.ps1 presets` | Preset manager smoke test |
| `.\dev.ps1 captures` | Capture summary; `-Clean`, `-Legacy` or `-Open` |

## Workflow

1. Start from `develop` and create a branch for your change (`feature/*`, `fix/*`).
2. Make small, verifiable changes.
3. Run `.\dev.ps1 verify`.
4. If you touch the interface, use `.\dev.ps1 ui -Quick` while iterating and the
   full matrix `.\dev.ps1 ui` before calling it done; review the captures.
5. Open a pull request against `develop`. Commit messages are written in English.

`main` is the production branch: it only takes merges through a `develop -> main`
pull request once the version is ready. The GitHub Pages site is published from
`main` only, so `site/` changes stay invisible until that merge.

## Code style

- C++20, 4 spaces, braces on their own line (JUCE style).
- The domain layer (`src/domain`) does not depend on JUCE.
- No allocations, locks or I/O inside `render()`.
- **Comments, identifiers and dev tool output are written in English.** Spanish
  is reserved for the internal `docs/` tree and for the user-facing strings in
  `StringCatalog.inc`.
- Typography: use `ui::font()` and `ui::px()`; never hardcode sizes.
- i18n: interface strings live in `StringCatalog.inc`, English first, Spanish
  second.
- Large layout changes: mockup and visual review before implementing.

## Before submitting

```powershell
.\dev.ps1 verify
```

And if the change touches the UI or the presets:

```powershell
.\dev.ps1 ui
.\dev.ps1 presets
```

## Contribution licence

By submitting a pull request you agree that your contribution is distributed
under the GNU Affero General Public License v3.0 (AGPL-3.0), like the rest of the
project. Contributions under incompatible licences are not accepted.

---

## Español

Guía rápida para trabajar en el proyecto.

Los documentos públicos van en inglés primero y después en español; este archivo
sigue ese orden. La documentación interna (`docs/`) se mantiene en español.

```powershell
git submodule update --init --recursive
.\dev.ps1 doctor     # comprueba el entorno
.\dev.ps1 build      # compila Release
.\dev.ps1 test       # ejecuta los tests
.\dev.ps1 verify     # configura + compila + tests
.\dev.ps1 ui         # matriz visual F11/F12 (48 combinaciones)
.\dev.ps1 presets    # smoke del gestor de presets
```

Si PowerShell bloquea la ejecución de scripts:
`powershell -ExecutionPolicy Bypass -File .\dev.ps1 build`.

El ejecutable de desarrollo queda en
`build\Aeonix_artefacts\Release\AEONIX-dev.exe` (PRO:
`build-pro\Aeonix_artefacts\Release\AEONIX.exe`).

**Estilo:** C++20, 4 espacios, llaves en línea nueva; el dominio no depende de
JUCE; nada de asignaciones, locks o I/O dentro de `render()`. Comentarios,
identificadores y salida de herramientas en inglés; cadenas de interfaz en
`StringCatalog.inc`, inglés primero y español después. Tipografía con
`ui::font()` y `ui::px()`. Cambios grandes de layout: mockup y revisión visual
antes de implementar.

**Flujo:** parte de `develop`, haz cambios pequeños y verificables, ejecuta
`.\dev.ps1 verify` (y `ui`/`presets` si tocas interfaz o presets) y abre un PR
hacia `develop`. `main` solo recibe merges por PR `develop -> main` cuando la
versión está lista; el sitio de GitHub Pages se publica únicamente desde `main`.

**Licencia:** al enviar un pull request aceptas que tu contribución se
distribuya bajo la GNU Affero General Public License v3.0 (AGPL-3.0). No se
aceptan contribuciones con licencias incompatibles.
