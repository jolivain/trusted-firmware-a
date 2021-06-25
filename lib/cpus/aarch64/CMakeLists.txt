#
# Copyright (c) 2021, ARM Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

add_library(tfa-lib-cpu-cortex-a35 INTERFACE)
target_sources(tfa-lib-cpu-cortex-a35 INTERFACE "cortex_a35.S")
target_link_libraries(tfa-lib-cpu-cortex-a35 INTERFACE tfa-lib-cpu-common)

add_library(tfa-lib-cpu-cortex-a53 INTERFACE)
target_sources(tfa-lib-cpu-cortex-a53 INTERFACE "cortex_a53.S")
target_link_libraries(tfa-lib-cpu-cortex-a53 INTERFACE tfa-lib-cpu-common)

add_library(tfa-lib-cpu-cortex-a55 INTERFACE)
target_sources(tfa-lib-cpu-cortex-a55 INTERFACE "cortex_a55.S")
target_link_libraries(tfa-lib-cpu-cortex-a55 INTERFACE tfa-lib-cpu-common)

add_library(tfa-lib-cpu-cortex-a57 INTERFACE)
target_sources(tfa-lib-cpu-cortex-a57 INTERFACE "cortex_a57.S")
target_link_libraries(tfa-lib-cpu-cortex-a57 INTERFACE tfa-lib-cpu-common)

add_library(tfa-lib-cpu-cortex-a65 INTERFACE)
target_sources(tfa-lib-cpu-cortex-a65 INTERFACE "cortex_a65.S")
target_link_libraries(tfa-lib-cpu-cortex-a65 INTERFACE tfa-lib-cpu-common)

add_library(tfa-lib-cpu-cortex-a65ae INTERFACE)
target_sources(tfa-lib-cpu-cortex-a65ae INTERFACE "cortex_a65ae.S")
target_link_libraries(tfa-lib-cpu-cortex-a65ae INTERFACE tfa-lib-cpu-common)

add_library(tfa-lib-cpu-cortex-a72 INTERFACE)
target_sources(tfa-lib-cpu-cortex-a72 INTERFACE "cortex_a72.S")
target_link_libraries(tfa-lib-cpu-cortex-a72 INTERFACE tfa-lib-cpu-common)

add_library(tfa-lib-cpu-cortex-a73 INTERFACE)
target_sources(tfa-lib-cpu-cortex-a73 INTERFACE "cortex_a73.S")
target_link_libraries(tfa-lib-cpu-cortex-a73 INTERFACE tfa-lib-cpu-common)

add_library(tfa-lib-cpu-cortex-a75 INTERFACE)
target_sources(tfa-lib-cpu-cortex-a75 INTERFACE "cortex_a75.S")
target_link_libraries(tfa-lib-cpu-cortex-a75 INTERFACE tfa-lib-cpu-common)

add_library(tfa-lib-cpu-cortex-a76 INTERFACE)
target_sources(tfa-lib-cpu-cortex-a76 INTERFACE "cortex_a76.S")
target_link_libraries(tfa-lib-cpu-cortex-a76 INTERFACE tfa-lib-cpu-common)

add_library(tfa-lib-cpu-cortex-a76ae INTERFACE)
target_sources(tfa-lib-cpu-cortex-a76ae INTERFACE "cortex_a76ae.S")
target_link_libraries(tfa-lib-cpu-cortex-a76ae INTERFACE tfa-lib-cpu-common)

add_library(tfa-lib-cpu-cortex-a77 INTERFACE)
target_sources(tfa-lib-cpu-cortex-a77 INTERFACE "cortex_a77.S")
target_link_libraries(tfa-lib-cpu-cortex-a77 INTERFACE tfa-lib-cpu-common)

add_library(tfa-lib-cpu-cortex-a78 INTERFACE)
target_sources(tfa-lib-cpu-cortex-a78 INTERFACE "cortex_a78.S")
target_link_libraries(tfa-lib-cpu-cortex-a78 INTERFACE tfa-lib-cpu-common)

add_library(tfa-lib-cpu-cortex-a78ae INTERFACE)
target_sources(tfa-lib-cpu-cortex-a78ae INTERFACE "cortex_a78_ae.S")
target_link_libraries(tfa-lib-cpu-cortex-a78ae INTERFACE tfa-lib-cpu-common)

add_library(tfa-lib-cpu-cortex-klein INTERFACE)
target_sources(tfa-lib-cpu-cortex-klein INTERFACE "cortex_klein.S")
target_link_libraries(tfa-lib-cpu-cortex-klein INTERFACE tfa-lib-cpu-common)

add_library(tfa-lib-cpu-cortex-matterhorn INTERFACE)
target_sources(tfa-lib-cpu-cortex-matterhorn INTERFACE "cortex_matterhorn.S")
target_link_libraries(tfa-lib-cpu-cortex-matterhorn INTERFACE tfa-lib-cpu-common)

add_library(tfa-lib-cpu-neoverse-e1 INTERFACE)
target_sources(tfa-lib-cpu-neoverse-e1 INTERFACE "neoverse_e1.S")
target_link_libraries(tfa-lib-cpu-neoverse-e1 INTERFACE tfa-lib-cpu-common)

add_library(tfa-lib-cpu-neoverse-n1 INTERFACE)
target_sources(tfa-lib-cpu-neoverse-n1 INTERFACE "neoverse_n1.S")
target_link_libraries(tfa-lib-cpu-neoverse-n1 INTERFACE tfa-lib-cpu-common)

add_library(tfa-lib-cpu-neoverse-n2 INTERFACE)
target_sources(tfa-lib-cpu-neoverse-n2 INTERFACE "neoverse_n2.S")
target_link_libraries(tfa-lib-cpu-neoverse-n2 INTERFACE tfa-lib-cpu-common)

add_library(tfa-lib-cpu-neoverse-v1 INTERFACE)
target_sources(tfa-lib-cpu-neoverse-v1 INTERFACE "neoverse_v1.S")
target_link_libraries(tfa-lib-cpu-neoverse-v1 INTERFACE tfa-lib-cpu-common)

#[=======================================================================[.rst:
.. cmake:variable:: TFA_WORKAROUND_CVE_2017_5715

   Enables the security workaround for `CVE-2017-5715`_. This option can be
   disabled by the platform if none of the PEs in the system need the
   workaround. Disavling tihs option provides no performance benefit for
   non-affected platforms, it just helps to comply with the recommendation in
   the specification regarding workaround discovery. Enabled by default.

.. _CVE-2017-5715: http://cve.mitre.org/cgi-bin/cvename.cgi?name=CVE-2017-5715
#]=======================================================================]

arm_config_option(
    NAME TFA_WORKAROUND_CVE_2017_5715
    HELP "Enable CVE-2017-5715 workaround?"
    DEFAULT TRUE)

target_compile_definitions(tfa-lib-cpu-common
    INTERFACE "WORKAROUND_CVE_2017_5715=$<BOOL:${TFA_WORKAROUND_CVE_2017_5715}>")

#[=======================================================================[.rst:
.. cmake:variable:: TFA_WORKAROUND_CVE_2018_3639

   Enables the security workaround for `CVE-2018-3639`_. The TF-A project
   recommends keeping this option enabled even on platforms that are unaffected
   by CVE-2018-3639, in order to comply with the recommendation in the
   specification regarding workaround discovery. Enabled by default.

.. _CVE-2018-3639: http://cve.mitre.org/cgi-bin/cvename.cgi?name=CVE-2018-3639
#]=======================================================================]

