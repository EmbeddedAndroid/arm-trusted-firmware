/*
 * Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdbool.h>
#include <stdint.h>

#include <arch_helpers.h>
#include <bl31/bl31.h>
#include <common/debug.h>
#include <drivers/arm/gicv3.h>
#include <drivers/delay_timer.h>
#include <lib/mmio.h>
#include <lib/psci/psci.h>
#include <lib/spinlock.h>
#include <lib/utils_def.h>
#include <plat/common/platform.h>

#include <platform_def.h>
#include <qti_plat.h>

/* Per-cluster NCC_ARCH registers, one bit (or bit group) per core. */
#define NCC_BOOT_CFG1				0x260U
#define NCC_BOOT_CFG1_CPUFETCHFWBLOCK(c)	BIT_64(16U + (c))
#define NCC_BOOT_CFG1_CORRVBARMASKWAKERCVD(c)	BIT_64(33U + (c))
#define NCC_PWR_CTL1				0x278U
#define NCC_PWR_CTL1_COREPWRUPREQ(c)		BIT_64(c)
#define NCC_PWR_CFG1				0x290U
#define NCC_PWR_CFG1_OK2ENTERC3(c)		BIT_64(c)
#define NCC_PWR_CFG1_OK2ENTERC4(c)		BIT_64(8U + (c))
#define NCC_PWR_CFG1_OK2ENTERCL3		BIT_64(17)
#define NCC_PWR_CFG1_OK2ENTERCL4		BIT_64(18)
#define NCC_PWR_STS1				0x2b0U
#define NCC_PWR_STS1_CORDOMISON(c)		BIT_64(c)
#define NCC_PWR_STS1_CORSPRINITDONE(c)		BIT_64(8U + (c))
#define NCC_RVBAR				0x328U
#define NCC_PWR_CTL3				0x410U
#define NCC_PWR_CTL3_CORFRCCPURVBAR(c)		BIT_64(c)
#define NCC_PWR_CTL3_CORFRCCPURVBAR_ALL		GENMASK_64(7, 0)

#define NCC_POWERUP_POLL			2000000U

/* Per-cluster NCC blocks next to NCC_ARCH, one cluster stride apart. */
#define NCC_GBL_BASE				0x19898000U
/* Cluster LPM configuration in an always-on spare register: CL4 or running. */
#define NCC_GBL_SPARE_AON0			0x268U
#define NCC_GBL_SPARE_AON0_CL4			U(0x2f)
#define NCC_GBL_SPARE_AON0_RUN			U(0x1)
#define NCC_UDBG_BASE				0x19830000U
#define NCC_UDBG_HID34_EL3			0xf8U
#define NCC_HID34_CAGPSML3CGOVRRIDE		BIT_64(11)
#define NCC_MPAM_S_LLC0_BASE			0x19860000U
#define NCC_MPAM_S_LLC3_BASE			0x19863000U
#define NCC_MPAM_NS_LLC0_BASE			0x19870000U
#define NCC_MPAM_NS_LLC3_BASE			0x19873000U
#define NCC_MPAMCFG_IMPL_SLC_SCID		0x0U
#define NCC_MPAMCFG_PART_SEL			0x100U
#define NCC_MPAMCFG_PRI				0x400U
#define NCC_MPAMCFG_PRI_DSPRI			(U(0x3) << 16)
#define NCC_LLCC_SCID_CPU			1U

/*
 * CPUCP secure SCMI channel: the cluster LPM entry and exit are reported to
 * CPUCP with a power domain POWER_STATE_SET for the cluster (domain
 * cluster << 8), state CL3 on entry and CL3 as the previous state on exit.
 */
