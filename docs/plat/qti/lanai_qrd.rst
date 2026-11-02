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

- BL31 runs the native PSCI: it powers the secondary cores on directly and
  parks cores turned off in BL31, and CPU standby (WFI) is the only idle
  state.
- SYSTEM_RESET and SYSTEM_OFF go through PS_HOLD and the PMK8550 PON over
  the PMIC arbiter v7. On the QRD, de-asserting PS_HOLD does not yet reset or
  power off the SoC.
- BL31 enables the apps PDC.
- CPUCP runs from the boot firmware; BL31 asks it over SCMI to enable each
  clock domain when the first core of the domain comes up, so that the
  kernel's EPSS cpufreq driver can scale it. No request is sent at cold
  boot, while CPUCP is still initialising.
- BL31 programs the CPU limits management (LMh) blocks of the three
  cluster rails before that, so the clusters are throttled at 95 C and
  105 C on their own temperature sensors.
- BL31 services the secure APSS watchdog and hands the non-secure one to
  the normal world. Like PS_HOLD, a bite does not yet reset the SoC on the
  QRD.
- TF-A programs no XPU: whether the boot chain protects the TZ carve-out
  that holds BL32 is not yet verified.

Known limitations
-----------------

Both behave the same with the vendor boot firmware on the same QRD.

- Reboot, power off and a watchdog bite do not reset the board. On
  SYSTEM_RESET, BL31 de-asserts PS_HOLD and the AOSS reset controller
  records it (``AOSS_CC_RESET_STATUS`` PS_HOLD bit set), but the PMK8550
  PON real-time status still reports PS_HOLD high, and BL31 panics when
  the reset does not happen. The vendor stack (PSCI SYSTEM_RESET and
  SYSTEM_RESET2) also hangs after "reboot: Restarting system", so the
  board's PS_HOLD path is suspected rather than the firmware; until it is
  resolved, warm reboot cannot be exercised on this board.
- With the userspace governor, the Cortex-X4 cpufreq policy (policy7) can
  read back a frequency other than the one set: at a request of 499200 kHz,
  ``cpuinfo_cur_freq``, which the kernel takes from the EPSS domain state
  register, alternates between 499200 and 1670400 kHz while the core is
  busy. A timed busy loop on that core runs at the requested 499.2 MHz,
  so the register misreports the frequency; the requested frequency is
  applied.

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
