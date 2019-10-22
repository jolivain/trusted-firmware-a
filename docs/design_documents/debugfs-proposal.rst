============================
TF-A DebugFS Design Proposal
============================

:Author: Olivier Deprez
:Organization: Arm Limited
:Contact: Olivier Deprez <olivier.deprez@arm.com>
:Status: Draft

.. contents::

Overview
--------

The purpose of this document is to present a proposal to a new (experimental)
debug feature called *DebugFS*. It is primarily aimed at exposing firmware
debug data to higher SW layers such as a non-secure component. Such component
can be the TFTF test payload or a Linux kernel module.

Virtual filesystem
^^^^^^^^^^^^^^^^^^

The core of the functionality lies in an virtual file system based on a 9p
file server interface (`Notes on the Plan 9 Kernel Source`_). The implementation
permits exposing virtual files, firmware drivers, and file blobs.

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
^^^^^^^^^^^^^

The communication with the 9p layer in BL31 is made through an SMC conduit
(`SMC Calling Convention PDD`_), using a specific SiP Function Id. An NS shared
buffer is used to pass path string parameters, or e.g. to exchange data on a
read operation.

Security considerations
-----------------------

- Due to the nature of the exposed data, the feature is considered experimental
  and importantly **only available in debug builds**.
- Several primitive imply string manipulations and usage of string formats. If
  this proposal is implemented, a thorough code review and threat analysis is
  expected, in addition to specific testing.
- Special care is taken with the shared buffer to avoid TOCTOU attacks.

Limitations
-----------

- In order to setup the shared buffer, the component consuming the interface
  needs to allocate a physical page and transmit its address to the DebugFS.
- Such buffer agreement is for now using an ad-hoc protocol, but more a
  standardized option might be proposed.
- In order to map the shared buffer, BL31 requires enabling the dynamic xlat
  table option.
- Data exchange is limited by the shared buffer length. A large read operation
  might be split into multiple read operations of smaller chunks.
- Concurrency is still an open topic:

  - Multiple cores may call the DebugFS interface resulting in concurrent access
    to internal shared data. At this stage, wrappers implement a mutex to prevent
    such concurrent calls.
  - A physical device driver if exposed by the firmware may conflict with the
    higher level OS if the latter implements its own driver for the same
    physical device.

- The directory hierarchy is currently built from an independent Makefile+script
  system. Longer term, we might think of a more elaborated device tree based
  description.

Applications
------------

The SMC interface is accessible from an NS environment, that is:

- a test payload, bootloader or hypervisor running at NS-EL2
- a Linux kernel driver running at NS-EL1
- a Linux userspace application through the kernel driver

The following applications are possible (non-exhaustive):

- Supply boot and/or live statistics on firmware internals.
- Expose FIP contents:

  - Notice this might require mapping flash space to BL31, or adding flash
    operations to BL31 which is questionable.
  - A possible application is a FW Update test interface: a means to check that
    a firmware update process occurred successfully by dumping the FIP contents
    at TFTF level. Verify checksums before/after the update and compare to known
    checksums values of FIP contents.

- Expose PMF timestamps. An ad-hoc interface already exists for this feature. It
  can be chosen mid-term to transition it to DebugFS.
- Expose PSCI topology

TFTF payload
^^^^^^^^^^^^

The TF-A test payload can be enriched to use the DebugFS feature to probe the
internal TF-A state. The following wrappers may be used for the open and read
primitives. Those wrappers could be supplied as an independent library consumed
by test cases:

.. code:: c

    static int open(const char *name, int flags)
    {
        smc_ret_values ret;
        smc_args args;

        strncpy(payload, name, 256); /* FIXME: arbitrary max path length */

        args.fid  = ARM_SIP_SVC_DRIVER_NG;
        args.arg1 = OPEN;
        args.arg2 = (u_register_t) payload;
        args.arg3 = flags;
        ret = tftf_smc(&args);

        return ret.ret0;
    }

    static int read(int fd, void *buf, size_t size)
    {
        smc_ret_values ret;
        smc_args args;

        args.fid  = ARM_SIP_SVC_DRIVER_NG;
        args.arg1 = READ;
        args.arg2 = (u_register_t) fd;
        args.arg3 = (u_register_t) payload;
        args.arg4 = (u_register_t) size;

        ret = tftf_smc(&args);
        if (ret.ret0 > 0) {
            memcpy(buf, payload, size);
        }

        return ret.ret0;
    }

