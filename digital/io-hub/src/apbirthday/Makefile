BASE = ../../../ucoolib

TARGETS = host stm32f4
PROGS = apbirthday
apbirthday_SOURCES = main.cc robot.cc hardware.host.cc hardware.stm32.cc \
		     top.cc \
		     angfsm.host.c angfsm_gen_arm_AI.arm.c \
		     $(AVR_SOURCES)

MODULES = hal/gpio hal/uart hal/usb base/proto

# Hack mode, include old code from AVR modules.
AVR_MODULES = math/geometry math/fixed
AVR_BASE = $(BASE)/../avr
INCLUDES += -I$(AVR_BASE) -I$(AVR_BASE)/common
host_DEFS += -DHOST
include $(AVR_MODULES:%=$(AVR_BASE)/modules/%/Makefile.module)
vpath %.c $(AVR_MODULES:%=$(AVR_BASE)/modules/%)
AVR_SOURCES = $(foreach module,$(AVR_MODULES),$($(subst /,_,$(module))_SOURCES))

# FSM.
INCLUDES += -I$(BASE)/../ai/src/fsm -I.
vpath %.c $(BASE)/../ai/src/fsm

include $(BASE)/build/top.mk

clean: clean.project
clean.project:
	rm -f usb zb_uart angfsm_gen_arm_AI.h angfsm_gen_arm_AI.arm.c

obj/main.stm32f4.o: angfsm_gen_arm_AI.h
angfsm_gen_arm_AI.arm.c: angfsm_gen_arm_AI.h
angfsm_gen_arm_AI.h: apbirthday.host
	./$< --ang-gen arm && mv angfsm_gen_arm_AI.c angfsm_gen_arm_AI.arm.c