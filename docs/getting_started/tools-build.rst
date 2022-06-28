Building Supporting Tools
=========================

Building and installing OpenSSL 3.0
-----------------------------------

OpenSSL 3.0 has to be used in order to build and run the tools. This section
explains how to build and install OpenSSL 3.0.

Download and build OpenSSL 3.0
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Run the following script with ``sudo`` privilege in order to download and
build OpenSSL 3.0:

.. code:: shell

    TOOLS_DIR=/tmp
    OPENSSL_VER="3.0.2"
    OPENSSL_DIRNAME="openssl-${OPENSSL_VER}"
    OPENSSL_FILENAME="openssl-${OPENSSL_VER}"
    OPENSSL_CHECKSUM="98e91ccead4d4756ae3c9cde5e09191a8e586d9f4d50838e7ec09d6411dfdb63"
    curl --connect-timeout 5 --retry 5 --retry-delay 1 --create-dirs -fsSLo /tmp/${OPENSSL_FILENAME}.tar.gz \
      https://www.openssl.org/source/${OPENSSL_FILENAME}.tar.gz
    echo "${OPENSSL_CHECKSUM}  /tmp/${OPENSSL_FILENAME}.tar.gz" | sha256sum -c
    mkdir -p ${TOOLS_DIR}/${OPENSSL_DIRNAME} && tar -xzf /tmp/${OPENSSL_FILENAME}.tar.gz -C ${TOOLS_DIR}/${OPENSSL_DIRNAME} --strip-components=1
    cd ${TOOLS_DIR}/${OPENSSL_DIRNAME}
    ./Configure --libdir=lib --prefix=/usr --api=1.0.1
    cd ${TOOLS_DIR}
    make -C ${TOOLS_DIR}/${OPENSSL_DIRNAME}

There are two approaches to use the OpenSSL 3.0 build: it can be used as a
local build with no need to install it, or it can be installed on the system,
overriding any previous version of OpenSSL.

Using OpenSSL 3.0 as a local build
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

This approach can be used to avoid installing OpenSSL 3.0 in the system or
modifying any of the installed libraries or packages.

The ``OPENSSL_DIR`` variable can be used when building the tools to indicate
the directory where the OpenSSL build directory is located. For the script
above, this variable can be set at ``${TOOLS_DIR}/${OPENSSL_DIRNAME}``. This
way, the indicated directory will be used to locate the OpenSSL library and
binaries.

In addition to building, as the libaries are normally loaded from the
system's default directories (i.e.: ``/usr/lib`` or ``/usr/local/lib``), it
is necessary to explicitly indicate where the libraries are being loaded from
to prevent loading the default OpenSSL libraries installed in the system.
This is done by setting the ``LD_LIBRARY_PATH`` variable with the directory
where the local libraries are built (e.g.:
``${TOOLS_DIR}/${OPENSSL_DIRNAME}``).

Installing OpenSSL 3.0
^^^^^^^^^^^^^^^^^^^^^^

This installation will override the previous version of OpenSSL.

To install the built project, run the following command:

.. code:: shell

    make -C ${TOOLS_DIR}/${OPENSSL_DIRNAME} install

This will override the previous version of OpenSSL installed under ``/usr``.

*Note: All versions of package ``libssl-dev`` mus be removed in order to
prevent compatibility issues with older versions.*

Building and using the FIP tool
-------------------------------

Firmware Image Package (FIP) is a packaging format used by TF-A to package
firmware images in a single binary. The number and type of images that should
be packed in a FIP is platform specific and may include TF-A images and other
firmware images required by the platform. For example, most platforms require
a BL33 image which corresponds to the normal world bootloader (e.g. UEFI or
U-Boot).

The TF-A build system provides the make target ``fip`` to create a FIP file
for the specified platform using the FIP creation tool included in the TF-A
project. Examples below show how to build a FIP file for FVP, packaging TF-A
and BL33 images.

For AArch64:

.. code:: shell

    make PLAT=fvp BL33=<path-to>/bl33.bin fip

For AArch32:

.. code:: shell

    make PLAT=fvp ARCH=aarch32 AARCH32_SP=sp_min BL33=<path-to>/bl33.bin fip

The resulting FIP may be found in:

::

    build/fvp/<build-type>/fip.bin

For advanced operations on FIP files, it is also possible to independently build
the tool and create or modify FIPs using this tool. To do this, follow these
steps:

It is recommended to remove old artifacts before building the tool:

.. code:: shell

    make -C tools/fiptool clean

Build the tool:

.. code:: shell

    make [OPENSSL_DIR=${TOOLS_DIR}/${OPENSSL_DIRNAME}] [DEBUG=1] [V=1] fiptool

