/* ==========================================================================
   AEONIX — landing publica
   Cambio ES/EN, visor de capturas, selector de temas y menú móvil.
   Sin dependencias, sin red, sin analítica.
   ========================================================================== */

(() => {
  'use strict';

  const I18N = {
    es: {
      'meta.title': 'AEONIX — Sintetizador sustractivo para Windows',
      'meta.description': 'Sintetizador sustractivo standalone para Windows. 64 controles, cuatro temas, tres tamaños de texto y gestor de presets. Gratis y open source (AGPL-3.0).',
      'skip': 'Saltar al contenido',
      'brand.label': 'AEONIX, inicio',

      'nav.label': 'Principal',
      'nav.mobile': 'Móvil',
      'nav.product': 'Producto',
      'nav.gallery': 'Capturas',
      'nav.themes': 'Temas',
      'nav.access': 'Accesibilidad',
      'nav.docs': 'Proyecto',
      'nav.github': 'GitHub',
      'nav.github.label': 'GitHub de AEONIX',
      'nav.download': 'Descargar',
      'nav.menu.open': 'Abrir menú',
      'nav.menu.close': 'Cerrar menú',
      'lang.status': 'Idioma: español',

      'hero.eyebrow': 'Versión 0.1.0 · Windows x64',
      'hero.tagline': 'Sintetizador sustractivo standalone para Windows',
      'hero.lead': 'Una sola pantalla para esculpir el sonido: 64 controles, presets de fábrica y de usuario, y un gestor que encuentra lo que buscas. Sin pestañas, sin menús escondidos.',
      'hero.cta.download': 'Descargar',
      'hero.cta.gallery': 'Ver capturas',
      'hero.note': 'Gratis y open source · Windows 10 y 11 · x64 · 3,5 MB',
      'hero.window': 'Vista principal',
      'hero.caption': 'Superficie principal con texto Muy grande (1316 × 786). Captura de AEONIX 0.1.0, build PRO.',
      'hero.alt': 'Interfaz de AEONIX: barra de preset, secciones de síntesis, teclado virtual y controles de efectos.',

      'stats.label': 'Datos del producto',
      'stats.controls': 'controles en una pantalla',
      'stats.themes': 'temas Holo y Prototipo',
      'stats.sizes': 'tamaños de texto',
      'stats.langs': 'interfaz completa',
      'stats.presets.value': 'Fábrica + usuario',
      'stats.presets': 'presets',

      'features.eyebrow': 'Producto',
      'features.title': 'Todo el flujo de señal, en una sola vista',
      'features.lead': 'Del oscilador al master sin cambiar de contexto. Cada control existe porque hace falta.',

      'feat.engine.title': 'Motor sustractivo',
      'feat.engine.body': 'Oscilador, filtro y envolventes forman una cadena clara. Ajusta, escucha y compara sin abrir cuadros de diálogo.',
      'feat.osc.title': 'Oscilador, filtro, envolventes y LFO',
      'feat.osc.body': 'Los controles de cada etapa viven juntos y en orden de señal.',
      'feat.fx.title': 'Delay y reverb',
      'feat.fx.body': 'Efectos integrados para dar espacio al sonido sin salir de la superficie principal.',
      'feat.unison.title': 'Unison',
      'feat.unison.body': 'Voces desafinadas para dar cuerpo y anchura al oscilador.',
      'feat.keys.title': 'Teclado virtual y MIDI',
      'feat.keys.body': 'Toca con el ratón o conecta un controlador: el teclado sigue lo que suena.',
      'feat.presets.title': 'Gestor de presets',
      'feat.presets.body': 'Búsqueda, filtros, favoritos y recientes. Guarda, copia, renombra y borra con confirmación, sin perder el hilo.',
      'feat.presets.search': 'Buscar preset',
      'feat.presets.fav': 'Favoritos',
      'feat.presets.recent': 'Recientes',
      'feat.presets.user': 'Usuario',
      'feat.a11y.title': 'Accesibilidad de serie',
      'feat.a11y.body': 'Tres tamaños de texto, foco visible, español e inglés, y alturas mínimas de ventana documentadas.',
      'feat.a11y.focus': 'Foco visible',

      'gallery.eyebrow': 'Capturas',
      'gallery.title': 'La interfaz, de cerca',
      'gallery.lead': 'Capturas reales de AEONIX 0.1.0 en Windows. Pulsa una imagen para ampliarla.',
      'gallery.note': 'Todas las capturas son de la build PRO, sin herramientas de desarrollo.',
      'shot.1.title': 'Superficie principal',
      'shot.1.meta': 'Texto Normal · 1176 × 748',
      'shot.1.alt': 'Superficie principal de AEONIX con texto Normal.',
      'shot.1.open': 'Ampliar captura: superficie principal',
      'shot.2.title': 'Texto Grande',
      'shot.2.meta': '1216 × 766',
      'shot.2.alt': 'Superficie de AEONIX con texto Grande.',
      'shot.2.open': 'Ampliar captura: texto Grande',
      'shot.3.title': 'Gestor de presets',
      'shot.3.meta': 'Búsqueda, filtros y favoritos',
      'shot.3.alt': 'Gestor de presets de AEONIX con búsqueda, filtros y favoritos.',
      'shot.3.open': 'Ampliar captura: gestor de presets',
      'shot.4.title': 'Búsqueda en la biblioteca',
      'shot.4.meta': 'Filtro «lead» · 1176 × 748',
      'shot.4.alt': 'Gestor de presets de AEONIX filtrado por la búsqueda lead.',
      'shot.4.open': 'Ampliar captura: búsqueda en la biblioteca',
      'shot.5.title': 'Ajustes de apariencia',
      'shot.5.meta': 'Tema, idioma y tamaño de texto',
      'shot.5.alt': 'Ajustes de AEONIX en la pestaña Apariencia, con tema, idioma y tamaño de texto.',
      'shot.5.open': 'Ampliar captura: ajustes de apariencia',
      'gallery.look.title': 'Qué mirar',
      'gallery.look.1': 'La barra de preset marca cuándo hay cambios sin guardar.',
      'gallery.look.2': 'El gestor filtra por nombre, categoría, favoritos y recientes.',
      'gallery.look.3': 'El texto Muy grande no recorta etiquetas ni valores.',
      'gallery.look.cta': 'Ampliar el gestor',

      'themes.eyebrow': 'Temas',
      'themes.title': 'Cuatro acabados, la misma lógica',
      'themes.lead': 'El tema no cambia el layout: cambia la luz. Elige uno para ver la captura.',
      'themes.choose': 'Elige un tema',
      'themes.previewMeta': '1176 × 748 · build PRO',
      'theme.accents.desc': 'Títulos y líneas con degradado holográfico. El acabado de referencia.',
      'theme.glass.desc': 'Paneles translúcidos, campo de estrellas y brillo violeta en el teclado.',
      'theme.minimal.desc': 'Sin degradados ni translucidez: contraste sobrio y bordes discretos.',
      'theme.proto.name': 'Prototipo',
      'theme.proto.desc': 'Gris neutro, pensado para leer el layout sin distracciones.',
      'theme.alt': 'Captura de AEONIX con el tema {name}.',
      'theme.open': 'Ampliar captura del tema',

      'a11y.eyebrow': 'Accesibilidad',
      'a11y.title': 'Legible a la primera, también con teclado',
      'a11y.lead': 'La app escala el texto y el layout por separado para que nada se recorte al crecer.',
      'a11y.1.title': 'Tres tamaños de texto',
      'a11y.1.body': '12, 15 y 18 px sobre Segoe UI. El layout acompaña al texto con factores 1,00, 1,07 y 1,15.',
      'a11y.2.title': 'Ventanas mínimas',
      'a11y.2.body': 'Cada tamaño declara su altura mínima para que ningún control quede fuera de la ventana.',
      'a11y.3.title': 'Foco y contraste',
      'a11y.3.body': 'Anillo de foco visible, texto principal por encima de 15:1 y atenuado por encima de 8:1 sobre el fondo.',
      'a11y.4.title': 'Dos idiomas',
      'a11y.4.body': 'Español e inglés en toda la interfaz, también en diálogos y en el gestor de presets.',
      'a11y.specimen.title': 'Escala tipográfica',
      'a11y.specimen.sample': 'Sintetizador sustractivo',
      'a11y.specimen.xl': 'Muy grande · 18 px',
      'a11y.specimen.lg': 'Grande · 15 px',
      'a11y.specimen.normal': 'Normal · 12 px',
      'a11y.specimen.note': 'Tamaños reales de la app. La web usa una escala de lectura más cómoda con la misma familia.',
      'a11y.contrast.title': 'Contraste sobre fondo oscuro',
      'a11y.contrast.text': 'Texto',
      'a11y.contrast.muted': 'Atenuado',
      'a11y.contrast.accent': 'Acento',
      'a11y.contrast.button': 'Botón',
      'a11y.win.title': 'Altura mínima por tamaño',
      'a11y.win.col': 'Texto',
      'a11y.win.min': 'Ventana mínima',
      'a11y.win.normal': 'Normal',
      'a11y.win.large': 'Grande',
      'a11y.win.xl': 'Muy grande',
      'a11y.win.caption': 'La ventana crece con el texto; el mínimo cambia con cada nivel.',

      'download.eyebrow': 'Descarga',
      'download.title': 'AEONIX 0.1.0, para Windows',
      'download.lead': 'Descarga el ZIP, descomprímelo y ejecuta AEONIX.exe. Sin instalador y sin dependencias: no hace falta Visual C++ Redistributable.',
      'download.cta': 'Descargar AEONIX 0.1.0',
      'download.size': '3,5 MB',
      'download.github': 'Ver la release en GitHub',
      'download.note': 'El ejecutable no está firmado: SmartScreen puede avisar la primera vez. Comprueba el SHA256 antes de ejecutarlo.',
      'download.release.status': 'Disponible',
      'download.release.platform': 'Plataforma',
      'download.release.platformVal': 'Windows 10/11 · x64',
      'download.release.audio': 'Audio',
      'download.release.audioVal': 'Salida del sistema',
      'download.release.midi': 'MIDI',
      'download.release.midiVal': 'Opcional',
      'download.release.size': 'Tamaño',
      'download.release.sizeVal': 'ZIP 3,5 MB · EXE 8,1 MB',
      'download.release.license': 'Licencia',
      'download.release.licenseVal': 'AGPL-3.0',
      'download.checksum.zip': 'SHA256 del ZIP',
      'download.checksum.exe': 'SHA256 de AEONIX.exe',
      'download.checksum.copy': 'Copiar',
      'download.checksum.copied': 'Copiado',
      'download.checksum.copyAria': 'Copiar SHA256 del ZIP',
      'download.checksum.copyExeAria': 'Copiar SHA256 de AEONIX.exe',
      'download.checksum.hint': 'Compruébalo en PowerShell: Get-FileHash AEONIX-0.1.0-win64.zip -Algorithm SHA256',

      'docs.eyebrow': 'Proyecto',
      'docs.title': 'Código abierto, sin letra pequeña',
      'docs.lead': 'El repositorio incluye el código, las instrucciones de compilación y la licencia completa.',
      'docs.1.title': 'Guía rápida',
      'docs.1.body': 'Qué es AEONIX, requisitos y primeros pasos con presets.',
      'docs.1.link': 'Leer el README',
      'docs.2.title': 'Compilar',
      'docs.2.body': 'C++20, CMake y JUCE 8. Comandos para las builds DEV y PRO.',
      'docs.2.link': 'Ver instrucciones',
      'docs.3.title': 'Licencia AGPL-3.0',
      'docs.3.body': 'Código abierto. Si distribuyes una versión modificada, comparte también el código.',
      'docs.3.link': 'Leer la licencia',

      'footer.nav': 'Pie de página',
      'footer.tagline': 'Sintetizador sustractivo standalone para Windows.',
      'footer.built': 'Hecho con C++20 + JUCE 8',
      'footer.github': 'GitHub',
      'footer.docs': 'Proyecto',
      'footer.license': 'Licencia',
      'footer.issues': 'Issues',
      'footer.version': 'Versión 0.1.0',
      'footer.legal': 'AGPL-3.0 · Copyright (C) 2026 AEONIX contributors · Sin cookies ni analítica.',

      'lightbox.label': 'Visor de capturas',
      'lightbox.close': 'Cerrar visor',
      'lightbox.prev': 'Captura anterior',
      'lightbox.next': 'Captura siguiente',
      'lightbox.hint': 'Usa ← → para navegar y Esc para cerrar.'
    },

    en: {
      'meta.title': 'AEONIX — Subtractive synthesizer for Windows',
      'meta.description': 'Standalone subtractive synthesizer for Windows. 64 controls, four themes, three text sizes and a preset manager. Free and open source (AGPL-3.0).',
      'skip': 'Skip to content',
      'brand.label': 'AEONIX, home',

      'nav.label': 'Main',
      'nav.mobile': 'Mobile',
      'nav.product': 'Product',
      'nav.gallery': 'Screenshots',
      'nav.themes': 'Themes',
      'nav.access': 'Accessibility',
      'nav.docs': 'Project',
      'nav.github': 'GitHub',
      'nav.github.label': 'AEONIX on GitHub',
      'nav.download': 'Download',
      'nav.menu.open': 'Open menu',
      'nav.menu.close': 'Close menu',
      'lang.status': 'Language: English',

      'hero.eyebrow': 'Version 0.1.0 · Windows x64',
      'hero.tagline': 'Standalone subtractive synthesizer for Windows',
      'hero.lead': 'One screen to shape your sound: 64 controls, factory and user presets, and a manager that finds what you need. No tabs, no hidden menus.',
      'hero.cta.download': 'Download',
      'hero.cta.gallery': 'View screenshots',
      'hero.note': 'Free and open source · Windows 10 and 11 · x64 · 3.5 MB',
      'hero.window': 'Main view',
      'hero.caption': 'Main surface with extra-large text (1316 × 786). Screenshot of AEONIX 0.1.0, PRO build.',
      'hero.alt': 'AEONIX interface: preset bar, synthesis sections, virtual keyboard and effects controls.',

      'stats.label': 'Product facts',
      'stats.controls': 'controls on one screen',
      'stats.themes': 'Holo and Prototype themes',
      'stats.sizes': 'text sizes',
      'stats.langs': 'full interface',
      'stats.presets.value': 'Factory + user',
      'stats.presets': 'presets',

      'features.eyebrow': 'Product',
      'features.title': 'The whole signal flow, on one screen',
      'features.lead': 'From oscillator to master without changing context. Every control earns its place.',

      'feat.engine.title': 'Subtractive engine',
      'feat.engine.body': 'Oscillator, filter and envelopes form a clear chain. Tweak, listen and compare without opening dialogs.',
      'feat.osc.title': 'Oscillator, filter, envelopes and LFO',
      'feat.osc.body': "Each stage's controls live together, in signal order.",
      'feat.fx.title': 'Delay and reverb',
      'feat.fx.body': 'Built-in effects to add space without leaving the main surface.',
      'feat.unison.title': 'Unison',
      'feat.unison.body': 'Detuned voices to add body and width to the oscillator.',
      'feat.keys.title': 'Virtual keyboard and MIDI',
      'feat.keys.body': 'Play with the mouse or plug in a controller: the keyboard follows what you hear.',
      'feat.presets.title': 'Preset manager',
      'feat.presets.body': 'Search, filters, favorites and recent. Save, copy, rename and delete with confirmation, without losing your place.',
      'feat.presets.search': 'Search preset',
      'feat.presets.fav': 'Favorites',
      'feat.presets.recent': 'Recent',
      'feat.presets.user': 'User',
      'feat.a11y.title': 'Accessibility built in',
      'feat.a11y.body': 'Three text sizes, visible focus, Spanish and English, and documented minimum window sizes.',
      'feat.a11y.focus': 'Visible focus',

      'gallery.eyebrow': 'Screenshots',
      'gallery.title': 'The interface, up close',
      'gallery.lead': 'Real screenshots of AEONIX 0.1.0 on Windows. Click an image to enlarge it.',
      'gallery.note': 'All screenshots come from the PRO build, without developer tools.',
      'shot.1.title': 'Main surface',
      'shot.1.meta': 'Normal text · 1176 × 748',
      'shot.1.alt': 'AEONIX main surface with normal text.',
      'shot.1.open': 'Enlarge screenshot: main surface',
      'shot.2.title': 'Large text',
      'shot.2.meta': '1216 × 766',
      'shot.2.alt': 'AEONIX surface with large text.',
      'shot.2.open': 'Enlarge screenshot: large text',
      'shot.3.title': 'Preset manager',
      'shot.3.meta': 'Search, filters and favorites',
      'shot.3.alt': 'AEONIX preset manager with search, filters and favorites.',
      'shot.3.open': 'Enlarge screenshot: preset manager',
      'shot.4.title': 'Library search',
      'shot.4.meta': '“lead” filter · 1176 × 748',
      'shot.4.alt': 'AEONIX preset manager filtered by the search term lead.',
      'shot.4.open': 'Enlarge screenshot: library search',
      'shot.5.title': 'Appearance settings',
      'shot.5.meta': 'Theme, language and text size',
      'shot.5.alt': 'AEONIX settings on the Appearance tab, with theme, language and text size.',
      'shot.5.open': 'Enlarge screenshot: appearance settings',
      'gallery.look.title': 'What to look for',
      'gallery.look.1': 'The preset bar shows when there are unsaved changes.',
      'gallery.look.2': 'The manager filters by name, category, favourites and recent.',
      'gallery.look.3': "Extra-large text doesn't clip labels or values.",
      'gallery.look.cta': 'Enlarge the manager',

      'themes.eyebrow': 'Themes',
      'themes.title': 'Four finishes, same logic',
      'themes.lead': "The theme doesn't change the layout, only the light. Pick one to preview.",
      'themes.choose': 'Choose a theme',
      'themes.previewMeta': '1176 × 748 · PRO build',
      'theme.accents.desc': 'Holographic gradient titles and lines. The reference finish.',
      'theme.glass.desc': 'Translucent panels, a starfield and a violet glow on the keyboard.',
      'theme.minimal.desc': 'No gradients or translucency: sober contrast and discreet borders.',
      'theme.proto.name': 'Prototype',
      'theme.proto.desc': 'Neutral grey, made for reading the layout without distractions.',
      'theme.alt': 'AEONIX screenshot with the {name} theme.',
      'theme.open': 'Enlarge theme screenshot',

      'a11y.eyebrow': 'Accessibility',
      'a11y.title': 'Readable at a glance, keyboard included',
      'a11y.lead': 'The app scales text and layout separately so nothing gets clipped as it grows.',
      'a11y.1.title': 'Three text sizes',
      'a11y.1.body': '12, 15 and 18 px in Segoe UI. The layout follows the text with 1.00, 1.07 and 1.15 factors.',
      'a11y.2.title': 'Minimum windows',
      'a11y.2.body': 'Each size declares its minimum height so no control falls outside the window.',
      'a11y.3.title': 'Focus and contrast',
      'a11y.3.body': 'Visible focus ring, main text above 15:1 and muted text above 8:1 on the background.',
      'a11y.4.title': 'Two languages',
      'a11y.4.body': 'Spanish and English across the interface, including dialogs and the preset manager.',
      'a11y.specimen.title': 'Type scale',
      'a11y.specimen.sample': 'Subtractive synthesizer',
      'a11y.specimen.xl': 'Extra large · 18 px',
      'a11y.specimen.lg': 'Large · 15 px',
      'a11y.specimen.normal': 'Normal · 12 px',
      'a11y.specimen.note': 'Actual app sizes. The site uses a more comfortable reading scale with the same family.',
      'a11y.contrast.title': 'Contrast on dark background',
      'a11y.contrast.text': 'Text',
      'a11y.contrast.muted': 'Muted',
      'a11y.contrast.accent': 'Accent',
      'a11y.contrast.button': 'Button',
      'a11y.win.title': 'Minimum height per size',
      'a11y.win.col': 'Text',
      'a11y.win.min': 'Minimum window',
      'a11y.win.normal': 'Normal',
      'a11y.win.large': 'Large',
      'a11y.win.xl': 'Extra large',
      'a11y.win.caption': 'The window grows with the text; the minimum changes per level.',

      'download.eyebrow': 'Download',
      'download.title': 'AEONIX 0.1.0, for Windows',
      'download.lead': 'Download the ZIP, extract it and run AEONIX.exe. No installer, no dependencies: the Visual C++ Redistributable is not required.',
      'download.cta': 'Download AEONIX 0.1.0',
      'download.size': '3.5 MB',
      'download.github': 'View release on GitHub',
      'download.note': 'The executable is not signed: SmartScreen may warn on first run. Check the SHA256 before running it.',
      'download.release.status': 'Available',
      'download.release.platform': 'Platform',
      'download.release.platformVal': 'Windows 10/11 · x64',
      'download.release.audio': 'Audio',
      'download.release.audioVal': 'System output',
      'download.release.midi': 'MIDI',
      'download.release.midiVal': 'Optional',
      'download.release.size': 'Size',
      'download.release.sizeVal': 'ZIP 3.5 MB · EXE 8.1 MB',
      'download.release.license': 'License',
      'download.release.licenseVal': 'AGPL-3.0',
      'download.checksum.zip': 'ZIP SHA256',
      'download.checksum.exe': 'AEONIX.exe SHA256',
      'download.checksum.copy': 'Copy',
      'download.checksum.copied': 'Copied',
      'download.checksum.copyAria': 'Copy ZIP SHA256',
      'download.checksum.copyExeAria': 'Copy AEONIX.exe SHA256',
      'download.checksum.hint': 'Check it in PowerShell: Get-FileHash AEONIX-0.1.0-win64.zip -Algorithm SHA256',

      'docs.eyebrow': 'Project',
      'docs.title': 'Open source, no fine print',
      'docs.lead': 'The repository includes the source, build instructions and the full license.',
      'docs.1.title': 'Quick start',
      'docs.1.body': 'What AEONIX is, requirements and first steps with presets.',
      'docs.1.link': 'Read the README',
      'docs.2.title': 'Build from source',
      'docs.2.body': 'C++20, CMake and JUCE 8. Commands for the DEV and PRO builds.',
      'docs.2.link': 'See instructions',
      'docs.3.title': 'AGPL-3.0 license',
      'docs.3.body': 'Open source. If you distribute a modified version, share the source too.',
      'docs.3.link': 'Read the license',

      'footer.nav': 'Footer',
      'footer.tagline': 'Standalone subtractive synthesizer for Windows.',
      'footer.built': 'Built with C++20 + JUCE 8',
      'footer.github': 'GitHub',
      'footer.docs': 'Project',
      'footer.license': 'License',
      'footer.issues': 'Issues',
      'footer.version': 'Version 0.1.0',
      'footer.legal': 'AGPL-3.0 · Copyright (C) 2026 AEONIX contributors · No cookies, no analytics.',

      'lightbox.label': 'Screenshot viewer',
      'lightbox.close': 'Close viewer',
      'lightbox.prev': 'Previous screenshot',
      'lightbox.next': 'Next screenshot',
      'lightbox.hint': 'Use ← → to navigate and Esc to close.'
    }
  };

  const THEMES = {
    accents: {
      name: 'Holo Accents',
      src: 'assets/captures/theme-accents.png',
      width: 1176,
      height: 748
    },
    glass: {
      name: 'Holo Glass',
      src: 'assets/captures/theme-glass.png',
      width: 1176,
      height: 748
    },
    minimal: {
      name: 'Holo Minimal',
      src: 'assets/captures/theme-minimal.png',
      width: 1176,
      height: 748
    },
    prototype: {
      name: 'Prototipo',
      nameEn: 'Prototype',
      src: 'assets/captures/theme-prototype.png',
      width: 1176,
      height: 748
    }
  };

  function themeLabel(theme) {
    return state.lang === 'en' && theme.nameEn ? theme.nameEn : theme.name;
  }

  const doc = document;
  const $ = (selector, root = doc) => root.querySelector(selector);
  const $$ = (selector, root = doc) => Array.from(root.querySelectorAll(selector));

  const state = {
    lang: 'es',
    theme: 'accents',
    menuOpen: false,
    lastFocus: null,
    lightboxItems: [],
    lightboxIndex: 0
  };

  const storage = {
    get(key) {
      try {
        return window.localStorage.getItem(key);
      } catch (error) {
        return null;
      }
    },
    set(key, value) {
      try {
        window.localStorage.setItem(key, value);
      } catch (error) {
        /* file:// o modo privado: el cambio sigue funcionando en memoria */
      }
    }
  };

  const t = (key) => {
    const table = I18N[state.lang] || I18N.es;
    return table[key] !== undefined ? table[key] : (I18N.es[key] !== undefined ? I18N.es[key] : key);
  };

  /* ------------------------------------------------------------------ i18n */

  function updateMenuLabel() {
    const button = $('#menuBtn');
    if (!button) return;
    const label = t(state.menuOpen ? 'nav.menu.close' : 'nav.menu.open');
    button.setAttribute('aria-label', label);
  }

  function applyLang(lang, persist = true) {
    state.lang = lang === 'en' ? 'en' : 'es';
    doc.documentElement.lang = state.lang;

    doc.title = t('meta.title');
    const description = $('meta[name="description"]');
    if (description) description.setAttribute('content', t('meta.description'));

    $$('[data-i18n]').forEach((element) => {
      const value = t(element.dataset.i18n);
      if (typeof value === 'string') element.textContent = value;
    });

    $$('[data-i18n-alt]').forEach((element) => {
      element.setAttribute('alt', t(element.dataset.i18nAlt));
    });

    $$('[data-i18n-aria-label]').forEach((element) => {
      element.setAttribute('aria-label', t(element.dataset.i18nAriaLabel));
    });

    $$('[data-i18n-title]').forEach((element) => {
      element.setAttribute('title', t(element.dataset.i18nTitle));
    });

    $$('.lang__btn').forEach((button) => {
      button.setAttribute('aria-pressed', String(button.dataset.lang === state.lang));
    });

    updateMenuLabel();
    applyTheme(state.theme, false);

    if (persist) storage.set('aeonix.landing.lang', state.lang);
  }

  $$('.lang__btn').forEach((button) => {
    button.addEventListener('click', () => {
      if (button.dataset.lang !== state.lang) {
        applyLang(button.dataset.lang);
        const status = $('#langStatus');
        if (status) status.textContent = t('lang.status');
      }
    });
  });

  /* ----------------------------------------------------------------- themes */

  function applyTheme(id, persist = true) {
    const theme = THEMES[id] || THEMES.accents;
    const label = themeLabel(theme);
    state.theme = id in THEMES ? id : 'accents';

    $$('.theme-opt').forEach((button) => {
      button.setAttribute('aria-pressed', String(button.dataset.theme === state.theme));
    });

    ['#themeCaption', '#themeCaptionFoot'].forEach((selector) => {
      const node = $(selector);
      if (node) node.textContent = label;
    });

    const image = $('#themeImg');
    if (image) {
      image.alt = t('theme.alt').replace('{name}', label);
      if (image.getAttribute('src') !== theme.src) {
        image.classList.add('is-swapping');
        image.addEventListener('load', () => image.classList.remove('is-swapping'), { once: true });
        image.addEventListener('error', () => image.classList.remove('is-swapping'), { once: true });
        image.setAttribute('src', theme.src);
        image.setAttribute('width', String(theme.width));
        image.setAttribute('height', String(theme.height));
      }
    }

    if (persist) storage.set('aeonix.landing.theme', state.theme);
  }

  $$('.theme-opt').forEach((button) => {
    button.addEventListener('click', () => applyTheme(button.dataset.theme));
  });

  /* --------------------------------------------------------------- lightbox */

  const lightbox = $('#lightbox');
  const lightboxImage = $('#lightboxImg');
  const lightboxCaption = $('#lightboxCaption');
  const lightboxCount = $('#lightboxCount');
  const prevButton = $('[data-prev-shot]');
  const nextButton = $('[data-next-shot]');
  const closeButton = $('[data-close-lightbox]');

  const galleryShots = $$('.shot__btn').map((button) => {
    const image = $('img', button);
    return {
      src: image.getAttribute('src'),
      altKey: button.dataset.altKey,
      titleKey: button.dataset.titleKey,
      metaKey: button.dataset.metaKey
    };
  });

  function renderLightbox() {
    const item = state.lightboxItems[state.lightboxIndex];
    if (!item) return;

    let alt = t(item.altKey);
    if (item.themeName) alt = alt.replace('{name}', item.themeName);

    lightboxImage.setAttribute('src', item.src);
    lightboxImage.setAttribute('alt', alt);

    if (item.titleKey && item.metaKey) {
      lightboxCaption.textContent = `${t(item.titleKey)} · ${t(item.metaKey)}`;
    } else if (item.themeName) {
      lightboxCaption.textContent = item.themeName;
    }

    lightboxCount.textContent = `${state.lightboxIndex + 1} / ${state.lightboxItems.length}`;

    const multiple = state.lightboxItems.length > 1;
    if (prevButton) prevButton.hidden = !multiple;
    if (nextButton) nextButton.hidden = !multiple;
    lightboxCount.hidden = !multiple;
  }

  function openLightbox(items, index) {
    if (!items.length) return;

    state.lastFocus = doc.activeElement;
    state.lightboxItems = items;
    state.lightboxIndex = Math.max(0, Math.min(index, items.length - 1));

    if (!lightbox || typeof lightbox.showModal !== 'function') {
      window.open(items[state.lightboxIndex].src, '_blank', 'noopener');
      return;
    }

    renderLightbox();
    lightbox.showModal();
    doc.documentElement.classList.add('no-scroll');

    if (closeButton) closeButton.focus();
  }

  function stepLightbox(delta) {
    const total = state.lightboxItems.length;
    if (total < 2) return;
    state.lightboxIndex = (state.lightboxIndex + delta + total) % total;
    renderLightbox();
  }

  $$('.shot__btn').forEach((button, index) => {
    button.addEventListener('click', () => openLightbox(galleryShots, index));
  });

  $$('[data-open-shot]').forEach((button) => {
    button.addEventListener('click', () => {
      openLightbox(galleryShots, Number(button.dataset.openShot) || 0);
    });
  });

  /* --------------------------------------------------------------- checksum */

  $$('[data-copy-hash]').forEach((button) => {
    const label = button.querySelector('span');
    let timer = null;

    button.addEventListener('click', async () => {
      const value = button.getAttribute('data-copy-hash') || '';

      try {
        await navigator.clipboard.writeText(value);
      } catch (error) {
        const area = doc.createElement('textarea');
        area.value = value;
        area.setAttribute('readonly', '');
        area.style.position = 'fixed';
        area.style.opacity = '0';
        doc.body.appendChild(area);
        area.select();
        doc.execCommand('copy');
        doc.body.removeChild(area);
      }

      if (label) {
        label.textContent = t('download.checksum.copied');
        window.clearTimeout(timer);
        timer = window.setTimeout(() => {
          label.textContent = t('download.checksum.copy');
        }, 1800);
      }
    });
  });

  const themeOpen = $('#themeOpen');
  if (themeOpen) {
    themeOpen.addEventListener('click', () => {
      const theme = THEMES[state.theme] || THEMES.accents;
      const label = themeLabel(theme);
      openLightbox(
        [{
          src: theme.src,
          altKey: 'theme.alt',
          titleKey: null,
          metaKey: null,
          themeName: label
        }],
        0
      );
      lightboxCaption.textContent = label;
    });
  }

  if (prevButton) prevButton.addEventListener('click', () => stepLightbox(-1));
  if (nextButton) nextButton.addEventListener('click', () => stepLightbox(1));

  if (closeButton) {
    closeButton.addEventListener('click', () => lightbox.close());
  }

  if (lightbox) {
    lightbox.addEventListener('click', (event) => {
      if (event.target === lightbox) lightbox.close();
    });

    lightbox.addEventListener('keydown', (event) => {
      if (event.key === 'ArrowLeft') {
        event.preventDefault();
        stepLightbox(-1);
      } else if (event.key === 'ArrowRight') {
        event.preventDefault();
        stepLightbox(1);
      }
    });

    lightbox.addEventListener('close', () => {
      doc.documentElement.classList.remove('no-scroll');
      if (state.lastFocus && typeof state.lastFocus.focus === 'function') {
        state.lastFocus.focus();
      }
    });
  }

  /* ------------------------------------------------------------------- menu */

  const mobileMenu = $('#mobileMenu');
  const menuButton = $('#menuBtn');
  const header = $('.site-header');

  function setMenu(open) {
    state.menuOpen = Boolean(open);
    if (menuButton) menuButton.setAttribute('aria-expanded', String(state.menuOpen));
    if (mobileMenu) mobileMenu.hidden = !state.menuOpen;
    updateMenuLabel();
  }

  if (menuButton) {
    menuButton.addEventListener('click', () => setMenu(!state.menuOpen));
  }

  $$('#mobileMenu a').forEach((link) => {
    link.addEventListener('click', () => setMenu(false));
  });

  doc.addEventListener('keydown', (event) => {
    if (event.key === 'Escape' && state.menuOpen) {
      setMenu(false);
      if (menuButton) menuButton.focus();
    }
  });

  doc.addEventListener('click', (event) => {
    if (!state.menuOpen || !header) return;
    if (!header.contains(event.target)) setMenu(false);
  });

  window.addEventListener('resize', () => {
    if (window.innerWidth > 980 && state.menuOpen) setMenu(false);
  });

  /* ----------------------------------------------------------------- header */

  function syncHeader() {
    if (!header) return;
    header.classList.toggle('is-scrolled', window.scrollY > 8);
  }

  syncHeader();
  window.addEventListener('scroll', syncHeader, { passive: true });

  /* ----------------------------------------------------------------- reveal */

  const reveals = $$('.reveal');
  const reduceMotion = window.matchMedia('(prefers-reduced-motion: reduce)').matches;

  if (!('IntersectionObserver' in window) || reduceMotion) {
    reveals.forEach((element) => element.classList.add('in'));
  } else {
    const observer = new IntersectionObserver(
      (entries) => {
        entries.forEach((entry) => {
          if (entry.isIntersecting) {
            entry.target.classList.add('in');
            observer.unobserve(entry.target);
          }
        });
      },
      { rootMargin: '0px 0px -8% 0px', threshold: 0.1 }
    );

    reveals.forEach((element) => observer.observe(element));
  }

  /* ------------------------------------------------------------------- init */

  const storedLang = storage.get('aeonix.landing.lang');
  const storedTheme = storage.get('aeonix.landing.theme');

  applyLang(storedLang === 'en' ? 'en' : 'es', false);
  if (storedTheme && storedTheme in THEMES) applyTheme(storedTheme, false);
})();
