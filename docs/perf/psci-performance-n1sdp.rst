PSCI Performance Measurements on N1SDP
======================================

Binaries and Source Trees
-------------------------

- Trusted Firmware [`v2.8-669-g184595713`_]
- TFTF [`v2.8-52-ge0400c6`_]
- SCP/MCP `Prebuilt`_ Images

Procedure
---------

#. Build TFTF with runtime instrumentation enabled:

    .. code:: shell

        make CROSS_COMPILE=aarch64-none-elf- PLAT=n1sdp \
            TESTS=runtime-instrumentation all

#. Build TF-A with the following build options:

    .. code:: shell

        make CROSS_COMPILE=aarch64-none-elf- PLAT=n1sdp \
            ENABLE_RUNTIME_INSTRUMENTATION=1 GENERATE_COT=1 fiptool all

#. Fetch the SCP firmware images:

    .. code:: shell

        curl --fail --connect-timeout 5 --retry 5 \
            -sLS -o build/n1sdp/release/scp_rom.bin \
            https://downloads.trustedfirmware.org/tf-a/css_scp_2.11.0/n1sdp/release/n1sdp-bl1.bin
        curl --fail --connect-timeout 5 \
            --retry 5 -sLS -o build/n1sdp/release/scp_ram.bin \
            https://downloads.trustedfirmware.org/tf-a/css_scp_2.11.0/n1sdp/release/n1sdp-bl2.bin

#. Fetch the MCP firmware images:

    .. code:: shell

        curl --fail --connect-timeout 5 --retry 5 \
            -sLS -o build/n1sdp/release/mcp_rom.bin \
            https://downloads.trustedfirmware.org/tf-a/css_scp_2.11.0/n1sdp/release/n1sdp-mcp-bl1.bin
        curl --fail --connect-timeout 5 --retry 5 \
            -sLS -o build/n1sdp/release/mcp_ram.bin \
            https://downloads.trustedfirmware.org/tf-a/css_scp_2.11.0/n1sdp/release/n1sdp-mcp-bl2.bin

#. Using the fiptool, create new FIP package and append the SCP ram image onto it.

    .. code:: shell

        ./tools/fiptool/fiptool create --blob \
                uuid=cfacc2c4-15e8-4668-82be-430a38fad705,file=build/n1sdp/release/bl1.bin \
                --scp-fw build/n1sdp/release/scp_ram.bin build/n1sdp/release/scp_fw.bin

#. Append the MCP image to the FIP.

    .. code:: shell

        ./tools/fiptool/fiptool create \
            --blob uuid=54464222-a4cf-4bf8-b1b6-cee7dade539e,file=build/n1sdp/release/mcp_ram.bin \
            build/n1sdp/release/mcp_fw.bin

#. Then, add TFTF as the Non-Secure workload in the FIP image:

    .. code:: shell

        make CROSS_COMPILE=aarch64-none-elf- PLAT=n1sdp \
            ENABLE_RUNTIME_INSTRUMENTATION=1 SCP_BL2=/dev/null \
            BL33=<path/to/tftf.bin>  fip

#. Load the following images onto the development board: ``fip.bin``, ``scp_rom.bin``, ``scp_ram.bin``, ``mcp_rom.bin``, and ``mcp_ram.bin``.

.. note::

    These instructions presume you have a complete firmware stack, the N1SDP `user guide`_ provides a detailed explanation on how to get setup from scratch.

Results
-------

``CPU_SUSPEND`` to deepest power level
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. table:: ``CPU_SUSPEND`` latencies (in nanoseconds) to deepest power level in parallel

    +-----------+------+-----------+--------+-------+
    | Cluster   | Core | Powerdown | Wakeup | Cache |
    +===========+======+===========+========+=======+
    | 0         | 0    | 2790      | 410770 | 780   |
    +-----------+------+-----------+--------+-------+
    | 0         | 1    | 1895      | 320440 | 320   |
    +-----------+------+-----------+--------+-------+
    | 1         | 0    | 2820      | 322860 | 280   |
    +-----------+------+-----------+--------+-------+
    | 1         | 1    | 3345      | 324850 | 350   |
    +-----------+------+-----------+--------+-------+


