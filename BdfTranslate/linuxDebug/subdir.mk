################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../BdfTranslate.cpp \
../SRbdf.cpp \
../SRconstraint.cpp \
../SRcoord.cpp \
../SRelemBrickWedge.cpp \
../SRelement.cpp \
../SRfile.cpp \
../SRforce.cpp \
../SRinput.cpp \
../SRmachDep.cpp \
../SRmaterial.cpp \
../SRmath.cpp \
../SRmodel.cpp \
../SRnode.cpp \
../SRoutput.cpp \
../SRstring.cpp \
../SRutil.cpp 

OBJS += \
./BdfTranslate.o \
./SRbdf.o \
./SRconstraint.o \
./SRcoord.o \
./SRelemBrickWedge.o \
./SRelement.o \
./SRfile.o \
./SRforce.o \
./SRinput.o \
./SRmachDep.o \
./SRmaterial.o \
./SRmath.o \
./SRmodel.o \
./SRnode.o \
./SRoutput.o \
./SRstring.o \
./SRutil.o 

CPP_DEPS += \
./BdfTranslate.d \
./SRbdf.d \
./SRconstraint.d \
./SRcoord.d \
./SRelemBrickWedge.d \
./SRelement.d \
./SRfile.d \
./SRforce.d \
./SRinput.d \
./SRmachDep.d \
./SRmaterial.d \
./SRmath.d \
./SRmodel.d \
./SRnode.d \
./SRoutput.d \
./SRstring.d \
./SRutil.d 


# Each subdirectory must supply rules for building sources it contributes
%.o: ../%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -I/opt/intel/mkl/include -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


