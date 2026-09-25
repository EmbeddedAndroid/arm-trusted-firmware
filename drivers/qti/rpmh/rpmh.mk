#
# Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.
#
# SPDX-License-Identifier: BSD-3-Clause
#
# RPMh (Resource Power Manager hardened) command service driver
#

$(eval $(call add_define,QTI_RPMH_ENABLED))

RPMH_DRV_PATH := drivers/qti/rpmh

# SoCs whose apps RSC and AOP message RAM match another SoC's use its data.
RPMH_CHIPSET ?= $(CHIPSET)

PLAT_INCLUDES += \
	-I$(RPMH_DRV_PATH) \
	-I$(RPMH_DRV_PATH)/$(RPMH_CHIPSET)

BL31_SOURCES += \
	$(RPMH_DRV_PATH)/rpmh_client.c
