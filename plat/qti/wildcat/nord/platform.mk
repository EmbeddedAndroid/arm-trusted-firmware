#
# Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.
#
# SPDX-License-Identifier: BSD-3-Clause
#
# Makefile for the Nord (SA8797P) QTI platform.

# NORD_XBL_BL31=1 builds the BL31 that XBL enters directly with bl_params
# (xblbl31/xblbl31.mk); by default TF-A BL2 and BL31 boot natively (nord.mk).
NORD_XBL_BL31 ?= 0
$(eval $(call assert_boolean,NORD_XBL_BL31))

ifeq ($(NORD_XBL_BL31),1)
include plat/qti/wildcat/nord/xblbl31/xblbl31.mk
else
include plat/qti/wildcat/nord/nord.mk
endif
