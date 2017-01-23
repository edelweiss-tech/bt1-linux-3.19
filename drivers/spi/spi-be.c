/*
 * Baikal SPI core boot controller driver (refer spi-dw.c)
 *
 * Copyright (c) 2015, Baikal Electronics OJSC.
 * Copyright (c) 2009, Intel Corporation.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms and conditions of the GNU General Public License,
 * version 2, as published by the Free Software Foundation.
 *
 * This program is distributed in the hope it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin St - Fifth Floor, Boston, MA 02110-1301 USA.
 */

#include <linux/interrupt.h>
#include <linux/module.h>
#include <linux/highmem.h>
#include <linux/delay.h>
#include <linux/slab.h>
#include <linux/spi/spi.h>

#include "spi-be.h"

#ifdef CONFIG_DEBUG_FS
#include <linux/debugfs.h>
#define SSI_RESOURCE_NAME "boot_spi"
#endif

#define START_STATE	((void *)0)
#define RUNNING_STATE	((void *)1)
#define DONE_STATE	((void *)2)
#define ERROR_STATE	((void *)-1)

/* Slave spi_dev related */
struct chip_data {
	u16 cr0;
	u8 cs;			/* chip select pin */
	u8 n_bytes;		/* current is a 1/2/4 byte op */
	u8 tmode;		/* TR/TO/RO/EEPROM */
	u8 type;		/* SPI/SSP/MicroWire */

	u8 poll_mode;		/* 1 means use poll mode */

	u32 rx_threshold;
	u32 tx_threshold;
	u8 bits_per_word;
	u16 clk_div;		/* baud rate divider */
	u32 speed_hz;		/* baud rate */
	void (*cs_control)(u32 command);
};

#ifdef CONFIG_DEBUG_FS
#define SPI_REGS_BUFSIZE	1024
static ssize_t  spi_show_regs(struct file *file, char __user *user_buf,
				size_t count, loff_t *ppos)
{
	struct be_spi *bes;
	char *buf;
	u32 len = 0;
	ssize_t ret;

	bes = file->private_data;

	buf = kzalloc(SPI_REGS_BUFSIZE, GFP_KERNEL);
	if (!buf)
		return 0;

	len += snprintf(buf + len, SPI_REGS_BUFSIZE - len,
			"MRST SPI%d registers:\n", bes->bus_num);
	len += snprintf(buf + len, SPI_REGS_BUFSIZE - len,
			"=================================\n");
	len += snprintf(buf + len, SPI_REGS_BUFSIZE - len,
			"CTRL0: \t\t0x%08x\n", be_readl(bes, BE_SPI_CTRL0));
	len += snprintf(buf + len, SPI_REGS_BUFSIZE - len,
			"CTRL1: \t\t0x%08x\n", be_readl(bes, BE_SPI_CTRL1));
	len += snprintf(buf + len, SPI_REGS_BUFSIZE - len,
			"SSIENR: \t0x%08x\n", be_readl(bes, BE_SPI_SSIENR));
	len += snprintf(buf + len, SPI_REGS_BUFSIZE - len,
			"SER: \t\t0x%08x\n", be_readl(bes, BE_SPI_SER));
	len += snprintf(buf + len, SPI_REGS_BUFSIZE - len,
			"BAUDR: \t\t0x%08x\n", be_readl(bes, BE_SPI_BAUDR));
	len += snprintf(buf + len, SPI_REGS_BUFSIZE - len,
			"TXFTLR: \t0x%08x\n", be_readl(bes, BE_SPI_TXFLTR));
	len += snprintf(buf + len, SPI_REGS_BUFSIZE - len,
			"RXFTLR: \t0x%08x\n", be_readl(bes, BE_SPI_RXFLTR));
	len += snprintf(buf + len, SPI_REGS_BUFSIZE - len,
			"TXFLR: \t\t0x%08x\n", be_readl(bes, BE_SPI_TXFLR));
	len += snprintf(buf + len, SPI_REGS_BUFSIZE - len,
			"RXFLR: \t\t0x%08x\n", be_readl(bes, BE_SPI_RXFLR));
	len += snprintf(buf + len, SPI_REGS_BUFSIZE - len,
			"SR: \t\t0x%08x\n", be_readl(bes, BE_SPI_SR));
	len += snprintf(buf + len, SPI_REGS_BUFSIZE - len,
			"IMR: \t\t0x%08x\n", be_readl(bes, BE_SPI_IMR));
	len += snprintf(buf + len, SPI_REGS_BUFSIZE - len,
			"ISR: \t\t0x%08x\n", be_readl(bes, BE_SPI_ISR));
	len += snprintf(buf + len, SPI_REGS_BUFSIZE - len,
			"DMACR: \t\t0x%08x\n", be_readl(bes, BE_SPI_DMACR));
	len += snprintf(buf + len, SPI_REGS_BUFSIZE - len,
			"DMATDLR: \t0x%08x\n", be_readl(bes, BE_SPI_DMATDLR));
	len += snprintf(buf + len, SPI_REGS_BUFSIZE - len,
			"DMARDLR: \t0x%08x\n", be_readl(bes, BE_SPI_DMARDLR));
	len += snprintf(buf + len, SPI_REGS_BUFSIZE - len,
			"=================================\n");

	ret =  simple_read_from_buffer(user_buf, count, ppos, buf, len);
	kfree(buf);
	return ret;
}

