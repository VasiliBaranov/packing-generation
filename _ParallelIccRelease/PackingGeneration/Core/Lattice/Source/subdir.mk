################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../PackingGeneration/Core/Lattice/Source/ColumnMajorIndexingProvider.cpp \
../PackingGeneration/Core/Lattice/Source/D3Q27Lattice.cpp \
../PackingGeneration/Core/Lattice/Source/LatticeIndexingProvider.cpp 

OBJS += \
./PackingGeneration/Core/Lattice/Source/ColumnMajorIndexingProvider.o \
./PackingGeneration/Core/Lattice/Source/D3Q27Lattice.o \
./PackingGeneration/Core/Lattice/Source/LatticeIndexingProvider.o 

CPP_DEPS += \
./PackingGeneration/Core/Lattice/Source/ColumnMajorIndexingProvider.d \
./PackingGeneration/Core/Lattice/Source/D3Q27Lattice.d \
./PackingGeneration/Core/Lattice/Source/LatticeIndexingProvider.d 


# Each subdirectory must supply rules for building sources it contributes
PackingGeneration/Core/Lattice/Source/%.o: ../PackingGeneration/Core/Lattice/Source/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: Cygwin C++ Compiler'
	mpiicpc -DPARALLEL -DMPICH_IGNORE_CXX_SEEK -DBOOST_DISABLE_ASSERTS -DMPICH_SKIP_MPICXX -I../Externals/Boost -I../PackingGeneration -O3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


