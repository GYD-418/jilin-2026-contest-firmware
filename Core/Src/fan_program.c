#include "fan_program.h"

#include "alarm_led.h"
#include "buzzer.h"
#include "motor_pwm.h"

#define FAN_PROGRAM_TOTAL_TIME_MS         60000U /* 转速程控总时长 */
#define FAN_PROGRAM_CONTROL_PERIOD_MS       100U /* 闭环调速更新周期 */
#define FAN_PROGRAM_MAX_TARGET_RPM         1000U /* 程控最高目标转速 */
#define FAN_PROGRAM_RPM_ALARM_THRESHOLD      20U /* 人为干扰报警偏差阈值 */
#define FAN_PROGRAM_MAX_DUTY_CHANGE_PER_STEP 10  /* 单次 PWM 最大变化步长 */
#define FAN_PROGRAM_STOP_RPM_THRESHOLD       30U /* 认为风扇已停下的转速阈值 */
#define FAN_PROGRAM_STOP_HOLD_MS            800U /* 停稳保持时间 */
#define FAN_PROGRAM_STARTUP_DUTY_PERCENT     42U /* 起转助推占空比 */
#define FAN_PROGRAM_MIN_RUNNING_DUTY_PERCENT 24U /* 低速运行最小占空比 */
#define FAN_PROGRAM_STARTUP_BOOST_MS        1200U/* 起转助推持续时间 */
#define FAN_PROGRAM_HIGH_SPEED_DUTY_MIN      72U /* 高速段最小占空比 */
#define FAN_PROGRAM_HIGH_SPEED_RPM_THRESHOLD 900U/* 进入高速段的目标转速阈值 */
#define FAN_PROGRAM_HIGH_SPEED_EXTRA_DUTY     1U /* 高速段额外补偿占空比 */
#define FAN_PROGRAM_NEAR_TOP_RPM_THRESHOLD   980U/* 接近满速时的抑制阈值 */
#define FAN_PROGRAM_NEAR_TOP_BRAKE_DUTY        4 /* 接近满速时减掉的占空比 */
#define FAN_PROGRAM_IN_BAND_THRESHOLD        15U /* 正常跟稳判定误差范围 */
#define FAN_PROGRAM_ARM_HOLD_MS             300U /* 跟稳后允许报警前的稳定时间 */
#define FAN_PROGRAM_ARM_MIN_SEGMENT_MS     1500U /* 普通分段最早报警判定时间 */
#define FAN_PROGRAM_LATE_SEGMENT_ARM_MIN_MS 3500U/* 后两段最早报警判定时间 */
#define FAN_PROGRAM_LATE_SEGMENT_IN_BAND     25U /* 后两段跟稳判定误差范围 */
#define FAN_PROGRAM_ALARM_CLEAR_THRESHOLD   15U /* 报警解除误差范围 */

typedef enum
{
    FAN_PROGRAM_STATE_IDLE = 0U,
    FAN_PROGRAM_STATE_BRAKE_TO_STOP = 1U,
    FAN_PROGRAM_STATE_RUNNING = 2U,
    FAN_PROGRAM_STATE_COAST_TO_STOP = 3U
} FanProgramRunState_t;

typedef struct
{
    uint8_t active;
    uint8_t alarm_active;
    uint8_t run_state;
    uint8_t duty_percent;
    uint16_t target_rpm;
    uint32_t start_tick;
    uint32_t last_control_tick;
    uint32_t elapsed_ms;
    uint32_t stop_detect_tick;
    uint32_t alarm_over_limit_tick;
    uint32_t in_band_tick;
    uint8_t alarm_armed;
    uint8_t last_segment_index;
} FanProgram_State_t;

static FanProgram_State_t g_fan_program = {0};
static uint16_t g_fan_custom_targets[6] = {0};
static uint8_t g_fan_use_custom_targets = 0U;

static uint8_t FanProgram_ClampDuty(int16_t duty)
{
    if (duty < 0)
    {
        return 0U;
    }

    if (duty > 100)
    {
        return 100U;
    }

    return (uint8_t)duty;
}

