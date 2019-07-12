========
Debug FS
========

.. contents::

Overview
--------

The *DebugFS* feature is primarily aimed at exposing firmware debug data to
higher SW layers such as a non-secure component. Such component can be the
TFTF test payload or a Linux kernel module.

Virtual filesystem
------------------

The core functionality lies in a virtual file system based on a 9p file server
interface (`Notes on the Plan 9 Kernel Source`_). The implementation permits
exposing virtual files, firmware drivers, and file blobs.

Namespace
~~~~~~~~~

Two namespaces are exposed:

  - # is used as root for drivers (e.g. #t0 is the first uart)
  - / is used as root for virtual "files" (e.g. /fip, or /dev/uart)

9p interface
~~~~~~~~~~~~

The associated primitives are:

- Unix-like:

  - open(): create a file descriptor that acts as a handle to the file passed as
    an argument.
  - close(): close the file descriptor created by open().
  - read(): read from a file to a buffer.
  - write(): write from a buffer to a file.
  - seek(): set the file position indicator of a file descriptor either to a
    relative or an absolute offset.
  - stat(): get information about a file (type, mode, size, ...).

.. code:: c

    int open(const char *name, int flags);
    int close(int fd);
    int read(int fd, void *buf, int n);
    int write(int fd, void *buf, int n);
    int seek(int fd, long off, int whence);
    int stat(char *path, dir_t *dir);

- Specific primitives :

  - mount(): create a link between a driver and spec.
  - create(): create a file in a specific location.
  - bind(): expose the content of a directory to another directory.

.. code:: c

    int mount(char *srv, char *mnt, char *spec);
    int create(const char *name, int flags);
    int bind(char *path, char *where);

This interface is embedded into the BL31 run-time payload when selected by build
options. The interface multiplexes drivers or emulated "files":

- Debug data can be partitioned into different virtual files e.g. expose PMF
  measurements through a file, and internal firmware state counters through
  another file.
- This permits direct access to a firmware driver, mainly for test purposes
  (e.g. a hardware device that may not be accessible to non-privileged/
  non-secure layers, or for which no support exists in the NS side).

SMC interface
-------------

The communication with the 9p layer in BL31 is made through an SMC conduit
(`SMC Calling Convention PDD`_), using a specific SiP Function Id. An NS shared
buffer is used to pass path string parameters, or e.g. to exchange data on a
read operation.

String parameters are passed through the shared buffer using a specific union:

.. code:: c

    union debugfs_parms {
        struct {
            char fname[MAX_PATH_LEN];
        } open;

        struct mount {
            char srv[MAX_PATH_LEN];
            char where[MAX_PATH_LEN];
            char spec[MAX_PATH_LEN];
        } mount;

        struct {
            char path[MAX_PATH_LEN];
            dir_t dir;
        } stat;

        struct {
            char oldpath[MAX_PATH_LEN];
            char newpath[MAX_PATH_LEN];
        } bind;
    };

MOUNT
~~~~~

Description
^^^^^^^^^^^
This operation mounts a blob of data pointed to by path stored in `src`, at
filesystem location pointed to by path stored in `where`, using driver pointed
to by path in `spec`.

Parameters
^^^^^^^^^^
======== ============================================================
uint32_t FunctionID (0x0x82000021)
uint32_t 0 (MOUNT)
======== ============================================================

Return values
^^^^^^^^^^^^^

=============== ==========================================================
int32_t         0 on success

                Non-zero if mount operation failed.
=============== ==========================================================

OPEN
~~~~

Description
^^^^^^^^^^^
This operation opens the file path pointed to by `fname`.

Parameters
^^^^^^^^^^

======== ============================================================
uint32_t FunctionID (0x0x82000021)
uint32_t 2 (OPEN)
uint32_t mode
======== ============================================================

mode can be one of:

.. code:: c

    enum mode {
        O_READ   = 1 << 0,
        O_WRITE  = 1 << 1,
        O_RDWR   = 1 << 2,
        O_BIND   = 1 << 3,
        O_DIR    = 1 << 4,
        O_STAT   = 1 << 5
    };

Return values
^^^^^^^^^^^^^

=============== ==========================================================
int32_t         File descriptor id on success.

                <= 0 if open operation failed.
