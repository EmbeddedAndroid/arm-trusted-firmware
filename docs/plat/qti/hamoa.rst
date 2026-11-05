Hamoa (X1E80100)
================

Trusted Firmware-A (TF-A) platform port for the X1E80100 (Hamoa): three
clusters of four Qualcomm Oryon cores, with a GIC-700.

``PLAT=hamoa`` builds one of two BL31s:

- Without ``BL31QTILIB_PATH``, TF-A BL2 and BL31 boot on their own. BL31 uses
  the Wildcat family code that does not depend on the library (core position,
  reset and panic handlers, power domain tree, interrupt properties, the Oryon
  CPU operations) and the native power and SiP back-ends of the common QTI
  code. This document describes that build.
- With ``BL31QTILIB_PATH`` pointing at a BL31QTILIB, XBL enters BL31 directly
  with ``bl_params`` and BL31 uses the library for power, SiP and debug policy
  (``plat/qti/wildcat/hamoa/bl31qtilib``).

Boot flow
---------

TF-A BL2 goes into the ``tz`` partitions and the FIP payload into the ``uefi``
partitions::

        PBL (ROM) -> XBL -> TF-A BL2 -> TF-A BL31 -> BL33 -> Linux kernel
                                            |
                                             --> OP-TEE as BL32

- XBL authenticates and loads TF-A BL2 from the ``tz_a/b`` partition into
  system IMEM (0x14680000, inside the 100 KB window XBL accepts there) and the
  FIP payload from ``uefi_a/b`` into DRAM at 0xa7000000.
- BL2 parses the FIP and loads BL31 into the ``qtee`` carve-out of the
  X1E80100 device tree (0xd8100000), BL32 into the ``ta`` region the same
  device tree reserves (0xd8600000, 30 MB) and BL33 at 0xa7800000.

Status
------

- CPUCP runs from the boot firmware and powers the cores: BL31 starts a
  secondary core with an MPIDR-addressed SCMI POWER_STATE_SET on the secure
  CPUCP channel, after programming each cluster's NCC_RVBAR with the warm
  boot entry point. CPU_ON of a core that is fused off on the part (SMEM
  platform information) returns INVALID_PARAMETERS.
- CPU_OFF has the cluster's NCC power the core down in C4 once it is in WFI;
  CPU_ON of such a core wakes it with its wake-up SGI.
- The idle states of the X1E80100 device tree (CPU C4, cluster CL4 and CL5
  and the system state, all retention) are entered with a WFI; no deeper
  hardware state is programmed. In OS-initiated mode PSCI refuses CL5 and
  the system state: their State IDs give the cluster (5) or the PDC (0xc) a
  deeper local state than the cores' C4 (4), which its state coordination
  does not allow.
- SYSTEM_RESET and SYSTEM_OFF go through PS_HOLD and the PMK8550 PON over
  the PMIC arbiter v7.
- BL31 services the secure APSS watchdog, which XBL leaves running.
- The apps PDC, the SMMUs, LPASS and the GPU are not set up by BL31.
- TF-A programs no XPU: whether the boot chain protects the carve-outs that
  hold BL31 and BL32 is not verified. The QTI SiP MEM_ASSIGN call is
  accepted without effect, as on the other platforms without an access
  control back-end.

How to build
------------

Steps to build TF-A BL2 and the FIP payload::

        $ make CROSS_COMPILE=aarch64-none-elf- PLAT=hamoa SPD=opteed \
            BL32=<path-to-optee-bin> BL33=<path-to-os-bootloader-bin> fip all

        $ ./tools/qti/generate_fip_elf.sh build/hamoa/release/fip.bin \
            0xa7000000

The bl2.elf generated here has to be signed for the ``tz`` partition with QTI
signing involved. The fip.elf is signed with
`qtestsign <https://github.com/msm8916-mainline/qtestsign>`__, which only
adds the OEM test signature accepted by devices with secure boot disabled.

--------------

*Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.*
