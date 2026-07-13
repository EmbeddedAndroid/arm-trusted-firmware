/*
 * Copyright (c) 2026 Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef BL31QTILIB_CB_INTERFACE_H
#define BL31QTILIB_CB_INTERFACE_H

#include "bl31qtilib_defs.h"

int bl31qtilib_cb_spin_lock_init(struct bl31qtilib_cb_spinlock *lock);
int bl31qtilib_cb_spin_lock(struct bl31qtilib_cb_spinlock *lock);
int bl31qtilib_cb_spin_unlock(struct bl31qtilib_cb_spinlock *lock);
unsigned int bl31qtilib_cb_plat_my_core_pos(void);
unsigned int bl31qtilib_cb_plat_core_pos_by_mpidr(uintptr_t mpidr);
unsigned int bl31qtilib_cb_plat_my_cluster_pos(void);

void bl31qtilib_cb_udelay(uint32_t usec);
void bl31qtilib_cb_timer_init(const struct bl31qtilib_timer_ops *ops_ptr);
uint64_t bl31qtilib_cb_read_cntfrq_el0(void);
uint64_t bl31qtilib_cb_read_cntpct_el0(void);
void bl31qtilib_cb_enable_cpu_timer(void);
void bl31qtilib_cb_disable_cpu_timer(void);
void bl31qtilib_cb_write_cntps_cval_el1(uint64_t ticks);
void bl31qtilib_cb_write_cntfrq_el0(uint32_t cntfrq);
unsigned int bl31qtilib_cb_plat_get_syscnt_freq2(void);
int bl31qtilib_cb_mmap_dynamic_region(uintptr_t base_va, uintptr_t base_pa,
				      size_t size, uint8_t attr);
int bl31qtilib_cb_smem_lookup(uint16_t item, void **item_ptr,
				      size_t *item_size);
uintptr_t bl31qtilib_cb_get_qtee_entry_point(void);

int bl31qtilib_cb_int_register_isr(uint32_t intnum, const char *int_desc,
				   void *(*fn)(void *), void *ctx,
				   uint32_t flags, bool enable);
int bl31qtilib_cb_int_register_fatal_isr(uint32_t intnum, const char *int_desc,
					 void *(*fn)(void *), void *ctx,
					 uint32_t flags, bool enable);
void bl31qtilib_cb_int_unregister_isr(uint32_t intnum, uint32_t flags);
int bl31qtilib_cb_int_disable(uint32_t intnum);
int bl31qtilib_cb_int_disable_fatal_isr(uint32_t intnum);
int bl31qtilib_cb_int_enable(uint32_t intnum);
int bl31qtilib_cb_set_int_targets(uint32_t intnum, uint32_t target);

bool bl31qtilib_cb_is_image_authenticated(uint32_t image_id,
					  uint64_t *entrypoint);

void bl31qtilib_cb_error_fatal(int error_code);
void bl31qtilib_cb_set_error_fatal(int error_code);
void bl31qtilib_cb_set_error_fatal_with_cond(int error_code,
					     bool return_to_hlos);

uint32_t bl31qtilib_cb_dispatch_sdei_event(void);

int bl31qtilib_cb_ns_va_to_pa(uintptr_t va, unsigned int client_mode,
			      uintptr_t *pa_out);

#endif /* BL31QTILIB_CB_INTERFACE_H */
