#include "water_control.h"
#include "pump.h"
#include "ultrasonic.h"
#include "flow_sensor.h"

static uint8_t  auto_mode;
static uint16_t target;
static uint32_t pump_run_start_tick;
static uint8_t  protection_active;

void WaterControl_Init(void)
{
  auto_mode          = 0U;
  target             = WATER_CONTROL_DEFAULT_TARGET_X10_CM;
  pump_run_start_tick = 0U;
  protection_active  = 0U;
}

void WaterControl_Process(uint32_t now_tick)
{
  uint16_t distance;
  uint16_t water_level;
  uint16_t low_thresh;
  uint16_t high_thresh;
  uint32_t run_duration;
  uint16_t flow_rate;

  if (auto_mode == 0U) {
    return;
  }

  if (protection_active == 1U) {
    Pump_RequestOff();
    return;
  }

  if (Ultrasonic_HasValidData() == 0U) {
    return;
  }

  distance    = Ultrasonic_GetLastDistanceX10Cm();
  water_level = Ultrasonic_ConvertDistanceToWaterLevelX10Cm(distance);

  low_thresh  = target - (WATER_CONTROL_HYSTERESIS_X10_CM / 2U);
  high_thresh = target + (WATER_CONTROL_HYSTERESIS_X10_CM / 2U);

  if (water_level < low_thresh) {
    Pump_RequestOn();
  } else if (water_level > high_thresh) {
    Pump_RequestOff();
  }

  if (Pump_IsOn() == 1U) {
    if (pump_run_start_tick == 0U) {
      pump_run_start_tick = now_tick;
    }

    run_duration = now_tick - pump_run_start_tick;

    if (run_duration > WATER_CONTROL_MAX_RUN_MS) {
      protection_active = 1U;
      Pump_RequestOff();
      return;
    }

    flow_rate = FlowSensor_GetFlowMlPerMin();
    if (flow_rate < WATER_CONTROL_MIN_FLOW_ML_PER_MIN && run_duration > 10000U) {
      protection_active = 1U;
      Pump_RequestOff();
      return;
    }
  } else {
    pump_run_start_tick = 0U;
  }
}

void WaterControl_SetTargetX10Cm(uint16_t val)
{
  target = val;
}

uint16_t WaterControl_GetTargetX10Cm(void)
{
  return target;
}

void WaterControl_SetAutoMode(uint8_t enable)
{
  if (enable == 0U) {
    auto_mode = 0U;
    Pump_RequestOff();
    protection_active = 0U;
  } else {
    auto_mode = 1U;
  }
}

uint8_t WaterControl_IsAutoMode(void)
{
  return auto_mode;
}

uint8_t WaterControl_IsPumpProtectionActive(void)
{
  return protection_active;
}

const char *WaterControl_GetStateString(void)
{
  if (protection_active == 1U) {
    return "PROTECT";
  }
  if (auto_mode == 1U) {
    if (Pump_IsOn() == 1U) {
      return "AUTO:ON";
    } else {
      return "AUTO:OFF";
    }
  }
  return "MANUAL";
}
