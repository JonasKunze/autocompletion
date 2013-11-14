################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../src/BuilderNode.cpp \
../src/CompletionServer.cpp \
../src/CompletionTrie.cpp \
../src/CompletionTrieBuilder.cpp \
../src/PackedNode.cpp \
../src/SuggestionList.cpp \
../src/SuggestionStore.cpp \
../src/main.cpp \
../src/zmqJSONServer.cpp 

OBJS += \
./src/BuilderNode.o \
./src/CompletionServer.o \
./src/CompletionTrie.o \
./src/CompletionTrieBuilder.o \
./src/PackedNode.o \
./src/SuggestionList.o \
./src/SuggestionStore.o \
./src/main.o \
./src/zmqJSONServer.o 

CPP_DEPS += \
./src/BuilderNode.d \
./src/CompletionServer.d \
./src/CompletionTrie.d \
./src/CompletionTrieBuilder.d \
./src/PackedNode.d \
./src/SuggestionList.d \
./src/SuggestionStore.d \
./src/main.d \
./src/zmqJSONServer.d 


# Each subdirectory must supply rules for building sources it contributes
src/%.o: ../src/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -D__GXX_EXPERIMENTAL_CXX0X__ -O3 -g3 -pg -Wall -c -fmessage-length=0 -std=c++11 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


