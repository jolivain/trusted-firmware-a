TFA_ARM_ROTPK_HASH
==================

.. default-domain:: cmake

.. variable:: TFA_ARM_ROTPK_HASH

Used when :variable:`ARM_ROTPK_LOCATION` is ``devel_*``. Specifies the location
of the ROTPK hash. Not expected to be a build option. This defaults to
``plat/arm/board/common/rotpk/*_sha256.bin`` depending on the specified
algorithm. Providing :variable:`TFA_ROT_KEY` enforces generation of the hash
from the :variable:`TFA_ROT_KEY` and overwrites the default hash file.

--------------

*Copyright (c) 2021, Arm Limited and Contributors. All rights reserved.*

