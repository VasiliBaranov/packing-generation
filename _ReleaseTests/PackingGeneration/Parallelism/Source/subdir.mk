################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../PackingGeneration/Parallelism/Source/FileLock.cpp \
../PackingGeneration/Parallelism/Source/RandomLoadBalancer.cpp \
../PackingGeneration/Parallelism/Source/TaskManager.cpp 

OBJS += \
./PackingGeneration/Parallelism/Source/FileLock.o \
./PackingGeneration/Parallelism/Source/RandomLoadBalancer.o \
./PackingGeneration/Parallelism/Source/TaskManager.o 

CPP_DEPS += \
./PackingGeneration/Parallelism/Source/FileLock.d \
./PackingGeneration/Parallelism/Source/RandomLoadBalancer.d \
./PackingGeneration/Parallelism/Source/TaskManager.d 


# Each subdirectory must supply rules for building sources it contributes
PackingGeneration/Parallelism/Source/%.o: ../PackingGeneration/Parallelism/Source/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: Cygwin C++ Compiler'
	g++ -DBOOST_DISABLE_ASSERTS -DNDEBUG -I../Externals/Boost -I../Externals/Eigen -I../PackingGeneration -O3 -funroll-loops -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


