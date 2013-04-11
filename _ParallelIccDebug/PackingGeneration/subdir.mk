################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../PackingGeneration/Main.cpp 

OBJS += \
./PackingGeneration/Main.o 

CPP_DEPS += \
./PackingGeneration/Main.d 


# Each subdirectory must supply rules for building sources it contributes
PackingGeneration/%.o: ../PackingGeneration/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: Cygwin C++ Compiler'
	mpiicpc -DPARALLEL -DMPICH_IGNORE_CXX_SEEK -DMPICH_SKIP_MPICXX -DDEBUG -I../Externals/Boost -I../PackingGeneration -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