arm_config_option(
    NAME TFA_WORKAROUND_CVE_2018_3639
    HELP "Enable CVE-2018-3639 workaround?"
    DEFAULT TRUE)

target_compile_definitions(tfa-lib-cpu-common
    INTERFACE "WORKAROUND_CVE_2018_3639=$<BOOL:${TFA_WORKAROUND_CVE_2018_3639}>")

#[=======================================================================[.rst:
.. cmake:variable:: TFA_DYNAMIC_WORKAROUND_CVE_2018_3639

   Enables dynamic mitigation for `CVE-2018-3639`_. This build option should be
   enabled if the target platform contains at least 1 CPU that requires dynamic
   mitigation. Disabled by default.

.. _CVE-2018-3639: http://cve.mitre.org/cgi-bin/cvename.cgi?name=CVE-2018-3639
#]=======================================================================]

arm_config_option(
    NAME TFA_DYNAMIC_WORKAROUND_CVE_2018_3639
    HELP "Enable dynamic workaround for CVE-2018-3639?")

target_compile_definitions(tfa-lib-cpu-common
    INTERFACE "DYNAMIC_WORKAROUND_CVE_2018_3639=$<BOOL:${TFA_DYNAMIC_WORKAROUND_CVE_2018_3639}>")

#[=======================================================================[.rst:
.. cmake:variable:: TFA_ERRATA_A9_794073

   This applies the erratum 794073 workaround to the Cortex-A9 CPU. This needs
   to be enabled for all revisions of this CPU.
#]=======================================================================]

arm_config_option(
    NAME TFA_ERRATA_A9_794073
    HELP "Enable Cortex-A9 workaround for erratum 794073?")

target_compile_definitions(tfa-lib-cpu-common
    INTERFACE "ERRATA_A9_794073=$<BOOL:${TFA_ERRATA_A9_794073}>")

#[=======================================================================[.rst:
.. cmake:variable:: TFA_ERRATA_A15_816470

   This applies errata 816470 workaround to the Cortex-A15 CPU. This needs to be
   enabled only for revision >= r3p0 of the CPU.
#]=======================================================================]

arm_config_option(
    NAME TFA_ERRATA_A15_816470
    HELP "Enable Cortex-A15 workaround for erratum 816470?")

target_compile_definitions(tfa-lib-cpu-common
    INTERFACE "ERRATA_A15_816470=$<BOOL:${TFA_ERRATA_A15_816470}>")

#[=======================================================================[.rst:
.. cmake:variable:: TFA_ERRATA_A15_827671

   This applies errata 827671 workaround to the Cortex-A15 CPU. This needs to be
   enabled only for revision >= r3p0 of the CPU.
#]=======================================================================]

arm_config_option(
    NAME TFA_ERRATA_A15_827671
    HELP "Enable Cortex-A15 workaround for erratum 827671?")

target_compile_definitions(tfa-lib-cpu-common
    INTERFACE "ERRATA_A15_827671=$<BOOL:${TFA_ERRATA_A15_827671}>")

#[=======================================================================[.rst:
.. cmake:variable:: TFA_ERRATA_A17_852421

   This applies errata 852421 workaround to the Cortex-A17 CPU. This needs to be
   enabled only for revision <= r1p2 of the CPU.
#]=======================================================================]

arm_config_option(
    NAME TFA_ERRATA_A17_852421
    HELP "Enable Cortex-A17 workaround for erratum 852421?")

target_compile_definitions(tfa-lib-cpu-common
    INTERFACE "ERRATA_A17_852421=$<BOOL:${TFA_ERRATA_A17_852421}>")

#[=======================================================================[.rst:
.. cmake:variable:: TFA_ERRATA_A17_852423

   This applies errata 852423 workaround to the Cortex-A17 CPU. This needs to be
   enabled only for revision <= r1p2 of the CPU.
#]=======================================================================]

arm_config_option(
    NAME TFA_ERRATA_A17_852423
    HELP "Enable Cortex-A17 workaround for erratum 852423?")

target_compile_definitions(tfa-lib-cpu-common
    INTERFACE "ERRATA_A17_852423=$<BOOL:${TFA_ERRATA_A17_852423}>")

#[=======================================================================[.rst:
.. cmake:variable:: TFA_ERRATA_A35_855472

   This applies errata 855472 workaround to the Cortex-A35 CPUs. This needs to
   be enabled only for revision r0p0 of the Cortex-A35.
#]=======================================================================]

arm_config_option(
    NAME TFA_ERRATA_A35_855472
    HELP "Enable Cortex-A53 workaround for erratum 855472?")

target_compile_definitions(tfa-lib-cpu-common
    INTERFACE "ERRATA_A35_855472=$<BOOL:${TFA_ERRATA_A35_855472}>")

#[=======================================================================[.rst:
.. cmake:variable:: TFA_ERRATA_A53_819472

   This applies errata 819472 workaround to all CPUs. This needs to be enabled
   only for revision <= r0p1 of the Cortex-A53.
#]=======================================================================]

arm_config_option(
    NAME TFA_ERRATA_A53_819472
    HELP "Enable Cortex-A53 workaround for erratum 819472?")

target_compile_definitions(tfa-lib-cpu-common
    INTERFACE "ERRATA_A53_819472=$<BOOL:${TFA_ERRATA_A53_819472}>")

#[=======================================================================[.rst:
.. cmake:variable:: TFA_ERRATA_A53_824069

   This applies errata 824069 workaround to all CPUs. This needs to be enabled
   only for revision <= r0p2 of the Cortex-A53.
#]=======================================================================]

arm_config_option(
    NAME TFA_ERRATA_A53_824069
    HELP "Enable Cortex-A53 workaround for erratum 824069?")

target_compile_definitions(tfa-lib-cpu-common
    INTERFACE "ERRATA_A53_824069=$<BOOL:${TFA_ERRATA_A53_824069}>")

#[=======================================================================[.rst:
.. cmake:variable:: TFA_ERRATA_A53_826319

   This applies errata 826319 workaround to the Cortex-A53 CPU. This needs to be
   enabled only for revision <= r0p2 of the CPU.
#]=======================================================================]

arm_config_option(
    NAME TFA_ERRATA_A53_826319
    HELP "Enable Cortex-A53 workaround for erratum 826319?")

target_compile_definitions(tfa-lib-cpu-common
    INTERFACE "ERRATA_A53_826319=$<BOOL:${TFA_ERRATA_A53_826319}>")

#[=======================================================================[.rst:
.. cmake:variable:: TFA_ERRATA_A53_827319

   This applies errata 827319 workaround to all CPUs. This needs to be enabled
   only for revision <= r0p2 of the Cortex-A53.
#]=======================================================================]

arm_config_option(
    NAME TFA_ERRATA_A53_827319
    HELP "Enable Cortex-A53 workaround for erratum 827319?")

target_compile_definitions(tfa-lib-cpu-common
    INTERFACE "ERRATA_A53_827319=$<BOOL:${TFA_ERRATA_A53_827319}>")

#[=======================================================================[.rst:
.. cmake:variable:: TFA_ERRATA_A53_835769

   This applies erratum 835769 workaround at compile and link time to the
   Cortex-A53 CPU. This needs to be enabled for some variants of revision <=
   r0p4. This workaround can lead the linker to create ``*.stub`` sections.
#]=======================================================================]

