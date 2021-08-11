GCC_PREFIX = arm-none-eabi-

CROSS_GCC = $(GCC_PREFIX)gcc
GCC = gcc
RM = -rm -rf
HIDE = @

PREFIX = Align4_
TESTCASENAME = dynamic_
CASE = $(TESTCASENAME)
AGCASE = $(PREFIX)$(TESTCASENAME)

SRCS = $(wildcard *.c)
SO = $(patsubst %.c,%.so,$(SRCS))
ALIGN_SO = $(patsubst %.c,$(PREFIX)%.so,$(SRCS))
# cpu
CPU = -mcpu=cortex-m4
# fpu
FPU = -mfpu=fpv4-sp-d16
# float-abi
FLOAT-ABI = -mfloat-abi=hard
# mcu
MCU = $(CPU) -mthumb $(FPU) $(FLOAT-ABI)
CFLAGS = -nostdlib -nostartfiles
ALIGN_CFLAGS = $(CFLAGS) -z max-page-size=4
INIT_FINI_CFLAGS = $(ALIGN_CFLAGS) -Wl,-init,init_first -Wl,-fini,fini_end

all: $(SO) $(ALIGN_SO)
$(SO): %.so : %.c
	$(HIDE)$(CROSS_GCC) -fPIC -shared $(MCU) $(CFLAGS) $(CASE)athmtc.c -o $(CASE)athmtc.so
	$(HIDE)$(CROSS_GCC) -fPIC -shared $(MCU) $(CFLAGS) $(CASE)bss.c -o $(CASE)bss.so
	$(HIDE)$(CROSS_GCC) -fPIC -shared $(MCU) $(CFLAGS) $(CASE)initfini.c -o $(CASE)align10000.so
	$(HIDE)$(CROSS_GCC) -fPIC -shared $(MCU) $(CFLAGS) $(CASE)static.c -o $(CASE)static.so
	$(HIDE)$(CROSS_GCC) -fPIC -shared $(MCU) $(CFLAGS) $(CASE)sym.c -o $(CASE)sym.so
	$(HIDE)$(CROSS_GCC) -fPIC -shared $(MCU) $(CFLAGS) $(CASE)undfunc.c -o $(CASE)undfunc.so
	$(HIDE)$(CROSS_GCC) -fPIC -shared $(MCU) $(CFLAGS) $(CASE)undval.c -o $(CASE)undval.so
	$(HIDE)$(CROSS_GCC) -fPIC -shared $(MCU) $(CFLAGS) $(CASE)xxxxx.c -o $(CASE)xxxxx.so

$(ALIGN_SO): $(PREFIX)%.so : %.c
	$(HIDE)$(CROSS_GCC) -fPIC -shared $(MCU) $(ALIGN_CFLAGS) $(CASE)initfini.c -o $(AGCASE)align4.so
	$(HIDE)$(CROSS_GCC) -fPIC -shared $(MCU) $(INIT_FINI_CFLAGS) $(CASE)initfini.c -o $(AGCASE)init_fini.so
# shared library not for need
	$(HIDE)$(GCC) -fPIC -shared $(ALIGN_CFLAGS) $(CASE)initfini.c -o $(AGCASE)x86.so
	$(HIDE)$(CROSS_GCC) -fPIE -pie $(MCU) $(ALIGN_CFLAGS) $(CASE)initfini.c -o $(AGCASE)pie.so
	$(HIDE)$(CROSS_GCC) $(MCU) $(ALIGN_CFLAGS) $(CASE)initfini.c -o $(AGCASE)exec.so
	$(HIDE)$(CROSS_GCC) -shared $(MCU) $(ALIGN_CFLAGS) $(CASE)initfini.c -o $(AGCASE)nopic.so
	$(HIDE)$(CROSS_GCC) -fPIC -shared $(MCU) $(ALIGN_CFLAGS) $(AGCASE)align4.so $(CASE)initfini.c -o \
                        $(AGCASE)need_others.so
	$(HIDE)$(CROSS_GCC) -fPIC -shared $(ALIGN_CFLAGS) $(CASE)initfini.c -o $(AGCASE)noThumb.so
	$(HIDE)$(CROSS_GCC) -fPIC -shared $(MCU) $(CASE)initfini.c -o $(AGCASE)stdlib.so

clean:
	$(HIDE)$(RM) *.so

.PHONY: all clean

