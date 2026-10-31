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

#include "draw_ui.h"
#include <lv_conf.h>

#include "../../../../gcode/gcode.h"
#include "../../../../gcode/queue.h"
#include "../../../../inc/MarlinConfig.h"

extern lv_group_t *g;
static lv_obj_t *scr;
static lv_obj_t *action_label;

enum {
  ID_M_POINT1 = 1,
  ID_M_POINT2,
  ID_M_POINT3,
  ID_M_POINT4,
  ID_M_POINT5,
  #if ENABLED(AUTO_BED_LEVELING_BILINEAR)
  ID_M_AUTOLEVEL,
  ID_M_ZOFFSET,
  #endif
  ID_MANUAL_RETURN
};

static void event_handler(lv_obj_t *obj, lv_event_t event) {
  if (event != LV_EVENT_RELEASED) return;

  switch (obj->mks_obj_id) {
    case ID_M_POINT1:
      if (queue.length == 0) {
        if (uiCfg.leveling_first_time) {
          queue.enqueue_now_P(PSTR("G28"));
          uiCfg.leveling_first_time = 0;
        }
        lv_disp_action_status(ID_M_POINT1, 0);
        queue.enqueue_now_P(PSTR("G1 Z10"));
        sprintf_P(public_buf_l, PSTR("G1 X%d Y%d"), (int)gCfgItems.levelingPos[0][0], (int)gCfgItems.levelingPos[0][1]);
        queue.enqueue_one_now(public_buf_l);
        queue.enqueue_now_P(PSTR("G1 Z0"));
      }
      break;
    case ID_M_POINT2:
      if (queue.length == 0) {
        if (uiCfg.leveling_first_time) {
          queue.enqueue_now_P(PSTR("G28"));
          uiCfg.leveling_first_time = 0;
        }
        lv_disp_action_status(ID_M_POINT2, 0);
        queue.enqueue_now_P(PSTR("G1 Z10"));
        sprintf_P(public_buf_l, PSTR("G1 X%d Y%d"), (int)gCfgItems.levelingPos[1][0], (int)gCfgItems.levelingPos[1][1]);
        queue.enqueue_one_now(public_buf_l);
        queue.enqueue_now_P(PSTR("G1 Z0"));
      }
      break;
    case ID_M_POINT3:
      if (queue.length == 0) {
        if (uiCfg.leveling_first_time) {
          queue.enqueue_now_P(PSTR("G28"));
          uiCfg.leveling_first_time = 0;
        }
        lv_disp_action_status(ID_M_POINT3, 0);
        queue.enqueue_now_P(PSTR("G1 Z10"));
        sprintf_P(public_buf_l, PSTR("G1 X%d Y%d"), (int)gCfgItems.levelingPos[2][0], (int)gCfgItems.levelingPos[2][1]);
        queue.enqueue_one_now(public_buf_l);
        queue.enqueue_now_P(PSTR("G1 Z0"));
      }

      break;
    case ID_M_POINT4:
      if (queue.length == 0) {
        if (uiCfg.leveling_first_time) {
          queue.enqueue_now_P(PSTR("G28"));
          uiCfg.leveling_first_time = 0;
        }
        lv_disp_action_status(ID_M_POINT4, 0);
        queue.enqueue_now_P(PSTR("G1 Z10"));
        sprintf_P(public_buf_l, PSTR("G1 X%d Y%d"), (int)gCfgItems.levelingPos[3][0], (int)gCfgItems.levelingPos[3][1]);
        queue.enqueue_one_now(public_buf_l);
        queue.enqueue_now_P(PSTR("G1 Z0"));
      }
      break;
    case ID_M_POINT5:
      if (queue.length == 0) {
        if (uiCfg.leveling_first_time) {
          queue.enqueue_now_P(PSTR("G28"));
          uiCfg.leveling_first_time = 0;
        }
        lv_disp_action_status(ID_M_POINT5, 0);
        queue.enqueue_now_P(PSTR("G1 Z10"));
        sprintf_P(public_buf_l, PSTR("G1 X%d Y%d"), (int)gCfgItems.levelingPos[4][0], (int)gCfgItems.levelingPos[4][1]);
        queue.enqueue_one_now(public_buf_l);
        queue.enqueue_now_P(PSTR("G1 Z0"));
      }
      break;
    #if ENABLED(AUTOLEVELING_IN_MANUAL_LEVELING_MENU)
    #if ENABLED(AUTO_BED_LEVELING_BILINEAR)
    case ID_M_AUTOLEVEL:
      lv_disp_action_status(ID_M_AUTOLEVEL, 1);
      uiCfg.autoLeveling = 1;
      get_gcode_command(AUTO_LEVELING_COMMAND_ADDR, (uint8_t *)public_buf_m);
      public_buf_m[sizeof(public_buf_m) - 1] = 0;
      gcode.process_subcommands_now_P(PSTR(public_buf_m));
      gcode.process_subcommands_now_P(PSTR("G28 X Y"));
      uiCfg.autoLeveling = 0;
      lv_disp_action_status(ID_M_AUTOLEVEL, 2);
      break;
    case ID_M_ZOFFSET:
      lv_clear_manualLevel();
      #ifdef BLTOUCH
      bltouch_do_init(false);
      lv_draw_bltouch_settings();
      #endif
      #ifdef TOUCH_MI_PROBE
      lv_draw_touchmi_settings();
      #endif
      lv_draw_manualLevel();
      break;
    #endif
    #endif

    case ID_MANUAL_RETURN:
      lv_clear_manualLevel();
      lv_draw_tool();
      break;
  }
}

