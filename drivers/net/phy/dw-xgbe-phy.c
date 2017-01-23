/*
 * DesignWire 10Gb Ethernet PHY driver
 *
 * This file is available to you under your choice of the following two
 * licenses:
 *
 * License 1: GPLv2
 *
 * Copyright (c) 2015 Baikal Electronics, JSC
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


MODULE_AUTHOR(
    "Tom Lendacky <thomas.lendacky@amd.com>\n"
    "Dmitry Dunaev <dmitry.dunaev@baikalelectronics.ru>"
);
MODULE_LICENSE("Dual BSD/GPL");
MODULE_VERSION("1.0.1");
MODULE_DESCRIPTION("DesignWire 10GbE (dw-xgbe) PHY driver. Based on amd-xgbe");

#define XGBE_PHY_ID	0x7996ced0
#define XGBE_PHY_MASK	0xfffffff0

#define XGBE_PHY_SPEEDSET_PROPERTY	"snps,speed-set"

#define XGBE_AN_INT_CMPLT		0x01
#define XGBE_AN_INC_LINK		0x02
#define XGBE_AN_PG_RCV			0x04

#define XNP_MCF_NULL_MESSAGE		0x001
#define XNP_ACK_PROCESSED		(1 << 12)
#define XNP_MP_FORMATTED		(1 << 13)
#define XNP_NP_EXCHANGE			(1 << 15)

#define XGBE_PHY_RATECHANGE_COUNT	500

#ifndef MDIO_PMA_10GBR_PMD_CTRL
#define MDIO_PMA_10GBR_PMD_CTRL		0x0096
#endif
#ifndef MDIO_PMA_10GBR_FEC_CTRL
#define MDIO_PMA_10GBR_FEC_CTRL		0x00ab
#endif
#ifndef MDIO_AN_XNP
#define MDIO_AN_XNP			0x0016
#endif

#ifndef MDIO_AN_INTMASK
#define MDIO_AN_INTMASK			0x8001
#endif
#ifndef MDIO_AN_INT
#define MDIO_AN_INT			0x8002
#endif

#ifndef MDIO_CTRL1_SPEED1G
#define MDIO_CTRL1_SPEED1G		(MDIO_CTRL1_SPEED10G & ~BMCR_SPEED100)
#endif

/* SerDes integration register offsets */
#define SIR0_KR_RT_1			0x002c
#define SIR0_STATUS			0x0040
#define SIR1_SPEED			0x0000

/* SerDes integration register entry bit positions and sizes */
#define SIR0_KR_RT_1_RESET_INDEX	11
#define SIR0_KR_RT_1_RESET_WIDTH	1
#define SIR0_STATUS_RX_READY_INDEX	0
#define SIR0_STATUS_RX_READY_WIDTH	1
#define SIR0_STATUS_TX_READY_INDEX	8
#define SIR0_STATUS_TX_READY_WIDTH	1
#define SIR1_SPEED_DATARATE_INDEX	4
#define SIR1_SPEED_DATARATE_WIDTH	2
#define SIR1_SPEED_PI_SPD_SEL_INDEX	12
#define SIR1_SPEED_PI_SPD_SEL_WIDTH	4
#define SIR1_SPEED_PLLSEL_INDEX		3
#define SIR1_SPEED_PLLSEL_WIDTH		1
#define SIR1_SPEED_RATECHANGE_INDEX	6
#define SIR1_SPEED_RATECHANGE_WIDTH	1
#define SIR1_SPEED_TXAMP_INDEX		8
#define SIR1_SPEED_TXAMP_WIDTH		4
#define SIR1_SPEED_WORDMODE_INDEX	0
#define SIR1_SPEED_WORDMODE_WIDTH	3

#define SPEED_10000_CDR			0x7
#define SPEED_10000_PLL			0x1
#define SPEED_10000_RATE		0x0
#define SPEED_10000_TXAMP		0xa
#define SPEED_10000_WORD		0x7

#define SPEED_2500_CDR			0x2
#define SPEED_2500_PLL			0x0
#define SPEED_2500_RATE			0x1
#define SPEED_2500_TXAMP		0xf
#define SPEED_2500_WORD			0x1

#define SPEED_1000_CDR			0x2
#define SPEED_1000_PLL			0x0
#define SPEED_1000_RATE			0x3
#define SPEED_1000_TXAMP		0xf
#define SPEED_1000_WORD			0x1


/* SerDes RxTx register offsets */
#define RXTX_REG20			0x0050
#define RXTX_REG114			0x01c8

/* SerDes RxTx register entry bit positions and sizes */
#define RXTX_REG20_BLWC_ENA_INDEX	2
#define RXTX_REG20_BLWC_ENA_WIDTH	1
#define RXTX_REG114_PQ_REG_INDEX	9
#define RXTX_REG114_PQ_REG_WIDTH	7