static const struct file_operations mrst_spi_regs_ops = {
	.owner		= THIS_MODULE,
	.open		= simple_open,
	.read		= spi_show_regs,
	.llseek		= default_llseek,
};

static int mrst_spi_debugfs_init(struct be_spi *bes)
{
	char name[12];

	/* Create instance name */
	sprintf(name, "%s%d", SSI_RESOURCE_NAME, bes->bus_num);
	/* Create debugfs entries */
	bes->debugfs = debugfs_create_dir(name, NULL);
	if (!bes->debugfs)
		return -ENOMEM;

	debugfs_create_file("registers", S_IFREG | S_IRUGO,
		bes->debugfs, (void *)bes, &mrst_spi_regs_ops);
	return 0;
}

static void mrst_spi_debugfs_remove(struct be_spi *bes)
{
	if (bes->debugfs)
		debugfs_remove_recursive(bes->debugfs);
}

#else
static inline int mrst_spi_debugfs_init(struct be_spi *bes)
{
	return 0;
}

static inline void mrst_spi_debugfs_remove(struct be_spi *bes)
{
}
#endif /* CONFIG_DEBUG_FS */

/* Return the max entries we can fill into tx fifo */
static inline u32 tx_max(struct be_spi *bes)
{
	u32 tx_left, tx_room, rxtx_gap;

	tx_left = (bes->tx_end - bes->tx) / bes->n_bytes;
	tx_room = bes->fifo_len - be_readl(bes, BE_SPI_TXFLR);

	/*
	 * Another concern is about the tx/rx mismatch, we
	 * though to use (bes->fifo_len - rxflr - txflr) as
	 * one maximum value for tx, but it doesn't cover the
	 * data which is out of tx/rx fifo and inside the
	 * shift registers. So a control from sw point of
	 * view is taken.
	 */
	rxtx_gap =  ((bes->rx_end - bes->rx) - (bes->tx_end - bes->tx))
			/ bes->n_bytes;

	pr_info("SPI: rx_max: tx_left=%d, tx_room=%d, rxtx_gap=%d, fifo_rxtx_gap=%d\n",
		tx_left, tx_room, rxtx_gap, (bes->fifo_len - rxtx_gap));

	return min3(tx_left, tx_room, (u32) (bes->fifo_len - rxtx_gap));
}

/* Return the max entries we should read out of rx fifo */
static inline u32 rx_max(struct be_spi *bes)
{
	u32 rx_left = (bes->rx_end - bes->rx) / bes->n_bytes;

	pr_info("SPI: rx_max: rx_left=%d, rxflr=%d\n", rx_left, be_readl(bes, BE_SPI_RXFLR));

	return min(rx_left, (u32)be_readl(bes, BE_SPI_RXFLR));
}

static void be_writer(struct be_spi *bes)
{
	u32 max = tx_max(bes);
	u16 txw = 0;

	while (max--) {
		/* Set the tx word if the transfer's original "tx" is not null */
		if (bes->tx_end - bes->len) {
			if (bes->n_bytes == 1)
				txw = *(u8 *)(bes->tx);
			else
				txw = *(u16 *)(bes->tx);
		}
		be_writel(bes, BE_SPI_DR, txw);
		pr_info("---> spi_tx=%02x  max=%d\n", txw, max
			);
		bes->tx += bes->n_bytes;
	}
	pr_info("SPI: be_writer done\n");
}