arm_config_option(
    NAME TFA_ERRATA_A53_835769
    HELP "Enable Cortex-A53 workaround for erratum 835769?")

target_compile_definitions(tfa-lib-cpu-common
    INTERFACE "ERRATA_A53_835769=$<BOOL:${TFA_ERRATA_A53_835769}>")

#[=======================================================================[.rst:
.. cmake:variable:: TFA_ERRATA_A53_836870

   This applies errata 836870 workaround to the Cortex-A53 CPU. This needs to be
   enabled only for revision <= r0p3 of the CPU. From r0p4 and onwards, this
   errata is enabled by default in hardware.
#]=======================================================================]

arm_config_option(
    NAME TFA_ERRATA_A53_836870
    HELP "Enable Cortex-A53 workaround for erratum 836870?")

target_compile_definitions(tfa-lib-cpu-common
    INTERFACE "ERRATA_A53_836870=$<BOOL:${TFA_ERRATA_A53_836870}>")

#[=======================================================================[.rst:
.. cmake:variable:: TFA_ERRATA_A53_843419

   This applies erratum 843419 workaround at link time to the Cortex-A53 CPU.
   This needs to be enabled for some variants of revision <= r0p4. This
   workaround can lead the linker to emit ``*.stub`` sections which are 4kB
   aligned.
#]=======================================================================]

arm_config_option(
    NAME TFA_ERRATA_A53_843419
    HELP "Enable Cortex-A53 workaround for erratum 843419?")

target_compile_definitions(tfa-lib-cpu-common
    INTERFACE "ERRATA_A53_843419=$<BOOL:${TFA_ERRATA_A53_843419}>")

#[=======================================================================[.rst:
.. cmake:variable:: TFA_ERRATA_A53_855873

   This applies errata 855873 workaround to the Cortex-A53 CPUs. Though the
   erratum is present in every revision of the CPU, this workaround is only
   applied to CPUs from r0p3 onwards, which feature a chicken bit in
   `CPUACTLR_EL1` to enable a hardware workaround. Earlier revisions of the CPU
   have other errata which require the same workaround in software, so they
   should be covered anyway.
#]=======================================================================]

arm_config_option(
    NAME TFA_ERRATA_A53_855873
    HELP "Enable Cortex-A53 workaround for erratum 855873?")

target_compile_definitions(tfa-lib-cpu-common
    INTERFACE "ERRATA_A53_855873=$<BOOL:${TFA_ERRATA_A53_855873}>")

#[=======================================================================[.rst:
.. cmake:variable:: TFA_ERRATA_A53_1530924

   This applies errata 1530924 workaround to all revisions of the Cortex-A53
   CPU.
#]=======================================================================]

arm_config_option(
    NAME TFA_ERRATA_A53_1530924
    HELP "Enable Cortex-A53 workaround for erratum 1530924?")

target_compile_definitions(tfa-lib-cpu-common
    INTERFACE "ERRATA_A53_1530924=$<BOOL:${TFA_ERRATA_A53_1530924}>")

if(TFA_ERRATA_A53_1530924)
    arm_config_option_override(
        NAME TFA_ERRATA_SPECULATIVE_AT
        FORCE TRUE)

    target_compile_definitions(tfa-lib-cpu-common
        INTERFACE "ERRATA_SPECULATIVE_AT=$<BOOL:${TFA_ERRATA_SPECULATIVE_AT}>")
endif()

#[=======================================================================[.rst:
.. cmake:variable:: TFA_ERRATA_A55_768277

   This applies errata 768277 workaround to the Cortex-A55 CPU. This needs to be
   enabled only for revision r0p0 of the CPU.
#]=======================================================================]

arm_config_option(
    NAME TFA_ERRATA_A55_768277
    HELP "Enable Cortex-A55 workaround for erratum 768277?")

target_compile_definitions(tfa-lib-cpu-common
    INTERFACE "ERRATA_A55_768277=$<BOOL:${TFA_ERRATA_A55_768277}>")

#[=======================================================================[.rst:
.. cmake:variable:: TFA_ERRATA_A55_778703

   This applies errata 778703 workaround to the Cortex-A55 CPU. This needs to be
   enabled only for revision r0p0 of the CPU.
#]=======================================================================]

arm_config_option(
    NAME TFA_ERRATA_A55_778703
    HELP "Enable Cortex-A55 workaround for erratum 778703?")

target_compile_definitions(tfa-lib-cpu-common
    INTERFACE "ERRATA_A55_778703=$<BOOL:${TFA_ERRATA_A55_778703}>")

#[=======================================================================[.rst:
.. cmake:variable:: TFA_ERRATA_A55_798797

   This applies errata 798797 workaround to the Cortex-A55 CPU. This needs to be
   enabled only for revision r0p0 of the CPU.
#]=======================================================================]

arm_config_option(
    NAME TFA_ERRATA_A55_798797
    HELP "Enable Cortex-A55 workaround for erratum 798797?")

target_compile_definitions(tfa-lib-cpu-common
    INTERFACE "ERRATA_A55_798797=$<BOOL:${TFA_ERRATA_A55_798797}>")

#[=======================================================================[.rst:
.. cmake:variable:: TFA_ERRATA_A55_846532

   This applies errata 846532 workaround to the Cortex-A55 CPU. This needs to be
   enabled only for revision <= r0p1 of the CPU.
#]=======================================================================]

arm_config_option(
    NAME TFA_ERRATA_A55_846532
    HELP "Enable Cortex-A55 workaround for erratum 846532?")

target_compile_definitions(tfa-lib-cpu-common
    INTERFACE "ERRATA_A55_846532=$<BOOL:${TFA_ERRATA_A55_846532}>")

#[=======================================================================[.rst:
.. cmake:variable:: TFA_ERRATA_A55_903758

   This applies errata 903758 workaround to the Cortex-A55 CPU. This needs to be
   enabled only for revision <= r0p1 of the CPU.
#]=======================================================================]

arm_config_option(
    NAME TFA_ERRATA_A55_903758
    HELP "Enable Cortex-A55 workaround for erratum 903758?")

target_compile_definitions(tfa-lib-cpu-common
    INTERFACE "ERRATA_A55_903758=$<BOOL:${TFA_ERRATA_A55_903758}>")

#[=======================================================================[.rst:
.. cmake:variable:: TFA_ERRATA_A55_1221012

   This applies errata 1221012 workaround to the Cortex-A55 CPU. This needs to
   be enabled only for revision <= r1p0 of the CPU.
#]=======================================================================]

arm_config_option(
    NAME TFA_ERRATA_A55_1221012
    HELP "Enable Cortex-A55 workaround for erratum 1221012?")

target_compile_definitions(tfa-lib-cpu-common
    INTERFACE "ERRATA_A55_1221012=$<BOOL:${TFA_ERRATA_A55_1221012}>")

#[=======================================================================[.rst:
.. cmake:variable:: TFA_ERRATA_A55_1530923

   This applies errata 1530923 workaround to all revisions of the Cortex-A55
   CPU.
#]=======================================================================]

arm_config_option(
    NAME TFA_ERRATA_A55_1530923
    HELP "Enable Cortex-A55 workaround for erratum 1530923?")

target_compile_definitions(tfa-lib-cpu-common
    INTERFACE "ERRATA_A55_1530923=$<BOOL:${TFA_ERRATA_A55_1530923}>")

