cmake_minimum_required(VERSION 3.22)
# Enable CMake support for ASM and C languages
enable_language(C ASM)
# STM32CubeMX generated symbols (macros)
set(MX_Defines_Syms 
	USE_HAL_DRIVER 
	STM32F407xx
    $<$<CONFIG:Debug>:DEBUG>
)

# STM32CubeMX generated include paths
set(MX_Include_Dirs
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Core/Inc
    C:/32CubeMax/Update/STM32Cube_FW_F4_V1.28.3/Drivers/STM32F4xx_HAL_Driver/Inc
    C:/32CubeMax/Update/STM32Cube_FW_F4_V1.28.3/Drivers/STM32F4xx_HAL_Driver/Inc/Legacy
    C:/32CubeMax/Update/STM32Cube_FW_F4_V1.28.3/Middlewares/Third_Party/FreeRTOS/Source/include
    C:/32CubeMax/Update/STM32Cube_FW_F4_V1.28.3/Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS_V2
    C:/32CubeMax/Update/STM32Cube_FW_F4_V1.28.3/Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM4F
    C:/32CubeMax/Update/STM32Cube_FW_F4_V1.28.3/Drivers/CMSIS/Device/ST/STM32F4xx/Include
    C:/32CubeMax/Update/STM32Cube_FW_F4_V1.28.3/Drivers/CMSIS/Include
)

# STM32CubeMX generated application sources
set(MX_Application_Src
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Core/Src/main.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Core/Src/gpio.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Core/Src/freertos.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Core/Src/adc.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Core/Src/i2c.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Core/Src/tim.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Core/Src/usart.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Core/Src/stm32f4xx_it.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Core/Src/stm32f4xx_hal_msp.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Core/Src/stm32f4xx_hal_timebase_tim.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Core/Src/sysmem.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Core/Src/syscalls.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Core/Src/alarm_led.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Core/Src/auto_light.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Core/Src/bluetooth.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Core/Src/button.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Core/Src/buzzer.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Core/Src/curtain.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Core/Src/dht11.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Core/Src/fan_program.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Core/Src/flow_sensor.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Core/Src/gate_access.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Core/Src/irrigation.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Core/Src/light_pwm.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Core/Src/light_sensor.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Core/Src/motor_pwm.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Core/Src/motor_speed.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Core/Src/oled.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Core/Src/oled_data.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Core/Src/output_fault.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Core/Src/pump.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Core/Src/servo.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Core/Src/temperature_sensor.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Core/Src/ultrasonic.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Core/Src/water_control.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Core/Src/water_level_alarm.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../startup_stm32f407xx.s
)

# STM32 HAL/LL Drivers
set(STM32_Drivers_Src
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Core/Src/system_stm32f4xx.c
    C:/32CubeMax/Update/STM32Cube_FW_F4_V1.28.3/Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_tim.c
    C:/32CubeMax/Update/STM32Cube_FW_F4_V1.28.3/Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_tim_ex.c
    C:/32CubeMax/Update/STM32Cube_FW_F4_V1.28.3/Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_adc.c
    C:/32CubeMax/Update/STM32Cube_FW_F4_V1.28.3/Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_adc_ex.c
    C:/32CubeMax/Update/STM32Cube_FW_F4_V1.28.3/Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_ll_adc.c
    C:/32CubeMax/Update/STM32Cube_FW_F4_V1.28.3/Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_rcc.c
    C:/32CubeMax/Update/STM32Cube_FW_F4_V1.28.3/Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_rcc_ex.c
    C:/32CubeMax/Update/STM32Cube_FW_F4_V1.28.3/Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_flash.c
    C:/32CubeMax/Update/STM32Cube_FW_F4_V1.28.3/Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_flash_ex.c
    C:/32CubeMax/Update/STM32Cube_FW_F4_V1.28.3/Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_flash_ramfunc.c
    C:/32CubeMax/Update/STM32Cube_FW_F4_V1.28.3/Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_gpio.c
    C:/32CubeMax/Update/STM32Cube_FW_F4_V1.28.3/Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_dma_ex.c
    C:/32CubeMax/Update/STM32Cube_FW_F4_V1.28.3/Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_dma.c
    C:/32CubeMax/Update/STM32Cube_FW_F4_V1.28.3/Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_pwr.c
    C:/32CubeMax/Update/STM32Cube_FW_F4_V1.28.3/Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_pwr_ex.c
    C:/32CubeMax/Update/STM32Cube_FW_F4_V1.28.3/Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_cortex.c
    C:/32CubeMax/Update/STM32Cube_FW_F4_V1.28.3/Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal.c
    C:/32CubeMax/Update/STM32Cube_FW_F4_V1.28.3/Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_exti.c
    C:/32CubeMax/Update/STM32Cube_FW_F4_V1.28.3/Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_i2c.c
    C:/32CubeMax/Update/STM32Cube_FW_F4_V1.28.3/Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_i2c_ex.c
    C:/32CubeMax/Update/STM32Cube_FW_F4_V1.28.3/Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_uart.c
)

