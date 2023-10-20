#
# Copyright (c) 2023, Arm Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

#
# Define the implicit arguments to each tool class.
#
# These arguments will always be passed when calling a tool via the usual
# functions helper functions (`cc`, `cpp`, `ld`, etc.).
#

as-arguments += as
as-arguments += target
as-arguments += mcpu
as-arguments += mfpu
as-arguments += output-file
as-arguments += deps-file
as-arguments += input-files
as-arguments += defines
as-arguments += include-dirs
as-arguments += flags

cc-arguments += cc
cc-arguments += target
cc-arguments += mcpu
cc-arguments += mfpu
cc-arguments += output-file
cc-arguments += deps-file
cc-arguments += input-files
cc-arguments += defines
cc-arguments += include-dirs
cc-arguments += flags

cpp-arguments += cpp
cpp-arguments += target
cpp-arguments += mcpu
cpp-arguments += mfpu
cpp-arguments += output-file
cpp-arguments += deps-file
cpp-arguments += input-files
cpp-arguments += defines
cpp-arguments += include-dirs
cpp-arguments += flags

ld-arguments += ld
ld-arguments += target
ld-arguments += mcpu
ld-arguments += mfpu
ld-arguments += output-file
ld-arguments += map-file
ld-arguments += flags

oc-arguments += oc
oc-arguments += flags

od-arguments += od
od-arguments += flags

#
# Define the `ar` argument.
#
# This argument defines the flags required to configure the archiver to create
# new or update existing archive files.
#

argument-ar-arm-ar = -r -c
argument-ar-llvm-ar = rc
argument-ar-gnu-ar = rc

#
# Define the `as` argument.
#
# This argument defines the flags required to configure the assembler to compile
# assembly files to object files.
#

argument-as-arm-clang = -x assembler-with-cpp -c
argument-as-llvm-clang = -x assembler-with-cpp -c
argument-as-gnu-gcc = -x assembler-with-cpp -c

#
# Define the `cc` argument.
#
# This argument defines the flags required to configure the C compiler to
# compile C files to object files.
#

argument-cc-arm-clang = -x c -c
argument-cc-llvm-clang = -x c -c
argument-cc-gnu-gcc = -x c -c

#
# Define the `cpp` argument.
#
# This argument defines the flags required to configure the C preprocessor to
# preprocess arbitrary files.
#

argument-cpp-arm-clang = -x assembler-with-cpp -E
argument-cpp-llvm-clang = -x assembler-with-cpp -E
argument-cpp-gnu-gcc = -x assembler-with-cpp -E

#
# Define the `ld` argument.
#
# This argument defines the flags required to configure the linker to link
# object files and libraries into binaries.
#

argument-ld-arm-clang =
argument-ld-llvm-clang =
argument-ld-gnu-gcc =

#
# Define the `oc` argument.
#
# This argument defines the flags required to configure the object copier to
# convert binary files into some other binary format.
#

argument-oc-arm-fromelf =
argument-oc-llvm-objcopy =
argument-oc-gnu-objcopy =

#
# Define the `od` argument.
#
# This argument defines the flags required to configure the object dumper to
# dump the disassembly and section headers of an image in a human-readable
# format.
#

argument-od-arm-fromelf = --text -c -v
argument-od-llvm-objdump = --disassemble --section-headers
argument-od-gnu-objdump = --disassemble --section-headers

#
# Define the `response-file` argument.
#
# This argument defines the flags required to configure the tool to take its
# arguments from a file given by the `response-file` argument parameter.
#

argument-response-file-arm-ar = --via $(call tool-parameter,$(1),$(2),$(3),response-file)
argument-response-file-arm-clang = @$(call tool-parameter,$(1),$(2),$(3),response-file)

argument-response-file-llvm-ar = @$(call tool-parameter,$(1),$(2),$(3),response-file)
argument-response-file-llvm-clang = @$(call tool-parameter,$(1),$(2),$(3),response-file)
argument-response-file-llvm-objcopy = @$(call tool-parameter,$(1),$(2),$(3),response-file)
argument-response-file-llvm-objdump = @$(call tool-parameter,$(1),$(2),$(3),response-file)

argument-response-file-gnu-ar = # https://gcc.gnu.org/bugzilla/show_bug.cgi?id=77576
argument-response-file-gnu-gcc = @$(call tool-parameter,$(1),$(2),$(3),response-file)
argument-response-file-gnu-objcopy = @$(call tool-parameter,$(1),$(2),$(3),response-file)
argument-response-file-gnu-objdump = @$(call tool-parameter,$(1),$(2),$(3),response-file)

