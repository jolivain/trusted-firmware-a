add_library(tfa-lib-aarch32 INTERFACE)

target_include_directories(tfa-lib-aarch32
    INTERFACE "${TFA_SOURCE_DIR}/include/arch/${TFA_ARCH_STATE_LOWER}")

target_sources(tfa-lib-aarch32
    INTERFACE "cache_helpers.S"
              "misc_helpers.S")

if(CMAKE_C_COMPILER_ID STREQUAL "ARMClang")
    target_sources(tfa-lib-aarch32
        INTERFACE "armclang_printf.S")
endif()

target_sources(tfa-lib-aarch32
    INTERFACE "aarch32/arm32_aeabi_divmod.c"
              "aarch32/arm32_aeabi_divmod_a32.S")

add_library(tfa-lib-arch
    ALIAS tfa-lib-aarch32)
