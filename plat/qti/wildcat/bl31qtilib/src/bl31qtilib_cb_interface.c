/*
 * Copyright (c) 2026 Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <errno.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include <arch.h>
#include <arch_helpers.h>
#include <bl31/bl31.h>
#include <bl31/interrupt_mgmt.h>
#include <drivers/arm/gicv3.h>
#include <drivers/delay_timer.h>
#include <drivers/qti/smem/smem.h>
#include <lib/spinlock.h>
#include <lib/xlat_tables/xlat_tables_v2.h>
#include <platform.h>

#include <bl31qtilib_cb_interface.h>
#include <qti_interrupt_svc.h>
#include <wildcat_plat.h>

#define ALL_BITS_SET(val, mask)	(((val) & (mask)) == (mask))

/*
 * Ensure that the BL31QTILIB spinlock layout matches spinlock_t, as
 * bl31qtilib_cb_spin_* casts it to spinlock_t.
 */
_Static_assert(sizeof(struct bl31qtilib_cb_spinlock) == sizeof(spinlock_t),
	       "BL31QTILIB spinlock layout mismatch");
_Static_assert(offsetof(struct bl31qtilib_cb_spinlock, lock) ==
		       offsetof(spinlock_t, lock),
	       "BL31QTILIB spinlock layout mismatch");

int bl31qtilib_cb_spin_lock_init(struct bl31qtilib_cb_spinlock *lock)
{
	if (lock == NULL) {
		return 1;
	}
	memset(lock, 0, sizeof(struct bl31qtilib_cb_spinlock));
	return 0;
}

int bl31qtilib_cb_spin_lock(struct bl31qtilib_cb_spinlock *lock)
{
	if (lock == NULL) {
		return 1;
	}
	spin_lock((spinlock_t *)lock);
	return 0;
}

int bl31qtilib_cb_spin_unlock(struct bl31qtilib_cb_spinlock *lock)
{
	if (lock == NULL) {
		return 1;
	}
	spin_unlock((spinlock_t *)lock);
	return 0;
}

unsigned int bl31qtilib_cb_plat_my_core_pos(void)
{
	return plat_my_core_pos();
}

unsigned int bl31qtilib_cb_plat_core_pos_by_mpidr(uintptr_t mpidr)
{
	return (unsigned int)plat_core_pos_by_mpidr((u_register_t)mpidr);
}

unsigned int bl31qtilib_cb_plat_my_cluster_pos(void)
{
	return plat_qti_my_cluster_pos();
}

void bl31qtilib_cb_udelay(uint32_t usec)
{
	udelay(usec);
}

/*
 * Ensure that the BL31QTILIB timer operations layout matches timer_ops_t, as
 * bl31qtilib_cb_timer_init casts it to timer_ops_t.
 */
_Static_assert(sizeof(struct bl31qtilib_timer_ops) == sizeof(timer_ops_t),
	       "BL31QTILIB timer operations layout mismatch");
_Static_assert(offsetof(struct bl31qtilib_timer_ops, get_timer_value) ==
		       offsetof(timer_ops_t, get_timer_value),
	       "BL31QTILIB timer operations layout mismatch");
_Static_assert(offsetof(struct bl31qtilib_timer_ops, clk_mult) ==
		       offsetof(timer_ops_t, clk_mult),
	       "BL31QTILIB timer operations layout mismatch");
_Static_assert(offsetof(struct bl31qtilib_timer_ops, clk_div) ==
		       offsetof(timer_ops_t, clk_div),
	       "BL31QTILIB timer operations layout mismatch");
_Static_assert(offsetof(struct bl31qtilib_timer_ops, timeout_init_us) ==
		       offsetof(timer_ops_t, timeout_init_us),
	       "BL31QTILIB timer operations layout mismatch");
_Static_assert(offsetof(struct bl31qtilib_timer_ops, timeout_elapsed) ==
		       offsetof(timer_ops_t, timeout_elapsed),
	       "BL31QTILIB timer operations layout mismatch");

void bl31qtilib_cb_timer_init(const struct bl31qtilib_timer_ops *ops_ptr)
{
	timer_init((timer_ops_t *)ops_ptr);
}

