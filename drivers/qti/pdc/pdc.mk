#
# Copyright (c) 2026 Qualcomm Technologies, Inc. and/or its subsidiaries.
#
# SPDX-License-Identifier: BSD-3-Clause
#
# PDC (Power Domain Controller) driver
#

$(eval $(call add_define,QTI_PDC_ENABLED))

PDC_DRV_PATH := drivers/qti/pdc

# SoCs whose apps PDC matches another SoC's select that SoC's tables.
PDC_CHIPSET ?= $(CHIPSET)

PLAT_INCLUDES += \
	-I$(PDC_DRV_PATH)/$(PDC_CHIPSET)

BL31_SOURCES += \
	$(PDC_DRV_PATH)/pdc.c					\
	$(PDC_DRV_PATH)/pdc_seq.c				\
	$(PDC_DRV_PATH)/pdc_tcs.c				\
	$(PDC_DRV_PATH)/$(PDC_CHIPSET)/pdc_seq_cfg.c		\
	$(PDC_DRV_PATH)/$(PDC_CHIPSET)/interrupt_table.c	\
	$(PDC_DRV_PATH)/$(PDC_CHIPSET)/gpio_table.c		\
	$(PDC_DRV_PATH)/$(PDC_CHIPSET)/tcs_resource.c
