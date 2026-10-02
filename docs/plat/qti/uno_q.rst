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
  ``0x0c100000``. BL31 runs from the pIMEM aperture at ``0x10100000``.
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
and the FIP carries the certificates. There are no non-volatile counters:
the certificates must carry counter 0.

With mbed TLS, only the code and data of BL2 fit in the IMEM TZ window. Its
stacks, bss and translation tables go to ``0x60000000`` instead
(``SEPARATE_BL2_NOLOAD_REGION``), the first MiB of the DDR that XBL_SEC
keeps secure-only; BL2 zeroes and maps them itself. XBL refuses a TZ image
with a segment outside IMEM and pIMEM, even one without data, so the build
also writes ``bl2_tz.elf``, which is ``bl2.elf`` without that segment. Sign
``bl2_tz.elf`` instead of ``bl2.elf``. ::

	$ make CROSS_COMPILE=aarch64-none-elf- PLAT=uno_q SPD=opteed \
	    TRUSTED_BOARD_BOOT=1 GENERATE_COT=1 KEY_ALG=ecdsa \
	    MBEDTLS_DIR=<path-to-mbedtls> ROT_KEY=<rot-key.pem> \
	    BL32=<path-to-optee-bin> BL33=<path-to-u-boot-bin> fip all

.. warning::
   ``bl2_tz.elf`` works around XBL. XBL should accept BL2's NOLOAD segment
   in secure DDR in the future, so that ``bl2.elf`` can be signed as is and
   this step can go away.

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
