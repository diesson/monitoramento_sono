################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../accelerometer.c \
../main.c \
../monitor.c \
../oximetro.c \
../temperatura.c 

OBJS += \
./accelerometer.o \
./main.o \
./monitor.o \
./oximetro.o \
./temperatura.o 

C_DEPS += \
./accelerometer.d \
./main.d \
./monitor.d \
./oximetro.d \
./temperatura.d 


# Each subdirectory must supply rules for building sources it contributes
%.o: ../%.c
	@echo 'Building file: $<'
	@echo 'Invoking: AVR Compiler'
	avr-gcc -Wall -Os -fpack-struct -fshort-enums -ffunction-sections -fdata-sections -std=gnu99 -funsigned-char -funsigned-bitfields -mmcu=atmega328p -DF_CPU=16000000UL -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -c -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


