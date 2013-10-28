################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../src/CompletionTrie.cpp \
../src/CompletionTrieBuilder.cpp \
../src/PackedNode.cpp \
../src/main.cpp 

OBJS += \
./src/CompletionTrie.o \
./src/CompletionTrieBuilder.o \
./src/PackedNode.o \
./src/main.o 

CPP_DEPS += \
./src/CompletionTrie.d \
./src/CompletionTrieBuilder.d \
./src/PackedNode.d \
./src/main.d 


# Each subdirectory must supply rules for building sources it contributes
src/%.o: ../src/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -O0 -g3 -Wall -c -fmessage-length=0 -std=c++11 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