uint64_t bl31qtilib_cb_read_cntfrq_el0(void)
{
	return read_cntfrq_el0();
}

uint64_t bl31qtilib_cb_read_cntpct_el0(void)
{
	return read_cntpct_el0();
}

unsigned int bl31qtilib_cb_plat_get_syscnt_freq2(void)
{
	return plat_get_syscnt_freq2();
}

void bl31qtilib_cb_enable_cpu_timer(void)
{
	write_cntps_ctl_el1(0x1);
}

void bl31qtilib_cb_disable_cpu_timer(void)
{
	write_cntps_ctl_el1(0x0);
}

void bl31qtilib_cb_write_cntps_cval_el1(uint64_t ticks)
{
	write_cntps_cval_el1(ticks);
}

void bl31qtilib_cb_write_cntfrq_el0(uint32_t cntfrq)
{
	write_cntfrq_el0(cntfrq);
}

int bl31qtilib_cb_mmap_dynamic_region(uintptr_t base_va, uintptr_t base_pa,
				      size_t size, uint8_t attr)
{
	unsigned int l_attr = 0;

	(void)base_pa;

	/*
	 * BL31QTILIB always passes VA == PA; base_pa is ignored. Revisit if
	 * non-identity mappings are ever added.
	 */
	switch (attr) {
	case BL31QTILIB_UNMAP:
		return qti_mmap_remove_dynamic_region(base_va, size);

	case BL31QTILIB_MAP_NS_RW_NC_ISH_XN:
		l_attr = MT_NS | MT_RW | MT_NON_CACHEABLE |
			 MT_SHAREABILITY_ISH | MT_EXECUTE_NEVER;
		break;

	case BL31QTILIB_MAP_S_RW_WBWA_ISH_XN:
		l_attr = MT_SECURE | MT_RW | MT_MEMORY | MT_SHAREABILITY_ISH |
			 MT_EXECUTE_NEVER;
		break;

	case BL31QTILIB_MAP_S_RW_DEVICE_ISH_XN:
		l_attr = MT_SECURE | MT_RW | MT_DEVICE | MT_SHAREABILITY_ISH |
			 MT_EXECUTE_NEVER;
		break;

	default:
		return -1;
	}

	return qti_mmap_add_dynamic_region(base_va, size, l_attr);
}

int bl31qtilib_cb_smem_lookup(uint16_t item, void **item_ptr,
				      size_t *item_size)
{
	if (item_ptr != NULL) {
		*item_ptr = NULL;
	}
	if (item_size != NULL) {
		*item_size = 0U;
	}

	return qti_smem_lookup(QTI_SMEM_HOST_COMMON, item,
			       QTI_SMEM_FLAG_NONE, item_ptr, item_size);
}

uintptr_t bl31qtilib_cb_get_qtee_entry_point(void)
{
	entry_point_info_t *ep_info = bl31_plat_get_next_image_ep_info(SECURE);

	if (ep_info != NULL) {
		return ep_info->pc;
	}

	WARN("QTEE entry point not provided by BL2 boot loader\n");
	return 0;
}

int bl31qtilib_cb_int_register_isr(uint32_t intnum, const char *int_desc,
				   void *(*fn)(void *), void *ctx,
				   uint32_t flags, bool enable)
{
	uint32_t target;
	int ret;

	ret = qti_register_intr_type_el3(intnum, fn, ctx);
	if (ret != 0) {
		ERROR("Failed to register EL3 interrupt %d, %s", intnum,
		      int_desc);
		goto bail;
	}

	if (ALL_BITS_SET(flags, BL31QTILIB_INTR_ROUTING_MODE_ALL)) {
		target = BL31QTILIB_INTR_ROUTING_MODE_ALL;
	} else if (ALL_BITS_SET(flags, BL31QTILIB_INTR_ROUTING_MODE_SELF)) {
		target = BL31QTILIB_INTR_ROUTING_MODE_SELF;
	} else {
		/* Preserve the existing default for callers with no mode bits. */
		target = BL31QTILIB_INTR_ROUTING_MODE_ALL;
	}

	ret = bl31qtilib_cb_set_int_targets(intnum, target);
	if (ret != 0) {
		ERROR("Failed to set EL3 interrupt target %d, %s, 0x%x", intnum,
		      int_desc, target);
		goto bail;
	}

	if (enable) {
		plat_ic_enable_interrupt(intnum);
	} else {
		plat_ic_disable_interrupt(intnum);
	}

bail:
	return ret;
}

