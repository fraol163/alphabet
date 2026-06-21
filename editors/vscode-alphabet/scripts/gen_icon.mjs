// Generate a high-quality 256x256 PNG icon for the Alphabet extension.
// Renders a stylized "A" inside a rounded purple gradient square with a
// multi-color stripe at the bottom representing multilingual keywords.

import * as fs from 'fs';
import * as zlib from 'zlib';
import * as path from 'path';

const W = 256;
const H = 256;

function rgba(r, g, b, a = 255) {
    return { r, g, b, a };
}

// Pixel buffer (RGBA, top-to-bottom in image order).
const pixels = new Array(W * H);

// Fill entire image with transparent.
for (let i = 0; i < pixels.length; i++) pixels[i] = rgba(0, 0, 0, 0);

function setPx(x, y, c) {
    if (x < 0 || x >= W || y < 0 || y >= H) return;
    pixels[y * W + x] = c;
}

function fillRect(x0, y0, x1, y1, c) {
    for (let y = y0; y < y1; y++)
        for (let x = x0; x < x1; x++)
            setPx(x, y, c);
}

// Linear gradient (vertical) from top to bottom.
function gradFillRect(x0, y0, x1, y1, cTop, cBot) {
    for (let y = y0; y < y1; y++) {
        const t = (y - y0) / Math.max(1, (y1 - y0 - 1));
        const c = {
            r: Math.round(cTop.r + (cBot.r - cTop.r) * t),
            g: Math.round(cTop.g + (cBot.g - cTop.g) * t),
            b: Math.round(cTop.b + (cBot.b - cTop.b) * t),
            a: Math.round(cTop.a + (cBot.a - cTop.a) * t),
        };
        for (let x = x0; x < x1; x++) setPx(x, y, c);
    }
}

// Rounded-rectangle fill (clips corners to radius).
function fillRoundRect(x0, y0, x1, y1, radius, c) {
    for (let y = y0; y < y1; y++) {
        for (let x = x0; x < x1; x++) {
            let dx = 0, dy = 0;
            if (x < x0 + radius) dx = x0 + radius - x;
            else if (x >= x1 - radius) dx = x - (x1 - radius - 1);
            if (y < y0 + radius) dy = y0 + radius - y;
            else if (y >= y1 - radius) dy = y - (y1 - radius - 1);
            if (dx === 0 || dy === 0) {
                setPx(x, y, c);
            } else if (dx * dx + dy * dy <= radius * radius) {
                setPx(x, y, c);
            }
        }
    }
}

// ---- Background ----------------------------------------------------------
// Rounded square with vertical purple gradient.
const PAD = 8;
const R = 32;
const top = rgba(124, 58, 237, 255);    // violet-500
const bot = rgba(76, 29, 149, 255);     // violet-800
gradFillRect(PAD, PAD, W - PAD, H - PAD, top, bot);
// Apply rounded corners by clearing alpha outside the rounded rect.
for (let y = 0; y < H; y++) {
    for (let x = 0; x < W; x++) {
        let dx = 0, dy = 0;
        const innerX0 = PAD, innerX1 = W - PAD, innerY0 = PAD, innerY1 = H - PAD;
        if (x < innerX0 + R) dx = innerX0 + R - x;
        else if (x >= innerX1 - R) dx = x - (innerX1 - R - 1);
        if (y < innerY0 + R) dy = innerY0 + R - y;
        else if (y >= innerY1 - R) dy = y - (innerY1 - R - 1);
        if (dx > 0 && dy > 0 && dx * dx + dy * dy > R * R) {
            setPx(x, y, rgba(0, 0, 0, 0));
        }
    }
}

// ---- "A" letterform ------------------------------------------------------
const letterTop = 50;
const letterBot = 220;
const letterLeft = 58;
const letterRight = W - 58;
const letterW = letterRight - letterLeft;
const letterH = letterBot - letterTop;
const strokeW = 22;