#define CPUCP_SCMI_SEC_CH			0x18b33000U
#define CPUCP_SCMI_CH_STATUS			0x4U
#define CPUCP_SCMI_CH_STATUS_FREE		BIT_32(0)
#define CPUCP_SCMI_CH_STATUS_ERROR		BIT_32(1)
#define CPUCP_SCMI_CH_FLAGS			0x10U
#define CPUCP_SCMI_CH_LEN			0x14U
#define CPUCP_SCMI_CH_HDR			0x18U
#define CPUCP_SCMI_CH_PAYLOAD			0x1cU
#define CPUCP_DOORBELL				0x17824004U
#define CPUCP_DOORBELL_OSM_IPC			BIT_32(28)
#define CPUCP_SCMI_TIMEOUT_US			10000U
#define SCMI_PWR_DMN_PROTO_ID			0x11U
#define SCMI_PWR_STATE_SET_MSG			0x4U
#define CPUCP_PWR_STATE_CL3			0x13U
#define CPUCP_PWR_STATE_PREV_SHIFT		12U

/* Local state the device tree passes for the cluster state, see below. */
#define NORD_DT_STATE_CL5			5U

#define NORD_CLUSTER_CORES_MASK			((1U << PLAT_CORE_COUNT_PER_CLUSTER) - 1U)

/*
 * Cluster LPM controller: the CL5 sequencer trigger and debug power-up wake
 * of each cluster, set up once before the first CPU_ON, and the APSS power
 * control bits.
 */
#define APSS_CL_LPM_BASE			0x18878000U
#define APSS_CL_LPM_CL5_TRIG_CTRL(cl)		(APSS_CL_LPM_BASE + 0x40U + (4U * (cl)))
#define APSS_CL_LPM_DBG_PWRUP_WAKE_CFG(cl)	(APSS_CL_LPM_BASE + 0x80U + (4U * (cl)))
#define APSS_GICD_PWR_CTL			0x17750800U
#define APSS_GICD_PWR_CTL_BIT2			BIT_32(2)
#define APSS_CPUSS_LPM_CTL			0x188a0088U

static uintptr_t warm_entry;

/*
 * Per cluster: the lock serialises the read-modify-writes of the NCC
 * registers the cores share, the idle mask and the cluster LPM state. The
 * idle mask has a bit for every core of the cluster that is in C4, parked by
 * CPU_OFF or not started yet; the cluster LPM is only armed when it is full.
 */
static spinlock_t cl_lock[PLAT_CLUSTER_COUNT];
static uint32_t cl_idle_mask[PLAT_CLUSTER_COUNT];
static volatile bool cluster_lpm_armed[PLAT_CLUSTER_COUNT];

/* The CPUCP channel; once a transaction fails, the domain states are gone. */
static spinlock_t cpucp_lock;
static volatile bool cpucp_lpm_ok;

/* SCR_EL3 of a core in a multi-level retention state, see below. */
static u_register_t suspend_scr[PLATFORM_CORE_COUNT];

static void core_lpm_exit(unsigned int cluster, unsigned int core);

/*
 * Per core: started is set once the core has run (it is only written and
 * read by CPU_ON callers, with caches on); release is set by CPU_ON for a
 * core parked by CPU_OFF and read by that core with its MMU off.
 */
struct nord_cpu_slot {
	uint32_t started;
	uint32_t release;
} __aligned(CACHE_WRITEBACK_GRANULE);

static struct nord_cpu_slot nord_cpu[PLATFORM_CORE_COUNT];

/* MPIDR Aff2 is the cluster, Aff1 the core (no MT bit on Nord). */
static unsigned int my_cluster(void)
{
	return (unsigned int)MPIDR_AFFLVL2_VAL(read_mpidr_el1());
}

static uintptr_t ncc_base(unsigned int cluster)
{
	return PLAT_NCC_ARCH_BASE + (cluster * PLAT_NCC_CLUSTER_STRIDE);
}

static void ncc_clrbits_64(uintptr_t addr, uint64_t clr)
{
	mmio_write_64(addr, mmio_read_64(addr) & ~clr);
}

/*
 * Before the first CPU_ON: clear the forced RVBAR vectoring of every
 * cluster so each core starts from a known state, enable the CL5 sequencer
 * trigger of every cluster and set the APSS power controls the cores need
 * to come up.
 */
