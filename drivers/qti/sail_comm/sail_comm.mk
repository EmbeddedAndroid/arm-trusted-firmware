#
# Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.
#
# SPDX-License-Identifier: BSD-3-Clause
#

SAIL_COMM_DRV_PATH := drivers/qti/sail_comm

$(eval $(call add_define,QTI_SAIL_COMM_ENABLE))

BL31_SOURCES += \
	$(SAIL_COMM_DRV_PATH)/sail_comm.c
