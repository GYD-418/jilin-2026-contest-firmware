#ifndef __BUTTON_H__
#define __BUTTON_H__

#include "main.h"
#include <stdint.h>

#define BUTTON_COUNT       4U

/* 按键ID */
#define BUTTON_KEY1        0U  /* PE10 - 菜单/选择 */
#define BUTTON_KEY2        1U  /* PE11 - 上/增加 */
#define BUTTON_KEY3        2U  /* PE12 - 下/减少 */
#define BUTTON_KEY4        3U  /* PE13 - 返回/取消 */

/* 按键事件类型 */
#define BUTTON_EVENT_NONE        0U
#define BUTTON_EVENT_SHORT_PRESS 1U  /* 短按 < 800ms */
#define BUTTON_EVENT_LONG_PRESS  2U  /* 长按 >= 800ms */
#define BUTTON_EVENT_REPEAT      3U  /* 长按连发 (每200ms) */

#define BUTTON_DEBOUNCE_MS       30U   /* 消抖时间 */
#define BUTTON_LONG_PRESS_MS     800U  /* 长按判定时间 */
#define BUTTON_REPEAT_MS         200U  /* 连发间隔 */

void Button_Init(void);
void Button_Process(void);
uint8_t Button_GetEvent(uint8_t button_id);
uint8_t Button_IsPressed(uint8_t button_id);
uint8_t Button_GetAnyEvent(void);

#endif
