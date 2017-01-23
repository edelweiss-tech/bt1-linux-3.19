/*
 * Baikal Electronics 10Gb Ethernet PHY driver
 *
 * based on AMD 10Gb Ethernet PHY driver
 * drivers/net/phy/amd-xgbe-phy.c
 *
 * This file is available to you under your choice of the following two
 * licenses:
 *
 * License 1: GPLv2
 *
 * Copyright (c) 2015,2016 Baikal Electronics, JSC
 * Copyright (c) 2014 Advanced Micro Devices, Inc.
 *
 * This file is free software; you may copy, redistribute and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or (at
 * your option) any later version.
 *
 * This file is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 *
 * License 2: Modified BSD
 *
 * Copyright (c) 2015 Baikal Electronics, JSC
 * Copyright (c) 2014 Advanced Micro Devices, Inc.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of Advanced Micro Devices, Inc. nor the
 *       names of its contributors may be used to endorse or promote products
 *       derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL <COPYRIGHT HOLDER> BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <linux/kernel.h>
#include <linux/device.h>
#include <linux/platform_device.h>
#include <linux/string.h>
#include <linux/errno.h>
#include <linux/unistd.h>
#include <linux/slab.h>
#include <linux/interrupt.h>
#include <linux/init.h>
#include <linux/clk.h>
#include <linux/delay.h>
#include <linux/netdevice.h>
#include <linux/etherdevice.h>
#include <linux/skbuff.h>
#include <linux/mm.h>
#include <linux/module.h>
#include <linux/mii.h>
#include <linux/ethtool.h>
#include <linux/phy.h>
#include <linux/mdio.h>
#include <linux/io.h>
#include <linux/of.h>
#include <linux/of_platform.h>
#include <linux/of_device.h>
#include <linux/uaccess.h>
#include <linux/of_mdio.h>


MODULE_AUTHOR(
    "Tom Lendacky <thomas.lendacky@amd.com>\n"
    "Dmitry Dunaev <dmitry.dunaev@baikalelectronics.ru>"
);
MODULE_LICENSE("Dual BSD/GPL");
MODULE_VERSION("1.0.1");
MODULE_DESCRIPTION("Baikal Electronics 10GbE (be-xgbe) PHY driver. Based on amd-xgbe.");

#define BE_T_XGBE_PHY_ID				0x7996ced0
#define BE_T1_XGBE_PHY_ID				0x7996ced0
#define BE_XGBE_PHY_MASK				0xfffffff0

#define XGBE_PHY_MODESET_PROPERTY		"be,mode-set"
#define XGBE_PHY_CLOCK_PROPERTY			"be,external-clock"
#define XGBE_PHY_SYS_CLOCK				"xgbe_clk"

#ifndef MDIO_PMA_10GBR_PMD_CTRL
#define MDIO_PMA_10GBR_PMD_CTRL			0x0096
#endif
#ifndef MDIO_PMA_10GBR_FEC_CTRL
#define MDIO_PMA_10GBR_FEC_CTRL			0x00ab
#endif
#ifndef MDIO_AN_XNP
#define MDIO_AN_XNP						0x0016
#endif

#ifndef MDIO_AN_INTMASK
#define MDIO_AN_INTMASK					0x8001
#endif
#ifndef MDIO_AN_INT
#define MDIO_AN_INT						0x8002
#endif
#ifndef PORT_BACKPLANE
#define PORT_BACKPLANE					0x06
#endif

#ifndef VR_XS_PMA_MII_Gen5_MPLL_CTRL
#define VR_XS_PMA_MII_Gen5_MPLL_CTRL 	0x807A
#endif
#define VR_XS_PMA_MII_Gen5_MPLL_CTRL_REF_CLK_SEL_bit	(1 << 13)
#define VR_XS_PCS_DIG_CTRL1				0x8000
#define VR_XS_PCS_DIG_CTRL1_VR_RST_Bit		MDIO_CTRL1_RESET
#define SR_XC_or_PCS_MMD_Control1			MDIO_CTRL1
#define SR_XC_or_PCS_MMD_Control1_RST_Bit	MDIO_CTRL1_RESET
#define DWC_GLBL_PLL_MONITOR			0x8010
#define SDS_PCS_CLOCK_READY_mask		0x1C
#define SDS_PCS_CLOCK_READY_bit			0x10


#ifndef MDIO_CTRL1_SPEED1G
#define MDIO_CTRL1_SPEED1G		(MDIO_CTRL1_SPEED10G & ~BMCR_SPEED100)
#endif

enum be_xgbe_phy_mode {
	BE_XGBE_MODE_KR,
	BE_XGBE_MODE_KX,
};

enum be_xgbe_phy_speedset {
	BE_XGBE_PHY_SPEEDSET_1000_10000,
	BE_XGBE_PHY_SPEEDSET_2500_10000,
};

struct be_xgbe_phy_priv {
	struct platform_device *pdev;
	struct device *dev;

	/* Self phydevice and tranciever */
	struct phy_device *phydev;
	struct phy_device *xmit;

	/* Device clock */
	struct clk *sysclk;
	unsigned int ext_clk;

	/* Maintain link status for re-starting auto-negotiation */
	unsigned int link;
	enum be_xgbe_phy_mode mode;
	unsigned int speed_set;

	/* Auto-negotiation state machine support */
	struct mutex an_mutex;
	struct work_struct an_work;
	struct workqueue_struct *an_workqueue;
};