if(TFA_ERRATA_A55_1530923)
    arm_config_option_override(
        NAME TFA_ERRATA_SPECULATIVE_AT
        FORCE TRUE)

    target_compile_definitions(tfa-lib-cpu-common
        INTERFACE "ERRATA_SPECULATIVE_AT=$<BOOL:${TFA_ERRATA_SPECULATIVE_AT}>")
endif()

#[=======================================================================[.rst:
.. cmake:variable:: TFA_ERRATA_A57_806969

   This applies errata 806969 workaround to the Cortex-A57 CPU. This needs to be
   enabled only for revision r0p0 of the CPU.
#]=======================================================================]

arm_config_option(
    NAME TFA_ERRATA_A57_806969
    HELP "Enable Cortex-A57 workaround for erratum 806969?")

target_compile_definitions(tfa-lib-cpu-common
    INTERFACE "ERRATA_A57_806969=$<BOOL:${TFA_ERRATA_A57_806969}>")

#[=======================================================================[.rst:
.. cmake:variable:: TFA_ERRATA_A57_813419

   This applies errata 813419 workaround to the Cortex-A57 CPU. This needs to be
   enabled only for revision r0p0 of the CPU.
#]=======================================================================]

arm_config_option(
    NAME TFA_ERRATA_A57_813419
    HELP "Enable Cortex-A57 workaround for erratum 813419?")

target_compile_definitions(tfa-lib-cpu-common
    INTERFACE "ERRATA_A57_813419=$<BOOL:${TFA_ERRATA_A57_813419}>")

#[=======================================================================[.rst:
.. cmake:variable:: TFA_ERRATA_A57_813420

   This applies errata 813420 workaround to the Cortex-A57 CPU. This needs to be
   enabled only for revision r0p0 of the CPU.
#]=======================================================================]

arm_config_option(
    NAME TFA_ERRATA_A57_813420
    HELP "Enable Cortex-A57 workaround for erratum 813420?")

target_compile_definitions(tfa-lib-cpu-common
    INTERFACE "ERRATA_A57_813420=$<BOOL:${TFA_ERRATA_A57_813420}>")

#[=======================================================================[.rst:
.. cmake:variable:: TFA_ERRATA_A57_814670

   This applies errata 814670 workaround to the Cortex-A57 CPU. This needs to be
   enabled only for revision r0p0 of the CPU.
#]=======================================================================]

arm_config_option(
    NAME TFA_ERRATA_A57_814670
    HELP "Enable Cortex-A57 workaround for erratum 814670?")

target_compile_definitions(tfa-lib-cpu-common
    INTERFACE "ERRATA_A57_814670=$<BOOL:${TFA_ERRATA_A57_814670}>")

#[=======================================================================[.rst:
.. cmake:variable:: TFA_ERRATA_A57_817169

   This applies errata 817169 workaround to the Cortex-A57 CPU. This needs to be
   enabled only for revision <= r0p1 of the CPU.
#]=======================================================================]

arm_config_option(
    NAME TFA_ERRATA_A57_817169
    HELP "Enable Cortex-A57 workaround for erratum 817169?")

target_compile_definitions(tfa-lib-cpu-common
    INTERFACE "ERRATA_A57_817169=$<BOOL:${TFA_ERRATA_A57_817169}>")

#[=======================================================================[.rst:
.. cmake:variable:: TFA_ERRATA_A57_826974

   This applies errata 826974 workaround to the Cortex-A57 CPU. This needs to be
   enabled only for revision <= r1p1 of the CPU.
#]=======================================================================]

arm_config_option(
    NAME TFA_ERRATA_A57_826974
    HELP "Enable Cortex-A57 workaround for erratum 826974?")

target_compile_definitions(tfa-lib-cpu-common
    INTERFACE "ERRATA_A57_826974=$<BOOL:${TFA_ERRATA_A57_826974}>")

#[=======================================================================[.rst:
.. cmake:variable:: TFA_ERRATA_A57_826977

   This applies errata 826977 workaround to the Cortex-A57 CPU. This needs to be
   enabled only for revision <= r1p1 of the CPU.
#]=======================================================================]

arm_config_option(
    NAME TFA_ERRATA_A57_826977
    HELP "Enable Cortex-A57 workaround for erratum 826977?")

target_compile_definitions(tfa-lib-cpu-common
    INTERFACE "ERRATA_A57_826977=$<BOOL:${TFA_ERRATA_A57_826977}>")

#[=======================================================================[.rst:
.. cmake:variable:: TFA_ERRATA_A57_828024

   This applies errata 828024 workaround to the Cortex-A57 CPU. This needs to be
   enabled only for revision <= r1p1 of the CPU.
#]=======================================================================]

arm_config_option(
    NAME TFA_ERRATA_A57_828024
    HELP "Enable Cortex-A57 workaround for erratum 828024?")

target_compile_definitions(tfa-lib-cpu-common
    INTERFACE "ERRATA_A57_828024=$<BOOL:${TFA_ERRATA_A57_828024}>")

#[=======================================================================[.rst:
.. cmake:variable:: TFA_ERRATA_A57_829520

   This applies errata 829520 workaround to the Cortex-A57 CPU. This needs to be
   enabled only for revision <= r1p2 of the CPU.
#]=======================================================================]

arm_config_option(
    NAME TFA_ERRATA_A57_829520
    HELP "Enable Cortex-A57 workaround for erratum 829520?")

target_compile_definitions(tfa-lib-cpu-common
    INTERFACE "ERRATA_A57_829520=$<BOOL:${TFA_ERRATA_A57_829520}>")

#[=======================================================================[.rst:
.. cmake:variable:: TFA_ERRATA_A57_833471

   This applies errata 833471 workaround to the Cortex-A57 CPU. This needs to be
   enabled only for revision <= r1p2 of the CPU.
#]=======================================================================]

arm_config_option(
    NAME TFA_ERRATA_A57_833471
    HELP "Enable Cortex-A57 workaround for erratum 833471?")

target_compile_definitions(tfa-lib-cpu-common
    INTERFACE "ERRATA_A57_833471=$<BOOL:${TFA_ERRATA_A57_833471}>")

#[=======================================================================[.rst:
.. cmake:variable:: TFA_ERRATA_A57_859972

   This applies errata 859972 workaround to the Cortex-A57 CPU. This needs to be
   enabled only for revision <= r1p3 of the CPU.
#]=======================================================================]

arm_config_option(
    NAME TFA_ERRATA_A57_859972
    HELP "Enable Cortex-A57 workaround for erratum 859972?")

target_compile_definitions(tfa-lib-cpu-common
    INTERFACE "ERRATA_A57_859972=$<BOOL:${TFA_ERRATA_A57_859972}>")

#[=======================================================================[.rst:
.. cmake:variable:: TFA_ERRATA_A57_1319537

   This applies errata 1319537 workaround to all revisions of the Cortex-A57
   CPU.
#]=======================================================================]

arm_config_option(
    NAME TFA_ERRATA_A57_1319537
    HELP "Enable Cortex-A57 workaround for erratum 1319537?")

target_compile_definitions(tfa-lib-cpu-common
    INTERFACE "ERRATA_A57_1319537=$<BOOL:${TFA_ERRATA_A57_1319537}>")

if(TFA_ERRATA_A57_1319537)
    arm_config_option_override(
        NAME TFA_ERRATA_SPECULATIVE_AT
        FORCE TRUE)

    target_compile_definitions(tfa-lib-cpu-common
        INTERFACE "ERRATA_SPECULATIVE_AT=$<BOOL:${TFA_ERRATA_SPECULATIVE_AT}>")