static void ncc_lpm_init(void)
{
	unsigned int cl;

	for (cl = 0U; cl < PLAT_CLUSTER_COUNT; cl++) {
		ncc_clrbits_64(ncc_base(cl) + NCC_PWR_CTL3,
			       NCC_PWR_CTL3_CORFRCCPURVBAR_ALL);
	}
	dsbsy();

	for (cl = 0U; cl < PLAT_CLUSTER_COUNT; cl++) {
		mmio_write_32(APSS_CL_LPM_CL5_TRIG_CTRL(cl), 1U);
		mmio_write_32(APSS_CL_LPM_DBG_PWRUP_WAKE_CFG(cl), 0U);
	}
	mmio_clrbits_32(APSS_GICD_PWR_CTL, APSS_GICD_PWR_CTL_BIT2);
	mmio_setbits_32(APSS_CPUSS_LPM_CTL, BIT_32(0));
	dsbsy();
	isb();
}

/*
 * Power a core up through its cluster's NCC and let it fetch from NCC_RVBAR.
 * The wake-received mask is set and cleared around the power-up request:
 * its 1 to 0 transition is what vectors a cold core to RVBAR. The forced
 * RVBAR vectoring is dropped once the core runs, so that a core woken from
 * a low power state resumes where it stopped.
 */
static int ncc_power_on_core(unsigned int cluster, unsigned int core)
{
	uintptr_t base = ncc_base(cluster);
	unsigned int to;

	spin_lock(&cl_lock[cluster]);

	mmio_write_64(base + NCC_RVBAR, (uint64_t)warm_entry);
	mmio_setbits_64(base + NCC_PWR_CTL3, NCC_PWR_CTL3_CORFRCCPURVBAR(core));
	mmio_setbits_64(base + NCC_BOOT_CFG1,
			NCC_BOOT_CFG1_CORRVBARMASKWAKERCVD(core));

	ncc_clrbits_64(base + NCC_PWR_CFG1, NCC_PWR_CFG1_OK2ENTERC3(core) |
					    NCC_PWR_CFG1_OK2ENTERC4(core));
	mmio_setbits_64(base + NCC_BOOT_CFG1, NCC_BOOT_CFG1_CPUFETCHFWBLOCK(core));
	ncc_clrbits_64(base + NCC_BOOT_CFG1,
		       NCC_BOOT_CFG1_CORRVBARMASKWAKERCVD(core));
	mmio_setbits_64(base + NCC_PWR_CTL1, NCC_PWR_CTL1_COREPWRUPREQ(core));

	for (to = 0U; to < NCC_POWERUP_POLL; to++) {
		if ((mmio_read_64(base + NCC_PWR_STS1) &
		     NCC_PWR_STS1_CORSPRINITDONE(core)) != 0U) {
			break;
		}
	}
	for (to = 0U; to < NCC_POWERUP_POLL; to++) {
		if ((mmio_read_64(base + NCC_PWR_STS1) &
		     NCC_PWR_STS1_CORDOMISON(core)) != 0U) {
			break;
		}
	}
	if (to == NCC_POWERUP_POLL) {
		spin_unlock(&cl_lock[cluster]);
		return PSCI_E_INTERN_FAIL;
	}

	ncc_clrbits_64(base + NCC_BOOT_CFG1, NCC_BOOT_CFG1_CPUFETCHFWBLOCK(core));
	/* Wake-ups from the low power states resume the core after its WFI. */
	ncc_clrbits_64(base + NCC_PWR_CTL3, NCC_PWR_CTL3_CORFRCCPURVBAR(core));
	dsbsy();
	isb();

	cl_idle_mask[cluster] &= ~BIT_32(core);
	spin_unlock(&cl_lock[cluster]);

	return PSCI_E_SUCCESS;
}

