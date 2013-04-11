################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../PackingGeneration/Generation/PackingServices/Source/GeometryCollisionService.cpp \
../PackingGeneration/Generation/PackingServices/Source/GeometryService.cpp \
../PackingGeneration/Generation/PackingServices/Source/MathService.cpp \
../PackingGeneration/Generation/PackingServices/Source/PackingSerializer.cpp 

OBJS += \
./PackingGeneration/Generation/PackingServices/Source/GeometryCollisionService.o \
./PackingGeneration/Generation/PackingServices/Source/GeometryService.o \
./PackingGeneration/Generation/PackingServices/Source/MathService.o \
./PackingGeneration/Generation/PackingServices/Source/PackingSerializer.o 

CPP_DEPS += \
./PackingGeneration/Generation/PackingServices/Source/GeometryCollisionService.d \
./PackingGeneration/Generation/PackingServices/Source/GeometryService.d \
./PackingGeneration/Generation/PackingServices/Source/MathService.d \
./PackingGeneration/Generation/PackingServices/Source/PackingSerializer.d 


# Each subdirectory must supply rules for building sources it contributes
PackingGeneration/Generation/PackingServices/Source/%.o: ../PackingGeneration/Generation/PackingServices/Source/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: Cygwin C++ Compiler'
	g++ -DDEBUG -I../Externals/Boost -I../PackingGeneration -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


