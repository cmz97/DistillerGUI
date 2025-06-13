#include "dashboard_ui.h"
#include "font_distiller.h"
#include "system_monitor.h"
#include <stdio.h>

static dashboard_ui_elements_t *dashboard_ui_create_0(lv_obj_t *parent);

static lv_obj_t *create_stat_block(lv_obj_t *parent, lv_obj_t *previous_block,
				   const char *icon_text,
				   const char *label_text)
{
	lv_obj_t *block = lv_obj_create(parent);
	lv_obj_set_size(block, lv_pct(100), 20);

	if (previous_block) {
		lv_obj_align_to(block, previous_block, LV_ALIGN_OUT_BOTTOM_MID,
				0, 4);
	} else {
		lv_obj_align(block, LV_ALIGN_TOP_MID, 0, 0);
	}

	lv_obj_set_style_bg_opa(block, LV_OPA_TRANSP, LV_PART_MAIN);
	lv_obj_set_style_border_width(block, 0, LV_PART_MAIN);
	lv_obj_set_style_pad_all(block, 2, LV_PART_MAIN);

	lv_obj_t *icon = lv_obj_create(block);
	lv_obj_set_size(icon, 16, 16);
	lv_obj_align(icon, LV_ALIGN_LEFT_MID, 0, 0);
	lv_obj_set_style_bg_color(icon, lv_color_black(), LV_PART_MAIN);
	lv_obj_set_style_border_width(icon, 0, LV_PART_MAIN);

	lv_obj_t *icon_label = lv_label_create(icon);
	lv_label_set_text(icon_label, icon_text);
	lv_obj_set_style_text_color(icon_label, lv_color_white(), LV_PART_MAIN);
	lv_obj_set_style_text_font(icon_label, &font_distiller, LV_PART_MAIN);
	lv_obj_center(icon_label);

	lv_obj_t *label = lv_label_create(block);
	lv_label_set_text(label, label_text);
	lv_obj_set_style_text_color(label, lv_color_black(), LV_PART_MAIN);
	lv_obj_set_style_text_font(label, &font_distiller, LV_PART_MAIN);
	lv_obj_align_to(label, icon, LV_ALIGN_OUT_RIGHT_TOP, 4, -2);

	return block;
}

static lv_obj_t *create_value_label(lv_obj_t *parent, const char *initial_text)
{
	lv_obj_t *value = lv_label_create(parent);
	lv_label_set_text(value, initial_text);
	lv_obj_set_style_text_color(value, lv_color_black(), LV_PART_MAIN);
	lv_obj_set_style_text_font(value, &font_distiller, LV_PART_MAIN);
	lv_obj_align(value, LV_ALIGN_RIGHT_MID, 0, 0);

	return value;
}

dashboard_ui_elements_t *dashboard_ui_create(lv_obj_t *parent)
{
	return dashboard_ui_create_styled(parent, DASHBOARD_STYLE_0);
}

dashboard_ui_elements_t *dashboard_ui_create_styled(lv_obj_t *parent,
						    dashboard_style_t style)
{
	switch (style) {
	case DASHBOARD_STYLE_1:
		return dashboard_ui_create_1(parent);
	case DASHBOARD_STYLE_2:
		return dashboard_ui_create_2(parent);
	case DASHBOARD_STYLE_3:
		return dashboard_ui_create_3(parent);
	case DASHBOARD_STYLE_4:
		return dashboard_ui_create_4(parent);
	case DASHBOARD_STYLE_0:
	default:
		return dashboard_ui_create_0(parent);
	}
}

