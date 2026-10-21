Snapdragon 8 Gen 3 (SM8650) QRD
===============================

Trusted Firmware-A (TF-A) platform port for the SM8650 (Lanai) QRD
reference device: 2x Cortex-A520, 5x Cortex-A720 and 1x Cortex-X4 in one
DynamIQ cluster, with a GIC-700.

Boot flow
---------

TF-A BL2 replaces the TZ image and the FIP payload replaces the UEFI image::

        PBL (ROM) -> XBL -> TF-A BL2 -> TF-A BL31 -> BL33 -> Linux kernel
                                            |
                                             --> OP-TEE as BL32

- XBL authenticates and loads TF-A BL2 from the ``tz_a/b`` partition into
  system IMEM (0x14693000) and the FIP payload from ``uefi_a/b`` into DRAM at
  0xa7000000, the stock UEFI load address.
- BL2 parses the FIP and loads BL31 into the secure OS slot of the
  ``tz-merged`` carve-out (0xd8300000), BL32 into the TA area of the TZ
  carve-out above it (0xd8800000, 30 MB), and BL33 at 0xa7800000.

Status
------

- BL31 runs the native PSCI on the boot core only: CPU_ON is not supported
  yet, and CPU standby (WFI) is the only idle state.
- SYSTEM_RESET and SYSTEM_OFF go through PS_HOLD and the PMK8550 PON over
  the PMIC arbiter v7. On the QRD, de-asserting PS_HOLD does not yet reset or
  power off the SoC.
- The secure watchdog is not serviced.
- TF-A programs no XPU: whether the boot chain protects the TZ carve-out
  that holds BL32 is not yet verified.

How to build
------------

Steps to build TF-A BL2 and the FIP payload::

        $ make CROSS_COMPILE=aarch64-none-elf- PLAT=lanai_qrd SPD=opteed \
            BL32=<path-to-optee-bin> BL33=<path-to-os-bootloader-bin> fip all

        $ ./tools/qti/generate_fip_elf.sh build/lanai_qrd/release/fip.bin \
            0xa7000000

The bl2.elf generated here has to be signed as the TZ image with QTI signing
involved. The fip.elf is signed with
`qtestsign <https://github.com/msm8916-mainline/qtestsign>`__, which only
adds the OEM test signature accepted by devices with secure boot disabled.

--------------

*Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.*
