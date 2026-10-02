Arduino UNO Q
=============

Trusted Firmware-A (TF-A) platform port for the Arduino UNO Q board, based on
the Qualcomm QRB2210 (Agatti) SoC. The SoC support lives in
``plat/qti/bruin/agatti`` and the board in ``plat/qti/bruin/agatti/uno_q``.

Agatti specifics:

- CPUs: four Kryo 2xx Silver cores (Cortex-A53) in one cluster, powered up
  through the APCS per-CPU power controllers as on MSM8916.
- Interrupt controller: GIC-500 (GICv3).
- Debug UART: QUPv3 wrap 0, serial engine 4 at ``0x4a90000``.
- XBL enters the TZ image in the 100 KiB IMEM TZ window, so BL2 runs from
  ``0x0c100000`` (from pIMEM with ``TRUSTED_BOARD_BOOT``, see below). BL31
  runs from the pIMEM aperture at ``0x10100000``.
- BL32 (OP-TEE) runs from ``0x45700000``, the 6 MiB carve-out the Linux DT
  reserves for the hypervisor; Linux runs at EL2.
- Memory protection: XBL_SEC leaves the DDR and pIMEM MPUs open to the
  normal world outside its own regions. Before it reads the FIP, BL2 takes
  one resource group on each so that BL31 and BL32 are reachable from the
  secure world only, and another one on the DDR MPU for the FIP that XBL
  loads to ``0x5f800000``, which it frees when it exits. BL31 applies the
  BL31 and BL32 configuration again and reports xPU and VMIDMT violations at
  EL3.
- The PMIC (PM4125) is set up for a shutdown or a warm reset before PS_HOLD
  is dropped.
- Storage is eMMC with 512-byte blocks.

Boot flow
---------

As on :ref:`Dragonwing RB3 Gen 2 development platform`: XBL loads BL2 from the
``tz`` partition and the FIP ELF from the ``uefi`` partition. BL2 loads BL31,
BL32 (OP-TEE) and BL33 (U-Boot) from the FIP.

How to build
------------

Steps to build TF-A BL2 and the FIP payload::

	$ make CROSS_COMPILE=aarch64-none-elf- PLAT=uno_q SPD=opteed \
	    BL32=<path-to-optee-bin> BL33=<path-to-u-boot-bin> fip all

	$ ./tools/qti/generate_fip_elf.sh build/uno_q/release/fip.bin \
	    0x5f800000

XBL authenticates the TZ image with the QTI authenticator even when secure
boot is disabled, so ``bl2.elf`` must be signed as a TZ image with QTI
signing. An OEM test signature from `qtestsign
<https://github.com/msm8916-mainline/qtestsign>`__ is not accepted for BL2.
The ``fip.elf`` is signed with qtestsign.

Trusted board boot
------------------

With ``TRUSTED_BOARD_BOOT=1``, BL2 authenticates BL31, BL32 and BL33 with
the TBBR chain of trust (:ref:`Trusted Board Boot`) once it has copied each
of them to its destination. BL2 holds the SHA-256 hash of the public part of
``ROT_KEY``, so the QTI signature that XBL checks on BL2 anchors the chain,
and the FIP carries the certificates. mbed TLS does not fit in the IMEM TZ
window, so this BL2 runs from pIMEM at ``0x1000c000``, below BL31, and the
pIMEM resource group it takes for BL31 also covers BL2. Until BL2 takes that
group, pIMEM is open to the normal world, as XBL leaves it. There are no
non-volatile counters: the certificates must carry counter 0. ::

	$ make CROSS_COMPILE=aarch64-none-elf- PLAT=uno_q SPD=opteed \
	    TRUSTED_BOARD_BOOT=1 GENERATE_COT=1 KEY_ALG=ecdsa \
	    MBEDTLS_DIR=<path-to-mbedtls> ROT_KEY=<rot-key.pem> \
	    BL32=<path-to-optee-bin> BL33=<path-to-u-boot-bin> fip all

Build options
-------------

- ``QTI_BL2_ACCESS_CONTROL``: BL2 locks the BL31 and BL32 carve-outs and the
  FIP staging area before it reads the FIP, as described above. ``1`` by
  default on this platform; ``0`` builds BL2 without it.

How to flash
------------

Put the board in EDL mode and write the ``a`` slot with `qdl
<https://github.com/linux-msm/qdl>`__ and the eMMC firehose programmer shipped
with the board software::

	$ qdl --storage emmc prog_firehose_ddr.elf \
	    write tz_a bl2.mbn write uefi_a fip.elf

--------------

*Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.*
