# AEONIX

Sintetizador sustractivo **standalone** (estilo Sylenth1), orientado a plucks y
pads cinematográficos/espaciales. Windows primero, preparado para Linux/macOS y
futuro plugin VST3.

- **Stack**: C++20, CMake, JUCE (audio + MIDI + GUI).
- **Arquitectura**: clean architecture + MVVM (núcleo DSP en C++ puro, sin JUCE).
- **Accesibilidad**: tamaño de texto Normal / Grande / Muy grande, detección
  automática por pantalla y contraste mejorado.

## Características

- Superficie única con **64 controles**: oscilador, filtro, envolventes, LFO,
  unison, delay, reverb y master.
- Teclado virtual, entrada MIDI y ruedas de pitch/modulación.
- Presets de fábrica y de usuario, con buscador, filtros, favoritos y recientes.
- Cuatro temas visuales: Holo Accents, Holo Glass, Holo Minimal y Prototipo.
- Interfaz en español e inglés, con tres tamaños de texto.

## Descargar

La versión para Windows se publica en
[GitHub Releases](https://github.com/amareca/aeonix-synth/releases).

Web del proyecto: <https://amareca.github.io/aeonix-synth/>

- Requisitos: **Windows 10/11 de 64 bits**. El ejecutable enlaza el runtime de
  C++ de forma estática, así que no hace falta instalar Visual C++.
- Descarga `AEONIX-0.1.0-win64.zip`, descomprímelo y ejecuta `AEONIX.exe`.
- El ejecutable **no está firmado**: SmartScreen puede avisar la primera vez.
  Comprueba el SHA256 publicado en la release y usa
  "Más información → Ejecutar de todas formas".
- Los datos de usuario viven en `%APPDATA%\Aeonix` y
  `Documentos\Aeonix\Presets`.

## Compilar (Windows)

Requisitos: Visual Studio 2022 Build Tools (workload C++), CMake 3.24+.

```powershell
git submodule update --init --recursive   # descarga JUCE
.\dev.ps1 build
```

También puedes usar CMake directamente:

```powershell
cmake --preset dev
cmake --build --preset release
```

El ejecutable de desarrollo se genera en
`build/Aeonix_artefacts/Release/AEONIX-dev.exe`; PRO va a
`build-pro/Aeonix_artefacts/Release/AEONIX.exe`.

## Desarrollo

`dev.ps1` es el punto de entrada único:

| Comando | Para qué |
|---|---|
| `.\dev.ps1 doctor` | Diagnóstico del entorno |
| `.\dev.ps1 build` | Compilar Release (`-Configuration Debug` para Debug) |
| `.\dev.ps1 test` | Tests con CTest (`-Tag engine`, `-Changed`, `-Failed`, `-Build`) |
| `.\dev.ps1 verify` | Configurar + compilar + tests |
| `.\dev.ps1 ui` | Matriz F11/F12 de temas, tamaños e idiomas (`-Quick` para iterar) |
| `.\dev.ps1 presets` | Smoke del gestor de presets |
| `.\dev.ps1 captures` | Resumen de capturas; `-Clean`, `-Legacy` o `-Open` |

## Verificar

```powershell
.\dev.ps1 verify
```

Si la política de ejecución de PowerShell lo bloquea:

```powershell
powershell -ExecutionPolicy Bypass -File .\dev.ps1 verify
```

Configura CMake, compila Release y ejecuta todos los tests con `ctest`. Si algo
falla, devuelve un código de error. `verify.ps1` se mantiene como envoltorio de
compatibilidad.

## Contribuir

Consulta [`CONTRIBUTING.md`](CONTRIBUTING.md) para requisitos, comandos y flujo
de trabajo.

## Licencia

AEONIX se distribuye bajo la **GNU Affero General Public License v3.0**
(AGPL-3.0). El texto completo está en [`LICENSE`](LICENSE).

Copyright (C) 2026 AEONIX contributors.

JUCE 8.0.15 se usa bajo AGPLv3 y Catch2 bajo BSL-1.0; consulta
[`THIRD_PARTY_NOTICES.md`](THIRD_PARTY_NOTICES.md) para los avisos de terceros.
Las versiones binarias deben ofrecer el código fuente correspondiente.
