#
# Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.
#
# SPDX-License-Identifier: BSD-3-Clause
#
# Park cores turned off with CPU_OFF in BL31, for SoCs where nothing
# completes the power-down request of a core turned off.
#

# The pen does not maintain the PMF timestamps stored before a core parks.
ifeq (${ENABLE_RUNTIME_INSTRUMENTATION},1)
$(error QTI_CPU_OFF_HOLD does not support ENABLE_RUNTIME_INSTRUMENTATION=1)
endif

$(eval $(call add_define,QTI_CPU_OFF_HOLD))

BL31_SOURCES	+=	$(PLAT_PATH)/hoya/common/hoya_cpu_hold.c		\
			$(PLAT_PATH)/hoya/common/$(ARCH)/hoya_cpu_hold_park.S