int plat_qti_pwr_psci_init(uintptr_t warmboot_entry)
{
	unsigned int cl;

	warm_entry = warmboot_entry;
	nord_cpu[plat_my_core_pos()].started = 1U;

	/* Only the boot core runs; the others count as idle until CPU_ON. */
	for (cl = 0U; cl < PLAT_CLUSTER_COUNT; cl++) {
		cl_idle_mask[cl] = NORD_CLUSTER_CORES_MASK;
	}
	cl_idle_mask[my_cluster()] &=
		~BIT_32(MPIDR_AFFLVL1_VAL(read_mpidr_el1()));

	cpucp_lpm_ok = (mmio_read_32(CPUCP_SCMI_SEC_CH + CPUCP_SCMI_CH_STATUS) &
			CPUCP_SCMI_CH_STATUS_FREE) != 0U;
	if (!cpucp_lpm_ok) {
		WARN("CPUCP secure channel busy: no cluster idle states\n");
	}

	return PSCI_E_SUCCESS;
}

int plat_qti_pwr_domain_on(u_register_t mpidr, int core_pos)
{
	static bool lpm_init_done;
	int ret;
	unsigned int core = (unsigned int)MPIDR_AFFLVL1_VAL(mpidr);
	unsigned int cluster = (unsigned int)MPIDR_AFFLVL2_VAL(mpidr);

	if ((cluster >= PLAT_CLUSTER_COUNT) ||
	    (core >= PLAT_CORE_COUNT_PER_CLUSTER)) {
		return PSCI_E_INVALID_PARAMS;
	}

	if (!lpm_init_done) {
		ncc_lpm_init();
		lpm_init_done = true;
	}

	/*
	 * gicv3_cpuif_enable() on the new core polls GICR_WAKER.ChildrenAsleep,
	 * which never clears while its redistributor is powered down.
	 */
	gicv3_rdistif_on((unsigned int)core_pos);

	/*
	 * The NCC power-up request only vectors a core that is powered down;
	 * a core parked by CPU_OFF is still on and is released instead.
	 */
	if (nord_cpu[core_pos].started != 0U) {
		nord_cpu[core_pos].release = 1U;
		flush_dcache_range((uintptr_t)&nord_cpu[core_pos],
				   sizeof(nord_cpu[core_pos]));
		dsbsy();
		sev();
		return PSCI_E_SUCCESS;
	}

	ret = ncc_power_on_core(cluster, core);
	if (ret == PSCI_E_SUCCESS) {
		nord_cpu[core_pos].started = 1U;
	}

	return ret;
}

void plat_qti_pwr_domain_on_finish(int core_pos, const uint8_t *states)
{
	(void)core_pos;
	(void)states;

	plat_qti_gic_pcpu_init();
	/* A core released from the CPU_OFF park runs again in its cluster. */
	core_lpm_exit(my_cluster(), (unsigned int)MPIDR_AFFLVL1_VAL(read_mpidr_el1()));
}

const unsigned int *plat_qti_pm_idle_states(void)
{
	static const unsigned int core_states[] = {
		qti_make_pwrstate_lvl0(QTI_LOCAL_STATE_STB,
				       PSTATE_TYPE_STANDBY),
		qti_make_pwrstate_lvl0(QTI_LOCAL_STATE_C4,
				       PSTATE_TYPE_STANDBY),
		0,
	};
	static const unsigned int idle_states[] = {
		qti_make_pwrstate_lvl0(QTI_LOCAL_STATE_STB,
				       PSTATE_TYPE_STANDBY),
		qti_make_pwrstate_lvl0(QTI_LOCAL_STATE_C4,
				       PSTATE_TYPE_STANDBY),
		qti_make_pwrstate_lvl1(NORD_DT_STATE_CL5, QTI_LOCAL_STATE_C4,
				       PSTATE_TYPE_STANDBY),
		/* The subsystem LPMs need CL5: only the cluster part is done. */
		qti_make_pwrstate_lvl2(QTI_LOCAL_STATE_SYS, NORD_DT_STATE_CL5,
				       QTI_LOCAL_STATE_C4, PSTATE_TYPE_STANDBY),
		0,
	};

	return cpucp_lpm_ok ? idle_states : core_states;
}

/*
 * The device tree passes 5 for the cluster state, above the 3 of core C4,
 * but PSCI wants a parent's local state no deeper than those of its
 * children: the cluster state is local state 3 too.
 */