static int be_xgbe_an_enable_kr_training(struct phy_device *phydev)
{
	int ret;

	return 0;

	ret = phy_read_mmd(phydev, MDIO_MMD_PMAPMD, MDIO_PMA_10GBR_PMD_CTRL);
	if (ret < 0)
		return ret;

	ret |= 0x02;
	phy_write_mmd(phydev, MDIO_MMD_PMAPMD, MDIO_PMA_10GBR_PMD_CTRL, ret);

	return 0;
}

static int be_xgbe_phy_pcs_power_cycle(struct phy_device *phydev)
{
	int ret;

	return 0;

	ret = phy_read_mmd(phydev, MDIO_MMD_PCS, MDIO_CTRL1);
	if (ret < 0)
		return ret;

	ret |= MDIO_CTRL1_LPOWER;
	phy_write_mmd(phydev, MDIO_MMD_PCS, MDIO_CTRL1, ret);

	usleep_range(75, 100);

	ret &= ~MDIO_CTRL1_LPOWER;
	phy_write_mmd(phydev, MDIO_MMD_PCS, MDIO_CTRL1, ret);

	return 0;
}

static int be_xgbe_phy_xgmii_mode(struct phy_device *phydev)
{
	struct be_xgbe_phy_priv *priv = phydev->priv;
	int ret;

	/* Enable KR training */
	ret = be_xgbe_an_enable_kr_training(phydev);
	if (ret < 0)
		return ret;

	/* Set PCS to KR/10G speed */
	ret = phy_read_mmd(phydev, MDIO_MMD_PCS, MDIO_CTRL2);
	if (ret < 0)
		return ret;

	ret &= ~MDIO_PCS_CTRL2_TYPE;
	ret |= MDIO_PCS_CTRL2_10GBR;
	phy_write_mmd(phydev, MDIO_MMD_PCS, MDIO_CTRL2, ret);

	ret = phy_read_mmd(phydev, MDIO_MMD_PCS, MDIO_CTRL1);
	if (ret < 0)
		return ret;

	ret &= ~MDIO_CTRL1_SPEEDSEL;
	ret |= MDIO_CTRL1_SPEED10G;
	phy_write_mmd(phydev, MDIO_MMD_PCS, MDIO_CTRL1, ret);

	ret = be_xgbe_phy_pcs_power_cycle(phydev);
	if (ret < 0)
	return ret;

	priv->mode = BE_XGBE_MODE_KR;

	return 0;
}

