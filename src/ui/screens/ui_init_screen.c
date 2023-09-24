// This file was generated by SquareLine Studio
// SquareLine Studio version: SquareLine Studio 1.3.2
// LVGL version: 8.3.6
// Project name: SquareLine_Project

#include "ui/ui.h"

void ui_init_screen_screen_init(void)
{
  ui_init_screen = lv_obj_create(NULL);
  lv_obj_clear_flag(ui_init_screen, LV_OBJ_FLAG_SCROLLABLE); /// Flags
  lv_obj_set_flex_flow(ui_init_screen, LV_FLEX_FLOW_COLUMN);
  lv_obj_set_flex_align(ui_init_screen, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

  ui_init_screen_spinner = lv_spinner_create(ui_init_screen, 1000, 90);
  lv_obj_set_width(ui_init_screen_spinner, 80);
  lv_obj_set_height(ui_init_screen_spinner, 80);
  lv_obj_set_x(ui_init_screen_spinner, -2);
  lv_obj_set_y(ui_init_screen_spinner, 1);
  lv_obj_set_align(ui_init_screen_spinner, LV_ALIGN_CENTER);
  lv_obj_clear_flag(ui_init_screen_spinner, LV_OBJ_FLAG_CLICKABLE); /// Flags
  lv_obj_set_style_arc_width(ui_init_screen_spinner, 5, LV_PART_MAIN | LV_STATE_DEFAULT);

  lv_obj_set_style_arc_width(ui_init_screen_spinner, 5, LV_PART_INDICATOR | LV_STATE_DEFAULT);

  ui_init_screen_label = lv_label_create(ui_init_screen);
  lv_obj_set_width(ui_init_screen_label, LV_SIZE_CONTENT);  /// 1
  lv_obj_set_height(ui_init_screen_label, LV_SIZE_CONTENT); /// 1
  lv_obj_set_x(ui_init_screen_label, -38);
  lv_obj_set_y(ui_init_screen_label, -109);
  lv_obj_set_align(ui_init_screen_label, LV_ALIGN_CENTER);
  lv_label_set_text(ui_init_screen_label, "Connecting to WiFi...");
}