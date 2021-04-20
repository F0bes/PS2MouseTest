EE_OBJS	= HIDMouse.o irx/iomanX.cpp irx/ps2mouse.cpp irx/usbd.cpp
EE_BIN = HIDMouse.elf
EE_LIBS = -L$(GSKIT)/lib -lkernel -lgskit -ldmakit -lmouse -lpatches -lgskit_toolkit
EE_INCS += -I$(GSKIT)/include -I$(PS2DEV)/ports/libpng/include

all: $(EE_BIN)

clean:
	rm -f $(EE_BIN) $(EE_OBJS) 

run: $(EE_BIN)
	ps2client execee host:$(EE_BIN)

reset:
	ps2client reset
	ps2client netdump

include $(PS2SDK)/samples/Makefile.pref
include $(PS2SDK)/samples/Makefile.eeglobal