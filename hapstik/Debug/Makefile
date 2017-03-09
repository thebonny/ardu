################################################################################
# Automatically-generated file. Do not edit!
################################################################################

SHELL := cmd.exe
RM := rm -rf

USER_OBJS :=

LIBS := 
PROJ := 

O_SRCS := 
C_SRCS := 
S_SRCS := 
S_UPPER_SRCS := 
OBJ_SRCS := 
ASM_SRCS := 
PREPROCESSING_SRCS := 
OBJS := 
OBJS_AS_ARGS := 
C_DEPS := 
C_DEPS_AS_ARGS := 
EXECUTABLES := 
OUTPUT_FILE_PATH :=
OUTPUT_FILE_PATH_AS_ARGS :=
AVR_APP_PATH :=$$$AVR_APP_PATH$$$
QUOTE := "
ADDITIONAL_DEPENDENCIES:=
OUTPUT_FILE_DEP:=
LIB_DEP:=
LINKER_SCRIPT_DEP:=

# Every subdirectory with source files must be described here
SUBDIRS :=  \
../config


# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS +=  \
../ADC.cpp \
../cycle_counter.cpp \
../hapstik.cpp \
../PID.cpp \
../ppm_capture.cpp \
../ppm_out.cpp \
../PWM.cpp \
../record_playback.cpp \
../utils.cpp


PREPROCESSING_SRCS += 


ASM_SRCS += 


OBJS +=  \
ADC.o \
cycle_counter.o \
hapstik.o \
PID.o \
ppm_capture.o \
ppm_out.o \
PWM.o \
record_playback.o \
utils.o

OBJS_AS_ARGS +=  \
ADC.o \
cycle_counter.o \
hapstik.o \
PID.o \
ppm_capture.o \
ppm_out.o \
PWM.o \
record_playback.o \
utils.o

C_DEPS +=  \
ADC.d \
cycle_counter.d \
hapstik.d \
PID.d \
ppm_capture.d \
ppm_out.d \
PWM.d \
record_playback.d \
utils.d

C_DEPS_AS_ARGS +=  \
ADC.d \
cycle_counter.d \
hapstik.d \
PID.d \
ppm_capture.d \
ppm_out.d \
PWM.d \
record_playback.d \
utils.d

OUTPUT_FILE_PATH +=libhapstik.a

OUTPUT_FILE_PATH_AS_ARGS +=libhapstik.a

ADDITIONAL_DEPENDENCIES:=

OUTPUT_FILE_DEP:= ./makedep.mk

LIB_DEP+= 

LINKER_SCRIPT_DEP+= 


# AVR32/GNU C Compiler



















./%.o: .././%.cpp
	@echo Building file: $<
	@echo Invoking: ARM/GNU C Compiler : 5.3.1
	$(QUOTE)C:\Program Files (x86)\Atmel\Studio\7.0\toolchain\arm\arm-gnu-toolchain\bin\arm-none-eabi-g++.exe$(QUOTE) -mthumb -D__SAM3X8E__ -DDEBUG  -I"C:\Program Files (x86)\Atmel\Studio\7.0\Packs\atmel\SAM3X_DFP\1.0.50\include" -I"C:\Program Files (x86)\Atmel\Studio\7.0\Packs\arm\cmsis\4.2.0\CMSIS\Include"  -O1 -ffunction-sections -fno-rtti -fno-exceptions -mlong-calls -g3 -Wall -mcpu=cortex-m3 -c -MD -MP -MF "$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -MT"$(@:%.o=%.o)"   -o "$@" "$<" 
	@echo Finished building: $<
	



# AVR32/GNU Preprocessing Assembler



# AVR32/GNU Assembler




ifneq ($(MAKECMDGOALS),clean)
ifneq ($(strip $(C_DEPS)),)
-include $(C_DEPS)
endif
endif

# Add inputs and outputs from these tool invocations to the build variables 

# All Target
all: $(OUTPUT_FILE_PATH) $(ADDITIONAL_DEPENDENCIES)


$(OUTPUT_FILE_PATH): $(OBJS) $(USER_OBJS) $(OUTPUT_FILE_DEP)
	@echo Building target: $@
	@echo Invoking: ARM/GNU Archiver : 5.3.1
	$(QUOTE)C:\Program Files (x86)\Atmel\Studio\7.0\toolchain\arm\arm-gnu-toolchain\bin\arm-none-eabi-ar.exe$(QUOTE) -r  -o$(OUTPUT_FILE_PATH_AS_ARGS) $(OBJS_AS_ARGS) $(USER_OBJS) $(LIBS) 
	@echo Finished building target: $@
	
	




# Other Targets
clean:
	-$(RM) $(OBJS_AS_ARGS) $(EXECUTABLES)  
	-$(RM) $(C_DEPS_AS_ARGS)   
	rm -rf "libhapstik.elf" "libhapstik.a" "libhapstik.hex" "libhapstik.bin" "libhapstik.lss" "libhapstik.eep" "libhapstik.map" "libhapstik.srec"
	