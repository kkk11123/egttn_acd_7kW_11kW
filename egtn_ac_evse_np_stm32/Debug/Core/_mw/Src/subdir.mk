################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (9-2020-q2-update)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Core/_mw/Src/_mw_cp.c \
../Core/_mw/Src/_mw_eeprom.c \
../Core/_mw/Src/_mw_gpio.c \
../Core/_mw/Src/_mw_indiled.c \
../Core/_mw/Src/_mw_it.c \
../Core/_mw/Src/_mw_ntc.c \
../Core/_mw/Src/_mw_pwm.c \
../Core/_mw/Src/_mw_uart.c 

OBJS += \
./Core/_mw/Src/_mw_cp.o \
./Core/_mw/Src/_mw_eeprom.o \
./Core/_mw/Src/_mw_gpio.o \
./Core/_mw/Src/_mw_indiled.o \
./Core/_mw/Src/_mw_it.o \
./Core/_mw/Src/_mw_ntc.o \
./Core/_mw/Src/_mw_pwm.o \
./Core/_mw/Src/_mw_uart.o 

C_DEPS += \
./Core/_mw/Src/_mw_cp.d \
./Core/_mw/Src/_mw_eeprom.d \
./Core/_mw/Src/_mw_gpio.d \
./Core/_mw/Src/_mw_indiled.d \
./Core/_mw/Src/_mw_it.d \
./Core/_mw/Src/_mw_ntc.d \
./Core/_mw/Src/_mw_pwm.d \
./Core/_mw/Src/_mw_uart.d 


# Each subdirectory must supply rules for building sources it contributes
Core/_mw/Src/_mw_cp.o: ../Core/_mw/Src/_mw_cp.c Core/_mw/Src/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m0 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32F030x8 -c -I../Core/Inc -I../Drivers/STM32F0xx_HAL_Driver/Inc -I../Drivers/STM32F0xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32F0xx/Include -I../Drivers/CMSIS/Include -I../Core/_lib/Inc -I../Core/_mw/Inc -I../Core/_app/Inc -O2 -ffunction-sections -fdata-sections -Wall -fstack-usage -MMD -MP -MF"Core/_mw/Src/_mw_cp.d" -MT"$@" --specs=nano.specs -mfloat-abi=soft -mthumb -o "$@"
Core/_mw/Src/_mw_eeprom.o: ../Core/_mw/Src/_mw_eeprom.c Core/_mw/Src/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m0 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32F030x8 -c -I../Core/Inc -I../Drivers/STM32F0xx_HAL_Driver/Inc -I../Drivers/STM32F0xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32F0xx/Include -I../Drivers/CMSIS/Include -I../Core/_lib/Inc -I../Core/_mw/Inc -I../Core/_app/Inc -O2 -ffunction-sections -fdata-sections -Wall -fstack-usage -MMD -MP -MF"Core/_mw/Src/_mw_eeprom.d" -MT"$@" --specs=nano.specs -mfloat-abi=soft -mthumb -o "$@"
Core/_mw/Src/_mw_gpio.o: ../Core/_mw/Src/_mw_gpio.c Core/_mw/Src/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m0 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32F030x8 -c -I../Core/Inc -I../Drivers/STM32F0xx_HAL_Driver/Inc -I../Drivers/STM32F0xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32F0xx/Include -I../Drivers/CMSIS/Include -I../Core/_lib/Inc -I../Core/_mw/Inc -I../Core/_app/Inc -O2 -ffunction-sections -fdata-sections -Wall -fstack-usage -MMD -MP -MF"Core/_mw/Src/_mw_gpio.d" -MT"$@" --specs=nano.specs -mfloat-abi=soft -mthumb -o "$@"
Core/_mw/Src/_mw_indiled.o: ../Core/_mw/Src/_mw_indiled.c Core/_mw/Src/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m0 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32F030x8 -c -I../Core/Inc -I../Drivers/STM32F0xx_HAL_Driver/Inc -I../Drivers/STM32F0xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32F0xx/Include -I../Drivers/CMSIS/Include -I../Core/_lib/Inc -I../Core/_mw/Inc -I../Core/_app/Inc -O2 -ffunction-sections -fdata-sections -Wall -fstack-usage -MMD -MP -MF"Core/_mw/Src/_mw_indiled.d" -MT"$@" --specs=nano.specs -mfloat-abi=soft -mthumb -o "$@"
Core/_mw/Src/_mw_it.o: ../Core/_mw/Src/_mw_it.c Core/_mw/Src/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m0 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32F030x8 -c -I../Core/Inc -I../Drivers/STM32F0xx_HAL_Driver/Inc -I../Drivers/STM32F0xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32F0xx/Include -I../Drivers/CMSIS/Include -I../Core/_lib/Inc -I../Core/_mw/Inc -I../Core/_app/Inc -O2 -ffunction-sections -fdata-sections -Wall -fstack-usage -MMD -MP -MF"Core/_mw/Src/_mw_it.d" -MT"$@" --specs=nano.specs -mfloat-abi=soft -mthumb -o "$@"
Core/_mw/Src/_mw_ntc.o: ../Core/_mw/Src/_mw_ntc.c Core/_mw/Src/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m0 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32F030x8 -c -I../Core/Inc -I../Drivers/STM32F0xx_HAL_Driver/Inc -I../Drivers/STM32F0xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32F0xx/Include -I../Drivers/CMSIS/Include -I../Core/_lib/Inc -I../Core/_mw/Inc -I../Core/_app/Inc -O2 -ffunction-sections -fdata-sections -Wall -fstack-usage -MMD -MP -MF"Core/_mw/Src/_mw_ntc.d" -MT"$@" --specs=nano.specs -mfloat-abi=soft -mthumb -o "$@"
Core/_mw/Src/_mw_pwm.o: ../Core/_mw/Src/_mw_pwm.c Core/_mw/Src/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m0 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32F030x8 -c -I../Core/Inc -I../Drivers/STM32F0xx_HAL_Driver/Inc -I../Drivers/STM32F0xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32F0xx/Include -I../Drivers/CMSIS/Include -I../Core/_lib/Inc -I../Core/_mw/Inc -I../Core/_app/Inc -O2 -ffunction-sections -fdata-sections -Wall -fstack-usage -MMD -MP -MF"Core/_mw/Src/_mw_pwm.d" -MT"$@" --specs=nano.specs -mfloat-abi=soft -mthumb -o "$@"
Core/_mw/Src/_mw_uart.o: ../Core/_mw/Src/_mw_uart.c Core/_mw/Src/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m0 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32F030x8 -c -I../Core/Inc -I../Drivers/STM32F0xx_HAL_Driver/Inc -I../Drivers/STM32F0xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32F0xx/Include -I../Drivers/CMSIS/Include -I../Core/_lib/Inc -I../Core/_mw/Inc -I../Core/_app/Inc -O2 -ffunction-sections -fdata-sections -Wall -fstack-usage -MMD -MP -MF"Core/_mw/Src/_mw_uart.d" -MT"$@" --specs=nano.specs -mfloat-abi=soft -mthumb -o "$@"

