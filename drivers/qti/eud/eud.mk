#
# Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.
#
# SPDX-License-Identifier: BSD-3-Clause
#
# Embedded USB Debugger: put EUD in debug mode from BL2, so the SoC can be
# debugged over USB from the first TF-A stage. On by default in debug builds.
#

QTI_EUD_ENABLE		?=	${DEBUG}
$(eval $(call assert_boolean,QTI_EUD_ENABLE))

ifeq (${QTI_EUD_ENABLE},1)
$(eval $(call add_define,QTI_EUD_ENABLE))
BL2_SOURCES		+=	drivers/qti/eud/eud.c
endif