static dashboard_ui_elements_t *dashboard_ui_create_0(lv_obj_t *parent)
{
	static dashboard_ui_elements_t ui_elements = { 0 };
	ui_elements.current_style = DASHBOARD_STYLE_0;

	system_monitor_update();
	float ram_usage = system_monitor_get_ram_usage();
	float cpu_usage = system_monitor_get_cpu_usage();
	float temperature = system_monitor_get_temperature();

	lv_obj_t *dashboard = lv_obj_create(parent);
	lv_obj_set_size(dashboard, 238, 116);
	lv_obj_center(dashboard);
	lv_obj_set_style_bg_color(dashboard, lv_color_white(), LV_PART_MAIN);
	lv_obj_set_style_border_color(dashboard, lv_color_black(),
				      LV_PART_MAIN);
	lv_obj_set_style_border_width(dashboard, 3, LV_PART_MAIN);
	lv_obj_set_style_pad_all(dashboard, 8, LV_PART_MAIN);

	lv_obj_t *header = lv_obj_create(dashboard);
	lv_obj_set_size(header, lv_pct(100), 24);
	lv_obj_align(header, LV_ALIGN_TOP_MID, 0, 0);
	lv_obj_set_style_bg_color(header, lv_color_black(), LV_PART_MAIN);
	lv_obj_set_style_border_width(header, 1, LV_PART_MAIN);
	lv_obj_set_style_border_color(header, lv_color_black(), LV_PART_MAIN);
	lv_obj_set_style_pad_all(header, 4, LV_PART_MAIN);

	lv_obj_t *title = lv_label_create(header);
	lv_label_set_text(title, "Distiller");
	lv_obj_set_style_text_color(title, lv_color_white(), LV_PART_MAIN);
	lv_obj_set_style_text_font(title, &font_distiller, LV_PART_MAIN);
	lv_obj_align(title, LV_ALIGN_LEFT_MID, 4, 0);

	lv_obj_t *stats_area = lv_obj_create(dashboard);
	lv_obj_set_size(stats_area, lv_pct(100), 76);
	lv_obj_align_to(stats_area, header, LV_ALIGN_OUT_BOTTOM_MID, 0, 4);
	lv_obj_set_style_bg_opa(stats_area, LV_OPA_TRANSP, LV_PART_MAIN);
	lv_obj_set_style_border_width(stats_area, 0, LV_PART_MAIN);
	lv_obj_set_style_pad_all(stats_area, 0, LV_PART_MAIN);

	lv_obj_t *temp_block = create_stat_block(stats_area, NULL, "T", "TEMP");
	lv_obj_t *cpu_block =
		create_stat_block(stats_area, temp_block, "C", "CPU");
	lv_obj_t *mem_block =
		create_stat_block(stats_area, cpu_block, "M", "RAM");

	lv_obj_t *temp_label = lv_obj_get_child(temp_block, 1);
	lv_obj_t *cpu_label = lv_obj_get_child(cpu_block, 1);
	lv_obj_t *mem_label = lv_obj_get_child(mem_block, 1);

	char temp_str[16], cpu_str[16], mem_str[16];
	snprintf(temp_str, sizeof(temp_str), "%.0f°C", temperature);
	snprintf(cpu_str, sizeof(cpu_str), "%.0f%%", cpu_usage);
	snprintf(mem_str, sizeof(mem_str), "%.1fG", ram_usage * 8.0f / 100.0f);

	ui_elements.temp_value = create_value_label(temp_block, temp_str);
	ui_elements.cpu_value = create_value_label(cpu_block, cpu_str);
	ui_elements.mem_value = create_value_label(mem_block, mem_str);

	lv_obj_t *power_indicator = lv_obj_create(dashboard);
	lv_obj_set_size(power_indicator, 8, 8);
	lv_obj_align(power_indicator, LV_ALIGN_TOP_RIGHT, -4, 4);
	lv_obj_set_style_bg_color(power_indicator, lv_color_black(),
				  LV_PART_MAIN);
	lv_obj_set_style_border_width(power_indicator, 0, LV_PART_MAIN);
	lv_obj_set_style_radius(power_indicator, 4, LV_PART_MAIN);

	return &ui_elements;
}