void plat_qti_map_pwr_state(psci_power_state_t *req_state)
{
	if (req_state->pwr_domain_state[QTI_PWR_LVL1] == NORD_DT_STATE_CL5) {
		req_state->pwr_domain_state[QTI_PWR_LVL1] = QTI_LOCAL_STATE_CL4;
	}
}

/*
 * Report a cluster LPM transition to CPUCP. A transaction that fails takes
 * the domain states away for good: CPUCP may still act on a request it did
 * not answer, so the pairing of entries and exits cannot be trusted.
 */
static bool cpucp_cluster_lpm_notify(unsigned int cluster, bool entry)
{
	uintptr_t ch = CPUCP_SCMI_SEC_CH;
	uint32_t state = entry ? CPUCP_PWR_STATE_CL3 :
		(CPUCP_PWR_STATE_CL3 << CPUCP_PWR_STATE_PREV_SHIFT);
	uint64_t timeout;
	uint32_t sts;
	bool ok = false;

	spin_lock(&cpucp_lock);
	if ((mmio_read_32(ch + CPUCP_SCMI_CH_STATUS) &
	     CPUCP_SCMI_CH_STATUS_FREE) == 0U) {
		goto out;
	}

	mmio_write_32(ch + CPUCP_SCMI_CH_PAYLOAD, 0U);
	mmio_write_32(ch + CPUCP_SCMI_CH_PAYLOAD + 4U, cluster << 8);
	mmio_write_32(ch + CPUCP_SCMI_CH_PAYLOAD + 8U, state);
	mmio_write_32(ch + CPUCP_SCMI_CH_HDR,
		      SCMI_PWR_STATE_SET_MSG | (SCMI_PWR_DMN_PROTO_ID << 10));
	mmio_write_32(ch + CPUCP_SCMI_CH_LEN, 16U);
	mmio_write_32(ch + CPUCP_SCMI_CH_FLAGS, 0U);
	mmio_write_32(ch + CPUCP_SCMI_CH_STATUS, 0U);
	dsbsy();
	mmio_write_32(CPUCP_DOORBELL, CPUCP_DOORBELL_OSM_IPC);

	timeout = timeout_init_us(CPUCP_SCMI_TIMEOUT_US);
	do {
		sts = mmio_read_32(ch + CPUCP_SCMI_CH_STATUS);
		if ((sts & CPUCP_SCMI_CH_STATUS_FREE) != 0U) {
			break;
		}
	} while (!timeout_elapsed(timeout));

	ok = ((sts & CPUCP_SCMI_CH_STATUS_FREE) != 0U) &&
	     ((sts & CPUCP_SCMI_CH_STATUS_ERROR) == 0U) &&
	     (mmio_read_32(ch + CPUCP_SCMI_CH_PAYLOAD) == 0U);
out:
	if (!ok) {
		cpucp_lpm_ok = false;
	}
	spin_unlock(&cpucp_lock);

	return ok;
}

static void ncc_hid34_cg_override(unsigned int cluster, bool set)
{
	uintptr_t hid34 = NCC_UDBG_BASE + (cluster * PLAT_NCC_CLUSTER_STRIDE) +
			  NCC_UDBG_HID34_EL3;

	if (set) {
		mmio_setbits_64(hid34, NCC_HID34_CAGPSML3CGOVRRIDE);
	} else {
		ncc_clrbits_64(hid34, NCC_HID34_CAGPSML3CGOVRRIDE);
	}
}

