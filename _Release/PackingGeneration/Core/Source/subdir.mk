################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../PackingGeneration/Core/Source/EndiannessProvider.cpp \
../PackingGeneration/Core/Source/Exceptions.cpp \
../PackingGeneration/Core/Source/Math.cpp \
../PackingGeneration/Core/Source/MpiManager.cpp \
../PackingGeneration/Core/Source/Path.cpp \
../PackingGeneration/Core/Source/Utilities.cpp \
../PackingGeneration/Core/Source/VectorUtilities.cpp 

OBJS += \
./PackingGeneration/Core/Source/EndiannessProvider.o \
./PackingGeneration/Core/Source/Exceptions.o \
./PackingGeneration/Core/Source/Math.o \
./PackingGeneration/Core/Source/MpiManager.o \
./PackingGeneration/Core/Source/Path.o \
./PackingGeneration/Core/Source/Utilities.o \
./PackingGeneration/Core/Source/VectorUtilities.o 

CPP_DEPS += \
./PackingGeneration/Core/Source/EndiannessProvider.d \
./PackingGeneration/Core/Source/Exceptions.d \
./PackingGeneration/Core/Source/Math.d \
./PackingGeneration/Core/Source/MpiManager.d \
./PackingGeneration/Core/Source/Path.d \
./PackingGeneration/Core/Source/Utilities.d \
./PackingGeneration/Core/Source/VectorUtilities.d 


# Each subdirectory must supply rules for building sources it contributes
PackingGeneration/Core/Source/%.o: ../PackingGeneration/Core/Source/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: Cygwin C++ Compiler'
	g++ -DBOOST_DISABLE_ASSERTS -DNDEBUG -I../Externals/Eigen -I../Externals/Boost -I../PackingGeneration -O3 -funroll-loops -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


