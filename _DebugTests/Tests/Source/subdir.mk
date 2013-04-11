################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../Tests/Source/Assert.cpp \
../Tests/Source/ByteUtilityTests.cpp \
../Tests/Source/ClosestPairProviderTests.cpp \
../Tests/Source/ColumnMajorIndexingProviderTests.cpp \
../Tests/Source/EndiannessProviderStub.cpp \
../Tests/Source/GeometryCollisionServiceTests.cpp \
../Tests/Source/HcpGeneratorTests.cpp \
../Tests/Source/HessianServiceTests.cpp \
../Tests/Source/OrderedPriorityQueueTests.cpp \
../Tests/Source/PackingSerializerTests.cpp \
../Tests/Source/ParticleCollisionServiceTests.cpp \
../Tests/Source/RattlerRemovalServiceTests.cpp \
../Tests/Source/SphericalHarmonicsComputerTests.cpp \
../Tests/Source/TestRunner.cpp \
../Tests/Source/VelocityServiceTests.cpp 

OBJS += \
./Tests/Source/Assert.o \
./Tests/Source/ByteUtilityTests.o \
./Tests/Source/ClosestPairProviderTests.o \
./Tests/Source/ColumnMajorIndexingProviderTests.o \
./Tests/Source/EndiannessProviderStub.o \
./Tests/Source/GeometryCollisionServiceTests.o \
./Tests/Source/HcpGeneratorTests.o \
./Tests/Source/HessianServiceTests.o \
./Tests/Source/OrderedPriorityQueueTests.o \
./Tests/Source/PackingSerializerTests.o \
./Tests/Source/ParticleCollisionServiceTests.o \
./Tests/Source/RattlerRemovalServiceTests.o \
./Tests/Source/SphericalHarmonicsComputerTests.o \
./Tests/Source/TestRunner.o \
./Tests/Source/VelocityServiceTests.o 

CPP_DEPS += \
./Tests/Source/Assert.d \
./Tests/Source/ByteUtilityTests.d \
./Tests/Source/ClosestPairProviderTests.d \
./Tests/Source/ColumnMajorIndexingProviderTests.d \
./Tests/Source/EndiannessProviderStub.d \
./Tests/Source/GeometryCollisionServiceTests.d \
./Tests/Source/HcpGeneratorTests.d \
./Tests/Source/HessianServiceTests.d \
./Tests/Source/OrderedPriorityQueueTests.d \
./Tests/Source/PackingSerializerTests.d \
./Tests/Source/ParticleCollisionServiceTests.d \
./Tests/Source/RattlerRemovalServiceTests.d \
./Tests/Source/SphericalHarmonicsComputerTests.d \
./Tests/Source/TestRunner.d \
./Tests/Source/VelocityServiceTests.d 


# Each subdirectory must supply rules for building sources it contributes
Tests/Source/%.o: ../Tests/Source/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: Cygwin C++ Compiler'
	g++ -DDEBUG -I../Externals/Boost -I../PackingGeneration -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


