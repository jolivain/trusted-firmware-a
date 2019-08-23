#-------------------------------------------------------------------------------
# Copyright (c) 2019-2020, Arm Limited. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#
#-------------------------------------------------------------------------------

if(NOT DEFINED TFA_ROOT_DIR)
	message(FATAL_ERROR "TF-A root dir not defined")
endif()

stgt_get_param(NAME bl1 KEY DTS_HW_CONFIG RET _dts_hw_config)
stgt_get_param(NAME bl1 KEY DTS_TB_FW_CONFIG RET _dts_tb_fw_config)
stgt_get_param(NAME bl1 KEY DTS_SOC_FW_CONFIG RET _dts_soc_fw_config)
stgt_get_param(NAME bl1 KEY DTS_NT_FW_CONFIG RET _dts_nt_fw_config)

# dtc complation needs the dtc tool
find_package(dtc REQUIRED)

dtc_compile(SRC ${_dts_hw_config} DST ${CMAKE_BINARY_DIR}/hw.dtb)
dtc_compile(SRC ${_dts_tb_fw_config} DST ${CMAKE_BINARY_DIR}/tb_fw.dtb)
dtc_compile(SRC ${_dts_soc_fw_config} DST ${CMAKE_BINARY_DIR}/soc_fw.dtb)
dtc_compile(SRC ${_dts_nt_fw_config} DST ${CMAKE_BINARY_DIR}/nt_fw.dtb)

# fip package creation need the fiptool
find_package(fiptool REQUIRED)

add_custom_target(fip ALL
	DEPENDS bl2 bl31
		${CMAKE_BINARY_DIR}/hw.dtb
		${CMAKE_BINARY_DIR}/tb_fw.dtb
		${CMAKE_BINARY_DIR}/soc_fw.dtb
		${CMAKE_BINARY_DIR}/nt_fw.dtb
	COMMAND ${fiptool} create
		--hw-config ${CMAKE_BINARY_DIR}/hw.dtb
		--tb-fw-config ${CMAKE_BINARY_DIR}/tb_fw.dtb
		--soc-fw-config ${CMAKE_BINARY_DIR}/soc_fw.dtb
		--nt-fw-config ${CMAKE_BINARY_DIR}/nt_fw.dtb
		--tb-fw ${CMAKE_BINARY_DIR}/bl2.bin
		--soc-fw ${CMAKE_BINARY_DIR}/bl31.bin
		#TODO: retrieve this path from the config or command line
		#--nt-fw ${TFA_ROOT_DIR}/../fvp-latest-busybox-uboot/bl33-uboot.bin
		${CMAKE_BINARY_DIR}/fip.bin
)