static int __maybe_unused be_xgbe_phy_soft_reset(struct phy_device *phydev)
{
	int count, ret;

	ret = phy_read_mmd(phydev, MDIO_MMD_PCS, MDIO_CTRL1);
	if (ret < 0)
		return ret;

	ret |= MDIO_CTRL1_RESET;
	phy_write_mmd(phydev, MDIO_MMD_PCS, MDIO_CTRL1, ret);

	count = 50;
	do {
		msleep(20);
		ret = phy_read_mmd(phydev, MDIO_MMD_PCS, MDIO_CTRL1);
		if (ret < 0)
			return ret;
	} while ((ret & MDIO_CTRL1_RESET) && --count);

	if (ret & MDIO_CTRL1_RESET)
		return -ETIMEDOUT;

	return 0;
}

static int be_xgbe_phy_config_init(struct phy_device *phydev)
{
	struct be_xgbe_phy_priv *priv = phydev->priv;
	int ret = 0;


	/* Initialize supported features */
	phydev->supported = SUPPORTED_Autoneg;
	phydev->supported |= SUPPORTED_Pause | SUPPORTED_Asym_Pause;
	phydev->supported |= SUPPORTED_Backplane |
				SUPPORTED_10000baseKX4_Full;
	phydev->supported |= SUPPORTED_10000baseKR_Full |
			    SUPPORTED_10000baseR_FEC;
	switch (priv->speed_set) {
	case BE_XGBE_PHY_SPEEDSET_1000_10000:
		phydev->supported |= SUPPORTED_1000baseKX_Full;
		break;
	case BE_XGBE_PHY_SPEEDSET_2500_10000:
		phydev->supported |= SUPPORTED_2500baseX_Full;
		break;
	}

	if (priv->xmit)
		phydev->supported |= SUPPORTED_10000baseT_Full;

	phydev->advertising = phydev->supported;
	phydev->pause = 0;
	phydev->asym_pause = 0;

	if (priv->ext_clk) {
		/* Switch XGMAC PHY PLL to use extrnal ref clock from pad */
		ret = phy_read_mmd(phydev, MDIO_MMD_PMAPMD, VR_XS_PMA_MII_Gen5_MPLL_CTRL);
		ret &= ~(VR_XS_PMA_MII_Gen5_MPLL_CTRL_REF_CLK_SEL_bit);
		phy_write_mmd(phydev, MDIO_MMD_PMAPMD, VR_XS_PMA_MII_Gen5_MPLL_CTRL, ret);
		wmb();
		/* Turn off internal XGMAC PHY clock */
		clk_disable_unprepare(priv->sysclk);
	}

	/* Make vendor specific soft reset */
	ret = phy_read_mmd(phydev, MDIO_MMD_PCS, VR_XS_PCS_DIG_CTRL1);
	ret |= VR_XS_PCS_DIG_CTRL1_VR_RST_Bit;
	phy_write_mmd(phydev, MDIO_MMD_PCS, VR_XS_PCS_DIG_CTRL1, ret);
	wmb();

	/* Wait reset finish */
	do {
		usleep_range(500, 600);
		ret = phy_read_mmd(phydev, MDIO_MMD_PCS, VR_XS_PCS_DIG_CTRL1);
	} while ( (ret & VR_XS_PCS_DIG_CTRL1_VR_RST_Bit) != 0 );

	/*
	 * Wait for the RST (bit 15) of the “SR XS or PCS MMD Control1” Register is 0.
	 * This bit is self-cleared when Bits[4:2] in VR XS or PCS MMD Digital
	 * Status Register are equal to 3’b100, that is, Tx/Rx clocks are stable
	 * and in Power_Good state.
	 */
	do {
		usleep_range(500, 600);
		ret = phy_read_mmd(phydev, MDIO_MMD_PCS, SR_XC_or_PCS_MMD_Control1);
	} while ( (ret & SR_XC_or_PCS_MMD_Control1_RST_Bit) != 0 );

	/*
	 * This bit is self-cleared when Bits[4:2] in VR XS or PCS MMD Digital
	 * Status Register are equal to 3’b100, that is, Tx/Rx clocks are stable
	 * and in Power_Good state.
	 */
	do {
		usleep_range(500, 600);
		ret = phy_read_mmd(phydev, MDIO_MMD_PCS, DWC_GLBL_PLL_MONITOR);
	} while ( (ret & SDS_PCS_CLOCK_READY_mask) != SDS_PCS_CLOCK_READY_bit );

	/* Turn off and clear interrupts */
	phy_write_mmd(phydev, MDIO_MMD_AN, MDIO_AN_INTMASK, 0);
	phy_write_mmd(phydev, MDIO_MMD_AN, MDIO_AN_INT, 0);
	wmb();

	ret = be_xgbe_phy_xgmii_mode(phydev);
	if (ret < 0)
		return ret;

	return 0;
}

