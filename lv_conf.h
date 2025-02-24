#if 1 /*Set it to "1" to enable content*/

#ifndef LV_CONF_H
#define LV_CONF_H

#include <stdint.h>

/* Core features */
#define LV_USE_DRAW_SW 1
#define LV_USE_DRAW_SW_GRADIENT 1

/* Display driver features */
#define LV_USE_DRAW_BUF 1
#define LV_USE_DISP 1
#define LV_USE_DISP_DRV 1

/* Enable required display features */
#define LV_DRV_NO_CONF     1
#define LV_USE_DISPLAY     1

/* Basic features */
#define LV_USE_MEMCPY_BUILTIN 1
#define LV_USE_ASSERT_NULL 1
#define LV_USE_ASSERT_MEM 1

/* Memory settings */
#define LV_MEM_CUSTOM      0
#define LV_MEM_SIZE    (32U * 1024U)
#define LV_MEM_ATTR
#define LV_MEM_ADR          0
#define LV_MEM_CUSTOM_INCLUDE <stdlib.h>
#define LV_MEM_CUSTOM_ALLOC   malloc
#define LV_MEM_CUSTOM_FREE    free

/* Color settings */
#define LV_COLOR_DEPTH     32
#define LV_COLOR_16_SWAP   0
#define LV_COLOR_SCREEN_TRANSP    0

/* HAL settings */
#define LV_TICK_CUSTOM     0
#define LV_DPI_DEF         130
#define LV_DISP_DEF_REFR_PERIOD  30

/* Enable features */
#define LV_USE_THEME_DEFAULT 1
#define LV_FONT_MONTSERRAT_14 1
#define LV_USE_LABEL 1
#define LV_FONT_DEFAULT &lv_font_montserrat_14

/* Display buffer size */
#define LV_DISP_DEF_REFR_PERIOD    30
#define LV_DISP_BUF_SIZE           (128 * 1024)

/* Enable GPU */
#define LV_USE_GPU_ARM2D  0
#define LV_USE_GPU_STM32_DMA2D  0
#define LV_USE_GPU_SWM341_DMA2D  0
#define LV_USE_GPU_NXP_PXP  0
#define LV_USE_GPU_NXP_VG_LITE  0

/* Logging */
#define LV_USE_LOG      1
#define LV_LOG_LEVEL    LV_LOG_LEVEL_ERROR  /* Only show errors */
#define LV_LOG_PRINTF   1

#endif /*LV_CONF_H*/
#endif /*End of "Content enable"*/ 