#
# Define the `flags` argument.
#
# This argument forwards arbitrary flags to each tool when they are given via
# the `flags` argument parameter.
#

define define-tool-flags
        $(eval tool := $(1))

        $(tool)-flags = $$(call tool-parameters,$$(1),$$(2),$$(3),flags)
endef

$(foreach tool,$(tools), \
        $(eval $(call define-tool-flags,$(tool))))

#
# Define the `target` argument.
#
# This argument models the LLVM Clang-like `-target` flag, which accepts a
# target triplet given by the `target` argument parameter.
#

aarch32-arm-clang-target := arm-arm-none-eabi
aarch32-llvm-clang-target := arm-none-eabi

aarch64-arm-clang-target := aarch64-arm-none-eabi
aarch64-llvm-clang-target := aarch64-none-elf

argument-target-arm-clang = $(addprefix -target ,$(call tool-parameter,$(1),$(2),$(3),target))
argument-target-llvm-clang = $(addprefix -target ,$(call tool-parameter,$(1),$(2),$(3),target))

#
# Define the `output-file` argument.
#
# This argument defines the flags required to direct the tool's output to an
# output file given by the `output-file` argument parameter.
#

argument-output-file-arm-ar = $(call tool-parameter,$(1),$(2),$(3),output-file)
argument-output-file-llvm-ar = $(call tool-parameter,$(1),$(2),$(3),output-file)
argument-output-file-gnu-ar = $(call tool-parameter,$(1),$(2),$(3),output-file)

argument-output-file-arm-clang = $(addprefix -o ,$(call tool-parameter,$(1),$(2),$(3),output-file))
argument-output-file-llvm-clang = $(addprefix -o ,$(call tool-parameter,$(1),$(2),$(3),output-file))
argument-output-file-gnu-gcc = $(addprefix -o ,$(call tool-parameter,$(1),$(2),$(3),output-file))

argument-output-file-arm-fromelf = $(addprefix --output ,$(call tool-parameter,$(1),$(2),$(3),output-file))
argument-output-file-llvm-objcopy = $(call tool-parameter,$(1),$(2),$(3),output-file)
argument-output-file-gnu-objcopy = $(call tool-parameter,$(1),$(2),$(3),output-file)

#
# Define the `input-files` argument.
#
# This argument defines the flags required to specify one or more input files
# given by the `input-files` argument parameter.
#

argument-input-files-arm-clang = $(call tool-parameters,$(1),$(2),$(3),input-files)
argument-input-files-llvm-clang = $(call tool-parameters,$(1),$(2),$(3),input-files)
argument-input-files-gnu-gcc = $(call tool-parameters,$(1),$(2),$(3),input-files)

argument-input-files-arm-ar = $(call tool-parameters,$(1),$(2),$(3),input-files)
argument-input-files-llvm-ar = $(call tool-parameters,$(1),$(2),$(3),input-files)
argument-input-files-gnu-ar = $(call tool-parameters,$(1),$(2),$(3),input-files)

argument-input-files-arm-fromelf = $(call tool-parameters,$(1),$(2),$(3),input-files)
argument-input-files-llvm-objdump = $(call tool-parameters,$(1),$(2),$(3),input-files)
argument-input-files-gnu-objdump = $(call tool-parameters,$(1),$(2),$(3),input-files)

argument-input-files-llvm-objcopy = $(call tool-parameters,$(1),$(2),$(3),input-files)
argument-input-files-gnu-objcopy = $(call tool-parameters,$(1),$(2),$(3),input-files)

#
# Define the `deps-file` argument.
#
# This argument defines the flags required to specify a dependencies file given
# by the `deps-file` argument parameter. Dependencies files are files generated
# at build time, which can be included back into the Makefile to detect changes
# that might otherwise have not been detected.
#

argument-deps-file-arm-clang = $(addprefix -MD -MP $(addprefix -MT ,$(call tool-parameter,$(1),$(2),$(3),output-file)) -MF ,$(call tool-parameter,$(1),$(2),$(3),deps-file))
argument-deps-file-llvm-clang = $(addprefix -MD -MP $(addprefix -MT ,$(call tool-parameter,$(1),$(2),$(3),output-file)) -MF ,$(call tool-parameter,$(1),$(2),$(3),deps-file))
argument-deps-file-gnu-gcc = $(addprefix -MD -MP $(addprefix -MT ,$(call tool-parameter,$(1),$(2),$(3),output-file)) -MF ,$(call tool-parameter,$(1),$(2),$(3),deps-file))

