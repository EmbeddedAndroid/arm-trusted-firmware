#
# Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.
#
# SPDX-License-Identifier: BSD-3-Clause
#

# Makefile for the X1P42100 (Purwa): an X1E80100 with two of its three Oryon
# clusters, booted by the same boot firmware, built from the Hamoa sources.

HAMOA_CLUSTER_COUNT			:=	2

include plat/qti/wildcat/hamoa/hamoa.mk
