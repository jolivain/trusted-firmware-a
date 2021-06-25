#
# Set up the configuration types for both single and multi-configuration
# generators.
#

set(config_types "Debug" "RelWithDebInfo" "MinSizeRel" "Release")
set(default_config "MinSizeRel")

get_property(multi_config GLOBAL PROPERTY "GENERATOR_IS_MULTI_CONFIG")

if(multi_config)
    arm_config_option(
        NAME CMAKE_CONFIGURATION_TYPES
        HELP "Multi-generator configuration types."
        DEFAULT "${config_types}"
        TYPE INTERNAL)

    arm_config_option(
        NAME CMAKE_DEFAULT_BUILD_TYPE
        HELP "Default multi-generator configuration type."
        DEFAULT "${default_config}"
        TYPE INTERNAL)
else()
    arm_config_option(
        NAME CMAKE_BUILD_TYPE
        HELP "Build type."
        STRINGS ${config_types}
        DEFAULT ${default_config}
        FORCE NOT CMAKE_BUILD_TYPE)
endif()
