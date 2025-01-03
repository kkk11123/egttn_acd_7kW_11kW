################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (9-2020-q2-update)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Core/_app/Src/_app_charging_service.c \
../Core/_app/Src/_app_configration_service.c \
../Core/_app/Src/_app_metering_service.c \
../Core/_app/Src/_app_nonos_loop.c \
../Core/_app/Src/_app_rfid_comm.c \
../Core/_app/Src/_app_system_control.c 

OBJS += \
./Core/_app/Src/_app_charging_service.o \
./Core/_app/Src/_app_configration_service.o \
./Core/_app/Src/_app_metering_service.o \
./Core/_app/Src/_app_nonos_loop.o \
./Core/_app/Src/_app_rfid_comm.o \
./Core/_app/Src/_app_system_control.o 

C_DEPS += \
./Core/_app/Src/_app_charging_service.d \
./Core/_app/Src/_app_configration_service.d \
./Core/_app/Src/_app_metering_service.d \
./Core/_app/Src/_app_nonos_loop.d \
./Core/_app/Src/_app_rfid_comm.d \
./Core/_app/Src/_app_system_control.d 


# Each subdirectory must supply rules for building sources it contributes
Core/_app/Src/_app_charging_service.o: ../Core/_app/Src/_app_charging_service.c Core/_app/Src/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m0 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32F030x8 -c -I../Core/Inc -I../Drivers/STM32F0xx_HAL_Driver/Inc -I../Drivers/STM32F0xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32F0xx/Include -I../Drivers/CMSIS/Include -I../Core/_lib/Inc -I../Core/_mw/Inc -I../Core/_app/Inc -O2 -ffunction-sections -fdata-sections -Wall -fstack-usage -MMD -MP -MF"Core/_app/Src/_app_charging_service.d" -MT"$@" --specs=nano.specs -mfloat-abi=soft -mthumb -o "$@"
Core/_app/Src/_app_configration_service.o: ../Core/_app/Src/_app_configration_service.c Core/_app/Src/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m0 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32F030x8 -c -I../Core/Inc -I../Drivers/STM32F0xx_HAL_Driver/Inc -I../Drivers/STM32F0xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32F0xx/Include -I../Drivers/CMSIS/Include -I../Core/_lib/Inc -I../Core/_mw/Inc -I../Core/_app/Inc -O2 -ffunction-sections -fdata-sections -Wall -fstack-usage -MMD -MP -MF"Core/_app/Src/_app_configration_service.d" -MT"$@" --specs=nano.specs -mfloat-abi=soft -mthumb -o "$@"
Core/_app/Src/_app_metering_service.o: ../Core/_app/Src/_app_metering_service.c Core/_app/Src/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m0 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32F030x8 -c -I../Core/Inc -I../Drivers/STM32F0xx_HAL_Driver/Inc -I../Drivers/STM32F0xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32F0xx/Include -I../Drivers/CMSIS/Include -I../Core/_lib/Inc -I../Core/_mw/Inc -I../Core/_app/Inc -O2 -ffunction-sections -fdata-sections -Wall -fstack-usage -MMD -MP -MF"Core/_app/Src/_app_metering_service.d" -MT"$@" --specs=nano.specs -mfloat-abi=soft -mthumb -o "$@"
Core/_app/Src/_app_nonos_loop.o: ../Core/_app/Src/_app_nonos_loop.c Core/_app/Src/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m0 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32F030x8 -c -I../Core/Inc -I../Drivers/STM32F0xx_HAL_Driver/Inc -I../Drivers/STM32F0xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32F0xx/Include -I../Drivers/CMSIS/Include -I../Core/_lib/Inc -I../Core/_mw/Inc -I../Core/_app/Inc -O2 -ffunction-sections -fdata-sections -Wall -fstack-usage -MMD -MP -MF"Core/_app/Src/_app_nonos_loop.d" -MT"$@" --specs=nano.specs -mfloat-abi=soft -mthumb -o "$@"
Core/_app/Src/_app_rfid_comm.o: ../Core/_app/Src/_app_rfid_comm.c Core/_app/Src/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m0 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32F030x8 -c -I../Core/Inc -I../Drivers/STM32F0xx_HAL_Driver/Inc -I../Drivers/STM32F0xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32F0xx/Include -I../Drivers/CMSIS/Include -I../Core/_lib/Inc -I../Core/_mw/Inc -I../Core/_app/Inc -O2 -ffunction-sections -fdata-sections -Wall -fstack-usage -MMD -MP -MF"Core/_app/Src/_app_rfid_comm.d" -MT"$@" --specs=nano.specs -mfloat-abi=soft -mthumb -o "$@"
Core/_app/Src/_app_system_control.o: ../Core/_app/Src/_app_system_control.c Core/_app/Src/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m0 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32F030x8 -c -I../Core/Inc -I../Drivers/STM32F0xx_HAL_Driver/Inc -I../Drivers/STM32F0xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32F0xx/Include -I../Drivers/CMSIS/Include -I../Core/_lib/Inc -I../Core/_mw/Inc -I../Core/_app/Inc -O2 -ffunction-sections -fdata-sections -Wall -fstack-usage -MMD -MP -MF"Core/_app/Src/_app_system_control.d" -MT"$@" --specs=nano.specs -mfloat-abi=soft -mthumb -o "$@"

