EE_OBJS	= HIDMouse.o iomanX.o ps2mouse.o usbd.o
EE_BIN = HIDMouse.elf
EE_LIBS = -L$(GSKIT)/lib -L$(PS2SDK)/ports/lib -lkernel -lgskit -ldmakit -lmouse -lpatches -lpng -lz -lgskit_toolkit
EE_INCS += -I$(GSKIT)/include -I$(PS2DEV)/ports/libpng/include

all: $(EE_BIN)

BIN2C = $(PS2SDK)/bin/bin2c

iomanX.c: $(PS2SDK)/iop/irx/iomanX.irx | $(EE_ASM_DIR)
	$(BIN2C) $< $@ iomanX_irx

ps2mouse.c: $(PS2SDK)/iop/irx/ps2mouse.irx | $(EE_ASM_DIR)
	$(BIN2C) $< $@ ps2mouse_irx

usbd.c: $(PS2SDK)/iop/irx/usbd.irx | $(EE_ASM_DIR)
	$(BIN2C) $< $@ usbd_irx

clean:
	rm -f $(EE_BIN) $(EE_OBJS) 

run: $(EE_BIN)
	ps2client execee host:$(EE_BIN)

reset:
	ps2client reset
	ps2client netdump

include $(PS2SDK)/samples/Makefile.pref
include $(PS2SDK)/samples/Makefile.eeglobal