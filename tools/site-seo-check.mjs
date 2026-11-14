// Validates the public site before GitHub Pages deploys it.
// Run with: node tools/site-seo-check.mjs
import fs from 'node:fs';
import path from 'node:path';
import { fileURLToPath } from 'node:url';

const root = path.resolve(path.dirname(fileURLToPath(import.meta.url)), '..');
const siteDir = path.join(root, 'site');
const BASE = 'https://amareca.github.io/aeonix-synth';

const errors = [];
const check = (condition, message) => {
  if (!condition) errors.push(message);
};

const read = (rel) => fs.readFileSync(path.join(siteDir, rel), 'utf8');

const pages = [
  { file: 'index.html', lang: 'en', url: BASE + '/', dir: siteDir },
  { file: 'es/index.html', lang: 'es', url: BASE + '/es/', dir: path.join(siteDir, 'es') }
];

for (const page of pages) {
  const html = read(page.file);
  const label = page.file;

  const title = (html.match(/<title>([\s\S]*?)<\/title>/) || [])[1] || '';
  const description = (html.match(/<meta name="description" content="([^"]*)">/) || [])[1] || '';
  const canonical = (html.match(/<link rel="canonical" href="([^"]*)">/) || [])[1] || '';
  const langAttr = (html.match(/<html lang="([^"]*)"/) || [])[1] || '';

  check(langAttr === page.lang, `${label}: html lang is "${langAttr}", expected "${page.lang}"`);
  check(title.length >= 20 && title.length <= 70, `${label}: title length ${title.length} (20-70)`);
  check(description.length >= 70 && description.length <= 165, `${label}: description length ${description.length} (70-165)`);
  check(canonical === page.url, `${label}: canonical is "${canonical}", expected "${page.url}"`);
  check(html.includes('hreflang="en"'), `${label}: missing hreflang en`);
  check(html.includes('hreflang="es"'), `${label}: missing hreflang es`);
  check(html.includes('hreflang="x-default"'), `${label}: missing hreflang x-default`);
  check((html.match(/<h1\b/g) || []).length === 1, `${label}: expected exactly one h1`);

  const blocks = [...html.matchAll(/<script type="application\/ld\+json">([\s\S]*?)<\/script>/g)].map((m) => m[1]);
  check(blocks.length >= 2, `${label}: expected SoftwareApplication and FAQPage JSON-LD`);

  const types = [];
  for (const raw of blocks) {
    try {
      types.push(JSON.parse(raw)['@type']);
    } catch (error) {
      errors.push(`${label}: invalid JSON-LD (${error.message})`);
    }
  }
  check(types.includes('SoftwareApplication'), `${label}: missing SoftwareApplication JSON-LD`);
  check(types.includes('FAQPage'), `${label}: missing FAQPage JSON-LD`);

  const faqItems = (html.match(/class="faq__item"/g) || []).length;
  check(faqItems >= 6, `${label}: found ${faqItems} FAQ items, expected 6 or more`);

  const refs = new Set();
  for (const match of html.matchAll(/(?:href|src)="([^"]+)"/g)) {
    const ref = match[1];
    if (
      !ref ||
      ref.startsWith('#') ||
      ref.startsWith('http') ||
      ref.startsWith('mailto:') ||
      ref.startsWith('data:') ||
      ref.startsWith('//')
    ) {
      continue;
    }
    refs.add(ref);
  }

  for (const ref of refs) {
    const target = path.resolve(page.dir, ref);
    const candidate = ref.endsWith('/') ? path.join(target, 'index.html') : target;
    check(fs.existsSync(candidate), `${label}: broken local reference "${ref}"`);
  }
}

const sitemap = read('sitemap.xml');
for (const loc of [BASE + '/', BASE + '/es/']) {
  check(sitemap.includes(`<loc>${loc}</loc>`), `sitemap.xml: missing ${loc}`);
}
check(sitemap.includes('hreflang="x-default"'), 'sitemap.xml: missing hreflang alternates');

const robots = read('robots.txt');
check(robots.includes(`${BASE}/sitemap.xml`), 'robots.txt: missing sitemap reference');
check(fs.existsSync(path.join(siteDir, 'llms.txt')), 'llms.txt: missing');

if (errors.length) {
  console.error('SEO check failed:');
  for (const error of errors) console.error('  - ' + error);
  process.exit(1);
}

console.log('SEO check OK: 2 language pages, JSON-LD, hreflang, sitemap and llms.txt.');
