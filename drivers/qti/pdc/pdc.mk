#
# Copyright (c) 2026 Qualcomm Technologies, Inc. and/or its subsidiaries.
#
# SPDX-License-Identifier: BSD-3-Clause
#
# PDC (Power Domain Controller) driver
#

$(eval $(call add_define,QTI_PDC_ENABLED))

PDC_DRV_PATH := drivers/qti/pdc

# Every supported apps PDC has the hoya register map. Talos has its own
# interrupt, GPIO and TCS tables; the other chipsets use the hoya tables.
# The sequences differ per chipset.
PDC_COMMON := $(PDC_DRV_PATH)/hoya
ifneq ($(filter lemans monaco,$(CHIPSET)),)
PDC_SEQ_CFG := $(PDC_COMMON)/pdc_seq_cfg.c
else
PDC_SEQ_CFG := $(PDC_DRV_PATH)/$(CHIPSET)/pdc_seq_cfg.c
endif
ifneq ($(filter talos,$(CHIPSET)),)
PDC_TABLES := $(PDC_DRV_PATH)/$(CHIPSET)
else
PDC_TABLES := $(PDC_COMMON)
endif

# A chipset whose PDC departs from the defaults below says so in the
# pdc_config.mk of its tables.
-include $(PDC_TABLES)/pdc_config.mk

# Set to 0 where the AOP programs the apps PDC sleep and wake TCSes.
PDC_ENABLE_TCS ?= 1
# Set to 1 where the interrupt owners are banked bitmasks with a single
# non-zero owner DRV.
QTI_PDC_IRQ_OWNER_BANKED ?= 0

$(eval $(call assert_booleans,PDC_ENABLE_TCS QTI_PDC_IRQ_OWNER_BANKED))
$(eval $(call add_defines,PDC_ENABLE_TCS QTI_PDC_IRQ_OWNER_BANKED))

PLAT_INCLUDES += \
	-I$(PDC_COMMON)

BL31_SOURCES += \
	$(PDC_DRV_PATH)/pdc.c					\
	$(PDC_DRV_PATH)/pdc_seq.c				\
	$(PDC_SEQ_CFG)						\
	$(PDC_TABLES)/interrupt_table.c				\
	$(PDC_TABLES)/gpio_table.c

ifeq ($(PDC_ENABLE_TCS),1)
BL31_SOURCES += \
	$(PDC_DRV_PATH)/pdc_tcs.c				\
	$(PDC_TABLES)/tcs_resource.c
endif
