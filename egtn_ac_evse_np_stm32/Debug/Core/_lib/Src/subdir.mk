################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (9-2020-q2-update)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Core/_lib/Src/_lib_LPF.c \
../Core/_lib/Src/_lib_bitop.c \
../Core/_lib/Src/_lib_debouncecheck.c \
../Core/_lib/Src/_lib_logging.c \
../Core/_lib/Src/_lib_u8queue.c \
../Core/_lib/Src/_lib_userdelay.c 

OBJS += \
./Core/_lib/Src/_lib_LPF.o \
./Core/_lib/Src/_lib_bitop.o \
./Core/_lib/Src/_lib_debouncecheck.o \
./Core/_lib/Src/_lib_logging.o \
./Core/_lib/Src/_lib_u8queue.o \
./Core/_lib/Src/_lib_userdelay.o 

C_DEPS += \
./Core/_lib/Src/_lib_LPF.d \
./Core/_lib/Src/_lib_bitop.d \
./Core/_lib/Src/_lib_debouncecheck.d \
./Core/_lib/Src/_lib_logging.d \
./Core/_lib/Src/_lib_u8queue.d \
./Core/_lib/Src/_lib_userdelay.d 


# Each subdirectory must supply rules for building sources it contributes
Core/_lib/Src/_lib_LPF.o: ../Core/_lib/Src/_lib_LPF.c Core/_lib/Src/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m0 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32F030x8 -c -I../Core/Inc -I../Drivers/STM32F0xx_HAL_Driver/Inc -I../Drivers/STM32F0xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32F0xx/Include -I../Drivers/CMSIS/Include -I../Core/_lib/Inc -I../Core/_mw/Inc -I../Core/_app/Inc -O2 -ffunction-sections -fdata-sections -Wall -fstack-usage -MMD -MP -MF"Core/_lib/Src/_lib_LPF.d" -MT"$@" --specs=nano.specs -mfloat-abi=soft -mthumb -o "$@"
Core/_lib/Src/_lib_bitop.o: ../Core/_lib/Src/_lib_bitop.c Core/_lib/Src/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m0 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32F030x8 -c -I../Core/Inc -I../Drivers/STM32F0xx_HAL_Driver/Inc -I../Drivers/STM32F0xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32F0xx/Include -I../Drivers/CMSIS/Include -I../Core/_lib/Inc -I../Core/_mw/Inc -I../Core/_app/Inc -O2 -ffunction-sections -fdata-sections -Wall -fstack-usage -MMD -MP -MF"Core/_lib/Src/_lib_bitop.d" -MT"$@" --specs=nano.specs -mfloat-abi=soft -mthumb -o "$@"
Core/_lib/Src/_lib_debouncecheck.o: ../Core/_lib/Src/_lib_debouncecheck.c Core/_lib/Src/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m0 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32F030x8 -c -I../Core/Inc -I../Drivers/STM32F0xx_HAL_Driver/Inc -I../Drivers/STM32F0xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32F0xx/Include -I../Drivers/CMSIS/Include -I../Core/_lib/Inc -I../Core/_mw/Inc -I../Core/_app/Inc -O2 -ffunction-sections -fdata-sections -Wall -fstack-usage -MMD -MP -MF"Core/_lib/Src/_lib_debouncecheck.d" -MT"$@" --specs=nano.specs -mfloat-abi=soft -mthumb -o "$@"
Core/_lib/Src/_lib_logging.o: ../Core/_lib/Src/_lib_logging.c Core/_lib/Src/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m0 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32F030x8 -c -I../Core/Inc -I../Drivers/STM32F0xx_HAL_Driver/Inc -I../Drivers/STM32F0xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32F0xx/Include -I../Drivers/CMSIS/Include -I../Core/_lib/Inc -I../Core/_mw/Inc -I../Core/_app/Inc -O2 -ffunction-sections -fdata-sections -Wall -fstack-usage -MMD -MP -MF"Core/_lib/Src/_lib_logging.d" -MT"$@" --specs=nano.specs -mfloat-abi=soft -mthumb -o "$@"
Core/_lib/Src/_lib_u8queue.o: ../Core/_lib/Src/_lib_u8queue.c Core/_lib/Src/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m0 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32F030x8 -c -I../Core/Inc -I../Drivers/STM32F0xx_HAL_Driver/Inc -I../Drivers/STM32F0xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32F0xx/Include -I../Drivers/CMSIS/Include -I../Core/_lib/Inc -I../Core/_mw/Inc -I../Core/_app/Inc -O2 -ffunction-sections -fdata-sections -Wall -fstack-usage -MMD -MP -MF"Core/_lib/Src/_lib_u8queue.d" -MT"$@" --specs=nano.specs -mfloat-abi=soft -mthumb -o "$@"
Core/_lib/Src/_lib_userdelay.o: ../Core/_lib/Src/_lib_userdelay.c Core/_lib/Src/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m0 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32F030x8 -c -I../Core/Inc -I../Drivers/STM32F0xx_HAL_Driver/Inc -I../Drivers/STM32F0xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32F0xx/Include -I../Drivers/CMSIS/Include -I../Core/_lib/Inc -I../Core/_mw/Inc -I../Core/_app/Inc -O2 -ffunction-sections -fdata-sections -Wall -fstack-usage -MMD -MP -MF"Core/_lib/Src/_lib_userdelay.d" -MT"$@" --specs=nano.specs -mfloat-abi=soft -mthumb -o "$@"

