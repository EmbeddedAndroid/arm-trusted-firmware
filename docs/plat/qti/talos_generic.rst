Talos
=====

Trusted Firmware-A (TF-A) platform port for boards based on the Qualcomm
Talos SoC that boot through XBL. The SoC support lives in
``plat/qti/hoya/talos`` and the port in ``plat/qti/hoya/talos/talos_generic``;
it has no board-specific code.

Talos specifics:

- CPUs: one DSU cluster of six Kryo 4xx Silver (Cortex-A55 based) and two
  Kryo 4xx Gold (Cortex-A76 based) cores.
- Debug UART: QUPv3 wrap 0, serial engine 0 at ``0x880000``.
- BL2 runs from the system IMEM window that XBL gives the TZ image
  (``0x14680000``, 100 KiB); XBL resets the SoC when the TZ entry point is in
  pIMEM. BL31 runs from pIMEM and BL32 from DDR at ``0x87a00000``.
- There is no CPUCP. BL31 brings the secondary cores up through the APSS power
  sequencer and offers CPU standby only. XBL leaves the gold (APC1) rail
  collapsed, so BL31 raises it to its boot voltage through the gold SAW4
  before the first gold core powers on. Access control is not configured.
- Storage is UFS.

Boot flow
---------

Same as :ref:`Dragonwing RB3 Gen 2 development platform`: XBL loads BL2 from
the ``tz`` partition and the FIP ELF from the ``uefi`` partition. BL2 loads
BL31, BL32 (OP-TEE) and BL33 (U-Boot) from the FIP.

How to build
------------

Steps to build TF-A BL2 and FIP payload::

	$ make CROSS_COMPILE=aarch64-none-elf- PLAT=talos_generic SPD=opteed \
	    BL32=<path-to-optee-bin> BL33=<path-to-u-boot-bin> fip all

	$ ./tools/qti/generate_fip_elf.sh build/talos_generic/release/fip.bin \
	    0x9fc00000

The ``bl2.elf`` has to be signed as a TZ image with QTI signing. The
``fip.elf`` is signed with
`qtestsign <https://github.com/msm8916-mainline/qtestsign>`__.

How to flash
------------

In the board's ``qcomflash`` package, replace ``tz.mbn`` with the signed BL2
and ``uefi.elf`` with ``fip.elf``, then flash it with
`qdl <https://github.com/linux-msm/qdl>`__.

--------------

*Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.*
