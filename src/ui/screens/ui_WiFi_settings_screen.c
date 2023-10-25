// This file was generated by SquareLine Studio
// SquareLine Studio version: SquareLine Studio 1.3.2
// LVGL version: 8.3.6
// Project name: SquareLine_Project

#include "ui/ui.h"

void ui_WiFi_settings_screen_screen_init(void)
{
  ui_WiFi_settings_screen = lv_obj_create(NULL);
  lv_obj_clear_flag(ui_WiFi_settings_screen, LV_OBJ_FLAG_SCROLLABLE); /// Flags

  ui_WiFi_settings_title = lv_label_create(ui_WiFi_settings_screen);
  lv_obj_set_width(ui_WiFi_settings_title, 210);
  lv_obj_set_height(ui_WiFi_settings_title, LV_SIZE_CONTENT); /// 1
  lv_obj_set_x(ui_WiFi_settings_title, -5);
  lv_obj_set_y(ui_WiFi_settings_title, -126);
  lv_obj_set_align(ui_WiFi_settings_title, LV_ALIGN_CENTER);
  lv_label_set_text(ui_WiFi_settings_title, "Wi-Fi-asetukset");
  lv_obj_set_style_text_font(ui_WiFi_settings_title, &lv_font_montserrat_26, LV_PART_MAIN | LV_STATE_DEFAULT);

  ui_WiFi_settings_WiFi_pass_textarea = lv_textarea_create(ui_WiFi_settings_screen);
  lv_obj_set_width(ui_WiFi_settings_WiFi_pass_textarea, 224);
  lv_obj_set_height(ui_WiFi_settings_WiFi_pass_textarea, LV_SIZE_CONTENT); /// 70
  lv_obj_set_x(ui_WiFi_settings_WiFi_pass_textarea, 0);
  lv_obj_set_y(ui_WiFi_settings_WiFi_pass_textarea, 13);
  lv_obj_set_align(ui_WiFi_settings_WiFi_pass_textarea, LV_ALIGN_CENTER);
  lv_textarea_set_max_length(ui_WiFi_settings_WiFi_pass_textarea, 63);
  lv_textarea_set_placeholder_text(ui_WiFi_settings_WiFi_pass_textarea, "Salasana");
  lv_textarea_set_one_line(ui_WiFi_settings_WiFi_pass_textarea, true);

  ui_WiFi_settings_WiFi_pass_label = lv_label_create(ui_WiFi_settings_screen);
  lv_obj_set_width(ui_WiFi_settings_WiFi_pass_label, LV_SIZE_CONTENT);  /// 1
  lv_obj_set_height(ui_WiFi_settings_WiFi_pass_label, LV_SIZE_CONTENT); /// 1
  lv_obj_set_x(ui_WiFi_settings_WiFi_pass_label, 17);
  lv_obj_set_y(ui_WiFi_settings_WiFi_pass_label, -15);
  lv_obj_set_align(ui_WiFi_settings_WiFi_pass_label, LV_ALIGN_LEFT_MID);
  lv_label_set_text(ui_WiFi_settings_WiFi_pass_label, "Salasana");

  ui_WiFi_settings_OK_button = lv_btn_create(ui_WiFi_settings_screen);
  lv_obj_set_width(ui_WiFi_settings_OK_button, 76);
  lv_obj_set_height(ui_WiFi_settings_OK_button, 40);
  lv_obj_set_x(ui_WiFi_settings_OK_button, 73);
  lv_obj_set_y(ui_WiFi_settings_OK_button, 130);
  lv_obj_set_align(ui_WiFi_settings_OK_button, LV_ALIGN_CENTER);
  lv_obj_set_flex_flow(ui_WiFi_settings_OK_button, LV_FLEX_FLOW_ROW);
  lv_obj_set_flex_align(ui_WiFi_settings_OK_button, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
  lv_obj_add_flag(ui_WiFi_settings_OK_button, LV_OBJ_FLAG_SCROLL_ON_FOCUS); /// Flags
  lv_obj_clear_flag(ui_WiFi_settings_OK_button, LV_OBJ_FLAG_SCROLLABLE);    /// Flags

  ui_WiFi_settings_OK_button_label = lv_label_create(ui_WiFi_settings_OK_button);
  lv_obj_set_width(ui_WiFi_settings_OK_button_label, LV_SIZE_CONTENT);  /// 1
  lv_obj_set_height(ui_WiFi_settings_OK_button_label, LV_SIZE_CONTENT); /// 1
  lv_obj_set_x(ui_WiFi_settings_OK_button_label, -32);
  lv_obj_set_y(ui_WiFi_settings_OK_button_label, 49);
  lv_obj_set_align(ui_WiFi_settings_OK_button_label, LV_ALIGN_CENTER);
  lv_label_set_text(ui_WiFi_settings_OK_button_label, "OK");

  ui_WiFi_settings_cancel_button = lv_btn_create(ui_WiFi_settings_screen);
  lv_obj_set_width(ui_WiFi_settings_cancel_button, 78);
  lv_obj_set_height(ui_WiFi_settings_cancel_button, 40);
  lv_obj_set_x(ui_WiFi_settings_cancel_button, -73);
  lv_obj_set_y(ui_WiFi_settings_cancel_button, 130);
  lv_obj_set_align(ui_WiFi_settings_cancel_button, LV_ALIGN_CENTER);
  lv_obj_set_flex_flow(ui_WiFi_settings_cancel_button, LV_FLEX_FLOW_ROW);
  lv_obj_set_flex_align(ui_WiFi_settings_cancel_button, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
  lv_obj_add_flag(ui_WiFi_settings_cancel_button, LV_OBJ_FLAG_SCROLL_ON_FOCUS); /// Flags
  lv_obj_clear_flag(ui_WiFi_settings_cancel_button, LV_OBJ_FLAG_SCROLLABLE);    /// Flags
  lv_obj_set_style_bg_color(ui_WiFi_settings_cancel_button, lv_color_hex(0x848484), LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_bg_opa(ui_WiFi_settings_cancel_button, 255, LV_PART_MAIN | LV_STATE_DEFAULT);

  ui_WiFi_settings_cancel_button_label = lv_label_create(ui_WiFi_settings_cancel_button);
  lv_obj_set_width(ui_WiFi_settings_cancel_button_label, LV_SIZE_CONTENT);  /// 1
  lv_obj_set_height(ui_WiFi_settings_cancel_button_label, LV_SIZE_CONTENT); /// 1
  lv_obj_set_x(ui_WiFi_settings_cancel_button_label, -111);
  lv_obj_set_y(ui_WiFi_settings_cancel_button_label, 85);
  lv_obj_set_align(ui_WiFi_settings_cancel_button_label, LV_ALIGN_CENTER);
  lv_label_set_text(ui_WiFi_settings_cancel_button_label, "Peruuta");

  ui_WiFi_settings_WiFi_name_label = lv_label_create(ui_WiFi_settings_screen);
  lv_obj_set_width(ui_WiFi_settings_WiFi_name_label, LV_SIZE_CONTENT);  /// 1
  lv_obj_set_height(ui_WiFi_settings_WiFi_name_label, LV_SIZE_CONTENT); /// 1
  lv_obj_set_x(ui_WiFi_settings_WiFi_name_label, 17);
  lv_obj_set_y(ui_WiFi_settings_WiFi_name_label, -80);
  lv_obj_set_align(ui_WiFi_settings_WiFi_name_label, LV_ALIGN_LEFT_MID);
  lv_label_set_text(ui_WiFi_settings_WiFi_name_label, "Verkon nimi");

  ui_WiFi_settings_WiFi_name_textarea = lv_textarea_create(ui_WiFi_settings_screen);
  lv_obj_set_width(ui_WiFi_settings_WiFi_name_textarea, 224);
  lv_obj_set_height(ui_WiFi_settings_WiFi_name_textarea, LV_SIZE_CONTENT); /// 70
  lv_obj_set_x(ui_WiFi_settings_WiFi_name_textarea, 0);
  lv_obj_set_y(ui_WiFi_settings_WiFi_name_textarea, -52);
  lv_obj_set_align(ui_WiFi_settings_WiFi_name_textarea, LV_ALIGN_CENTER);
  lv_textarea_set_max_length(ui_WiFi_settings_WiFi_name_textarea, 32);
  lv_textarea_set_placeholder_text(ui_WiFi_settings_WiFi_name_textarea, "Nimi");
  lv_textarea_set_one_line(ui_WiFi_settings_WiFi_name_textarea, true);

  ui_WiFi_settings_keyboard = lv_keyboard_create(ui_WiFi_settings_screen);
  lv_obj_set_height(ui_WiFi_settings_keyboard, 128);
  lv_obj_set_width(ui_WiFi_settings_keyboard, lv_pct(100));
  lv_obj_set_align(ui_WiFi_settings_keyboard, LV_ALIGN_BOTTOM_MID);
  lv_obj_add_flag(ui_WiFi_settings_keyboard, LV_OBJ_FLAG_HIDDEN); /// Flags
}
