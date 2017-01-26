/*
 * mmc_spi.c - Access SD/MMC cards through SPI master controllers
 *
 * (C) Copyright 2005, Intec Automation,
 *		Mike Lavender (mike@steroidmicros)
 * (C) Copyright 2006-2007, David Brownell
 * (C) Copyright 2007, Axis Communications,
 *		Hans-Peter Nilsson (hp@axis.com)
 * (C) Copyright 2007, ATRON electronic GmbH,
 *		Jan Nikitenko <jan.nikitenko@gmail.com>
 *
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */
#include <linux/sched.h>
#include <linux/delay.h>
#include <linux/slab.h>
#include <linux/module.h>
#include <linux/bio.h>
#include <linux/dma-mapping.h>
#include <linux/crc7.h>
#include <linux/crc-itu-t.h>
#include <linux/scatterlist.h>

#include <linux/mmc/host.h>
#include <linux/mmc/mmc.h>		/* for R1_SPI_* bit values */
#include <linux/mmc/slot-gpio.h>

#include <linux/spi/spi.h>
#include <linux/spi/mmc_spi.h>

#include <asm/unaligned.h>

#define KBLOCK_SIZE 512
#define SD_VOLTAGE_WINDOW_SD            ((uint32_t)0x80100000)
#define SD_HIGH_CAPACITY                ((uint32_t)0x40000000)
#define SLOW_HZ 100000
#define FAST_HZ 7000000

struct kspisd_data {
  uint8_t *rxbuf;
  uint8_t *txbuf;
};

#define KPRINTBUF(KBUF,KSZ) {int i = 0; for (i=0;i<KSZ; i++) { printk("%02x ", KBUF[i]); } printk("\n"); }

static int kspisd_send_cmd0(struct spi_device *spi, struct kspisd_data *d) {
  struct spi_transfer t = {
    .tx_buf = d->txbuf,
    .rx_buf = d->rxbuf,
    .len = KBLOCK_SIZE,
    .speed_hz = SLOW_HZ,
    .cs_change = 1
  };
  struct spi_message m;
  int status = 0;
  printk(KERN_INFO "Send cmd0\n");  
  memset(d->txbuf, 0xff, KBLOCK_SIZE);
  memset(d->rxbuf, 0x00, KBLOCK_SIZE);
  d->txbuf[1] = 0x40;
  d->txbuf[2] = 0;
  d->txbuf[3] = 0;
  d->txbuf[4] = 0;
  d->txbuf[5] = 0;
  d->txbuf[6] = crc7_be(0, d->txbuf+1, 5) | 1;
  spi_message_init(&m);
  spi_message_add_tail(&t, &m);
  status = spi_sync(spi, &m);
  printk(KERN_INFO "CMD0 reply:\n");
  KPRINTBUF(d->txbuf, 20);
  KPRINTBUF(d->rxbuf, 20);
  if (d->rxbuf[8] == 0x01) {
    printk(KERN_INFO "CMD0 Resp is ok\n");
  } else {
    printk(KERN_INFO "CMD0 Resp is not found\n");
    status = -ENOTCONN;
  }
  return status;
}

static int kspisd_send_cmd1(struct spi_device *spi, struct kspisd_data *d) {
  struct spi_transfer t = {
    .tx_buf = d->txbuf,
    .rx_buf = d->rxbuf,
    .len = KBLOCK_SIZE,
    .speed_hz = SLOW_HZ,
    .cs_change = 1
  };
  struct spi_message m;
  int status = 0;
  printk(KERN_INFO "Send cmd1\n");  
  memset(d->txbuf, 0xff, KBLOCK_SIZE);
  memset(d->rxbuf, 0x00, KBLOCK_SIZE);
  d->txbuf[1] = 0x40 | 1;
  d->txbuf[2] = 0;
  d->txbuf[3] = 0;
  d->txbuf[4] = 0;
  d->txbuf[5] = 0;
  d->txbuf[6] = crc7_be(0, d->txbuf+1, 5) | 1;
  spi_message_init(&m);
  spi_message_add_tail(&t, &m);
  status = spi_sync(spi, &m);
  printk(KERN_INFO "CMD1 reply:\n");
  KPRINTBUF(d->txbuf, 20);
  KPRINTBUF(d->rxbuf, 20);
  if (d->rxbuf[8] == 1) {
    status = -EAGAIN;
  }
  return status;
}