dashboard_ui_elements_t *dashboard_ui_create_3(lv_obj_t *parent)
{
	static dashboard_ui_elements_t ui_elements = { 0 };
	ui_elements.current_style = DASHBOARD_STYLE_3;

	system_monitor_update();
	float ram_usage = system_monitor_get_ram_usage();
	float cpu_usage = system_monitor_get_cpu_usage();
	float temperature = system_monitor_get_temperature();

	lv_obj_t *dashboard = lv_obj_create(parent);
	lv_obj_set_size(dashboard, 238, 116);
	lv_obj_center(dashboard);
	lv_obj_set_style_bg_color(dashboard, lv_color_white(), LV_PART_MAIN);
	lv_obj_set_style_border_color(dashboard, lv_color_black(),
				      LV_PART_MAIN);
	lv_obj_set_style_border_width(dashboard, 3, LV_PART_MAIN);
	lv_obj_set_style_radius(dashboard, 8, LV_PART_MAIN);
	lv_obj_set_style_pad_all(dashboard, 8, LV_PART_MAIN);

	lv_obj_t *header = lv_obj_create(dashboard);
	lv_obj_set_size(header, lv_pct(120), 24);
	lv_obj_align(header, LV_ALIGN_TOP_MID, 0, 0);
	lv_obj_set_style_bg_color(header, lv_color_black(), LV_PART_MAIN);
	lv_obj_set_style_border_width(header, 0, LV_PART_MAIN);
	lv_obj_set_style_pad_all(header, 20, LV_PART_MAIN);

	lv_obj_t *title = lv_label_create(header);
	lv_label_set_text(title, "DISTILLER SYSTEM STATS");
	lv_obj_set_style_text_color(title, lv_color_white(), LV_PART_MAIN);
	lv_obj_set_style_text_font(title, &font_distiller, LV_PART_MAIN);
	lv_obj_align(title, LV_ALIGN_LEFT_MID, 2, 0);

	lv_obj_t *stats_area = lv_obj_create(dashboard);
	lv_obj_set_size(stats_area, lv_pct(120), 76);
	lv_obj_align_to(stats_area, header, LV_ALIGN_OUT_BOTTOM_MID, 0, 4);
	lv_obj_set_style_bg_opa(stats_area, LV_OPA_TRANSP, LV_PART_MAIN);
	lv_obj_set_style_border_width(stats_area, 0, LV_PART_MAIN);
	lv_obj_set_style_pad_all(stats_area, 0, LV_PART_MAIN);

	lv_obj_t *temp_prompt = lv_label_create(stats_area);
	lv_label_set_text(temp_prompt, "> TEMP:");
	lv_obj_set_style_text_color(temp_prompt, lv_color_black(),
				    LV_PART_MAIN);
	lv_obj_set_style_text_font(temp_prompt, &font_distiller, LV_PART_MAIN);
	lv_obj_align(temp_prompt, LV_ALIGN_TOP_LEFT, 20, 0);

	char temp_str[16];
	snprintf(temp_str, sizeof(temp_str), "%.0f°C", temperature);
	ui_elements.temp_value = lv_label_create(stats_area);
	lv_label_set_text(ui_elements.temp_value, temp_str);
	lv_obj_set_style_text_color(ui_elements.temp_value, lv_color_black(),
				    LV_PART_MAIN);
	lv_obj_set_style_text_font(ui_elements.temp_value, &font_distiller,
				   LV_PART_MAIN);
	lv_obj_align_to(ui_elements.temp_value, temp_prompt,
			LV_ALIGN_OUT_RIGHT_MID, 4, 0);

	lv_obj_t *cpu_prompt = lv_label_create(stats_area);
	lv_label_set_text(cpu_prompt, "> CPU:");
	lv_obj_set_style_text_color(cpu_prompt, lv_color_black(), LV_PART_MAIN);
	lv_obj_set_style_text_font(cpu_prompt, &font_distiller, LV_PART_MAIN);
	lv_obj_align_to(cpu_prompt, temp_prompt, LV_ALIGN_OUT_BOTTOM_LEFT, 0,
			4);

	char cpu_str[16];
	snprintf(cpu_str, sizeof(cpu_str), "%.0f%%", cpu_usage);
	ui_elements.cpu_value = lv_label_create(stats_area);
	lv_label_set_text(ui_elements.cpu_value, cpu_str);
	lv_obj_set_style_text_color(ui_elements.cpu_value, lv_color_black(),
				    LV_PART_MAIN);
	lv_obj_set_style_text_font(ui_elements.cpu_value, &font_distiller,
				   LV_PART_MAIN);
	lv_obj_align_to(ui_elements.cpu_value, cpu_prompt,
			LV_ALIGN_OUT_RIGHT_MID, 4, 0);

	lv_obj_t *mem_prompt = lv_label_create(stats_area);
	lv_label_set_text(mem_prompt, "> MEM:");
	lv_obj_set_style_text_color(mem_prompt, lv_color_black(), LV_PART_MAIN);
	lv_obj_set_style_text_font(mem_prompt, &font_distiller, LV_PART_MAIN);
	lv_obj_align_to(mem_prompt, cpu_prompt, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 4);

	char mem_str[16];
	snprintf(mem_str, sizeof(mem_str), "%.1fG", ram_usage * 8.0f / 100.0f);
	ui_elements.mem_value = lv_label_create(stats_area);
	lv_label_set_text(ui_elements.mem_value, mem_str);
	lv_obj_set_style_text_color(ui_elements.mem_value, lv_color_black(),
				    LV_PART_MAIN);
	lv_obj_set_style_text_font(ui_elements.mem_value, &font_distiller,
				   LV_PART_MAIN);
	lv_obj_align_to(ui_elements.mem_value, mem_prompt,
			LV_ALIGN_OUT_RIGHT_MID, 4, 0);

	return &ui_elements;
}