A test case accesses the filesystem through the wrappers, like it would
naturally use an OS' system calls:

.. code:: c

    /* open root directory */
    fd = open("/", O_READ);
    if (fd < 0) {
        tftf_testcase_printf("open failed fd=%d\n", fd);
        return TEST_RESULT_FAIL;
    }

	/* read directory entries */
    ret = read(fd, &dir, sizeof(dir));
    while (ret > 0) {
        dir_print(&dir);
        ret = read(fd, &dir, sizeof(dir));
    }

    /* close root directory handle */
    ret = close(fd);
    if (ret < 0) {
        tftf_testcase_printf("close failed ret=%d\n", ret);
        return TEST_RESULT_FAIL;
    }

Linux kernel module
^^^^^^^^^^^^^^^^^^^

Linux debug fs
~~~~~~~~~~~~~~

9p primitives may be "tunneled" to the linux debug filesystem. Below are
wrappers for open and read primitives:

.. code:: c

    static int open(const char *name, int flags)
    {
        struct arm_smccc_res res;

        mutex_lock(&debugfs_lock);
        strlcpy(shared_buffer, name, 256);
        arm_smccc_1_1_smc(ARM_SIP_SVC_DEBUGFS,
                          TFA_DEBUGFS_OPEN, SHARED_BUFFER_PHYS, flags, &res);
        mutex_unlock(&debugfs_lock);

       return res.a0;
    }

    static int read(int fd, void *buf, size_t size)
    {
        struct arm_smccc_res res;

        mutex_lock(&debugfs_lock);
        arm_smccc_1_1_smc(ARM_SIP_SVC_DEBUGFS,
                          TFA_DEBUGFS_READ, fd, SHARED_BUFFER_PHYS, size, &res);

        if (res.a0 > 0) {
            memcpy(buf, shared_buffer, size);
        }
        mutex_unlock(&debugfs_lock);

        return res.a0;
    }

By using such wrappers a dedicated kernel driver can enumerate the TF-A DebugFS
hierarchy through the Linux debug file-system:

.. code:: c

    static int tfa_debugfs_rootdir(void)
    {
        int ret, fd;
        dir_t dir;
        struct dentry *dent;

        /* open root directory */
        fd = open("/", __O_READ);
        if (fd < 0) {
            printk(KERN_ERR "open failed %d\n", fd);
            return -1;
        }

        /* read directory entries */
        do {
            ret = read(fd, &dir, sizeof(dir));
            if (ret > 0) {
                if (dir.qid & 0x8000) {
                    dent = debugfs_create_dir(dir.name, tfa_rootdir);
                } else {
                    dent = debugfs_create_file(dir.name, O_RDONLY, tfa_rootdir,
                                               NULL, NULL);
                }

                printk(KERN_INFO "debugfs create %p\n", dent);
           }
        } while (ret > 0);

        /* close root directory handle */
        ret = close(fd);
        if (ret < 0) {
            printk(KERN_ERR "close failed %d\n", ret);
            return -1;
        }

        return 0;
    }

Moving forward, the intent is to create read/write hooks into the kernel driver.
This permits usage of userspace tools such as echo or cat to interact with the
exposed files down to the TF-A firmware:

::

    root@genericarmv8:~# ls -lart /sys/kernel/debug/tfa/
    total 0
    drwx------   23 root     root             0 Jan  1  1970 ..
    drwxr-xr-x    2 root     root             0 Jan  1  1970 fip
    drwxr-xr-x    2 root     root             0 Jan  1  1970 dev
    drwxr-xr-x    2 root     root             0 Jan  1  1970 blobs
    drwxr-xr-x    5 root     root             0 Jan  1  1970 .

9p VFS
~~~~~~

A more ambitious goal could be to hook the interface to Linux' VFS as a remote
file server (`Linux 9p remote filesystem protocol`_).

References
----------

.. [#] `SMC Calling Convention PDD`_
.. [#] `Notes on the Plan 9 Kernel Source`_
.. [#] `Linux 9p remote filesystem protocol`_

*Copyright (c) 2019, Arm Limited and Contributors. All rights reserved.*

.. _SMC Calling Convention PDD: http://infocenter.arm.com/help/topic/com.arm.doc.den0028b/
.. _Notes on the Plan 9 Kernel Source: http://lsub.org/who/nemo/9.pdf
.. _Linux 9p remote filesystem protocol: https://www.kernel.org/doc/Documentation/filesystems/9p.txt
