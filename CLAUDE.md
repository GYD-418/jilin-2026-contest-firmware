# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

**celiang** (测量) — Smart greenhouse / environmental monitoring and control firmware for STM32F407VET6 (ARM Cortex-M4F, 168 MHz). Built with STM32 HAL + FreeRTOS V10.3.1 (CMSIS-RTOS V2 API). All source is C11; comments and in-code documentation are in Chinese.

The system manages sensors (DHT11 humidity, NTC temperature, light ADC, ultrasonic water-level, flow meter, fan RPM encoder), actuators (8-ch PWM LED lighting with 10-level bar, PID-controlled fan, water pump relay, proportional irrigation valve, servo/gate, roller curtain, buzzer, alarm LED), and communicates over USART1 (HC-08 Bluetooth @ 9600 bps), USART2 (serial display screen @ 256000 bps), and I2C1 (OLED).

## Build Commands

Requires `arm-none-eabi-gcc` in PATH and Ninja installed.

```bash
# Debug
cmake --preset Debug
cmake --build --preset Debug

# Release
cmake --preset Release
cmake --build --preset Release
```

Build output goes to `build/Debug/` or `build/Release/`; the final artifact is `celiang.elf`.

There are no unit tests or linting tools configured. Testing is done on hardware.

## External Dependencies

HAL and CMSIS drivers are **not vendored** into the repo. They are referenced via absolute path to a local STM32CubeMX firmware package:

```
C:/32CubeMax/Update/STM32Cube_FW_F4_V1.28.3/
```

This path is hardcoded in `celiang.ioc` (`LibraryCopy=2` mode). If building on a different machine, this path must be updated in the `.ioc` file or the firmware package must be installed at that exact location.

## Architecture

### Entry point and startup

- `startup_stm32f407xx.s` — ARM vector table and Reset_Handler
- `Core/Src/main.c` — HAL init, clock config (168 MHz), peripheral init, then `osKernelStart()`
- `Core/Src/freertos.c` — **central application file** (~38 KB). All FreeRTOS task bodies live here. Also contains the complete Chinese-language project documentation, all 38 UART command definitions, and serial display protocol logic.

### FreeRTOS tasks (defined in `.ioc`, implemented in `freertos.c`)

| Task | Priority | Stack | Role |
|------|----------|-------|------|
| `defaultTask` | Normal (24) | 256 words | Sensor reads, actuator state machines, OLED refresh, serial screen data, Bluetooth push |
| `displayTask` | BelowNormal (16) | 256 words | Button polling, OLED menu state machine (6 sub-pages) |
| `buzzerTask` | Low (8) | 128 words | 1 ms GPIO toggle for ~1 kHz buzzer tone |

### Module organization (`Core/Src/` and `Core/Inc/`)

All application source and headers live flat in `Core/Src/` and `Core/Inc/` (no subdirectories). Modules follow a `name.c` / `name.h` pairing:

- **Sensors:** `dht11`, `ultrasonic`, `light_sensor`, `temperature_sensor`, `flow_sensor`, `motor_speed`
- **Actuators:** `motor_pwm` (fan PWM), `fan_program` (PID closed-loop), `light_pwm` (8-ch LED), `pump`, `irrigation`, `curtain`, `servo`, `buzzer`, `alarm_led`
- **Control logic:** `auto_light`, `water_control`, `water_level_alarm`, `gate_access`, `output_fault`, `button`
- **Communication:** `bluetooth`, `usart` (3 UARTs + command parser), `oled`, `oled_data`
- **HAL/RTOS init:** `main`, `freertos`, `stm32f4xx_it` (interrupt handlers), `stm32f4xx_hal_msp`

### UART command protocol

38 commands sent over USART2, each suffixed with `"KK"`. Parsing and dispatch is in `usart.c` / `usart.h`. The full command table and serial display data format (5-phase round-robin with 26 data commands) are documented in the header comments of `freertos.c` (lines 55–271).

### OLED menu system

6 sub-pages navigated with 4 GPIO buttons (debounced in `button.c`). Menu state machine runs in `displayTask`. Display driver and font data in `oled.c` / `oled_data.c`.

## Key Pin Mapping (see `Core/Inc/main.h` for full list)

| Function | Pin | Peripheral |
|----------|-----|------------|
| DHT11 data | PB12 | GPIO bit-bang |
| Ultrasonic TRIG | PB15 | GPIO |
| Ultrasonic ECHO | PB8 | TIM4 input capture |
| Light sensor | PA0 | ADC1_IN0 |
| NTC thermistor | PA1 | ADC1_IN1 |
| Flow sensor | PC0 | EXTI interrupt |
| Fan RPM | PA8 | EXTI interrupt |
| Fan PWM | PA15 | TIM2_CH1 |
| Servo | PA6 | TIM3_CH1 |
| Irrigation valve | PE5 | TIM1_CH4 + PE4 GPIO switch |
| Curtain | PB1/PB2 | GPIO H-bridge (fwd/rev) |
| Buzzer | PB13 | GPIO flip-drive |
| Alarm LED | PB14 | GPIO |
| OLED SCL/SDA | PB6/PB7 | I2C1 |
| Bluetooth TX/RX | PA9/PA10 | USART1 |
| Display TX/RX | PA2/PA3 | USART2 |

## Important Notes

- `Core/Src/freertos.c` is the single source of truth for application logic, serial protocol, and inline documentation. Start here when understanding system behavior.
- The STM32CubeMX `.ioc` file controls pin/peripheral configuration. Regenerating from CubeMX will overwrite `Core/Src/main.c`, `Core/Src/stm32f4xx_hal_msp.c`, `Core/Src/stm32f4xx_it.c`, and the `cmake/stm32cubemx/CMakeLists.txt`. Custom application code in other `Core/Src/*.c` files is safe from regeneration.
- The linker script `STM32F407XX_FLASH.ld` defines memory layout: 512K Flash, 128K RAM, 64K CCMRAM.
