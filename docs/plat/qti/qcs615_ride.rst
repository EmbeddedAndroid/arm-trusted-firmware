QCS615 Ride
===========

Trusted Firmware-A (TF-A) platform port for the QCS615 Ride, a Talos board
that boots through XBL. The SoC support lives in ``plat/qti/hoya/talos`` and
the port in ``plat/qti/hoya/talos/qcs615_ride``; ``PLAT=qcs615`` next to it is
the coreboot flow with QTISECLIB (see the Chrome page).

It was tested with the XBL of the QCS615 boot binaries 00142 and relies on
that handoff: the TZ image entered in system IMEM, the FIP ELF loaded at
``0x9fc00000``, the debug UART at ``0x880000`` and the Talos DDR memory map.

Talos specifics:

- CPUs: one DSU cluster of six Kryo 4xx Silver (Cortex-A55 based) and two
  Kryo 4xx Gold (Cortex-A76 based) cores.
- Debug UART: QUPv3 wrap 0, serial engine 0 at ``0x880000``.
- BL2 runs from the system IMEM window that XBL gives the TZ image
  (``0x14680000``, 100 KiB); XBL resets the SoC when the TZ entry point is in
  pIMEM. BL31 runs from pIMEM and BL32 from DDR at ``0x87a00000``.
- There is no CPUCP. BL31 brings the secondary cores up through the APSS power
  sequencer. XBL leaves the gold (APC1) rail collapsed, so BL31 raises it to
  its boot voltage through the gold SAW4 before the first gold core powers on.
- BL31 enables the apps PDC, without which RPMh requests from the OS do not
  complete, and programs the GPU SMMU aperture the Adreno CP uses for
  per-process page tables. The OS can point the aperture at context banks 0
  and 1 through the QTI SiP call.
- Storage is UFS.

Boot flow
---------

Same as :ref:`Dragonwing RB3 Gen 2 development platform`: XBL loads BL2 from
the ``tz`` partition and the FIP ELF from the ``uefi`` partition. BL2 loads
BL31, BL32 (OP-TEE) and BL33 (U-Boot) from the FIP.

How to build
------------

OP-TEE (BL32) is built for ``PLATFORM=qcom-talos``; the FIP takes
``out/arm/core/tee-raw.bin``. Its core and TA RAM fill the 28 MiB TZ Apps
carve-out ``0x87a00000`` to ``0x89600000``, which Linux has to leave out of its
memory.

U-Boot (BL33) is built from ``qcom_qcs615_defconfig`` with the TF-A/OP-TEE
fragment::

	$ make qcom_qcs615_defconfig tfa-optee.config
	$ make

``tfa-optee.config`` has to enable ``CONFIG_OPTEE_LIB``, which copies the
OP-TEE node U-Boot finds into the device tree it passes to Linux. Until it
is merged, that takes the U-Boot patch "board: qualcomm: tfa-optee: enable
the OP-TEE library"
(https://lore.kernel.org/all/20260928-monaco-ventuno-q-v2-1-0e046c8242be@oss.qualcomm.com/).

Steps to build TF-A BL2 and FIP payload::

	$ make CROSS_COMPILE=aarch64-none-elf- PLAT=qcs615_ride SPD=opteed \
	    BL32=<path-to-tee-raw.bin> BL33=<path-to-u-boot.bin> fip all

	$ ./tools/qti/generate_fip_elf.sh build/qcs615_ride/release/fip.bin \
	    0x9fc00000

XBL authenticates the TZ image with the QTI authenticator even when secure
boot is disabled, so ``bl2.elf`` has to be signed as a TZ image with a QTI RSA
signature. The ``fip.elf`` is signed with
`qtestsign <https://github.com/msm8916-mainline/qtestsign>`__, an OEM test
signature that only devices in non-secure mode accept. Devices with secure
boot enabled need images signed with their production keys instead.

How to flash
------------

In the board's ``qcomflash`` package, replace ``tz.mbn`` with the signed BL2
and ``uefi.elf`` with ``fip.elf``, then flash it with
`qdl <https://github.com/linux-msm/qdl>`__.

Limitations
-----------

- Only CPU standby (WFI) is offered as an idle state. BL31 refuses the
  power-collapse idle states that the Talos device tree describes, and PSCI
  SYSTEM_SUSPEND, which the common QTI PSCI operations list, is denied.
- Nothing completes the power-down request of a core turned off, so PSCI
  CPU_OFF does not power the core down: it waits in BL31, powered, until
  CPU_ON releases it.
- CPU frequency scaling is not set up: the CPUs run at the frequencies XBL
  leaves.
- There is no access control driver. BL31 does not program the XPUs, so the
  protection of the secure carve-outs (BL31 in pIMEM, OP-TEE in DDR) is what
  XBL set up. For development, the QTI SiP MEM_ASSIGN call is accepted as a
  no-op (``QTI_MEM_ASSIGN_NOOP``), so that FastRPC with ``qcom,vmids`` probes;
  the memory is not restricted to the destination VMs.

--------------

*Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.*
