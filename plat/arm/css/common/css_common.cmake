#-------------------------------------------------------------------------------
# Copyright (c) 2019-2020, Arm Limited. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#
#-------------------------------------------------------------------------------

if(NOT DEFINED PROJECT_SOURCE_DIR)
	message(FATAL_ERROR "PROJECT_SOURCE_DIR not defined")
endif()

# Include framework files
include(Common/STGT)

#plat_bl_common
stgt_add_src_param(NAME bl1 bl2 bl31 KEY ARCH SRC
	${PROJECT_SOURCE_DIR}/plat/arm/css/common/@ARCH@/css_helpers.S
)

stgt_get_param(NAME bl2 KEY CSS_USE_SCMI_SDS_DRIVER RET _css_use_scmi_sds_driver)
if(_css_use_scmi_sds_driver EQUAL 1)
	stgt_add_src_param(NAME bl1 bl2 bl31 KEY ARCH SRC
		${PROJECT_SOURCE_DIR}/drivers/arm/css/sds/@ARCH@/sds_helpers.S
	)
endif()
unset(_css_use_scmi_sds_driver)

#plat_bl1
stgt_add_src(NAME bl1 SRC
	${PROJECT_SOURCE_DIR}/plat/arm/css/common/css_bl1_setup.c
)

#plat_bl2
stgt_add_src(NAME bl2 SRC
	${PROJECT_SOURCE_DIR}/plat/arm/css/common/css_bl2_setup.c
)

#plat_bl31
stgt_add_src(NAME bl31 SRC
	${PROJECT_SOURCE_DIR}/plat/arm/css/common/css_pm.c
	${PROJECT_SOURCE_DIR}/plat/arm/css/common/css_topology.c
)

#TODO: if CSS_USE_SCMI_SDS_DRIVER=0
stgt_add_src_cond(NAME bl31 KEY CSS_USE_SCMI_SDS_DRIVER VAL 1 SRC
	${PROJECT_SOURCE_DIR}/drivers/arm/css/mhu/css_mhu_doorbell.c
	${PROJECT_SOURCE_DIR}/drivers/arm/css/scmi/scmi_ap_core_proto.c
	${PROJECT_SOURCE_DIR}/drivers/arm/css/scmi/scmi_common.c
	${PROJECT_SOURCE_DIR}/drivers/arm/css/scmi/scmi_pwr_dmn_proto.c
	${PROJECT_SOURCE_DIR}/drivers/arm/css/scmi/scmi_sys_pwr_proto.c
	${PROJECT_SOURCE_DIR}/drivers/arm/css/scp/css_pm_scmi.c
)

stgt_get_param(NAME bl2 KEY CSS_LOAD_SCP_IMAGES RET _css_load_scp_images)
if(_css_load_scp_images EQUAL 1)
	#TODO: TRUSTED_BOARD_BOOT
	stgt_add_src_cond(NAME bl2 KEY CSS_USE_SCMI_SDS_DRIVER VAL 0 SRC
		${PROJECT_SOURCE_DIR}/drivers/arm/css/mhu/css_mhu.c
		${PROJECT_SOURCE_DIR}/drivers/arm/css/scp/css_bom_bootloader.c
		${PROJECT_SOURCE_DIR}/drivers/arm/css/scpi/css_scpi.c
	)
	stgt_add_src_cond(NAME bl2 KEY CSS_USE_SCMI_SDS_DRIVER VAL 1 SRC
		${PROJECT_SOURCE_DIR}/drivers/arm/css/scp/css_sds.c
		${PROJECT_SOURCE_DIR}/drivers/arm/css/sds/sds.c
	)
endif()
unset(_css_load_scp_images)