static uint16_t FanProgram_GetTargetByElapsedMs(uint32_t elapsed_ms)
{
    uint32_t segment_ms;
    uint8_t seg_index;

    if (g_fan_use_custom_targets != 0U)
    {
        seg_index = (uint8_t)(elapsed_ms / 10000U);
        if (seg_index >= 6U) { seg_index = 5U; }
        if (seg_index == 0U)
        {
            return (uint16_t)((elapsed_ms * g_fan_custom_targets[0]) / 10000U);
        }
        else
        {
            uint32_t seg_start = (uint32_t)seg_index * 10000U;
            uint32_t seg_elapsed = elapsed_ms - seg_start;
            uint16_t prev_target = g_fan_custom_targets[seg_index - 1U];
            uint16_t cur_target = g_fan_custom_targets[seg_index];
            int32_t delta = (int32_t)cur_target - (int32_t)prev_target;
            return (uint16_t)((int32_t)prev_target + (int32_t)((seg_elapsed * delta) / 10000U));
        }
    }

    if (elapsed_ms >= FAN_PROGRAM_TOTAL_TIME_MS)
    {
        return FAN_PROGRAM_MAX_TARGET_RPM;
    }

    if (elapsed_ms < 10000U)
    {
        return (uint16_t)((elapsed_ms * 500U) / 10000U);
    }

    if (elapsed_ms < 20000U)
    {
        segment_ms = elapsed_ms - 10000U;
        return (uint16_t)(500U + ((segment_ms * 100U) / 10000U));
    }

    if (elapsed_ms < 30000U)
    {
        segment_ms = elapsed_ms - 20000U;
        return (uint16_t)(600U + ((segment_ms * 100U) / 10000U));
    }

    if (elapsed_ms < 40000U)
    {
        segment_ms = elapsed_ms - 30000U;
        return (uint16_t)(700U + ((segment_ms * 100U) / 10000U));
    }

    if (elapsed_ms < 50000U)
    {
        segment_ms = elapsed_ms - 40000U;
        return (uint16_t)(800U + ((segment_ms * 100U) / 10000U));
    }

    segment_ms = elapsed_ms - 50000U;
    return (uint16_t)(900U + ((segment_ms * 100U) / 10000U));
}

static uint8_t FanProgram_GetSegmentIndexByElapsedMs(uint32_t elapsed_ms)
{
    uint8_t index = (uint8_t)(elapsed_ms / 10000U);

    if (index > 5U)
    {
        index = 5U;
    }

    return index;
}

static uint32_t FanProgram_GetArmMinSegmentMs(uint8_t segment_index)
{
    if (segment_index >= 4U)
    {
        return FAN_PROGRAM_LATE_SEGMENT_ARM_MIN_MS;
    }

    return FAN_PROGRAM_ARM_MIN_SEGMENT_MS;
}

static uint32_t FanProgram_GetInBandThreshold(uint8_t segment_index)
{
    if (segment_index >= 4U)
    {
        return FAN_PROGRAM_LATE_SEGMENT_IN_BAND;
    }

    return FAN_PROGRAM_IN_BAND_THRESHOLD;
}

static void FanProgram_SetAlarm(uint8_t enable)
{
    if (enable != 0U)
    {
        AlarmLed_AutoOn();
        Buzzer_AutoOn();
        g_fan_program.alarm_active = 1U;
    }
    else
    {
        AlarmLed_AutoOff();
        Buzzer_AutoOff();
        g_fan_program.alarm_active = 0U;
    }
}

void FanProgram_Init(void)
{
    g_fan_program.active = 0U;
    g_fan_program.alarm_active = 0U;
    g_fan_program.run_state = FAN_PROGRAM_STATE_IDLE;
    g_fan_program.duty_percent = MotorPwm_GetDutyPercent();
    g_fan_program.target_rpm = 0U;
    g_fan_program.start_tick = 0U;
    g_fan_program.last_control_tick = 0U;
    g_fan_program.elapsed_ms = 0U;
    g_fan_program.stop_detect_tick = 0U;
    g_fan_program.alarm_over_limit_tick = 0U;
    g_fan_program.in_band_tick = 0U;
    g_fan_program.alarm_armed = 0U;
    g_fan_program.last_segment_index = 0U;
}