static int be_xgbe_phy_config_aneg(struct phy_device *phydev)
{
	struct be_xgbe_phy_priv *priv = phydev->priv;
	int reg;

	reg = phy_read_mmd(phydev, MDIO_MMD_AN, MDIO_CTRL1);
	if (reg < 0)
		return reg;
	/* Disable autonegotiation with tranceiver */
	if (priv->xmit) {
		reg &= ~MDIO_AN_CTRL1_ENABLE;
		phydev->autoneg = AUTONEG_DISABLE;
	}
	else {
		reg |= MDIO_AN_CTRL1_ENABLE;
		phydev->autoneg = AUTONEG_ENABLE;
	}

	phy_write_mmd(phydev, MDIO_MMD_AN, MDIO_CTRL1, reg);

	return 0;
}

static int be_xgbe_phy_aneg_done(struct phy_device *phydev)
{
	int reg;

	reg = phy_read_mmd(phydev, MDIO_MMD_AN, MDIO_STAT1);
	if (reg < 0)
		return reg;

	return (reg & MDIO_AN_STAT1_COMPLETE) ? 1 : 0;
}

static int be_xgbe_phy_update_link(struct phy_device *phydev)
{
	struct be_xgbe_phy_priv *priv = phydev->priv;
	int reg;

	/* Set default values */
	phydev->link = 1;

	/* Check tranceiver status */
	if (priv->xmit) {
		priv->xmit->drv->read_status(priv->xmit);
		if (!priv->xmit->link)
			phydev->link = 0;
	}

	/* Check PMA status */
	reg = phy_read_mmd(phydev, MDIO_MMD_PMAPMD, MDIO_STAT1);
	if ((reg < 0) || !(reg & MDIO_STAT1_LSTATUS))
		phydev->link = 0;
	dev_dbg(&phydev->dev, "PMA link=%d\n", (reg & MDIO_STAT1_LSTATUS)?1:0);

	/* Check PCS status */
	reg = phy_read_mmd(phydev, MDIO_MMD_PCS, MDIO_STAT1);
	if ((reg < 0) || !(reg & MDIO_STAT1_LSTATUS))
		phydev->link = 0;
	dev_dbg(&phydev->dev, "PCS link=%d\n", (reg & MDIO_STAT1_LSTATUS)?1:0);

	return 0;
}

static int be_xgbe_phy_read_status(struct phy_device *phydev)
{
	struct be_xgbe_phy_priv *priv = phydev->priv;
	int reg;

	/* Read current mode */
	reg = phy_read_mmd(phydev, MDIO_MMD_PCS, MDIO_CTRL2);
	reg &= MDIO_PCS_CTRL2_TYPE;

	/* Set speed */
	if (reg == MDIO_PCS_CTRL2_10GBR) {
		phydev->speed = SPEED_10000;
	} else {
		if (priv->speed_set ==
			BE_XGBE_PHY_SPEEDSET_1000_10000)
			phydev->speed = SPEED_1000;
		else
			phydev->speed = SPEED_2500;
	}

	/* Update link status */
	be_xgbe_phy_update_link(phydev);

	/* Set duplex */
	if (priv->xmit)
		phydev->duplex = priv->xmit->duplex;
	else
		phydev->duplex = DUPLEX_FULL;

	return 0;
}

