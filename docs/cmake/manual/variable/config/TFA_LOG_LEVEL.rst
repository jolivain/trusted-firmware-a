TFA_LOG_LEVEL
=============

.. default-domain:: cmake

.. variable:: TFA_LOG_LEVEL

    Chooses the log level, which controls the amount of console log output
    compiled into the build. This should be one of the following:

    - ``None``: No log output
    - ``Error``: Error log output
    - ``Notice``: Notice log output
    - ``Warning``: Warning log output
    - ``Info``: Informational log output
    - ``Verbose``: Verbose log output

    All log output up to and including the selected log level is compiled into
    the build.

    Defaults to ``Info`` if building in debug mode, otherwise to ``Notice``.


--------------

*Copyright (c) 2021, Arm Limited and Contributors. All rights reserved.*