static void be_reader(struct be_spi *bes)
{
	u32 max = rx_max(bes);
	u16 rxw;

	while (max--) {
		rxw = be_readl(bes, BE_SPI_DR);
		pr_info("<--- spi_rx=%02x  max=%d\n", rxw, max);
		/* Care rx only if the transfer's original "rx" is not null */
		if (bes->rx_end - bes->len) {
			if (bes->n_bytes == 1)
				*(u8 *)(bes->rx) = rxw;
			else
				*(u16 *)(bes->rx) = rxw;
		}
		bes->rx += bes->n_bytes;
	}
	pr_info("SPI: be_reader done\n");
}

static void *next_transfer(struct be_spi *bes)
{
	struct spi_message *msg = bes->cur_msg;
	struct spi_transfer *trans = bes->cur_transfer;

	pr_info("next_transfer\n");

	/* Move to next transfer */
	if (trans->transfer_list.next != &msg->transfers) {
		bes->cur_transfer =
			list_entry(trans->transfer_list.next,
					struct spi_transfer,
					transfer_list);
		return RUNNING_STATE;
	} else
		return DONE_STATE;
}

/* Caller already set message->status and pio irqs are blocked */
static void giveback(struct be_spi *bes)
{
	struct spi_transfer *last_transfer;
	struct spi_message *msg;

	msg = bes->cur_msg;
	bes->cur_msg = NULL;
	bes->cur_transfer = NULL;
	bes->prev_chip = bes->cur_chip;
	bes->cur_chip = NULL;

	last_transfer = list_last_entry(&msg->transfers, struct spi_transfer,
					transfer_list);

	if (!last_transfer->cs_change)
		spi_chip_sel(bes, msg->spi, 0);

	spi_finalize_current_message(bes->master);
	pr_info("giveback done\n");
}

void be_spi_xfer_done(struct be_spi *bes)
{
	/* Update total byte transferred return count actual bytes read */
	bes->cur_msg->actual_length += bes->len;

	/* Move to next transfer */
	bes->cur_msg->state = next_transfer(bes);

	pr_info("be_spi_xfer_done\n");
	/* Handle end of message */
	if (bes->cur_msg->state == DONE_STATE) {
		bes->cur_msg->status = 0;
		giveback(bes);
	} else
		tasklet_schedule(&bes->pump_transfers);
}
EXPORT_SYMBOL_GPL(be_spi_xfer_done);

/* Must be called inside pump_transfers() */
static void poll_transfer(struct be_spi *bes)
{
	int i = 0;
	do {
		be_writer(bes);
		be_reader(bes);
		cpu_relax();
		i++;
		pr_info("SPI: [poll] tick=%d  rx_end_rx=%d\n", i, ((u32)bes->rx_end - (u32)bes->rx));
	} while (bes->rx_end > bes->rx);

	pr_info("poll_transfer: tick_count=%d\n", i);

	be_spi_xfer_done(bes);
}