dashboard_ui_elements_t *dashboard_ui_create_4(lv_obj_t *parent)
{
	static dashboard_ui_elements_t ui_elements = { 0 };
	ui_elements.current_style = DASHBOARD_STYLE_4;

	system_monitor_update();
	float ram_usage = system_monitor_get_ram_usage();
	float cpu_usage = system_monitor_get_cpu_usage();
	float temperature = system_monitor_get_temperature();

	lv_obj_t *dashboard = lv_obj_create(parent);
	lv_obj_set_size(dashboard, 238, 116);
	lv_obj_center(dashboard);
	lv_obj_set_style_bg_color(dashboard, lv_color_white(), LV_PART_MAIN);
	lv_obj_set_style_border_color(dashboard, lv_color_black(),
				      LV_PART_MAIN);
	lv_obj_set_style_border_width(dashboard, 3, LV_PART_MAIN);
	lv_obj_set_style_pad_all(dashboard, 8, LV_PART_MAIN);

	lv_obj_t *header = lv_obj_create(dashboard);
	lv_obj_set_size(header, lv_pct(100), 18);
	lv_obj_align(header, LV_ALIGN_TOP_MID, 0, 0);
	lv_obj_set_style_bg_color(header, lv_color_white(), LV_PART_MAIN);
	lv_obj_set_style_border_color(header, lv_color_black(), LV_PART_MAIN);
	lv_obj_set_style_border_width(header, 1, LV_PART_MAIN);
	lv_obj_set_style_pad_all(header, 2, LV_PART_MAIN);

	lv_obj_t *title = lv_label_create(header);
	lv_label_set_text(title, "DISTILLER");
	lv_obj_set_style_text_color(title, lv_color_black(), LV_PART_MAIN);
	lv_obj_set_style_text_font(title, &font_distiller, LV_PART_MAIN);
	lv_obj_center(title);

	lv_obj_t *stats_area = lv_obj_create(dashboard);
	lv_obj_set_size(stats_area, lv_pct(100), 76);
	lv_obj_align_to(stats_area, header, LV_ALIGN_OUT_BOTTOM_MID, 0, 4);
	lv_obj_set_style_bg_opa(stats_area, LV_OPA_TRANSP, LV_PART_MAIN);
	lv_obj_set_style_border_width(stats_area, 0, LV_PART_MAIN);
	lv_obj_set_style_pad_all(stats_area, 0, LV_PART_MAIN);

	lv_obj_t *temp_block = lv_obj_create(stats_area);
	lv_obj_set_size(temp_block, lv_pct(100), 18);
	lv_obj_align(temp_block, LV_ALIGN_TOP_MID, 0, 0);
	lv_obj_set_style_border_color(temp_block, lv_color_black(),
				      LV_PART_MAIN);
	lv_obj_set_style_border_width(temp_block, 1, LV_PART_MAIN);
	lv_obj_set_style_pad_all(temp_block, 2, LV_PART_MAIN);

	lv_obj_t *temp_label = lv_label_create(temp_block);
	lv_label_set_text(temp_label, "THERMAL");
	lv_obj_set_style_text_color(temp_label, lv_color_black(), LV_PART_MAIN);
	lv_obj_set_style_text_font(temp_label, &font_distiller, LV_PART_MAIN);
	lv_obj_align(temp_label, LV_ALIGN_LEFT_MID, 4, 0);

	char temp_str[16];
	snprintf(temp_str, sizeof(temp_str), "%.0f°C", temperature);
	ui_elements.temp_value = lv_label_create(temp_block);
	lv_label_set_text(ui_elements.temp_value, temp_str);
	lv_obj_set_style_text_color(ui_elements.temp_value, lv_color_black(),
				    LV_PART_MAIN);
	lv_obj_set_style_text_font(ui_elements.temp_value, &font_distiller,
				   LV_PART_MAIN);
	lv_obj_align(ui_elements.temp_value, LV_ALIGN_RIGHT_MID, -4, 0);

	lv_obj_t *cpu_block = lv_obj_create(stats_area);
	lv_obj_set_size(cpu_block, lv_pct(100), 18);
	lv_obj_align_to(cpu_block, temp_block, LV_ALIGN_OUT_BOTTOM_MID, 0, 4);
	lv_obj_set_style_border_color(cpu_block, lv_color_black(),
				      LV_PART_MAIN);
	lv_obj_set_style_border_width(cpu_block, 1, LV_PART_MAIN);
	lv_obj_set_style_pad_all(cpu_block, 2, LV_PART_MAIN);

	lv_obj_t *cpu_label = lv_label_create(cpu_block);
	lv_label_set_text(cpu_label, "COMPUTE");
	lv_obj_set_style_text_color(cpu_label, lv_color_black(), LV_PART_MAIN);
	lv_obj_set_style_text_font(cpu_label, &font_distiller, LV_PART_MAIN);
	lv_obj_align(cpu_label, LV_ALIGN_LEFT_MID, 4, 0);

	char cpu_str[16];
	snprintf(cpu_str, sizeof(cpu_str), "%.0f%%", cpu_usage);
	ui_elements.cpu_value = lv_label_create(cpu_block);
	lv_label_set_text(ui_elements.cpu_value, cpu_str);
	lv_obj_set_style_text_color(ui_elements.cpu_value, lv_color_black(),
				    LV_PART_MAIN);
	lv_obj_set_style_text_font(ui_elements.cpu_value, &font_distiller,
				   LV_PART_MAIN);
	lv_obj_align(ui_elements.cpu_value, LV_ALIGN_RIGHT_MID, -4, 0);

	lv_obj_t *mem_block = lv_obj_create(stats_area);
	lv_obj_set_size(mem_block, lv_pct(100), 18);
	lv_obj_align_to(mem_block, cpu_block, LV_ALIGN_OUT_BOTTOM_MID, 0, 4);
	lv_obj_set_style_border_color(mem_block, lv_color_black(),
				      LV_PART_MAIN);
	lv_obj_set_style_border_width(mem_block, 1, LV_PART_MAIN);
	lv_obj_set_style_pad_all(mem_block, 2, LV_PART_MAIN);

	lv_obj_t *mem_label = lv_label_create(mem_block);
	lv_label_set_text(mem_label, "MEMORY");
	lv_obj_set_style_text_color(mem_label, lv_color_black(), LV_PART_MAIN);
	lv_obj_set_style_text_font(mem_label, &font_distiller, LV_PART_MAIN);
	lv_obj_align(mem_label, LV_ALIGN_LEFT_MID, 4, 0);

	char mem_str[16];
	snprintf(mem_str, sizeof(mem_str), "%.1fG", ram_usage * 8.0f / 100.0f);
	ui_elements.mem_value = lv_label_create(mem_block);
	lv_label_set_text(ui_elements.mem_value, mem_str);
	lv_obj_set_style_text_color(ui_elements.mem_value, lv_color_black(),
				    LV_PART_MAIN);
	lv_obj_set_style_text_font(ui_elements.mem_value, &font_distiller,
				   LV_PART_MAIN);
	lv_obj_align(ui_elements.mem_value, LV_ALIGN_RIGHT_MID, -4, 0);

	return &ui_elements;
}

