################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../display/LCD.c \
../display/displaySimpleMux.c 

OBJS += \
./display/LCD.o \
./display/displaySimpleMux.o 

C_DEPS += \
./display/LCD.d \
./display/displaySimpleMux.d 


# Each subdirectory must supply rules for building sources it contributes
display/%.o: ../display/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: AVR Compiler'
	avr-gcc -Wall -Os -fpack-struct -fshort-enums -ffunction-sections -fdata-sections -std=gnu99 -funsigned-char -funsigned-bitfields -mmcu=atmega328p -DF_CPU=16000000UL -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -c -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


