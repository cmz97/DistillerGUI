#ifndef DASHBOARD_UI_H
#define DASHBOARD_UI_H

#include "lvgl/lvgl.h"

// UI Style themes
typedef enum {
	DASHBOARD_STYLE_0,
	DASHBOARD_STYLE_1,
	DASHBOARD_STYLE_2,
	DASHBOARD_STYLE_3,
	DASHBOARD_STYLE_4,
} dashboard_style_t;

// Structure to hold references to UI elements that need to be updated
typedef struct {
	lv_obj_t *temp_bar;
	lv_obj_t *cpu_bar;
	lv_obj_t *mem_bar;
	lv_obj_t *temp_value;
	lv_obj_t *cpu_value;
	lv_obj_t *mem_value;
	dashboard_style_t current_style;
} dashboard_ui_elements_t;

// Function declarations
dashboard_ui_elements_t *dashboard_ui_create(lv_obj_t *parent);
dashboard_ui_elements_t *dashboard_ui_create_styled(lv_obj_t *parent,
						    dashboard_style_t style);
void dashboard_ui_update_health_bar(lv_obj_t *bar_container, float percentage);
void dashboard_ui_update_values(dashboard_ui_elements_t *ui_elements,
				float temperature, float cpu_usage,
				float ram_usage);
void dashboard_ui_switch_style(dashboard_ui_elements_t *ui_elements,
			       lv_obj_t *parent, dashboard_style_t new_style);

// Style-specific creation functions
dashboard_ui_elements_t *dashboard_ui_create_1(lv_obj_t *parent);
dashboard_ui_elements_t *dashboard_ui_create_2(lv_obj_t *parent);
dashboard_ui_elements_t *dashboard_ui_create_3(lv_obj_t *parent);
dashboard_ui_elements_t *dashboard_ui_create_4(lv_obj_t *parent);

#endif // DASHBOARD_UI_H
