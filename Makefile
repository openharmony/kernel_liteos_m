# Copyright (c) 2013-2019 Huawei Technologies Co., Ltd. All rights reserved.
# Copyright (c) 2020-2021 Huawei Device Co., Ltd. All rights reserved.
#
# Redistribution and use in source and binary forms, with or without modification,
# are permitted provided that the following conditions are met:
#
# 1. Redistributions of source code must retain the above copyright notice, this list of
#    conditions and the following disclaimer.
#
# 2. Redistributions in binary form must reproduce the above copyright notice, this list
#    of conditions and the following disclaimer in the documentation and/or other materials
#    provided with the distribution.
#
# 3. Neither the name of the copyright holder nor the names of its contributors may be used
#    to endorse or promote products derived from this software without specific prior written
#    permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
# "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
# THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
# PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR
# CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
# EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
# PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
# OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
# WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
# OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
# ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

LITEOSTOPDIR := $(realpath $(dir $(lastword $(MAKEFILE_LIST))))
export LITEOSTOPDIR

LITEOS_TARGET = liteos
HIDE := @
KCONFIG_CMDS := $(notdir $(wildcard $(dir $(shell which menuconfig))*config))

ohos_kernel ?= liteos_m
$(foreach line,$(shell hb env | sed 's/\[OHOS INFO\]/ohos/g;s/ /_/g;s/:_/=/g' || true),$(eval $(line)))
ifneq ($(ohos_kernel),liteos_m)
$(error The selected product ($(ohos_product)) is not a liteos_m kernel type product)
endif

ifeq ($(PRODUCT_PATH),)
PRODUCT_PATH:=$(ohos_product_path)
endif

ifeq ($(DEVICE_PATH),)
DEVICE_PATH:=$(ohos_device_path)
endif

ifeq ($(BOARD_COMPANY),)
BOARD_COMPANY:=$(ohos_device_company)
endif

ifeq ($(TEE:1=y),y)
tee = _tee
endif
ifeq ($(RELEASE:1=y),y)
CONFIG ?= $(PRODUCT_PATH)/kernel_configs/release$(tee).config
else
CONFIG ?= $(PRODUCT_PATH)/kernel_configs/debug$(tee).config
endif

KCONFIG_CONFIG ?= $(CONFIG)
LITEOS_MENUCONFIG_H ?= $(LITEOSTOPDIR)/config.h
LITEOS_CONFIG_FILE ?= $(LITEOSTOPDIR)/.config

# export los_config.mk related environment variables
export LITEOS_MENUCONFIG_H
export LITEOS_CONFIG_FILE
export BOARD_COMPANY
export DEVICE_PATH
export PRODUCT_PATH

# export kconfig related environment variables
export CONFIG_=LOSCFG_
export srctree=$(LITEOSTOPDIR)

-include $(LITEOS_CONFIG_FILE)

define HELP =
Usage: make [TARGET]... [PARAMETER=VALUE]...

Targets:
    help:       display this help and exit
    clean:      clean compiled objects
    cleanall:   clean all build outputs
    all:        do build (Default target)
    update_config:  update product kernel config (use menuconfig)
    xxconfig:   invoke xxconfig command of kconfiglib (xxconfig is one of $(KCONFIG_CMDS))

Parameters:
    TEE:        boolean value(1 or y for true), enable tee
    RELEASE:    boolean value(1 or y for true), build release version
    CONFIG:     kernel config file to be use
    args:       arguments for xxconfig command
endef
export HELP

all:
	$(HIDE)hb build -f --gn-args "liteos_kernel_only=true liteos_name=\"$(LITEOS_TARGET)\""

help:
	$(HIDE)echo "$$HELP"

$(filter-out menuconfig,$(KCONFIG_CMDS)):
	$(HIDE)$@ $(args)

$(LITEOS_CONFIG_FILE): $(KCONFIG_CONFIG)
	$(HIDE)env KCONFIG_CONFIG=$< genconfig --config-out $@ --header-path $(LITEOS_MENUCONFIG_H)

update_config menuconfig:
	$(HIDE)test -f "$(CONFIG)" && cp -v "$(CONFIG)" .config && menuconfig $(args) && savedefconfig --out "$(CONFIG)"

clean:
	$(HIDE)hb clean
	$(HIDE)echo "clean $(LOSCFG_PLATFORM) finish"

cleanall: clean
	$(HIDE)echo "clean all done"

.PHONY: all clean cleanall help update_config $(KCONFIG_CMDS) $(KCONFIG_CONFIG)
