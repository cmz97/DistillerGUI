#ifndef MONORAMA_14_H
#define MONORAMA_14_H

#ifdef LV_LVGL_H_INCLUDE_SIMPLE
#include "lvgl.h"
#else
#include "lvgl/lvgl.h"
#endif

#ifndef MONORAMA_14
#define MONORAMA_14 1
#endif

#if MONORAMA_14

// Declare the font as extern since it's defined in monorama_14.c
#if LVGL_VERSION_MAJOR >= 8
extern const lv_font_t monorama_14;
#else
extern lv_font_t monorama_14;
#endif

#endif /*#if MONORAMA_14*/

#endif /*MONORAMA_14_H*/ 