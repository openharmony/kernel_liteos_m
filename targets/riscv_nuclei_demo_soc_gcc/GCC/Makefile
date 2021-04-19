# ------------------------------------------------
# Generic Makefile (based on gcc)
#
# ChangeLog :
#	2021-04-02 - first version
# ------------------------------------------------

######################################
# target
######################################
TARGET = Nuclei-demo-soc

######################################
# building variables
######################################
# debug build?
DEBUG = 1
# optimization
OPT = -O2


#######################################
# paths
#######################################
# Build path
BUILD_DIR = build

#######################################
# Base directory
#######################################
# LiteOS top path
LITEOSTOPDIR := ../../../

######################################
# source
######################################
# C sources
C_SOURCES =  \
$(wildcard ../Src/*.c) \
$(wildcard ../SoC/demosoc/Common/Source/*.c) \
$(wildcard ../SoC/demosoc/Common/Source/Drivers/*.c) \
$(wildcard ../SoC/demosoc/Common/Source/Stubs/*.c) 

# ASM sources
ASMS_SOURCES =  \
$(wildcard ../SoC/demosoc/Common/Source/GCC/*.S) 


#######################################
# binaries
#######################################
PREFIX = riscv-nuclei-elf-
# The gcc compiler bin path can be either defined in make command via NUCLEI_TOOL_ROOT variable (> make NUCLEI_TOOL_ROOT=xxx)
# either it can be added to the PATH environment variable.
NUCLEI_RISCV_GCC_ROOT ?= $(NUCLEI_TOOL_ROOT)/gcc
NUCLEI_OPENOCD_ROOT ?= $(NUCLEI_TOOL_ROOT)/openocd

NUCLEI_TOOL_ROOT_EXIST = 0
ifneq ($(wildcard $(NUCLEI_RISCV_GCC_ROOT)),)
ifneq ($(wildcard $(NUCLEI_OPENOCD_ROOT)),)
NUCLEI_TOOL_ROOT_EXIST = 1
endif
endif

ifeq ($(NUCLEI_TOOL_ROOT_EXIST),1)
CC = $(NUCLEI_RISCV_GCC_ROOT)/bin/$(PREFIX)gcc
AS = $(NUCLEI_RISCV_GCC_ROOT)/bin/$(PREFIX)gcc -x assembler-with-cpp
CP = $(NUCLEI_RISCV_GCC_ROOT)/bin/$(PREFIX)objcopy
DP = $(NUCLEI_RISCV_GCC_ROOT)/bin/$(PREFIX)objdump
SZ = $(NUCLEI_RISCV_GCC_ROOT)/bin/$(PREFIX)size
GDB = $(NUCLEI_RISCV_GCC_ROOT)/bin/$(PREFIX)gdb
OPENOCD := $(NUCLEI_OPENOCD_ROOT)/bin/openocd
else
CC = $(PREFIX)gcc
AS = $(PREFIX)gcc -x assembler-with-cpp
CP = $(PREFIX)objcopy
DP = $(PREFIX)objdump
SZ = $(PREFIX)size
GDB = $(PREFIX)gdb
OPENOCD := openocd
endif
HEX = $(CP) -O ihex
BIN = $(CP) -O binary -S

ECHO := echo

OPENOCD_CFG := openocd_demosoc.cfg

#######################################
# CFLAGS
#######################################
#risc-v arch & abi
CORE_ARCH_ABI = -march=rv32imafc -mabi=ilp32f

#other flags
OTHER_FLAGS += -g -mcmodel=medany -fno-common

# macros for gcc
# AS defines
AS_DEFS = -DDOWNLOAD_MODE=DOWNLOAD_MODE_ILM

# C defines
C_DEFS = -DDOWNLOAD_MODE=DOWNLOAD_MODE_ILM


# AS includes
AS_INCLUDES =

# C includes
C_INCLUDES =  \
-I../OS_CONFIG \
-I../Src \
-I../SoC/demosoc/Board/nuclei_fpga_eval/Include \
-I../SoC/demosoc/Common/Include \
-I../SoC/demosoc/Common/Source/Stubs 

# compile gcc flags
ASFLAGS = $(AS_DEFS) $(OPT) $(CORE_ARCH_ABI) $(OTHER_FLAGS) $(AS_INCLUDES) -Wall -fdata-sections -ffunction-sections

CFLAGS = $(C_DEFS) $(C_INCLUDES) $(OPT) $(CORE_ARCH_ABI) $(OTHER_FLAGS) -Wall -fdata-sections -ffunction-sections

# Generate dependency information
CFLAGS += -MMD -MP -MF"$(@:%.o=%.d)"

# Set your GDB port using variable GDB_PORT
GDB_PORT ?= 3333
## Makefile Variable GDBREMOTE
## You can change GDBREMOTE to other gdb remotes
## eg. if you have started openocd server with (bindto 0.0.0.0 defined in openocd.cfg)
## make sure your machine can connect to remote machine
## in remote machine(ipaddr 192.168.43.199, port 3333) which connect the hardware board,
## then you can change the GDBREMOTE to 192.168.43.199:3333
## GDBREMOTE ?= 192.168.43.199:3333
GDBREMOTE ?= | $(OPENOCD) --pipe -f $(OPENOCD_CFG)

GDB_UPLOAD_ARGS ?= --batch
GDB_UPLOAD_CMDS += -ex "monitor halt"
GDB_UPLOAD_CMDS += -ex "monitor flash protect 0 0 last off"
GDB_UPLOAD_CMDS += -ex "load"
GDB_UPLOAD_CMDS += -ex "monitor resume"
GDB_UPLOAD_CMDS += -ex "quit"

OPENOCD_PORT_ARGS = -c "gdb_port $(GDB_PORT)"

OPENOCD_ARGS += -f $(OPENOCD_CFG)
GDB_CMDS += -ex "set remotetimeout 240"
GDB_CMDS += -ex "target extended-remote localhost:$(GDB_PORT)"

#######################################
# LDFLAGS
#######################################
# link script
LDSCRIPT = gcc_demosoc_ilm.ld

# libraries
LIBS = -lm 
LIBDIR =
LDFLAGS = $(CORE_ARCH_ABI) $(OTHER_FLAGS) -specs=nano.specs -specs=nosys.specs -T $(LDSCRIPT) $(LIBDIR) $(LIBS) -nostartfiles -Wl,-Map=$(BUILD_DIR)/$(TARGET).map,--cref -Wl,--gc-sections

# default action: build all
all: $(BUILD_DIR)/$(TARGET).elf $(BUILD_DIR)/$(TARGET).dasm $(BUILD_DIR)/$(TARGET).hex $(BUILD_DIR)/$(TARGET).bin

include liteos_m.mk

#######################################
# build the application
#######################################
# list of objects
OBJECTS += $(addprefix $(BUILD_DIR)/,$(notdir $(C_SOURCES:.c=.o)))
vpath %.c $(sort $(dir $(C_SOURCES)))
# list of ASM program objects
OBJECTS += $(addprefix $(BUILD_DIR)/,$(notdir $(ASM_SOURCES:.S=.o)))
vpath %.s $(sort $(dir $(ASM_SOURCES)))

$(BUILD_DIR)/%.o: %.c Makefile | $(BUILD_DIR)
	$(CC) -c $(CFLAGS) -Wa,-a,-ad,-alms=$(BUILD_DIR)/$(notdir $(<:.c=.lst)) $< -o $@

$(BUILD_DIR)/%.o: %.s Makefile | $(BUILD_DIR)
	$(AS) -c $(CFLAGS) $< -o $@

$(BUILD_DIR)/$(TARGET).elf: $(OBJECTS) Makefile $(LDSCRIPT)
	$(CC) $(OBJECTS) $(LDFLAGS) -o $@
	$(SZ) $@

$(BUILD_DIR)/%.dasm: $(BUILD_DIR)/%.elf | $(BUILD_DIR)
	$(DP) -D -S $< > $@

$(BUILD_DIR)/%.hex: $(BUILD_DIR)/%.elf | $(BUILD_DIR)
	$(HEX) $< $@

$(BUILD_DIR)/%.bin: $(BUILD_DIR)/%.elf | $(BUILD_DIR)
	$(BIN) $< $@

$(BUILD_DIR):
	mkdir $@

#######################################
# clean up
#######################################
clean:
	-rm -fR $(BUILD_DIR)

#######################################
# upload & debug
#######################################
upload: $(BUILD_DIR)/$(TARGET).elf
	@$(ECHO) "Download and run $<"
	$(GDB) $< -ex "set remotetimeout 240" \
	-ex "target remote $(GDBREMOTE)" \
	$(GDB_UPLOAD_ARGS) $(GDB_UPLOAD_CMDS)

debug: $(BUILD_DIR)/$(TARGET).elf
	@$(ECHO) "Download and debug $<"
	$(GDB) $< -ex "set remotetimeout 240" \
	-ex "target remote $(GDBREMOTE)"

#######################################
# dependencies
#######################################
-include $(wildcard $(BUILD_DIR)/*.d)

# *** EOF ***
