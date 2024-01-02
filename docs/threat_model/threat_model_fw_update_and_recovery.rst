Threat Model for TF-A with PSA FWU or TBBR FWU support
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Introduction
************

This document provides a threat model of TF-A firmware for platforms with
the feature PSA firmware update or TBBR firmware update or both are enabled.

Although it is a separate document, it references the :ref:`Generic Threat
Model` in a number of places, as some of the contets are applicable to this
threat model.

Target of Evaluation
********************

In this threat model, the target of evaluation is the Trusted Firmware for
A-class Processors (TF-A) when PSA FWU support enabled or TBBR FWU mode
is enabled. This includes the boot ROM (BL1), the trusted boot firmware (BL2).

Threat Assessment
*****************

For this section, please reference the Threat Assessment under the general TF-A
threat-model document, :ref:`Generic Threat Model`. All the threats listed
there are applicable for the AP core, here only the differences are highlighted.

PSA FWU
*******

Threats to be Mitigated by the Boot Firmware
--------------------------------------------

The following table analyses the :ref:`General Threats` in the context of this
threat model. Only additional details are pointed out.

+----+-------------+-------------------------------------------------------+
| ID | Applicable? | Comments                                              |
+====+=============+=======================================================+
| 01 |     Yes     |  | Attacker reset the platform during the update to   |
|    |             |    leave system in the inconsistent way.              |
+----+-------------+-------------------------------------------------------+
| 02 |     Yes     |                                                       |
+----+-------------+-------------------------------------------------------+
| 03 |     Yes     |                                                       |
+----+-------------+-------------------------------------------------------+
| 04 |     Yes     |                                                       |
+----+-------------+-------------------------------------------------------+


Threats to be mitigated by platform design
------------------------------------------

FWU metadata corruption:

1. Attacker tries to alter FWU data to avoid running the updated
   firmware.
2. Attacker alters the FWU metadata to prevent anti-rollback update.

The safeguarding of FWU metadata relies on the platform's robust hardware
design to mitigate potential attacks on it.

TBBR FWU - Firmware Recovery
****************************

Threats to be Mitigated by the Boot Firmware
--------------------------------------------

+----+-------------+-------------------------------------------------------+
| ID | Applicable? | Comments                                              |
+====+=============+=======================================================+
| 01 |     Yes     | | Attacker can use arbitrary images to recover the    |
|    |             |   system and also, can try to recover the system with |
|    |             |   the vulnerable/older firmware.                      |
+----+-------------+-------------------------------------------------------+
| 02 |     Yes     |                                                       |
+----+-------------+-------------------------------------------------------+
| 03 |     Yes     |                                                       |
+----+-------------+-------------------------------------------------------+
| 04 |     Yes     |                                                       |
+----+-------------+-------------------------------------------------------+
