add_library(tfa-bl1-aarch64 INTERFACE)

target_sources(tfa-bl1-aarch64
    INTERFACE "bl1_arch_setup.c"
              "bl1_context_mgmt.c"
              "bl1_entrypoint.S"
              "bl1_exceptions.S")