void dashboard_ui_update_values(dashboard_ui_elements_t *ui_elements,
				float temperature, float cpu_usage,
				float ram_usage)
{
	if (!ui_elements)
		return;

	char temp_str[16], cpu_str[16], mem_str[16];
	snprintf(temp_str, sizeof(temp_str), "%.0f°C", temperature);
	snprintf(cpu_str, sizeof(cpu_str), "%.0f%%", cpu_usage);
	snprintf(mem_str, sizeof(mem_str), "%.1fG", ram_usage * 8.0f / 100.0f);

	lv_label_set_text(ui_elements->temp_value, temp_str);
	lv_label_set_text(ui_elements->cpu_value, cpu_str);
	lv_label_set_text(ui_elements->mem_value, mem_str);
}

dashboard_ui_elements_t *dashboard_ui_create_1(lv_obj_t *parent)
{
	static dashboard_ui_elements_t ui_elements = { 0 };
	ui_elements.current_style = DASHBOARD_STYLE_1;

	system_monitor_update();
	float ram_usage = system_monitor_get_ram_usage();
	float cpu_usage = system_monitor_get_cpu_usage();
	float temperature = system_monitor_get_temperature();

	lv_obj_t *dashboard = lv_obj_create(parent);
	lv_obj_set_size(dashboard, 238, 116);
	lv_obj_center(dashboard);
	lv_obj_set_style_bg_color(dashboard, lv_color_white(), LV_PART_MAIN);
	lv_obj_set_style_border_color(dashboard, lv_color_black(),
				      LV_PART_MAIN);
	lv_obj_set_style_border_width(dashboard, 4, LV_PART_MAIN);
	lv_obj_set_style_radius(dashboard, 0, LV_PART_MAIN);
	lv_obj_set_style_pad_all(dashboard, 8, LV_PART_MAIN);

	lv_obj_t *title_bar = lv_obj_create(dashboard);
	lv_obj_set_size(title_bar, lv_pct(100), 18);
	lv_obj_align(title_bar, LV_ALIGN_TOP_MID, 0, 0);
	lv_obj_set_style_bg_color(title_bar, lv_color_black(), LV_PART_MAIN);
	lv_obj_set_style_border_color(title_bar, lv_color_black(),
				      LV_PART_MAIN);
	lv_obj_set_style_border_width(title_bar, 2, LV_PART_MAIN);
	lv_obj_set_style_radius(title_bar, 0, LV_PART_MAIN);
	lv_obj_set_style_pad_all(title_bar, 2, LV_PART_MAIN);

	lv_obj_t *title = lv_label_create(title_bar);
	lv_label_set_text(title, "DISTILLER STATS");
	lv_obj_set_style_text_color(title, lv_color_white(), LV_PART_MAIN);
	lv_obj_set_style_text_font(title, &font_distiller, LV_PART_MAIN);
	lv_obj_center(title);

	lv_obj_t *stats_area = lv_obj_create(dashboard);
	lv_obj_set_size(stats_area, lv_pct(100), 76);
	lv_obj_align_to(stats_area, title_bar, LV_ALIGN_OUT_BOTTOM_MID, 0, 4);
	lv_obj_set_style_bg_opa(stats_area, LV_OPA_TRANSP, LV_PART_MAIN);
	lv_obj_set_style_border_width(stats_area, 0, LV_PART_MAIN);
	lv_obj_set_style_pad_all(stats_area, 0, LV_PART_MAIN);

	lv_obj_t *temp_block = lv_obj_create(stats_area);
	lv_obj_set_size(temp_block, lv_pct(100), 18);
	lv_obj_align(temp_block, LV_ALIGN_OUT_BOTTOM_MID, 0, 4);
	lv_obj_set_style_bg_color(temp_block, lv_color_white(), LV_PART_MAIN);
	lv_obj_set_style_border_color(temp_block, lv_color_black(),
				      LV_PART_MAIN);
	lv_obj_set_style_border_width(temp_block, 2, LV_PART_MAIN);

	lv_obj_t *temp_label = lv_label_create(temp_block);
	lv_label_set_text(temp_label, "HEAT");
	lv_obj_set_style_text_color(temp_label, lv_color_black(), LV_PART_MAIN);
	lv_obj_set_style_text_font(temp_label, &font_distiller, LV_PART_MAIN);
	lv_obj_align(temp_label, LV_ALIGN_LEFT_MID, 4, 0);

	char temp_str[16];
	snprintf(temp_str, sizeof(temp_str), "%.0f°C", temperature);
	ui_elements.temp_value = lv_label_create(temp_block);
	lv_label_set_text(ui_elements.temp_value, temp_str);
	lv_obj_set_style_text_color(ui_elements.temp_value, lv_color_black(),
				    LV_PART_MAIN);
	lv_obj_set_style_text_font(ui_elements.temp_value, &font_distiller,
				   LV_PART_MAIN);
	lv_obj_align(ui_elements.temp_value, LV_ALIGN_RIGHT_MID, -4, 0);

	lv_obj_t *cpu_block = lv_obj_create(stats_area);
	lv_obj_set_size(cpu_block, lv_pct(100), 18);
	lv_obj_align_to(cpu_block, temp_block, LV_ALIGN_OUT_BOTTOM_MID, 0, 4);
	lv_obj_set_style_bg_color(cpu_block, lv_color_white(), LV_PART_MAIN);
	lv_obj_set_style_border_color(cpu_block, lv_color_black(),
				      LV_PART_MAIN);
	lv_obj_set_style_border_width(cpu_block, 2, LV_PART_MAIN);

	lv_obj_t *cpu_label = lv_label_create(cpu_block);
	lv_label_set_text(cpu_label, "POWER");
	lv_obj_set_style_text_color(cpu_label, lv_color_black(), LV_PART_MAIN);
	lv_obj_set_style_text_font(cpu_label, &font_distiller, LV_PART_MAIN);
	lv_obj_align(cpu_label, LV_ALIGN_LEFT_MID, 4, 0);

	char cpu_str[16];
	snprintf(cpu_str, sizeof(cpu_str), "%.0f%%", cpu_usage);
	ui_elements.cpu_value = lv_label_create(cpu_block);
	lv_label_set_text(ui_elements.cpu_value, cpu_str);
	lv_obj_set_style_text_color(ui_elements.cpu_value, lv_color_black(),
				    LV_PART_MAIN);
	lv_obj_set_style_text_font(ui_elements.cpu_value, &font_distiller,
				   LV_PART_MAIN);
	lv_obj_align(ui_elements.cpu_value, LV_ALIGN_RIGHT_MID, -4, 0);

	lv_obj_t *mem_block = lv_obj_create(stats_area);
	lv_obj_set_size(mem_block, lv_pct(100), 18);
	lv_obj_align_to(mem_block, cpu_block, LV_ALIGN_OUT_BOTTOM_MID, 0, 4);
	lv_obj_set_style_bg_color(mem_block, lv_color_white(), LV_PART_MAIN);
	lv_obj_set_style_border_color(mem_block, lv_color_black(),
				      LV_PART_MAIN);
	lv_obj_set_style_border_width(mem_block, 2, LV_PART_MAIN);

	lv_obj_t *mem_label = lv_label_create(mem_block);
	lv_label_set_text(mem_label, "STORAGE");
	lv_obj_set_style_text_color(mem_label, lv_color_black(), LV_PART_MAIN);
	lv_obj_set_style_text_font(mem_label, &font_distiller, LV_PART_MAIN);
	lv_obj_align(mem_label, LV_ALIGN_LEFT_MID, 4, 0);

	char mem_str[16];
	snprintf(mem_str, sizeof(mem_str), "%.1fG", ram_usage * 8.0f / 100.0f);
	ui_elements.mem_value = lv_label_create(mem_block);
	lv_label_set_text(ui_elements.mem_value, mem_str);
	lv_obj_set_style_text_color(ui_elements.mem_value, lv_color_black(),
				    LV_PART_MAIN);
	lv_obj_set_style_text_font(ui_elements.mem_value, &font_distiller,
				   LV_PART_MAIN);
	lv_obj_align(ui_elements.mem_value, LV_ALIGN_RIGHT_MID, -4, 0);

	return &ui_elements;
}

