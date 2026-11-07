Nord (SA8797P)
==============

Trusted Firmware-A (TF-A) platform port for the SA8797P (Nord): three
clusters of six Qualcomm Oryon cores, with a GIC-700.

``PLAT=nord`` builds one of two BL31s:

- Without ``BL31QTILIB_PATH``, TF-A BL2 and BL31 boot on their own. BL31 uses
  the Wildcat family code that does not depend on the library (core position,
  reset and panic handlers, power domain tree, interrupt properties, the Oryon
  CPU operations) and the native power and SiP back-ends of the common QTI
  code. This document describes that build.
- With ``BL31QTILIB_PATH`` pointing at a BL31QTILIB, XBL enters BL31 directly
  with ``bl_params`` and BL31 uses the library for power, SiP and debug policy
  (``plat/qti/wildcat/nord/bl31qtilib``).

Boot flow
---------

TF-A BL2 goes into the ``tz`` partitions and the FIP payload into the ``uefi``
partitions::

        PBL (ROM) -> XBL -> TF-A BL2 -> TF-A BL31 -> BL33 -> Linux kernel
                                            |
                                             --> OP-TEE as BL32

- XBL authenticates and loads TF-A BL2 from the ``tz_a/b`` partition into
  system IMEM at 0x14694000 and the FIP payload from ``uefi_a/b`` into DRAM at
  0xd8d00000, where the stock UEFI image is loaded.
- BL2 parses the FIP and loads BL31 into the QTEE region of the DARE-TZ
  carve-out (0xbc180000, 1 MB), BL32 right after it (0xbc280000, OP-TEE
  ``PLATFORM=qcom-nord``) and BL33 into non-secure DRAM at 0xa0200000.

Status
------

- CPU_ON powers a core up through its cluster's NCC, after powering its GIC
  redistributor, and vectors it to the warm boot entry point through the
  cluster's NCC_RVBAR. All 18 cores come up.
- CPU standby (WFI) is the only idle state. CPU_OFF and suspend leave the core
  powered: the NCC does not power a core down on its own, so a core taken
  offline is not brought back by CPU_ON.
- BL31 services the secure APSS watchdog and the NCC QTimer frames, sets the
  APSS SPI types, initializes the PDC tables and configures the SMMUs.
- SYSTEM_RESET and SYSTEM_OFF tell the safety island (SAIL) first, then go
  through PS_HOLD and the PMIC PON over the PMIC arbiter v7. Without the SAIL
  message XBL handles the next boot as a main-domain crash and waits for SAIL.
- SMEM is the 4 MB region the device tree reserves at 0x89b00000.
- TF-A programs no XPU.

How to build
------------

Steps to build TF-A BL2 and the FIP payload::

        $ make CROSS_COMPILE=aarch64-none-elf- PLAT=nord SPD=opteed \
            BL32=<path-to-optee-bin> BL33=<path-to-os-bootloader-bin> fip all

For FF-A with OP-TEE as the S-EL1 SPMC, build with ``SPD=spmd
SPMD_SPM_AT_SEL2=0`` and OP-TEE with ``CFG_CORE_SEL1_SPMC=y``. The FIP then
also carries the SPMC core manifest, built from BL32_BASE and BL32_SIZE,
which BL2 loads into the last 8 KB of the BL31 slot for the SPMD.

bl2.elf has to be signed for the ``tz`` partition with QTI signing involved.
The FIP goes into the ``uefi`` partition as an ELF with one loadable segment
at 0xd8d00000, padded to a multiple of 4 KB (XBL reads it from UFS in whole
sectors), with ``e_machine`` EM_ARM as the stock UEFI image has, and signed
for the UEFI image ID.

--------------

*Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.*