void FanProgram_Start(void)
{
    g_fan_program.active = 1U;
    g_fan_program.alarm_active = 0U;
    g_fan_program.run_state = FAN_PROGRAM_STATE_BRAKE_TO_STOP;
    g_fan_program.start_tick = 0U;
    g_fan_program.last_control_tick = 0U;
    g_fan_program.elapsed_ms = 0U;
    g_fan_program.target_rpm = 0U;
    g_fan_program.stop_detect_tick = 0U;
    g_fan_program.alarm_over_limit_tick = 0U;
    g_fan_program.in_band_tick = 0U;
    g_fan_program.alarm_armed = 0U;
    g_fan_program.last_segment_index = 0U;
    g_fan_program.duty_percent = 0U;
    MotorPwm_SetDutyPercent(0U);
    FanProgram_SetAlarm(0U);
}

void FanProgram_Stop(void)
{
    g_fan_program.active = 0U;
    g_fan_program.run_state = FAN_PROGRAM_STATE_IDLE;
    g_fan_program.target_rpm = 0U;
    g_fan_program.elapsed_ms = 0U;
    g_fan_program.stop_detect_tick = 0U;
    g_fan_program.alarm_over_limit_tick = 0U;
    g_fan_program.in_band_tick = 0U;
    g_fan_program.alarm_armed = 0U;
    g_fan_program.last_segment_index = 0U;
    FanProgram_SetAlarm(0U);
}