endif()

#[=======================================================================[.rst:
.. cmake:variable:: TFA_ERRATA_A72_859971

   This applies errata 859971 workaround to the Cortex-A72 CPU. This needs to be
   enabled only for revision <= r0p3 of the CPU.
#]=======================================================================]

arm_config_option(
    NAME TFA_ERRATA_A72_859971
    HELP "Enable Cortex-A72 workaround for erratum 859971?")

target_compile_definitions(tfa-lib-cpu-common
    INTERFACE "ERRATA_A72_859971=$<BOOL:${TFA_ERRATA_A72_859971}>")

#[=======================================================================[.rst:
.. cmake:variable:: TFA_ERRATA_A72_1319367

   This applies errata 1319367 workaround to all revisions of the Cortex-A72 CPU.
#]=======================================================================]

arm_config_option(
    NAME TFA_ERRATA_A72_1319367
    HELP "Enable Cortex-A72 workaround for erratum 1319367?")

target_compile_definitions(tfa-lib-cpu-common
    INTERFACE "ERRATA_A72_1319367=$<BOOL:${TFA_ERRATA_A72_1319367}>")

if(TFA_ERRATA_A72_1319367)
    arm_config_option_override(
        NAME TFA_ERRATA_SPECULATIVE_AT
        FORCE TRUE)

    target_compile_definitions(tfa-lib-cpu-common
        INTERFACE "ERRATA_SPECULATIVE_AT=$<BOOL:${TFA_ERRATA_SPECULATIVE_AT}>")
endif()

#[=======================================================================[.rst:
.. cmake:variable:: TFA_ERRATA_A73_852427

   This applies errata 852427 workaround to the Cortex-A73 CPU. This needs to be
   enabled only for revision r0p0 of the CPU.
#]=======================================================================]

arm_config_option(
    NAME TFA_ERRATA_A73_852427
    HELP "Enable Cortex-A73 workaround for erratum 852427?")

target_compile_definitions(tfa-lib-cpu-common
    INTERFACE "ERRATA_A73_852427=$<BOOL:${TFA_ERRATA_A73_852427}>")

#[=======================================================================[.rst:
.. cmake:variable:: TFA_ERRATA_A73_855423

   This applies errata 855423 workaround to the Cortex-A73 CPU. This needs to be
   enabled only for revision <= r0p1 of the CPU.
#]=======================================================================]

arm_config_option(
    NAME TFA_ERRATA_A73_855423
    HELP "Enable Cortex-A73 workaround for erratum 855423?")

target_compile_definitions(tfa-lib-cpu-common
    INTERFACE "ERRATA_A73_855423=$<BOOL:${TFA_ERRATA_A73_855423}>")

#[=======================================================================[.rst:
.. cmake:variable:: TFA_ERRATA_A75_764081

   This applies errata 764081 workaround to the Cortex-A75 CPU. This needs to be
   enabled only for revision r0p0 of the CPU.
#]=======================================================================]

arm_config_option(
    NAME TFA_ERRATA_A75_764081
    HELP "Enable Cortex-A75 workaround for erratum 764081?")

target_compile_definitions(tfa-lib-cpu-common
    INTERFACE "ERRATA_A75_764081=$<BOOL:${TFA_ERRATA_A75_764081}>")

#[=======================================================================[.rst:
.. cmake:variable:: TFA_ERRATA_A75_790748

   This applies errata 790748 workaround to the Cortex-A75  CPU. This needs to
   be enabled only for revision r0p0 of the CPU.
#]=======================================================================]

arm_config_option(
    NAME TFA_ERRATA_A75_790748
    HELP "Enable Cortex-A75 workaround for erratum 790748?")

target_compile_definitions(tfa-lib-cpu-common
    INTERFACE "ERRATA_A75_790748=$<BOOL:${TFA_ERRATA_A75_790748}>")

#[=======================================================================[.rst:
.. cmake:variable:: TFA_ERRATA_A76_1073348

   This applies errata 1073348 workaround to the Cortex-A76 CPU. This needs to
   be enabled only for revision <= r1p0 of the CPU.
#]=======================================================================]

arm_config_option(
    NAME TFA_ERRATA_A76_1073348
    HELP "Enable Cortex-A76 workaround for erratum 1073348?")

target_compile_definitions(tfa-lib-cpu-common
    INTERFACE "ERRATA_A76_1073348=$<BOOL:${TFA_ERRATA_A76_1073348}>")

#[=======================================================================[.rst:
.. cmake:variable:: TFA_ERRATA_A76_1130799

   This applies errata 1130799 workaround to the Cortex-A76 CPU. This needs to
   be enabled only for revision <= r2p0 of the CPU.
#]=======================================================================]

arm_config_option(
    NAME TFA_ERRATA_A76_1130799
    HELP "Enable Cortex-A76 workaround for erratum 1130799?")

target_compile_definitions(tfa-lib-cpu-common
    INTERFACE "ERRATA_A76_1130799=$<BOOL:${TFA_ERRATA_A76_1130799}>")

#[=======================================================================[.rst:
.. cmake:variable:: TFA_ERRATA_A76_1220197

   This applies errata 1220197 workaround to the Cortex-A76 CPU. This needs to
   be enabled only for revision <= r2p0 of the CPU.
#]=======================================================================]

arm_config_option(
    NAME TFA_ERRATA_A76_1220197
    HELP "Enable Cortex-A76 workaround for erratum 1220197?")

target_compile_definitions(tfa-lib-cpu-common
    INTERFACE "ERRATA_A76_1220197=$<BOOL:${TFA_ERRATA_A76_1220197}>")

#[=======================================================================[.rst:
.. cmake:variable:: TFA_ERRATA_A76_1257314

   This applies errata 1257314 workaround to the Cortex-A76 CPU. This needs to
   be enabled only for revision <= r3p0 of the CPU.
#]=======================================================================]

arm_config_option(
    NAME TFA_ERRATA_A76_1257314
    HELP "Enable Cortex-A76 workaround for erratum 1257314?")

target_compile_definitions(tfa-lib-cpu-common
    INTERFACE "ERRATA_A76_1257314=$<BOOL:${TFA_ERRATA_A76_1257314}>")

#[=======================================================================[.rst:
.. cmake:variable:: TFA_ERRATA_A76_1262606

   This applies errata 1262606 workaround to the Cortex-A76 CPU. This needs to
   be enabled only for revision <= r3p0 of the CPU.
#]=======================================================================]

arm_config_option(
    NAME TFA_ERRATA_A76_1262606
    HELP "Enable Cortex-A76 workaround for erratum 1262606?")

target_compile_definitions(tfa-lib-cpu-common
    INTERFACE "ERRATA_A76_1262606=$<BOOL:${TFA_ERRATA_A76_1262606}>")

#[=======================================================================[.rst:
.. cmake:variable:: TFA_ERRATA_A76_1262888

   This applies errata 1262888 workaround to the Cortex-A76 CPU. This needs to
   be enabled only for revision <= r3p0 of the CPU.
#]=======================================================================]

arm_config_option(
    NAME TFA_ERRATA_A76_1262888
    HELP "Enable Cortex-A76 workaround for erratum 1262888?")

target_compile_definitions(tfa-lib-cpu-common
    INTERFACE "ERRATA_A76_1262888=$<BOOL:${TFA_ERRATA_A76_1262888}>")

