#!/usr/bin/env node
/**
 * Orca Slicer post-processing script: gives MKS TFT (Marlin mks_ui) G-code previews.
 *
 * The MKS firmware wants, at the very top of the file (first 256 bytes):
 *   - ";simage:" : 100x100 RGB565 preview shown in the file list
 *   - ";gimage:" : 200x200 RGB565 preview shown before printing
 * written as raw hex, one image row per line (8-char prefix + hex + LF).
 * Orca only writes PNG (or ColPic) thumbnails further down, so this converts the
 * PNG thumbnail and inserts the MKS blocks at the top of the file.
 *
 * Usage (Orca passes the G-code path as the last argument):
 *   node orca_mks_thumbnail.js file.gcode
 *
 * Orca settings: Printer > General > G-code thumbnails = e.g. "300x300/PNG"
 */
'use strict';
const fs = require('fs');
const zlib = require('zlib');

const SMALL = 100, BIG = 200;
const PREVIEW_SIZE = 202720; // PREVIEW_SIZE in mks_ui/draw_ui.h

function fail(msg) { console.error('orca_mks_thumbnail: ' + msg); process.exit(1); }

// ---- PNG decoding (8-bit, non-interlaced) ------------------------------------------
function decodePng(buf) {
  if (buf.length < 8 || buf.readUInt32BE(0) !== 0x89504e47) throw new Error('not a PNG');
  let pos = 8, w = 0, h = 0, depth = 0, ctype = 0, interlace = 0, plte = null, trns = null;
  const idat = [];
  while (pos + 8 <= buf.length) {
    const len = buf.readUInt32BE(pos), type = buf.toString('latin1', pos + 4, pos + 8);
    const data = buf.subarray(pos + 8, pos + 8 + len);
    pos += 12 + len;
    if (type === 'IHDR') {
      w = data.readUInt32BE(0); h = data.readUInt32BE(4);
      depth = data[8]; ctype = data[9]; interlace = data[12];
    }
    else if (type === 'PLTE') plte = data;
    else if (type === 'tRNS') trns = data;
    else if (type === 'IDAT') idat.push(data);
    else if (type === 'IEND') break;
  }
  if (depth !== 8 || interlace !== 0) throw new Error('unsupported PNG (need 8-bit, non-interlaced)');
  const bpp = { 0: 1, 2: 3, 3: 1, 4: 2, 6: 4 }[ctype];
  if (!bpp) throw new Error('unsupported PNG color type ' + ctype);

  const raw = zlib.inflateSync(Buffer.concat(idat));
  const stride = w * bpp, px = Buffer.alloc(stride * h);
  for (let y = 0; y < h; y++) {
    const ft = raw[y * (stride + 1)], src = y * (stride + 1) + 1, dst = y * stride;
    for (let i = 0; i < stride; i++) {
      const a = i >= bpp ? px[dst + i - bpp] : 0;
      const b = y > 0 ? px[dst - stride + i] : 0;
      const c = (y > 0 && i >= bpp) ? px[dst - stride + i - bpp] : 0;
      let v = raw[src + i];
      if (ft === 1) v += a;
      else if (ft === 2) v += b;
      else if (ft === 3) v += (a + b) >> 1;
      else if (ft === 4) {
        const p = a + b - c, pa = Math.abs(p - a), pb = Math.abs(p - b), pc = Math.abs(p - c);
        v += (pa <= pb && pa <= pc) ? a : (pb <= pc ? b : c);
      }
      px[dst + i] = v & 255;
    }
  }

  // Convert to RGBA
  const rgba = Buffer.alloc(w * h * 4);
  for (let i = 0, n = w * h; i < n; i++) {
    let r, g, b, a = 255;
    if (ctype === 6) { r = px[i * 4]; g = px[i * 4 + 1]; b = px[i * 4 + 2]; a = px[i * 4 + 3]; }
    else if (ctype === 2) { r = px[i * 3]; g = px[i * 3 + 1]; b = px[i * 3 + 2]; }
    else if (ctype === 0) { r = g = b = px[i]; }
    else if (ctype === 4) { r = g = b = px[i * 2]; a = px[i * 2 + 1]; }
    else { const k = px[i]; r = plte[k * 3]; g = plte[k * 3 + 1]; b = plte[k * 3 + 2]; if (trns && k < trns.length) a = trns[k]; }
    rgba[i * 4] = r; rgba[i * 4 + 1] = g; rgba[i * 4 + 2] = b; rgba[i * 4 + 3] = a;
  }
  return { w, h, rgba };
}

