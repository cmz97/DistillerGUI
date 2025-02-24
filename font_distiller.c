/*******************************************************************************
 * Size: 18 px
 * Bpp: 1
 * Opts: --bpp 1 --size 18 --no-compress --font m6x11plus.ttf --range 13,32-95,97-126,161-172,174,176-177,182,186-187,191-207,209-239,241-263,272-277,280-281,286-287,298-301,304-305,321-324,332-335,338-339,346-347,350-351,362-365,376-380,467-468,956 --format lvgl -o font_distiller.c
 ******************************************************************************/

#ifdef LV_LVGL_H_INCLUDE_SIMPLE
#include "lvgl.h"
#else
#include "lvgl/lvgl.h"
#endif

#ifndef FONT_DISTILLER
#define FONT_DISTILLER 1
#endif

#if FONT_DISTILLER

/*-----------------
 *    BITMAPS
 *----------------*/

/*Store the image of the glyphs*/
static LV_ATTRIBUTE_LARGE_CONST const uint8_t glyph_bitmap[] = {
    /* U+000D "\r" */
    0x0,

    /* U+0020 " " */
    0x0,

    /* U+0021 "!" */
    0xff, 0xff, 0x3c,

    /* U+0022 "\"" */
    0xde, 0xd2,

    /* U+0023 "#" */
    0x6c, 0xd9, 0xb7, 0xff, 0xed, 0xbf, 0xff, 0x6c,
    0xd9, 0xb0,

    /* U+0024 "$" */
    0x30, 0xc7, 0xff, 0xc3, 0xe7, 0xc3, 0xff, 0xe3,
    0xc,

    /* U+0025 "%" */
    0xc3, 0x31, 0xce, 0x73, 0x8c, 0xc3,

    /* U+0026 "&" */
    0x3c, 0x3f, 0x19, 0x8c, 0xc7, 0xe7, 0xef, 0x7f,
    0x9e, 0xce, 0x7f, 0xde, 0xe0,

    /* U+0027 "'" */
    0xf4,

    /* U+0028 "(" */
    0x7f, 0xcc, 0xcc, 0xcc, 0xcc, 0xcf, 0x70,

    /* U+0029 ")" */
    0xef, 0x33, 0x33, 0x33, 0x33, 0x3f, 0xe0,

    /* U+002A "*" */
    0xdf, 0xdd, 0xfd, 0x80,

    /* U+002B "+" */
    0x30, 0xcf, 0xff, 0x30, 0xc0,

    /* U+002C "," */
    0xf4,

    /* U+002D "-" */
    0xff, 0xf0,

    /* U+002E "." */
    0xf0,

    /* U+002F "/" */
    0xc, 0x31, 0xc6, 0x38, 0xc7, 0x18, 0xe3, 0xc,
    0x0,

    /* U+0030 "0" */
    0x7b, 0xfc, 0xf3, 0xcf, 0x3c, 0xf3, 0xcf, 0xf7,
    0x80,

    /* U+0031 "1" */
    0x33, 0xcf, 0xc, 0x30, 0xc3, 0xc, 0x33, 0xff,
    0xc0,

    /* U+0032 "2" */
    0x7b, 0xfc, 0xc3, 0xc, 0x73, 0x9c, 0xe3, 0xff,
    0xc0,

    /* U+0033 "3" */
    0x7b, 0xfc, 0xc3, 0x38, 0xf0, 0xc3, 0xcf, 0xf7,
    0x80,

    /* U+0034 "4" */
    0xcf, 0x3c, 0xf3, 0xcf, 0xf7, 0xc3, 0xc, 0x30,
    0xc0,

    /* U+0035 "5" */
    0xff, 0xfc, 0x30, 0xfb, 0xf0, 0xc3, 0xf, 0xff,
    0x80,

    /* U+0036 "6" */
    0x7b, 0xfc, 0xf0, 0xfb, 0xfc, 0xf3, 0xcf, 0xf7,
    0x80,

    /* U+0037 "7" */
    0xff, 0xf0, 0xc3, 0x1c, 0x63, 0x8c, 0x30, 0xc3,
    0x0,

    /* U+0038 "8" */
    0x7b, 0xfc, 0xf3, 0x7b, 0xfc, 0xf3, 0xcf, 0xf7,
    0x80,

    /* U+0039 "9" */
    0x7b, 0xfc, 0xf3, 0xcf, 0xf7, 0xc3, 0xcf, 0xf7,
    0x80,

    /* U+003A ":" */
    0xf0, 0xf,

    /* U+003B ";" */
    0xf0, 0xf, 0x40,

    /* U+003C "<" */
    0x19, 0xdd, 0xce, 0x38, 0xe3,

    /* U+003D "=" */
    0xff, 0xf0, 0x0, 0xff, 0xf0,

    /* U+003E ">" */
    0xc7, 0x1c, 0x73, 0xbb, 0x98,

    /* U+003F "?" */
    0x7b, 0xfc, 0xc3, 0x1c, 0xe3, 0xc, 0x0, 0xc3,
    0x0,

    /* U+0040 "@" */
    0x3e, 0x3f, 0xb8, 0xf9, 0xfd, 0xfe, 0xdf, 0x7f,
    0x9e, 0xe0, 0x3f, 0xcf, 0xe0,

    /* U+0041 "A" */
    0x7b, 0xfc, 0xf3, 0xcf, 0xff, 0xf3, 0xcf, 0x3c,
    0xc0,

    /* U+0042 "B" */
    0xfb, 0xfc, 0xf3, 0xff, 0xec, 0xf3, 0xcf, 0xff,
    0x80,

    /* U+0043 "C" */
    0x7f, 0xfc, 0x30, 0xc3, 0xc, 0x30, 0xc3, 0xf7,
    0xc0,

    /* U+0044 "D" */
    0xfb, 0xfc, 0xf3, 0xcf, 0x3c, 0xf3, 0xcf, 0xff,
    0x80,

    /* U+0045 "E" */
    0x7f, 0xfc, 0x30, 0xfb, 0xec, 0x30, 0xc3, 0xf7,
    0xc0,

    /* U+0046 "F" */
    0x7f, 0xfc, 0x30, 0xfb, 0xec, 0x30, 0xc3, 0xc,
    0x0,

    /* U+0047 "G" */
    0x7f, 0xfc, 0x30, 0xdf, 0x7c, 0xf3, 0xcf, 0xf7,
    0x80,

    /* U+0048 "H" */
    0xcf, 0x3c, 0xf3, 0xff, 0xfc, 0xf3, 0xcf, 0x3c,
    0xc0,

    /* U+0049 "I" */
    0xff, 0xf3, 0xc, 0x30, 0xc3, 0xc, 0x33, 0xff,
    0xc0,

    /* U+004A "J" */
    0xff, 0xf0, 0xc3, 0xc, 0x30, 0xc3, 0xcf, 0xf7,
    0x80,

    /* U+004B "K" */
    0xcf, 0x3c, 0xf7, 0xfb, 0xcf, 0xb7, 0xcf, 0x3c,
    0xc0,

    /* U+004C "L" */
    0xc3, 0xc, 0x30, 0xc3, 0xc, 0x30, 0xc3, 0xf7,
    0xc0,

    /* U+004D "M" */
    0xfe, 0xff, 0xdb, 0xdb, 0xdb, 0xdb, 0xdb, 0xdb,
    0xdb, 0xc3, 0xc3,

    /* U+004E "N" */
    0xfb, 0xfc, 0xf3, 0xcf, 0x3c, 0xf3, 0xcf, 0x3c,
    0xc0,

    /* U+004F "O" */
    0x7b, 0xfc, 0xf3, 0xcf, 0x3c, 0xf3, 0xcf, 0xf7,
    0x80,

    /* U+0050 "P" */
    0xfb, 0xfc, 0xf3, 0xcf, 0xff, 0xb0, 0xc3, 0xc,
    0x0,

    /* U+0051 "Q" */
    0x79, 0xfb, 0x36, 0x6c, 0xd9, 0xb3, 0x66, 0xdd,
    0xfd, 0xd8,

    /* U+0052 "R" */
    0xfb, 0xfc, 0xf3, 0xff, 0xed, 0xf3, 0xcf, 0x3c,
    0xc0,

    /* U+0053 "S" */
    0x7f, 0xfc, 0x30, 0xf9, 0xf0, 0xc3, 0xf, 0xff,
    0x80,

    /* U+0054 "T" */
    0xff, 0xf3, 0xc, 0x30, 0xc3, 0xc, 0x30, 0xc3,
    0x0,

    /* U+0055 "U" */
    0xcf, 0x3c, 0xf3, 0xcf, 0x3c, 0xf3, 0xcf, 0xf7,
    0x80,

    /* U+0056 "V" */
    0xcf, 0x3c, 0xf3, 0xcf, 0x3c, 0xf3, 0xfd, 0xe3,
    0x0,

    /* U+0057 "W" */
    0xc3, 0xc3, 0xc3, 0xc3, 0xdb, 0xdb, 0xdb, 0xdb,
    0xdb, 0xff, 0x7e,

    /* U+0058 "X" */
    0xc7, 0x8f, 0x1f, 0x77, 0xc7, 0x1f, 0x77, 0xc7,
    0x8f, 0x18,

    /* U+0059 "Y" */
    0xcf, 0x3c, 0xf3, 0xfd, 0xe3, 0xc, 0x30, 0xc3,
    0x0,

    /* U+005A "Z" */
    0xff, 0xf1, 0xc6, 0x38, 0xc7, 0x18, 0xe3, 0xff,
    0xc0,

    /* U+005B "[" */
    0xff, 0xcc, 0xcc, 0xcc, 0xcc, 0xcf, 0xf0,

    /* U+005C "\\" */
    0xc3, 0xe, 0x18, 0x70, 0xc3, 0x86, 0x1c, 0x30,
    0xc0,

    /* U+005D "]" */
    0xff, 0x33, 0x33, 0x33, 0x33, 0x3f, 0xf0,

    /* U+005E "^" */
    0x31, 0xef, 0xf3,

    /* U+005F "_" */
    0xff, 0xf0,

    /* U+0061 "a" */
    0xfb, 0xf0, 0xdf, 0xff, 0x3f, 0xdf,

    /* U+0062 "b" */
    0xc3, 0xc, 0x3e, 0xff, 0x3c, 0xf3, 0xcf, 0xff,
    0x80,

    /* U+0063 "c" */
    0x7f, 0xfc, 0x30, 0xc3, 0xf, 0xdf,

    /* U+0064 "d" */
    0xc, 0x30, 0xdf, 0xff, 0x3c, 0xf3, 0xcf, 0xf7,
    0xc0,

    /* U+0065 "e" */
    0x7b, 0xfc, 0xff, 0xff, 0xf, 0xdf,

    /* U+0066 "f" */
    0x3d, 0xf6, 0x3e, 0xf9, 0x86, 0x18, 0x61, 0x86,
    0x0,

    /* U+0067 "g" */
    0x7b, 0xfc, 0xf3, 0xcf, 0x3f, 0xdf, 0xf, 0xff,
    0x80,

    /* U+0068 "h" */
    0xc3, 0xc, 0x3e, 0xff, 0x3c, 0xf3, 0xcf, 0x3c,
    0xc0,

    /* U+0069 "i" */
    0xf3, 0xff, 0xfc,

    /* U+006A "j" */
    0x33, 0x3, 0x33, 0x33, 0x33, 0x33, 0xfe,

    /* U+006B "k" */
    0xc3, 0xc, 0x3e, 0xff, 0x3f, 0xfe, 0xdf, 0x3c,
    0xc0,

    /* U+006C "l" */
    0xff, 0xff, 0xfc,

    /* U+006D "m" */
    0xfe, 0xff, 0xdb, 0xdb, 0xdb, 0xdb, 0xdb, 0xdb,

    /* U+006E "n" */
    0xfb, 0xfc, 0xf3, 0xcf, 0x3c, 0xf3,

    /* U+006F "o" */
    0x7b, 0xfc, 0xf3, 0xcf, 0x3f, 0xde,

    /* U+0070 "p" */
    0x7b, 0xfc, 0xf3, 0xcf, 0x3f, 0xfe, 0xc3, 0xc,
    0x0,

    /* U+0071 "q" */
    0x7b, 0xfc, 0xf3, 0xcf, 0x3f, 0xdf, 0xc, 0x30,
    0xc0,

    /* U+0072 "r" */
    0xdf, 0xfe, 0x30, 0xc3, 0xc, 0x30,

    /* U+0073 "s" */
    0x7f, 0xfc, 0x3e, 0x7c, 0x3f, 0xfe,

    /* U+0074 "t" */
    0x61, 0x86, 0x3f, 0xfd, 0x86, 0x18, 0x61, 0xf3,
    0xc0,

    /* U+0075 "u" */
    0xcf, 0x3c, 0xf3, 0xcf, 0x3f, 0xde,

    /* U+0076 "v" */
    0xcf, 0x3c, 0xf3, 0xcf, 0xf7, 0x8c,

    /* U+0077 "w" */
    0xc3, 0xdb, 0xdb, 0xdb, 0xdb, 0xdb, 0xff, 0x7e,

    /* U+0078 "x" */
    0xc7, 0xdd, 0xf1, 0xc3, 0x8f, 0xbb, 0xe3,

    /* U+0079 "y" */
    0xcf, 0x3c, 0xf3, 0xcf, 0x3f, 0xdf, 0xf, 0xff,
    0x80,

    /* U+007A "z" */
    0xff, 0xf1, 0xce, 0x73, 0x8f, 0xff,

    /* U+007B "{" */
    0x3b, 0xd8, 0xc6, 0x73, 0xc, 0x63, 0x18, 0xf3,
    0x80,

    /* U+007C "|" */
    0xff, 0xff, 0xff, 0xc0,

    /* U+007D "}" */
    0xe7, 0x8c, 0x63, 0x1c, 0x66, 0x31, 0x8d, 0xee,
    0x0,

    /* U+007E "~" */
    0x63, 0xdb, 0xc6,

    /* U+00A1 "¡" */
    0xf3, 0xff, 0xfc,

    /* U+00A2 "¢" */
    0x30, 0xc7, 0xff, 0xc3, 0xc, 0x30, 0xfd, 0xf3,
    0xc,

    /* U+00A3 "£" */
    0x3c, 0xfd, 0x9b, 0x6, 0x1f, 0xbf, 0x30, 0x61,
    0xff, 0xf8,

    /* U+00A4 "¤" */
    0xc3, 0xff, 0x7e, 0x66, 0x66, 0x7e, 0xff, 0xc3,

    /* U+00A5 "¥" */
    0xcf, 0x3c, 0xff, 0x78, 0xcf, 0xcc, 0xfc, 0xc3,
    0x0,

    /* U+00A6 "¦" */
    0xff, 0xc0, 0xff, 0xc0,

    /* U+00A7 "§" */
    0x7f, 0xfc, 0x3c, 0x7b, 0xfc, 0xf3, 0xfd, 0xe3,
    0xc3, 0xff, 0xe0,

    /* U+00A8 "¨" */
    0xde, 0xc0,

    /* U+00A9 "©" */
    0x3e, 0x3f, 0xb8, 0xf9, 0xbd, 0x1e, 0x8f, 0x47,
    0x9b, 0xe3, 0xbf, 0x8f, 0x80,

    /* U+00AA "ª" */
    0xc5, 0xd6,

    /* U+00AB "«" */
    0x19, 0x9d, 0xdd, 0xdd, 0xce, 0xe3, 0xb8, 0xee,
    0x33,

    /* U+00AC "¬" */
    0xff, 0xf0, 0xc3,

    /* U+00AE "®" */
    0x3e, 0x3f, 0xb8, 0xfb, 0x3d, 0x5e, 0xcf, 0x57,
    0xab, 0xe3, 0xbf, 0x8f, 0x80,

    /* U+00B0 "°" */
    0x77, 0xf7, 0xf7, 0x0,

    /* U+00B1 "±" */
    0x30, 0xcf, 0xff, 0x30, 0xcf, 0xff,

    /* U+00B6 "¶" */
    0x7f, 0xff, 0xfb, 0xfb, 0xfb, 0x7b, 0x1b, 0x1b,
    0x1b, 0x1b, 0x1b, 0x1b, 0x1b, 0x1b,

    /* U+00BA "º" */
    0x77, 0xf7, 0xbf, 0xb8,

    /* U+00BB "»" */
    0xcc, 0x77, 0x1d, 0xc7, 0x73, 0xbb, 0xbb, 0xb9,
    0x98,

    /* U+00BF "¿" */
    0x30, 0xc0, 0xc, 0x31, 0xce, 0x30, 0xcf, 0xf7,
    0x80,

    /* U+00C0 "À" */
    0x30, 0xc1, 0x0, 0x7b, 0xfc, 0xf3, 0xcf, 0xff,
    0xf3, 0xcf, 0x3c, 0xc0,

    /* U+00C1 "Á" */
    0x30, 0xc2, 0x0, 0x7b, 0xfc, 0xf3, 0xcf, 0xff,
    0xf3, 0xcf, 0x3c, 0xc0,

    /* U+00C2 "Â" */
    0x31, 0xe4, 0x80, 0x7b, 0xfc, 0xf3, 0xcf, 0xff,
    0xf3, 0xcf, 0x3c, 0xc0,

    /* U+00C3 "Ã" */
    0x67, 0xf9, 0x80, 0x7b, 0xfc, 0xf3, 0xcf, 0xff,
    0xf3, 0xcf, 0x3c, 0xc0,

    /* U+00C4 "Ä" */
    0xcf, 0x30, 0x1e, 0xff, 0x3c, 0xf3, 0xff, 0xfc,
    0xf3, 0xcf, 0x30,

    /* U+00C5 "Å" */
    0x38, 0xa3, 0x80, 0x7b, 0xfc, 0xf3, 0xcf, 0xff,
    0xf3, 0xcf, 0x3c, 0xc0,

    /* U+00C6 "Æ" */
    0x7f, 0xff, 0xfc, 0xc3, 0x30, 0xcf, 0xbf, 0xef,
    0xc3, 0x30, 0xcc, 0x33, 0xfc, 0xfc,

    /* U+00C7 "Ç" */
    0x7f, 0xfc, 0x30, 0xc3, 0xc, 0x30, 0xc3, 0xf7,
    0xc2, 0x38,

    /* U+00C8 "È" */
    0x30, 0xc1, 0x0, 0x7f, 0xfc, 0x30, 0xfb, 0xec,
    0x30, 0xc3, 0xf7, 0xc0,

    /* U+00C9 "É" */
    0x30, 0xc2, 0x0, 0x7f, 0xfc, 0x30, 0xfb, 0xec,
    0x30, 0xc3, 0xf7, 0xc0,

    /* U+00CA "Ê" */
    0x31, 0xe4, 0x80, 0x7f, 0xfc, 0x30, 0xfb, 0xec,
    0x30, 0xc3, 0xf7, 0xc0,

    /* U+00CB "Ë" */
    0xcf, 0x30, 0x1f, 0xff, 0xc, 0x3e, 0xfb, 0xc,
    0x30, 0xfd, 0xf0,

    /* U+00CC "Ì" */
    0x30, 0xc1, 0x0, 0xff, 0xf3, 0xc, 0x30, 0xc3,
    0xc, 0x33, 0xff, 0xc0,

    /* U+00CD "Í" */
    0x30, 0xc2, 0x0, 0xff, 0xf3, 0xc, 0x30, 0xc3,
    0xc, 0x33, 0xff, 0xc0,

    /* U+00CE "Î" */
    0x31, 0xe4, 0x80, 0xff, 0xf3, 0xc, 0x30, 0xc3,
    0xc, 0x33, 0xff, 0xc0,

    /* U+00CF "Ï" */
    0xcf, 0x30, 0x3f, 0xfc, 0xc3, 0xc, 0x30, 0xc3,
    0xc, 0xff, 0xf0,

    /* U+00D1 "Ñ" */
    0x67, 0xf9, 0x80, 0xfb, 0xfc, 0xf3, 0xcf, 0x3c,
    0xf3, 0xcf, 0x3c, 0xc0,

    /* U+00D2 "Ò" */
    0x30, 0xc1, 0x0, 0x7b, 0xfc, 0xf3, 0xcf, 0x3c,
    0xf3, 0xcf, 0xf7, 0x80,

    /* U+00D3 "Ó" */
    0x30, 0xc2, 0x0, 0x7b, 0xfc, 0xf3, 0xcf, 0x3c,
    0xf3, 0xcf, 0xf7, 0x80,

    /* U+00D4 "Ô" */
    0x31, 0xe4, 0x80, 0x7b, 0xfc, 0xf3, 0xcf, 0x3c,
    0xf3, 0xcf, 0xf7, 0x80,

    /* U+00D5 "Õ" */
    0x67, 0xf9, 0x80, 0x7b, 0xfc, 0xf3, 0xcf, 0x3c,
    0xf3, 0xcf, 0xf7, 0x80,

    /* U+00D6 "Ö" */
    0xcf, 0x30, 0x1e, 0xff, 0x3c, 0xf3, 0xcf, 0x3c,
    0xf3, 0xfd, 0xe0,

    /* U+00D7 "×" */
    0xdf, 0xdd, 0xfd, 0x80,

    /* U+00D8 "Ø" */
    0x3d, 0x7f, 0x66, 0x66, 0x6e, 0x7e, 0x76, 0x66,
    0x66, 0xfe, 0xbc,

    /* U+00D9 "Ù" */
    0x30, 0xc1, 0x0, 0xcf, 0x3c, 0xf3, 0xcf, 0x3c,
    0xf3, 0xcf, 0xf7, 0x80,

    /* U+00DA "Ú" */
    0x30, 0xc2, 0x0, 0xcf, 0x3c, 0xf3, 0xcf, 0x3c,
    0xf3, 0xcf, 0xf7, 0x80,

    /* U+00DB "Û" */
    0x31, 0xe4, 0x80, 0xcf, 0x3c, 0xf3, 0xcf, 0x3c,
    0xf3, 0xcf, 0xf7, 0x80,

    /* U+00DC "Ü" */
    0xcf, 0x30, 0x33, 0xcf, 0x3c, 0xf3, 0xcf, 0x3c,
    0xf3, 0xfd, 0xe0,

    /* U+00DD "Ý" */
    0x30, 0xc2, 0x0, 0xcf, 0x3c, 0xf3, 0xfd, 0xe3,
    0xc, 0x30, 0xc3, 0x0,

    /* U+00DE "Þ" */
    0xc3, 0xf, 0xbf, 0xcf, 0x3c, 0xff, 0xfb, 0xc,
    0x0,

    /* U+00DF "ß" */
    0xf9, 0xfb, 0x36, 0x6d, 0x9b, 0xb3, 0xe3, 0xc7,
    0xbf, 0x70,

    /* U+00E0 "à" */
    0x30, 0xc1, 0x0, 0xfb, 0xf0, 0xdf, 0xff, 0x3f,
    0xdf,

    /* U+00E1 "á" */
    0x30, 0xc2, 0x0, 0xfb, 0xf0, 0xdf, 0xff, 0x3f,
    0xdf,

    /* U+00E2 "â" */
    0x31, 0xe4, 0x80, 0xfb, 0xf0, 0xdf, 0xff, 0x3f,
    0xdf,

    /* U+00E3 "ã" */
    0x67, 0xf9, 0x80, 0xfb, 0xf0, 0xdf, 0xff, 0x3f,
    0xdf,

    /* U+00E4 "ä" */
    0xcf, 0x30, 0x3e, 0xfc, 0x37, 0xff, 0xcf, 0xf7,
    0xc0,

    /* U+00E5 "å" */
    0x38, 0xa3, 0x80, 0xfb, 0xf0, 0xdf, 0xff, 0x3f,
    0xdf,

    /* U+00E6 "æ" */
    0xff, 0xbf, 0xf0, 0xcd, 0xff, 0xff, 0xf3, 0xf,
    0xfd, 0xff,

    /* U+00E7 "ç" */
    0x7f, 0xfc, 0x30, 0xc3, 0xf, 0xdf, 0x8, 0xe0,

    /* U+00E8 "è" */
    0x30, 0xc1, 0x0, 0x7b, 0xfc, 0xff, 0xff, 0xf,
    0xdf,

    /* U+00E9 "é" */
    0x30, 0xc2, 0x0, 0x7b, 0xfc, 0xff, 0xff, 0xf,
    0xdf,

    /* U+00EA "ê" */
    0x31, 0xe4, 0x80, 0x7b, 0xfc, 0xff, 0xff, 0xf,
    0xdf,

    /* U+00EB "ë" */
    0xcf, 0x30, 0x1e, 0xff, 0x3f, 0xff, 0xc3, 0xf7,
    0xc0,

    /* U+00EC "ì" */
    0xf4, 0xff, 0xff,

    /* U+00ED "í" */
    0xf8, 0xff, 0xff,

    /* U+00EE "î" */
    0x6f, 0x90, 0x66, 0x66, 0x66, 0x66,

    /* U+00EF "ï" */
    0xcf, 0x30, 0xc, 0x30, 0xc3, 0xc, 0x30, 0xc3,
    0x0,

    /* U+00F1 "ñ" */
    0x67, 0xf9, 0x80, 0xfb, 0xfc, 0xf3, 0xcf, 0x3c,
    0xf3,

    /* U+00F2 "ò" */
    0x30, 0xc1, 0x0, 0x7b, 0xfc, 0xf3, 0xcf, 0x3f,
    0xde,

    /* U+00F3 "ó" */
    0x30, 0xc2, 0x0, 0x7b, 0xfc, 0xf3, 0xcf, 0x3f,
    0xde,

    /* U+00F4 "ô" */
    0x31, 0xe4, 0x80, 0x7b, 0xfc, 0xf3, 0xcf, 0x3f,
    0xde,

    /* U+00F5 "õ" */
    0x67, 0xf9, 0x80, 0x7b, 0xfc, 0xf3, 0xcf, 0x3f,
    0xde,

    /* U+00F6 "ö" */
    0xcf, 0x30, 0x1e, 0xff, 0x3c, 0xf3, 0xcf, 0xf7,
    0x80,

    /* U+00F7 "÷" */
    0x30, 0xc0, 0x3f, 0xfc, 0x3, 0xc,

    /* U+00F8 "ø" */
    0x3d, 0x7f, 0x66, 0x6e, 0x76, 0x66, 0xfe, 0xbc,

    /* U+00F9 "ù" */
    0x30, 0xc1, 0x0, 0xcf, 0x3c, 0xf3, 0xcf, 0x3f,
    0xde,

    /* U+00FA "ú" */
    0x30, 0xc2, 0x0, 0xcf, 0x3c, 0xf3, 0xcf, 0x3f,
    0xde,

    /* U+00FB "û" */
    0x31, 0xe4, 0x80, 0xcf, 0x3c, 0xf3, 0xcf, 0x3f,
    0xde,

    /* U+00FC "ü" */
    0xcf, 0x30, 0x33, 0xcf, 0x3c, 0xf3, 0xcf, 0xf7,
    0x80,

    /* U+00FD "ý" */
    0x30, 0xc2, 0x0, 0xcf, 0x3c, 0xf3, 0xcf, 0x3f,
    0xdf, 0xf, 0xff, 0x80,

    /* U+00FE "þ" */
    0xc3, 0xc, 0x3e, 0xff, 0x3c, 0xf3, 0xcf, 0xff,
    0xb0, 0xc0,

    /* U+00FF "ÿ" */
    0xcf, 0x30, 0x33, 0xcf, 0x3c, 0xf3, 0xcf, 0xf7,
    0xc3, 0xff, 0xe0,

    /* U+0100 "Ā" */
    0x79, 0xe0, 0x1e, 0xff, 0x3c, 0xf3, 0xff, 0xfc,
    0xf3, 0xcf, 0x30,

    /* U+0101 "ā" */
    0x79, 0xe0, 0x3e, 0xfc, 0x37, 0xff, 0xcf, 0xf7,
    0xc0,

    /* U+0102 "Ă" */
    0x49, 0xe3, 0x0, 0x7b, 0xfc, 0xf3, 0xcf, 0xff,
    0xf3, 0xcf, 0x3c, 0xc0,

    /* U+0103 "ă" */
    0x49, 0xe3, 0x0, 0xfb, 0xf0, 0xdf, 0xff, 0x3f,
    0xdf,

    /* U+0104 "Ą" */
    0x79, 0xfb, 0x36, 0x6c, 0xdf, 0xbf, 0x66, 0xcd,
    0x9b, 0x30, 0x40, 0xe0,

    /* U+0105 "ą" */
    0xf9, 0xf8, 0x33, 0xef, 0xd9, 0xbf, 0x3e, 0x8,
    0x1c,

    /* U+0106 "Ć" */
    0x30, 0xc2, 0x0, 0x7f, 0xfc, 0x30, 0xc3, 0xc,
    0x30, 0xc3, 0xf7, 0xc0,

    /* U+0107 "ć" */
    0x30, 0xc2, 0x0, 0x7f, 0xfc, 0x30, 0xc3, 0xf,
    0xdf,

    /* U+0110 "Đ" */
    0x7c, 0xfd, 0x9b, 0x36, 0x7e, 0xd9, 0xb3, 0x66,
    0xfd, 0xf0,

    /* U+0111 "đ" */
    0xc, 0x3c, 0x33, 0xef, 0xd9, 0xb3, 0x66, 0xcd,
    0xf9, 0xe0,

    /* U+0112 "Ē" */
    0x79, 0xe0, 0x1f, 0xff, 0xc, 0x3e, 0xfb, 0xc,
    0x30, 0xfd, 0xf0,

    /* U+0113 "ē" */
    0x79, 0xe0, 0x1e, 0xff, 0x3f, 0xff, 0xc3, 0xf7,
    0xc0,

    /* U+0114 "Ĕ" */
    0x49, 0xe3, 0x0, 0x7f, 0xfc, 0x30, 0xfb, 0xec,
    0x30, 0xc3, 0xf7, 0xc0,

    /* U+0115 "ĕ" */
    0x49, 0xe3, 0x0, 0x7b, 0xfc, 0xff, 0xff, 0xf,
    0xdf,

    /* U+0118 "Ę" */
    0x7d, 0xfb, 0x6, 0xf, 0x9f, 0x30, 0x60, 0xc1,
    0xf9, 0xf0, 0x40, 0xe0,

    /* U+0119 "ę" */
    0x79, 0xfb, 0x37, 0xef, 0xd8, 0x3f, 0x3e, 0x8,
    0x1c,

    /* U+011E "Ğ" */
    0x49, 0xe3, 0x0, 0x7f, 0xfc, 0x30, 0xdf, 0x7c,
    0xf3, 0xcf, 0xf7, 0x80,

    /* U+011F "ğ" */
    0x49, 0xe3, 0x0, 0x7b, 0xfc, 0xf3, 0xcf, 0x3f,
    0xdf, 0xf, 0xff, 0x80,

    /* U+012A "Ī" */
    0x79, 0xe0, 0x3f, 0xfc, 0xc3, 0xc, 0x30, 0xc3,
    0xc, 0xff, 0xf0,

    /* U+012B "ī" */
    0xff, 0x6, 0x66, 0x66, 0x66, 0x60,

    /* U+012C "Ĭ" */
    0x49, 0xe3, 0x0, 0xff, 0xf3, 0xc, 0x30, 0xc3,
    0xc, 0x33, 0xff, 0xc0,

    /* U+012D "ĭ" */
    0x9f, 0x60, 0x66, 0x66, 0x66, 0x66,

    /* U+0130 "İ" */
    0x30, 0xc0, 0x3f, 0xfc, 0xc3, 0xc, 0x30, 0xc3,
    0xc, 0xff, 0xf0,

    /* U+0131 "ı" */
    0xff, 0xff,

    /* U+0141 "Ł" */
    0x30, 0x30, 0x30, 0x34, 0x38, 0x30, 0x70, 0xb0,
    0x30, 0x3f, 0x1f,

    /* U+0142 "ł" */
    0x30, 0xc3, 0xd, 0x38, 0xc7, 0x2c, 0x30, 0xc3,
    0x0,

    /* U+0143 "Ń" */
    0x30, 0xc2, 0x0, 0xfb, 0xfc, 0xf3, 0xcf, 0x3c,
    0xf3, 0xcf, 0x3c, 0xc0,

    /* U+0144 "ń" */
    0x30, 0xc2, 0x0, 0xfb, 0xfc, 0xf3, 0xcf, 0x3c,
    0xf3,

    /* U+014C "Ō" */
    0x79, 0xe0, 0x1e, 0xff, 0x3c, 0xf3, 0xcf, 0x3c,
    0xf3, 0xfd, 0xe0,

    /* U+014D "ō" */
    0x79, 0xe0, 0x1e, 0xff, 0x3c, 0xf3, 0xcf, 0xf7,
    0x80,

    /* U+014E "Ŏ" */
    0x49, 0xe3, 0x0, 0x7b, 0xfc, 0xf3, 0xcf, 0x3c,
    0xf3, 0xcf, 0xf7, 0x80,

    /* U+014F "ŏ" */
    0x49, 0xe3, 0x0, 0x7b, 0xfc, 0xf3, 0xcf, 0x3f,
    0xde,

    /* U+0152 "Œ" */
    0x7f, 0xff, 0xfc, 0xc3, 0x30, 0xcf, 0xb3, 0xec,
    0xc3, 0x30, 0xcc, 0x3f, 0xf7, 0xfc,

    /* U+0153 "œ" */
    0x7f, 0xbf, 0xfc, 0xcf, 0x3f, 0xcf, 0xf3, 0xf,
    0xfd, 0xff,

    /* U+015A "Ś" */
    0x30, 0xc2, 0x0, 0x7f, 0xfc, 0x30, 0xf9, 0xf0,
    0xc3, 0xf, 0xff, 0x80,

    /* U+015B "ś" */
    0x30, 0xc2, 0x0, 0x7f, 0xfc, 0x3e, 0x7c, 0x3f,
    0xfe,

    /* U+015E "Ş" */
    0x7f, 0xfc, 0x30, 0xf9, 0xf0, 0xc3, 0xf, 0xff,
    0x84, 0x70,

    /* U+015F "ş" */
    0x7f, 0xfc, 0x3e, 0x7c, 0x3f, 0xfe, 0x11, 0xc0,

    /* U+016A "Ū" */
    0x79, 0xe0, 0x33, 0xcf, 0x3c, 0xf3, 0xcf, 0x3c,
    0xf3, 0xfd, 0xe0,

    /* U+016B "ū" */
    0x79, 0xe0, 0x33, 0xcf, 0x3c, 0xf3, 0xcf, 0xf7,
    0x80,

    /* U+016C "Ŭ" */
    0x49, 0xe3, 0x0, 0xcf, 0x3c, 0xf3, 0xcf, 0x3c,
    0xf3, 0xcf, 0xf7, 0x80,

    /* U+016D "ŭ" */
    0x49, 0xe3, 0x0, 0xcf, 0x3c, 0xf3, 0xcf, 0x3f,
    0xde,

    /* U+0178 "Ÿ" */
    0xcf, 0x30, 0x33, 0xcf, 0x3c, 0xff, 0x78, 0xc3,
    0xc, 0x30, 0xc0,

    /* U+0179 "Ź" */
    0x30, 0xc2, 0x0, 0xff, 0xf1, 0xc6, 0x38, 0xc7,
    0x18, 0xe3, 0xff, 0xc0,

    /* U+017A "ź" */
    0x30, 0xc2, 0x0, 0xff, 0xf1, 0xce, 0x73, 0x8f,
    0xff,

    /* U+017B "Ż" */
    0x30, 0xc0, 0x3f, 0xfc, 0x71, 0x8e, 0x31, 0xc6,
    0x38, 0xff, 0xf0,

    /* U+017C "ż" */
    0x30, 0xc0, 0x3f, 0xfc, 0x73, 0x9c, 0xe3, 0xff,
    0xc0,

    /* U+01D3 "Ǔ" */
    0x49, 0xe3, 0x0, 0xcf, 0x3c, 0xf3, 0xcf, 0x3c,
    0xf3, 0xcf, 0xf7, 0x80,

    /* U+01D4 "ǔ" */
    0x49, 0xe3, 0x0, 0xcf, 0x3c, 0xf3, 0xcf, 0x3f,
    0xde,

    /* U+03BC "μ" */
    0xcf, 0x3c, 0xf3, 0xcf, 0x3f, 0xfe, 0xc3, 0x0
};


