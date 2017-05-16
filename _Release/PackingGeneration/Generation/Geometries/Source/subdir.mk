################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../PackingGeneration/Generation/Geometries/Source/BaseGeometry.cpp \
../PackingGeneration/Generation/Geometries/Source/BulkGeometry.cpp \
../PackingGeneration/Generation/Geometries/Source/CircleGeometry.cpp \
../PackingGeneration/Generation/Geometries/Source/RectangleGeometry.cpp \
../PackingGeneration/Generation/Geometries/Source/TrapezoidGeometry.cpp 

OBJS += \
./PackingGeneration/Generation/Geometries/Source/BaseGeometry.o \
./PackingGeneration/Generation/Geometries/Source/BulkGeometry.o \
./PackingGeneration/Generation/Geometries/Source/CircleGeometry.o \
./PackingGeneration/Generation/Geometries/Source/RectangleGeometry.o \
./PackingGeneration/Generation/Geometries/Source/TrapezoidGeometry.o 

CPP_DEPS += \
./PackingGeneration/Generation/Geometries/Source/BaseGeometry.d \
./PackingGeneration/Generation/Geometries/Source/BulkGeometry.d \
./PackingGeneration/Generation/Geometries/Source/CircleGeometry.d \
./PackingGeneration/Generation/Geometries/Source/RectangleGeometry.d \
./PackingGeneration/Generation/Geometries/Source/TrapezoidGeometry.d 


# Each subdirectory must supply rules for building sources it contributes
PackingGeneration/Generation/Geometries/Source/%.o: ../PackingGeneration/Generation/Geometries/Source/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: Cygwin C++ Compiler'
	g++ -DBOOST_DISABLE_ASSERTS -DNDEBUG -I../Externals/Eigen -I../Externals/Boost -I../PackingGeneration -O3 -funroll-loops -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