# Drivers Midllewares


set(FreeRTOS_Src
    C:/32CubeMax/Update/STM32Cube_FW_F4_V1.28.3/Middlewares/Third_Party/FreeRTOS/Source/croutine.c
    C:/32CubeMax/Update/STM32Cube_FW_F4_V1.28.3/Middlewares/Third_Party/FreeRTOS/Source/event_groups.c
    C:/32CubeMax/Update/STM32Cube_FW_F4_V1.28.3/Middlewares/Third_Party/FreeRTOS/Source/list.c
    C:/32CubeMax/Update/STM32Cube_FW_F4_V1.28.3/Middlewares/Third_Party/FreeRTOS/Source/queue.c
    C:/32CubeMax/Update/STM32Cube_FW_F4_V1.28.3/Middlewares/Third_Party/FreeRTOS/Source/stream_buffer.c
    C:/32CubeMax/Update/STM32Cube_FW_F4_V1.28.3/Middlewares/Third_Party/FreeRTOS/Source/tasks.c
    C:/32CubeMax/Update/STM32Cube_FW_F4_V1.28.3/Middlewares/Third_Party/FreeRTOS/Source/timers.c
    C:/32CubeMax/Update/STM32Cube_FW_F4_V1.28.3/Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS_V2/cmsis_os2.c
    C:/32CubeMax/Update/STM32Cube_FW_F4_V1.28.3/Middlewares/Third_Party/FreeRTOS/Source/portable/MemMang/heap_4.c
    C:/32CubeMax/Update/STM32Cube_FW_F4_V1.28.3/Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM4F/port.c
)

# Link directories setup
set(MX_LINK_DIRS

)
# Project static libraries
set(MX_LINK_LIBS 
    STM32_Drivers
    ${TOOLCHAIN_LINK_LIBRARIES}
    FreeRTOS	
)
# Interface library for includes and symbols
add_library(stm32cubemx INTERFACE)
target_include_directories(stm32cubemx INTERFACE ${MX_Include_Dirs})
target_compile_definitions(stm32cubemx INTERFACE ${MX_Defines_Syms})

# Create STM32_Drivers static library
add_library(STM32_Drivers OBJECT)
target_sources(STM32_Drivers PRIVATE ${STM32_Drivers_Src})
target_link_libraries(STM32_Drivers PUBLIC stm32cubemx)


# Create FreeRTOS static library
add_library(FreeRTOS OBJECT)
target_sources(FreeRTOS PRIVATE ${FreeRTOS_Src})
target_link_libraries(FreeRTOS PUBLIC stm32cubemx)

# Add STM32CubeMX generated application sources to the project
target_sources(${CMAKE_PROJECT_NAME} PRIVATE ${MX_Application_Src})

# Link directories setup
target_link_directories(${CMAKE_PROJECT_NAME} PRIVATE ${MX_LINK_DIRS})

# Add libraries to the project
target_link_libraries(${CMAKE_PROJECT_NAME} ${MX_LINK_LIBS})

# Add the map file to the list of files to be removed with 'clean' target
set_target_properties(${CMAKE_PROJECT_NAME} PROPERTIES ADDITIONAL_CLEAN_FILES ${CMAKE_PROJECT_NAME}.map)

# Validate that STM32CubeMX code is compatible with C standard
if((CMAKE_C_STANDARD EQUAL 90) OR (CMAKE_C_STANDARD EQUAL 99))
    message(ERROR "Generated code requires C11 or higher")
endif()
