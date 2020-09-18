/*
 * Copyright (c) 2020, MediaTek Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <common/debug.h>
#include <drivers/delay_timer.h>
#include <drivers/gpio.h>
#include <lib/mmio.h>
#include <mtgpio.h>
#include <platform_def.h>
#include <stdbool.h>

/******************************************************************************
 *Macro Definition
 ******************************************************************************/
#define GPIO_MODE_BITS		4
#define MAX_GPIO_MODE_PER_REG	8
#define MAX_GPIO_REG_BITS	32
#define DIR_BASE		(GPIO_BASE + 0x000)
#define DOUT_BASE		(GPIO_BASE + 0x100)
#define DIN_BASE		(GPIO_BASE + 0x200)
#define MODE_BASE		(GPIO_BASE + 0x300)
#define SET			0x4
#define CLR			0x8

static void mt_set_gpio_dir_chip(uint32_t pin, int dir)
{
	uint32_t pos, bit;

	assert(pin < MAX_GPIO_PIN);
	assert(dir < MT_GPIO_DIR_MAX);

	pos = pin / MAX_GPIO_REG_BITS;
	bit = pin % MAX_GPIO_REG_BITS;

	if (dir == MT_GPIO_DIR_IN)
		mmio_write_32(DIR_BASE + 0x10 * pos + CLR, 1U << bit);
	else
		mmio_write_32(DIR_BASE + 0x10 * pos + SET, 1U << bit);
}

static int mt_get_gpio_dir_chip(uint32_t pin)
{
	uint32_t pos, bit;
	uint32_t reg;

	assert(pin < MAX_GPIO_PIN);

	pos = pin / MAX_GPIO_REG_BITS;
	bit = pin % MAX_GPIO_REG_BITS;

	reg = mmio_read_32(DIR_BASE + 0x10 * pos);
	return (((reg & (1U << bit)) != 0) ? MT_GPIO_DIR_OUT : MT_GPIO_DIR_IN);
}

static void mt_set_gpio_out_chip(uint32_t pin, int output)
{
	uint32_t pos, bit;

	assert(pin < MAX_GPIO_PIN);
	assert(output < MT_GPIO_OUT_MAX);

	pos = pin / MAX_GPIO_REG_BITS;
	bit = pin % MAX_GPIO_REG_BITS;

	if (output == MT_GPIO_OUT_ZERO)
		mmio_write_32(DOUT_BASE + 0x10 * pos + CLR, 1U << bit);
	else
		mmio_write_32(DOUT_BASE + 0x10 * pos + SET, 1U << bit);
}

static int mt_get_gpio_out_chip(uint32_t pin)
{
	uint32_t pos, bit;
	uint32_t reg;

	assert(pin < MAX_GPIO_PIN);

	pos = pin / MAX_GPIO_REG_BITS;
	bit = pin % MAX_GPIO_REG_BITS;

	reg = mmio_read_32(DOUT_BASE + 0x10 * pos);
	return (((reg & (1U << bit)) != 0) ? 1 : 0);
}

static int mt_get_gpio_in_chip(uint32_t pin)
{
	uint32_t pos, bit;
	uint32_t reg;

	assert(pin < MAX_GPIO_PIN);

	pos = pin / MAX_GPIO_REG_BITS;
	bit = pin % MAX_GPIO_REG_BITS;

	reg = mmio_read_32(DIN_BASE + 0x10 * pos);
	return (((reg & (1U << bit)) != 0) ? 1 : 0);
}

static void mt_set_gpio_mode_chip(uint32_t pin, int mode)
{
	uint32_t pos, bit;
	uint32_t data;
	uint32_t mask;

	assert(pin < MAX_GPIO_PIN);
	assert(mode < GPIO_MODE_MAX);

	mask = (1U << GPIO_MODE_BITS) - 1;

	mode = mode & mask;
	pos = pin / MAX_GPIO_MODE_PER_REG;
	bit = (pin % MAX_GPIO_MODE_PER_REG) * GPIO_MODE_BITS;

	data = mmio_read_32(MODE_BASE + 0x10 * pos);
	data &= (~(mask << bit));
	data |= (mode << bit);
	mmio_write_32(MODE_BASE + 0x10 * pos, data);
}

