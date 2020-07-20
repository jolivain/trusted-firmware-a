#-------------------------------------------------------------------------------
# Copyright (c) 2019-2020, Arm Limited. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#
#-------------------------------------------------------------------------------

if(NOT DEFINED TFA_ROOT_DIR)
	message(FATAL_ERROR "TF-A root dir not defined")
endif()

stgt_get_param(NAME bl1 KEY ARCH RET _arch)

stgt_get_param(NAME bl1 KEY DTS_HW_CONFIG RET _dts_hw_config)
stgt_get_param(NAME bl1 KEY DTS_TB_FW_CONFIG RET _dts_tb_fw_config)
stgt_get_param(NAME bl1 KEY DTS_SOC_FW_CONFIG RET _dts_soc_fw_config)
stgt_get_param(NAME bl1 KEY DTS_NT_FW_CONFIG RET _dts_nt_fw_config)
stgt_get_param(NAME bl1 KEY DTS_FW_CONFIG RET _dts_fw_config)
stgt_get_param(NAME bl1 KEY DTS_TOS_FW_CONFIG RET _dts_tos_fw_config)

# dtc complation needs the dtc tool
find_package(dtc REQUIRED)

dtc_compile(SRC ${_dts_hw_config} DST ${CMAKE_BINARY_DIR}/hw.dtb)
dtc_compile(SRC ${_dts_tb_fw_config} DST ${CMAKE_BINARY_DIR}/tb_fw.dtb)
dtc_compile(SRC ${_dts_soc_fw_config} DST ${CMAKE_BINARY_DIR}/soc_fw.dtb)
dtc_compile(SRC ${_dts_nt_fw_config} DST ${CMAKE_BINARY_DIR}/nt_fw.dtb)
dtc_compile(SRC ${_dts_fw_config} DST ${CMAKE_BINARY_DIR}/fw.dtb)
dtc_compile(SRC ${_dts_tos_fw_config} DST ${CMAKE_BINARY_DIR}/tos_fw.dtb)

# fip package creation need the fiptool
find_package(fiptool REQUIRED)


set(_fiptool_args
	--hw-config ${CMAKE_BINARY_DIR}/hw.dtb
	--tb-fw-config ${CMAKE_BINARY_DIR}/tb_fw.dtb
	--nt-fw-config ${CMAKE_BINARY_DIR}/nt_fw.dtb
	--fw-config ${CMAKE_BINARY_DIR}/fw.dtb

	--tb-fw ${CMAKE_BINARY_DIR}/bl2.bin
	#TODO: retrieve this path from the config or command line
	#--nt-fw ${TFA_ROOT_DIR}/../tf-a-tests/build/fvp/debug/tftf.bin
)

# Package BL31 if ARCH == aarch64
if(_arch STREQUAL aarch64)
	list(APPEND _fiptool_args --soc-fw-config ${CMAKE_BINARY_DIR}/soc_fw.dtb)
	list(APPEND _fiptool_args --soc-fw ${CMAKE_BINARY_DIR}/bl31.bin)
endif()

# Package BL32 if AARCH32_SP set
if(AARCH32_SP)
	list(APPEND _fiptool_args --tos-fw-config ${CMAKE_BINARY_DIR}/tos_fw.dtb)
	list(APPEND _fiptool_args --tos-fw ${CMAKE_BINARY_DIR}/bl32.bin)
endif()

# FIP output file name
list(APPEND _fiptool_args ${CMAKE_BINARY_DIR}/fip.bin)

add_custom_target(fip ALL
	DEPENDS bl2
		$<$<STREQUAL:${_arch},aarch64>:bl31>
		$<$<BOOL:${AARCH32_SP}>:bl32>
		${CMAKE_BINARY_DIR}/hw.dtb
		${CMAKE_BINARY_DIR}/tb_fw.dtb
		${CMAKE_BINARY_DIR}/nt_fw.dtb
		${CMAKE_BINARY_DIR}/fw.dtb
		$<$<STREQUAL:${_arch},aarch64>:${CMAKE_BINARY_DIR}/soc_fw.dtb>
		$<$<BOOL:${AARCH32_SP}>:${CMAKE_BINARY_DIR}/tos_fw.dtb>
	COMMAND ${fiptool} create ${_fiptool_args}
)

