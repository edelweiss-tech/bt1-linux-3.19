/*
 * drivers/net/phy/dw_phy.c
 *
 * Driver for Synopsis DesignWire Generic Ethernet PHY
 *
 * Author: Dmitry Dunaev
 *
 * Copyright (c) 2013 Baikal Electronics.
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the terms of  the GNU General  Public License as published by the
 * Free Software Foundation;  either version 2 of the  License, or (at your
 * option) any later version.
 *
 */
#include <linux/kernel.h>
#include <linux/string.h>
#include <linux/errno.h>
#include <linux/unistd.h>
#include <linux/interrupt.h>
#include <linux/init.h>
#include <linux/delay.h>
#include <linux/netdevice.h>
#include <linux/etherdevice.h>
#include <linux/skbuff.h>
#include <linux/spinlock.h>
#include <linux/mm.h>
#include <linux/module.h>
#include <linux/mii.h>
#include <linux/ethtool.h>
#include <linux/phy.h>

#include <asm/io.h>
#include <asm/irq.h>
#include <asm/uaccess.h>

/* 
 * At now DW Generic PHY doesn't provide any PHYS_ID.
 * So we need to set 'simulated' ID in drever
 * which will be replaced with real ID during
 * the probe routine.
 */
#define DWPHY_MASK				0xffffffff
#define DWPHY_PHYS_ID			0xdeadbeef
#define DWPHY_SIMULATE_ID		0x00000000

/* 
 * DesignWire PHY Registers.
 * Only MII generic registers is defined.
 */
/* Control Register */
#define MII_DWPHY_CONTROL		0
#define MII_DWPHY_CONTROL_INIT	0x0000
#define MII_DWPHY_CONTROL_SET	((1 << 12) | (1 << 9))
/* Status Register */
#define MII_DWPHY_STATUS		1
#define MII_DWPHY_STATUS_INIT	0x7F89 
/* Physical ID Register 1 */
#define MII_DWPHY_PHYS_ID1		2
#define MII_DWPHY_PHYS_ID1_INIT	((DWPHY_PHYS_ID >> 16) & 0xffff)
/* Physical ID Register 2 */
#define MII_DWPHY_PHYS_ID2		3
#define MII_DWPHY_PHYS_ID2_INIT	(DWPHY_PHYS_ID & 0xffff)
/* Auto-Negotiation Advertisement Register */
#define MII_DWPHY_ANA 			4
#define MII_DWPHY_ANA_INIT 		0x81E1 
/* Auto-Negotiation Link Partner Base Page Ability */
#define MII_DWPHY_ANLPBPA		5
#define MII_DWPHY_ANLPBPA_INIT	0xB381
/* Auto-Negotiation Expansion */
#define MII_DWPHY_ANE			6
#define MII_DWPHY_ANE_INIT		0x0064
/* Auto-Negotiation Next Page Transmit */
#define MII_DWPHY_ANNPT			7
#define MII_DWPHY_ANNPT_INIT	0x0000
/* Auto-Negotiation Link Partner Received Next Page */
#define MII_DWPHY_ANLPRNP		8
#define MII_DWPHY_ANLPRNP_INIT	0x0000 
/* Master Slave Control */
#define MII_DWPHY_MSCR			9
#define MII_DWPHY_MSCR_INIT		0x0300
/* Master Slave Status */
#define MII_DWPHY_MSSR 			10
#define MII_DWPHY_MSSR_INIT		0x0000 
/* Extended Status */
#define MII_DWPHY_EXT_STATUS	15
#define MII_DWPHY_EXT_INIT 		0x0000

static int dw_phy_probe(struct phy_device *phydev)
{
	/* 
	 * Read PHY status register. 
	 * The zero value means than phy doesn't exists
	 */
	int err = phy_read(phydev, MII_DWPHY_STATUS);
	if (!err)
		return -ENODEV;

	pr_debug("%s: Probed driver %s\n", __func__, phydev->drv->name);
	pr_debug("%s: Status: %d\n", __func__, err);
	/* Ugly hack: Set desired PHY_ID to device */
	// phydev->phy_id = DWPHY_PHYS_ID;
	/* Probe success */
	return 0;
}

static int dw_phy_config_init(struct phy_device *phydev)
{
	int err;

	/* Enable auto-negotiation and restart handshake */ 
	err = phy_write(phydev, MII_DWPHY_CONTROL,
			MII_DWPHY_CONTROL_SET);
	/* Fail on error */
	if (err < 0)
		return err;
	/* Init success */
	return 0;
}

/* Phy register structure */
static struct phy_driver dw_phy_driver[] = {
{
	.phy_id			= DWPHY_PHYS_ID,
	.name			= "DesignWire PHY",
	.phy_id_mask	= DWPHY_MASK,
	.features		= PHY_GBIT_FEATURES,
	.probe			= &dw_phy_probe,
	.config_init	= &dw_phy_config_init,
	.config_aneg	= &genphy_config_aneg,
	.read_status	= &genphy_read_status,
	.config_aneg	= &genphy_config_aneg,
	.driver			= { 
						.owner = THIS_MODULE,
					  },
}, {

	.phy_id			= DWPHY_SIMULATE_ID,
	.name			= "DesignWire Sim PHY",
	.phy_id_mask	= DWPHY_MASK,
	.features		= PHY_GBIT_FEATURES,
/*
	.flags			= PHY_HAS_INTERRUPT,
*/
	.probe			= &dw_phy_probe,
	.config_init	= &dw_phy_config_init,
	.config_aneg	= &genphy_config_aneg,
	.read_status	= &genphy_read_status,
	.config_aneg	= &genphy_config_aneg,
	.driver			= { 
						.owner = THIS_MODULE,
					  },
} };

static int __init dw_phy_init(void)
{
	return phy_drivers_register(dw_phy_driver, ARRAY_SIZE(dw_phy_driver));
}

static void __exit dw_phy_exit(void)
{
	phy_drivers_unregister(dw_phy_driver, ARRAY_SIZE(dw_phy_driver));
}

module_init(dw_phy_init);
module_exit(dw_phy_exit);

static struct mdio_device_id __maybe_unused dw_phy_tbl[] = {
	{ DWPHY_SIMULATE_ID, DWPHY_MASK },
	{ DWPHY_PHYS_ID, DWPHY_MASK },
	{ }
};

MODULE_DEVICE_TABLE(mdio, dw_phy_tbl);
MODULE_DESCRIPTION("DesignWire PHY driver");
MODULE_AUTHOR("Dmitry Dunaev");
MODULE_LICENSE("GPL");