static int be_xgbe_phy_resume(struct phy_device *phydev)
{
	int ret;

	mutex_lock(&phydev->lock);

	ret = phy_read_mmd(phydev, MDIO_MMD_PCS, MDIO_CTRL1);
	if (ret < 0)
		goto unlock;

	ret &= ~MDIO_CTRL1_LPOWER;
	phy_write_mmd(phydev, MDIO_MMD_PCS, MDIO_CTRL1, ret);

	ret = 0;

unlock:
	mutex_unlock(&phydev->lock);

	return ret;
}

static int be_xgbe_phy_suspend(struct phy_device *phydev)
{
	int ret;

	mutex_lock(&phydev->lock);

	ret = phy_read_mmd(phydev, MDIO_MMD_PCS, MDIO_CTRL1);
	if (ret < 0)
		goto unlock;

	ret |= MDIO_CTRL1_LPOWER;
	phy_write_mmd(phydev, MDIO_MMD_PCS, MDIO_CTRL1, ret);

	ret = 0;

unlock:
	mutex_unlock(&phydev->lock);

	return ret;
}

static int be_xgbe_xmit_probe(struct phy_device *phydev)
{
	struct device_node *xmit_node;
	struct be_xgbe_phy_priv *priv = phydev->priv;
	struct phy_device *xmit;
	int ret;

	/* Retrieve the xmit-handle */
	xmit_node = of_parse_phandle(phydev->dev.of_node, "phy-handle", 0);
	if (!xmit_node) {
		dev_info(&phydev->dev, "no phy-handle, work in KR/KX mode\n");
		return -ENODEV;
	}

	xmit = of_phy_find_device(xmit_node);
	if (!xmit)
		return -EINVAL;

	ret = phy_init_hw(xmit);
	if (ret < 0)
		return ret;

	xmit->speed = SPEED_10000;
	xmit->duplex = DUPLEX_FULL;

	priv->xmit = xmit;
	/* refcount is held by phy_attach_direct() on success */
	put_device(&xmit->dev);
#if 0
	/* Add sysfs link to netdevice */
	ret = sysfs_create_link(&(xmit->bus->dev.kobj),
		&(phydev->attached_dev->dev.kobj), "traceiver");
	if (ret)
		dev_warn(&xmit->dev, "sysfs link to netdevice failed\n");
#endif
	return 0;
}

static int be_xgbe_phy_probe(struct phy_device *phydev)
{
	struct be_xgbe_phy_priv *priv;
	struct platform_device *pdev;
	struct device *dev;
	unsigned int mode_set;
	int ret;

	if (!phydev->dev.of_node)
		return -EINVAL;

	pdev = of_find_device_by_node(phydev->dev.of_node);
	if (!pdev)
		return -EINVAL;
	dev = &pdev->dev;

	priv = devm_kzalloc(dev, sizeof(*priv), GFP_KERNEL);
	if (!priv) {
		ret = -ENOMEM;
		goto err_pdev;
	}

	priv->pdev = pdev;
	priv->dev = dev;
	priv->phydev = phydev;

	mode_set = 0;
	ret = of_property_read_u32(dev->of_node, XGBE_PHY_MODESET_PROPERTY,
				   &mode_set);
	if (!ret) {
		if (mode_set > MDIO_PCS_CTRL2_10GBT) {
			dev_err(dev, "invalid be,mode-set property\n");
			ret = -EINVAL;
			goto err_speed;
		}
		phy_write_mmd(phydev, MDIO_MMD_PCS, MDIO_CTRL2, mode_set);
	}

	/* XGBE internal system clock */
	priv->ext_clk = of_property_read_bool(dev->of_node,
				XGBE_PHY_CLOCK_PROPERTY);
	priv->sysclk = devm_clk_get(dev, XGBE_PHY_SYS_CLOCK);
	if (IS_ERR(priv->sysclk))
		dev_warn(dev, "xgbe_clk devm_clk_get failed\n");

	priv->link = 0;
	priv->mode = BE_XGBE_MODE_KR;

	phydev->priv = priv;
    phydev->speed = SPEED_10000;
    phydev->duplex = DUPLEX_FULL;
	phydev->state = PHY_READY;

	of_dev_put(pdev);

	ret = be_xgbe_xmit_probe(phydev);
	dev_info(dev, "xmit probe result %d\n", ret);

	return 0;

err_speed:
	devm_kfree(dev, priv);

err_pdev:
	of_dev_put(pdev);

	return ret;
}


