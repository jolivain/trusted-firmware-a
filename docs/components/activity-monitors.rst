Activity Monitors
=================

FEAT_AMUv1 of the Armv8-A architecture introduced the Activity Monitors
extension. This extension describes the architecture for the Activity Monitor
Unit (AMU), an optional non-invasive component for monitoring core events
through a set of 64-bit counters.

When the ``ENABLE_AMU=1`` build option is provided, Trusted Firmware-A sets up
the AMU prior to its exit from EL3, and will save and restore AMU counters as
necessary upon suspend and resume.

Auxiliary counters
------------------

Whilst the architectural description of the AMU describes a fixed number of
architected counters, implementation-defined auxiliary counters may also be
provided. By default Trusted Firmware-A does not enable these counters, as doing
so could risk unintentional leakage of information or side-channel attacks on
some platforms. Instead, platforms may describe their auxiliary counters through
the hardware configuration device tree blob (``HW_CONFIG``). This device tree
blob *must* be passed to BL31 by the platform in order for the auxiliary
counters to be enabled. Failing to do so will cause the BL31 image to assume
that no auxiliary counters are to be enabled.

Device tree nodes (``HW_CONFIG``)
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

In order to support platform-defined auxiliary counters, the ``HW_CONFIG``
device tree specification has been expanded. This adds several new nodes,
described in their own sections below.

An example system offering four cores made up of two clusters, where the cores
of each cluster share different AMUs, may use something like the following:

.. code-block::

    cpus {
        #address-cells = <2>;
        #size-cells = <0>;

        amus {
            amu0: amu@0 {
                #address-cells = <1>;
                #size-cells = <0>;

                counterX: counter@0 {
                    reg = <0>;

                    enable-at-el3;
                };

                counterY: counter@1 {
                    reg = <1>;

                    enable-at-el3;
                };
            };

            amu1: amu@1 {
                #address-cells = <1>;
                #size-cells = <0>;

                counterZ: counter@0 {
                        reg = <0>;

                        enable-at-el3;
                };
            };
        };

        cpu0@00000 {
            ...

            amu = <&amu0>;
        };

        cpu1@00100 {
            ...

            amu = <&amu0>;
        };

        cpu2@10000 {
            ...

            amu = <&amu1>;
        };

        cpu3@10100 {
            ...

            amu = <&amu1>;
        };
    }

In this situation, ``cpu0`` and ``cpu1`` (the two cores in the first cluster),
share the view of their AMUs defined by ``amu0``. Likewise, ``cpu2`` and
``cpu3`` (the two cores in the second cluster), share the view of their AMUs
defined by ``amu1``. This will cause ``counterX`` and ``counterY`` to be enabled
for both ``cpu0`` and ``cpu1``, and ``counterZ`` to be enabled for both ``cpu2``
and ``cpu3``.

``/cpus/cpus/cpu*`` node properties
"""""""""""""""""""""""""""""""""""

The ``cpu`` node has been augmented to support a handle to an associated AMU
view, which should describe the counters offered by the core.

+---------------+-------+---------------+------------------------------------+
| Property name | Usage | Value type    | Description                        |
+===============+=======+===============+====================================+
| ``amu``       | O     | ``<phandle>`` | If presents, indicates that an AMU |
|               |       |               | is available and its counters are  |
|               |       |               | described by the node provided.    |
+---------------+-------+---------------+------------------------------------+

``/cpus/amus`` node properties
""""""""""""""""""""""""""""""

The ``amus`` node describes the AMUs implemented by the cores in the system.
This node does not have any properties.

``/cpus/amus/amu*`` node properties
"""""""""""""""""""""""""""""""""""

An ``amu`` node describes the layout and meaning of the auxiliary counter
registers of one or more AMUs, and may be common across multiple cores.

+--------------------+-------+------------+------------------------------------+
| Property name      | Usage | Value type | Description                        |
+====================+=======+============+====================================+
| ``#address-cells`` | R     | ``<u32>``  | Value shall be 1. Specifies that   |
|                    |       |            | the ``reg`` property array of      |
|                    |       |            | children of this node uses a       |
|                    |       |            | single cell.                       |
+--------------------+-------+------------+------------------------------------+
| ``#size-cells``    | R     | ``<u32>``  | Value shall be 0. Specifies that   |
|                    |       |            | no size is required in the ``reg`` |
|                    |       |            | property in children of this node. |
+--------------------+-------+------------+------------------------------------+

``/cpus/amus/amu*/counter*`` node properties
""""""""""""""""""""""""""""""""""""""""""""

A ``counter`` node describes an AMU auxiliary counter belonging to the parent
Activity Monitor Unit.

+-------------------+-------+-------------+------------------------------------+
| Property name     | Usage | Value type  | Description                        |
+===================+=======+=============+====================================+
| ``reg``           | R     | array       | Represents the counter register    |
|                   |       |             | index, and must be a single cell.  |
+-------------------+-------+-------------+------------------------------------+
| ``enable-at-el3`` | O     | ``<empty>`` | The presence of this property      |
|                   |       |             | indicates that this counter should |
|                   |       |             | be enabled prior to EL3 exit.      |
+-------------------+-------+-------------+------------------------------------+

--------------

*Copyright (c) 2021, Arm Limited. All rights reserved.*