#[=======================================================================[.rst:
.. cmake:variable:: TFA_ERRATA_A76_1275112

   This applies errata 1275112 workaround to the Cortex-A76 CPU. This needs to
   be enabled only for revision <= r3p0 of the CPU.
#]=======================================================================]

arm_config_option(
    NAME TFA_ERRATA_A76_1275112
    HELP "Enable Cortex-A76 workaround for erratum 1275112?")

target_compile_definitions(tfa-lib-cpu-common
    INTERFACE "ERRATA_A76_1275112=$<BOOL:${TFA_ERRATA_A76_1275112}>")

#[=======================================================================[.rst:
.. cmake:variable:: ERRATA_A76_1286807

   This applies errata 1286807 workaround to the Cortex-A76 CPU. This needs to
   be enabled only for revision <= r3p0 of the CPU.
#]=======================================================================]

arm_config_option(
    NAME TFA_ERRATA_A76_1286807
    HELP "Enable Cortex-A76 workaround for erratum 1286807?")

target_compile_definitions(tfa-lib-cpu-common
    INTERFACE "ERRATA_A76_1286807=$<BOOL:${TFA_ERRATA_A76_1286807}>")

#[=======================================================================[.rst:
.. cmake:variable:: TFA_ERRATA_A76_1791580

   This applies errata 1791580 workaround to the Cortex-A76 CPU. This needs to
   be enabled only for revision <= r4p0 of the CPU.
#]=======================================================================]

arm_config_option(
    NAME TFA_ERRATA_A76_1791580
    HELP "Enable Cortex-A76 workaround for erratum 1791580?")

target_compile_definitions(tfa-lib-cpu-common
    INTERFACE "ERRATA_A76_1791580=$<BOOL:${TFA_ERRATA_A76_1791580}>")

#[=======================================================================[.rst:
.. cmake:variable:: TFA_ERRATA_A76_1165522

   This applies errata 1165522 workaround to all revisions of the Cortex-A76
   CPU. This errata is fixed in r3p0 but due to limitation of errata framework
   this erratum is applied to all revisions of the Cortex-A76 CPU.
#]=======================================================================]

arm_config_option(
    NAME TFA_ERRATA_A76_1165522
    HELP "Enable Cortex-A76 workaround for erratum 1165522?")

target_compile_definitions(tfa-lib-cpu-common
    INTERFACE "ERRATA_A76_1165522=$<BOOL:${TFA_ERRATA_A76_1165522}>")

if(TFA_ERRATA_A72_1319367)
    arm_config_option_override(
        NAME TFA_ERRATA_SPECULATIVE_AT
        FORCE TRUE)

    target_compile_definitions(tfa-lib-cpu-common
        INTERFACE "ERRATA_SPECULATIVE_AT=$<BOOL:${TFA_ERRATA_SPECULATIVE_AT}>")
endif()

#[=======================================================================[.rst:
.. cmake:variable:: TFA_ERRATA_A76_1868343

   This applies errata 1868343 workaround to the Cortex-A76 CPU. This needs to
   be enabled only for revision <= r4p0 of the CPU.
#]=======================================================================]

arm_config_option(
    NAME TFA_ERRATA_A76_1868343
    HELP "Enable Cortex-A76 workaround for erratum 1868343?")

target_compile_definitions(tfa-lib-cpu-common
    INTERFACE "ERRATA_A76_1868343=$<BOOL:${TFA_ERRATA_A76_1868343}>")

#[=======================================================================[.rst:
.. cmake:variable:: TFA_ERRATA_A76_1946160

   This applies errata 1946160 workaround to the Cortex-A76 CPU. This needs to
   be enabled only for revisions r3p0 to r4p1 of the CPU.
#]=======================================================================]

arm_config_option(
    NAME TFA_ERRATA_A76_1946160
    HELP "Enable Cortex-A76 workaround for erratum 1946160?")

target_compile_definitions(tfa-lib-cpu-common
    INTERFACE "ERRATA_A76_1946160=$<BOOL:${TFA_ERRATA_A76_1946160}>")

#[=======================================================================[.rst:
.. cmake:variable:: TFA_ERRATA_A77_1508412

   This applies errata 1508412 workaround to the Cortex-A77 CPU. This needs to
   be enabled only for revision <= r1p0 of the CPU.
#]=======================================================================]

arm_config_option(
    NAME TFA_ERRATA_A77_1508412
    HELP "Enable Cortex-A77 workaround for erratum 1508412?")

target_compile_definitions(tfa-lib-cpu-common
    INTERFACE "ERRATA_A77_1508412=$<BOOL:${TFA_ERRATA_A77_1508412}>")

#[=======================================================================[.rst:
.. cmake:variable:: TFA_ERRATA_A77_1925769

   This applies errata 1925769 workaround to the Cortex-A77 CPU. This needs to
   be enabled only for revision <= r1p1 of the CPU.
#]=======================================================================]

arm_config_option(
    NAME TFA_ERRATA_A77_1925769
    HELP "Enable Cortex-A77 workaround for erratum 1925769?")

target_compile_definitions(tfa-lib-cpu-common
    INTERFACE "ERRATA_A77_1925769=$<BOOL:${TFA_ERRATA_A77_1925769}>")

#[=======================================================================[.rst:
.. cmake:variable:: TFA_ERRATA_A78_1688305

   This applies errata 1688305 workaround to the Cortex-A78 CPU. This needs to
   be enabled only for revision r0p0 to r1p0 of the CPU.
#]=======================================================================]

arm_config_option(
    NAME TFA_ERRATA_A78_1688305
    HELP "Enable Cortex-A78 workaround for erratum 1688305?")

target_compile_definitions(tfa-lib-cpu-common
    INTERFACE "ERRATA_A78_1688305=$<BOOL:${TFA_ERRATA_A78_1688305}>")

#[=======================================================================[.rst:
.. cmake:variable:: TFA_ERRATA_A78_1941498

   This applies errata 1941498 workaround to the Cortex-A78 CPU. This needs to
   be enabled for revisions r0p0, r1p0, and r1p1 of the CPU.
#]=======================================================================]

arm_config_option(
    NAME TFA_ERRATA_A78_1941498
    HELP "Enable Cortex-A78 workaround for erratum 1941498?")

target_compile_definitions(tfa-lib-cpu-common
    INTERFACE "ERRATA_A78_1941498=$<BOOL:${TFA_ERRATA_A78_1941498}>")

#[=======================================================================[.rst:
.. cmake:variable:: TFA_ERRATA_A78_1951500

   This applies errata 1951500 workaround to the Cortex-A78 CPU. This needs to
   be enabled for revisions r1p0 and r1p1. There is no workaround available for
   r0p0.
#]=======================================================================]

arm_config_option(
    NAME TFA_ERRATA_A78_1951500
    HELP "Enable Cortex-A78 workaround for erratum 1951500?")

target_compile_definitions(tfa-lib-cpu-common
    INTERFACE "ERRATA_A78_1951500=$<BOOL:${TFA_ERRATA_A78_1951500}>")

#[=======================================================================[.rst:
.. cmake:variable:: TFA_ERRATA_N1_1043202

   This applies the T32 CLREX workaround during reset to the Neoversse-N1 CPU.
   This needs to be enabled only for revision r0p0 and r1p0 of the CPU.
#]=======================================================================]

arm_config_option(
    NAME TFA_ERRATA_N1_1043202
    HELP "Enable Neoverse-N1 workaround for erratum 1043202?")

