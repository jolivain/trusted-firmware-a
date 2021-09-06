TFA_WORKAROUND_CVE_2017_5715
============================

.. default-domain:: cmake

.. variable:: TFA_WORKAROUND_CVE_2017_5715

Enables the security workaround for `CVE-2017-5715`_. This option can be
disabled by the platform if none of the PEs in the system need the
workaround. Disabling tihs option provides no performance benefit for
non-affected platforms, it just helps to comply with the recommendation in
the specification regarding workaround discovery.

Enabled by default.

.. _CVE-2017-5715: http://cve.mitre.org/cgi-bin/cvename.cgi?name=CVE-2017-5715

--------------

*Copyright (c) 2021, Arm Limited and Contributors. All rights reserved.*