The tool binary can be located in:

::

    [LD_LIBRARY_PATH=${TOOLS_DIR}/${OPENSSL_DIRNAME}] ./tools/fiptool/fiptool

Invoking the tool with ``help`` will print a help message with all available
options.

Example 1: create a new Firmware package ``fip.bin`` that contains BL2 and BL31:

.. code:: shell

    [LD_LIBRARY_PATH=${TOOLS_DIR}/${OPENSSL_DIRNAME} \]
    ./tools/fiptool/fiptool create \
        --tb-fw build/<platform>/<build-type>/bl2.bin \
        --soc-fw build/<platform>/<build-type>/bl31.bin \
        fip.bin

Example 2: view the contents of an existing Firmware package:

.. code:: shell

    [LD_LIBRARY_PATH=${TOOLS_DIR}/${OPENSSL_DIRNAME} \]
    ./tools/fiptool/fiptool info <path-to>/fip.bin

Example 3: update the entries of an existing Firmware package:

.. code:: shell

    # Change the BL2 from Debug to Release version
    [LD_LIBRARY_PATH=${TOOLS_DIR}/${OPENSSL_DIRNAME} \]
    ./tools/fiptool/fiptool update \
        --tb-fw build/<platform>/release/bl2.bin \
        build/<platform>/debug/fip.bin

Example 4: unpack all entries from an existing Firmware package:

.. code:: shell

    # Images will be unpacked to the working directory
    [LD_LIBRARY_PATH=${TOOLS_DIR}/${OPENSSL_DIRNAME} \]
    ./tools/fiptool/fiptool unpack <path-to>/fip.bin

Example 5: remove an entry from an existing Firmware package:

.. code:: shell

    [LD_LIBRARY_PATH=${TOOLS_DIR}/${OPENSSL_DIRNAME} \]
    ./tools/fiptool/fiptool remove \
        --tb-fw build/<platform>/debug/fip.bin

Note that if the destination FIP file exists, the create, update and
remove operations will automatically overwrite it.

The unpack operation will fail if the images already exist at the
destination. In that case, use -f or --force to continue.

More information about FIP can be found in the :ref:`Firmware Design` document.

.. _tools_build_cert_create:

Building the Certificate Generation Tool
----------------------------------------

The ``cert_create`` tool is built as part of the TF-A build process when the
``fip`` make target is specified and TBB is enabled (as described in the
previous section), but it can also be built separately with the following
command:

.. code:: shell

    make [OPENSSL_DIR=${TOOLS_DIR}/${OPENSSL_DIRNAME}] \
        PLAT=<platform> [DEBUG=1] [V=1] certtool

For platforms that require their own IDs in certificate files, the generic
'cert_create' tool can be built with the following command. Note that the target
platform must define its IDs within a ``platform_oid.h`` header file for the
build to succeed.

.. code:: shell

    make [OPENSSL_DIR=${TOOLS_DIR}/${OPENSSL_DIRNAME}] \
        PLAT=<platform> USE_TBBR_DEFS=0 [DEBUG=1] [V=1] certtool

``DEBUG=1`` builds the tool in debug mode. ``V=1`` makes the build process more
verbose. The following command should be used to obtain help about the tool:

.. code:: shell

    [LD_LIBRARY_PATH=${TOOLS_DIR}/${OPENSSL_DIRNAME} \]
    ./tools/cert_create/cert_create -h

.. _tools_build_enctool:

Building the Firmware Encryption Tool
-------------------------------------

The ``encrypt_fw`` tool is built as part of the TF-A build process when the
``fip`` make target is specified, DECRYPTION_SUPPORT and TBB are enabled, but
it can also be built separately with the following command:

.. code:: shell

    make [OPENSSL_DIR=${TOOLS_DIR}/${OPENSSL_DIRNAME}] \
        PLAT=<platform> [DEBUG=1] [V=1] enctool

``DEBUG=1`` builds the tool in debug mode. ``V=1`` makes the build process more
verbose. The following command should be used to obtain help about the tool:

.. code:: shell

    [LD_LIBRARY_PATH=${TOOLS_DIR}/${OPENSSL_DIRNAME} \]
    ./tools/encrypt_fw/encrypt_fw -h

Note that the enctool in its current implementation only supports encryption
key to be provided in plain format. A typical implementation can very well
extend this tool to support custom techniques to protect encryption key.

Also, a user may choose to provide encryption key or nonce as an input file
via using ``cat <filename>`` instead of a hex string.

--------------

*Copyright (c) 2019, Arm Limited. All rights reserved.*
