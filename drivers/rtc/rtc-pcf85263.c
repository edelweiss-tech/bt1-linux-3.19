/*
 * drivers/rtc/rtc-pcf85263.c
 *
 * Driver for NXP PCF85263 real-time clock.
 *
 * Copyright (C) 2018 Konstantin Kirik
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * Based loosely on rtc-85063 by Søren Andersen
 */
#include <linux/module.h>
#include <linux/i2c.h>
#include <linux/slab.h>
#include <linux/rtc.h>
#include <linux/init.h>
#include <linux/err.h>
#include <linux/errno.h>
#include <linux/bcd.h>
#include <linux/of.h>
#include <linux/of_device.h>
#include <linux/regmap.h>

#define DRIVER_NAME "rtc-pcf85263"

/*
 * Date/Time registers
 */
#define DT_100THS	0x00
#define DT_SECS		0x01
#define DT_MINUTES	0x02
#define DT_HOURS	0x03
#define DT_DAYS		0x04
#define DT_WEEKDAYS	0x05
#define DT_MONTHS	0x06
#define DT_YEARS	0x07

/*
 * Alarm registers
 */
#define DT_SECOND_ALM1	0x08
#define DT_MINUTE_ALM1	0x09
#define DT_HOUR_ALM1	0x0a
#define DT_DAY_ALM1	0x0b
#define DT_MONTH_ALM1	0x0c
#define DT_MINUTE_ALM2	0x0d
#define DT_HOUR_ALM2	0x0e
#define DT_WEEKDAY_ALM2	0x0f
#define DT_ALARM_EN	0x10

/*
 * Time stamp registers
 */
#define DT_TIMESTAMP1	0x11
#define DT_TIMESTAMP2	0x17
#define DT_TIMESTAMP3	0x1d
#define DT_TS_MODE	0x23

/*
 * control registers
 */
#define CTRL_OFFSET	0x24
#define CTRL_OSCILLATOR	0x25
#define CTRL_BATTERY	0x26
#define CTRL_PIN_IO	0x27
#define CTRL_FUNCTION	0x28
#define CTRL_INTA_EN	0x29
#define CTRL_INTB_EN	0x2a
#define CTRL_FLAGS	0x2b
#define CTRL_RAMBYTE	0x2c
#define CTRL_WDOG	0x2d
#define CTRL_STOP_EN	0x2e
#define CTRL_RESETS	0x2f
#define CTRL_RAM	0x40

static struct i2c_driver pcf85263_driver;

struct pcf85263 {
	struct rtc_device	*rtc;
};

static int pcf85263_rtc_get_datetime(struct i2c_client *client, struct rtc_time *tm)
{
	//struct pcf85263 *pcf85263 = i2c_get_clientdata(client);
  unsigned char buf[8] = { 0 };
	struct i2c_msg msgs[] = {
		{/* setup read ptr */
			.addr = client->addr,
			.len = 1,
			.buf = buf
		},
		{/* read status + date */
			.addr = client->addr,
			.flags = I2C_M_RD,
			.len = 8,
			.buf = buf
		},
	};

  /* read registers */
	if ((i2c_transfer(client->adapter, msgs, 2)) != 2) {
		dev_err(&client->dev, "%s: read error\n", __func__);
		return -EIO;
	}

	tm->tm_year = bcd2bin(buf[DT_YEARS]);
	/* adjust for 1900 base of rtc_time */
	tm->tm_year += 100;

	tm->tm_wday = buf[DT_WEEKDAYS] & 7;
	buf[DT_SECS] &= 0x7F;
	tm->tm_sec = bcd2bin(buf[DT_SECS]);
	buf[DT_MINUTES] &= 0x7F;
	tm->tm_min = bcd2bin(buf[DT_MINUTES]);
	tm->tm_hour = bcd2bin(buf[DT_HOURS]);
	tm->tm_mday = bcd2bin(buf[DT_DAYS]);
	tm->tm_mon = bcd2bin(buf[DT_MONTHS]) - 1;

  /* the clock can give out invalid datetime, but we cannot return
	 * -EINVAL otherwise hwclock will refuse to set the time on bootup.
	 */
	if (rtc_valid_tm(tm) < 0)
		dev_err(&client->dev, "retrieved date/time is not valid.\n");
  
	return 0;
}

static int pcf85263_rtc_set_datetime(struct i2c_client *client, struct rtc_time *tm)
{
  int i = 0, err = 0;
	unsigned char buf[DT_YEARS + 1];
	//int len = sizeof(buf);

	buf[DT_100THS] = 0;
	buf[DT_SECS] = bin2bcd(tm->tm_sec);
	buf[DT_MINUTES] = bin2bcd(tm->tm_min);
	buf[DT_HOURS] = bin2bcd(tm->tm_hour);
	buf[DT_DAYS] = bin2bcd(tm->tm_mday);
	buf[DT_WEEKDAYS] = tm->tm_wday;
	buf[DT_MONTHS] = bin2bcd(tm->tm_mon + 1);
	buf[DT_YEARS] = bin2bcd(tm->tm_year % 100);

  for (i = 0; i < sizeof(buf); i++) {
		unsigned char data[2] = { i, buf[i] };

		err = i2c_master_send(client, data, sizeof(data));
		if (err != sizeof(data)) {
			dev_err(&client->dev, "%s: err=%d addr=%02x, data=%02x\n",
					__func__, err, data[0], data[1]);
			return -EIO;
		}
	}

	return 0;
}

static int pcf85263_rtc_read_time(struct device *dev, struct rtc_time *tm)
{
	return pcf85263_rtc_get_datetime(to_i2c_client(dev), tm);
}

static int pcf85263_rtc_set_time(struct device *dev, struct rtc_time *tm)
{
	return pcf85263_rtc_set_datetime(to_i2c_client(dev), tm);
}

static const struct rtc_class_ops rtc_ops = {
	.read_time	= pcf85263_rtc_read_time,
	.set_time	= pcf85263_rtc_set_time,
};

static int pcf85263_probe(struct i2c_client *client,
			  const struct i2c_device_id *id)
{
	struct pcf85263 *pcf85263;
  printk(KERN_INFO "PCF85263\n");
	dev_dbg(&client->dev, "%s\n", __func__);

	if (!i2c_check_functionality(client->adapter, I2C_FUNC_I2C))
		return -ENODEV;

	pcf85263 = devm_kzalloc(&client->dev, sizeof(struct pcf85263),
				GFP_KERNEL);
	if (!pcf85263)
		return -ENOMEM;

	i2c_set_clientdata(client, pcf85263);

	pcf85263->rtc = devm_rtc_device_register(&client->dev,
				pcf85263_driver.driver.name,
				&rtc_ops, THIS_MODULE);

	return PTR_ERR_OR_ZERO(pcf85263->rtc);
}

static const struct of_device_id dev_ids[] = {
	{ .compatible = "nxp,pcf85263" },
	{}
};
MODULE_DEVICE_TABLE(of, dev_ids);

static struct i2c_driver pcf85263_driver = {
	.driver	= {
		.name	= "pcf85263",
		.of_match_table = of_match_ptr(dev_ids),
	},
	.probe	= pcf85263_probe,
};

module_i2c_driver(pcf85263_driver);

MODULE_AUTHOR("Konstantin Kirik");
MODULE_DESCRIPTION("pcf85263 I2C RTC driver");
MODULE_LICENSE("GPL");