.. table:: ``CPU_SUSPEND`` latencies (in nanoseconds) to deepest power level in serial

    +-----------+------+-----------+--------+-------+
    | Cluster   | Core | Powerdown | Wakeup | Cache |
    +===========+======+===========+========+=======+
    | 0         | 0    | 550       | 410590 | 320   |
    +-----------+------+-----------+--------+-------+
    | 0         | 1    | 1480      | 410150 | 340   |
    +-----------+------+-----------+--------+-------+
    | 1         | 0    | 2290      | 411000 | 555   |
    +-----------+------+-----------+--------+-------+
    | 1         | 1    | 2645      | 318355 | 850   |
    +-----------+------+-----------+--------+-------+

``CPU_SUSPEND`` to power level 0
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. table:: ``CPU_SUSPEND`` latencies (in nanoseconds) to power level 0 in parallel

    +-----------+------+-----------+--------+-------+
    | Cluster   | Core | Powerdown | Wakeup | Cache |
    +===========+======+===========+========+=======+
    | 0         | 0    | 860       | 319930 | 290   |
    +-----------+------+-----------+--------+-------+
    | 0         | 1    | 2065      | 415150 | 385   |
    +-----------+------+-----------+--------+-------+
    | 1         | 0    | 1550      | 415510 | 265   |
    +-----------+------+-----------+--------+-------+
    | 1         | 1    | 1800      | 319990 | 1255  |
    +-----------+------+-----------+--------+-------+

.. table:: ``CPU_SUSPEND`` latencies (in nanoseconds) to power level 0 in serial

    +-----------+------+-----------+--------+-------+
    | Cluster   | Core | Powerdown | Wakeup | Cache |
    +===========+======+===========+========+=======+
    | 0         | 0    | 1475      | 229970 | 2300  |
    +-----------+------+-----------+--------+-------+
    | 0         | 1    | 605       | 139970 | 3290  |
    +-----------+------+-----------+--------+-------+
    | 1         | 0    | 2720      | 410250 | 1045  |
    +-----------+------+-----------+--------+-------+
    | 1         | 1    | 2095      | 410100 | 465   |
    +-----------+------+-----------+--------+-------+

``CPU_OFF`` on all non-lead CPUs
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

``CPU_OFF`` on all non-lead CPUs in sequence then, ``CPU_SUSPEND`` on the lead core to the deepest power level.

.. table:: ``CPU_OFF`` latencies (in nanoseconds) on all non-lead CPUs

    +-----------+------+-----------+--------+-------+
    | Cluster   | Core | Powerdown | Wakeup | Cache |
    +===========+======+===========+========+=======+
    | 0         | 0    | 170       | 410545 | 310   |
    +-----------+------+-----------+--------+-------+
    | 0         | 1    | 13225     | 6945   | 230   |
    +-----------+------+-----------+--------+-------+
    | 1         | 0    | 14030     | 10490  | 275   |
    +-----------+------+-----------+--------+-------+
    | 1         | 1    | 10680     | 13305  | 255   |
    +-----------+------+-----------+--------+-------+


``CPU_VERSION`` in parallel
~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. table:: ``CPU_VERSION`` latency (in nanoseconds) in parallel on all cores

    +-----------+------+---------+
    | Cluster   | Core | Total   |
    +===========+======+=========+
    | 0         | 0    | 80      |
    +-----------+------+---------+
    | 0         | 1    | 215     |
    +-----------+------+---------+
    | 1         | 0    | 280     |
    +-----------+------+---------+
    | 1         | 1    | 270     |
    +-----------+------+---------+

--------------

*Copyright (c) 2023, Arm Limited. All rights reserved.*

.. _v2.8-669-g184595713: https://review.trustedfirmware.org/plugins/gitiles/TF-A/trusted-firmware-a/+/refs/heads/v2.8-669-g184595713
.. _v2.8-52-ge0400c6: https://review.trustedfirmware.org/plugins/gitiles/TF-A/tf-a-tests/+/refs/heads/v2.8-52-ge0400c6
.. _user guide: https://gitlab.arm.com/arm-reference-solutions/arm-reference-solutions-docs/-/blob/master/docs/n1sdp/user-guide.rst
.. _Prebuilt:  https://downloads.trustedfirmware.org/tf-a/css_scp_2.11.0/n1sdp/release/