int bl31qtilib_cb_int_register_fatal_isr(uint32_t intnum, const char *int_desc,
					 void *(*fn)(void *), void *ctx,
					 uint32_t flags, bool enable)
{
	return bl31qtilib_cb_int_register_isr(intnum, int_desc, fn, ctx,
					      flags, enable);
}

void bl31qtilib_cb_int_unregister_isr(uint32_t intnum, uint32_t flags)
{
	(void)flags;
	qti_unregister_intr_type_el3(intnum);
}

int bl31qtilib_cb_int_disable_fatal_isr(uint32_t intnum)
{
	return bl31qtilib_cb_int_disable(intnum);
}

int bl31qtilib_cb_int_disable(uint32_t intnum)
{
	plat_ic_disable_interrupt(intnum);
	return 0;
}

int bl31qtilib_cb_int_enable(uint32_t intnum)
{
	plat_ic_enable_interrupt(intnum);
	return 0;
}

int bl31qtilib_cb_set_int_targets(uint32_t intnum, uint32_t target)
{
	uint32_t irm = 0U;
	uint32_t mpidr = 0U;
	int ret = 0;

	if (!IS_SPI(intnum)) {
		ret = 0;
		goto bail;
	}

	if (target == BL31QTILIB_INTR_ROUTING_MODE_ALL) {
		irm = INTR_ROUTING_MODE_ANY;
		/* mpidr_el1 is ignored for this routing model */
	} else {
		irm = INTR_ROUTING_MODE_PE;

		if (target == BL31QTILIB_INTR_ROUTING_MODE_SELF) {
			mpidr = read_mpidr_el1();
		} else {
			mpidr = plat_qti_logical_cpu_num_to_mpidr(target);

			if (mpidr == QTI_INVALID_MPID_ID) {
				ret = -EINVAL;
				goto bail;
			}
		}
	}

	plat_ic_set_spi_routing(intnum, irm, mpidr);

bail:
	return ret;
}

/*
 * This function is used to check if the image is authenticated.
 * It is called from bl31_main() in bl31_entrypoint.S.
 * The function returns true if the image is authenticated, false otherwise.
 * The entrypoint is returned in the entrypoint parameter.
 * This implementation will return hardcoded address for COREBOOT enabled
 * targets.
 * This is needed because the entrypoint address is not available from the
 * bootloader in COREBOOT configurations.
 */
bool bl31qtilib_cb_is_image_authenticated(uint32_t image_id,
					  uint64_t *entrypoint)
{
	uint64_t entry_point;

	switch (image_id) {
	case TZ_AC_CONFIG_SW_TYPE:
		entry_point = TZ_TZ_AC_CONFIG_BASE_ADDR;
		break;

	case HYP_AC_CONFIG_SW_TYPE:
		entry_point = TZ_HYP_AC_CONFIG_BASE_ADDR;
		break;

	default:
		return false;
	}

	if (entrypoint != NULL) {
		*entrypoint = entry_point;
	}

	return true;
}

void bl31qtilib_cb_error_fatal(int error_code)
{
	plat_error_handler(error_code);
}

void bl31qtilib_cb_set_error_fatal(int error_code)
{
	plat_error_handler(error_code);
}

void bl31qtilib_cb_set_error_fatal_with_cond(int error_code,
					     bool return_to_hlos)
{
	(void)return_to_hlos;

	plat_error_handler(error_code);
}

uint32_t bl31qtilib_cb_dispatch_sdei_event(void)
{
	ERROR("SDEI not enabled\n");
	panic();
}

int bl31qtilib_cb_ns_va_to_pa(uintptr_t va, unsigned int client_mode,
			      uintptr_t *pa_out)
{
	return qti_ns_va_to_pa(va, client_mode, pa_out);
}
