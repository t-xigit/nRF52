################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../MQTTPacket/samples/baremetalserial/ping_nb.c \
../MQTTPacket/samples/baremetalserial/pub0sub1_nb.c \
../MQTTPacket/samples/baremetalserial/transport.c 

OBJS += \
./MQTTPacket/samples/baremetalserial/ping_nb.o \
./MQTTPacket/samples/baremetalserial/pub0sub1_nb.o \
./MQTTPacket/samples/baremetalserial/transport.o 

C_DEPS += \
./MQTTPacket/samples/baremetalserial/ping_nb.d \
./MQTTPacket/samples/baremetalserial/pub0sub1_nb.d \
./MQTTPacket/samples/baremetalserial/transport.d 


# Each subdirectory must supply rules for building sources it contributes
MQTTPacket/samples/baremetalserial/%.o: ../MQTTPacket/samples/baremetalserial/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -I"C:\Users\timur\Documents\paho.mqtt.embedded-c\MQTTPacket\src" -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


