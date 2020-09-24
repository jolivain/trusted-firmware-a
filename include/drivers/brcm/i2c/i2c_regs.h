/*
 * Copyright (c) 2016 - 2020, Broadcom
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef I2C_REGS
#define I2C_REGS

/* SMBUS Config register */
#define SMB_CFG_REG 0x0

#define SMB_CFG_RST_MASK                         0x80000000
#define SMB_CFG_RST_SHIFT                        31

#define SMB_CFG_SMBEN_MASK                       0x40000000
#define SMB_CFG_SMBEN_SHIFT                      30

#define SMB_CFG_BITBANGEN_MASK                   0x20000000
#define SMB_CFG_BITBANGEN_SHIFT                  29

#define SMB_CFG_EN_NIC_SMBADDR0_MASK             0x10000000
#define SMB_CFG_EN_NIC_SMBADDR0_SHIFT            28

#define SMB_CFG_PROMISCMODE_MASK                 0x08000000
#define SMB_CFG_PROMISCMODE_SHIFT                27

#define SMB_CFG_TSTMPCNTEN_MASK                  0x04000000
#define SMB_CFG_TSTMPCNTEN_SHIFT                 26

#define SMB_CFG_MSTRRTRYCNT_MASK                 0x000F0000
#define SMB_CFG_MSTRRTRYCNT_SHIFT                16

/* SMBUS Timing config register */
#define SMB_TIMGCFG_REG 0x4

#define SMB_TIMGCFG_MODE400_MASK                 0x80000000
#define SMB_TIMGCFG_MODE400_SHIFT                31

#define SMB_TIMGCFG_RNDSLVSTR_MASK               0x7F000000
#define SMB_TIMGCFG_RNDSLVSTR_SHIFT              24

#define SMB_TIMGCFG_PERSLVSTR_MASK               0x00FF0000
#define SMB_TIMGCFG_PERSLVSTR_SHIFT              16

#define SMB_TIMGCFG_IDLTIME_MASK                 0x0000FF00
#define SMB_TIMGCFG_IDLTIME_SHIFT                8

/* SMBUS Slave address register */
#define SMB_ADDR_REG 0x8

#define SMB_EN_NIC_SMBADDR3_MASK                 0x80000000
#define SMB_EN_NIC_SMBADDR3_SHIFT                31

#define SMB_NIC_SMBADDR3_MASK                    0x7F000000
#define SMB_NIC_SMBADDR3_SHIFT                   24

#define SMB_EN_NIC_SMBADDR2_MASK                 0x00800000
#define SMB_EN_NIC_SMBADDR2_SHIFT                23

#define SMB_NIC_SMBADDR2_MASK                    0x007F0000
#define SMB_NIC_SMBADDR2_SHIFT                   16

#define SMB_EN_NIC_SMBADDR1_MASK                 0x00008000
#define SMB_EN_NIC_SMBADDR1_SHIFT                15

#define SMB_NIC_SMBADDR1_MASK                    0x00007F00
#define SMB_NIC_SMBADDR1_SHIFT                   8

#define SMB_EN_NIC_SMBADDR0_MASK                 0x00000080
#define SMB_EN_NIC_SMBADDR0_SHIFT                7

#define SMB_NIC_SMBADDR0_MASK                    0x0000007F
#define SMB_NIC_SMBADDR0_SHIFT                   0

/* SMBUS Master FIFO control register */
#define SMB_MSTRFIFOCTL_REG 0xC

#define SMB_MSTRRXFIFOFLSH_MASK                  0x80000000
#define SMB_MSTRRXFIFOFLSH_SHIFT                 31

#define SMB_MSTRTXFIFOFLSH_MASK                  0x40000000
#define SMB_MSTRTXFIFOFLSH_SHIFT                 30

#define SMB_MSTRRXPKTCNT_MASK                    0x007F0000
#define SMB_MSTRRXPKTCNT_SHIFT                   16

#define SMB_MSTRRXFIFOTHR_MASK                   0x00003F00
#define SMB_MSTRRXFIFOTHR_SHIFT                  8

