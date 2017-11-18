################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../MQTTPacket/samples/null.c \
../MQTTPacket/samples/ping.c \
../MQTTPacket/samples/ping_nb.c \
../MQTTPacket/samples/pub0sub1.c \
../MQTTPacket/samples/pub0sub1_nb.c \
../MQTTPacket/samples/qos0pub.c \
../MQTTPacket/samples/transport.c 

OBJS += \
./MQTTPacket/samples/null.o \
./MQTTPacket/samples/ping.o \
./MQTTPacket/samples/ping_nb.o \
./MQTTPacket/samples/pub0sub1.o \
./MQTTPacket/samples/pub0sub1_nb.o \
./MQTTPacket/samples/qos0pub.o \
./MQTTPacket/samples/transport.o 

C_DEPS += \
./MQTTPacket/samples/null.d \
./MQTTPacket/samples/ping.d \
./MQTTPacket/samples/ping_nb.d \
./MQTTPacket/samples/pub0sub1.d \
./MQTTPacket/samples/pub0sub1_nb.d \
./MQTTPacket/samples/qos0pub.d \
./MQTTPacket/samples/transport.d 


# Each subdirectory must supply rules for building sources it contributes
MQTTPacket/samples/%.o: ../MQTTPacket/samples/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -I"C:\Users\timur\Documents\paho.mqtt.embedded-c\MQTTPacket\src" -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