static int kspisd_send_cmd8(struct spi_device *spi, struct kspisd_data *d) {
  struct spi_transfer t = {
    .tx_buf = d->txbuf,
    .rx_buf = d->rxbuf,
    .len = KBLOCK_SIZE,
    .speed_hz = SLOW_HZ,
    .cs_change = 1
  };
  struct spi_message m;
  int status = 0;
  printk(KERN_INFO "Send cmd8\n");  
  memset(d->txbuf, 0xff, KBLOCK_SIZE);
  memset(d->rxbuf, 0x00, KBLOCK_SIZE);
  d->txbuf[1] = 0x40 | 8;
  d->txbuf[2] = 0;
  d->txbuf[3] = 0;
  d->txbuf[4] = 1;
  d->txbuf[5] = 0xaa;
  d->txbuf[6] = crc7_be(0, d->txbuf+1, 5) | 1;
  spi_message_init(&m);
  spi_message_add_tail(&t, &m);
  status = spi_sync(spi, &m);
  printk(KERN_INFO "CMD8 reply:\n");
  KPRINTBUF(d->txbuf, 20);
  KPRINTBUF(d->rxbuf, 20);
  if ((d->rxbuf[11] != 0x01) || (d->rxbuf[12] != 0xaa)) {
    status = -EPROTO;
  }
  return status;
}

//read single block
static int kspisd_send_cmd17(struct spi_device *spi, struct kspisd_data *d) {
  struct spi_transfer t = {
    .tx_buf = d->txbuf,
    .rx_buf = d->rxbuf,
    .len = KBLOCK_SIZE*2,
    .speed_hz = FAST_HZ,
    .cs_change = 1
  };
  struct spi_message m;
  int status = 0;
  printk(KERN_INFO "Send cmd17\n");  
  memset(d->txbuf, 0xff, KBLOCK_SIZE*2);
  memset(d->rxbuf, 0x00, KBLOCK_SIZE*2);
  d->txbuf[1] = 0x40 | 17;
  d->txbuf[2] = 0;
  d->txbuf[3] = 0;
  d->txbuf[4] = 0;
  d->txbuf[5] = 0;
  d->txbuf[6] = crc7_be(0, d->txbuf+1, 5) | 1;
  spi_message_init(&m);
  spi_message_add_tail(&t, &m);
  status = spi_sync(spi, &m);
  printk(KERN_INFO "CMD17 reply:\n");
  KPRINTBUF(d->txbuf, (KBLOCK_SIZE*2));
  KPRINTBUF(d->rxbuf, (KBLOCK_SIZE*2));
  if (d->rxbuf[8] != 0x01) {
    status = -EPROTO;
  }
  return status;
}

//write single block
static int kspisd_send_cmd24(struct spi_device *spi, struct kspisd_data *d) {
  struct spi_transfer t = {
    .tx_buf = d->txbuf,
    .rx_buf = d->rxbuf,
    .len = KBLOCK_SIZE*2,
    .speed_hz = FAST_HZ,
    .cs_change = 1
  };
  struct spi_message m;
  int status = 0;
  int i = 0;
  uint16_t crc;
  printk(KERN_INFO "Send cmd24\n");  
  memset(d->txbuf, 0xff, KBLOCK_SIZE*2);
  for (i=0;i<512;i++) {
    d->txbuf[11+i] = i%256;
  }
  memset(d->rxbuf, 0x00, KBLOCK_SIZE*2);
  d->txbuf[1] = 0x40 | 24;
  d->txbuf[2] = 0;
  d->txbuf[3] = 0;
  d->txbuf[4] = 0;
  d->txbuf[5] = 0;
  d->txbuf[6] = crc7_be(0, d->txbuf+1, 5) | 1;
  d->txbuf[10] = 0xfe; // data token
  crc = crc_itu_t(0, d->txbuf+11, KBLOCK_SIZE);
  d->txbuf[11+KBLOCK_SIZE] = 0xff&(crc>>8);
  d->txbuf[11+KBLOCK_SIZE+1] = 0xff&crc;
  spi_message_init(&m);
  spi_message_add_tail(&t, &m);
  status = spi_sync(spi, &m);
  printk(KERN_INFO "CMD24 reply:\n");
  KPRINTBUF(d->txbuf, (KBLOCK_SIZE*2));
  KPRINTBUF(d->rxbuf, (KBLOCK_SIZE*2));
  if (d->rxbuf[8] != 0x01) {
    status = -EPROTO;
  }
  return status;
}