dashboard_ui_elements_t *dashboard_ui_create_2(lv_obj_t *parent)
{
	static dashboard_ui_elements_t ui_elements = { 0 };
	ui_elements.current_style = DASHBOARD_STYLE_2;

	system_monitor_update();
	float ram_usage = system_monitor_get_ram_usage();
	float cpu_usage = system_monitor_get_cpu_usage();
	float temperature = system_monitor_get_temperature();

	lv_obj_t *dashboard = lv_obj_create(parent);
	lv_obj_set_size(dashboard, 238, 116);
	lv_obj_center(dashboard);
	lv_obj_set_style_bg_color(dashboard, lv_color_white(), LV_PART_MAIN);
	lv_obj_set_style_border_color(dashboard, lv_color_black(),
				      LV_PART_MAIN);
	lv_obj_set_style_border_width(dashboard, 3, LV_PART_MAIN);
	lv_obj_set_style_pad_all(dashboard, 8, LV_PART_MAIN);

	lv_obj_t *title_container = lv_obj_create(dashboard);
	lv_obj_set_size(title_container, lv_pct(100), 24);
	lv_obj_align(title_container, LV_ALIGN_TOP_MID, 0, 0);
	lv_obj_set_style_bg_color(title_container, lv_color_black(),
				  LV_PART_MAIN);
	lv_obj_set_style_border_color(title_container, lv_color_white(),
				      LV_PART_MAIN);
	lv_obj_set_style_border_width(title_container, 1, LV_PART_MAIN);
	lv_obj_set_style_pad_all(title_container, 2, LV_PART_MAIN);

	lv_obj_t *title = lv_label_create(title_container);
	lv_label_set_text(title, "~ SYSTEM STATS ~");
	lv_obj_set_style_text_color(title, lv_color_white(), LV_PART_MAIN);
	lv_obj_set_style_text_font(title, &font_distiller, LV_PART_MAIN);
	lv_obj_center(title);

	lv_obj_t *stats_area = lv_obj_create(dashboard);
	lv_obj_set_size(stats_area, lv_pct(100), 76);
	lv_obj_align_to(stats_area, title_container, LV_ALIGN_OUT_BOTTOM_MID, 0,
			4);
	lv_obj_set_style_bg_opa(stats_area, LV_OPA_TRANSP, LV_PART_MAIN);
	lv_obj_set_style_border_width(stats_area, 0, LV_PART_MAIN);
	lv_obj_set_style_pad_all(stats_area, 0, LV_PART_MAIN);

	lv_obj_t *temp_container = lv_obj_create(stats_area);
	lv_obj_set_size(temp_container, lv_pct(100), 20);
	lv_obj_align(temp_container, LV_ALIGN_TOP_MID, 0, 0);
	lv_obj_set_style_bg_opa(temp_container, LV_OPA_TRANSP, LV_PART_MAIN);
	lv_obj_set_style_border_width(temp_container, 0, LV_PART_MAIN);
	lv_obj_set_style_pad_all(temp_container, 2, LV_PART_MAIN);

	lv_obj_t *temp_icon = lv_obj_create(temp_container);
	lv_obj_set_size(temp_icon, 16, 16);
	lv_obj_align(temp_icon, LV_ALIGN_LEFT_MID, 0, 0);
	lv_obj_set_style_bg_color(temp_icon, lv_color_black(), LV_PART_MAIN);
	lv_obj_set_style_border_color(temp_icon, lv_color_black(),
				      LV_PART_MAIN);
	lv_obj_set_style_border_width(temp_icon, 1, LV_PART_MAIN);

	lv_obj_t *temp_label = lv_label_create(temp_container);
	lv_label_set_text(temp_label, "TEMP");
	lv_obj_set_style_text_color(temp_label, lv_color_black(), LV_PART_MAIN);
	lv_obj_set_style_text_font(temp_label, &font_distiller, LV_PART_MAIN);
	lv_obj_align_to(temp_label, temp_icon, LV_ALIGN_OUT_RIGHT_MID, 4, 0);

	char temp_str[16];
	snprintf(temp_str, sizeof(temp_str), "%.0f°C", temperature);
	ui_elements.temp_value = lv_label_create(temp_container);
	lv_label_set_text(ui_elements.temp_value, temp_str);
	lv_obj_set_style_text_color(ui_elements.temp_value, lv_color_black(),
				    LV_PART_MAIN);
	lv_obj_set_style_text_font(ui_elements.temp_value, &font_distiller,
				   LV_PART_MAIN);
	lv_obj_align(ui_elements.temp_value, LV_ALIGN_RIGHT_MID, 0, 0);

	lv_obj_t *cpu_container = lv_obj_create(stats_area);
	lv_obj_set_size(cpu_container, lv_pct(100), 20);
	lv_obj_align_to(cpu_container, temp_container, LV_ALIGN_OUT_BOTTOM_MID,
			0, 2);
	lv_obj_set_style_bg_opa(cpu_container, LV_OPA_TRANSP, LV_PART_MAIN);
	lv_obj_set_style_border_width(cpu_container, 0, LV_PART_MAIN);
	lv_obj_set_style_pad_all(cpu_container, 2, LV_PART_MAIN);

	lv_obj_t *cpu_icon = lv_obj_create(cpu_container);
	lv_obj_set_size(cpu_icon, 16, 16);
	lv_obj_align(cpu_icon, LV_ALIGN_LEFT_MID, 0, 0);
	lv_obj_set_style_bg_color(cpu_icon, lv_color_black(), LV_PART_MAIN);
	lv_obj_set_style_border_color(cpu_icon, lv_color_black(), LV_PART_MAIN);
	lv_obj_set_style_border_width(cpu_icon, 1, LV_PART_MAIN);

	lv_obj_t *cpu_label = lv_label_create(cpu_container);
	lv_label_set_text(cpu_label, "CPU");
	lv_obj_set_style_text_color(cpu_label, lv_color_black(), LV_PART_MAIN);
	lv_obj_set_style_text_font(cpu_label, &font_distiller, LV_PART_MAIN);
	lv_obj_align_to(cpu_label, cpu_icon, LV_ALIGN_OUT_RIGHT_MID, 4, 0);

	char cpu_str[16];
	snprintf(cpu_str, sizeof(cpu_str), "%.0f%%", cpu_usage);
	ui_elements.cpu_value = lv_label_create(cpu_container);
	lv_label_set_text(ui_elements.cpu_value, cpu_str);
	lv_obj_set_style_text_color(ui_elements.cpu_value, lv_color_black(),
				    LV_PART_MAIN);
	lv_obj_set_style_text_font(ui_elements.cpu_value, &font_distiller,
				   LV_PART_MAIN);
	lv_obj_align(ui_elements.cpu_value, LV_ALIGN_RIGHT_MID, 0, 0);

	lv_obj_t *mem_container = lv_obj_create(stats_area);
	lv_obj_set_size(mem_container, lv_pct(100), 20);
	lv_obj_align_to(mem_container, cpu_container, LV_ALIGN_OUT_BOTTOM_MID,
			0, 2);
	lv_obj_set_style_bg_opa(mem_container, LV_OPA_TRANSP, LV_PART_MAIN);
	lv_obj_set_style_border_width(mem_container, 0, LV_PART_MAIN);
	lv_obj_set_style_pad_all(mem_container, 2, LV_PART_MAIN);

	lv_obj_t *mem_icon = lv_obj_create(mem_container);
	lv_obj_set_size(mem_icon, 16, 16);
	lv_obj_align(mem_icon, LV_ALIGN_LEFT_MID, 0, 0);
	lv_obj_set_style_bg_color(mem_icon, lv_color_black(), LV_PART_MAIN);
	lv_obj_set_style_border_color(mem_icon, lv_color_black(), LV_PART_MAIN);
	lv_obj_set_style_border_width(mem_icon, 1, LV_PART_MAIN);

	lv_obj_t *mem_label = lv_label_create(mem_container);
	lv_label_set_text(mem_label, "MEM");
	lv_obj_set_style_text_color(mem_label, lv_color_black(), LV_PART_MAIN);
	lv_obj_set_style_text_font(mem_label, &font_distiller, LV_PART_MAIN);
	lv_obj_align_to(mem_label, mem_icon, LV_ALIGN_OUT_RIGHT_MID, 4, 0);

	char mem_str[16];
	snprintf(mem_str, sizeof(mem_str), "%.1fG", ram_usage * 8.0f / 100.0f);
	ui_elements.mem_value = lv_label_create(mem_container);
	lv_label_set_text(ui_elements.mem_value, mem_str);
	lv_obj_set_style_text_color(ui_elements.mem_value, lv_color_black(),
				    LV_PART_MAIN);
	lv_obj_set_style_text_font(ui_elements.mem_value, &font_distiller,
				   LV_PART_MAIN);
	lv_obj_align(ui_elements.mem_value, LV_ALIGN_RIGHT_MID, 0, 0);

	return &ui_elements;
}