target_compile_definitions(tfa-lib-cpu-common
    INTERFACE "ERRATA_N1_1043202=$<BOOL:${TFA_ERRATA_N1_1043202}>")

#[=======================================================================[.rst:
.. cmake:variable:: TFA_ERRATA_N1_1073348

   This applies errata 1073348 workaround to the Neoverse-N1 CPU. This needs to
   be enabled only for revision r0p0 and r1p0 of the CPU.
#]=======================================================================]

arm_config_option(
    NAME TFA_ERRATA_N1_1073348
    HELP "Enable Neoverse-N1 workaround for erratum 1073348?")

target_compile_definitions(tfa-lib-cpu-common
    INTERFACE "ERRATA_N1_1073348=$<BOOL:${TFA_ERRATA_N1_1073348}>")

#[=======================================================================[.rst:
.. cmake:variable:: TFA_ERRATA_N1_1130799

   This applies errata 1130799 workaround to the Neoverse-N1 CPU. This needs to
   be enabled only for revision <= r2p0 of the CPU.
#]=======================================================================]

arm_config_option(
    NAME TFA_ERRATA_N1_1130799
    HELP "Enable Neoverse-N1 workaround for erratum 1130799?")

target_compile_definitions(tfa-lib-cpu-common
    INTERFACE "ERRATA_N1_1130799=$<BOOL:${TFA_ERRATA_N1_1130799}>")

#[=======================================================================[.rst:
.. cmake:variable:: TFA_ERRATA_N1_1165347

   This applies errata 1165347 workaround to the Neoverse-N1 CPU. This needs to
   be enabled only for revision <= r2p0 of the CPU.
#]=======================================================================]

arm_config_option(
    NAME TFA_ERRATA_N1_1165347
    HELP "Enable Neoverse-N1 workaround for erratum 1165347?")

target_compile_definitions(tfa-lib-cpu-common
    INTERFACE "ERRATA_N1_1165347=$<BOOL:${TFA_ERRATA_N1_1165347}>")

#[=======================================================================[.rst:
.. cmake:variable:: TFA_ERRATA_N1_1207823

   This applies errata 1207823 workaround to the Neoverse-N1 CPU. This needs to
   be enabled only for revision <= r2p0 of the CPU.
#]=======================================================================]

arm_config_option(
    NAME TFA_ERRATA_N1_1207823
    HELP "Enable Neoverse-N1 workaround for erratum 1207823?")

target_compile_definitions(tfa-lib-cpu-common
    INTERFACE "ERRATA_N1_1207823=$<BOOL:${TFA_ERRATA_N1_1207823}>")

#[=======================================================================[.rst:
.. cmake:variable:: TFA_ERRATA_N1_1220197

   This applies errata 1220197 workaround to the Neoverse-N1 CPU. This needs to
   be enabled only for revision <= r2p0 of the CPU.
#]=======================================================================]

arm_config_option(
    NAME TFA_ERRATA_N1_1220197
    HELP "Enable Neoverse-N1 workaround for erratum 1220197?")

target_compile_definitions(tfa-lib-cpu-common
    INTERFACE "ERRATA_N1_1220197=$<BOOL:${TFA_ERRATA_N1_1220197}>")

#[=======================================================================[.rst:
.. cmake:variable:: TFA_ERRATA_N1_1257314

   This applies errata 1257314 workaround to the Neoverse-N1 CPU. This needs to
   be enabled only for revision <= r3p0 of the CPU.
#]=======================================================================]

arm_config_option(
    NAME TFA_ERRATA_N1_1257314
    HELP "Enable Neoverse-N1 workaround for erratum 1257314?")

target_compile_definitions(tfa-lib-cpu-common
    INTERFACE "ERRATA_N1_1257314=$<BOOL:${TFA_ERRATA_N1_1257314}>")

#[=======================================================================[.rst:
.. cmake:variable:: TFA_ERRATA_N1_1262606

   This applies errata 1262606 workaround to the Neoverse-N1 CPU. This needs to
   be enabled only for revision <= r3p0 of the CPU.
#]=======================================================================]

arm_config_option(
    NAME TFA_ERRATA_N1_1262606
    HELP "Enable Neoverse-N1 workaround for erratum 1262606?")

target_compile_definitions(tfa-lib-cpu-common
    INTERFACE "ERRATA_N1_1262606=$<BOOL:${TFA_ERRATA_N1_1262606}>")

#[=======================================================================[.rst:
.. cmake:variable:: TFA_ERRATA_N1_1262888

   This applies errata 1262888 workaround to the Neoverse-N1 CPU. This needs to
   be enabled only for revision <= r3p0 of the CPU.
#]=======================================================================]

arm_config_option(
    NAME TFA_ERRATA_N1_1262888
    HELP "Enable Neoverse-N1 workaround for erratum 1262888?")

target_compile_definitions(tfa-lib-cpu-common
    INTERFACE "ERRATA_N1_1262888=$<BOOL:${TFA_ERRATA_N1_1262888}>")

#[=======================================================================[.rst:
.. cmake:variable:: TFA_ERRATA_N1_1275112

   This applies errata 1275112 workaround to the Neoverse-N1 CPU. This needs to
   be enabled only for revision <= r3p0 of the CPU.
#]=======================================================================]

arm_config_option(
    NAME TFA_ERRATA_N1_1275112
    HELP "Enable Neoverse-N1 workaround for erratum 1275112?")

target_compile_definitions(tfa-lib-cpu-common
    INTERFACE "ERRATA_N1_1275112=$<BOOL:${TFA_ERRATA_N1_1275112}>")

#[=======================================================================[.rst:
.. cmake:variable:: TFA_ERRATA_N1_1315703

   This applies errata 1315703 workaround to the Neoverse-N1 CPU. This needs to
   be enabled only for revisions <= r3p0 of the CPU.
#]=======================================================================]

arm_config_option(
    NAME TFA_ERRATA_N1_1315703
    HELP "Enable Neoverse-N1 workaround for erratum 1315703?")

target_compile_definitions(tfa-lib-cpu-common
    INTERFACE "ERRATA_N1_1315703=$<BOOL:${TFA_ERRATA_N1_1315703}>")

#[=======================================================================[.rst:
.. cmake:variable:: TFA_ERRATA_N1_1542419

   This applies errata 1542419 workaround to the Neoverse-N1 CPU. This needs to
   be enabled only for revisions r3p0 to r4p0 of the CPU.
#]=======================================================================]

arm_config_option(
    NAME TFA_ERRATA_N1_1542419
    HELP "Enable Neoverse-N1 workaround for erratum 1542419?")

target_compile_definitions(tfa-lib-cpu-common
    INTERFACE "ERRATA_N1_1542419=$<BOOL:${TFA_ERRATA_N1_1542419}>")

#[=======================================================================[.rst:
.. cmake:variable:: TFA_ERRATA_N1_1868343

   This applies errata 1868343 workaround to the Neoverse-N1 CPU. This needs to
   be enabled only for revision <= r4p0 of the CPU.
#]=======================================================================]

arm_config_option(
    NAME TFA_ERRATA_N1_1868343
    HELP "Enable Neoverse-N1 workaround for erratum 1868343?")

target_compile_definitions(tfa-lib-cpu-common
    INTERFACE "ERRATA_N1_1868343=$<BOOL:${TFA_ERRATA_N1_1868343}>")