/* SMBUS Slave FIFO control register */
#define SMB_SLVFIFOCTL_REG 0x10

#define SMB_SLVRXFIFOFLSH_MASK                   0x80000000
#define SMB_SLVRXFIFOFLSH_SHIFT                  31

#define SMB_SLVTXFIFOFLSH_MASK                   0x40000000
#define SMB_SLVTXFIFOFLSH_SHIFT                  30

#define SMB_SLVRXPKTCNT_MASK                     0x007F0000
#define SMB_SLVRXPKTCNT_SHIFT                    16

#define SMB_SLVRXFIFOTHR_MASK                    0x00003F00
#define SMB_SLVRXFIFOTHR_SHIFT                   8

/* SMBUS Bit-bang mode control register */
#define SMB_BITBANGCTL_REG 0x14

#define SMB_SMBCLKIN_MASK                        0x80000000
#define SMB_SMBCLKIN_SHIFT                       31

#define SMB_SMBCLKOUTEN_MASK                     0x40000000
#define SMB_SMBCLKOUTEN_SHIFT                    30

#define SMB_SMBDATAIN_MASK                       0x20000000
#define SMB_SMBDATAIN_SHIFT                      29

#define SMB_SMBDATAOUTEN_MASK                    0x10000000
#define SMB_SMBDATAOUTEN_SHIFT                   28

/* SMBUS Master command register */
#define SMB_MSTRCMD_REG 0x30

#define SMB_MSTRSTARTBUSYCMD_MASK                0x80000000
#define SMB_MSTRSTARTBUSYCMD_SHIFT               31

#define SMB_MSTRABORT_MASK                       0x40000000
#define SMB_MSTRABORT_SHIFT                      30

#define SMB_MSTRSTS_MASK                         0x0E000000
#define SMB_MSTRSTS_SHIFT                        25

#define SMB_MSTRSMBUSPROTO_MASK                  0x00001E00
#define SMB_MSTRSMBUSPROTO_SHIFT                 9

#define SMB_MSTRPEC_MASK                         0x00000100
#define SMB_MSTRPEC_SHIFT                        8

#define SMB_MSTRRDBYTECNT_MASK                   0x000000FF
#define SMB_MSTRRDBYTECNT_SHIFT                  0

/* SMBUS Slave command register */
#define SMB_SLVCMD_REG 0x34

#define SMB_SLVSTARTBUSYCMD_MASK                 0x80000000
#define SMB_SLVSTARTBUSYCMD_SHIFT                31

#define SMB_SLVABORT_MASK                        0x40000000
#define SMB_SLVABORT_SHIFT                       30

#define SMB_SLVSTS_MASK                          0x03800000
#define SMB_SLVSTS_SHIFT                         23

#define SMB_SLVPEC_MASK                          0x00000100
#define SMB_SLVPEC_SHIFT                         8

/* SMBUS Event enable register */
#define SMB_EVTEN_REG 0x38

#define SMB_MSTRRXFIFOFULLEN_MASK                0x80000000
#define SMB_MSTRRXFIFOFULLEN_SHIFT               31

#define SMB_MSTRRXFIFOTHRHITEN_MASK              0x40000000
#define SMB_MSTRRXFIFOTHRHITEN_SHIFT             30

#define SMB_MSTRRXEVTEN_MASK                     0x20000000
#define SMB_MSTRRXEVTEN_SHIFT                    29

#define SMB_MSTRSTARTBUSYEN_MASK                 0x10000000
#define SMB_MSTRSTARTBUSYEN_SHIFT                28

#define SMB_MSTRTXUNDEN_MASK                     0x08000000
#define SMB_MSTRTXUNDEN_SHIFT                    27

#define SMB_SLVRXFIFOFULLEN_MASK                 0x04000000
#define SMB_SLVRXFIFOFULLEN_SHIFT                26

#define SMB_SLVRXFIFOTHRHITEN_MASK               0x02000000
#define SMB_SLVRXFIFOTHRHITEN_SHIFT              25