// ---- Resize to a square of `size`, keep aspect ratio, composite over black ----------
// Returns an array of RGB565 values (transparent -> 0x0000, which the firmware turns into the UI background).
function toRgb565(img, size) {
  const scale = Math.min(size / img.w, size / img.h);
  const dw = Math.max(1, Math.round(img.w * scale)), dh = Math.max(1, Math.round(img.h * scale));
  const ox = (size - dw) >> 1, oy = (size - dh) >> 1;
  const out = new Uint16Array(size * size);
  for (let y = 0; y < dh; y++) {
    const y0 = Math.floor(y * img.h / dh), y1 = Math.max(y0 + 1, Math.floor((y + 1) * img.h / dh));
    for (let x = 0; x < dw; x++) {
      const x0 = Math.floor(x * img.w / dw), x1 = Math.max(x0 + 1, Math.floor((x + 1) * img.w / dw));
      let r = 0, g = 0, b = 0, a = 0, n = 0;
      for (let sy = y0; sy < y1; sy++) for (let sx = x0; sx < x1; sx++) {
        const i = (sy * img.w + sx) * 4, al = img.rgba[i + 3];
        r += img.rgba[i] * al; g += img.rgba[i + 1] * al; b += img.rgba[i + 2] * al; a += al; n++;
      }
      if (a < 128 * n) continue; // mostly transparent: leave 0 (background)
      r = r / a | 0; g = g / a | 0; b = b / a | 0;
      let v = ((r >> 3) << 11) | ((g >> 2) << 5) | (b >> 3);
      if (v === 0) v = 1; // 0x0000 means "background" for the firmware
      out[(y + oy) * size + x + ox] = v;
    }
  }
  return out;
}

// The firmware reads the hex as bytes and casts them to uint16 (little endian): low byte first.
function rowHex(px, size, y) {
  let s = '';
  for (let x = 0; x < size; x++) {
    const v = px[y * size + x];
    s += ((v & 255) | 0x100).toString(16).slice(1) + ((v >> 8) | 0x100).toString(16).slice(1);
  }
  return s.toUpperCase();
}

// ---- Find the PNG thumbnails written by Orca ---------------------------------------
function findThumbnails(text) {
  const list = [];
  const re = /^;\s*thumbnail begin (\d+)x(\d+) \d+\r?\n((?:;[^\r\n]*\r?\n)*?);\s*thumbnail end/gm;
  let m;
  while ((m = re.exec(text))) {
    const b64 = m[3].replace(/^;\s*/gm, '').replace(/\s+/g, '');
    list.push({ w: +m[1], h: +m[2], b64 });
  }
  return list;
}

// ---- Main ----------------------------------------------------------------------------
const file = process.argv[process.argv.length - 1];
if (process.argv.length < 3 || !fs.existsSync(file)) fail('usage: node orca_mks_thumbnail.js file.gcode');

const data = fs.readFileSync(file);
if (data.toString('latin1', 0, 8) === ';simage:') process.exit(0); // already done

const thumbs = findThumbnails(data.toString('latin1'));
if (!thumbs.length) fail('no PNG thumbnail found. In Orca: Printer settings > General > G-code thumbnails, e.g. 300x300/PNG');
thumbs.sort((a, b) => b.w * b.h - a.w * a.h);

let img;
try { img = decodePng(Buffer.from(thumbs[0].b64, 'base64')); }
catch (e) { fail('cannot read the PNG thumbnail: ' + e.message); }

const small = toRgb565(img, SMALL), big = toRgb565(img, BIG);
const lines = [];
for (let y = 0; y < SMALL; y++) lines.push(';simage:' + rowHex(small, SMALL, y) + '\n');
lines.push('\r;;gimage:' + rowHex(big, BIG, 0) + '\n');
for (let y = 1; y < BIG; y++) lines.push(';gimage:' + rowHex(big, BIG, y) + '\n');
let block = lines.join('');
// Pad with a comment so the block is exactly PREVIEW_SIZE (the firmware skips it when computing progress)
const pad = PREVIEW_SIZE - block.length;
if (pad < 2) fail('internal error: preview block too large');
block += ';' + '-'.repeat(pad - 2) + '\n';

const tmp = file + '.mks.tmp';
fs.writeFileSync(tmp, Buffer.concat([Buffer.from(block, 'latin1'), data]));
fs.renameSync(tmp, file);
console.log('orca_mks_thumbnail: preview added (' + thumbs[0].w + 'x' + thumbs[0].h + ' -> 100x100 + 200x200)');
