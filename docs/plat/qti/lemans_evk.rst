Dragonwing IQ-9075 development platform
=========================================

Trusted Firmware-A (TF-A) platform port for Lemans SoC based EVK platform also
known as IQ-9075 as the marketing name. For details about IQ-9075 platform,
refer to this
`link <https://www.qualcomm.com/internet-of-things/products/iq9-series/iq-9075>`__

Boot flow
-------------

Similar to :ref:`Dragonwing RB3 Gen 2 development platform`.

How to build
------------

Steps to build TF-A BL2 and FIP payload::

	$ make CROSS_COMPILE=aarch64-none-elf- PLAT=lemans_evk SPD=opteed \
            QTISECLIB_PATH=<qtiseclib-path> BL32=<path-to-optee-bin> \
            BL33=<path-to-os-bootloader-bin> fip all

        $ ./tools/qti/generate_fip_elf.sh build/lemans_evk/release/fip.bin \
            0xaf000000

QTISECLIB for lemans is still work under progress, the current port allows to
boot OP-TEE OS.

Note that the bl2.elf generated here is required to be signed as TZ image with
QTI signing involved. Also, the fip.elf generated above is signed using
`qtestsign <https://github.com/msm8916-mainline/qtestsign>`__ which just puts
the OEM test signature to allow it to work on devices in non-secure mode. In
secure mode, it is required to be signed using
`sectools v2 <https://docs.qualcomm.com/bundle/publicresource/topics/80-70020-11/tools.html>`__

FF-A with OP-TEE as the S-EL1 SPMC
----------------------------------

With ``SPD=spmd`` (``SPMD_SPM_AT_SEL2=0``) BL31 starts OP-TEE as the S-EL1
SPMC from the SPMC core manifest ``fdts/spmc_manifest.dts``, so that the
normal world and a hypervisor such as Xen reach OP-TEE over FF-A. OP-TEE is
built with ``CFG_CORE_SEL1_SPMC=y``.

TF-A BL2 loads the manifest from the FIP as TOS_FW_CONFIG. When BL31 and
OP-TEE are loaded by U-Boot SPL from a FIT instead, nothing loads the
manifest; with ``QTI_SPMC_MANIFEST_IN_BL31=1`` (the default with
``SPD=spmd``) BL31 carries a copy and places it at ``TOS_FW_CONFIG_BASE``
when the BL32 entry point arrives without one::

	$ make CROSS_COMPILE=aarch64-none-elf- PLAT=lemans_evk SPD=spmd \
	    SPMD_SPM_AT_SEL2=0 bl31

DRTM (Arm DEN0113)
------------------

``DRTM_SUPPORT=1`` (with ``SPD=spmd``) enables the DRTM service as the
Dynamic Configuration Environment. It needs mbedTLS (``MBEDTLS_DIR``, for
the SHA-256 and SHA-384 event log) and the ``contrib/libeventlog``
submodule::

	$ make CROSS_COMPILE=aarch64-none-elf- PLAT=lemans_evk SPD=spmd \
	    SPMD_SPM_AT_SEL2=0 DRTM_SUPPORT=1 MBEDTLS_DIR=<mbedtls-3.6> bl31

Platform specifics:

- DMA protection: for a dynamic launch BL31 sets every non-secure stream of
  the apps and PCIe SMMU-500 instances to fault
  (``smmuv2_ns_set_abort_all()``), and of the GPU SMMU while the GPU CX
  domain is powered (its registers read as zero and the GPU cannot master
  while CX is collapsed), holding the secure GPU SMMU clock vote while it
  does so. AOP, SAIL, CPUCP and the other always-running subsystem
  processors run trusted firmware and are reported as non-host platforms.
  The GIC is not covered: its redistributors access the LPI tables in DRAM
  outside the SMMUs, which the generic DRTM DMA protection does not
  restrict yet.
- TPM: the discrete TPM belongs to OP-TEE. BL31 records the launch in it
  with FF-A direct requests from an SPMD logical partition (ID 0xffc0,
  ``ENABLE_SPMD_LP``) to OP-TEE's D-RTM measurement endpoint (ID 0x8100,
  OP-TEE ``CFG_TPM2_DRTM_LSP``), which runs the locality 4 hash sequence
  for the DCE event and extends PCRs 17 and 18 with the other events, in
  both active PCR banks of the TPM (SHA-256 and SHA-384, the event log
  carries both digests).
  ``DRTM_SUPPORT=1`` therefore needs ``SPD=spmd``. The launch event tells
  OP-TEE which banks are measured; OP-TEE refuses the launch when the TPM's
  active banks differ.
