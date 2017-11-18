################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../MQTTPacket/src/MQTTConnectClient.c \
../MQTTPacket/src/MQTTConnectServer.c \
../MQTTPacket/src/MQTTDeserializePublish.c \
../MQTTPacket/src/MQTTFormat.c \
../MQTTPacket/src/MQTTPacket.c \
../MQTTPacket/src/MQTTSerializePublish.c \
../MQTTPacket/src/MQTTSubscribeClient.c \
../MQTTPacket/src/MQTTSubscribeServer.c \
../MQTTPacket/src/MQTTUnsubscribeClient.c \
../MQTTPacket/src/MQTTUnsubscribeServer.c 

OBJS += \
./MQTTPacket/src/MQTTConnectClient.o \
./MQTTPacket/src/MQTTConnectServer.o \
./MQTTPacket/src/MQTTDeserializePublish.o \
./MQTTPacket/src/MQTTFormat.o \
./MQTTPacket/src/MQTTPacket.o \
./MQTTPacket/src/MQTTSerializePublish.o \
./MQTTPacket/src/MQTTSubscribeClient.o \
./MQTTPacket/src/MQTTSubscribeServer.o \
./MQTTPacket/src/MQTTUnsubscribeClient.o \
./MQTTPacket/src/MQTTUnsubscribeServer.o 

C_DEPS += \
./MQTTPacket/src/MQTTConnectClient.d \
./MQTTPacket/src/MQTTConnectServer.d \
./MQTTPacket/src/MQTTDeserializePublish.d \
./MQTTPacket/src/MQTTFormat.d \
./MQTTPacket/src/MQTTPacket.d \
./MQTTPacket/src/MQTTSerializePublish.d \
./MQTTPacket/src/MQTTSubscribeClient.d \
./MQTTPacket/src/MQTTSubscribeServer.d \
./MQTTPacket/src/MQTTUnsubscribeClient.d \
./MQTTPacket/src/MQTTUnsubscribeServer.d 


# Each subdirectory must supply rules for building sources it contributes
MQTTPacket/src/%.o: ../MQTTPacket/src/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -I"C:\Users\timur\Documents\paho.mqtt.embedded-c\MQTTPacket\src" -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