static void pump_transfers(unsigned long data)
{
	struct be_spi *bes = (struct be_spi *)data;
	struct spi_message *message = NULL;
	struct spi_transfer *transfer = NULL;
	struct spi_transfer *previous = NULL;
	struct spi_device *spi = NULL;
	struct chip_data *chip = NULL;
	u8 bits = 0;
	u8 imask = 0;
	u8 cs_change = 0;
	u16 txint_level = 0;
	u16 clk_div = 0;
	u32 speed = 0;
	u32 cr0 = 0;

	/* Get current state information */
	message = bes->cur_msg;
	transfer = bes->cur_transfer;
	chip = bes->cur_chip;
	spi = message->spi;

	if (unlikely(!chip->clk_div))
		chip->clk_div = bes->max_freq / chip->speed_hz;

	if (message->state == ERROR_STATE) {
		message->status = -EIO;
		goto early_exit;
	}

	/* Handle end of message */
	if (message->state == DONE_STATE) {
		message->status = 0;
		goto early_exit;
	}

	/* Delay if requested at end of transfer*/
	if (message->state == RUNNING_STATE) {
		previous = list_entry(transfer->transfer_list.prev,
					struct spi_transfer,
					transfer_list);
		if (previous->delay_usecs)
			udelay(previous->delay_usecs);
	}

	bes->n_bytes = chip->n_bytes;
	bes->cs_control = chip->cs_control;

	bes->tx = (void *)transfer->tx_buf;
	bes->tx_end = bes->tx + transfer->len;
	bes->rx = transfer->rx_buf;
	bes->rx_end = bes->rx + transfer->len;
	bes->len = bes->cur_transfer->len;
	if (chip != bes->prev_chip)
		cs_change = 1;

	cr0 = chip->cr0;

	/* Handle per transfer options for bpw and speed */
	if (transfer->speed_hz) {
		speed = chip->speed_hz;

		if (transfer->speed_hz != speed) {
			speed = transfer->speed_hz;

			/* clk_div doesn't support odd number */
			clk_div = bes->max_freq / speed;
			clk_div = (clk_div + 1) & 0xfffe;

			chip->speed_hz = speed;
			chip->clk_div = clk_div;
		}
	}
	if (transfer->bits_per_word) {
		bits = transfer->bits_per_word;
		cr0 = (bits - 1)
			| (chip->type << SPI_FRF_OFFSET)
			| (spi->mode << SPI_MODE_OFFSET)
			| (chip->tmode << SPI_TMOD_OFFSET);
	}
	message->state = RUNNING_STATE;

	/*
	 * Adjust transfer mode if necessary. Requires platform dependent
	 * chipselect mechanism.
	 */
	if (bes->cs_control) {
		if (bes->rx && bes->tx)
			chip->tmode = SPI_TMOD_TR;
		else if (bes->rx)
			chip->tmode = SPI_TMOD_RO;
		else
			chip->tmode = SPI_TMOD_TO;

		cr0 &= ~SPI_TMOD_MASK;
		cr0 |= (chip->tmode << SPI_TMOD_OFFSET);
	}

	/*
	 * Reprogram registers only if
	 *	1. chip select changes
	 *	2. clk_div is changed
	 *	3. control value changes
	 */
	if (be_readl(bes, BE_SPI_CTRL0) != cr0 || cs_change || clk_div || imask) {
		spi_enable_chip(bes, 0);

		if (be_readl(bes, BE_SPI_CTRL0) != cr0)
			be_writel(bes, BE_SPI_CTRL0, cr0);

		spi_set_clk(bes, clk_div ? clk_div : chip->clk_div);
		spi_chip_sel(bes, spi, 1);

		/* Set the interrupt mask, for poll mode just disable all int */
		spi_mask_intr(bes, 0xff);
		if (imask)
			spi_umask_intr(bes, imask);
		if (txint_level)
			be_writel(bes, BE_SPI_TXFLTR, txint_level);

		spi_enable_chip(bes, 1);
		if (cs_change)
			bes->prev_chip = chip;
	}

	pr_info("pump_transfers\n");
	poll_transfer(bes);

	return;

early_exit:
	giveback(bes);
	return;
}

static int be_spi_transfer_one_message(struct spi_master *master,
		struct spi_message *msg)
{
	struct be_spi *bes = spi_master_get_devdata(master);

	bes->cur_msg = msg;
	/* Initial message state*/
	bes->cur_msg->state = START_STATE;
	bes->cur_transfer = list_entry(bes->cur_msg->transfers.next,
						struct spi_transfer,
						transfer_list);
	bes->cur_chip = spi_get_ctldata(bes->cur_msg->spi);

	/* Launch transfers */
	tasklet_schedule(&bes->pump_transfers);

	return 0;
}

/* This may be called twice for each spi dev */
static int be_spi_setup(struct spi_device *spi)
{
	struct be_spi_chip *chip_info = NULL;
	struct chip_data *chip;

	/* Only alloc on first setup */
	chip = spi_get_ctldata(spi);
	if (!chip) {
		chip = devm_kzalloc(&spi->dev, sizeof(struct chip_data),
				GFP_KERNEL);
		if (!chip)
			return -ENOMEM;
		spi_set_ctldata(spi, chip);
	}

	/*
	 * Protocol drivers may change the chip settings, so...
	 * if chip_info exists, use it
	 */
	chip_info = spi->controller_data;

	/* chip_info doesn't always exist */
	if (chip_info) {
		if (chip_info->cs_control)
			chip->cs_control = chip_info->cs_control;

		chip->poll_mode = 1;
		chip->type = chip_info->type;

		chip->rx_threshold = 0;
		chip->tx_threshold = 0;
	}

	if (spi->bits_per_word == 8) {
		chip->n_bytes = 1;
	} else if (spi->bits_per_word == 16) {
		chip->n_bytes = 2;
	}
	chip->bits_per_word = spi->bits_per_word;

	if (!spi->max_speed_hz) {
		dev_err(&spi->dev, "No max speed HZ parameter\n");
		return -EINVAL;
	}
	chip->speed_hz = spi->max_speed_hz;

	chip->tmode = 0; /* Tx & Rx */
	/* Default SPI mode is SCPOL = 0, SCPH = 0 */
	chip->cr0 = (chip->bits_per_word - 1)
			| (chip->type << SPI_FRF_OFFSET)
			| (spi->mode  << SPI_MODE_OFFSET)
			| (chip->tmode << SPI_TMOD_OFFSET);

	return 0;
}