/*---------------------
 *  GLYPH DESCRIPTION
 *--------------------*/

static const lv_font_fmt_txt_glyph_dsc_t glyph_dsc[] = {
    {.bitmap_index = 0, .adv_w = 0, .box_w = 0, .box_h = 0, .ofs_x = 0, .ofs_y = 0} /* id = 0 reserved */,
    {.bitmap_index = 0, .adv_w = 16, .box_w = 1, .box_h = 1, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1, .adv_w = 80, .box_w = 1, .box_h = 1, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 2, .adv_w = 48, .box_w = 2, .box_h = 11, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 5, .adv_w = 96, .box_w = 5, .box_h = 3, .ofs_x = 0, .ofs_y = 8},
    {.bitmap_index = 7, .adv_w = 128, .box_w = 7, .box_h = 11, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 17, .adv_w = 112, .box_w = 6, .box_h = 12, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 26, .adv_w = 112, .box_w = 6, .box_h = 8, .ofs_x = 0, .ofs_y = 1},
    {.bitmap_index = 32, .adv_w = 160, .box_w = 9, .box_h = 11, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 45, .adv_w = 48, .box_w = 2, .box_h = 3, .ofs_x = 0, .ofs_y = 8},
    {.bitmap_index = 46, .adv_w = 80, .box_w = 4, .box_h = 13, .ofs_x = 0, .ofs_y = -2},
    {.bitmap_index = 53, .adv_w = 80, .box_w = 4, .box_h = 13, .ofs_x = 0, .ofs_y = -2},
    {.bitmap_index = 60, .adv_w = 96, .box_w = 5, .box_h = 5, .ofs_x = 0, .ofs_y = 6},
    {.bitmap_index = 64, .adv_w = 112, .box_w = 6, .box_h = 6, .ofs_x = 0, .ofs_y = 2},
    {.bitmap_index = 69, .adv_w = 48, .box_w = 2, .box_h = 3, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 70, .adv_w = 112, .box_w = 6, .box_h = 2, .ofs_x = 0, .ofs_y = 4},
    {.bitmap_index = 72, .adv_w = 48, .box_w = 2, .box_h = 2, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 73, .adv_w = 112, .box_w = 6, .box_h = 11, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 82, .adv_w = 112, .box_w = 6, .box_h = 11, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 91, .adv_w = 112, .box_w = 6, .box_h = 11, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 100, .adv_w = 112, .box_w = 6, .box_h = 11, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 109, .adv_w = 112, .box_w = 6, .box_h = 11, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 118, .adv_w = 112, .box_w = 6, .box_h = 11, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 127, .adv_w = 112, .box_w = 6, .box_h = 11, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 136, .adv_w = 112, .box_w = 6, .box_h = 11, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 145, .adv_w = 112, .box_w = 6, .box_h = 11, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 154, .adv_w = 112, .box_w = 6, .box_h = 11, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 163, .adv_w = 112, .box_w = 6, .box_h = 11, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 172, .adv_w = 48, .box_w = 2, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 174, .adv_w = 48, .box_w = 2, .box_h = 9, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 177, .adv_w = 96, .box_w = 5, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 182, .adv_w = 112, .box_w = 6, .box_h = 6, .ofs_x = 0, .ofs_y = 2},
    {.bitmap_index = 187, .adv_w = 96, .box_w = 5, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 192, .adv_w = 112, .box_w = 6, .box_h = 11, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 201, .adv_w = 160, .box_w = 9, .box_h = 11, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 214, .adv_w = 112, .box_w = 6, .box_h = 11, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 223, .adv_w = 112, .box_w = 6, .box_h = 11, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 232, .adv_w = 112, .box_w = 6, .box_h = 11, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 241, .adv_w = 112, .box_w = 6, .box_h = 11, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 250, .adv_w = 112, .box_w = 6, .box_h = 11, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 259, .adv_w = 112, .box_w = 6, .box_h = 11, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 268, .adv_w = 112, .box_w = 6, .box_h = 11, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 277, .adv_w = 112, .box_w = 6, .box_h = 11, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 286, .adv_w = 112, .box_w = 6, .box_h = 11, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 295, .adv_w = 112, .box_w = 6, .box_h = 11, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 304, .adv_w = 112, .box_w = 6, .box_h = 11, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 313, .adv_w = 112, .box_w = 6, .box_h = 11, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 322, .adv_w = 144, .box_w = 8, .box_h = 11, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 333, .adv_w = 112, .box_w = 6, .box_h = 11, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 342, .adv_w = 112, .box_w = 6, .box_h = 11, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 351, .adv_w = 112, .box_w = 6, .box_h = 11, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 360, .adv_w = 128, .box_w = 7, .box_h = 11, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 370, .adv_w = 112, .box_w = 6, .box_h = 11, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 379, .adv_w = 112, .box_w = 6, .box_h = 11, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 388, .adv_w = 112, .box_w = 6, .box_h = 11, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 397, .adv_w = 112, .box_w = 6, .box_h = 11, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 406, .adv_w = 112, .box_w = 6, .box_h = 11, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 415, .adv_w = 144, .box_w = 8, .box_h = 11, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 426, .adv_w = 128, .box_w = 7, .box_h = 11, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 436, .adv_w = 112, .box_w = 6, .box_h = 11, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 445, .adv_w = 112, .box_w = 6, .box_h = 11, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 454, .adv_w = 80, .box_w = 4, .box_h = 13, .ofs_x = 0, .ofs_y = -2},
    {.bitmap_index = 461, .adv_w = 112, .box_w = 6, .box_h = 11, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 470, .adv_w = 80, .box_w = 4, .box_h = 13, .ofs_x = 0, .ofs_y = -2},
    {.bitmap_index = 477, .adv_w = 112, .box_w = 6, .box_h = 4, .ofs_x = 0, .ofs_y = 7},
    {.bitmap_index = 480, .adv_w = 112, .box_w = 6, .box_h = 2, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 482, .adv_w = 112, .box_w = 6, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 488, .adv_w = 112, .box_w = 6, .box_h = 11, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 497, .adv_w = 112, .box_w = 6, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 503, .adv_w = 112, .box_w = 6, .box_h = 11, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 512, .adv_w = 112, .box_w = 6, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 518, .adv_w = 112, .box_w = 6, .box_h = 11, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 527, .adv_w = 112, .box_w = 6, .box_h = 11, .ofs_x = 0, .ofs_y = -3},
    {.bitmap_index = 536, .adv_w = 112, .box_w = 6, .box_h = 11, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 545, .adv_w = 48, .box_w = 2, .box_h = 11, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 548, .adv_w = 80, .box_w = 4, .box_h = 14, .ofs_x = 0, .ofs_y = -3},
    {.bitmap_index = 555, .adv_w = 112, .box_w = 6, .box_h = 11, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 564, .adv_w = 48, .box_w = 2, .box_h = 11, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 567, .adv_w = 144, .box_w = 8, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 575, .adv_w = 112, .box_w = 6, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 581, .adv_w = 112, .box_w = 6, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 587, .adv_w = 112, .box_w = 6, .box_h = 11, .ofs_x = 0, .ofs_y = -3},
    {.bitmap_index = 596, .adv_w = 112, .box_w = 6, .box_h = 11, .ofs_x = 0, .ofs_y = -3},
    {.bitmap_index = 605, .adv_w = 112, .box_w = 6, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 611, .adv_w = 112, .box_w = 6, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 617, .adv_w = 112, .box_w = 6, .box_h = 11, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 626, .adv_w = 112, .box_w = 6, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 632, .adv_w = 112, .box_w = 6, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 638, .adv_w = 144, .box_w = 8, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 646, .adv_w = 128, .box_w = 7, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 653, .adv_w = 112, .box_w = 6, .box_h = 11, .ofs_x = 0, .ofs_y = -3},
    {.bitmap_index = 662, .adv_w = 112, .box_w = 6, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 668, .adv_w = 96, .box_w = 5, .box_h = 13, .ofs_x = 0, .ofs_y = -2},
    {.bitmap_index = 677, .adv_w = 48, .box_w = 2, .box_h = 13, .ofs_x = 0, .ofs_y = -2},
    {.bitmap_index = 681, .adv_w = 96, .box_w = 5, .box_h = 13, .ofs_x = 0, .ofs_y = -2},
    {.bitmap_index = 690, .adv_w = 112, .box_w = 6, .box_h = 4, .ofs_x = 0, .ofs_y = 3},
    {.bitmap_index = 693, .adv_w = 48, .box_w = 2, .box_h = 11, .ofs_x = 0, .ofs_y = -3},
    {.bitmap_index = 696, .adv_w = 112, .box_w = 6, .box_h = 12, .ofs_x = 0, .ofs_y = -2},
    {.bitmap_index = 705, .adv_w = 128, .box_w = 7, .box_h = 11, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 715, .adv_w = 144, .box_w = 8, .box_h = 8, .ofs_x = 0, .ofs_y = 2},
    {.bitmap_index = 723, .adv_w = 112, .box_w = 6, .box_h = 11, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 732, .adv_w = 48, .box_w = 2, .box_h = 13, .ofs_x = 0, .ofs_y = -2},
    {.bitmap_index = 736, .adv_w = 112, .box_w = 6, .box_h = 14, .ofs_x = 0, .ofs_y = -3},
    {.bitmap_index = 747, .adv_w = 96, .box_w = 5, .box_h = 2, .ofs_x = 0, .ofs_y = 9},
    {.bitmap_index = 749, .adv_w = 160, .box_w = 9, .box_h = 11, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 762, .adv_w = 64, .box_w = 3, .box_h = 5, .ofs_x = 0, .ofs_y = 6},
    {.bitmap_index = 764, .adv_w = 160, .box_w = 9, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 773, .adv_w = 112, .box_w = 6, .box_h = 4, .ofs_x = 0, .ofs_y = 7},
    {.bitmap_index = 776, .adv_w = 160, .box_w = 9, .box_h = 11, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 789, .adv_w = 96, .box_w = 5, .box_h = 5, .ofs_x = 0, .ofs_y = 6},
    {.bitmap_index = 793, .adv_w = 112, .box_w = 6, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 799, .adv_w = 144, .box_w = 8, .box_h = 14, .ofs_x = 0, .ofs_y = -3},
    {.bitmap_index = 813, .adv_w = 96, .box_w = 5, .box_h = 6, .ofs_x = 0, .ofs_y = 5},
    {.bitmap_index = 817, .adv_w = 160, .box_w = 9, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 826, .adv_w = 112, .box_w = 6, .box_h = 11, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 835, .adv_w = 112, .box_w = 6, .box_h = 15, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 847, .adv_w = 112, .box_w = 6, .box_h = 15, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 859, .adv_w = 112, .box_w = 6, .box_h = 15, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 871, .adv_w = 112, .box_w = 6, .box_h = 15, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 883, .adv_w = 112, .box_w = 6, .box_h = 14, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 894, .adv_w = 112, .box_w = 6, .box_h = 15, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 906, .adv_w = 176, .box_w = 10, .box_h = 11, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 920, .adv_w = 112, .box_w = 6, .box_h = 13, .ofs_x = 0, .ofs_y = -2},
    {.bitmap_index = 930, .adv_w = 112, .box_w = 6, .box_h = 15, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 942, .adv_w = 112, .box_w = 6, .box_h = 15, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 954, .adv_w = 112, .box_w = 6, .box_h = 15, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 966, .adv_w = 112, .box_w = 6, .box_h = 14, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 977, .adv_w = 112, .box_w = 6, .box_h = 15, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 989, .adv_w = 112, .box_w = 6, .box_h = 15, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1001, .adv_w = 112, .box_w = 6, .box_h = 15, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1013, .adv_w = 112, .box_w = 6, .box_h = 14, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1024, .adv_w = 112, .box_w = 6, .box_h = 15, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1036, .adv_w = 112, .box_w = 6, .box_h = 15, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1048, .adv_w = 112, .box_w = 6, .box_h = 15, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1060, .adv_w = 112, .box_w = 6, .box_h = 15, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1072, .adv_w = 112, .box_w = 6, .box_h = 15, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1084, .adv_w = 112, .box_w = 6, .box_h = 14, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1095, .adv_w = 96, .box_w = 5, .box_h = 5, .ofs_x = 0, .ofs_y = 2},
    {.bitmap_index = 1099, .adv_w = 144, .box_w = 8, .box_h = 11, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1110, .adv_w = 112, .box_w = 6, .box_h = 15, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1122, .adv_w = 112, .box_w = 6, .box_h = 15, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1134, .adv_w = 112, .box_w = 6, .box_h = 15, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1146, .adv_w = 112, .box_w = 6, .box_h = 14, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1157, .adv_w = 112, .box_w = 6, .box_h = 15, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1169, .adv_w = 112, .box_w = 6, .box_h = 11, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1178, .adv_w = 128, .box_w = 7, .box_h = 11, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1188, .adv_w = 112, .box_w = 6, .box_h = 12, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1197, .adv_w = 112, .box_w = 6, .box_h = 12, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1206, .adv_w = 112, .box_w = 6, .box_h = 12, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1215, .adv_w = 112, .box_w = 6, .box_h = 12, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1224, .adv_w = 112, .box_w = 6, .box_h = 11, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1233, .adv_w = 112, .box_w = 6, .box_h = 12, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1242, .adv_w = 176, .box_w = 10, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1252, .adv_w = 112, .box_w = 6, .box_h = 10, .ofs_x = 0, .ofs_y = -2},
    {.bitmap_index = 1260, .adv_w = 112, .box_w = 6, .box_h = 12, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1269, .adv_w = 112, .box_w = 6, .box_h = 12, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1278, .adv_w = 112, .box_w = 6, .box_h = 12, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1287, .adv_w = 112, .box_w = 6, .box_h = 11, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1296, .adv_w = 48, .box_w = 2, .box_h = 12, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1299, .adv_w = 48, .box_w = 2, .box_h = 12, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1302, .adv_w = 80, .box_w = 4, .box_h = 12, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1308, .adv_w = 112, .box_w = 6, .box_h = 11, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1317, .adv_w = 112, .box_w = 6, .box_h = 12, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1326, .adv_w = 112, .box_w = 6, .box_h = 12, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1335, .adv_w = 112, .box_w = 6, .box_h = 12, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1344, .adv_w = 112, .box_w = 6, .box_h = 12, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1353, .adv_w = 112, .box_w = 6, .box_h = 12, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1362, .adv_w = 112, .box_w = 6, .box_h = 11, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1371, .adv_w = 112, .box_w = 6, .box_h = 8, .ofs_x = 0, .ofs_y = 1},
    {.bitmap_index = 1377, .adv_w = 144, .box_w = 8, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1385, .adv_w = 112, .box_w = 6, .box_h = 12, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1394, .adv_w = 112, .box_w = 6, .box_h = 12, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1403, .adv_w = 112, .box_w = 6, .box_h = 12, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1412, .adv_w = 112, .box_w = 6, .box_h = 11, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1421, .adv_w = 112, .box_w = 6, .box_h = 15, .ofs_x = 0, .ofs_y = -3},
    {.bitmap_index = 1433, .adv_w = 112, .box_w = 6, .box_h = 13, .ofs_x = 0, .ofs_y = -2},
    {.bitmap_index = 1443, .adv_w = 112, .box_w = 6, .box_h = 14, .ofs_x = 0, .ofs_y = -3},
    {.bitmap_index = 1454, .adv_w = 112, .box_w = 6, .box_h = 14, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1465, .adv_w = 112, .box_w = 6, .box_h = 11, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1474, .adv_w = 112, .box_w = 6, .box_h = 15, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1486, .adv_w = 112, .box_w = 6, .box_h = 12, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1495, .adv_w = 128, .box_w = 7, .box_h = 13, .ofs_x = 0, .ofs_y = -2},
    {.bitmap_index = 1507, .adv_w = 128, .box_w = 7, .box_h = 10, .ofs_x = 0, .ofs_y = -2},
    {.bitmap_index = 1516, .adv_w = 112, .box_w = 6, .box_h = 15, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1528, .adv_w = 112, .box_w = 6, .box_h = 12, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1537, .adv_w = 128, .box_w = 7, .box_h = 11, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1547, .adv_w = 128, .box_w = 7, .box_h = 11, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1557, .adv_w = 112, .box_w = 6, .box_h = 14, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1568, .adv_w = 112, .box_w = 6, .box_h = 11, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1577, .adv_w = 112, .box_w = 6, .box_h = 15, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1589, .adv_w = 112, .box_w = 6, .box_h = 12, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1598, .adv_w = 128, .box_w = 7, .box_h = 13, .ofs_x = 0, .ofs_y = -2},
    {.bitmap_index = 1610, .adv_w = 128, .box_w = 7, .box_h = 10, .ofs_x = 0, .ofs_y = -2},
    {.bitmap_index = 1619, .adv_w = 112, .box_w = 6, .box_h = 15, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1631, .adv_w = 112, .box_w = 6, .box_h = 15, .ofs_x = 0, .ofs_y = -3},
    {.bitmap_index = 1643, .adv_w = 112, .box_w = 6, .box_h = 14, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1654, .adv_w = 80, .box_w = 4, .box_h = 11, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1660, .adv_w = 112, .box_w = 6, .box_h = 15, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1672, .adv_w = 80, .box_w = 4, .box_h = 12, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1678, .adv_w = 112, .box_w = 6, .box_h = 14, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1689, .adv_w = 48, .box_w = 2, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1691, .adv_w = 144, .box_w = 8, .box_h = 11, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1702, .adv_w = 112, .box_w = 6, .box_h = 11, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1711, .adv_w = 112, .box_w = 6, .box_h = 15, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1723, .adv_w = 112, .box_w = 6, .box_h = 12, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1732, .adv_w = 112, .box_w = 6, .box_h = 14, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1743, .adv_w = 112, .box_w = 6, .box_h = 11, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1752, .adv_w = 112, .box_w = 6, .box_h = 15, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1764, .adv_w = 112, .box_w = 6, .box_h = 12, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1773, .adv_w = 176, .box_w = 10, .box_h = 11, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1787, .adv_w = 176, .box_w = 10, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1797, .adv_w = 112, .box_w = 6, .box_h = 15, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1809, .adv_w = 112, .box_w = 6, .box_h = 12, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1818, .adv_w = 112, .box_w = 6, .box_h = 13, .ofs_x = 0, .ofs_y = -2},
    {.bitmap_index = 1828, .adv_w = 112, .box_w = 6, .box_h = 10, .ofs_x = 0, .ofs_y = -2},
    {.bitmap_index = 1836, .adv_w = 112, .box_w = 6, .box_h = 14, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1847, .adv_w = 112, .box_w = 6, .box_h = 11, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1856, .adv_w = 112, .box_w = 6, .box_h = 15, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1868, .adv_w = 112, .box_w = 6, .box_h = 12, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1877, .adv_w = 112, .box_w = 6, .box_h = 14, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1888, .adv_w = 112, .box_w = 6, .box_h = 15, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1900, .adv_w = 112, .box_w = 6, .box_h = 12, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1909, .adv_w = 112, .box_w = 6, .box_h = 14, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1920, .adv_w = 112, .box_w = 6, .box_h = 11, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1929, .adv_w = 112, .box_w = 6, .box_h = 15, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1941, .adv_w = 112, .box_w = 6, .box_h = 12, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1950, .adv_w = 112, .box_w = 6, .box_h = 10, .ofs_x = 0, .ofs_y = -2}
};

