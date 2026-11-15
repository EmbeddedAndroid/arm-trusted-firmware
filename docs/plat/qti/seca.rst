SecaAU (QAM8787P)
=================

Trusted Firmware-A (TF-A) platform port for the QAM8787P (SecaAU): two
clusters of six Qualcomm Oryon cores, with a GIC-700.

SecaAU is a variant of Nord (:doc:`nord`) with one CPU cluster and one GPU
fewer. Its boot, interrupt, timer, watchdog, PMIC, PDC and SMMU blocks are the
Nord ones at the same addresses. ``PLAT=seca`` builds the Nord platform (TF-A
BL2 and BL31 booting natively) with ``NORD_SECA=1``, which selects:

- two clusters in the power domain tree, 12 cores,
- the SecaAU APSS SPI trigger types,
- the TCSR PCIe link mode left at its reset value,
- no configuration of the second GPU SMMU.
- the SiP GPU register setup call of the Adreno driver: it enables the
  software feature fuses, the GPU slices the hardware slice mask reports and
  the CBCAST voltage routing. The feature fuse and frequency limit values are
  the Kaanapali ones; the GPU temperature sensor and BCL broadcast routing are
  not set up.

Boot flow, memory layout, signing and the status of the native back-ends are
the ones described for Nord.

How to build
------------

Steps to build TF-A BL2 and the FIP payload::

        $ make CROSS_COMPILE=aarch64-none-elf- PLAT=seca SPD=opteed \
            BL32=<path-to-optee-bin> BL33=<path-to-os-bootloader-bin> fip all

--------------

*Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.*