/* Restart the controller, disable all interrupts, clean rx fifo */
static void spi_hw_init(struct be_spi *bes)
{
	/* Release SRAM */
	ctrl_release_sram(bes);
	/* Native SPI mode */
	ctrl_set_spi_mode(bes, 1);

	pr_info("---> BC_CSR=%x\n", be_ctrl_readl(bes, BE_CTRL_CSR));
	pr_info("---> BC_MAR=%x\n", be_ctrl_readl(bes, BE_CTRL_MAR));

	spi_enable_chip(bes, 0);
	spi_mask_intr(bes, 0xff);

	/*
	 * Try to detect the FIFO depth if not set by interface driver,
	 * the depth could be from 2 to 256 from HW spec
	 */
	if (!bes->fifo_len) {
		u32 fifo;
		for (fifo = 2; fifo <= 257; fifo++) {
			be_writel(bes, BE_SPI_TXFLTR, fifo);
			if (fifo != be_readl(bes, BE_SPI_TXFLTR))
				break;
		}

		bes->fifo_len = (fifo == 257) ? 0 : fifo;
		be_writel(bes, BE_SPI_TXFLTR, 0);
	}
	pr_info("bes->fifo_len=%d\n", bes->fifo_len);
	spi_enable_chip(bes, 1);
}

int be_spi_add_host(struct device *dev, struct be_spi *bes)
{
	struct spi_master *master;
	int ret;

	BUG_ON(bes == NULL);

	master = spi_alloc_master(dev, 0);
	if (!master)
		return -ENOMEM;

	bes->master = master;
	bes->type = SSI_MOTO_SPI;
	bes->prev_chip = NULL;
	snprintf(bes->name, sizeof(bes->name), "be_spi%d",
			bes->bus_num);

	master->mode_bits = SPI_CPOL | SPI_CPHA;
	master->bits_per_word_mask = SPI_BPW_MASK(8) | SPI_BPW_MASK(16);
	master->bus_num = bes->bus_num;
	master->num_chipselect = bes->num_cs;
	master->setup = be_spi_setup;
	master->transfer_one_message = be_spi_transfer_one_message;
	master->max_speed_hz = bes->max_freq;

	/* Basic HW init */
	spi_hw_init(bes);

	tasklet_init(&bes->pump_transfers, pump_transfers, (unsigned long)bes);

	spi_master_set_devdata(master, bes);
	ret = devm_spi_register_master(dev, master);
	if (ret) {
		dev_err(&master->dev, "problem registering spi master\n");
		goto err_exit;
	}

	mrst_spi_debugfs_init(bes);
	return 0;

err_exit:
	spi_enable_chip(bes, 0);
	spi_master_put(master);
	return ret;
}
EXPORT_SYMBOL_GPL(be_spi_add_host);

void be_spi_remove_host(struct be_spi *bes)
{
	if (!bes)
		return;
	mrst_spi_debugfs_remove(bes);

	spi_enable_chip(bes, 0);
	/* Disable clk */
	spi_set_clk(bes, 0);
}
EXPORT_SYMBOL_GPL(be_spi_remove_host);

int be_spi_suspend_host(struct be_spi *bes)
{
	int ret = 0;

	ret = spi_master_suspend(bes->master);
	if (ret)
		return ret;
	spi_enable_chip(bes, 0);
	spi_set_clk(bes, 0);
	return ret;
}
EXPORT_SYMBOL_GPL(be_spi_suspend_host);

int be_spi_resume_host(struct be_spi *bes)
{
	int ret;

	spi_hw_init(bes);
	ret = spi_master_resume(bes->master);
	if (ret)
		dev_err(&bes->master->dev, "fail to start queue (%d)\n", ret);
	return ret;
}
EXPORT_SYMBOL_GPL(be_spi_resume_host);

MODULE_ALIAS("platform:be_spi");
MODULE_AUTHOR("Feng Tang <feng.tang@intel.com>,\nDmitry Dunaev <dmitry.dunaev@baikalelectronics.ru>");
MODULE_DESCRIPTION("Driver for Baikal SPI boot controller core");
MODULE_LICENSE("GPL v2");
