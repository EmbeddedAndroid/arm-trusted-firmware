/*
 * Copyright (c) 2018, ARM Limited and Contributors. All rights reserved.
 * Copyright (c) 2018-2020, The Linux Foundation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef QTI_PLAT_H
#define QTI_PLAT_H

#include <stdbool.h>
#include <stdint.h>

#include <common/bl_common.h>
#include <drivers/arm/gicv3.h>
#include <lib/cassert.h>
#include <lib/el3_runtime/cpu_data.h>
#include <lib/psci/psci.h>
#include <lib/xlat_tables/xlat_tables_v2.h>

int qti_mmap_add_dynamic_region(uintptr_t base_pa, size_t size,
				unsigned int attr);
int qti_mmap_remove_dynamic_region(uintptr_t base_va, size_t size);
void qti_setup_page_tables(
			   uintptr_t total_base,
			   size_t total_size,
			   uintptr_t code_start,
			   uintptr_t code_limit,
			   uintptr_t rodata_start,
			   uintptr_t rodata_limit
			  );
int qti_io_setup(void);
struct image_info *qti_get_image_info(unsigned int image_id);

/*
 * SiP CP_SMMU_APERTURE: each 16-bit half of the four @pairs words names a
 * GPU SMMU context bank in its low byte and the aperture slot to point at
 * it in its high byte; a half with a 0xff byte is unused. Returns 0 on
 * success, -EINVAL for a bank or slot that does not exist, without
 * programming any slot, and another negative errno if the slots cannot be
 * programmed. It runs while the OS owns the GPU power and clocks: it must
 * not change them, and fails without programming any slot if what the
 * write needs is off.
 */
bool plat_qti_gpu_smmu_aperture_supported(void);
int plat_qti_gpu_smmu_aperture(const uint32_t pairs[4]);

void plat_qti_gic_driver_init(void);
void plat_qti_gic_init(void);
void plat_qti_gic_cpuif_enable(void);
void plat_qti_gic_cpuif_disable(void);
void plat_qti_gic_pcpu_init(void);
const interrupt_prop_t *plat_qti_get_interrupt_props(unsigned int *num_props);

unsigned int plat_qti_core_pos_by_mpidr(u_register_t mpidr);
unsigned int plat_qti_my_cluster_pos(void);

void gic_set_spi_routing(unsigned int id, unsigned int irm, u_register_t mpidr);

/* PMIC PON_PS_HOLD_RESET_CTL reset types */
#define PON_RESET_TYPE_WARM_RESET		1U
#define PON_RESET_TYPE_SHUTDOWN			4U
#define PON_RESET_TYPE_HARD_RESET		7U

void qti_pmic_prepare_reset(void);
void qti_pmic_prepare_shutdown(void);
void qti_pmic_route_pon_irqs(void);

int plat_qti_pwr_domain_on(u_register_t mpidr, int core_pos);
void plat_qti_pwr_domain_on_finish(int core_pos, const uint8_t *states);
void plat_qti_pwr_domain_off(const uint8_t *states);
void plat_qti_pwr_domain_suspend(const uint8_t *states);
void plat_qti_pwr_domain_suspend_finish(const uint8_t *states);
int  plat_qti_pwr_psci_init(uintptr_t warmboot_entry);

/*
 * Called on CPU_OFF and on a suspend that powers the core down, after the
 * backend's own preparation and the GIC CPU interface disable: request the
 * power down from the core itself. DynamIQ cores take the request through
 * CPUPWRCTLR_EL1.CORE_PWRDN_EN, which qti_cpu_pwrdn_en() sets; a backend
 * whose cores have no such register (Cortex-A53 class) leaves the function
 * empty.
 */
void plat_qti_cpu_pwrdn_request(void);
void qti_cpu_pwrdn_en(void);
void qti_cpu_pwrdn_clear(void);

/*
 * Where nothing completes the power-down request of a core turned off,
 * QTI_CPU_OFF_HOLD parks such cores in BL31 instead (hoya_cpu_hold.c):
 * plat_qti_cpu_hold_retire() clears CORE_PWRDN_EN and returns true for a
 * core that will park, which then raises no power-down request.
 */
#ifdef QTI_CPU_OFF_HOLD
bool plat_qti_cpu_hold_retire(unsigned int core_pos);
void plat_qti_cpu_hold_suspend(unsigned int core_pos);
bool plat_qti_cpu_hold_release(unsigned int core_pos);
void plat_qti_pwr_domain_pwr_down(const psci_power_state_t *target_state);
#define PLAT_QTI_PWR_DOMAIN_PWR_DOWN	plat_qti_pwr_domain_pwr_down
#else
static inline bool plat_qti_cpu_hold_retire(unsigned int core_pos)
{
	(void)core_pos;
	return false;
}

static inline void plat_qti_cpu_hold_suspend(unsigned int core_pos)
{
	(void)core_pos;
}

static inline bool plat_qti_cpu_hold_release(unsigned int core_pos)
{
	(void)core_pos;
	return false;
}
#define PLAT_QTI_PWR_DOMAIN_PWR_DOWN	NULL
#endif

void plat_qti_bl31_setup_post(void);
void plat_qti_invoke_unhandled_isr(uint32_t id, void *handle);

/*
 * Composite PSCI power-state encoding helpers, shared by the common PSCI layer
 * and the per-platform PM backends that define plat_qti_pm_idle_states(). The
 * users of these macros must include <lib/psci/psci.h> for the PSTATE_* shifts.
 */
#define QTI_LOCAL_PSTATE_WIDTH		4

/* Make composite power state parameter till level 0 */
#define qti_make_pwrstate_lvl0(lvl0_state, type) \
		(((lvl0_state) << PSTATE_ID_SHIFT) | ((type) << PSTATE_TYPE_SHIFT))

/* Make composite power state parameter till level 1 */
#define qti_make_pwrstate_lvl1(lvl1_state, lvl0_state, type) \
		(((lvl1_state) << QTI_LOCAL_PSTATE_WIDTH) | \
		qti_make_pwrstate_lvl0(lvl0_state, type))

/* Make composite power state parameter till level 2 */
#define qti_make_pwrstate_lvl2(lvl2_state, lvl1_state, lvl0_state, type) \
		(((lvl2_state) << (QTI_LOCAL_PSTATE_WIDTH * 2)) | \
		qti_make_pwrstate_lvl1(lvl1_state, lvl0_state, type))

/* Make composite power state parameter till level 3 */
#define qti_make_pwrstate_lvl3(lvl3_state, lvl2_state, lvl1_state, lvl0_state, type) \
		(((lvl3_state) << (QTI_LOCAL_PSTATE_WIDTH * 3)) | \
		qti_make_pwrstate_lvl2(lvl2_state, lvl1_state, lvl0_state, type))

/*
 * Backend-provided, 0-terminated array of the composite idle power states this
 * platform supports (deepest state last, immediately before the terminator).
 * Defined by the platform PM backend: e.g. hoya_pm.c advertises WFI-based CPU
 * standby only, while qtiseclib_pm.c advertises the firmware power-down ladder.
 */
const unsigned int *plat_qti_pm_idle_states(void);

typedef struct chip_id_info {
	uint16_t jtag_id;
	uint16_t chipinfo_id;
} chip_id_info_t;

#endif /* QTI_PLAT_H */
