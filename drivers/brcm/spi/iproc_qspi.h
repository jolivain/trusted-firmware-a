/*
 * Copyright (c) 2017 - 2019, Broadcom
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef IPROC_QSPI_H
#define IPROC_QSPI_H

#include <platform_def.h>

/*SPI configuration enable*/
#define IPROC_QSPI_CLK_SPEED	62500000
#define SPI_CPHA		(1 << 0)
#define SPI_CPOL		(1 << 1)
#define IPROC_QSPI_MODE0	0
#define IPROC_QSPI_MODE3	(SPI_CPOL|SPI_CPHA)

#define IPROC_QSPI_BUS                   0
#define IPROC_QSPI_CS                    0
#define IPROC_QSPI_BASE_REG              QSPI_CTRL_BASE_ADDR
#define IPROC_QSPI_CRU_CONTROL_REG       QSPI_CLK_CTRL

int iproc_qspi_setup(uint32_t bus, uint32_t cs,
		     uint32_t max_hz, uint32_t mode);
int iproc_qspi_claim_bus(void);
void iproc_qspi_release_bus(void);
int iproc_qspi_xfer(uint32_t bitlen, const void *dout,
		    void *din, unsigned long flags);

#endif	/* _IPROC_QSPI_H_ */