void lv_draw_manualLevel(void) {
  disp_state_stack._disp_index = 0;
  ZERO(disp_state_stack._disp_state);
  last_disp_state = LEVELING_UI;

  scr = lv_screen_create(LEVELING_UI);
  // Create an Image button
  lv_obj_t *buttonPoint1 = lv_big_button_create(scr, "F:/bmp_leveling1.bin", leveling_menu.position1, INTERVAL_V, titleHeight, event_handler, ID_M_POINT1);
  lv_obj_clear_protect(buttonPoint1, LV_PROTECT_FOLLOW);
  action_label = lv_label_create(scr, 240, TITLE_YPOS, nullptr);
  lv_disp_action_status(ID_MANUAL_RETURN, 0);
  lv_big_button_create(scr, "F:/bmp_leveling2.bin", leveling_menu.position2, BTN_X_PIXEL + INTERVAL_V * 2, titleHeight, event_handler, ID_M_POINT2);
  lv_big_button_create(scr, "F:/bmp_leveling3.bin", leveling_menu.position3, BTN_X_PIXEL * 2 + INTERVAL_V * 3, titleHeight, event_handler, ID_M_POINT3);
  lv_big_button_create(scr, "F:/bmp_leveling4.bin", leveling_menu.position4, BTN_X_PIXEL * 3 + INTERVAL_V * 4, titleHeight, event_handler, ID_M_POINT4);
  lv_big_button_create(scr, "F:/bmp_leveling5.bin", leveling_menu.position5, INTERVAL_V, BTN_Y_PIXEL + INTERVAL_H + titleHeight, event_handler, ID_M_POINT5);
  #ifdef AUTOLEVELING_IN_MANUAL_LEVELING_MENU
  #if ENABLED(AUTO_BED_LEVELING_BILINEAR)
  lv_big_button_create(scr, "F:/bmp_leveling.bin", tool_menu.autoleveling, BTN_X_PIXEL * 2 + INTERVAL_V * 3, BTN_Y_PIXEL + INTERVAL_H + titleHeight, event_handler, ID_M_AUTOLEVEL);
//  lv_big_button_create(scr, "F:/bmp_leveling.bin", tool_menu.autoleveling, BTN_X_PIXEL * 1 + INTERVAL_V * 2, BTN_Y_PIXEL + INTERVAL_H + titleHeight, event_handler, ID_M_AUTOLEVEL);
//  lv_big_button_create(scr, "F:/bmp_zoffset.bin", move_menu.zoffset, BTN_X_PIXEL * 2 + INTERVAL_V * 3, BTN_Y_PIXEL + INTERVAL_H + titleHeight, event_handler, ID_M_ZOFFSET);
  #endif
  #endif
  lv_big_button_create(scr, "F:/bmp_return.bin", common_menu.text_back, BTN_X_PIXEL * 3 + INTERVAL_V * 4, BTN_Y_PIXEL + INTERVAL_H + titleHeight, event_handler, ID_MANUAL_RETURN);
}

void lv_disp_action_status(uint16_t action, uint16_t status) {
  char buf[30];
  switch (action)
  {
    case ID_M_POINT1:
      sprintf_P(buf, PSTR("Point: 1 -> X %d Y %d"), (int)gCfgItems.levelingPos[0][0], (int)gCfgItems.levelingPos[0][1]);
      break;
    case ID_M_POINT2:
      sprintf_P(buf, PSTR("Point: 2 -> X %d Y %d"), (int)gCfgItems.levelingPos[1][0], (int)gCfgItems.levelingPos[1][1]);
      break;
    case ID_M_POINT3:
      sprintf_P(buf, PSTR("Point: 3 -> X %d Y %d"), (int)gCfgItems.levelingPos[2][0], (int)gCfgItems.levelingPos[2][1]);
      break;
    case ID_M_POINT4:
      sprintf_P(buf, PSTR("Point: 4 -> X %d Y %d"), (int)gCfgItems.levelingPos[3][0], (int)gCfgItems.levelingPos[3][1]);
      break;
    case ID_M_POINT5:
      sprintf_P(buf, PSTR("Point: 5 -> X %d Y %d"), (int)gCfgItems.levelingPos[4][0], (int)gCfgItems.levelingPos[4][1]);
      break;
  #ifdef AUTOLEVELING_IN_MANUAL_LEVELING_MENU
    case ID_M_AUTOLEVEL:
      if (status == 1) {
        sprintf_P(buf, PSTR("Autolevel %s"), "processing");
      }
      else if (status == 2) {
        sprintf_P(buf, PSTR("Autolevel %s"), "done");
      }
      break;
  #endif
    case ID_MANUAL_RETURN:
      sprintf_P(buf, PSTR("%s"), " ");
      break;
  }
  lv_label_set_text(action_label, buf);
}

void lv_clear_manualLevel() {
  #if HAS_ROTARY_ENCODER
    if (gCfgItems.encoder_enable) lv_group_remove_all_objs(g);
  #endif
  lv_obj_del(scr);
}

#endif // HAS_TFT_LVGL_UI
