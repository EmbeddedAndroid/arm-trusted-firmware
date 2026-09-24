Qualcomm Platforms
==================

.. toctree::
   :maxdepth: 1
   :caption: Contents

   chrome
   msm8916
   rb3gen2
   lemans_evk
   monza

Debugging over USB with EUD
---------------------------

Qualcomm SoCs embed an Embedded USB Debugger (EUD), a USB 2.0 hub in the
path of the primary USB port. On the platforms that boot through TF-A BL2,
a build with ``DEBUG=1`` puts EUD in debug mode from BL2 early platform
setup. The host then sees the EUD control peripheral (USB ID
``05c6:9501``) on the cable used for flashing, and a debugger such as
OpenOCD configured with ``--enable-eud`` can reach the SoC debug access
port over it. Pass ``QTI_EUD_ENABLE=0`` to leave EUD in bypass mode in a
debug build.

--------------

*Copyright (c) 2025, Qualcomm Technologies, Inc. and/or its subsidiaries.*
