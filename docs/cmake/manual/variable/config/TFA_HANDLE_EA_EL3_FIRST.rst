TFA_HANDLE_EA_EL3_FIRST
=======================

.. default-domain:: cmake

.. variable:: TFA_HANDLE_EA_EL3_FIRST

   Traps External Aborts and SError Interrupt in Exception Level 3 (i.e. in BL31). If
   disabled, these exceptions are instead trapped in the current exception
   level, or in Exeception Level 1 if the current exception level is Exception Level 0.

   Disabled by default.

--------------

*Copyright (c) 2021, Arm Limited and Contributors. All rights reserved.*