#
# Define the `map-file` argument.
#
# This argument defines the flags required to specify a path to a map file - a
# human-readable dissection of the generated binary - to generate during
# linking. The path to the map file is given by the `map-file` argument
# parameter.
#

argument-map-file-arm-clang = $(addprefix -Xlinker --map -Xlinker --list=$(call tool-parameter,$(1),$(2),$(3),map-file))
argument-map-file-llvm-clang = $(addprefix -Xlinker -Map=$(call tool-parameter,$(1),$(2),$(3),map-file))
argument-map-file-gnu-gcc = $(addprefix -Xlinker -Map=$(call tool-parameter,$(1),$(2),$(3),map-file))

#
# Define the `include-dirs` argument.
#
# This argument defines the flags required to specify include directories when
# preprocessing source files with a C preprocessor. Include directories are
# given by the `include-dirs` argument parameter.
#

argument-include-dirs-arm-clang = $(addprefix -I,$(call tool-parameters,$(1),$(2),$(3),include-dirs))
argument-include-dirs-llvm-clang = $(addprefix -I,$(call tool-parameters,$(1),$(2),$(3),include-dirs))
argument-include-dirs-gnu-gcc = $(addprefix -I,$(call tool-parameters,$(1),$(2),$(3),include-dirs))

#
# Define the `defines` argument.
#
# This argument defines the flags required to specify preprocessor definitions
# when preprocessing source files with a C preprocessor. Preprocessor
# definitions are given by the `defines` argument parameter and are expected to
# be of the form `KEY[=VALUE]`.
#

argument-defines-arm-clang = $(addprefix -D,$(call tool-parameters,$(1),$(2),$(3),defines))
argument-defines-llvm-clang = $(addprefix -D,$(call tool-parameters,$(1),$(2),$(3),defines))
argument-defines-gnu-gcc = $(addprefix -D,$(call tool-parameters,$(1),$(2),$(3),defines))

#
# Define the `linker-scripts` argument.
#
# This argument defines the flags required to specify a scatter file, or one
# more linker scripts to the linker. If the tool supports scatter files, one may
# be given by the `scatter-file` argument parameter. Tools that accept GNU-like
# linker scripts may will retrieve the list of linker scripts from the
# `linker-script` argument parameter.
#

argument-linker-scripts-arm-clang = $(addprefix -Xlinker --scatter=,$(call tool-parameter,$(1),$(2),$(3),scatter-file))
argument-linker-scripts-llvm-clang = $(addprefix -Xlinker --script=,$(call tool-parameters,$(1),$(2),$(3),linker-scripts))
argument-linker-scripts-gnu-gcc = $(addprefix -Xlinker --script=,$(call tool-parameters,$(1),$(2),$(3),linker-scripts))

#
# Define the `library-search-paths` argument.
#
# This argument defines the flags required to specify one or more library search
# paths to the linker, and are given by the `library-search-paths` argument
# parameter.
#

argument-library-search-paths-arm-clang = $(addprefix -L,$(call tool-parameters,$(1),$(2),$(3),library-search-paths))
argument-library-search-paths-llvm-clang = $(addprefix -L,$(call tool-parameters,$(1),$(2),$(3),library-search-paths))
argument-library-search-paths-gnu-gcc = $(addprefix -L,$(call tool-parameters,$(1),$(2),$(3),library-search-paths))

#
# Define the `libraries` argument.
#
# This argument defines the flags required to specify one or more libraries for
# the linker to link, and are given by the `libraries` argument parameter.
#

argument-libraries-arm-clang = $(addprefix -l,$(call tool-parameters,$(1),$(2),$(3),libraries))
argument-libraries-llvm-clang = $(addprefix -l,$(call tool-parameters,$(1),$(2),$(3),libraries))
argument-libraries-gnu-gcc = $(addprefix -l,$(call tool-parameters,$(1),$(2),$(3),libraries))

#
# Define the `output-binary` argument.
#
# This argument defines the flags required to configure an object copier to
# generate raw binary output.
#

argument-output-binary-arm-fromelf = --bin
argument-output-binary-llvm-objcopy = -O binary
argument-output-binary-gnu-objcopy = -O binary

#
# Define the `inhibit-line-markers` argument.
#
# This argument defines the flags required to prevent the C preprocessor from
# generating additional implementation-specific line markers, which can confuse
# tools that take the preprocessed output as input if they do not know how to
# handle them.
#

