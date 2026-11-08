/**
 * Marlin 3D Printer Firmware
 * Copyright (c) 2020 MarlinFirmware [https://github.com/MarlinFirmware/Marlin]
 *
 * Based on Sprinter and grbl.
 * Copyright (c) 2011 Camiel Gubbels / Erik van der Zalm
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 */
#include "../../../../inc/MarlinConfigPre.h"

#if HAS_TFT_LVGL_UI

#include "../../../../MarlinCore.h"
#include "../../../../gcode/queue.h"

#include "lv_conf.h"
#include "draw_ui.h"

extern lv_group_t * g;
static lv_obj_t * scr;

enum {
  ID_CUSTOM_1 = 1,
  ID_CUSTOM_2,
  ID_CUSTOM_3,
  ID_CUSTOM_4,
  ID_CUSTOM_5,
  ID_CUSTOM_6,
  ID_CUSTOM_7,
  ID_M_RETURN
};

static void event_handler(lv_obj_t * obj, lv_event_t event) {
  if (event != LV_EVENT_RELEASED) return;
  switch (obj->mks_obj_id) {
    #if ENABLED(USER_CMD_1_ENABLE)
      case ID_CUSTOM_1: queue.inject_P(PSTR(USER_GCODE_1)); lv_return_to_ready(); break;
    #endif
    #if ENABLED(USER_CMD_2_ENABLE)
      case ID_CUSTOM_2: queue.inject_P(PSTR(USER_GCODE_2)); lv_return_to_ready(); break;
    #endif
    #if ENABLED(USER_CMD_3_ENABLE)
      case ID_CUSTOM_3: queue.inject_P(PSTR(USER_GCODE_3)); lv_return_to_ready(); break;
    #endif
    #if ENABLED(USER_CMD_4_ENABLE)
      case ID_CUSTOM_4: queue.inject_P(PSTR(USER_GCODE_4)); lv_return_to_ready(); break;
    #endif
    #if ENABLED(USER_CMD_5_ENABLE)
      case ID_CUSTOM_5: queue.inject_P(PSTR(USER_GCODE_5)); lv_return_to_ready(); break;
    #endif
    #if ENABLED(USER_CMD_6_ENABLE)
      case ID_CUSTOM_6: queue.inject_P(PSTR(USER_GCODE_6)); lv_return_to_ready(); break;
    #endif
    #if ENABLED(USER_CMD_7_ENABLE)
      case ID_CUSTOM_7: queue.inject_P(PSTR(USER_GCODE_7)); lv_return_to_ready(); break;
    #endif
    case ID_M_RETURN:
      lv_clear_more();
      lv_draw_tool();
      break;
  }
}

void lv_return_to_ready (void) {
  #ifdef CUSTOM_RETURN_READY_PRINT
    lv_clear_more();
    lv_draw_ready_print();
  #endif
}

void lv_draw_more(void) {
  scr = lv_screen_create(MORE_UI);

  // Enabled buttons are packed from the first slot, so disabling one doesn't leave a gap
  uint8_t slot = 0;
  #define MORE_BTN_X() (BTN_X_PIXEL * (slot % 4) + INTERVAL_V * (slot % 4 + 1))
  #define MORE_BTN_Y() (titleHeight + (slot / 4) * (BTN_Y_PIXEL + INTERVAL_H))

  #if ENABLED(USER_CMD_1_ENABLE)
    lv_big_button_create(scr, "F:/bmp_custom1.bin", more_menu.custom1, MORE_BTN_X(), MORE_BTN_Y(), event_handler, ID_CUSTOM_1);
    slot++;
  #endif

  #if ENABLED(USER_CMD_2_ENABLE)
    lv_big_button_create(scr, "F:/bmp_custom2.bin", more_menu.custom2, MORE_BTN_X(), MORE_BTN_Y(), event_handler, ID_CUSTOM_2);
    slot++;
  #endif

  #if ENABLED(USER_CMD_3_ENABLE)
    lv_big_button_create(scr, "F:/bmp_custom3.bin", more_menu.custom3, MORE_BTN_X(), MORE_BTN_Y(), event_handler, ID_CUSTOM_3);
    slot++;
  #endif

  #if ENABLED(USER_CMD_4_ENABLE)
    lv_big_button_create(scr, "F:/bmp_preset_tpu.bin", more_menu.custom4, MORE_BTN_X(), MORE_BTN_Y(), event_handler, ID_CUSTOM_4);
    slot++;
  #endif

  #if ENABLED(USER_CMD_5_ENABLE)
    lv_big_button_create(scr, "F:/bmp_preset_abs.bin", more_menu.custom5, MORE_BTN_X(), MORE_BTN_Y(), event_handler, ID_CUSTOM_5);
    slot++;
  #endif

  #if ENABLED(USER_CMD_6_ENABLE)
    lv_big_button_create(scr, "F:/bmp_preset_asa.bin", more_menu.custom6, MORE_BTN_X(), MORE_BTN_Y(), event_handler, ID_CUSTOM_6);
    slot++;
  #endif

  #if ENABLED(USER_CMD_7_ENABLE)
    lv_big_button_create(scr, "F:/bmp_custom7.bin", more_menu.custom7, MORE_BTN_X(), MORE_BTN_Y(), event_handler, ID_CUSTOM_7);
    slot++;
  #endif

  #undef MORE_BTN_X
  #undef MORE_BTN_Y

  lv_big_button_create(scr, "F:/bmp_return.bin", common_menu.text_back, BTN_X_PIXEL * 3 + INTERVAL_V * 4, BTN_Y_PIXEL + INTERVAL_H + titleHeight, event_handler, ID_M_RETURN);

}

void lv_clear_more() {
  #if HAS_ROTARY_ENCODER
    if (gCfgItems.encoder_enable) lv_group_remove_all_objs(g);
  #endif
  lv_obj_del(scr);
}

#endif // HAS_TFT_LVGL_UI