static int mt_get_gpio_mode_chip(uint32_t pin)
{
	uint32_t pos, bit;
	uint32_t data;
	uint32_t mask;

	assert(pin < MAX_GPIO_PIN);

	mask = (1U << GPIO_MODE_BITS) - 1;

	pos = pin / MAX_GPIO_MODE_PER_REG;
	bit = (pin % MAX_GPIO_MODE_PER_REG) * GPIO_MODE_BITS;

	data = mmio_read_32(MODE_BASE + 0x10 * pos);
	return (data >> bit) & mask;
}

static uintptr_t mt_gpio_find_reg_addr(uint32_t pin)
{
	uintptr_t reg_addr;
	struct mt_pin_info gpio_info;

	gpio_info = mt8192_pin_infos[pin];
	switch(gpio_info.base & 0x0f) {
		case 0:
			reg_addr = IOCFG_RM_BASE;
			break;
		case 1:
			reg_addr = IOCFG_BM_BASE;
			break;
		case 2:
			reg_addr = IOCFG_BL_BASE;
			break;
		case 3:
			reg_addr = IOCFG_BR_BASE;
			break;
		case 4:
			reg_addr = IOCFG_LM_BASE;
			break;
		case 5:
			reg_addr = IOCFG_LB_BASE;
			break;
		case 6:
			reg_addr = IOCFG_RT_BASE;
			break;
		case 7:
			reg_addr = IOCFG_LT_BASE;
			break;
		case 8:
			reg_addr = IOCFG_TL_BASE;
			break;
		default:
			reg_addr = -1;
			break;
	}

	return reg_addr;
}

static void mt_gpio_set_spec_pull_pupd(uint32_t pin, int enable,
			       int select)
{
	uintptr_t reg1;
	uintptr_t reg2;
	struct mt_pin_info gpio_info;

	gpio_info = mt8192_pin_infos[pin];
	int bit = gpio_info.bit;

	reg1 = mt_gpio_find_reg_addr(pin) + gpio_info.offset;
	reg2 = reg1 + (gpio_info.base & 0xf0);
	if (enable == MT_GPIO_PULL_ENABLE) {
		mmio_write_32(reg2 + SET, (1 << bit));
		if (select == MT_GPIO_PULL_DOWN)
			mmio_write_32(reg1 + SET, (1 << bit));
		else
			mmio_write_32(reg1 + CLR, (1 << bit));
	} else {
		mmio_write_32(reg2 + CLR, (1 << bit));
		mmio_write_32((reg2 + 0x010) + CLR, (1 << bit));
	}
}

static void mt_gpio_set_pull_pu_pd(uint32_t pin, int enable,
				 int select)
{
	uintptr_t reg1;
	uintptr_t reg2;
	struct mt_pin_info gpio_info;

	gpio_info = mt8192_pin_infos[pin];
	int bit = gpio_info.bit;

	reg1 = mt_gpio_find_reg_addr(pin) + gpio_info.offset;
	reg2 = reg1 - (gpio_info.base & 0xf0);

	if (enable == MT_GPIO_PULL_ENABLE) {
		if (select == MT_GPIO_PULL_DOWN) {
			mmio_write_32(reg1 + CLR, (1 << bit));
			mmio_write_32(reg2 + SET, (1 << bit));
		} else {
			mmio_write_32(reg2 + CLR, (1 << bit));
			mmio_write_32(reg1 + SET, (1 << bit));
		}
	} else {
		mmio_write_32(reg1 + CLR , (1 << bit));
		mmio_write_32(reg2 + CLR, (1 << bit));
	}
}

static void mt_gpio_set_pull_chip(uint32_t pin, int enable,
		   int select)
{
	struct mt_pin_info gpio_info;

	gpio_info = mt8192_pin_infos[pin];
	if (gpio_info.flag)
		mt_gpio_set_spec_pull_pupd(pin, enable, select);
	else
		mt_gpio_set_pull_pu_pd(pin, enable, select);
}

static int mt_gpio_get_spec_pull_pupd(uint32_t pin)
{
	uintptr_t reg1;
	uintptr_t reg2;
	uint32_t r0;
	uint32_t r1;

	struct mt_pin_info gpio_info;

	gpio_info = mt8192_pin_infos[pin];
	int bit = gpio_info.bit;

	reg1 = mt_gpio_find_reg_addr(pin) + gpio_info.offset;
	reg2 = reg1 + (gpio_info.base & 0xf0);

	r0 = (mmio_read_32(reg2) >> bit) & 1;
	r1 = (mmio_read_32(reg2 + 0x010) >> bit) & 1;
	if (r0 == 0 && r1 == 0)
		return MT_GPIO_PULL_NONE;
	else {
		if(mmio_read_32(reg1) & (1 << bit))
			return MT_GPIO_PULL_DOWN;
		else
			return MT_GPIO_PULL_UP;
	}
}