#[=======================================================================[.rst:
.. cmake:variable:: TFA_ERRATA_N1_1946160

   This applies errata 1946160 workaround to the Neoverse-N1 CPU. This needs to
   be enabled for revisions r3p0, r3p1, r4p0, and r4p1. There is no workaround
   available for revisions r0p0, r1p0, and r2p0.
#]=======================================================================]

arm_config_option(
    NAME TFA_ERRATA_N1_1946160
    HELP "Enable Neoverse-N1 workaround for erratum 1946160?")

target_compile_definitions(tfa-lib-cpu-common
    INTERFACE "ERRATA_N1_1946160=$<BOOL:${TFA_ERRATA_N1_1946160}>")

#[=======================================================================[.rst:
.. cmake:variable:: TFA_ERRATA_DSU_798953

   This applies errata 798953 workaround for the affected DSU configurations.
   This errata applies only for DSUs older than r0p1. Please note that
   this workaround results in increased DSU power consumption on idle.
#]=======================================================================]

arm_config_option(
    NAME TFA_ERRATA_DSU_798953
    HELP "Enable DSU workaround for erratum 798953?")

target_compile_definitions(tfa-lib-cpu-common
    INTERFACE "ERRATA_DSU_798953=$<BOOL:${TFA_ERRATA_DSU_798953}>")

#[=======================================================================[.rst:
.. cmake:variable:: TFA_ERRATA_DSU_936184

   This applies errata 936184 workaround for the affected DSU configurations.
   This errata applies only for those DSUs that contain the ACP interface
   **and** the DSU revision is older than r2p0 (on r2p0 it is fixed). However,
   please note that this workaround results in increased DSU power consumption
   on idle.
#]=======================================================================]

arm_config_option(
    NAME TFA_ERRATA_DSU_936184
    HELP "Enable DSU workaround for erratum 936184?")

target_compile_definitions(tfa-lib-cpu-common
    INTERFACE "ERRATA_DSU_936184=$<BOOL:${TFA_ERRATA_DSU_936184}>")

#[=======================================================================[.rst:
.. cmake:variable:: TFA_ERRATA_SPECULATIVE_AT

   This option determines whether to enable the ``AT`` instruction speculation
   errata workaround or not.

   This workaround disables stage 1 page table walks for lower ELs (EL1 and EL0)
   in EL3 so that speculative fetches triggered by an ``AT`` instruction produce
   either the correct result or failure without a TLB allocation. This
   workaround is common to several CPU errata.

   This option is intended to be enabled only by other workarounds that require
   it.

   Disabled by default.
#]=======================================================================]

arm_config_option(
    NAME TFA_ERRATA_SPECULATIVE_AT HIDDEN
    HELP "Enable speculative AT errata workaround?")

target_compile_definitions(tfa-lib-cpu-common
    INTERFACE "ERRATA_SPECULATIVE_AT=$<BOOL:${TFA_ERRATA_SPECULATIVE_AT}>")

#[=======================================================================[.rst:
.. cmake:variable:: TFA_SKIP_A57_L1_FLUSH_PWR_DWN

   This option enables an optimization in the Cortex-A57 cluster power down
   sequence by not flushing the Level 1 data cache. The L1 data cache and the L2
   unified cache are inclusive. A flush of the L2 by set/way flushes any dirty
   lines from the L1 as well. This is a known safe deviation from the Cortex-A57
   TRM defined power down sequence. Each Cortex-A57 based platform must make its
   own decision on whether to use the optimization.
#]=======================================================================]

arm_config_option(
    NAME TFA_SKIP_A57_L1_FLUSH_PWR_DWN
    HELP "Skip flushing Cortex-A57 L1 cache on power down?")

target_compile_definitions(tfa-lib-cpu-common
    INTERFACE "SKIP_A57_L1_FLUSH_PWR_DWN=$<BOOL:${TFA_SKIP_A57_L1_FLUSH_PWR_DWN}>")

#[=======================================================================[.rst:
.. cmake:variable:: TFA_A53_DISABLE_NON_TEMPORAL_HINT

   This option disables the cache non-temporal hint. The LDNP/STNP instructions
   as implemented on the Cortex-A53 do not behave in a way most programmers
   expect, and will most probably result in a significant speed degradation to
   any code that employs them. The Armv8-A architecture (see Arm DDI 0487A.h,
   section D3.4.3) allows cores to ignore the non-temporal hint and treat
   LDNP/STNP as LDP/STP instead. Enabling this option enforces this behaviour.

   This needs to be enabled only for revisions <= r0p3 of the CPU and is enabled
   by default.
#]=======================================================================]

arm_config_option(
    NAME TFA_A53_DISABLE_NON_TEMPORAL_HINT
    HELP "Disable cache non-temporal hint for the Cortex-A53?"
    DEFAULT TRUE)

target_compile_definitions(tfa-lib-cpu-common
    INTERFACE "A53_DISABLE_NON_TEMPORAL_HINT=$<BOOL:${TFA_A53_DISABLE_NON_TEMPORAL_HINT}>")

#[=======================================================================[.rst:
.. cmake:variable:: TFA_A57_DISABLE_NON_TEMPORAL_HINT

   This option has the same behaviour as ``A53_DISABLE_NON_TEMPORAL_HINT`` but
   for the Cortex-A57. This needs to be enabled only for revisions <= r1p2 of
   the CPU and is enabled by default, as recommended in section "4.7
   Non-Temporal Loads/Stores" of the `Cortex-A57 Software Optimization Guide`_.

.. _Cortex-A57 Software Optimization Guide: http://infocenter.arm.com/help/topic/com.arm.doc.uan0015b/Cortex_A57_Software_Optimization_Guide_external.pdf
#]=======================================================================]

arm_config_option(
    NAME TFA_A57_DISABLE_NON_TEMPORAL_HINT
    HELP "Disable cache non-temporal hint for the Cortex-A57?"
    DEFAULT TRUE)

target_compile_definitions(tfa-lib-cpu-common
    INTERFACE "A57_DISABLE_NON_TEMPORAL_HINT=$<BOOL:${TFA_A57_DISABLE_NON_TEMPORAL_HINT}>")

#[=======================================================================[.rst:
.. cmake:variable:: A57_ENABLE_NONCACHEABLE_LOAD_FWD

   This option enables higher-performance non-cacheable load forwarding for the
   Cortex-A57.
#]=======================================================================]

arm_config_option(
    NAME TFA_A57_ENABLE_NONCACHEABLE_LOAD_FWD
    HELP "Enable non-cacheable load forwarding for the Cortex-A57?")

target_compile_definitions(tfa-lib-cpu-cortex-a57
    INTERFACE "A57_ENABLE_NONCACHEABLE_LOAD_FWD=$<BOOL:${TFA_A57_ENABLE_NONCACHEABLE_LOAD_FWD}>")

#[=======================================================================[.rst:
.. cmake:variable:: TFA_NEOVERSE_Nx_EXTERNAL_LLC

   This option indicates that an external last level cache (LLC) is present in
   the system, and that the DataSource field on the master CHI interface
   indicates when data is returned from the LLC. This is used to control how the
   LL_CACHE* PMU events count. Disabled by default.
#]=======================================================================]

arm_config_option(
    NAME TFA_NEOVERSE_Nx_EXTERNAL_LLC
    HELP "Is the last level cache external?")

target_compile_definitions(tfa-lib-cpu-common
    INTERFACE "NEOVERSE_Nx_EXTERNAL_LLC=$<BOOL:${TFA_NEOVERSE_Nx_EXTERNAL_LLC}>")
