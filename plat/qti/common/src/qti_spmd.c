/*
 * Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include <common/bl_common.h>

#include <platform_def.h>
#include <qti_interrupt_svc.h>
#include <qti_plat.h>

/*
 * SPMD group-0 interrupt handler. SPMD calls this when a Group0 (EL3)
 * interrupt fires while the normal world is running. Forward it through
 * the QTI ISR dispatch table so registered qti_interrupt_svc callbacks
 * still fire correctly. Return 0 to tell SPMD the interrupt was handled;
 * SPMD owns the EOI and will call plat_ic_end_of_interrupt() itself.
 */
int plat_spmd_handle_group0_interrupt(uint32_t intid)
{
	qti_interrupt_svc_dispatch(intid, NULL);
	return 0;
}

/*
 * When SPD=spmd, the SPMD framework registers INTR_TYPE_EL3 itself and
 * dispatches group-0 interrupts via plat_spmd_handle_group0_interrupt()
 * above, so QTI has no EL3 owner of its own to register here.
 */
void qti_interrupt_svc_register_el3_owner(uint64_t flags)
{
	(void)flags;
}

#ifdef QTI_SPMC_MANIFEST_IN_BL31
extern const uint8_t qti_spmc_manifest[];
extern const uint8_t qti_spmc_manifest_end[];
#endif

/*
 * A loader other than TF-A BL2 (U-Boot SPL) loads BL31 and BL32 but not
 * TOS_FW_CONFIG, so BL32 arrives without the SPMC core manifest the SPMD
 * needs. Place the copy built into BL31 where BL2 would have loaded it.
 * The copy is made with the data cache off, so it is in memory when the
 * SPMD and the SPMC map it.
 */
void qti_bl32_ep_setup(entry_point_info_t *bl32_ep)
{
#ifdef QTI_SPMC_MANIFEST_IN_BL31
	if ((bl32_ep->pc == 0U) || (bl32_ep->args.arg0 != 0U)) {
		return;
	}

	memcpy((void *)TOS_FW_CONFIG_BASE, qti_spmc_manifest,
	       (size_t)(qti_spmc_manifest_end - qti_spmc_manifest));
	bl32_ep->args.arg0 = TOS_FW_CONFIG_BASE;
#else
	(void)bl32_ep;
#endif
}