/* The cluster's MPAM LLC settings do not survive the cluster LPM. */
static void ncc_mpam_restore(unsigned int cluster)
{
	uintptr_t off = cluster * PLAT_NCC_CLUSTER_STRIDE;

	mmio_write_32(NCC_MPAM_S_LLC0_BASE + off + NCC_MPAMCFG_PART_SEL, 0U);
	mmio_write_32(NCC_MPAM_S_LLC3_BASE + off + NCC_MPAMCFG_IMPL_SLC_SCID,
		      NCC_LLCC_SCID_CPU);
	mmio_write_32(NCC_MPAM_NS_LLC0_BASE + off + NCC_MPAMCFG_PART_SEL, 0U);
	mmio_write_32(NCC_MPAM_NS_LLC3_BASE + off + NCC_MPAMCFG_IMPL_SLC_SCID,
		      NCC_LLCC_SCID_CPU);
	mmio_clrbits_32(NCC_MPAM_S_LLC0_BASE + off + NCC_MPAMCFG_PRI,
			NCC_MPAMCFG_PRI_DSPRI);
	mmio_clrbits_32(NCC_MPAM_NS_LLC0_BASE + off + NCC_MPAMCFG_PRI,
			NCC_MPAMCFG_PRI_DSPRI);
}

/*
 * Cluster LPM (CL4, with CL3 under it): once every core of the cluster is
 * in C4, the NCC takes the cluster to its CL4 retention state and brings it
 * back on an interrupt for any of its cores. The cluster's architected
 * timers keep running, so their interrupts still wake it. The deeper CL5
 * powers the cluster down with its timers and is not entered. CPUCP is told
 * on entry and exit. Both run with the cluster's lock held.
 */
static void cluster_lpm_arm(unsigned int cluster)
{
	uintptr_t base = ncc_base(cluster);

	if (!cpucp_cluster_lpm_notify(cluster, true)) {
		return;
	}
	mmio_write_32(NCC_GBL_BASE + (cluster * PLAT_NCC_CLUSTER_STRIDE) +
		      NCC_GBL_SPARE_AON0, NCC_GBL_SPARE_AON0_CL4);
	mmio_setbits_64(base + NCC_PWR_CFG1, NCC_PWR_CFG1_OK2ENTERCL4 |
					     NCC_PWR_CFG1_OK2ENTERCL3);
	ncc_hid34_cg_override(cluster, true);
	cluster_lpm_armed[cluster] = true;
}

static void cluster_lpm_disarm(unsigned int cluster)
{
	uintptr_t base = ncc_base(cluster);

	mmio_write_32(NCC_GBL_BASE + (cluster * PLAT_NCC_CLUSTER_STRIDE) +
		      NCC_GBL_SPARE_AON0, NCC_GBL_SPARE_AON0_RUN);
	ncc_mpam_restore(cluster);
	(void)cpucp_cluster_lpm_notify(cluster, false);

	ncc_clrbits_64(base + NCC_PWR_CFG1, NCC_PWR_CFG1_OK2ENTERCL4 |
					    NCC_PWR_CFG1_OK2ENTERCL3);
	ncc_hid34_cg_override(cluster, false);
	cluster_lpm_armed[cluster] = false;
}

/*
 * C4: with its ok2EnterC3 and ok2EnterC4 bits set in NCC_PWR_CFG1, the
 * cluster NCC saves the core's context and powers it down once it is in
 * WFI, and powers it up and resumes it after the WFI on the next interrupt
 * for the core. The bits are only set around the WFI of a C4 request, so a
 * WFI standby stays a WFI. A core that runs again leaves the cluster's idle
 * mask and, if it is the first one out, disarms the cluster LPM in the same
 * critical section, so that the LPM is never armed under a running core.
 */
static void core_lpm_enter(unsigned int cluster, unsigned int core,
			   bool cluster_lpm)
{
	uintptr_t cfg1 = ncc_base(cluster) + NCC_PWR_CFG1;

	spin_lock(&cl_lock[cluster]);
	mmio_setbits_64(cfg1, NCC_PWR_CFG1_OK2ENTERC3(core) |
			      NCC_PWR_CFG1_OK2ENTERC4(core));
	cl_idle_mask[cluster] |= BIT_32(core);
	if (cluster_lpm && cpucp_lpm_ok && !cluster_lpm_armed[cluster] &&
	    (cl_idle_mask[cluster] == NORD_CLUSTER_CORES_MASK)) {
		cluster_lpm_arm(cluster);
	}
	spin_unlock(&cl_lock[cluster]);
}

