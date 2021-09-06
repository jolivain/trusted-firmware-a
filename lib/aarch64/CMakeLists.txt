add_library(tfa-lib-aarch64 INTERFACE)

target_include_directories(tfa-lib-aarch64
    INTERFACE "${TFA_SOURCE_DIR}/include/arch/${TFA_ARCH_STATE_LOWER}")

target_sources(tfa-lib-aarch64
    INTERFACE "cache_helpers.S"
              "misc_helpers.S")

if(CMAKE_C_COMPILER_ID STREQUAL "ARMClang")
    target_sources(tfa-lib-aarch64
        INTERFACE "armclang_printf.S")
endif()

#
# Enable invalidation of cached read-write memory when BL2 runs from EL3.
#

arm_config_option(
    NAME TFA_BL2_INV_DCACHE
    HELP "Invalidate cached read-write memory used by BL2?"
    DEPENDS TFA_BL2_AT_EL3
    DEFAULT TRUE)

tfa_legacy_option(
    OLD BL2_INV_DCACHE
    NEW TFA_BL2_INV_DCACHE)

target_compile_definitions(tfa-common
    INTERFACE "BL2_INV_DCACHE=$<BOOL:${TFA_BL2_INV_DCACHE}>")

add_library(tfa-lib-arch
    ALIAS tfa-lib-aarch64)