static int kspisd_send_cmd55(struct spi_device *spi, struct kspisd_data *d) {
  struct spi_transfer t = {
    .tx_buf = d->txbuf,
    .rx_buf = d->rxbuf,
    .len = KBLOCK_SIZE,
    .speed_hz = SLOW_HZ,
    .cs_change = 1
  };
  struct spi_message m;
  int status = 0;
  printk(KERN_INFO "Send cmd55\n");  
  memset(d->txbuf, 0xff, KBLOCK_SIZE);
  memset(d->rxbuf, 0x00, KBLOCK_SIZE);
  d->txbuf[1] = 0x40 | 55;
  d->txbuf[2] = 0;
  d->txbuf[3] = 0;
  d->txbuf[4] = 0;
  d->txbuf[5] = 0;
  d->txbuf[6] = crc7_be(0, d->txbuf+1, 5) | 1;
  spi_message_init(&m);
  spi_message_add_tail(&t, &m);
  status = spi_sync(spi, &m);
  printk(KERN_INFO "CMD55 reply:\n");
  KPRINTBUF(d->txbuf, 20);
  KPRINTBUF(d->rxbuf, 20);
  if (d->rxbuf[8] != 0x00) {
    status = -EPROTO;
  }
  return status;
}

static int kspisd_send_cmd58(struct spi_device *spi, struct kspisd_data *d) {
  struct spi_transfer t = {
    .tx_buf = d->txbuf,
    .rx_buf = d->rxbuf,
    .len = KBLOCK_SIZE,
    .speed_hz = SLOW_HZ,
    .cs_change = 1
  };
  struct spi_message m;
  int status = 0;
  printk(KERN_INFO "Send cmd58\n");  
  memset(d->txbuf, 0xff, KBLOCK_SIZE);
  memset(d->rxbuf, 0x00, KBLOCK_SIZE);
  d->txbuf[1] = 0x40 | 58;
  d->txbuf[2] = 0;
  d->txbuf[3] = 0;
  d->txbuf[4] = 0;
  d->txbuf[5] = 0;
  d->txbuf[6] = crc7_be(0, d->txbuf+1, 5) | 1;
  spi_message_init(&m);
  spi_message_add_tail(&t, &m);
  status = spi_sync(spi, &m);
  printk(KERN_INFO "CMD58 reply:\n");
  KPRINTBUF(d->txbuf, 20);
  KPRINTBUF(d->rxbuf, 20);
  if (d->rxbuf[8] != 0x01) {
    status = -EPROTO;
  }
  return status;
}

static int kspisd_send_acmd41(struct spi_device *spi, struct kspisd_data *d) {
  struct spi_transfer t = {
    .tx_buf = d->txbuf,
    .rx_buf = d->rxbuf,
    .len = KBLOCK_SIZE,
    .speed_hz = SLOW_HZ,
    .cs_change = 1
  };
  struct spi_message m;
  int status = 0;
  uint32_t arg = SD_VOLTAGE_WINDOW_SD | SD_HIGH_CAPACITY;
  printk(KERN_INFO "Send cmd41\n");  
  memset(d->txbuf, 0xff, KBLOCK_SIZE);
  memset(d->rxbuf, 0x00, KBLOCK_SIZE);
  d->txbuf[1] = 0x40 | 41;
  d->txbuf[2] = 0xc0;
  d->txbuf[3] = 0x10;
  d->txbuf[4] = 0x00;
  d->txbuf[5] = 0x00;
  d->txbuf[6] = crc7_be(0, d->txbuf+1, 5) | 1;
  spi_message_init(&m);
  spi_message_add_tail(&t, &m);
  status = spi_sync(spi, &m);
  printk(KERN_INFO "ACMD41reply:\n");
  KPRINTBUF(d->txbuf, 20);
  KPRINTBUF(d->rxbuf, 20);
  if (d->rxbuf[8] != 0x00) {
    status = -EPROTO;
  }
  
  return status;
}