void FanProgram_Process(uint32_t now_tick, uint16_t current_rpm)
{
    int16_t duty_command;
    int16_t duty_delta;
    int32_t rpm_error;
    uint32_t rpm_error_abs;
    uint32_t segment_elapsed_ms;
    uint8_t current_segment_index;
    uint32_t arm_min_segment_ms;
    uint32_t in_band_threshold;

    if (g_fan_program.active == 0U)
    {
        return;
    }

    if (g_fan_program.run_state == FAN_PROGRAM_STATE_BRAKE_TO_STOP)
    {
        g_fan_program.target_rpm = 0U;
        MotorPwm_SetDutyPercent(0U);

        if (current_rpm <= FAN_PROGRAM_STOP_RPM_THRESHOLD)
        {
            if (g_fan_program.stop_detect_tick == 0U)
            {
                g_fan_program.stop_detect_tick = now_tick;
            }
            else if ((now_tick - g_fan_program.stop_detect_tick) >= FAN_PROGRAM_STOP_HOLD_MS)
            {
                g_fan_program.run_state = FAN_PROGRAM_STATE_RUNNING;
                g_fan_program.start_tick = now_tick;
                g_fan_program.last_control_tick = 0U;
                g_fan_program.elapsed_ms = 0U;
                g_fan_program.stop_detect_tick = 0U;
                g_fan_program.alarm_over_limit_tick = 0U;
                g_fan_program.in_band_tick = 0U;
                g_fan_program.alarm_armed = 0U;
                g_fan_program.last_segment_index = 0U;
            }
        }
        else
        {
            g_fan_program.stop_detect_tick = 0U;
        }

        return;
    }

    if (g_fan_program.run_state == FAN_PROGRAM_STATE_COAST_TO_STOP)
    {
        g_fan_program.target_rpm = 0U;
        g_fan_program.duty_percent = MotorPwm_GetDutyPercent();

        if (g_fan_program.duty_percent > FAN_PROGRAM_MAX_DUTY_CHANGE_PER_STEP)
        {
            g_fan_program.duty_percent = (uint8_t)(g_fan_program.duty_percent - FAN_PROGRAM_MAX_DUTY_CHANGE_PER_STEP);
        }
        else
        {
            g_fan_program.duty_percent = 0U;
        }

        MotorPwm_SetDutyPercent(g_fan_program.duty_percent);
        FanProgram_SetAlarm(0U);

        if (current_rpm <= FAN_PROGRAM_STOP_RPM_THRESHOLD)
        {
            g_fan_program.active = 0U;
            g_fan_program.run_state = FAN_PROGRAM_STATE_IDLE;
            g_fan_program.elapsed_ms = FAN_PROGRAM_TOTAL_TIME_MS;
        }

        return;
    }

    g_fan_program.elapsed_ms = now_tick - g_fan_program.start_tick;
    if (g_fan_program.elapsed_ms >= FAN_PROGRAM_TOTAL_TIME_MS)
    {
        g_fan_program.target_rpm = FAN_PROGRAM_MAX_TARGET_RPM;
        g_fan_program.run_state = FAN_PROGRAM_STATE_COAST_TO_STOP;
        FanProgram_SetAlarm(0U);
        return;
    }

    g_fan_program.target_rpm = FanProgram_GetTargetByElapsedMs(g_fan_program.elapsed_ms);
    if ((now_tick - g_fan_program.last_control_tick) < FAN_PROGRAM_CONTROL_PERIOD_MS)
    {
        return;
    }

    g_fan_program.last_control_tick = now_tick;
    g_fan_program.duty_percent = MotorPwm_GetDutyPercent();
    rpm_error = (int32_t)g_fan_program.target_rpm - (int32_t)current_rpm;
    rpm_error_abs = (rpm_error >= 0) ? (uint32_t)rpm_error : (uint32_t)(-rpm_error);
    current_segment_index = FanProgram_GetSegmentIndexByElapsedMs(g_fan_program.elapsed_ms);
    arm_min_segment_ms = FanProgram_GetArmMinSegmentMs(current_segment_index);
    in_band_threshold = FanProgram_GetInBandThreshold(current_segment_index);

    if (current_segment_index != g_fan_program.last_segment_index)
    {
        g_fan_program.last_segment_index = current_segment_index;
        g_fan_program.alarm_over_limit_tick = 0U;
        g_fan_program.in_band_tick = 0U;
        g_fan_program.alarm_armed = 0U;
        FanProgram_SetAlarm(0U);
    }

    segment_elapsed_ms = g_fan_program.elapsed_ms % 10000U;
    if ((segment_elapsed_ms >= arm_min_segment_ms) &&
        (rpm_error_abs <= in_band_threshold))
    {
        if (g_fan_program.in_band_tick == 0U)
        {
            g_fan_program.in_band_tick = now_tick;
        }
        else if ((now_tick - g_fan_program.in_band_tick) >= FAN_PROGRAM_ARM_HOLD_MS)
        {
            g_fan_program.alarm_armed = 1U;
        }
    }
    else
    {
        g_fan_program.in_band_tick = 0U;
    }

    if (g_fan_program.alarm_armed == 0U)
    {
        g_fan_program.alarm_over_limit_tick = 0U;
        FanProgram_SetAlarm(0U);
    }
    else if (rpm_error_abs > FAN_PROGRAM_RPM_ALARM_THRESHOLD)
    {
        g_fan_program.alarm_over_limit_tick = now_tick;
        FanProgram_SetAlarm(1U);
    }
    else if (rpm_error_abs <= FAN_PROGRAM_ALARM_CLEAR_THRESHOLD)
    {
        g_fan_program.alarm_over_limit_tick = 0U;
        FanProgram_SetAlarm(0U);
    }

    duty_command = (int16_t)((g_fan_program.target_rpm + 5U) / 10U);
    duty_command += (int16_t)(rpm_error / 8);

    if (g_fan_program.target_rpm >= FAN_PROGRAM_HIGH_SPEED_RPM_THRESHOLD)
    {
        duty_command += FAN_PROGRAM_HIGH_SPEED_EXTRA_DUTY;
    }

    if ((g_fan_program.target_rpm >= FAN_PROGRAM_HIGH_SPEED_RPM_THRESHOLD) &&
        (current_rpm >= FAN_PROGRAM_NEAR_TOP_RPM_THRESHOLD))
    {
        duty_command -= FAN_PROGRAM_NEAR_TOP_BRAKE_DUTY;
    }

    if (g_fan_program.elapsed_ms < FAN_PROGRAM_STARTUP_BOOST_MS)
    {
        if (duty_command < (int16_t)FAN_PROGRAM_STARTUP_DUTY_PERCENT)
        {
            duty_command = FAN_PROGRAM_STARTUP_DUTY_PERCENT;
        }
    }
    else if (g_fan_program.target_rpm >= FAN_PROGRAM_HIGH_SPEED_RPM_THRESHOLD)
    {
        if (duty_command < (int16_t)FAN_PROGRAM_HIGH_SPEED_DUTY_MIN)
        {
            duty_command = FAN_PROGRAM_HIGH_SPEED_DUTY_MIN;
        }
    }
    else if ((g_fan_program.target_rpm > 0U) &&
             (duty_command < (int16_t)FAN_PROGRAM_MIN_RUNNING_DUTY_PERCENT))
    {
        duty_command = FAN_PROGRAM_MIN_RUNNING_DUTY_PERCENT;
    }

    duty_delta = duty_command - (int16_t)g_fan_program.duty_percent;
    if (duty_delta > FAN_PROGRAM_MAX_DUTY_CHANGE_PER_STEP)
    {
        duty_command = (int16_t)g_fan_program.duty_percent + FAN_PROGRAM_MAX_DUTY_CHANGE_PER_STEP;
    }
    else if (duty_delta < -FAN_PROGRAM_MAX_DUTY_CHANGE_PER_STEP)
    {
        duty_command = (int16_t)g_fan_program.duty_percent - FAN_PROGRAM_MAX_DUTY_CHANGE_PER_STEP;
    }

    g_fan_program.duty_percent = FanProgram_ClampDuty(duty_command);
    MotorPwm_SetDutyPercent(g_fan_program.duty_percent);
}