#define RXTX_10000_BLWC			0
#define RXTX_10000_PQ			0x1e

#define RXTX_2500_BLWC			1
#define RXTX_2500_PQ			0xa

#define RXTX_1000_BLWC			1
#define RXTX_1000_PQ			0xa

enum dw_xgbe_phy_an {
	DW_XGBE_AN_READY = 0,
	DW_XGBE_AN_START,
	DW_XGBE_AN_EVENT,
	DW_XGBE_AN_PAGE_RECEIVED,
	DW_XGBE_AN_INCOMPAT_LINK,
	DW_XGBE_AN_COMPLETE,
	DW_XGBE_AN_NO_LINK,
	DW_XGBE_AN_EXIT,
	DW_XGBE_AN_ERROR,
};

enum dw_xgbe_phy_rx {
	DW_XGBE_RX_READY = 0,
	DW_XGBE_RX_BPA,
	DW_XGBE_RX_XNP,
	DW_XGBE_RX_COMPLETE,
};

enum dw_xgbe_phy_mode {
	DW_XGBE_MODE_KR,
	DW_XGBE_MODE_KX,
};

enum dw_xgbe_phy_speedset {
	DW_XGBE_PHY_SPEEDSET_1000_10000,
	DW_XGBE_PHY_SPEEDSET_2500_10000,
};

struct dw_xgbe_phy_priv {
	struct platform_device *pdev;
	struct device *dev;

	struct phy_device *phydev;

	/* SerDes related mmio resources */
	struct resource *rxtx_res;
	struct resource *sir0_res;
	struct resource *sir1_res;

	/* SerDes related mmio registers */
	void __iomem *rxtx_regs;	/* SerDes Rx/Tx CSRs */
	void __iomem *sir0_regs;	/* SerDes integration registers (1/2) */
	void __iomem *sir1_regs;	/* SerDes integration registers (2/2) */

	/* Maintain link status for re-starting auto-negotiation */
	unsigned int link;
	enum dw_xgbe_phy_mode mode;
	unsigned int speed_set;

	/* Auto-negotiation state machine support */
	struct mutex an_mutex;
	enum dw_xgbe_phy_an an_result;
	enum dw_xgbe_phy_an an_state;
	enum dw_xgbe_phy_rx kr_state;
	enum dw_xgbe_phy_rx kx_state;
	struct work_struct an_work;
	struct workqueue_struct *an_workqueue;
};

static int dw_xgbe_an_enable_kr_training(struct phy_device *phydev)
{
	int ret;

	ret = phy_read_mmd(phydev, MDIO_MMD_PMAPMD, MDIO_PMA_10GBR_PMD_CTRL);
	if (ret < 0)
		return ret;

	ret |= 0x02;
	phy_write_mmd(phydev, MDIO_MMD_PMAPMD, MDIO_PMA_10GBR_PMD_CTRL, ret);

	return 0;
}

static int dw_xgbe_an_disable_kr_training(struct phy_device *phydev)
{
	int ret;

	ret = phy_read_mmd(phydev, MDIO_MMD_PMAPMD, MDIO_PMA_10GBR_PMD_CTRL);
	if (ret < 0)
		return ret;

	ret &= ~0x02;
	phy_write_mmd(phydev, MDIO_MMD_PMAPMD, MDIO_PMA_10GBR_PMD_CTRL, ret);

	return 0;
}