static void core_lpm_exit(unsigned int cluster, unsigned int core)
{
	uintptr_t cfg1 = ncc_base(cluster) + NCC_PWR_CFG1;

	spin_lock(&cl_lock[cluster]);
	ncc_clrbits_64(cfg1, NCC_PWR_CFG1_OK2ENTERC3(core) |
			     NCC_PWR_CFG1_OK2ENTERC4(core));
	cl_idle_mask[cluster] &= ~BIT_32(core);
	if (cluster_lpm_armed[cluster]) {
		cluster_lpm_disarm(cluster);
	}
	spin_unlock(&cl_lock[cluster]);
}

void plat_qti_cpu_retention_enter(plat_local_state_t cpu_state)
{
	if (cpu_state == QTI_LOCAL_STATE_C4) {
		core_lpm_enter(my_cluster(),
			       (unsigned int)MPIDR_AFFLVL1_VAL(read_mpidr_el1()),
			       false);
	}
}

void plat_qti_cpu_retention_exit(plat_local_state_t cpu_state)
{
	if (cpu_state == QTI_LOCAL_STATE_C4) {
		core_lpm_exit(my_cluster(),
			      (unsigned int)MPIDR_AFFLVL1_VAL(read_mpidr_el1()));
	}
}

void plat_qti_pwr_domain_off(const uint8_t *states)
{
	(void)states;
}

/*
 * The NCC does not collapse a core that executes WFI after CPU_OFF, and
 * powering it up again does not reset it, so the core is parked in BL31:
 * PSCI has already disabled its GIC CPU interface and, with hardware
 * assisted coherency, leaves its caches to the hardware. It waits with the
 * MMU off for CPU_ON to release it and then takes the warm boot path as if
 * it had been powered up. The release flag is read and cleared with the
 * data cache off, so the releasing core cleans it to the PoC.
 */
void __dead2 plat_qti_cpu_park(const psci_power_state_t *target_state)
{
	struct nord_cpu_slot *slot = &nord_cpu[plat_my_core_pos()];

	unsigned int cluster = my_cluster();

	(void)target_state;

	spin_lock(&cl_lock[cluster]);
	cl_idle_mask[cluster] |= BIT_32(MPIDR_AFFLVL1_VAL(read_mpidr_el1()));
	spin_unlock(&cl_lock[cluster]);

	disable_mmu_icache_el3();
	while (mmio_read_32((uintptr_t)&slot->release) == 0U) {
		wfe();
	}
	mmio_write_32((uintptr_t)&slot->release, 0U);
	dsbsy();

	bl31_warm_entrypoint();
	panic();
}

/*
 * Cluster and system idle requests are retention states at every level,
 * which PSCI ends with a plain WFI after this hook: route the non-secure
 * interrupts to EL3 around it as the core standby does, so that they wake
 * the core.
 */
void plat_qti_pwr_domain_suspend(const uint8_t *states)
{
	unsigned int cluster = my_cluster();
	unsigned int core_pos = plat_my_core_pos();

	if (states[QTI_PWR_LVL0] != QTI_LOCAL_STATE_C4) {
		return;
	}

	suspend_scr[core_pos] = read_scr_el3();
	write_scr_el3(suspend_scr[core_pos] | SCR_IRQ_BIT | SCR_FIQ_BIT);
	isb();

	core_lpm_enter(cluster, (unsigned int)MPIDR_AFFLVL1_VAL(read_mpidr_el1()),
		       states[QTI_PWR_LVL1] == QTI_LOCAL_STATE_CL4);
}

void plat_qti_pwr_domain_suspend_finish(const uint8_t *states)
{
	unsigned int core_pos = plat_my_core_pos();

	if (states[QTI_PWR_LVL0] == QTI_LOCAL_STATE_C4) {
		core_lpm_exit(my_cluster(),
			      (unsigned int)MPIDR_AFFLVL1_VAL(read_mpidr_el1()));
		write_scr_el3(suspend_scr[core_pos]);
		isb();
		return;
	}

	plat_qti_gic_pcpu_init();
}
