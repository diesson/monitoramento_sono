################################################################################
# Automatically-generated file. Do not edit!
################################################################################

-include ../makefile.init

RM := rm -rf

# All of the sources participating in the build are defined here
-include sources.mk
-include lib/subdir.mk
-include display/subdir.mk
-include subdir.mk
-include objects.mk

ifneq ($(MAKECMDGOALS),clean)
ifneq ($(strip $(ASM_DEPS)),)
-include $(ASM_DEPS)
endif
ifneq ($(strip $(S_DEPS)),)
-include $(S_DEPS)
endif
ifneq ($(strip $(S_UPPER_DEPS)),)
-include $(S_UPPER_DEPS)
endif
ifneq ($(strip $(C_DEPS)),)
-include $(C_DEPS)
endif
endif

-include ../makefile.defs

# Add inputs and outputs from these tool invocations to the build variables 
LSS += \
MS_mao.lss \

FLASH_IMAGE += \
MS_mao.hex \

EEPROM_IMAGE += \
MS_mao.eep \

SIZEDUMMY += \
sizedummy \


# All Target
all: MS_mao.elf secondary-outputs

# Tool invocations
MS_mao.elf: $(OBJS) $(USER_OBJS)
	@echo 'Building target: $@'
	@echo 'Invoking: AVR C Linker'
	avr-gcc -Wl,-Map,MS_mao.map -mmcu=atmega328p -o "MS_mao.elf" $(OBJS) $(USER_OBJS) $(LIBS)
	@echo 'Finished building target: $@'
	@echo ' '

MS_mao.lss: MS_mao.elf
	@echo 'Invoking: AVR Create Extended Listing'
	-avr-objdump -h -S MS_mao.elf  >"MS_mao.lss"
	@echo 'Finished building: $@'
	@echo ' '

MS_mao.hex: MS_mao.elf
	@echo 'Create Flash image (ihex format)'
	-avr-objcopy -R .eeprom -R .fuse -R .lock -R .signature -O ihex MS_mao.elf  "MS_mao.hex"
	@echo 'Finished building: $@'
	@echo ' '

MS_mao.eep: MS_mao.elf
	@echo 'Create eeprom image (ihex format)'
	-avr-objcopy -j .eeprom --no-change-warnings --change-section-lma .eeprom=0 -O ihex MS_mao.elf  "MS_mao.eep"
	@echo 'Finished building: $@'
	@echo ' '

sizedummy: MS_mao.elf
	@echo 'Invoking: Print Size'
	-avr-size --format=avr --mcu=atmega328p MS_mao.elf
	@echo 'Finished building: $@'
	@echo ' '

# Other Targets
clean:
	-$(RM) $(FLASH_IMAGE)$(ELFS)$(OBJS)$(ASM_DEPS)$(EEPROM_IMAGE)$(S_DEPS)$(SIZEDUMMY)$(S_UPPER_DEPS)$(LSS)$(C_DEPS) MS_mao.elf
	-@echo ' '

secondary-outputs: $(LSS) $(FLASH_IMAGE) $(EEPROM_IMAGE) $(SIZEDUMMY)

.PHONY: all clean dependents

-include ../makefile.targets
