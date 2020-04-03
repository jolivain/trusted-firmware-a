#
# Copyright 2018-2020 NXP
#
# SPDX-License-Identifier: BSD-3-Clause
#
#

ifeq (${DDR_IMEM_UDIMM_1D},)
    DDR_IMEM_UDIMM_1D	:=	ddr4_pmu_train_imem.bin
endif

ifeq (${DDR_IMEM_UDIMM_2D},)
    DDR_IMEM_UDIMM_2D	:=	ddr4_2d_pmu_train_imem.bin
endif

ifeq (${DDR_DMEM_UDIMM_1D},)
    DDR_DMEM_UDIMM_1D	:=	ddr4_pmu_train_dmem.bin
endif

ifeq (${DDR_DMEM_UDIMM_2D},)
    DDR_DMEM_UDIMM_2D	:=	ddr4_2d_pmu_train_dmem.bin
endif

ifeq (${DDR_IMEM_RDIMM_1D},)
    DDR_IMEM_RDIMM_1D	:=	ddr4_rdimm_pmu_train_imem.bin
endif

ifeq (${DDR_IMEM_RDIMM_2D},)
    DDR_IMEM_RDIMM_2D	:=	ddr4_rdimm2d_pmu_train_imem.bin
endif

ifeq (${DDR_DMEM_RDIMM_1D},)
    DDR_DMEM_RDIMM_1D	:=	ddr4_rdimm_pmu_train_dmem.bin
endif

ifeq (${DDR_DMEM_RDIMM_2D},)
    DDR_DMEM_RDIMM_2D	:=	ddr4_rdimm2d_pmu_train_dmem.bin
endif

fip_ddr: fiptool
	./tools/fiptool/fiptool create  --ddr-immem-udimm-1d ${DDR_IMEM_UDIMM_1D} \
				        --ddr-immem-udimm-2d ${DDR_IMEM_UDIMM_2D} \
					--ddr-dmmem-udimm-1d ${DDR_DMEM_UDIMM_1D} \
					--ddr-dmmem-udimm-2d ${DDR_DMEM_UDIMM_2D} \
					--ddr-immem-rdimm-1d ${DDR_IMEM_RDIMM_1D} \
				        --ddr-immem-rdimm-2d ${DDR_IMEM_RDIMM_2D} \
					--ddr-dmmem-rdimm-1d ${DDR_DMEM_RDIMM_1D} \
					--ddr-dmmem-rdimm-2d ${DDR_DMEM_RDIMM_2D} $@.bin
