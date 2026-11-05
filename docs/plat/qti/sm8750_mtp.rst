Pakala (SM8750) MTP
===================

Trusted Firmware-A (TF-A) platform port for the SM8750 (Pakala) MTP: two
clusters of Qualcomm Oryon cores (six and two), with a GIC-700.

Pakala is a Wildcat part built without BL31QTILIB: BL31 uses the Wildcat
family code that does not depend on the library (core position, reset and
panic handlers, power domain tree, interrupt properties, the Oryon CPU
operations) and the native power and SiP back-ends of the common QTI code.

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

- CPUCP runs from the boot firmware and powers the cores: BL31 starts a
  secondary core with an MPIDR-addressed SCMI POWER_STATE_SET on the secure
  CPUCP channel, after programming each cluster's NCC_RVBAR with the warm
  boot entry point. CPU_ON of a core that is fused off on the part (SMEM
  platform information) returns INVALID_PARAMETERS. At CPU_OFF BL31 lets
  the cluster's NCC power the core down in C4 once it is in WFI, and CPU_ON
  wakes it with an SGI. The NCC resumes the core after that WFI, from where
  BL31 enters its warm boot path.
- BL31 reports DVFS calibration complete to the PDP of each cluster at cold
  boot (QCOM SCMI vendor protocol, "calcomp"), after which CPUCP serves the
  SCMI performance fastchannels the kernel's cpufreq driver writes.
- The idle states of the SM8750 device tree (CPU C4, cluster CL5 and the
  system state, all retention) are accepted, in OS-initiated mode too, and
  entered with a WFI. No deeper hardware state is programmed.
- SYSTEM_RESET and SYSTEM_OFF go through PS_HOLD and the PMK8550 PON over
  the PMIC arbiter v7.
- BL31 services the secure APSS watchdog, which XBL leaves running.
- XBL puts secure watchdog, thermal and PMIC RESIN resets on the debug path,
  where the AOSS halts the SoC for the system debug image, which ends in RAM
  dump mode. BL31 makes them plain resets; build with ``QTI_RAMDUMP=1`` to
  keep the RAM dumps.
- BL31 opens the SMEM partitions shared with the ADSP to the apps non-secure
  access domain at the LLCC MPU: the ADSP's traffic carries that domain,
  which the boot firmware leaves out. HLOS uses the same domain and gains
  access to those partitions too.
- The apps PDC, the SMMUs and the GPU are not set up by BL31.
- TF-A programs no XPU: whether the boot chain protects the carve-outs that
  hold BL31 and BL32 is not verified. The QTI SiP MEM_ASSIGN call is
  accepted without effect, as on the other platforms without an access
  control back-end.

How to build
------------

Steps to build TF-A BL2 and the FIP payload::

        $ make CROSS_COMPILE=aarch64-none-elf- PLAT=sm8750_mtp SPD=opteed \
            BL32=<path-to-optee-bin> BL33=<path-to-os-bootloader-bin> fip all

        $ ./tools/qti/generate_fip_elf.sh build/sm8750_mtp/release/fip.bin \
            0xa7000000

The bl2.elf generated here has to be signed for the ``tz`` partition with QTI
signing involved. The fip.elf is signed with
`qtestsign <https://github.com/msm8916-mainline/qtestsign>`__, which only
adds the OEM test signature accepted by devices with secure boot disabled.

--------------

*Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.*
