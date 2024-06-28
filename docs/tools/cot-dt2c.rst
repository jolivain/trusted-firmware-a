Prerequisites
~~~~~~~~~~~~~

#. Python (3.8 or later)
#. `Poetry`_ Python package manager

Getting Started
~~~~~~~~~~~~~~~

Install the tool 

.. code::
    
    make install


Verify its installation

.. code::
    
    cot-dt2c

This command will output the usage for this command

   Usage: cot-dt2c [OPTIONS] COMMAND [ARGS]...

    Options:
    --version  Show the version and exit.
    --help     Show this message and exit.

    Commands:
    convert-to-c


To test the installation

.. code::
    
    make test


To convert the certificate

.. code::
    
    cot-dt2c convert-to-c [INPUT DTB PATH] [OUTPUT C PATH]

*Copyright (c) 2024, Arm Limited. All rights reserved.*