argument-inhibit-line-markers-arm-clang = -P
argument-inhibit-line-markers-llvm-clang = -P
argument-inhibit-line-markers-gnu-gcc = -P

#
# Define the `entrypoint` argument.
#
# This argument defines the flags required to specify the entrypoint function
# in the binary, given by the `entrypoint` argument parameter.
#

argument-entrypoint-arm-clang = $(addprefix -Xlinker --entry=,$(call tool-parameter,$(1),$(2),$(3),entrypoint))
argument-entrypoint-llvm-clang = $(addprefix -Xlinker --entry=,$(call tool-parameter,$(1),$(2),$(3),entrypoint))
argument-entrypoint-gnu-gcc = $(addprefix -Xlinker --entry=,$(call tool-parameter,$(1),$(2),$(3),entrypoint))

#
# Define the `lto` argument.
#
# This argument defines the flags required to enable link-time optimization, and
# can applied to all tools that play some part in the compilation pipeline.
#

argument-lto-arm-clang = -flto
argument-lto-llvm-clang = -flto
argument-lto-gnu-gcc = -flto


#
# Define the `mcpu` argument.
#
# This argument models the `-mcpu` and `-march` flags supported by GCC and other
# compilers that emulate it. Only one of these flags is returned, with priority
# given to `-mcpu`. Values for `-mcpu` are given by the `mcpu-cpu` argument
# parameter, whilst the value for `-march` is given by the `mcpu-arch` argument.
#

aarch32-mcpu-cpus += $(if $(filter $(ARM_CORTEX_A17),yes),cortex-a17)
aarch32-mcpu-cpus += $(if $(filter $(ARM_CORTEX_A15),yes),cortex-a15)
aarch32-mcpu-cpus += $(if $(filter $(ARM_CORTEX_A12),yes),cortex-a12)
aarch32-mcpu-cpus += $(if $(filter $(ARM_CORTEX_A9),yes),cortex-a9)
aarch32-mcpu-cpus += $(if $(filter $(ARM_CORTEX_A7),yes),cortex-a7)
aarch32-mcpu-cpus += $(if $(filter $(ARM_CORTEX_A5),yes),cortex-a5)

aarch32-mcpu-arch ?= armv$(ARM_ARCH_MAJOR)$(if $(filter-out 0,$(ARM_ARCH_MINOR)),.$(ARM_ARCH_MINOR))-a
aarch64-mcpu-arch ?= armv$(ARM_ARCH_MAJOR)$(if $(filter-out 0,$(ARM_ARCH_MINOR)),.$(ARM_ARCH_MINOR))-a

aarch32-mcpu-features += $(filter-out none,$(subst +, ,$(ARM_ARCH_FEATURE)))
aarch64-mcpu-features += $(filter-out none,$(subst +, ,$(ARM_ARCH_FEATURE)))

argument-mcpu-arm-clang = $(call argument-mcpu-gnu-gcc,$(1),$(2),$(3))
argument-mcpu-llvm-clang = $(call argument-mcpu-gnu-gcc,$(1),$(2),$(3))

argument-mcpu-gnu-gcc = $(or $(call argument-mcpu-gnu-gcc-mcpu,$(1),$(2),$(3)),$(call argument-mcpu-gnu-gcc-march,$(1),$(2),$(3)))$(addprefix +,$(call argument-mcpu-gnu-gcc-features,$(1),$(2),$(3)))
argument-mcpu-gnu-gcc-mcpu = $(addprefix -mcpu=,$(subst $(space),.,$(call tool-parameters,$(1),$(2),$(3),mcpu-cpus)))
argument-mcpu-gnu-gcc-march = $(addprefix -march=,$(call tool-parameter,$(1),$(2),$(3),mcpu-arch))
argument-mcpu-gnu-gcc-features = $(subst $(space),+,$(call tool-parameters,$(1),$(2),$(3),mcpu-features))

#
# Define the `mfpu` argument.
#
# This argument models the `-mfpu` flag supported by GCC and other compilers
# that emulate it. The value for this argument is given by the `mfpu` argument
# parameter.
#

aarch32-mfpu-fpu ?= $(if $(filter $(ARM_WITH_NEON),yes),neon)

argument-arm-clang-mfpu = $(call argument-gnu-gcc-mfpu,$(1),$(2),$(3))
argument-llvm-clang-mfpu = $(call argument-gnu-gcc-mfpu,$(1),$(2),$(3))
argument-gnu-gcc-mfpu = $(addprefix -mfpu=,$(call tool-parameter,$(1),$(2),$(3),mfpu-fpu))
