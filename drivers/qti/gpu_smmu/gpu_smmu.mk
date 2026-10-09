#
# Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.
#
# SPDX-License-Identifier: BSD-3-Clause
#
# GPU SMMU aperture setup, for chipsets without a BL31 clock driver
#

$(eval $(call add_define,QTI_GPU_SMMU_ENABLED))

BL31_SOURCES += drivers/qti/gpu_smmu/$(CHIPSET)/gpu_smmu.c
