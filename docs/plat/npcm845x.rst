Nuvoton NPCM845X
================

Nuvoton NPCM845X is the Nuvoton Arbel NPCM8XX Board Management controller (BMC) SoC. 

The Nuvoton Arbel NPCM845X SoC is a fourth-generation BMC.
The NPCM845X computing subsystem comprises a quadcore ARM 
Cortex A35 ARM-V8 architecture.

This SoC was tested on the Arbel NPCM845X evaluation board.


How to Build
------------

.. code:: shell

    make CROSS_COMPILE=aarch64-none-linux-gnu- PLAT=npcm845x all SPD=opteed

