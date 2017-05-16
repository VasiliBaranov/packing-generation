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
	mpiicpc -DPARALLEL -DNDEBUG -DMPICH_IGNORE_CXX_SEEK -DBOOST_DISABLE_ASSERTS -DMPICH_SKIP_MPICXX -I../Externals/Boost -I../Externals/Eigen -I../PackingGeneration -O3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