static int dw_xgbe_phy_pcs_power_cycle(struct phy_device *phydev)
{
	int ret;

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

static int dw_xgbe_phy_xgmii_mode(struct phy_device *phydev)
{
	struct dw_xgbe_phy_priv *priv = phydev->priv;
	int ret;

	/* Enable KR training */
	ret = dw_xgbe_an_enable_kr_training(phydev);
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

	ret = dw_xgbe_phy_pcs_power_cycle(phydev);
	if (ret < 0)
		return ret;

	priv->mode = DW_XGBE_MODE_KR;

	return 0;
}

static int dw_xgbe_phy_gmii_2500_mode(struct phy_device *phydev)
{
	struct dw_xgbe_phy_priv *priv = phydev->priv;
	int ret;

	/* Disable KR training */
	ret = dw_xgbe_an_disable_kr_training(phydev);
	if (ret < 0)
		return ret;

	/* Set PCS to KX/1G speed */
	ret = phy_read_mmd(phydev, MDIO_MMD_PCS, MDIO_CTRL2);
	if (ret < 0)
		return ret;

	ret &= ~MDIO_PCS_CTRL2_TYPE;
	ret |= MDIO_PCS_CTRL2_10GBX;
	phy_write_mmd(phydev, MDIO_MMD_PCS, MDIO_CTRL2, ret);

	ret = phy_read_mmd(phydev, MDIO_MMD_PCS, MDIO_CTRL1);
	if (ret < 0)
		return ret;

	ret &= ~MDIO_CTRL1_SPEEDSEL;
	ret |= MDIO_CTRL1_SPEED1G;
	phy_write_mmd(phydev, MDIO_MMD_PCS, MDIO_CTRL1, ret);

	ret = dw_xgbe_phy_pcs_power_cycle(phydev);
	if (ret < 0)
		return ret;

	priv->mode = DW_XGBE_MODE_KX;

	return 0;
}

static int dw_xgbe_phy_gmii_mode(struct phy_device *phydev)
{
	struct dw_xgbe_phy_priv *priv = phydev->priv;
	int ret;

	/* Disable KR training */
	ret = dw_xgbe_an_disable_kr_training(phydev);
	if (ret < 0)
		return ret;

	/* Set PCS to KX/1G speed */
	ret = phy_read_mmd(phydev, MDIO_MMD_PCS, MDIO_CTRL2);
	if (ret < 0)
		return ret;

	ret &= ~MDIO_PCS_CTRL2_TYPE;
	ret |= MDIO_PCS_CTRL2_10GBX;
	phy_write_mmd(phydev, MDIO_MMD_PCS, MDIO_CTRL2, ret);

	ret = phy_read_mmd(phydev, MDIO_MMD_PCS, MDIO_CTRL1);
	if (ret < 0)
		return ret;

	ret &= ~MDIO_CTRL1_SPEEDSEL;
	ret |= MDIO_CTRL1_SPEED1G;
	phy_write_mmd(phydev, MDIO_MMD_PCS, MDIO_CTRL1, ret);

	ret = dw_xgbe_phy_pcs_power_cycle(phydev);
	if (ret < 0)
		return ret;

	priv->mode = DW_XGBE_MODE_KX;

	return 0;
}

static int dw_xgbe_phy_switch_mode(struct phy_device *phydev)
{
	struct dw_xgbe_phy_priv *priv = phydev->priv;
	int ret;

	/* If we are in KR switch to KX, and vice-versa */
	if (priv->mode == DW_XGBE_MODE_KR) {
		if (priv->speed_set == DW_XGBE_PHY_SPEEDSET_1000_10000)
			ret = dw_xgbe_phy_gmii_mode(phydev);
		else
			ret = dw_xgbe_phy_gmii_2500_mode(phydev);
	} else {
		ret = dw_xgbe_phy_xgmii_mode(phydev);
	}

	return ret;
}

static enum dw_xgbe_phy_an dw_xgbe_an_switch_mode(struct phy_device *phydev)
{
	int ret;

	ret = dw_xgbe_phy_switch_mode(phydev);
	if (ret < 0)
		return DW_XGBE_AN_ERROR;

	return DW_XGBE_AN_START;
}

static enum dw_xgbe_phy_an dw_xgbe_an_tx_training(struct phy_device *phydev,
						    enum dw_xgbe_phy_rx *state)
{
	struct dw_xgbe_phy_priv *priv = phydev->priv;
	int ad_reg, lp_reg, ret;

	*state = DW_XGBE_RX_COMPLETE;

	/* If we're in KX mode then we're done */
	if (priv->mode == DW_XGBE_MODE_KX)
		return DW_XGBE_AN_EVENT;

	/* Enable/Disable FEC */
	ad_reg = phy_read_mmd(phydev, MDIO_MMD_AN, MDIO_AN_ADVERTISE + 2);
	if (ad_reg < 0)
		return DW_XGBE_AN_ERROR;

	lp_reg = phy_read_mmd(phydev, MDIO_MMD_AN, MDIO_AN_LPA + 2);
	if (lp_reg < 0)
		return DW_XGBE_AN_ERROR;

	ret = phy_read_mmd(phydev, MDIO_MMD_PMAPMD, MDIO_PMA_10GBR_FEC_CTRL);
	if (ret < 0)
		return DW_XGBE_AN_ERROR;

	if ((ad_reg & 0xc000) && (lp_reg & 0xc000))
		ret |= 0x01;
	else
		ret &= ~0x01;

	phy_write_mmd(phydev, MDIO_MMD_PMAPMD, MDIO_PMA_10GBR_FEC_CTRL, ret);

	/* Start KR training */
	ret = phy_read_mmd(phydev, MDIO_MMD_PMAPMD, MDIO_PMA_10GBR_PMD_CTRL);
	if (ret < 0)
		return DW_XGBE_AN_ERROR;

	ret |= 0x01;
	phy_write_mmd(phydev, MDIO_MMD_PMAPMD, MDIO_PMA_10GBR_PMD_CTRL, ret);

	return DW_XGBE_AN_EVENT;
}

static enum dw_xgbe_phy_an dw_xgbe_an_tx_xnp(struct phy_device *phydev,
					       enum dw_xgbe_phy_rx *state)
{
	u16 msg;

	*state = DW_XGBE_RX_XNP;

	msg = XNP_MCF_NULL_MESSAGE;
	msg |= XNP_MP_FORMATTED;

	phy_write_mmd(phydev, MDIO_MMD_AN, MDIO_AN_XNP + 2, 0);
	phy_write_mmd(phydev, MDIO_MMD_AN, MDIO_AN_XNP + 1, 0);
	phy_write_mmd(phydev, MDIO_MMD_AN, MDIO_AN_XNP, msg);

	return DW_XGBE_AN_EVENT;
}

static enum dw_xgbe_phy_an dw_xgbe_an_rx_bpa(struct phy_device *phydev,
					       enum dw_xgbe_phy_rx *state)
{
	struct dw_xgbe_phy_priv *priv = phydev->priv;
	unsigned int link_support;
	int ret, ad_reg, lp_reg;

	/* Read Base Ability register 2 first */
	ret = phy_read_mmd(phydev, MDIO_MMD_AN, MDIO_AN_LPA + 1);
	if (ret < 0)
		return DW_XGBE_AN_ERROR;

	/* Check for a supported mode, otherwise restart in a different one */
	link_support = (priv->mode == DW_XGBE_MODE_KR) ? 0x80 : 0x20;
	if (!(ret & link_support))
		return dw_xgbe_an_switch_mode(phydev);

	/* Check Extended Next Page support */
	ad_reg = phy_read_mmd(phydev, MDIO_MMD_AN, MDIO_AN_ADVERTISE);
	if (ad_reg < 0)
		return DW_XGBE_AN_ERROR;

	lp_reg = phy_read_mmd(phydev, MDIO_MMD_AN, MDIO_AN_LPA);
	if (lp_reg < 0)
		return DW_XGBE_AN_ERROR;

	return ((ad_reg & XNP_NP_EXCHANGE) || (lp_reg & XNP_NP_EXCHANGE)) ?
	       dw_xgbe_an_tx_xnp(phydev, state) :
	       dw_xgbe_an_tx_training(phydev, state);
}

static enum dw_xgbe_phy_an dw_xgbe_an_rx_xnp(struct phy_device *phydev,
					       enum dw_xgbe_phy_rx *state)
{
	int ad_reg, lp_reg;

	/* Check Extended Next Page support */
	ad_reg = phy_read_mmd(phydev, MDIO_MMD_AN, MDIO_AN_ADVERTISE);
	if (ad_reg < 0)
		return DW_XGBE_AN_ERROR;

	lp_reg = phy_read_mmd(phydev, MDIO_MMD_AN, MDIO_AN_LPA);
	if (lp_reg < 0)
		return DW_XGBE_AN_ERROR;

	return ((ad_reg & XNP_NP_EXCHANGE) || (lp_reg & XNP_NP_EXCHANGE)) ?
	       dw_xgbe_an_tx_xnp(phydev, state) :
	       dw_xgbe_an_tx_training(phydev, state);
}

static enum dw_xgbe_phy_an dw_xgbe_an_start(struct phy_device *phydev)
{
	struct dw_xgbe_phy_priv *priv = phydev->priv;
	int ret;

	/* Be sure we aren't looping trying to negotiate */
	if (priv->mode == DW_XGBE_MODE_KR) {
		if (priv->kr_state != DW_XGBE_RX_READY)
			return DW_XGBE_AN_NO_LINK;
		priv->kr_state = DW_XGBE_RX_BPA;
	} else {
		if (priv->kx_state != DW_XGBE_RX_READY)
			return DW_XGBE_AN_NO_LINK;
		priv->kx_state = DW_XGBE_RX_BPA;
	}

	/* Set up Advertisement register 3 first */
	ret = phy_read_mmd(phydev, MDIO_MMD_AN, MDIO_AN_ADVERTISE + 2);
	if (ret < 0)
		return DW_XGBE_AN_ERROR;

	if (phydev->supported & SUPPORTED_10000baseR_FEC)
		ret |= 0xc000;
	else
		ret &= ~0xc000;

	phy_write_mmd(phydev, MDIO_MMD_AN, MDIO_AN_ADVERTISE + 2, ret);

	/* Set up Advertisement register 2 next */
	ret = phy_read_mmd(phydev, MDIO_MMD_AN, MDIO_AN_ADVERTISE + 1);
	if (ret < 0)
		return DW_XGBE_AN_ERROR;

	if (phydev->supported & SUPPORTED_10000baseKR_Full)
		ret |= 0x80;
	else
		ret &= ~0x80;

	if ((phydev->supported & SUPPORTED_1000baseKX_Full) ||
	    (phydev->supported & SUPPORTED_2500baseX_Full))
		ret |= 0x20;
	else
		ret &= ~0x20;

	phy_write_mmd(phydev, MDIO_MMD_AN, MDIO_AN_ADVERTISE + 1, ret);

	/* Set up Advertisement register 1 last */
	ret = phy_read_mmd(phydev, MDIO_MMD_AN, MDIO_AN_ADVERTISE);
	if (ret < 0)
		return DW_XGBE_AN_ERROR;

	if (phydev->supported & SUPPORTED_Pause)
		ret |= 0x400;
	else
		ret &= ~0x400;

	if (phydev->supported & SUPPORTED_Asym_Pause)
		ret |= 0x800;
	else
		ret &= ~0x800;

	/* We don't intend to perform XNP */
	ret &= ~XNP_NP_EXCHANGE;

	phy_write_mmd(phydev, MDIO_MMD_AN, MDIO_AN_ADVERTISE, ret);

	/* Enable and start auto-negotiation */
	phy_write_mmd(phydev, MDIO_MMD_AN, MDIO_AN_INT, 0);

	ret = phy_read_mmd(phydev, MDIO_MMD_AN, MDIO_CTRL1);
	if (ret < 0)
		return DW_XGBE_AN_ERROR;

	ret |= MDIO_AN_CTRL1_ENABLE;
	ret |= MDIO_AN_CTRL1_RESTART;
	phy_write_mmd(phydev, MDIO_MMD_AN, MDIO_CTRL1, ret);

	return DW_XGBE_AN_EVENT;
}

static enum dw_xgbe_phy_an dw_xgbe_an_event(struct phy_device *phydev)
{
	enum dw_xgbe_phy_an new_state;
	int ret;

	ret = phy_read_mmd(phydev, MDIO_MMD_AN, MDIO_AN_INT);
	if (ret < 0)
		return DW_XGBE_AN_ERROR;

	new_state = DW_XGBE_AN_EVENT;
	if (ret & XGBE_AN_PG_RCV)
		new_state = DW_XGBE_AN_PAGE_RECEIVED;
	else if (ret & XGBE_AN_INC_LINK)
		new_state = DW_XGBE_AN_INCOMPAT_LINK;
	else if (ret & XGBE_AN_INT_CMPLT)
		new_state = DW_XGBE_AN_COMPLETE;

	if (new_state != DW_XGBE_AN_EVENT)
		phy_write_mmd(phydev, MDIO_MMD_AN, MDIO_AN_INT, 0);

	return new_state;
}

static enum dw_xgbe_phy_an dw_xgbe_an_page_received(struct phy_device *phydev)
{
	struct dw_xgbe_phy_priv *priv = phydev->priv;
	enum dw_xgbe_phy_rx *state;
	int ret;

	state = (priv->mode == DW_XGBE_MODE_KR) ? &priv->kr_state
						 : &priv->kx_state;

	switch (*state) {
	case DW_XGBE_RX_BPA:
		ret = dw_xgbe_an_rx_bpa(phydev, state);
		break;

	case DW_XGBE_RX_XNP:
		ret = dw_xgbe_an_rx_xnp(phydev, state);
		break;

	default:
		ret = DW_XGBE_AN_ERROR;
	}

	return ret;
}

static enum dw_xgbe_phy_an dw_xgbe_an_incompat_link(struct phy_device *phydev)
{
	return dw_xgbe_an_switch_mode(phydev);
}

static void dw_xgbe_an_state_machine(struct work_struct *work)
{
	struct dw_xgbe_phy_priv *priv = container_of(work,
						      struct dw_xgbe_phy_priv,
						      an_work);
	struct phy_device *phydev = priv->phydev;
	enum dw_xgbe_phy_an cur_state;
	int sleep;
	unsigned int an_supported = 0;

	while (1) {
		mutex_lock(&priv->an_mutex);

		cur_state = priv->an_state;

		switch (priv->an_state) {
		case DW_XGBE_AN_START:
			priv->an_state = dw_xgbe_an_start(phydev);
			an_supported = 0;
			break;

		case DW_XGBE_AN_EVENT:
			priv->an_state = dw_xgbe_an_event(phydev);
			break;

		case DW_XGBE_AN_PAGE_RECEIVED:
			priv->an_state = dw_xgbe_an_page_received(phydev);
			an_supported++;
			break;

		case DW_XGBE_AN_INCOMPAT_LINK:
			priv->an_state = dw_xgbe_an_incompat_link(phydev);
			break;

		case DW_XGBE_AN_COMPLETE:
			netdev_info(phydev->attached_dev, "%s successful\n",
				    an_supported ? "Auto negotiation"
						 : "Parallel detection");
			/* fall through */

		case DW_XGBE_AN_NO_LINK:
		case DW_XGBE_AN_EXIT:
			goto exit_unlock;

		default:
			priv->an_state = DW_XGBE_AN_ERROR;
		}

		if (priv->an_state == DW_XGBE_AN_ERROR) {
			netdev_err(phydev->attached_dev,
				   "error during auto-negotiation, state=%u\n",
				   cur_state);
			goto exit_unlock;
		}

		sleep = (priv->an_state == DW_XGBE_AN_EVENT) ? 1 : 0;

		mutex_unlock(&priv->an_mutex);

		if (sleep)
			usleep_range(20, 50);
	}

exit_unlock:
	priv->an_result = priv->an_state;
	priv->an_state = DW_XGBE_AN_READY;

	mutex_unlock(&priv->an_mutex);
}

static int dw_xgbe_phy_soft_reset(struct phy_device *phydev)
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

static int dw_xgbe_phy_config_init(struct phy_device *phydev)
{
	struct dw_xgbe_phy_priv *priv = phydev->priv;

	/* Initialize supported features */
	phydev->supported = SUPPORTED_Autoneg;
	phydev->supported |= SUPPORTED_Pause | SUPPORTED_Asym_Pause;
	phydev->supported |= SUPPORTED_Backplane;
	phydev->supported |= SUPPORTED_10000baseKR_Full |
			     SUPPORTED_10000baseR_FEC;
	switch (priv->speed_set) {
	case DW_XGBE_PHY_SPEEDSET_1000_10000:
		phydev->supported |= SUPPORTED_1000baseKX_Full;
		break;
	case DW_XGBE_PHY_SPEEDSET_2500_10000:
		phydev->supported |= SUPPORTED_2500baseX_Full;
		break;
	}
	phydev->advertising = phydev->supported;

	/* Turn off and clear interrupts */
	phy_write_mmd(phydev, MDIO_MMD_AN, MDIO_AN_INTMASK, 0);
	phy_write_mmd(phydev, MDIO_MMD_AN, MDIO_AN_INT, 0);

	return 0;
}

static int dw_xgbe_phy_setup_forced(struct phy_device *phydev)
{
	int ret;

	/* Disable auto-negotiation */
	ret = phy_read_mmd(phydev, MDIO_MMD_AN, MDIO_CTRL1);
	if (ret < 0)
		return ret;

	ret &= ~MDIO_AN_CTRL1_ENABLE;
	phy_write_mmd(phydev, MDIO_MMD_AN, MDIO_CTRL1, ret);

	/* Validate/Set specified speed */
	switch (phydev->speed) {
	case SPEED_10000:
		ret = dw_xgbe_phy_xgmii_mode(phydev);
		break;

	case SPEED_2500:
		ret = dw_xgbe_phy_gmii_2500_mode(phydev);
		break;

	case SPEED_1000:
		ret = dw_xgbe_phy_gmii_mode(phydev);
		break;

	default:
		ret = -EINVAL;
	}

	if (ret < 0)
		return ret;

	/* Validate duplex mode */
	if (phydev->duplex != DUPLEX_FULL)
		return -EINVAL;

	phydev->pause = 0;
	phydev->asym_pause = 0;

	return 0;
}

static int dw_xgbe_phy_config_aneg(struct phy_device *phydev)
{
	struct dw_xgbe_phy_priv *priv = phydev->priv;
	u32 mmd_mask = phydev->c45_ids.devices_in_package;
	int ret;

	if (phydev->autoneg != AUTONEG_ENABLE)
		return dw_xgbe_phy_setup_forced(phydev);

	/* Make sure we have the AN MMD present */
	if (!(mmd_mask & MDIO_DEVS_AN))
		return -EINVAL;

	/* Get the current speed mode */
	ret = phy_read_mmd(phydev, MDIO_MMD_PCS, MDIO_CTRL2);
	if (ret < 0)
		return ret;

	/* Start/Restart the auto-negotiation state machine */
	mutex_lock(&priv->an_mutex);
	priv->an_result = DW_XGBE_AN_READY;
	priv->an_state = DW_XGBE_AN_START;
	priv->kr_state = DW_XGBE_RX_READY;
	priv->kx_state = DW_XGBE_RX_READY;
	mutex_unlock(&priv->an_mutex);

	queue_work(priv->an_workqueue, &priv->an_work);

	return 0;
}

static int dw_xgbe_phy_aneg_done(struct phy_device *phydev)
{
	struct dw_xgbe_phy_priv *priv = phydev->priv;
	enum dw_xgbe_phy_an state;

	mutex_lock(&priv->an_mutex);
	state = priv->an_result;
	mutex_unlock(&priv->an_mutex);

	return (state == DW_XGBE_AN_COMPLETE);
}

static int dw_xgbe_phy_update_link(struct phy_device *phydev)
{
	struct dw_xgbe_phy_priv *priv = phydev->priv;
	enum dw_xgbe_phy_an state;
	unsigned int check_again, autoneg;
	int ret;

	/* If we're doing auto-negotiation don't report link down */
	mutex_lock(&priv->an_mutex);
	state = priv->an_state;
	mutex_unlock(&priv->an_mutex);

	if (state != DW_XGBE_AN_READY) {
		phydev->link = 1;
		return 0;
	}

	/* Since the device can be in the wrong mode when a link is
	 * (re-)established (cable connected after the interface is
	 * up, etc.), the link status may report no link. If there
	 * is no link, try switching modes and checking the status
	 * again if auto negotiation is enabled.
	 */
	check_again = (phydev->autoneg == AUTONEG_ENABLE) ? 1 : 0;
again:
	/* Link status is latched low, so read once to clear
	 * and then read again to get current state
	 */
	ret = phy_read_mmd(phydev, MDIO_MMD_PCS, MDIO_STAT1);
	if (ret < 0)
		return ret;

	ret = phy_read_mmd(phydev, MDIO_MMD_PCS, MDIO_STAT1);
	if (ret < 0)
		return ret;

	phydev->link = (ret & MDIO_STAT1_LSTATUS) ? 1 : 0;

	if (!phydev->link) {
		if (check_again) {
			ret = dw_xgbe_phy_switch_mode(phydev);
			if (ret < 0)
				return ret;
			check_again = 0;
			goto again;
		}
	}

	autoneg = (phydev->link && !priv->link) ? 1 : 0;
	priv->link = phydev->link;
	if (autoneg) {
		/* Link is (back) up, re-start auto-negotiation */
		ret = dw_xgbe_phy_config_aneg(phydev);
		if (ret < 0)
			return ret;
	}

	return 0;
}

static int dw_xgbe_phy_read_status(struct phy_device *phydev)
{
	struct dw_xgbe_phy_priv *priv = phydev->priv;
	u32 mmd_mask = phydev->c45_ids.devices_in_package;
	int ret, mode, ad_ret, lp_ret;

	ret = dw_xgbe_phy_update_link(phydev);
	if (ret)
		return ret;

	mode = phy_read_mmd(phydev, MDIO_MMD_PCS, MDIO_CTRL2);
	if (mode < 0)
		return mode;
	mode &= MDIO_PCS_CTRL2_TYPE;

	if (phydev->autoneg == AUTONEG_ENABLE) {
		if (!(mmd_mask & MDIO_DEVS_AN))
			return -EINVAL;

		if (!dw_xgbe_phy_aneg_done(phydev))
			return 0;

		/* Compare Advertisement and Link Partner register 1 */
		ad_ret = phy_read_mmd(phydev, MDIO_MMD_AN, MDIO_AN_ADVERTISE);
		if (ad_ret < 0)
			return ad_ret;
		lp_ret = phy_read_mmd(phydev, MDIO_MMD_AN, MDIO_AN_LPA);
		if (lp_ret < 0)
			return lp_ret;

		ad_ret &= lp_ret;
		phydev->pause = (ad_ret & 0x400) ? 1 : 0;
		phydev->asym_pause = (ad_ret & 0x800) ? 1 : 0;

		/* Compare Advertisement and Link Partner register 2 */
		ad_ret = phy_read_mmd(phydev, MDIO_MMD_AN,
				      MDIO_AN_ADVERTISE + 1);
		if (ad_ret < 0)
			return ad_ret;
		lp_ret = phy_read_mmd(phydev, MDIO_MMD_AN, MDIO_AN_LPA + 1);
		if (lp_ret < 0)
			return lp_ret;

		ad_ret &= lp_ret;
		if (ad_ret & 0x80) {
			phydev->speed = SPEED_10000;
			if (mode != MDIO_PCS_CTRL2_10GBR) {
				ret = dw_xgbe_phy_xgmii_mode(phydev);
				if (ret < 0)
					return ret;
			}
		} else {
			int (*mode_fcn)(struct phy_device *);

			if (priv->speed_set ==
			    DW_XGBE_PHY_SPEEDSET_1000_10000) {
				phydev->speed = SPEED_1000;
				mode_fcn = dw_xgbe_phy_gmii_mode;
			} else {
				phydev->speed = SPEED_2500;
				mode_fcn = dw_xgbe_phy_gmii_2500_mode;
			}

			if (mode == MDIO_PCS_CTRL2_10GBR) {
				ret = mode_fcn(phydev);
				if (ret < 0)
					return ret;
			}
		}

		phydev->duplex = DUPLEX_FULL;
	} else {
		if (mode == MDIO_PCS_CTRL2_10GBR) {
			phydev->speed = SPEED_10000;
		} else {
			if (priv->speed_set ==
			    DW_XGBE_PHY_SPEEDSET_1000_10000)
				phydev->speed = SPEED_1000;
			else
				phydev->speed = SPEED_2500;
		}
		phydev->duplex = DUPLEX_FULL;
		phydev->pause = 0;
		phydev->asym_pause = 0;
	}

	return 0;
}

static int dw_xgbe_phy_suspend(struct phy_device *phydev)
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

static int dw_xgbe_phy_resume(struct phy_device *phydev)
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

static int dw_xgbe_phy_probe(struct phy_device *phydev)
{
	struct dw_xgbe_phy_priv *priv;
	struct platform_device *pdev;
	struct device *dev;
	char *wq_name;
	const __be32 *property;
	unsigned int speed_set;
	int ret;

	if (!phydev->dev.of_node)
		return -EINVAL;

	pdev = of_find_device_by_node(phydev->dev.of_node);
	if (!pdev)
		return -EINVAL;
	dev = &pdev->dev;

	wq_name = kasprintf(GFP_KERNEL, "%s-dw-xgbe-phy", phydev->bus->name);
	if (!wq_name) {
		ret = -ENOMEM;
		goto err_pdev;
	}

	priv = devm_kzalloc(dev, sizeof(*priv), GFP_KERNEL);
	if (!priv) {
		ret = -ENOMEM;
		goto err_name;
	}

	priv->pdev = pdev;
	priv->dev = dev;
	priv->phydev = phydev;

	/* Get the device speed set property */
	speed_set = 0;
	property = of_get_property(dev->of_node, XGBE_PHY_SPEEDSET_PROPERTY,
				   NULL);
	if (property)
		speed_set = be32_to_cpu(*property);

	switch (speed_set) {
	case 0:
		priv->speed_set = DW_XGBE_PHY_SPEEDSET_1000_10000;
		break;
	case 1:
		priv->speed_set = DW_XGBE_PHY_SPEEDSET_2500_10000;
		break;
	default:
		dev_err(dev, "invalid snps,speed-set property\n");
		ret = -EINVAL;
		goto err_sir1;
	}

	priv->link = 1;

	ret = phy_read_mmd(phydev, MDIO_MMD_PCS, MDIO_CTRL2);
	if (ret < 0)
		goto err_sir1;
	if ((ret & MDIO_PCS_CTRL2_TYPE) == MDIO_PCS_CTRL2_10GBR)
		priv->mode = DW_XGBE_MODE_KR;
	else
		priv->mode = DW_XGBE_MODE_KX;

	mutex_init(&priv->an_mutex);
	INIT_WORK(&priv->an_work, dw_xgbe_an_state_machine);
	priv->an_workqueue = create_singlethread_workqueue(wq_name);
	if (!priv->an_workqueue) {
		ret = -ENOMEM;
		goto err_sir1;
	}

	phydev->priv = priv;

	kfree(wq_name);
	of_dev_put(pdev);

	return 0;

err_sir1:
	devm_kfree(dev, priv);

err_name:
	kfree(wq_name);

err_pdev:
	of_dev_put(pdev);

	return ret;
}

static void dw_xgbe_phy_remove(struct phy_device *phydev)
{
	struct dw_xgbe_phy_priv *priv = phydev->priv;
	struct device *dev = priv->dev;

	/* Stop any in process auto-negotiation */
	mutex_lock(&priv->an_mutex);
	priv->an_state = DW_XGBE_AN_EXIT;
	mutex_unlock(&priv->an_mutex);

	flush_workqueue(priv->an_workqueue);
	destroy_workqueue(priv->an_workqueue);

	devm_kfree(dev, priv);
}

static int dw_xgbe_match_phy_device(struct phy_device *phydev)
{
	return phydev->c45_ids.device_ids[MDIO_MMD_PCS] == XGBE_PHY_ID;
}

static struct phy_driver dw_xgbe_phy_driver[] = {
	{
		.phy_id			= XGBE_PHY_ID,
		.phy_id_mask		= XGBE_PHY_MASK,
		.name			= "DW XGBE PHY",
		.features		= 0,
		.probe			= dw_xgbe_phy_probe,
		.remove			= dw_xgbe_phy_remove,
		.soft_reset		= dw_xgbe_phy_soft_reset,
		.config_init		= dw_xgbe_phy_config_init,
		.suspend		= dw_xgbe_phy_suspend,
		.resume			= dw_xgbe_phy_resume,
		.config_aneg		= dw_xgbe_phy_config_aneg,
		.aneg_done		= dw_xgbe_phy_aneg_done,
		.read_status		= dw_xgbe_phy_read_status,
		.match_phy_device	= dw_xgbe_match_phy_device,
		.driver			= {
			.owner = THIS_MODULE,
		},
	},
};

static int __init dw_xgbe_phy_init(void)
{
	return phy_drivers_register(dw_xgbe_phy_driver,
				    ARRAY_SIZE(dw_xgbe_phy_driver));
}

static void __exit dw_xgbe_phy_exit(void)
{
	phy_drivers_unregister(dw_xgbe_phy_driver,
			       ARRAY_SIZE(dw_xgbe_phy_driver));
}

module_init(dw_xgbe_phy_init);
module_exit(dw_xgbe_phy_exit);

static struct mdio_device_id __maybe_unused dw_xgbe_phy_ids[] = {
	{ XGBE_PHY_ID, XGBE_PHY_MASK },
	{ }
};
MODULE_DEVICE_TABLE(mdio, dw_xgbe_phy_ids);
