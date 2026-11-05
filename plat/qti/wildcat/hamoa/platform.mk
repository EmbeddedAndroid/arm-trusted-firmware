#
# Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.
#
# SPDX-License-Identifier: BSD-3-Clause
#
# Makefile for the Hamoa (X1E80100) QTI platform.

# Point this at a BL31QTILIB to build the BL31 that XBL enters with
# bl_params and that uses the library for power, SiP and debug policy;
# without it, BL2 and BL31 boot natively (hamoa.mk).
BL31QTILIB_PATH ?=

ifeq ($(BL31QTILIB_PATH),)
include plat/qti/wildcat/hamoa/hamoa.mk
else
include plat/qti/wildcat/hamoa/bl31qtilib/bl31qtilib.mk
endif