#define SMB_SLVRXEVTEN_MASK                      0x01000000
#define SMB_SLVRXEVTEN_SHIFT                     24

#define SMB_SLVSTARTBUSYEN_MASK                  0x00800000
#define SMB_SLVSTARTBUSYEN_SHIFT                 23

#define SMB_SLVTXUNDEN_MASK                      0x00400000
#define SMB_SLVTXUNDEN_SHIFT                     22

#define SMB_SLVRDEVTEN_MASK                      0x00200000
#define SMB_SLVRDEVTEN_SHIFT                     21

/* SMBUS Event status register */
#define SMB_EVTSTS_REG 0x3C

#define SMB_MSTRRXFIFOFULLSTS_MASK               0x80000000
#define SMB_MSTRRXFIFOFULLSTS_SHIFT              31

#define SMB_MSTRRXFIFOTHRHITSTS_MASK             0x40000000
#define SMB_MSTRRXFIFOTHRHITSTS_SHIFT            30

#define SMB_MSTRRXEVTSTS_MASK                    0x20000000
#define SMB_MSTRRXEVTSTS_SHIFT                   29

#define SMB_MSTRSTARTBUSYSTS_MASK                0x10000000
#define SMB_MSTRSTARTBUSYSTS_SHIFT               28

#define SMB_MSTRTXUNDSTS_MASK                    0x08000000
#define SMB_MSTRTXUNDSTS_SHIFT                   27

#define SMB_SLVRXFIFOFULLSTS_MASK                0x04000000
#define SMB_SLVRXFIFOFULLSTS_SHIFT               26

#define SMB_SLVRXFIFOTHRHITSTS_MASK              0x02000000
#define SMB_SLVRXFIFOTHRHITSTS_SHIFT             25

#define SMB_SLVRXEVTSTS_MASK                     0x01000000
#define SMB_SLVRXEVTSTS_SHIFT                    24

#define SMB_SLVSTARTBUSYSTS_MASK                 0x00800000
#define SMB_SLVSTARTBUSYSTS_SHIFT                23

#define SMB_SLVTXUNDSTS_MASK                     0x00400000
#define SMB_SLVTXUNDSTS_SHIFT                    22

#define SMB_SLVRDEVTSTS_MASK                     0x00200000
#define SMB_SLVRDEVTSTS_SHIFT                    21

/* SMBUS Master data write register */
#define SMB_MSTRDATAWR_REG 0x40

#define SMB_MSTRWRSTS_MASK                       0x80000000
#define SMB_MSTRWRSTS_SHIFT                      31

#define SMB_MSTRWRDATA_MASK                      0x000000FF
#define SMB_MSTRWRDATA_SHIFT                     0

/* SMBUS Master data read register */
#define SMB_MSTRDATARD_REG 0x44

#define SMB_MSTRRDSTS_MASK                       0xC0000000
#define SMB_MSTRRDSTS_SHIFT                      30

#define SMB_MSTRRDPECERR_MASK                    0x20000000
#define SMB_MSTRRDPECERR_SHIFT                   29

#define SMB_MSTRRDDATA_MASK                      0x000000FF
#define SMB_MSTRRDDATA_SHIFT                     0

/* SMBUS Slave data write register */
#define SMB_SLVDATAWR_REG 0x48

#define SMB_SLVWRSTS_MASK                        0x80000000
#define SMB_SLVWRSTS_SHIFT                       31

#define SMB_SLVWRDATA_MASK                       0x000000FF
#define SMB_SLVWRDATA_SHIFT                      0

/* SMBUS Slave data read register */
#define SMB_SLVDATARD_REG 0x4C

#define SMB_SLVRDSTS_MASK                        0xC0000000
#define SMB_SLVRDSTS_SHIFT                       30

#define SMB_SLVRDERRSTS_MASK                     0x30000000
#define SMB_SLVRDERRSTS_SHIFT                    28

#define SMB_SLVRDDATA_MASK                       0x000000FF
#define SMB_SLVRDDATA_SHIFT                      0

#endif /* I2C_REGS */
