TFA_BRANCH_PROTECTION_FLAGS
===========================

.. default-domain:: cmake

.. variable:: TFA_BRANCH_PROTECTION_FLAGS

    Optional additional compiler flags for enabling branch protection features
    introduced in Armv8.3-A and upwards. This option is only available when
    targeting Armv8.2-A or greater.

    .. warning::

        Be careful that you do not introduce compiler flags incompatible with
        the architecture version you are targeting. For instance, while the
        Pointer Authentication extension was introduced in Armv8.3-A, the Branch
        Target Identification extension was not introduced until Armv8.5-A.

    Depending on the level of support offered by your particular compiler, some
    known possible options may be presented in the GUI.

    For compilers that support the GNU-style ``-mbranch-protection`` option,
    these values are offered:

    +----------------------------+------------------------+-----+
    | ``-mbranch-protection=*``  | PAC                    | BTI |
    |                            +-------+-------+--------+     |
    |                            | A-key | B-key | Leaves |     |
    +============================+=======+=======+========+=====+
    | ``pac-ret``                | ✓     |       |        |     |
    +----------------------------+-------+-------+--------+-----+
    | ``pac-ret+b-key``          |       | ✓     |        |     |
    +----------------------------+-------+-------+--------+-----+
    | ``pac-ret+leaf``           | ✓     |       | ✓      |     |
    +----------------------------+-------+-------+--------+-----+
    | ``pac-ret+b-key+leaf``     |       | ✓     | ✓      |     |
    +----------------------------+-------+-------+--------+-----+
    | ``bti``                    |       |       |        | ✓   |
    +----------------------------+-------+-------+--------+-----+
    | ``pac-ret+bti``            | ✓     |       |        | ✓   |
    +----------------------------+-------+-------+--------+-----+
    | ``pac-ret+b-key+bti``      |       | ✓     |        | ✓   |
    +----------------------------+-------+-------+--------+-----+
    | ``pac-ret+leaf+bti``       | ✓     |       | ✓      | ✓   |
    +----------------------------+-------+-------+--------+-----+
    | ``pac-ret+b-key+leaf+bti`` |       | ✓     | ✓      | ✓   |
    +----------------------------+-------+-------+--------+-----+

    Compilers that support these extensions are expected to provide the relevant
    |ACLE| preprocessor definitions:

    - Branch Target Identification: ``__ARM_FEATURE_BTI_DEFAULT``
    - Pointer Authentication: ``__ARM_FEATURE_PAC_DEFAULT``

--------------

*Copyright (c) 2021, Arm Limited and Contributors. All rights reserved.*
