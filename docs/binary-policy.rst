Trusted Firmware binary policy
==============================


.. section-numbering::
    :suffix: .

.. contents::

While Trusted Firmware is an open-source project, we understand that some
platform ports may not be able or willing to release all software sources needed
to build a full image under the BSD-3-Clause license. For cases where these
platform ports still contain a significant portion of open-source code that is
contributed back to trustedfirmware.org, the Trusted Firmware project may offer
to host the additional closed-source binary components required to build a full
image in the `Trusted Firmware binary repository`_ so that they can be
integrated cleanly into the build system and are more easily accessible to
developers.

The Trusted Firmware Technical Steering Committee or Government Board will
decide whether to host binary components in this repository on a case by case
basis. Applications for inclusion can be sent to tsc@lists.trustedfirmware.org.
The following policy outlines general guidelines that the Committee/Board will
apply when judging an application.

Scope
-----

This policy is intended for cases where it's not possible to fully open-source a
Trusted Firmware platform port. Binary components should be a minimal as
possible, augmenting the more significant, open-source part of the platform
port. Binary components that contain all meaningful functionality of a platform
port and are merely linked into a boilerplate shell of open-source code will not
be accepted.

Binary components must always be associated with a specific platform port. No
generic code (i.e. any code outside the plat/<vendor> directory) may depend on
a binary component.

License
-------

All binary components must be accompanied by a distribution license. The license
should:

#. be irrevocable

#. be royalty-free

#. grant usage rights for both copyright and patents covering the binary

#. allow unlimited redistribution under the same terms

#. not restrict the ability to publish test results from the platform port

To simplify legal review and avoid potential concerns, we recommend that the
license should be as short and simple as possible. Special terms should be
limited to what is absolutely necessary. The `Permissive Binary License`_
adapted from the Arm Mbed project is a good example of a license that fulfills
these requirements, and contributors are welcome to reuse it for their binaries.
Reusing an already accepted license may greatly speed up and simplify the
acceptance process for a new binary.

When a binary is including third-party code (including open-source code) that is
covered by separate license terms, all those licenses must be uploaded together
with the binary and listed in the release notes.

Release Notes
-------------

All binary components must be clearly versioned and accompanied by a release
notes document that lists the following (updated for each version):

#. version

#. release date

#. full list of all licenses covering (parts of) the binary

#. supported silicon

#. changes since the last version

#. errata, known issues

#. commit hash of the Trusted Firmware sources that it was tested with

Support
-------

As interfaces in open-source Trusted Firmware change and get deprecated, it may
not be possible to keep a platform port functional without updating the binary
components it depends on. Contributors of binary components are expected to
maintain them together with their platform ports and release updates as
necessary. A binary component that no longer works with current builds of
Trusted Firmware and is not being kept up to date may be removed. Likewise, new
binary component updates need to be tested with the latest version of the
open-source platform port (and the latter needs to be updated if necessary)
before they can be accepted.

.. _Trusted Firmware binary repository: https://review.trustedfirmware.org/admin/repos/tf-binaries
.. _Permissive Binary License: https://git.trustedfirmware.org/tf-binaries.git/tree/permissive-binary-license.txt