function fillA() {
    const cx = (letterLeft + letterRight) / 2;
    const halfBaseW = letterW / 2;
    for (let y = letterTop; y < letterBot; y++) {
        const t = (y - letterTop) / Math.max(1, letterH);
        const halfW = halfBaseW * t;
        const leftX = cx - halfW - strokeW / 2;
        const rightX = cx - halfW + strokeW / 2;
        fillRect(Math.round(leftX), y, Math.round(rightX), y + 1, rgba(255, 255, 255, 255));
        const leftX2 = cx + halfW - strokeW / 2;
        const rightX2 = cx + halfW + strokeW / 2;
        fillRect(Math.round(leftX2), y, Math.round(rightX2), y + 1, rgba(255, 255, 255, 255));
    }
    const crossY = letterTop + Math.round(letterH * 0.62);
    const crossHalfW = (halfBaseW * (crossY - letterTop) / Math.max(1, letterH)) + strokeW;
    fillRect(Math.round(cx - crossHalfW), crossY - 8, Math.round(cx + crossHalfW), crossY + 8,
             rgba(255, 255, 255, 255));
}
fillA();

// ---- Multilingual stripes at bottom --------------------------------------
const stripeY = H - 50;
const stripeH = 12;
const colors = [
    rgba(96, 165, 250, 255),   // blue (en)
    rgba(251, 146, 60, 255),   // orange (am)
    rgba(248, 113, 113, 255),  // red (es)
    rgba(74, 222, 128, 255),   // green (de)
    rgba(168, 85, 247, 255),   // purple (fr)
];
const stripeW = 26;
const stripeGap = 8;
const totalW = colors.length * stripeW + (colors.length - 1) * stripeGap;
const stripeStart = Math.round((W - totalW) / 2);
for (let i = 0; i < colors.length; i++) {
    const sx = stripeStart + i * (stripeW + stripeGap);
    fillRoundRect(sx, stripeY, sx + stripeW, stripeY + stripeH, 3, colors[i]);
}

// ---- Encode PNG ----------------------------------------------------------
function encodePNG(pixels, width, height) {
    const sig = Buffer.from([137, 80, 78, 71, 13, 10, 26, 10]);
    function chunk(type, data) {
        const len = Buffer.alloc(4);
        len.writeUInt32BE(data.length, 0);
        const t = Buffer.from(type, 'ascii');
        const crc = Buffer.alloc(4);
        const crcVal = crc32(Buffer.concat([t, data]));
        crc.writeUInt32BE(crcVal >>> 0, 0);
        return Buffer.concat([len, t, data, crc]);
    }
    const ihdr = Buffer.alloc(13);
    ihdr.writeUInt32BE(width, 0);
    ihdr.writeUInt32BE(height, 4);
    ihdr.writeUInt8(8, 8);     // bit depth
    ihdr.writeUInt8(6, 9);     // color type RGBA
    ihdr.writeUInt8(0, 10);    // compression
    ihdr.writeUInt8(0, 11);    // filter
    ihdr.writeUInt8(0, 12);    // interlace

    const rowBytes = width * 4;
    const raw = Buffer.alloc((rowBytes + 1) * height);
    for (let y = 0; y < height; y++) {
        raw[y * (rowBytes + 1)] = 0; // None filter
        for (let x = 0; x < width; x++) {
            const p = pixels[y * width + x];
            const o = y * (rowBytes + 1) + 1 + x * 4;
            raw[o] = p.r;
            raw[o + 1] = p.g;
            raw[o + 2] = p.b;
            raw[o + 3] = p.a;
        }
    }
    const idatData = zlib.deflateSync(raw);
    return Buffer.concat([sig, chunk('IHDR', ihdr), chunk('IDAT', idatData), chunk('IEND', Buffer.alloc(0))]);
}

const CRC_TABLE = (() => {
    const t = new Uint32Array(256);
    for (let n = 0; n < 256; n++) {
        let c = n;
        for (let k = 0; k < 8; k++) c = (c & 1) ? (0xEDB88320 ^ (c >>> 1)) : (c >>> 1);
        t[n] = c >>> 0;
    }
    return t;
})();
function crc32(buf) {
    let c = 0xFFFFFFFF;
    for (let i = 0; i < buf.length; i++) c = CRC_TABLE[(c ^ buf[i]) & 0xFF] ^ (c >>> 8);
    return (c ^ 0xFFFFFFFF) >>> 0;
}

const png = encodePNG(pixels, W, H);
const outPath = path.resolve(process.argv[2] || 'icon.png');
fs.writeFileSync(outPath, png);
console.log(`Wrote ${png.length} bytes -> ${outPath} (${W}x${H})`);