static int mt_gpio_get_pull_pu_pd(uint32_t pin)
{
	uintptr_t reg1;
	uintptr_t reg2;
	uint32_t pu;
	uint32_t pd;

	struct mt_pin_info gpio_info;

	gpio_info = mt8192_pin_infos[pin];
	int bit = gpio_info.bit;

	reg1 = mt_gpio_find_reg_addr(pin) + gpio_info.offset;
	reg2 = reg1 - (gpio_info.base & 0xf0);
	pu = (mmio_read_32(reg1) >> bit) & 1;
	pd = (mmio_read_32(reg2) >> bit) & 1;
	if (pu == 1) {
		return MT_GPIO_PULL_UP;
	} else if (pd == 1) {
		return MT_GPIO_PULL_DOWN;
	} else {
	 	return MT_GPIO_PULL_NONE;
	}
}

static int mt_gpio_get_pull_chip(uint32_t pin)
{
	struct mt_pin_info gpio_info;

	gpio_info = mt8192_pin_infos[pin];
	if (gpio_info.flag)
		return mt_gpio_get_spec_pull_pupd(pin);
	else
		return mt_gpio_get_pull_pu_pd(pin);
}

static void mt_set_gpio_pull_select_chip(uint32_t pin, int sel)
{
	assert(pin < MAX_GPIO_PIN);

	if (sel == MT_GPIO_PULL_NONE) {
		mt_gpio_set_pull_chip(pin, MT_GPIO_PULL_DISABLE, MT_GPIO_PULL_DOWN);
	} else if (sel == MT_GPIO_PULL_UP) {
		mt_gpio_set_pull_chip(pin, MT_GPIO_PULL_ENABLE, MT_GPIO_PULL_UP);
	} else if (sel == MT_GPIO_PULL_DOWN) {
		mt_gpio_set_pull_chip(pin, MT_GPIO_PULL_ENABLE, MT_GPIO_PULL_DOWN);
	}
}

/* get pull-up or pull-down, regardless of resistor value */
static int mt_get_gpio_pull_select_chip(uint32_t pin)
{
	assert(pin < MAX_GPIO_PIN);

	return mt_gpio_get_pull_chip(pin);
}

void mt_set_gpio_dir(int gpio, int direction)
{
	mt_set_gpio_dir_chip((uint32_t)gpio, direction);
}

int mt_get_gpio_dir(int gpio)
{
	uint32_t pin;

	pin = (uint32_t)gpio;
	return mt_get_gpio_dir_chip(pin);
}

void mt_set_gpio_pull(int gpio, int pull)
{
	uint32_t pin;

	pin = (uint32_t)gpio;
	mt_set_gpio_pull_select_chip(pin, pull);
}

int mt_get_gpio_pull(int gpio)
{
	uint32_t pin;

	pin = (uint32_t)gpio;
	return mt_get_gpio_pull_select_chip(pin);
}

void mt_set_gpio_out(int gpio, int value)
{
	uint32_t pin;

	pin = (uint32_t)gpio;
	mt_set_gpio_out_chip(pin, value);
}

int mt_get_gpio_out(int gpio)
{
	uint32_t pin;

	pin = (uint32_t)gpio;
	return mt_get_gpio_out_chip(pin);
}

int mt_get_gpio_in(int gpio)
{
	uint32_t pin;

	pin = (uint32_t)gpio;
	return mt_get_gpio_in_chip(pin);
}

void mt_set_gpio_mode(int gpio, int mode)
{
	uint32_t pin;

	pin = (uint32_t)gpio;
	mt_set_gpio_mode_chip(pin, mode);
}

int mt_get_gpio_mode(int gpio)
{
	uint32_t pin;

	pin = (uint32_t)gpio;
	return mt_get_gpio_mode_chip(pin);
}

const gpio_ops_t mtgpio_ops = {
	 .get_direction = mt_get_gpio_dir,
	 .set_direction = mt_set_gpio_dir,
	 .get_value = mt_get_gpio_in,
	 .set_value = mt_set_gpio_out,
	 .set_pull = mt_set_gpio_pull,
	 .get_pull = mt_get_gpio_pull,
};
