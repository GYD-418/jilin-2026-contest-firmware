#include "button.h"

typedef struct {
  GPIO_TypeDef *port;
  uint16_t      pin;
  uint8_t       current_level;
  uint8_t       last_level;
  uint8_t       stable_level;
  uint32_t      debounce_time;
  uint32_t      press_time;
  uint8_t       long_triggered;
  uint32_t      repeat_time;
  uint8_t       event;
} ButtonState_t;

static ButtonState_t g_btn[BUTTON_COUNT];

void Button_Init(void)
{
  uint8_t i;

  for (i = 0U; i < BUTTON_COUNT; i++) {
    g_btn[i].event = BUTTON_EVENT_NONE;
    g_btn[i].long_triggered = 0U;
    g_btn[i].debounce_time = 0U;
    g_btn[i].press_time = 0U;
    g_btn[i].repeat_time = 0U;
  }

  g_btn[0].port = GPIOE;
  g_btn[0].pin  = GPIO_PIN_10;
  g_btn[1].port = key2_GPIO_Port;
  g_btn[1].pin  = key2_Pin;
  g_btn[2].port = key3_GPIO_Port;
  g_btn[2].pin  = key3_Pin;
  g_btn[3].port = key4_GPIO_Port;
  g_btn[3].pin  = key4_Pin;

  for (i = 0U; i < BUTTON_COUNT; i++) {
    g_btn[i].current_level = (uint8_t)HAL_GPIO_ReadPin(g_btn[i].port, g_btn[i].pin);
    g_btn[i].last_level    = g_btn[i].current_level;
    g_btn[i].stable_level  = g_btn[i].current_level;
  }
}

void Button_Process(void)
{
  uint8_t  i;
  uint32_t now;

  now = HAL_GetTick();

  for (i = 0U; i < BUTTON_COUNT; i++) {
    g_btn[i].current_level = (uint8_t)HAL_GPIO_ReadPin(g_btn[i].port, g_btn[i].pin);

    if (g_btn[i].current_level != g_btn[i].last_level) {
      g_btn[i].debounce_time = now;
    }

    if ((now - g_btn[i].debounce_time) >= BUTTON_DEBOUNCE_MS) {
      if (g_btn[i].current_level != g_btn[i].stable_level) {
        g_btn[i].stable_level = g_btn[i].current_level;

        if (g_btn[i].stable_level == 0U) {
          /* 下降沿: 按下 */
          g_btn[i].press_time     = now;
          g_btn[i].long_triggered = 0U;
          g_btn[i].repeat_time    = now;
        } else {
          /* 上升沿: 释放 */
          if (g_btn[i].long_triggered == 0U) {
            g_btn[i].event = BUTTON_EVENT_SHORT_PRESS;
          }
        }
      }
    }

    /* 持续按下检测 */
    if (g_btn[i].stable_level == 0U) {
      if (g_btn[i].long_triggered == 0U) {
        if ((now - g_btn[i].press_time) >= BUTTON_LONG_PRESS_MS) {
          g_btn[i].long_triggered = 1U;
          g_btn[i].event          = BUTTON_EVENT_LONG_PRESS;
          g_btn[i].repeat_time    = now;
        }
      } else {
        if ((now - g_btn[i].repeat_time) >= BUTTON_REPEAT_MS) {
          g_btn[i].repeat_time = now;
          g_btn[i].event       = BUTTON_EVENT_REPEAT;
        }
      }
    }

    g_btn[i].last_level = g_btn[i].current_level;
  }
}

uint8_t Button_GetEvent(uint8_t button_id)
{
  uint8_t ev;

  if (button_id >= BUTTON_COUNT) {
    return BUTTON_EVENT_NONE;
  }

  ev = g_btn[button_id].event;
  g_btn[button_id].event = BUTTON_EVENT_NONE;
  return ev;
}

uint8_t Button_IsPressed(uint8_t button_id)
{
  if (button_id >= BUTTON_COUNT) {
    return 0U;
  }

  return (g_btn[button_id].stable_level == 0U) ? 1U : 0U;
}

uint8_t Button_GetAnyEvent(void)
{
  uint8_t i;
  uint8_t ev;

  for (i = 0U; i < BUTTON_COUNT; i++) {
    ev = g_btn[i].event;
    if (ev != BUTTON_EVENT_NONE) {
      g_btn[i].event = BUTTON_EVENT_NONE;
      return (uint8_t)((i << 4U) | ev);
    }
  }

  return 0U;
}
