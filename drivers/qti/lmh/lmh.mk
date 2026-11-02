#
# Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.
#
# SPDX-License-Identifier: BSD-3-Clause
#
# CPU limits management (LMh) driver
#

$(eval $(call add_define,QTI_LMH_ENABLED))

LMH_DRV_PATH := drivers/qti/lmh

BL31_SOURCES += \
	$(LMH_DRV_PATH)/lmh.c				\
	$(LMH_DRV_PATH)/$(CHIPSET)/lmh_config.c
