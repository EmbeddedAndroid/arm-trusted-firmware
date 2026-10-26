Snapdragon 8 Gen 2 (SM8550) HDK
===============================

Trusted Firmware-A (TF-A) platform port for the SM8550 (Kailua) HDK:
3x Cortex-A510, 2x Cortex-A715, 2x Cortex-A710 and 1x Cortex-X3 in one
DynamIQ cluster, with a GIC-700.

Boot flow
---------

TF-A BL2 replaces the TZ image and the FIP payload replaces the UEFI image::

        PBL (ROM) -> XBL -> TF-A BL2 -> TF-A BL31 -> BL33 -> Linux kernel
                                            |
                                             --> OP-TEE as BL32

- XBL authenticates and loads TF-A BL2 from the ``tz_a/b`` partition into
  the TZ region of the system IMEM (0x14680000) and the FIP payload from
  ``uefi_a/b`` into DRAM at 0xa7000000, the stock UEFI load address.
- BL2 parses the FIP and loads BL31 into the secure OS slot of the TZ
  carve-out (0xd8300000), BL32 into the TA area above it (0xd8800000,
  30 MB), and BL33 at 0xa7800000.

Status
------

The port has not run on hardware yet.

- BL31 runs the native PSCI: it powers the secondary cores on directly
  (with the complex of the second and third Cortex-A510, and the gold rail
  of cores 3 to 7) and parks cores turned off in BL31, and CPU standby
  (WFI) is the only idle state.
- SYSTEM_RESET and SYSTEM_OFF go through PS_HOLD and the PMK8550 PON over
  the PMIC arbiter v7. On the SM8650 QRD that path, and a watchdog bite, do
  not yet reset the SoC; SM8550 is expected to behave the same.
- BL31 enables the apps PDC.
- BL31 services the secure APSS watchdog.
- TF-A programs no XPU: whether the boot chain protects the TA area that
  holds BL32 is not verified.

How to build
------------

Steps to build TF-A BL2 and the FIP payload::

        $ make CROSS_COMPILE=aarch64-none-elf- PLAT=kailua_hdk SPD=opteed \
            BL32=<path-to-optee-bin> BL33=<path-to-os-bootloader-bin> fip all

        $ ./tools/qti/generate_fip_elf.sh build/kailua_hdk/release/fip.bin \
            0xa7000000

The bl2.elf generated here has to be signed as the TZ image with QTI signing
involved. The fip.elf is signed with
`qtestsign <https://github.com/msm8916-mainline/qtestsign>`__, which only
adds the OEM test signature accepted by devices with secure boot disabled.

--------------

*Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.*
