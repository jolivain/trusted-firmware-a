Vendor Specific EL3 Monitor Service Calls
=========================================

This document enumerates and describes the Vendor Specific EL3 Monitor Service
Calls.

These are Service Calls defined by the vendor of the EL3 Monitor.
They are accessed via ``SMC`` ("SMC calls") instruction executed from Exception
Levels below EL3. SMC calls for Vendor Specific EL3 Monitor Services:

-  Follow `SMC Calling Convention`_;
-  Use SMC function IDs that fall in the vendor-specific EL3 range, which are

+---------------------------+--------------------------------------------------+
| SMC Function Identifier   | Service Type                                     |
+===========================+==================================================+
| 0x87000000 - 0x8700FFFF   | SMC32: Vendor Specific EL3 Monitor Service Calls |
+---------------------------+--------------------------------------------------+
| 0xC7000000 - 0xC700FFFF   | SMC64: Vendor Specific EL3 Monitor Service Calls |
+---------------------------+--------------------------------------------------+

Vendor-specific EL3 monitor services are as follows:

+-----------------------------------+-----------------------+---------------------------------------------+
| SMC Function Identifier           | Service Type          | FID's Usage                                 |
+===================================+=======================+=============================================+
| 0x87000010 - 0x8700001F (SMC32)   | DebugFS Interface     | | 0 - 11 are in use.                        |
+-----------------------------------+                       | | 12 - 15 are reserved for future expansion.|
| 0xC7000010 - 0xC700001F (SMC64)   |                       |                                             |
+-----------------------------------+-----------------------+---------------------------------------------+
| 0x87000020 - 0x8700002F (SMC32)   | Performance           | | 0 is in use.                              |
+-----------------------------------+ Measurement Framework | | 1 - 15 are reserved for future expansion. |
| 0xC7000020 - 0xC700002F (SMC64)   | (PMF)                 |                                             |
+-----------------------------------+-----------------------+---------------------------------------------+
| 0x87000030 - 0x8700FFFF (SMC32)   | Reserved              | | reserved for future expansion             |
+-----------------------------------+                       |                                             |
| 0xC7000030 - 0xC700FFFF (SMC64)   |                       |                                             |
+-----------------------------------+-----------------------+---------------------------------------------+

Source definitions for vendor-specific EL3 Monitor Service Calls are located in
the ``ven_el3_svc.h`` header file.

+----------------------------+----------------------------+--------------------------------+
| VEN_EL3_SVC_VERSION_MAJOR  | VEN_EL3_SVC_VERSION_MINOR  | Changes                        |
+============================+============================+================================+
|                          1 |                          0 | Added Debugfs and PMF services.|
+----------------------------+----------------------------+--------------------------------+

*Table 1: Showing different versions of Vendor-specific service and changes done with each version*

Some ground rules and examples when one should update VEN_EL3_SVC_VERSION_MAJOR and VEN_EL3_SVC_VERSION_MINOR

+---------------------------------------+-------------------------------+--------------------------+
| Usecase                               | Example                       | Version to Update        |
+=======================================+===============================+==========================+
| Adding/introducing a new monitor      | Adding an new monitor service |                          |
| sub service under el3 range the a new | at 0x30.                      |VEN_EL3_SVC_VERSION_MAJOR |
| fid range to be taken, this will be   | Debugfs starts at 0x10 and PMF|is incremented.           |
| at 16x fid's boundary for current     | starts at 0x20 next one will  |                          |
| use and for future expansion.         | start at 0x30.                |                          |
+---------------------------------------+-------------------------------+--------------------------+
| Removing/deprecating a sub service.   | Removing PMF sub service.     |VEN_EL3_SVC_VERSION_MAJOR |
|                                       |                               |is incremented.           |
+---------------------------------------+-------------------------------+--------------------------+
| Amending/modifying/bugfixing any      | Fixing a bug under debugfs    |VEN_EL3_SVC_VERSION_MINOR |
| existing sub service.                 | read function.                |is incremented.           |
+---------------------------------------+-------------------------------+--------------------------+
| Removing/deprecating existing         |                               |                          |
| function under sub service.           | Remove stat function debugfs. |VEN_EL3_SVC_VERSION_MINOR |
| One more fids from sub range to be    |                               |is incremented.           |
| depcreated leaving a hole in existing |                               |                          |
| continuous range.                     |                               |                          |
+---------------------------------------+-------------------------------+--------------------------+
| Adding new function under existing    | Adding a new function under   |                          |
| sub service so new Fid(s) will be     | debugfs like list size        |VEN_EL3_SVC_VERSION_MINOR |
| taken from available fid range.       | function which will use       |is incremented.           |
|                                       | fid 12                        |                          |
+---------------------------------------+-------------------------------+--------------------------+


Performance Measurement Framework (PMF)
---------------------------------------

The :ref:`Performance Measurement Framework <firmware_design_pmf>`
allows callers to retrieve timestamps captured at various paths in TF-A
execution.

DebugFS interface
-----------------

The optional DebugFS interface is accessed through Vendor specific EL3 service. Refer
to :ref:`DebugFS interface` documentation for further details and usage.

--------------

*Copyright (c) 2024, Arm Limited and Contributors. All rights reserved.*

.. _SMC Calling Convention: https://developer.arm.com/docs/den0028/latest
