#
# Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.
#
# SPDX-License-Identifier: BSD-3-Clause
#
# PMIC / SPMI ARB driver
#

# QTI_PMIC_ARB_VERSION selects which drivers/qti/pmic/<version>/ backend this
# platform builds against. Must be set explicitly by the platform's own
# platform.mk (e.g. pmicarb5, pmicarb7) -- as more chipsets are added, each
# brings its own PMIC arbiter version, so there is no safe default to fall
# back to.
ifndef QTI_PMIC_ARB_VERSION
$(error QTI_PMIC_ARB_VERSION must be set by platform.mk, e.g. pmicarb5 or pmicarb7)
endif

# QTI_PON_PBS_VERSION selects the PON-peripheral register layout (pm_pon.h vs.
# pm_pon5.h) this platform builds against, since PON_PS_HOLD_RESET_CTL/CTL2
# differ across PMIC generations. Set from the platform's own platform.mk;
# defaults to the PBS-based layout (pm_pon.h).
QTI_PON_PBS_VERSION	?=	pon

ifeq (${QTI_PON_PBS_VERSION},pon5)
$(eval $(call add_define,QTI_PON5))
endif

QTI_PMIC_SOURCES	:=	drivers/qti/pmic/spmi_arb_hal.c				\
				drivers/qti/pmic/pm_pon.c

PLAT_INCLUDES		+=	-Iinclude/drivers/qti/pmic				\
				-Iinclude/drivers/qti/pmic/${QTI_PMIC_ARB_VERSION}

BL31_SOURCES		+=	${QTI_PMIC_SOURCES}