static int kspisd_probe(struct spi_device *spi)
{
//	struct mmc_host		*mmc;
//	struct mmc_spi_host	*host;
  struct kspisd_data *kspisd = NULL;
	int	status;
  int i = 0;
	//bool has_ro = false;

  if (spi->master->flags & SPI_MASTER_HALF_DUPLEX)
		return -EINVAL;

	/* MMC and SD specs only seem to care that sampling is on the
	 * rising edge ... meaning SPI modes 0 or 3.  So either SPI mode
	 * should be legit.  We'll use mode 0 since the steady state is 0,
	 * which is appropriate for hotplugging, unless the platform data
	 * specify mode 3 (if hardware is not compatible to mode 0).
	 */
	if (spi->mode != SPI_MODE_3)
		spi->mode = SPI_MODE_0;
	spi->bits_per_word = 8;
  printk(KERN_INFO "needs SPI mode %02x, %d KHz\n",
         spi->mode, spi->max_speed_hz / 1000);
    

	status = spi_setup(spi);
	if (status < 0) {
		return status;
	}

  /* We need a supply of ones to transmit.  This is the only time
	 * the CPU touches these, so cache coherency isn't a concern.
	 *
	 * NOTE if many systems use more than one MMC-over-SPI connector
	 * it'd save some memory to share this.  That's evidently rare.
	 */
  kspisd = kmalloc(sizeof(struct kspisd_data), GFP_KERNEL);
	status = -ENOMEM;
  kspisd->rxbuf = kmalloc(KBLOCK_SIZE*2, GFP_KERNEL);
  if (!kspisd->rxbuf) {
    kfree(kspisd->rxbuf);
    return status;
  }
    
  kspisd->txbuf = kmalloc(KBLOCK_SIZE*2, GFP_KERNEL);
	if (!kspisd->txbuf) {
    kfree(kspisd->txbuf);
    return status;
  }
  printk(KERN_INFO "Allocated spi/sd buffers\n");
	dev_set_drvdata(&spi->dev, kspisd);

  memset(kspisd->txbuf, 0xff, KBLOCK_SIZE);
  struct spi_transfer t = {
    .tx_buf = kspisd->txbuf,
    .len = KBLOCK_SIZE
  };
  struct spi_message m;
  spi_message_init(&m);
  spi_message_add_tail(&t, &m);
  status = spi_sync(spi, &m);
  printk(KERN_INFO "Poweron seq status %i\n", status);
  status = kspisd_send_cmd0(spi, kspisd);
  if (status != 0) {
    return status;
  }
  /* for (i=0;i<100;i++) { */
  /*   status = kspisd_send_cmd1(spi, kspisd); */
  /*   if (status != -EAGAIN) { */
  /*     break; */
  /*   } */
  /*   msleep(10); */
  /* } */

  status = kspisd_send_cmd8(spi, kspisd);
  if (status != 0) {
    return status;
  }
  status = kspisd_send_cmd55(spi, kspisd);
  /* if (status != 0) { */
  /*   return status; */
  /* } */

  bool validvoltage = false;
  int voltage_trial_count = 0x10;
  while ((!validvoltage) && (voltage_trial_count > 0)) {
    status = kspisd_send_cmd55(spi, kspisd);
    /* if (status != 0) { */
    /*   return status; */
    /* } */
    status = kspisd_send_acmd41(spi, kspisd);
    /* if (status != 0) { */
    /*   return status; */
    /* } */
    voltage_trial_count --;
  }

  status = kspisd_send_cmd58(spi, kspisd);

  kspisd_send_cmd24(spi, kspisd);
  kspisd_send_cmd17(spi, kspisd);
  /* for (i=0;i<20;i++) { */
  /*   status = kspisd_send_cmd1(spi, kspisd); */
  /*   if (status == 0) { */
  /*     break; */
  /*   } */
  /*   msleep(1); */
  /* } */
  
  return 0;
}

static int kspisd_remove(struct spi_device *spi) {
  struct kspisd_data *kspisd = dev_get_drvdata(&spi->dev);
  kfree(kspisd->txbuf);
  kfree(kspisd->rxbuf);
  dev_set_drvdata(&spi->dev, NULL);
  kfree(kspisd);
  return 0;
}

static struct of_device_id kspisd_of_match_table[] = {
	{ .compatible = "kspisd-slot", },
	{},
};

static struct spi_driver kspisd_driver = {
	.driver = {
		.name =		"kspisd",
		.owner =	THIS_MODULE,
		.of_match_table = kspisd_of_match_table,
	},
	.probe =	kspisd_probe,
	.remove =	kspisd_remove,
};

module_spi_driver(kspisd_driver);

MODULE_AUTHOR("Konstantin Kirik");
MODULE_DESCRIPTION("SPI SD host driver");
MODULE_LICENSE("GPL");
MODULE_ALIAS("spi:kspisd");