uint8_t FanProgram_IsActive(void)
{
    return g_fan_program.active;
}

uint8_t FanProgram_IsAlarmActive(void)
{
    return g_fan_program.alarm_active;
}

uint16_t FanProgram_GetTargetRpm(void)
{
    return g_fan_program.target_rpm;
}

uint32_t FanProgram_GetElapsedMs(void)
{
    return g_fan_program.elapsed_ms;
}

uint8_t FanProgram_GetSegmentIndex(void)
{
    if (g_fan_program.run_state == FAN_PROGRAM_STATE_RUNNING)
    {
        return (uint8_t)(FanProgram_GetSegmentIndexByElapsedMs(g_fan_program.elapsed_ms) + 1U);
    }

    if (g_fan_program.run_state == FAN_PROGRAM_STATE_COAST_TO_STOP)
    {
        return 6U;
    }

    return 0U;
}

const char *FanProgram_GetStateText(void)
{
    switch ((FanProgramRunState_t)g_fan_program.run_state)
    {
        case FAN_PROGRAM_STATE_BRAKE_TO_STOP:
            return "WAIT";

        case FAN_PROGRAM_STATE_RUNNING:
            return "RUN";

        case FAN_PROGRAM_STATE_COAST_TO_STOP:
            return "DOWN";

        default:
            return "STOP";
    }
}

void FanProgram_SetTargets(const uint16_t targets[6])
{
    uint8_t i;
    if (targets == NULL) { return; }
    for (i = 0U; i < 6U; i++)
    {
        if (targets[i] > FAN_PROGRAM_MAX_TARGET_RPM)
        {
            g_fan_custom_targets[i] = FAN_PROGRAM_MAX_TARGET_RPM;
        }
        else
        {
            g_fan_custom_targets[i] = targets[i];
        }
    }
    g_fan_use_custom_targets = 1U;
}

void FanProgram_GetTargets(uint16_t targets[6])
{
    uint8_t i;
    if (targets == NULL) { return; }
    for (i = 0U; i < 6U; i++)
    {
        targets[i] = g_fan_use_custom_targets ? g_fan_custom_targets[i] : 0U;
    }
}
