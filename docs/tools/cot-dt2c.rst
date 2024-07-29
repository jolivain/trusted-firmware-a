TF-A CoT dt2c Tool
=======================

This tool is used to automatically generate the coresponding c file for a 
CoT DT file. Since currently TF-A support two type of CoT file: static c file
and CoT DT binding. This is error prone and hard to maintain, therefore this 
tool can generate the c file for the platform that does not support CoT DT 
binding, given the CoT DT file so the c file can be deprecated.

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
    validate-cot
    visualize-cot
    validate-dt


This command run a sanity checks to test the installation
of the cot-dt2c tool

.. code::

    make test


To convert the certificate

.. code::

    cot-dt2c convert-to-c [INPUT DTB PATH] [OUTPUT C PATH]
    cot-dt2c convert-to-c fdts/tbbr_cot_descriptors.dtsi test.c


To validate the certificate

.. code::

    cot-dt2c validate-cot [INPUT DTB PATH]
    cot-dt2c validate-cot fdts/tbbr_cot_descriptors.dtsi


The tests folder in the tool folder also provides some bad-example of the 
DT file, and the tool will throw exception on those files

Validator
~~~~~~~~~~~~~~~
The script will check the format of the CoT file while converting it to c file
1. The open bracket
2. The open ifdef macro
3. The missing mandatory attribute
4. Malformed DT file (cert missing parent, missing root certs. etc.)


To visualize the certificate

.. code::

    cot-dt2c visualize-cot [INPUT DTB PATH]
    cot-dt2c visualize-cot fdts/tbbr_cot_descriptors.dtsi

This is an example of the attribute getter functionality that the tool
support


To validate other dtsi/dts file

.. code::

    cot-dt2c validate-dt [INPUT DTS PATH or INPUT DTS folder]
    cot-dt2c validate-dt fdts/
    cot-dt2c validate-dt fdts/fvp-bsae-gicv3.dtsi


*Copyright (c) 2024, Arm Limited. All rights reserved.*

.. _Poetry: https://python-poetry.org/docs/
