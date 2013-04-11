################################################################################
# Automatically-generated file. Do not edit!
################################################################################

-include ../makefile.init

RM := rm -rf

# All of the sources participating in the build are defined here
-include sources.mk
-include Tests/Source/subdir.mk
-include Tests/subdir.mk
-include PackingGeneration/Parallelism/Source/subdir.mk
-include PackingGeneration/Generation/PackingServices/Source/subdir.mk
-include PackingGeneration/Generation/PackingServices/PostProcessing/Source/subdir.mk
-include PackingGeneration/Generation/PackingServices/EnergyServices/Source/subdir.mk
-include PackingGeneration/Generation/PackingServices/DistanceServices/Source/subdir.mk
-include PackingGeneration/Generation/PackingGenerators/Source/subdir.mk
-include PackingGeneration/Generation/PackingGenerators/LubachevsckyStillinger/Source/subdir.mk
-include PackingGeneration/Generation/PackingGenerators/InitialGenerators/Source/subdir.mk
-include PackingGeneration/Generation/Model/Source/subdir.mk
-include PackingGeneration/Generation/Geometries/Source/subdir.mk
-include PackingGeneration/Generation/subdir.mk
-include PackingGeneration/Execution/Source/subdir.mk
-include PackingGeneration/Core/Source/subdir.mk
-include PackingGeneration/Core/Lattice/Source/subdir.mk
-include PackingGeneration/Core/Geometry/Source/subdir.mk
-include Externals/Boost/libs/smart_ptr/src/subdir.mk
-include subdir.mk
-include objects.mk

ifneq ($(MAKECMDGOALS),clean)
ifneq ($(strip $(C++_DEPS)),)
-include $(C++_DEPS)
endif
ifneq ($(strip $(C_DEPS)),)
-include $(C_DEPS)
endif
ifneq ($(strip $(CC_DEPS)),)
-include $(CC_DEPS)
endif
ifneq ($(strip $(CPP_DEPS)),)
-include $(CPP_DEPS)
endif
ifneq ($(strip $(CXX_DEPS)),)
-include $(CXX_DEPS)
endif
ifneq ($(strip $(C_UPPER_DEPS)),)
-include $(C_UPPER_DEPS)
endif
endif

-include ../makefile.defs

# Add inputs and outputs from these tool invocations to the build variables 

# All Target
all: PackingGeneration.exe

# Tool invocations
PackingGeneration.exe: $(OBJS) $(USER_OBJS)
	@echo 'Building target: $@'
	@echo 'Invoking: Cygwin C++ Linker'
	g++  -o "PackingGeneration.exe" $(OBJS) $(USER_OBJS) $(LIBS)
	@echo 'Finished building target: $@'
	@echo ' '

# Other Targets
clean:
	-$(RM) $(C++_DEPS)$(OBJS)$(C_DEPS)$(CC_DEPS)$(CPP_DEPS)$(EXECUTABLES)$(CXX_DEPS)$(C_UPPER_DEPS) PackingGeneration.exe
	-@echo ' '

.PHONY: all clean dependents
.SECONDARY:

-include ../makefile.targets
