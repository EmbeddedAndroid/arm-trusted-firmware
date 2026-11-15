#
# Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.
#
# SPDX-License-Identifier: BSD-3-Clause
#
# Makefile for the SecaAU QTI platform, a variant of the Nord platform.

NORD_SECA				:=	1

ifeq ($(NORD_XBL_BL31),1)
$(error NORD_XBL_BL31 is not supported on SecaAU)
endif

include plat/qti/wildcat/nord/nord.mk

include drivers/qti/cmd_db/cmd_db.mk
