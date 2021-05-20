LITEOSTOPDIR := ../../
LITEOSTOPDIR := $(realpath $(LITEOSTOPDIR))

# Common
C_SOURCES     += $(wildcard $(LITEOSTOPDIR)/kernel/src/*.c) \
                 $(wildcard $(LITEOSTOPDIR)/kernel/src/mm/*.c) \
                 $(wildcard $(LITEOSTOPDIR)/components/cpup/*.c) \
                 $(wildcard $(LITEOSTOPDIR)/../../third_party/bounds_checking_function/src/*.c) \
                 $(wildcard $(LITEOSTOPDIR)/utils/*.c) \
                 $(wildcard $(LITEOSTOPDIR)/kal/cmsis/*.c) \
                 $(wildcard $(LITEOSTOPDIR)/kal/posix/src/*.c) \
                 Core/Src/task_sample.c

C_INCLUDES    += -I$(LITEOSTOPDIR)/utils \
                 -I$(LITEOSTOPDIR)/kal/cmsis \
                 -I$(LITEOSTOPDIR)/kal/posix/include \
                 -I$(LITEOSTOPDIR)/kernel/include \
                 -I$(LITEOSTOPDIR)/components/cpup \
                 -I$(LITEOSTOPDIR)/../../third_party/bounds_checking_function/include

# Related to arch
ASM_SOURCES   += $(wildcard $(LITEOSTOPDIR)/kernel/arch/arm/cortex-m7/gcc/*.s)

ASMS_SOURCES  += $(wildcard $(LITEOSTOPDIR)/kernel/arch/arm/cortex-m7/gcc/*.S)

C_SOURCES     += $(wildcard $(LITEOSTOPDIR)/kernel/arch/arm/cortex-m7/gcc/*.c)

C_INCLUDES    += -I. \
                 -I$(LITEOSTOPDIR)/kernel/arch/include \
                 -I$(LITEOSTOPDIR)/kernel/arch/arm/cortex-m7/gcc

CFLAGS        += -nostdinc -nostdlib

# list of ASM .S program objects
OBJECTS += $(addprefix $(BUILD_DIR)/,$(notdir $(ASMS_SOURCES:.S=.o)))
vpath %.S $(sort $(dir $(ASMS_SOURCES)))

$(BUILD_DIR)/%.o: %.S Makefile | $(BUILD_DIR)
	$(CC) -c $(CFLAGS) $< -o $@
