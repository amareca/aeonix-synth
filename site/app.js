/* ==========================================================================
   AEONIX — landing publica
   Visor de capturas, selector de temas y menú móvil. El idioma lo fija la
   página estática; no hay cambio ES/EN en tiempo de ejecución.
   Sin dependencias, sin red, sin analítica.
   ========================================================================== */

(() => {
  'use strict';

  const I18N = {
    es: {
      "theme.alt": "Captura de AEONIX con el tema {name}.",
      "nav.menu.open": "Abrir menú",
      "nav.menu.close": "Cerrar menú",
      "download.checksum.copy": "Copiar",
      "download.checksum.copied": "Copiado",
      "shot.1.title": "Superficie principal",
      "shot.1.meta": "Texto Normal · 1176 × 748",
      "shot.1.alt": "Superficie principal de AEONIX con texto Normal.",
      "shot.2.title": "Texto Grande",
      "shot.2.meta": "1216 × 766",
      "shot.2.alt": "Superficie de AEONIX con texto Grande.",
      "shot.3.title": "Gestor de presets",
      "shot.3.meta": "Búsqueda, filtros y favoritos",
      "shot.3.alt": "Gestor de presets de AEONIX con búsqueda, filtros y favoritos.",
      "shot.4.title": "Búsqueda en la biblioteca",
      "shot.4.meta": "Filtro «lead» · 1176 × 748",
      "shot.4.alt": "Gestor de presets de AEONIX filtrado por la búsqueda lead.",
      "shot.5.title": "Ajustes de apariencia",
      "shot.5.meta": "Tema, idioma y tamaño de texto",
      "shot.5.alt": "Ajustes de AEONIX en la pestaña Apariencia, con tema, idioma y tamaño de texto."
    },
    en: {
      "theme.alt": "AEONIX screenshot with the {name} theme.",
      "nav.menu.open": "Open menu",
      "nav.menu.close": "Close menu",
      "download.checksum.copy": "Copy",
      "download.checksum.copied": "Copied",
      "shot.1.title": "Main surface",
      "shot.1.meta": "Normal text · 1176 × 748",
      "shot.1.alt": "AEONIX main surface with normal text.",
      "shot.2.title": "Large text",
      "shot.2.meta": "1216 × 766",
      "shot.2.alt": "AEONIX surface with large text.",
      "shot.3.title": "Preset manager",
      "shot.3.meta": "Search, filters and favorites",
      "shot.3.alt": "AEONIX preset manager with search, filters and favorites.",
      "shot.4.title": "Library search",
      "shot.4.meta": "“lead” filter · 1176 × 748",
      "shot.4.alt": "AEONIX preset manager filtered by the search term lead.",
      "shot.5.title": "Appearance settings",
      "shot.5.meta": "Theme, language and text size",
      "shot.5.alt": "AEONIX settings on the Appearance tab, with theme, language and text size."
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
  const rootPrefix = doc.documentElement.dataset.root || '';
  const $ = (selector, root = doc) => root.querySelector(selector);
  const $$ = (selector, root = doc) => Array.from(root.querySelectorAll(selector));

  const state = {
    lang: doc.documentElement.lang === 'en' ? 'en' : 'es',
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
        image.setAttribute('src', rootPrefix + theme.src);
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
      width: image.getAttribute('width'),
      height: image.getAttribute('height'),
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
    if (item.width && item.height) {
      lightboxImage.setAttribute('width', item.width);
      lightboxImage.setAttribute('height', item.height);
    }
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
          src: rootPrefix + theme.src,
          width: theme.width,
          height: theme.height,
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

  const storedTheme = storage.get('aeonix.landing.theme');

  if (storedTheme && storedTheme in THEMES) applyTheme(storedTheme, false);
})();
