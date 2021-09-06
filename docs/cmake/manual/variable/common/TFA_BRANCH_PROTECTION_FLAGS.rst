TFA_BRANCH_PROTECTION_FLAGS
===========================

.. default-domain:: cmake

.. variable:: TFA_BRANCH_PROTECTION_FLAGS

Optional additional compiler options for enabling the Pointer Authentication
feature (``FEAT_Pauth``) and the Branch Target Identification feature
(``FEAT_BTI``). This option is only available when targeting Armv8.3-A or
greater.

.. note::

    Pointer authentication using the B-key is unsupported.

.. warning::

    Be careful that you do not introduce compiler flags incompatible with the
    architecture version you are targeting. For instance, while the Pointer
    Authentication extension was introduced in Armv8.3-A, the Branch Target
    Identification extension was not introduced until Armv8.5-A.

Depending on the level of support offered by your particular compiler, some
known possible options may be presented in the GUI.

For compilers that use the GNU-style ``-mbranch-protection`` option, these
values are offered (where supported):

+----------------------------+----------------+-----+
| ``-mbranch-protection=*``  | PAC            | BTI |
|                            +-------+--------+     |
|                            | A-key | Leaves |     |
+============================+=======+========+=====+
| ``pac-ret``                | ✓     |        |     |
+----------------------------+-------+--------+-----+
| ``pac-ret+leaf``           | ✓     | ✓      |     |
+----------------------------+-------+--------+-----+
| ``bti``                    |       |        | ✓   |
+----------------------------+-------+--------+-----+
| ``pac-ret+bti``            | ✓     |        | ✓   |
+----------------------------+-------+--------+-----+
| ``pac-ret+leaf+bti``       | ✓     | ✓      | ✓   |
+----------------------------+-------+--------+-----+

Compilers that support these extensions are expected to provide the relevant
|ACLE| preprocessor definitions:

- Branch Target Identification: ``__ARM_FEATURE_BTI_DEFAULT``
- Pointer Authentication: ``__ARM_FEATURE_PAC_DEFAULT``

--------------

*Copyright (c) 2021, Arm Limited and Contributors. All rights reserved.*
