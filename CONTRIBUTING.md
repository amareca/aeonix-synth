# Contribuir a AEONIX

Guía rápida para trabajar en el proyecto.

## Requisitos

- Windows 10/11.
- Visual Studio 2022 Build Tools con la carga de trabajo **C++**.
- CMake 3.24+ (el que trae Visual Studio sirve).
- Git y el submódulo de JUCE inicializado:

```powershell
git submodule update --init --recursive
```

## Primer arranque

```powershell
.\dev.ps1 doctor     # comprueba el entorno
.\dev.ps1 build      # compila Release
.\dev.ps1 test       # ejecuta los tests
```

Si PowerShell bloquea la ejecución de scripts:

```powershell
powershell -ExecutionPolicy Bypass -File .\dev.ps1 build
```

El ejecutable de desarrollo queda en `build\Aeonix_artefacts\Release\AEONIX-dev.exe`
(PRO: `build-pro\Aeonix_artefacts\Release\AEONIX.exe`).

## Comandos

| Comando | Para qué |
|---|---|
| `.\dev.ps1 doctor` | Diagnóstico del entorno |
| `.\dev.ps1 build` | Compilar Release (`-Configuration Debug` para Debug) |
| `.\dev.ps1 test` | Tests con CTest (`-Tag engine` para una etiqueta) |
| `.\dev.ps1 verify` | Configurar + compilar + tests |
| `.\dev.ps1 ui` | Matriz visual F11/F12 (48 combinaciones; `-Quick` para iterar) |
| `.\dev.ps1 presets` | Smoke del gestor de presets |
| `.\dev.ps1 captures` | Resumen de capturas; `-Clean`, `-Legacy` o `-Open` |

## Flujo de trabajo

1. Parte de `develop` y crea una rama para el cambio (`feature/*`, `fix/*`).
2. Haz cambios pequeños y verificables.
3. Ejecuta `.\dev.ps1 verify`.
4. Si tocas la interfaz, usa `.\dev.ps1 ui -Quick` mientras iteras y la matriz
   completa `.\dev.ps1 ui` antes de dar el cambio por terminado; revisa las
   capturas.
5. Abre un pull request hacia `develop`. Los mensajes de commit van en inglés.

`main` es la rama de producción: solo recibe merges por PR `develop -> main`
cuando la versión está lista. El sitio de GitHub Pages se publica únicamente
desde `main`, así que los cambios de `site/` no se ven hasta ese merge.

## Estilo de código

- C++20, 4 espacios, llaves en línea nueva (estilo JUCE).
- El dominio (`src/domain`) no depende de JUCE.
- Nada de asignaciones, locks o I/O dentro de `render()`.
- Comentarios y documentación en español; identificadores en inglés.
- Tipografía: usa `ui::font()` y `ui::px()`; no escribas tamaños fijos.
- i18n: las cadenas de interfaz van en `StringCatalog.inc`.
- Cambios grandes de layout: mockup y revisión visual antes de implementar.

## Antes de enviar

```powershell
.\dev.ps1 verify
```

Y si hay UI o presets:

```powershell
.\dev.ps1 ui
.\dev.ps1 presets
```

## Licencia de las contribuciones

Al enviar un pull request aceptas que tu contribución se distribuya bajo la
GNU Affero General Public License v3.0 (AGPL-3.0), igual que el resto del
proyecto. No se aceptan contribuciones con licencias incompatibles.
