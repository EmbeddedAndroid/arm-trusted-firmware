#
# Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.
#
# SPDX-License-Identifier: BSD-3-Clause
#

PLAT_DRIVERS_PATH		:=	drivers/qti
PLAT_DRIVERS_INCLUDE_PATH	:= 	include/drivers/qti

PLAT_INCLUDES +=	-I$(PLAT_DRIVERS_PATH)/accesscontrol \
			-I$(PLAT_DRIVERS_INCLUDE_PATH)/accesscontrol \

# Set to 1 to have BL2 lock memory before it reads the FIP. The platform
# then implements the qti_accesscontrol_bl2_*() hooks.
QTI_BL2_ACCESS_CONTROL		?=	0
$(call assert_boolean,QTI_BL2_ACCESS_CONTROL)
$(eval $(call add_define,QTI_BL2_ACCESS_CONTROL))

## Access control
BL31_SOURCES +=	$(PLAT_DRIVERS_PATH)/accesscontrol/access_control.c \

## VMIDMT
PLAT_INCLUDES +=	-I$(PLAT_DRIVERS_PATH)/accesscontrol/vmidmt \
			-I$(PLAT_DRIVERS_PATH)/accesscontrol/vmidmt/${CHIPSET} \

BL31_SOURCES += 	$(PLAT_DRIVERS_PATH)/accesscontrol/vmidmt/vmidmt.c \
			$(PLAT_DRIVERS_PATH)/accesscontrol/vmidmt/vmidmt_hal.c \
			$(PLAT_DRIVERS_PATH)/accesscontrol/vmidmt/${CHIPSET}/vmidmt_static_config.c

## XPU
PLAT_INCLUDES +=	-I$(PLAT_DRIVERS_PATH)/accesscontrol/xpu \
			-I$(PLAT_DRIVERS_PATH)/accesscontrol/xpu/${CHIPSET}

BL31_SOURCES += 	$(PLAT_DRIVERS_PATH)/accesscontrol/xpu/xpu3.c \
			$(PLAT_DRIVERS_PATH)/accesscontrol/xpu/${CHIPSET}/xpu_static_config.c \
			$(PLAT_DRIVERS_PATH)/accesscontrol/xpu/${CHIPSET}/xpu_target_info.c
