################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../lib/avr_gpio.c \
../lib/avr_i2c.c \
../lib/avr_onewire.c \
../lib/avr_usart.c 

OBJS += \
./lib/avr_gpio.o \
./lib/avr_i2c.o \
./lib/avr_onewire.o \
./lib/avr_usart.o 

C_DEPS += \
./lib/avr_gpio.d \
./lib/avr_i2c.d \
./lib/avr_onewire.d \
./lib/avr_usart.d 


# Each subdirectory must supply rules for building sources it contributes
lib/%.o: ../lib/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: AVR Compiler'
	avr-gcc -Wall -Os -fpack-struct -fshort-enums -ffunction-sections -fdata-sections -std=gnu99 -funsigned-char -funsigned-bitfields -mmcu=atmega328p -DF_CPU=16000000UL -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -c -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