=============== ==========================================================

CLOSE
~~~~~

Description
^^^^^^^^^^^

Parameters
^^^^^^^^^^

======== ============================================================
uint32_t FunctionID (0x0x82000021)
uint32_t 3 (CLOSE)
======== ============================================================

Return values
^^^^^^^^^^^^^

READ
~~~~

Description
^^^^^^^^^^^

This operation reads a number of bytes from a file descriptor obtained by
a previous call to OPEN.

Parameters
^^^^^^^^^^

======== ============================================================
uint32_t FunctionID (0x0x82000021)
uint32_t 4 (READ)
uint32_t File descriptor id returned by OPEN
uint32_t Number of bytes to read
======== ============================================================

Return values
^^^^^^^^^^^^^

On success, the read data is retrieved from the shared buffer after the
operation.

=============== ==========================================================
int32_t         Number of bytes read on success.

                <= 0 if read operation failed.
=============== ==========================================================

SEEK
~~~~

Description
^^^^^^^^^^^

Parameters
^^^^^^^^^^

======== ============================================================
uint32_t FunctionID (0x0x82000021)
uint32_t 6 (SEEK)
======== ============================================================

Return values
^^^^^^^^^^^^^

BIND
~~~~

Description
^^^^^^^^^^^

Parameters
^^^^^^^^^^

======== ============================================================
uint32_t FunctionID (0x0x82000021)
uint32_t 7 (BIND)
======== ============================================================

Return values
^^^^^^^^^^^^^

STAT
~~~~

Description
^^^^^^^^^^^

Parameters
^^^^^^^^^^

======== ============================================================
uint32_t FunctionID (0x0x82000021)
uint32_t 8 (STAT)
======== ============================================================

Return values
^^^^^^^^^^^^^

INIT
~~~~

Description
^^^^^^^^^^^
Initial call to setup the shared exchange buffer. Notice if successful once,
subsequent calls fail after a first initialization. The caller maps the same
page frame in its virtual space and uses this buffer to exchange string
parameters with filesystem primitives.

Parameters
^^^^^^^^^^

======== ============================================================
uint32_t FunctionID (0x0x82000021)
uint32_t 10 (INIT)
uint64_t Physical address of the shared buffer.
======== ============================================================

Return values
^^^^^^^^^^^^^

=============== ======================================================
int32_t         0 on success

                Non-zero if already initialized, or internal mapping
                failed.
=============== ======================================================

* Notice identifiers CREATE(1) and WRITE (5) are unimplemented and
  return `SMC_UNK`.

Security considerations
-----------------------

- Due to the nature of the exposed data, the feature is considered experimental
  and importantly **shall only be used in debug builds**.
- Several primitive imply string manipulations and usage of string formats.
- Special care is taken with the shared buffer to avoid TOCTOU attacks.

Limitations
-----------

- In order to setup the shared buffer, the component consuming the interface
  needs to allocate a physical page frame and transmit its address.
- In order to map the shared buffer, BL31 requires enabling the dynamic xlat
  table option.
- Data exchange is limited by the shared buffer length. A large read operation
  might be split into multiple read operations of smaller chunks.
- On concurrent access, a spinlock is implemented in the BL31 service to protect
  the internal work buffer, and re-entrancy into the filesystem layers.
- Notice, a physical device driver if exposed by the firmware may conflict with
  the higher level OS if the latter implements its own driver for the same
  physical device.

Applications
------------

The SMC interface is accessible from an NS environment, that is:

- a test payload, bootloader or hypervisor running at NS-EL2
- a Linux kernel driver running at NS-EL1
- a Linux userspace application through the kernel driver

References
----------

.. [#] `SMC Calling Convention PDD`_
.. [#] `Notes on the Plan 9 Kernel Source`_
.. [#] `Linux 9p remote filesystem protocol`_

*Copyright (c) 2019, Arm Limited and Contributors. All rights reserved.*

.. _SMC Calling Convention PDD: http://infocenter.arm.com/help/topic/com.arm.doc.den0028b/
.. _Notes on the Plan 9 Kernel Source: http://lsub.org/who/nemo/9.pdf
.. _Linux 9p remote filesystem protocol: https://www.kernel.org/doc/Documentation/filesystems/9p.txt