- Locality 2: after a successful launch BL31 tells OP-TEE to accept
  ``TPM2_PCR_Event`` requests at locality 2 for PCRs 19 to 22 from the
  DLME (FF-A source ID 0); ``DRTM_CLOSE_LOCALITY(2)`` and the next launch
  end that. Xen as the DLME measures dom0 into PCR 19 this way.
- DLME region: a DDR bank the boot firmware reports in its SMEM usable RAM
  table, above the last firmware carveout (0xdb300000). The same banks are
  the normal memory of the DLME address map, next to the peripheral range
  as device memory, copied at DRTM setup. The DCE cleans the DLME region
  and hashes the DLME image through a 2 MiB mapping window, so their size
  is not limited by the BL31 translation tables and needs no padding.
- Error code: ``DRTM_SET_ERROR`` stores the 64-bit code little endian in
  eight bytes of the primary PMIC SDAM from ``QTI_DRTM_ERROR_SDAM_ADDR``
  (0x70b8 to 0x70bf, the top of the SDAM at SID 0 PPID 0x70, the one the
  secure world has an arbiter channel to), so ``DRTM_GET_ERROR`` returns it
  after a reset for as long as the PMIC stays powered. The slot is
  provisional: it reads zero on stock boots, but its allocation in the
  Lemans SDAM map is not confirmed. A launch the DCE stops records 2 when
  the TPM refused a measurement and 3 when the launch failed after the
  dynamic launch event; 1 (DMA protection could not be undone) is not
  reached with complete DMA protection, the only type Lemans offers.

How to flash
------------

Generate a Yocto BSP image for iq-9075-evk following instructions from meta-qcom
`here <https://github.com/qualcomm-linux/meta-qcom?tab=readme-ov-file#quick-build>`__

It will generate a tar ball in the build folder for iq-9075-evk something like:
`qcom-multimedia-image-iq-9075-evk.rootfs.qcomflash.tar.gz`. Untar it and replace
`tz.mbn` with `bl2.mbn` and `uefi.elf` with `fip.elf` that you have generated by
yourself.

Then flash the board following the meta-qcom instructions
`here <https://github.com/qualcomm-linux/meta-qcom?tab=readme-ov-file#flash-images>`__

Boot Trace
-------------

Boot logs on lemans_evk::

        Format: Log Type - Time(microsec) - Message - Optional Info
        Log Type: B - Since Boot(Power On Reset),  D - Delta,  S - Statistic
        S - QC_IMAGE_VERSION_STRING=BOOT.MXF.1.0.c1-00430-KODIAKLA-1
        S - IMAGE_VARIANT_STRING=SocLeMansLAA
        S - OEM_IMAGE_VERSION_STRING=258af405f4c7
        <snip-XBL-logs>
        NOTICE:  BL2: v2.14.0(release):v2.13.0-1873-g0c07fd50a
        NOTICE:  BL2: Built : 17:52:29, Feb 17 2026
        NOTICE:  BL2: Booting BL31
        NOTICE:  BL31: v2.14.0(release):v2.13.0-1873-g0c07fd50a
        NOTICE:  BL31: Built : 17:52:29, Feb 17 2026
        ERROR:   Please use QTISECLIB_PATH while building TF-A
        ERROR:   Please refer docs/plat/qti/* for more details.
        I/TC:
        I/TC: OP-TEE version: 4.9.0-1-gc6cb871cc (gcc version 11.4.0 (Ubuntu 11.4.0-1ubuntu1~22.04)) #1 Tue Feb 17 12:22:27 UTC 2026 aarch64
        I/TC: WARNING: This OP-TEE configuration might be insecure!
        I/TC: WARNING: Please check https://optee.readthedocs.io/en/latest/architecture/porting_guidelines.html
        I/TC: Primary CPU initializing
        I/TC: Platform Qualcomm: Flavor lemans
        I/TC: Primary CPU switching to normal world boot
        PANIC at PC : 0x000000001c200d70
