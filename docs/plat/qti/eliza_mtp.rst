Eliza (SM7750) MTP
==================

Trusted Firmware-A (TF-A) platform port for the SM7750 (Eliza) MTP: 3x
Cortex-A520 and 5x Cortex-A720 in one DynamIQ cluster, with a GIC-700.

Eliza is a Wildcat part built without BL31QTILIB: BL31 uses the Wildcat
family code that does not depend on the library (core position, reset and
panic handlers, power domain tree, interrupt properties) and the native
power and SiP back-ends of the common QTI code.

The same build boots the Eliza CQS EVK, which holds the same partitions on
eMMC instead of UFS.

Boot flow
---------

TF-A BL2 goes into the ``tz`` partitions and the FIP payload into the ``uefi``
partitions::

        PBL (ROM) -> XBL -> TF-A BL2 -> TF-A BL31 -> BL33 -> Linux kernel
                                            |
                                             --> OP-TEE as BL32

- XBL authenticates and loads TF-A BL2 from the ``tz_a/b`` partition into
  system IMEM (0x14693000) and the FIP payload from ``uefi_a/b`` into DRAM at
  0xa7000000.
- BL2 parses the FIP and loads BL31 into the ``tz-merged`` carve-out
  (0xd8100000), BL32 into the ``oem-vm`` region (0xf7c00000, 30 MB) and BL33
  at 0xa7800000.

Status
------

- CPUCP runs from the boot firmware; BL31 asks it over SCMI to enable each
  CPU clock domain when the first core of the domain comes up, so that the
  kernel's EPSS cpufreq driver can scale it. No request is sent at cold
  boot.
- BL31 runs the native PSCI: it powers the secondary cores on directly. It
  loads the core and Cortex-A520 complex power-control programs into the
  IPM sequencer memory and arms the complex sequencers; at CPU_OFF the
  core's sequencer collapses the core (a complex goes down with its last
  core), and CPU_ON wakes it with an SGI. Only CPU standby (WFI) is offered as an idle state, so the core and
  cluster idle states of the SM7750 device tree are refused.
- SYSTEM_RESET and SYSTEM_OFF go through PS_HOLD and the PMK8550 PON over
  the PMIC arbiter v7. On the MTP, SYSTEM_RESET does not reset the board:
  PS_HOLD drops with the PON reset configured, the PMIC does not act, and
  BL31 panics a second later.
- BL31 services the secure APSS watchdog (TMR2) and hands the other one
  (TMR1) to the normal world.
- BL31 leaves the secure GPU CX and GX GDSC controls to the voting logic
  with register retention, clears SACR.CACHE_LOCK in both SMMUs and points
  GPU SMMU aperture slot 0 at context bank 0.
- BL31 releases the LPASS boot IMEM at cold boot, as the ADSP expects, and
  opens the SMEM partitions shared with the ADSP to the apps non-secure
  access domain at the LLCC MPU: the ADSP's traffic carries that domain,
  which the boot firmware leaves out. HLOS uses the same domain and gains
  access to those partitions too.
- BL31 gives the apps PDC inputs to the HLOS DRV; the AOP owns the PDC
  sequences.
- TF-A programs no XPU: whether the boot chain protects the carve-outs that
  hold BL31 and BL32 is not verified. The QTI SiP MEM_ASSIGN call is
  accepted without effect, as on the other platforms without an access
  control back-end; the kernel's remoteproc and FastRPC drivers need it to
  succeed.

How to build
------------

Steps to build TF-A BL2 and the FIP payload::

        $ make CROSS_COMPILE=aarch64-none-elf- PLAT=eliza_mtp SPD=opteed \
            BL32=<path-to-optee-bin> BL33=<path-to-os-bootloader-bin> fip all

        $ ./tools/qti/generate_fip_elf.sh build/eliza_mtp/release/fip.bin \
            0xa7000000

The bl2.elf generated here has to be signed for the ``tz`` partition with QTI
signing involved. The fip.elf is signed with
`qtestsign <https://github.com/msm8916-mainline/qtestsign>`__, which only
adds the OEM test signature accepted by devices with secure boot disabled.

--------------

*Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.*
