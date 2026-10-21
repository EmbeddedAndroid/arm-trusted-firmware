#
# Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.
#
# SPDX-License-Identifier: BSD-3-Clause
#

$(eval $(call add_define,QTI_SMMU_ENABLED))

PLAT_DRIVERS_PATH :=	drivers/qti
BL31_SOURCES	+=	$(PLAT_DRIVERS_PATH)/smmu/$(CHIPSET)/smmu_cfg.c	\
			$(PLAT_DRIVERS_PATH)/smmu/smmu.c

# Program GPU SMMU registers that sit in the GPU CX domain, powering it for
# the purpose, on chipsets without a BL31 clock driver.
ifeq (${QTI_SMMU_GPU_CX},1)
$(eval $(call add_define,QTI_SMMU_GPU_CX))
BL31_SOURCES	+=	$(PLAT_DRIVERS_PATH)/smmu/smmu_gpu.c
endif
