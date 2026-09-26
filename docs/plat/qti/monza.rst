Arduino VENTUNO Q
=================

Trusted Firmware-A (TF-A) platform port for the Arduino VENTUNO Q board, based
on the Qualcomm QCS8275 (Monaco) SoC, also marketed as Dragonwing IQ-8275. The
SoC support lives in ``plat/qti/hoya/monaco`` and the board in
``plat/qti/hoya/monaco/monza``.

Monaco specifics:

- CPUs: four Cortex-A78C cores in cluster 0 (CPU2 and CPU3 in their own gold+
  clock domain) and four Cortex-A55 cores in cluster 1.
- Debug UART: QUPv3 wrap 0, serial engine 7 at ``0x99c000``.
- XBL enters the TZ image in system IMEM, so BL2 runs from ``0x14680000``.
- XBL only loads the CPUCP firmware. BL31 hands it the chip and CPR fuse data
  and starts it, then enables the Cortex-A55 and gold+ clock domains through
  CPUCP when their first core powers on.
- The apps PDC has the Lemans interrupt mapping and sequencer programs and is
  set up from the Lemans tables (``PDC_CHIPSET := lemans``); without it RPMh
  requests from the normal world never complete.
- The Cortex-A55 cores lack pointer authentication, which Linux requires of
  every CPU once its boot CPU has it; BL33 has to hand the boot over to a
  Cortex-A55 core with PSCI ``CPU_ON`` before starting Linux.
- Storage is eMMC (``sdhc_1``) with 512-byte blocks.
- A system reset is a PMIC hard reset (``QTI_PMIC_HARD_RESET``).

Boot flow
---------

Similar to :ref:`Dragonwing RB3 Gen 2 development platform`: XBL loads BL2
from the ``tz`` partition and the FIP ELF from the ``uefi`` partition. BL2
loads BL31, BL32 (OP-TEE) and BL33 (U-Boot) from the FIP.

How to build
------------

Steps to build TF-A BL2 and FIP payload::

	$ make CROSS_COMPILE=aarch64-none-elf- PLAT=monza SPD=opteed \
	    BL32=<path-to-optee-bin> BL33=<path-to-u-boot-bin> fip all

	$ ./tools/qti/generate_fip_elf.sh build/monza/release/fip.bin \
	    0xaf000000

XBL authenticates the TZ image with the QTI authenticator even when secure
boot is disabled, so ``bl2.elf`` must be signed as a TZ image with QTI
signing, including the SW image version (SWIV) segment. An OEM test signature
from `qtestsign <https://github.com/msm8916-mainline/qtestsign>`__ is not
accepted for BL2. The ``fip.elf`` is signed with qtestsign.

How to flash
------------

Put the board in EDL mode and write both A and B slots with
`qdl <https://github.com/linux-msm/qdl>`__ and the eMMC firehose programmer
shipped with the board software::

	$ qdl --storage emmc prog_firehose_ddr.elf \
	    write tz_a bl2.mbn write tz_b bl2.mbn \
	    write uefi_a fip.elf write uefi_b fip.elf

Boot flow with U-Boot SPL
-------------------------

U-Boot SPL (``qcom_monaco_spl_defconfig``) can take the place of BL2 in the
``tz`` partition. XBL still loads the ``uefi`` partition to ``0xaf000000``
before it starts the TZ image; there it finds a FIT with BL31, BL32 and BL33
instead of the FIP. SPL loads the FIT images and starts BL31 with version 2
image parameters (``bl_params_t``) and no platform parameter in ``x1``, the
same interface BL2 uses, so only BL31 is built::

	$ make CROSS_COMPILE=aarch64-none-elf- PLAT=monza SPD=opteed bl31

In the FIT, ``bl31.bin`` loads at ``BL31_BASE`` (``0x1c200000``), OP-TEE at
``BL32_BASE`` (``0x1c300000``) and U-Boot at ``BL33_BASE`` (``0xaf400000``).
The signed SPL ELF goes to the ``tz`` partitions and the FIT, wrapped in an
ELF at ``0xaf000000`` with ``tools/qti/generate_fip_elf.sh``, to the ``uefi``
partitions. The U-Boot documentation for the board describes the FIT and
the SPL build.

--------------

*Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.*