/*---------------------
 *  CHARACTER MAPPING
 *--------------------*/

static const uint8_t glyph_id_ofs_list_3[] = {
    0, 1, 2, 3, 4, 5, 6, 7,
    8, 9, 10, 11, 0, 12, 0, 13,
    14, 0, 0, 0, 0, 15, 0, 0,
    0, 16, 17
};

static const uint16_t unicode_list_7[] = {
    0x0, 0x1, 0x2, 0x3, 0x4, 0x5, 0x8, 0x9,
    0xe, 0xf, 0x1a, 0x1b, 0x1c, 0x1d, 0x20, 0x21,
    0x31, 0x32, 0x33, 0x34, 0x3c, 0x3d, 0x3e, 0x3f,
    0x42, 0x43, 0x4a, 0x4b, 0x4e, 0x4f, 0x5a, 0x5b,
    0x5c, 0x5d, 0x68, 0x69, 0x6a, 0x6b, 0x6c, 0xc3,
    0xc4, 0x2ac
};

/*Collect the unicode lists and glyph_id offsets*/
static const lv_font_fmt_txt_cmap_t cmaps[] =
{
    {
        .range_start = 13, .range_length = 1, .glyph_id_start = 1,
        .unicode_list = NULL, .glyph_id_ofs_list = NULL, .list_length = 0, .type = LV_FONT_FMT_TXT_CMAP_FORMAT0_TINY
    },
    {
        .range_start = 32, .range_length = 64, .glyph_id_start = 2,
        .unicode_list = NULL, .glyph_id_ofs_list = NULL, .list_length = 0, .type = LV_FONT_FMT_TXT_CMAP_FORMAT0_TINY
    },
    {
        .range_start = 97, .range_length = 30, .glyph_id_start = 66,
        .unicode_list = NULL, .glyph_id_ofs_list = NULL, .list_length = 0, .type = LV_FONT_FMT_TXT_CMAP_FORMAT0_TINY
    },
    {
        .range_start = 161, .range_length = 27, .glyph_id_start = 96,
        .unicode_list = NULL, .glyph_id_ofs_list = glyph_id_ofs_list_3, .list_length = 27, .type = LV_FONT_FMT_TXT_CMAP_FORMAT0_FULL
    },
    {
        .range_start = 191, .range_length = 17, .glyph_id_start = 114,
        .unicode_list = NULL, .glyph_id_ofs_list = NULL, .list_length = 0, .type = LV_FONT_FMT_TXT_CMAP_FORMAT0_TINY
    },
    {
        .range_start = 209, .range_length = 31, .glyph_id_start = 131,
        .unicode_list = NULL, .glyph_id_ofs_list = NULL, .list_length = 0, .type = LV_FONT_FMT_TXT_CMAP_FORMAT0_TINY
    },
    {
        .range_start = 241, .range_length = 23, .glyph_id_start = 162,
        .unicode_list = NULL, .glyph_id_ofs_list = NULL, .list_length = 0, .type = LV_FONT_FMT_TXT_CMAP_FORMAT0_TINY
    },
    {
        .range_start = 272, .range_length = 685, .glyph_id_start = 185,
        .unicode_list = unicode_list_7, .glyph_id_ofs_list = NULL, .list_length = 42, .type = LV_FONT_FMT_TXT_CMAP_SPARSE_TINY
    }
};



