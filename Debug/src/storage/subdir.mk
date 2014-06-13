################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../src/storage/BuilderNode.cpp \
../src/storage/CompletionTrie.cpp \
../src/storage/CompletionTrieBuilder.cpp \
../src/storage/PackedNode.cpp \
../src/storage/SuggestionList.cpp \
../src/storage/SuggestionStore.cpp 

OBJS += \
./src/storage/BuilderNode.o \
./src/storage/CompletionTrie.o \
./src/storage/CompletionTrieBuilder.o \
./src/storage/PackedNode.o \
./src/storage/SuggestionList.o \
./src/storage/SuggestionStore.o 

CPP_DEPS += \
./src/storage/BuilderNode.d \
./src/storage/CompletionTrie.d \
./src/storage/CompletionTrieBuilder.d \
./src/storage/PackedNode.d \
./src/storage/SuggestionList.d \
./src/storage/SuggestionStore.d 


# Each subdirectory must supply rules for building sources it contributes
src/storage/%.o: ../src/storage/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -D__GXX_EXPERIMENTAL_CXX0X__ -O0 -g3 -Wall -c -fmessage-length=0 -std=c++11 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


