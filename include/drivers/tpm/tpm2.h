/*
 * Copyright (c) 2023-2024, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef TPM2_H
#define TPM2_H


#include <stdint.h>
#include <errno.h>
#include <endian.h>
#include <assert.h>
#include <drivers/tpm/tpm2_chip.h>


#pragma pack (1)
typedef struct {
	uint8_t	access;		 	  /* 0h */
	uint8_t	reserved1[7];	  /* 1h */
	uint32_t intr_enable;	  /* 8h */
	uint8_t	intr_vector;	  /* 0ch */
	uint8_t	reserved2[3];	  /* 0dh */
	uint32_t intr_sts;	 	  /* 10h */
	uint32_t intf_caps;	 	  /* 14h */
	uint8_t	status;		 	  /* 18h */
	uint16_t burst_count;	  /* 19h */
	uint8_t	reserved3[9];	  /* 21h */
	uint32_t data_fifo;	 	  /* 24h */
	uint8_t	reserved4[0xed8]; /* 28h */
	uint16_t vendid;	 	  /* 0f00h */
	uint16_t devid;		 	  /* 0f02h */
	uint8_t	revid;		 	  /* 0f04h */
} tpm_registers;
#pragma pack ()

#define TPM_ST_NO_SESSIONS	U(0x8001)
#define TPM_ST_SESSIONS		U(0x8002)

#define TPM_SU_CLEAR		U(0x0000)
#define	TPM_SU_STATE		U(0x0001)

#define	TPM_RS_PW			0x40000009

#define TPM_CMD_STARTUP		U(0x0144)
#define TPM_CMD_PCR_READ    U(0x017E)
#define TPM_CMD_PCR_EXTEND	U(0x0182)

#define	TPM_SUCCESS			U(0x0000)

#define	TPM_ACCESS_ACTIVE_LOCALITY		U(1 << 5)
#define TPM_ACCESS_VALID				U(1 << 7)
#define TPM_ACCESS_RELINQUISH_LOCALITY	U(1 << 5)
#define	TPM_ACCESS_REQUEST_USE			U(1 << 1)
#define TPM_ACCESS_REQUEST_PENDING		U(1 << 2)

#define	TPM_STAT_VALID			U(1 << 7)
#define	TPM_STAT_COMMAND_READY	U(1 << 6)
#define	TPM_STAT_GO				U(1 << 5)
#define	TPM_STAT_AVAIL			U(1 << 4)
#define TPM_STAT_EXPECT			U(1 << 3)

#define TPM_HEADER_SIZE		10
#define MAX_SIZE_CMDBUF		256

#pragma pack(1)
typedef struct tpm_cmd_hdr {
	uint16_t session;
	uint32_t cmd_size;
	uint32_t cmd;
} tpm_cmd_hdr;

typedef struct tpm_cmd {
	tpm_cmd_hdr cmdinf;
	uint8_t  data[255];
	uint8_t  data_length;
} tpm_cmd;
#pragma pack()

int tpm_interface_init(chip_data_t *chip_data, uint8_t locality);

void tpm_interface_close(chip_data_t *chip_data, uint8_t locality);

uint32_t tpm_startup(chip_data_t *chip_data , uint16_t mode);

uint32_t tpm_pcr_extend(chip_data_t* chip_data, uint32_t index,
						uint16_t algorithm, const uint8_t *digest,
						uint32_t digest_len);

#endif /* TPM2_H */