static int be_xgbe_match_phy_device(struct phy_device *phydev)
{
	unsigned int phy_id = phydev->c45_ids.device_ids[MDIO_MMD_PCS] &
					BE_XGBE_PHY_MASK;
	return  (phy_id == BE_T_XGBE_PHY_ID) ||
			(phy_id == BE_T1_XGBE_PHY_ID);
}

static struct phy_driver be_xgbe_phy_driver[] = {
	{
		.phy_id				= BE_T_XGBE_PHY_ID,
		.phy_id_mask		= BE_XGBE_PHY_MASK,
		.name				= "BE-T XGBE PHY",
		.features			= 0,
		.config_init		= &be_xgbe_phy_config_init,
		.suspend			= &be_xgbe_phy_suspend,
		.resume				= &be_xgbe_phy_resume,
		.config_aneg		= &be_xgbe_phy_config_aneg,
		.aneg_done			= &be_xgbe_phy_aneg_done,
		.read_status		= &be_xgbe_phy_read_status,
		.probe				= &be_xgbe_phy_probe,
		.soft_reset			= &be_xgbe_phy_soft_reset,
		.match_phy_device	= &be_xgbe_match_phy_device,
		.driver			= {
			.owner = THIS_MODULE,
		},
	},
	{
		.phy_id				= BE_T1_XGBE_PHY_ID,
		.phy_id_mask		= BE_XGBE_PHY_MASK,
		.name				= "BE-T1 XGBE PHY",
		.features			= 0,
		.config_init		= &be_xgbe_phy_config_init,
		.suspend			= &be_xgbe_phy_suspend,
		.resume				= &be_xgbe_phy_resume,
		.config_aneg		= &be_xgbe_phy_config_aneg,
		.aneg_done			= &be_xgbe_phy_aneg_done,
		.read_status		= &be_xgbe_phy_read_status,
		.probe				= &be_xgbe_phy_probe,
		.soft_reset			= &be_xgbe_phy_soft_reset,
		.match_phy_device	= &be_xgbe_match_phy_device,
		.driver			= {
			.owner = THIS_MODULE,
		},
	},
};

static int __init be_xgbe_phy_init(void)
{
	return phy_drivers_register(be_xgbe_phy_driver,
				    ARRAY_SIZE(be_xgbe_phy_driver));
}

static void __exit be_xgbe_phy_exit(void)
{
	phy_drivers_unregister(be_xgbe_phy_driver,
			       ARRAY_SIZE(be_xgbe_phy_driver));
}

module_init(be_xgbe_phy_init);
module_exit(be_xgbe_phy_exit);

static struct mdio_device_id __maybe_unused be_xgbe_phy_ids[] = {
	{ BE_T_XGBE_PHY_ID, BE_XGBE_PHY_MASK },
	{ BE_T1_XGBE_PHY_ID, BE_XGBE_PHY_MASK },
	{ }
};
MODULE_DEVICE_TABLE(mdio, be_xgbe_phy_ids);