/*--------------------
 *  ALL CUSTOM DATA
 *--------------------*/

#if LVGL_VERSION_MAJOR == 8
/*Store all the custom data of the font*/
static  lv_font_fmt_txt_glyph_cache_t cache;
#endif

#if LVGL_VERSION_MAJOR >= 8
static const lv_font_fmt_txt_dsc_t font_dsc = {
#else
static lv_font_fmt_txt_dsc_t font_dsc = {
#endif
    .glyph_bitmap = glyph_bitmap,
    .glyph_dsc = glyph_dsc,
    .cmaps = cmaps,
    .kern_dsc = NULL,
    .kern_scale = 0,
    .cmap_num = 8,
    .bpp = 1,
    .kern_classes = 0,
    .bitmap_format = 0,
#if LVGL_VERSION_MAJOR == 8
    .cache = &cache
#endif
};



/*-----------------
 *  PUBLIC FONT
 *----------------*/

/*Initialize a public general font descriptor*/
#if LVGL_VERSION_MAJOR >= 8
const lv_font_t font_distiller = {
#else
lv_font_t font_distiller = {
#endif
    .get_glyph_dsc = lv_font_get_glyph_dsc_fmt_txt,    /*Function pointer to get glyph's data*/
    .get_glyph_bitmap = lv_font_get_bitmap_fmt_txt,    /*Function pointer to get glyph's bitmap*/
    .line_height = 18,          /*The maximum line height required by the font*/
    .base_line = 3,             /*Baseline measured from the bottom of the line*/
#if !(LVGL_VERSION_MAJOR == 6 && LVGL_VERSION_MINOR == 0)
    .subpx = LV_FONT_SUBPX_NONE,
#endif
#if LV_VERSION_CHECK(7, 4, 0) || LVGL_VERSION_MAJOR >= 8
    .underline_position = 2,
    .underline_thickness = 1,
#endif
    .dsc = &font_dsc,          /*The custom font data. Will be accessed by `get_glyph_bitmap/dsc` */
#if LV_VERSION_CHECK(8, 2, 0) || LVGL_VERSION_MAJOR >= 9
    .fallback = NULL,
#endif
    .user_data = NULL,
};



#endif /*#if FONT_DISTILLER*/

