/*
 * Baikal Electronics 10Gb Ethernet driver
 *
 * Based on AMD 10Gb Ethernet driver
 * drivers/net/ethernet/amd/xgbe
 *
 * This file is available to you under your choice of the following two
 * licenses:
 *
 * License 1: GPLv2
 *
 * Copyright (c) 2015 Baikal Electronics JSC.
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
 * This file incorporates work covered by the following copyright and
 * permission notice:
 *     The Synopsys DWC ETHER XGMAC Software Driver and documentation
 *     (hereinafter "Software") is an unsupported proprietary work of Synopsys,
 *     Inc. unless otherwise expressly agreed to in writing between Synopsys
 *     and you.
 *
 *     The Software IS NOT an item of Licensed Software or Licensed Product
 *     under any End User Software License Agreement or Agreement for Licensed
 *     Product with Synopsys or any supplement thereto.  Permission is hereby
 *     granted, free of charge, to any person obtaining a copy of this software
 *     annotated with this license and the Software, to deal in the Software
 *     without restriction, including without limitation the rights to use,
 *     copy, modify, merge, publish, distribute, sublicense, and/or sell copies
 *     of the Software, and to permit persons to whom the Software is furnished
 *     to do so, subject to the following conditions:
 *
 *     The above copyright notice and this permission notice shall be included
 *     in all copies or substantial portions of the Software.
 *
 *     THIS SOFTWARE IS BEING DISTRIBUTED BY SYNOPSYS SOLELY ON AN "AS IS"
 *     BASIS AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
 *     TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
 *     PARTICULAR PURPOSE ARE HEREBY DISCLAIMED. IN NO EVENT SHALL SYNOPSYS
 *     BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 *     CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 *     SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 *     INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 *     CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 *     ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 *     THE POSSIBILITY OF SUCH DAMAGE.
 *
 *
 * License 2: Modified BSD
 *
 * Copyright (c) 2015 Baikal Electronics JSC.
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
 *
 * This file incorporates work covered by the following copyright and
 * permission notice:
 *     The Synopsys DWC ETHER XGMAC Software Driver and documentation
 *     (hereinafter "Software") is an unsupported proprietary work of Synopsys,
 *     Inc. unless otherwise expressly agreed to in writing between Synopsys
 *     and you.
 *
 *     The Software IS NOT an item of Licensed Software or Licensed Product
 *     under any End User Software License Agreement or Agreement for Licensed
 *     Product with Synopsys or any supplement thereto.  Permission is hereby
 *     granted, free of charge, to any person obtaining a copy of this software
 *     annotated with this license and the Software, to deal in the Software
 *     without restriction, including without limitation the rights to use,
 *     copy, modify, merge, publish, distribute, sublicense, and/or sell copies
 *     of the Software, and to permit persons to whom the Software is furnished
 *     to do so, subject to the following conditions:
 *
 *     The above copyright notice and this permission notice shall be included
 *     in all copies or substantial portions of the Software.
 *
 *     THIS SOFTWARE IS BEING DISTRIBUTED BY SYNOPSYS SOLELY ON AN "AS IS"
 *     BASIS AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
 *     TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
 *     PARTICULAR PURPOSE ARE HEREBY DISCLAIMED. IN NO EVENT SHALL SYNOPSYS
 *     BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 *     CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 *     SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 *     INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 *     CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 *     ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 *     THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef __XGBE_COMMON_H__
#define __XGBE_COMMON_H__

/* DMA register offsets */
#define DMA_MR				0x3000
#define DMA_SBMR			0x3004
#define DMA_ISR				0x3008
#define DMA_AXIARCR			0x3010
#define DMA_AXIAWCR			0x3018
#define DMA_DSR0			0x3020
#define DMA_DSR1			0x3024

/* DMA register entry bit positions and sizes */
#define DMA_AXIARCR_DRC_INDEX		0
#define DMA_AXIARCR_DRC_WIDTH		4
#define DMA_AXIARCR_DRD_INDEX		4
#define DMA_AXIARCR_DRD_WIDTH		2
#define DMA_AXIARCR_TEC_INDEX		8
#define DMA_AXIARCR_TEC_WIDTH		4
#define DMA_AXIARCR_TED_INDEX		12
#define DMA_AXIARCR_TED_WIDTH		2
#define DMA_AXIARCR_THC_INDEX		16
#define DMA_AXIARCR_THC_WIDTH		4
#define DMA_AXIARCR_THD_INDEX		20
#define DMA_AXIARCR_THD_WIDTH		2
#define DMA_AXIAWCR_DWC_INDEX		0
#define DMA_AXIAWCR_DWC_WIDTH		4
#define DMA_AXIAWCR_DWD_INDEX		4
#define DMA_AXIAWCR_DWD_WIDTH		2
#define DMA_AXIAWCR_RPC_INDEX		8
#define DMA_AXIAWCR_RPC_WIDTH		4
#define DMA_AXIAWCR_RPD_INDEX		12
#define DMA_AXIAWCR_RPD_WIDTH		2
#define DMA_AXIAWCR_RHC_INDEX		16
#define DMA_AXIAWCR_RHC_WIDTH		4
#define DMA_AXIAWCR_RHD_INDEX		20
#define DMA_AXIAWCR_RHD_WIDTH		2
#define DMA_AXIAWCR_TDC_INDEX		24
#define DMA_AXIAWCR_TDC_WIDTH		4
#define DMA_AXIAWCR_TDD_INDEX		28
#define DMA_AXIAWCR_TDD_WIDTH		2
#define DMA_ISR_MACIS_INDEX		17
#define DMA_ISR_MACIS_WIDTH		1
#define DMA_ISR_MTLIS_INDEX		16
#define DMA_ISR_MTLIS_WIDTH		1
#define DMA_MR_SWR_INDEX		0
#define DMA_MR_SWR_WIDTH		1
#define DMA_SBMR_EAME_INDEX		11
#define DMA_SBMR_EAME_WIDTH		1
#define DMA_SBMR_BLEN_16_INDEX		3
#define DMA_SBMR_BLEN_16_WIDTH		1
#define DMA_SBMR_BLEN_256_INDEX		7
#define DMA_SBMR_BLEN_256_WIDTH		1
#define DMA_SBMR_UNDEF_INDEX		0
#define DMA_SBMR_UNDEF_WIDTH		1

/* DMA register values */
#define DMA_DSR_RPS_WIDTH		4
#define DMA_DSR_TPS_WIDTH		4
#define DMA_DSR_Q_WIDTH			(DMA_DSR_RPS_WIDTH + DMA_DSR_TPS_WIDTH)
#define DMA_DSR0_RPS_START		8
#define DMA_DSR0_TPS_START		12
#define DMA_DSRX_FIRST_QUEUE		3
#define DMA_DSRX_INC			4
#define DMA_DSRX_QPR			4
#define DMA_DSRX_RPS_START		0
#define DMA_DSRX_TPS_START		4
#define DMA_TPS_STOPPED			0x00
#define DMA_TPS_SUSPENDED		0x06

/* DMA channel register offsets
 *   Multiple channels can be active.  The first channel has registers
 *   that begin at 0x3100.  Each subsequent channel has registers that
 *   are accessed using an offset of 0x80 from the previous channel.
 */
#define DMA_CH_BASE			0x3100
#define DMA_CH_INC			0x80

#define DMA_CH_CR			0x00
#define DMA_CH_TCR			0x04
#define DMA_CH_RCR			0x08
#define DMA_CH_TDLR_HI			0x10
#define DMA_CH_TDLR_LO			0x14
#define DMA_CH_RDLR_HI			0x18
#define DMA_CH_RDLR_LO			0x1c
#define DMA_CH_TDTR_LO			0x24
#define DMA_CH_RDTR_LO			0x2c
#define DMA_CH_TDRLR			0x30
#define DMA_CH_RDRLR			0x34
#define DMA_CH_IER			0x38
#define DMA_CH_RIWT			0x3c
#define DMA_CH_CATDR_LO			0x44
#define DMA_CH_CARDR_LO			0x4c
#define DMA_CH_CATBR_HI			0x50
#define DMA_CH_CATBR_LO			0x54
#define DMA_CH_CARBR_HI			0x58
#define DMA_CH_CARBR_LO			0x5c
#define DMA_CH_SR			0x60

/* DMA channel register entry bit positions and sizes */
#define DMA_CH_CR_PBLX8_INDEX		16
#define DMA_CH_CR_PBLX8_WIDTH		1
#define DMA_CH_CR_SPH_INDEX		24
#define DMA_CH_CR_SPH_WIDTH		1
#define DMA_CH_IER_AIE_INDEX		15
#define DMA_CH_IER_AIE_WIDTH		1
#define DMA_CH_IER_FBEE_INDEX		12
#define DMA_CH_IER_FBEE_WIDTH		1
#define DMA_CH_IER_NIE_INDEX		16
#define DMA_CH_IER_NIE_WIDTH		1
#define DMA_CH_IER_RBUE_INDEX		7
#define DMA_CH_IER_RBUE_WIDTH		1
#define DMA_CH_IER_RIE_INDEX		6
#define DMA_CH_IER_RIE_WIDTH		1
#define DMA_CH_IER_RSE_INDEX		8
#define DMA_CH_IER_RSE_WIDTH		1
#define DMA_CH_IER_TBUE_INDEX		2
#define DMA_CH_IER_TBUE_WIDTH		1
#define DMA_CH_IER_TIE_INDEX		0
#define DMA_CH_IER_TIE_WIDTH		1
#define DMA_CH_IER_TXSE_INDEX		1
#define DMA_CH_IER_TXSE_WIDTH		1
#define DMA_CH_RCR_PBL_INDEX		16
#define DMA_CH_RCR_PBL_WIDTH		6
#define DMA_CH_RCR_RBSZ_INDEX		1
#define DMA_CH_RCR_RBSZ_WIDTH		14
#define DMA_CH_RCR_SR_INDEX		0
#define DMA_CH_RCR_SR_WIDTH		1
#define DMA_CH_RIWT_RWT_INDEX		0
#define DMA_CH_RIWT_RWT_WIDTH		8
#define DMA_CH_SR_FBE_INDEX		12
#define DMA_CH_SR_FBE_WIDTH		1
#define DMA_CH_SR_RBU_INDEX		7
#define DMA_CH_SR_RBU_WIDTH		1
#define DMA_CH_SR_RI_INDEX		6
#define DMA_CH_SR_RI_WIDTH		1
#define DMA_CH_SR_RPS_INDEX		8
#define DMA_CH_SR_RPS_WIDTH		1
#define DMA_CH_SR_TBU_INDEX		2
#define DMA_CH_SR_TBU_WIDTH		1
#define DMA_CH_SR_TI_INDEX		0
#define DMA_CH_SR_TI_WIDTH		1
#define DMA_CH_SR_TPS_INDEX		1
#define DMA_CH_SR_TPS_WIDTH		1
#define DMA_CH_TCR_OSP_INDEX		4
#define DMA_CH_TCR_OSP_WIDTH		1
#define DMA_CH_TCR_PBL_INDEX		16
#define DMA_CH_TCR_PBL_WIDTH		6
#define DMA_CH_TCR_ST_INDEX		0
#define DMA_CH_TCR_ST_WIDTH		1
#define DMA_CH_TCR_TSE_INDEX		12
#define DMA_CH_TCR_TSE_WIDTH		1

/* DMA channel register values */
#define DMA_OSP_DISABLE			0x00
#define DMA_OSP_ENABLE			0x01
#define DMA_PBL_1			1
#define DMA_PBL_2			2
#define DMA_PBL_4			4
#define DMA_PBL_8			8
#define DMA_PBL_16			16
#define DMA_PBL_32			32
#define DMA_PBL_64			64      /* 8 x 8 */
#define DMA_PBL_128			128     /* 8 x 16 */
#define DMA_PBL_256			256     /* 8 x 32 */
#define DMA_PBL_X8_DISABLE		0x00
#define DMA_PBL_X8_ENABLE		0x01

/* MAC register offsets */
#define MAC_TCR				0x0000
#define MAC_RCR				0x0004
#define MAC_PFR				0x0008
#define MAC_WTR				0x000c
#define MAC_HTR0			0x0010
#define MAC_VLANTR			0x0050
#define MAC_VLANHTR			0x0058
#define MAC_VLANIR			0x0060
#define MAC_IVLANIR			0x0064
#define MAC_RETMR			0x006c
#define MAC_Q0TFCR			0x0070
#define MAC_RFCR			0x0090
#define MAC_RQC0R			0x00a0
#define MAC_RQC1R			0x00a4
#define MAC_RQC2R			0x00a8
#define MAC_RQC3R			0x00ac
#define MAC_ISR				0x00b0
#define MAC_IER				0x00b4
#define MAC_RTSR			0x00b8
#define MAC_PMTCSR			0x00c0
#define MAC_RWKPFR			0x00c4
#define MAC_LPICSR			0x00d0
#define MAC_LPITCR			0x00d4
#define MAC_VR				0x0110
#define MAC_DR				0x0114
#define MAC_HWF0R			0x011c
#define MAC_HWF1R			0x0120
#define MAC_HWF2R			0x0124
#define MAC_GPIOCR			0x0278
#define MAC_GPIOSR			0x027c
#define MAC_MACA0HR			0x0300
#define MAC_MACA0LR			0x0304
#define MAC_MACA1HR			0x0308
#define MAC_MACA1LR			0x030c
#define MAC_RSSCR			0x0c80
#define MAC_RSSAR			0x0c88
#define MAC_RSSDR			0x0c8c
#define MAC_TSCR			0x0d00
#define MAC_SSIR			0x0d04
#define MAC_STSR			0x0d08
#define MAC_STNR			0x0d0c
#define MAC_STSUR			0x0d10
#define MAC_STNUR			0x0d14
#define MAC_TSAR			0x0d18
#define MAC_TSSR			0x0d20
#define MAC_TXSNR			0x0d30
#define MAC_TXSSR			0x0d34

#define MAC_QTFCR_INC			4
#define MAC_MACA_INC			4
#define MAC_HTR_INC			4

#define MAC_RQC2_INC			4
#define MAC_RQC2_Q_PER_REG		4

/* MAC register entry bit positions and sizes */
#define MAC_HWF0R_ADDMACADRSEL_INDEX	18
#define MAC_HWF0R_ADDMACADRSEL_WIDTH	5
#define MAC_HWF0R_ARPOFFSEL_INDEX	9
#define MAC_HWF0R_ARPOFFSEL_WIDTH	1
#define MAC_HWF0R_EEESEL_INDEX		13
#define MAC_HWF0R_EEESEL_WIDTH		1
#define MAC_HWF0R_GMIISEL_INDEX		1
#define MAC_HWF0R_GMIISEL_WIDTH		1
#define MAC_HWF0R_MGKSEL_INDEX		7
#define MAC_HWF0R_MGKSEL_WIDTH		1
#define MAC_HWF0R_MMCSEL_INDEX		8
#define MAC_HWF0R_MMCSEL_WIDTH		1
#define MAC_HWF0R_RWKSEL_INDEX		6
#define MAC_HWF0R_RWKSEL_WIDTH		1
#define MAC_HWF0R_RXCOESEL_INDEX	16
#define MAC_HWF0R_RXCOESEL_WIDTH	1
#define MAC_HWF0R_SAVLANINS_INDEX	27
#define MAC_HWF0R_SAVLANINS_WIDTH	1
#define MAC_HWF0R_SMASEL_INDEX		5
#define MAC_HWF0R_SMASEL_WIDTH		1
#define MAC_HWF0R_TSSEL_INDEX		12
#define MAC_HWF0R_TSSEL_WIDTH		1
#define MAC_HWF0R_TSSTSSEL_INDEX	25
#define MAC_HWF0R_TSSTSSEL_WIDTH	2
#define MAC_HWF0R_TXCOESEL_INDEX	14
#define MAC_HWF0R_TXCOESEL_WIDTH	1
#define MAC_HWF0R_VLHASH_INDEX		4
#define MAC_HWF0R_VLHASH_WIDTH		1
#define MAC_HWF1R_ADVTHWORD_INDEX	13
#define MAC_HWF1R_ADVTHWORD_WIDTH	1
#define MAC_HWF1R_DBGMEMA_INDEX		19
#define MAC_HWF1R_DBGMEMA_WIDTH		1
#define MAC_HWF1R_DCBEN_INDEX		16
#define MAC_HWF1R_DCBEN_WIDTH		1
#define MAC_HWF1R_HASHTBLSZ_INDEX	24
#define MAC_HWF1R_HASHTBLSZ_WIDTH	3
#define MAC_HWF1R_L3L4FNUM_INDEX	27
#define MAC_HWF1R_L3L4FNUM_WIDTH	4
#define MAC_HWF1R_NUMTC_INDEX		21
#define MAC_HWF1R_NUMTC_WIDTH		3
#define MAC_HWF1R_RSSEN_INDEX		20
#define MAC_HWF1R_RSSEN_WIDTH		1
#define MAC_HWF1R_RXFIFOSIZE_INDEX	0
#define MAC_HWF1R_RXFIFOSIZE_WIDTH	5
#define MAC_HWF1R_SPHEN_INDEX		17
#define MAC_HWF1R_SPHEN_WIDTH		1
#define MAC_HWF1R_TSOEN_INDEX		18
#define MAC_HWF1R_TSOEN_WIDTH		1
#define MAC_HWF1R_TXFIFOSIZE_INDEX	6
#define MAC_HWF1R_TXFIFOSIZE_WIDTH	5
#define MAC_HWF2R_AUXSNAPNUM_INDEX	28
#define MAC_HWF2R_AUXSNAPNUM_WIDTH	3
#define MAC_HWF2R_PPSOUTNUM_INDEX	24
#define MAC_HWF2R_PPSOUTNUM_WIDTH	3
#define MAC_HWF2R_RXCHCNT_INDEX		12
#define MAC_HWF2R_RXCHCNT_WIDTH		4
#define MAC_HWF2R_RXQCNT_INDEX		0
#define MAC_HWF2R_RXQCNT_WIDTH		4
#define MAC_HWF2R_TXCHCNT_INDEX		18
#define MAC_HWF2R_TXCHCNT_WIDTH		4
#define MAC_HWF2R_TXQCNT_INDEX		6
#define MAC_HWF2R_TXQCNT_WIDTH		4
#define MAC_IER_TSIE_INDEX		12
#define MAC_IER_TSIE_WIDTH		1
#define MAC_ISR_MMCRXIS_INDEX		9
#define MAC_ISR_MMCRXIS_WIDTH		1
#define MAC_ISR_MMCTXIS_INDEX		10
#define MAC_ISR_MMCTXIS_WIDTH		1
#define MAC_ISR_PMTIS_INDEX		4
#define MAC_ISR_PMTIS_WIDTH		1
#define MAC_ISR_TSIS_INDEX		12
#define MAC_ISR_TSIS_WIDTH		1
#define MAC_MACA1HR_AE_INDEX		31
#define MAC_MACA1HR_AE_WIDTH		1
#define MAC_PFR_HMC_INDEX		2
#define MAC_PFR_HMC_WIDTH		1
#define MAC_PFR_HPF_INDEX		10
#define MAC_PFR_HPF_WIDTH		1
#define MAC_PFR_HUC_INDEX		1
#define MAC_PFR_HUC_WIDTH		1
#define MAC_PFR_PM_INDEX		4
#define MAC_PFR_PM_WIDTH		1
#define MAC_PFR_PR_INDEX		0
#define MAC_PFR_PR_WIDTH		1
#define MAC_PFR_VTFE_INDEX		16
#define MAC_PFR_VTFE_WIDTH		1
#define MAC_PMTCSR_MGKPKTEN_INDEX	1
#define MAC_PMTCSR_MGKPKTEN_WIDTH	1
#define MAC_PMTCSR_PWRDWN_INDEX		0
#define MAC_PMTCSR_PWRDWN_WIDTH		1
#define MAC_PMTCSR_RWKFILTRST_INDEX	31
#define MAC_PMTCSR_RWKFILTRST_WIDTH	1
#define MAC_PMTCSR_RWKPKTEN_INDEX	2
#define MAC_PMTCSR_RWKPKTEN_WIDTH	1
#define MAC_Q0TFCR_PT_INDEX		16
#define MAC_Q0TFCR_PT_WIDTH		16
#define MAC_Q0TFCR_TFE_INDEX		1
#define MAC_Q0TFCR_TFE_WIDTH		1
#define MAC_RCR_ACS_INDEX		1
#define MAC_RCR_ACS_WIDTH		1
#define MAC_RCR_CST_INDEX		2
#define MAC_RCR_CST_WIDTH		1
#define MAC_RCR_DCRCC_INDEX		3
#define MAC_RCR_DCRCC_WIDTH		1
#define MAC_RCR_HDSMS_INDEX		12
#define MAC_RCR_HDSMS_WIDTH		3
#define MAC_RCR_IPC_INDEX		9
#define MAC_RCR_IPC_WIDTH		1
#define MAC_RCR_JE_INDEX		8
#define MAC_RCR_JE_WIDTH		1
#define MAC_RCR_LM_INDEX		10
#define MAC_RCR_LM_WIDTH		1
#define MAC_RCR_RE_INDEX		0
#define MAC_RCR_RE_WIDTH		1
#define MAC_RFCR_PFCE_INDEX		8
#define MAC_RFCR_PFCE_WIDTH		1
#define MAC_RFCR_RFE_INDEX		0
#define MAC_RFCR_RFE_WIDTH		1
#define MAC_RFCR_UP_INDEX		1
#define MAC_RFCR_UP_WIDTH		1
#define MAC_RQC0R_RXQ0EN_INDEX		0
#define MAC_RQC0R_RXQ0EN_WIDTH		2
#define MAC_RSSAR_ADDRT_INDEX		2
#define MAC_RSSAR_ADDRT_WIDTH		1
#define MAC_RSSAR_CT_INDEX		1
#define MAC_RSSAR_CT_WIDTH		1
#define MAC_RSSAR_OB_INDEX		0
#define MAC_RSSAR_OB_WIDTH		1
#define MAC_RSSAR_RSSIA_INDEX		8
#define MAC_RSSAR_RSSIA_WIDTH		8
#define MAC_RSSCR_IP2TE_INDEX		1
#define MAC_RSSCR_IP2TE_WIDTH		1
#define MAC_RSSCR_RSSE_INDEX		0
#define MAC_RSSCR_RSSE_WIDTH		1
#define MAC_RSSCR_TCP4TE_INDEX		2
#define MAC_RSSCR_TCP4TE_WIDTH		1
#define MAC_RSSCR_UDP4TE_INDEX		3
#define MAC_RSSCR_UDP4TE_WIDTH		1
#define MAC_RSSDR_DMCH_INDEX		0
#define MAC_RSSDR_DMCH_WIDTH		4
#define MAC_SSIR_SNSINC_INDEX		8
#define MAC_SSIR_SNSINC_WIDTH		8
#define MAC_SSIR_SSINC_INDEX		16
#define MAC_SSIR_SSINC_WIDTH		8
#define MAC_TCR_SS_INDEX		29
#define MAC_TCR_SS_WIDTH		2
#define MAC_TCR_TE_INDEX		0
#define MAC_TCR_TE_WIDTH		1
#define MAC_TSCR_AV8021ASMEN_INDEX	28
#define MAC_TSCR_AV8021ASMEN_WIDTH	1
#define MAC_TSCR_SNAPTYPSEL_INDEX	16
#define MAC_TSCR_SNAPTYPSEL_WIDTH	2
#define MAC_TSCR_TSADDREG_INDEX		5
#define MAC_TSCR_TSADDREG_WIDTH		1
#define MAC_TSCR_TSCFUPDT_INDEX		1
#define MAC_TSCR_TSCFUPDT_WIDTH		1
#define MAC_TSCR_TSCTRLSSR_INDEX	9
#define MAC_TSCR_TSCTRLSSR_WIDTH	1
#define MAC_TSCR_TSENA_INDEX		0
#define MAC_TSCR_TSENA_WIDTH		1
#define MAC_TSCR_TSENALL_INDEX		8
#define MAC_TSCR_TSENALL_WIDTH		1
#define MAC_TSCR_TSEVNTENA_INDEX	14
#define MAC_TSCR_TSEVNTENA_WIDTH	1
#define MAC_TSCR_TSINIT_INDEX		2
#define MAC_TSCR_TSINIT_WIDTH		1
#define MAC_TSCR_TSIPENA_INDEX		11
#define MAC_TSCR_TSIPENA_WIDTH		1
#define MAC_TSCR_TSIPV4ENA_INDEX	13
#define MAC_TSCR_TSIPV4ENA_WIDTH	1
#define MAC_TSCR_TSIPV6ENA_INDEX	12
#define MAC_TSCR_TSIPV6ENA_WIDTH	1
#define MAC_TSCR_TSMSTRENA_INDEX	15
#define MAC_TSCR_TSMSTRENA_WIDTH	1
#define MAC_TSCR_TSVER2ENA_INDEX	10
#define MAC_TSCR_TSVER2ENA_WIDTH	1
#define MAC_TSCR_TXTSSTSM_INDEX		24
#define MAC_TSCR_TXTSSTSM_WIDTH		1
#define MAC_TSSR_TXTSC_INDEX		15
#define MAC_TSSR_TXTSC_WIDTH		1
#define MAC_TXSNR_TXTSSTSMIS_INDEX	31
#define MAC_TXSNR_TXTSSTSMIS_WIDTH	1
#define MAC_VLANHTR_VLHT_INDEX		0
#define MAC_VLANHTR_VLHT_WIDTH		16
#define MAC_VLANIR_VLTI_INDEX		20
#define MAC_VLANIR_VLTI_WIDTH		1
#define MAC_VLANIR_CSVL_INDEX		19
#define MAC_VLANIR_CSVL_WIDTH		1
#define MAC_VLANTR_DOVLTC_INDEX		20
#define MAC_VLANTR_DOVLTC_WIDTH		1
#define MAC_VLANTR_ERSVLM_INDEX		19
#define MAC_VLANTR_ERSVLM_WIDTH		1
#define MAC_VLANTR_ESVL_INDEX		18
#define MAC_VLANTR_ESVL_WIDTH		1
#define MAC_VLANTR_ETV_INDEX		16
#define MAC_VLANTR_ETV_WIDTH		1
#define MAC_VLANTR_EVLS_INDEX		21
#define MAC_VLANTR_EVLS_WIDTH		2
#define MAC_VLANTR_EVLRXS_INDEX		24
#define MAC_VLANTR_EVLRXS_WIDTH		1
#define MAC_VLANTR_VL_INDEX		0
#define MAC_VLANTR_VL_WIDTH		16
#define MAC_VLANTR_VTHM_INDEX		25
#define MAC_VLANTR_VTHM_WIDTH		1
#define MAC_VLANTR_VTIM_INDEX		17
#define MAC_VLANTR_VTIM_WIDTH		1
#define MAC_VR_DEVID_INDEX		8
#define MAC_VR_DEVID_WIDTH		8
#define MAC_VR_SNPSVER_INDEX		0
#define MAC_VR_SNPSVER_WIDTH		8
#define MAC_VR_USERVER_INDEX		16
#define MAC_VR_USERVER_WIDTH		8

/* MMC register offsets */
#define MMC_CR				0x0800
#define MMC_RISR			0x0804
#define MMC_TISR			0x0808
#define MMC_RIER			0x080c
#define MMC_TIER			0x0810
#define MMC_TXOCTETCOUNT_GB_LO		0x0814
#define MMC_TXOCTETCOUNT_GB_HI		0x0818
#define MMC_TXFRAMECOUNT_GB_LO		0x081c
#define MMC_TXFRAMECOUNT_GB_HI		0x0820
#define MMC_TXBROADCASTFRAMES_G_LO	0x0824
#define MMC_TXBROADCASTFRAMES_G_HI	0x0828
#define MMC_TXMULTICASTFRAMES_G_LO	0x082c
#define MMC_TXMULTICASTFRAMES_G_HI	0x0830
#define MMC_TX64OCTETS_GB_LO		0x0834
#define MMC_TX64OCTETS_GB_HI		0x0838
#define MMC_TX65TO127OCTETS_GB_LO	0x083c
#define MMC_TX65TO127OCTETS_GB_HI	0x0840
#define MMC_TX128TO255OCTETS_GB_LO	0x0844
#define MMC_TX128TO255OCTETS_GB_HI	0x0848
#define MMC_TX256TO511OCTETS_GB_LO	0x084c
#define MMC_TX256TO511OCTETS_GB_HI	0x0850
#define MMC_TX512TO1023OCTETS_GB_LO	0x0854
#define MMC_TX512TO1023OCTETS_GB_HI	0x0858
#define MMC_TX1024TOMAXOCTETS_GB_LO	0x085c
#define MMC_TX1024TOMAXOCTETS_GB_HI	0x0860
#define MMC_TXUNICASTFRAMES_GB_LO	0x0864
#define MMC_TXUNICASTFRAMES_GB_HI	0x0868
#define MMC_TXMULTICASTFRAMES_GB_LO	0x086c
#define MMC_TXMULTICASTFRAMES_GB_HI	0x0870
#define MMC_TXBROADCASTFRAMES_GB_LO	0x0874
#define MMC_TXBROADCASTFRAMES_GB_HI	0x0878
#define MMC_TXUNDERFLOWERROR_LO		0x087c
#define MMC_TXUNDERFLOWERROR_HI		0x0880
#define MMC_TXOCTETCOUNT_G_LO		0x0884
#define MMC_TXOCTETCOUNT_G_HI		0x0888
#define MMC_TXFRAMECOUNT_G_LO		0x088c
#define MMC_TXFRAMECOUNT_G_HI		0x0890
#define MMC_TXPAUSEFRAMES_LO		0x0894
#define MMC_TXPAUSEFRAMES_HI		0x0898
#define MMC_TXVLANFRAMES_G_LO		0x089c
#define MMC_TXVLANFRAMES_G_HI		0x08a0
#define MMC_RXFRAMECOUNT_GB_LO		0x0900
#define MMC_RXFRAMECOUNT_GB_HI		0x0904
#define MMC_RXOCTETCOUNT_GB_LO		0x0908
#define MMC_RXOCTETCOUNT_GB_HI		0x090c
#define MMC_RXOCTETCOUNT_G_LO		0x0910
#define MMC_RXOCTETCOUNT_G_HI		0x0914
#define MMC_RXBROADCASTFRAMES_G_LO	0x0918
#define MMC_RXBROADCASTFRAMES_G_HI	0x091c
#define MMC_RXMULTICASTFRAMES_G_LO	0x0920
#define MMC_RXMULTICASTFRAMES_G_HI	0x0924
#define MMC_RXCRCERROR_LO		0x0928
#define MMC_RXCRCERROR_HI		0x092c
#define MMC_RXRUNTERROR			0x0930
#define MMC_RXJABBERERROR		0x0934
#define MMC_RXUNDERSIZE_G		0x0938
#define MMC_RXOVERSIZE_G		0x093c
#define MMC_RX64OCTETS_GB_LO		0x0940
#define MMC_RX64OCTETS_GB_HI		0x0944
#define MMC_RX65TO127OCTETS_GB_LO	0x0948
#define MMC_RX65TO127OCTETS_GB_HI	0x094c
#define MMC_RX128TO255OCTETS_GB_LO	0x0950
#define MMC_RX128TO255OCTETS_GB_HI	0x0954
#define MMC_RX256TO511OCTETS_GB_LO	0x0958
#define MMC_RX256TO511OCTETS_GB_HI	0x095c
#define MMC_RX512TO1023OCTETS_GB_LO	0x0960
#define MMC_RX512TO1023OCTETS_GB_HI	0x0964
#define MMC_RX1024TOMAXOCTETS_GB_LO	0x0968
#define MMC_RX1024TOMAXOCTETS_GB_HI	0x096c
#define MMC_RXUNICASTFRAMES_G_LO	0x0970
#define MMC_RXUNICASTFRAMES_G_HI	0x0974
#define MMC_RXLENGTHERROR_LO		0x0978
#define MMC_RXLENGTHERROR_HI		0x097c
#define MMC_RXOUTOFRANGETYPE_LO		0x0980
#define MMC_RXOUTOFRANGETYPE_HI		0x0984
#define MMC_RXPAUSEFRAMES_LO		0x0988
#define MMC_RXPAUSEFRAMES_HI		0x098c
#define MMC_RXFIFOOVERFLOW_LO		0x0990
#define MMC_RXFIFOOVERFLOW_HI		0x0994
#define MMC_RXVLANFRAMES_GB_LO		0x0998
#define MMC_RXVLANFRAMES_GB_HI		0x099c
#define MMC_RXWATCHDOGERROR		0x09a0

/* MMC register entry bit positions and sizes */
#define MMC_CR_CR_INDEX				0
#define MMC_CR_CR_WIDTH				1
#define MMC_CR_CSR_INDEX			1
#define MMC_CR_CSR_WIDTH			1
#define MMC_CR_ROR_INDEX			2
#define MMC_CR_ROR_WIDTH			1
#define MMC_CR_MCF_INDEX			3
#define MMC_CR_MCF_WIDTH			1
#define MMC_CR_MCT_INDEX			4
#define MMC_CR_MCT_WIDTH			2
#define MMC_RIER_ALL_INTERRUPTS_INDEX		0
#define MMC_RIER_ALL_INTERRUPTS_WIDTH		23
#define MMC_RISR_RXFRAMECOUNT_GB_INDEX		0
#define MMC_RISR_RXFRAMECOUNT_GB_WIDTH		1
#define MMC_RISR_RXOCTETCOUNT_GB_INDEX		1
#define MMC_RISR_RXOCTETCOUNT_GB_WIDTH		1
#define MMC_RISR_RXOCTETCOUNT_G_INDEX		2
#define MMC_RISR_RXOCTETCOUNT_G_WIDTH		1
#define MMC_RISR_RXBROADCASTFRAMES_G_INDEX	3
#define MMC_RISR_RXBROADCASTFRAMES_G_WIDTH	1
#define MMC_RISR_RXMULTICASTFRAMES_G_INDEX	4
#define MMC_RISR_RXMULTICASTFRAMES_G_WIDTH	1
#define MMC_RISR_RXCRCERROR_INDEX		5
#define MMC_RISR_RXCRCERROR_WIDTH		1
#define MMC_RISR_RXRUNTERROR_INDEX		6
#define MMC_RISR_RXRUNTERROR_WIDTH		1
#define MMC_RISR_RXJABBERERROR_INDEX		7
#define MMC_RISR_RXJABBERERROR_WIDTH		1
#define MMC_RISR_RXUNDERSIZE_G_INDEX		8
#define MMC_RISR_RXUNDERSIZE_G_WIDTH		1
#define MMC_RISR_RXOVERSIZE_G_INDEX		9
#define MMC_RISR_RXOVERSIZE_G_WIDTH		1
#define MMC_RISR_RX64OCTETS_GB_INDEX		10
#define MMC_RISR_RX64OCTETS_GB_WIDTH		1
#define MMC_RISR_RX65TO127OCTETS_GB_INDEX	11
#define MMC_RISR_RX65TO127OCTETS_GB_WIDTH	1
#define MMC_RISR_RX128TO255OCTETS_GB_INDEX	12
#define MMC_RISR_RX128TO255OCTETS_GB_WIDTH	1
#define MMC_RISR_RX256TO511OCTETS_GB_INDEX	13
#define MMC_RISR_RX256TO511OCTETS_GB_WIDTH	1
#define MMC_RISR_RX512TO1023OCTETS_GB_INDEX	14
#define MMC_RISR_RX512TO1023OCTETS_GB_WIDTH	1
#define MMC_RISR_RX1024TOMAXOCTETS_GB_INDEX	15
#define MMC_RISR_RX1024TOMAXOCTETS_GB_WIDTH	1
#define MMC_RISR_RXUNICASTFRAMES_G_INDEX	16
#define MMC_RISR_RXUNICASTFRAMES_G_WIDTH	1
#define MMC_RISR_RXLENGTHERROR_INDEX		17
#define MMC_RISR_RXLENGTHERROR_WIDTH		1
#define MMC_RISR_RXOUTOFRANGETYPE_INDEX		18
#define MMC_RISR_RXOUTOFRANGETYPE_WIDTH		1
#define MMC_RISR_RXPAUSEFRAMES_INDEX		19
#define MMC_RISR_RXPAUSEFRAMES_WIDTH		1
#define MMC_RISR_RXFIFOOVERFLOW_INDEX		20
#define MMC_RISR_RXFIFOOVERFLOW_WIDTH		1
#define MMC_RISR_RXVLANFRAMES_GB_INDEX		21
#define MMC_RISR_RXVLANFRAMES_GB_WIDTH		1
#define MMC_RISR_RXWATCHDOGERROR_INDEX		22
#define MMC_RISR_RXWATCHDOGERROR_WIDTH		1
#define MMC_TIER_ALL_INTERRUPTS_INDEX		0
#define MMC_TIER_ALL_INTERRUPTS_WIDTH		18
#define MMC_TISR_TXOCTETCOUNT_GB_INDEX		0
#define MMC_TISR_TXOCTETCOUNT_GB_WIDTH		1
#define MMC_TISR_TXFRAMECOUNT_GB_INDEX		1
#define MMC_TISR_TXFRAMECOUNT_GB_WIDTH		1
#define MMC_TISR_TXBROADCASTFRAMES_G_INDEX	2
#define MMC_TISR_TXBROADCASTFRAMES_G_WIDTH	1
#define MMC_TISR_TXMULTICASTFRAMES_G_INDEX	3
#define MMC_TISR_TXMULTICASTFRAMES_G_WIDTH	1
#define MMC_TISR_TX64OCTETS_GB_INDEX		4
#define MMC_TISR_TX64OCTETS_GB_WIDTH		1
#define MMC_TISR_TX65TO127OCTETS_GB_INDEX	5
#define MMC_TISR_TX65TO127OCTETS_GB_WIDTH	1
#define MMC_TISR_TX128TO255OCTETS_GB_INDEX	6
#define MMC_TISR_TX128TO255OCTETS_GB_WIDTH	1
#define MMC_TISR_TX256TO511OCTETS_GB_INDEX	7
#define MMC_TISR_TX256TO511OCTETS_GB_WIDTH	1
#define MMC_TISR_TX512TO1023OCTETS_GB_INDEX	8
#define MMC_TISR_TX512TO1023OCTETS_GB_WIDTH	1
#define MMC_TISR_TX1024TOMAXOCTETS_GB_INDEX	9
#define MMC_TISR_TX1024TOMAXOCTETS_GB_WIDTH	1
#define MMC_TISR_TXUNICASTFRAMES_GB_INDEX	10
#define MMC_TISR_TXUNICASTFRAMES_GB_WIDTH	1
#define MMC_TISR_TXMULTICASTFRAMES_GB_INDEX	11
#define MMC_TISR_TXMULTICASTFRAMES_GB_WIDTH	1
#define MMC_TISR_TXBROADCASTFRAMES_GB_INDEX	12
#define MMC_TISR_TXBROADCASTFRAMES_GB_WIDTH	1
#define MMC_TISR_TXUNDERFLOWERROR_INDEX		13
#define MMC_TISR_TXUNDERFLOWERROR_WIDTH		1
#define MMC_TISR_TXOCTETCOUNT_G_INDEX		14
#define MMC_TISR_TXOCTETCOUNT_G_WIDTH		1
#define MMC_TISR_TXFRAMECOUNT_G_INDEX		15
#define MMC_TISR_TXFRAMECOUNT_G_WIDTH		1
#define MMC_TISR_TXPAUSEFRAMES_INDEX		16
#define MMC_TISR_TXPAUSEFRAMES_WIDTH		1
#define MMC_TISR_TXVLANFRAMES_G_INDEX		17
#define MMC_TISR_TXVLANFRAMES_G_WIDTH		1

/* MTL register offsets */
#define MTL_OMR				0x1000
#define MTL_FDCR			0x1008
#define MTL_FDSR			0x100c
#define MTL_FDDR			0x1010
#define MTL_ISR				0x1020
#define MTL_RQDCM0R			0x1030
#define MTL_TCPM0R			0x1040
#define MTL_TCPM1R			0x1044

#define MTL_RQDCM_INC			4
#define MTL_RQDCM_Q_PER_REG		4
#define MTL_TCPM_INC			4
#define MTL_TCPM_TC_PER_REG		4

/* MTL register entry bit positions and sizes */
#define MTL_OMR_ETSALG_INDEX		5
#define MTL_OMR_ETSALG_WIDTH		2
#define MTL_OMR_RAA_INDEX		2
#define MTL_OMR_RAA_WIDTH		1

/* MTL queue register offsets
 *   Multiple queues can be active.  The first queue has registers
 *   that begin at 0x1100.  Each subsequent queue has registers that
 *   are accessed using an offset of 0x80 from the previous queue.
 */
#define MTL_Q_BASE			0x1100
#define MTL_Q_INC			0x80

#define MTL_Q_TQOMR			0x00
#define MTL_Q_TQUR			0x04
#define MTL_Q_TQDR			0x08
#define MTL_Q_RQOMR			0x40
#define MTL_Q_RQMPOCR			0x44
#define MTL_Q_RQDR			0x4c
#define MTL_Q_RQFCR			0x50
#define MTL_Q_IER			0x70
#define MTL_Q_ISR			0x74

/* MTL queue register entry bit positions and sizes */
#define MTL_Q_RQFCR_RFA_INDEX		1
#define MTL_Q_RQFCR_RFA_WIDTH		6
#define MTL_Q_RQFCR_RFD_INDEX		17
#define MTL_Q_RQFCR_RFD_WIDTH		6
#define MTL_Q_RQOMR_EHFC_INDEX		7
#define MTL_Q_RQOMR_EHFC_WIDTH		1
#define MTL_Q_RQOMR_RQS_INDEX		16
#define MTL_Q_RQOMR_RQS_WIDTH		9
#define MTL_Q_RQOMR_RSF_INDEX		5
#define MTL_Q_RQOMR_RSF_WIDTH		1
#define MTL_Q_RQOMR_RTC_INDEX		0
#define MTL_Q_RQOMR_RTC_WIDTH		2
#define MTL_Q_TQOMR_FTQ_INDEX		0
#define MTL_Q_TQOMR_FTQ_WIDTH		1
#define MTL_Q_TQOMR_Q2TCMAP_INDEX	8
#define MTL_Q_TQOMR_Q2TCMAP_WIDTH	3
#define MTL_Q_TQOMR_TQS_INDEX		16
#define MTL_Q_TQOMR_TQS_WIDTH		10
#define MTL_Q_TQOMR_TSF_INDEX		1
#define MTL_Q_TQOMR_TSF_WIDTH		1
#define MTL_Q_TQOMR_TTC_INDEX		4
#define MTL_Q_TQOMR_TTC_WIDTH		3
#define MTL_Q_TQOMR_TXQEN_INDEX		2
#define MTL_Q_TQOMR_TXQEN_WIDTH		2

/* MTL queue register value */
#define MTL_RSF_DISABLE			0x00
#define MTL_RSF_ENABLE			0x01
#define MTL_TSF_DISABLE			0x00
#define MTL_TSF_ENABLE			0x01

#define MTL_RX_THRESHOLD_64		0x00
#define MTL_RX_THRESHOLD_96		0x02
#define MTL_RX_THRESHOLD_128		0x03
#define MTL_TX_THRESHOLD_32		0x01
#define MTL_TX_THRESHOLD_64		0x00
#define MTL_TX_THRESHOLD_96		0x02
#define MTL_TX_THRESHOLD_128		0x03
#define MTL_TX_THRESHOLD_192		0x04
#define MTL_TX_THRESHOLD_256		0x05
#define MTL_TX_THRESHOLD_384		0x06
#define MTL_TX_THRESHOLD_512		0x07

#define MTL_ETSALG_WRR			0x00
#define MTL_ETSALG_WFQ			0x01
#define MTL_ETSALG_DWRR			0x02
#define MTL_RAA_SP			0x00
#define MTL_RAA_WSP			0x01

#define MTL_Q_DISABLED			0x00
#define MTL_Q_ENABLED			0x02

/* MTL traffic class register offsets
 *   Multiple traffic classes can be active.  The first class has registers
 *   that begin at 0x1100.  Each subsequent queue has registers that
 *   are accessed using an offset of 0x80 from the previous queue.
 */
#define MTL_TC_BASE			MTL_Q_BASE
#define MTL_TC_INC			MTL_Q_INC

#define MTL_TC_ETSCR			0x10
#define MTL_TC_ETSSR			0x14
#define MTL_TC_QWR			0x18

/* MTL traffic class register entry bit positions and sizes */
#define MTL_TC_ETSCR_TSA_INDEX		0
#define MTL_TC_ETSCR_TSA_WIDTH		2
#define MTL_TC_QWR_QW_INDEX		0
#define MTL_TC_QWR_QW_WIDTH		21

/* MTL traffic class register value */
#define MTL_TSA_SP			0x00
#define MTL_TSA_ETS			0x02

/* PCS MMD select register offset
 *  The MMD select register is used for accessing PCS registers
 *  when the underlying APB3 interface is using indirect addressing.
 *  Indirect addressing requires accessing registers in two phases,
 *  an address phase and a data phase.  The address phases requires
 *  writing an address selection value to the MMD select regiesters.
 */
#define PCS_MMD_SELECT			0xff

/* Descriptor/Packet entry bit positions and sizes */
#define RX_PACKET_ERRORS_CRC_INDEX		2
#define RX_PACKET_ERRORS_CRC_WIDTH		1
#define RX_PACKET_ERRORS_FRAME_INDEX		3
#define RX_PACKET_ERRORS_FRAME_WIDTH		1
#define RX_PACKET_ERRORS_LENGTH_INDEX		0
#define RX_PACKET_ERRORS_LENGTH_WIDTH		1
#define RX_PACKET_ERRORS_OVERRUN_INDEX		1
#define RX_PACKET_ERRORS_OVERRUN_WIDTH		1

#define RX_PACKET_ATTRIBUTES_CSUM_DONE_INDEX	0
#define RX_PACKET_ATTRIBUTES_CSUM_DONE_WIDTH	1
#define RX_PACKET_ATTRIBUTES_VLAN_CTAG_INDEX	1
#define RX_PACKET_ATTRIBUTES_VLAN_CTAG_WIDTH	1
#define RX_PACKET_ATTRIBUTES_INCOMPLETE_INDEX	2
#define RX_PACKET_ATTRIBUTES_INCOMPLETE_WIDTH	1
#define RX_PACKET_ATTRIBUTES_CONTEXT_NEXT_INDEX	3
#define RX_PACKET_ATTRIBUTES_CONTEXT_NEXT_WIDTH	1
#define RX_PACKET_ATTRIBUTES_CONTEXT_INDEX	4
#define RX_PACKET_ATTRIBUTES_CONTEXT_WIDTH	1
#define RX_PACKET_ATTRIBUTES_RX_TSTAMP_INDEX	5
#define RX_PACKET_ATTRIBUTES_RX_TSTAMP_WIDTH	1
#define RX_PACKET_ATTRIBUTES_RSS_HASH_INDEX	6
#define RX_PACKET_ATTRIBUTES_RSS_HASH_WIDTH	1

#define RX_NORMAL_DESC0_OVT_INDEX		0
#define RX_NORMAL_DESC0_OVT_WIDTH		16
#define RX_NORMAL_DESC2_HL_INDEX		0
#define RX_NORMAL_DESC2_HL_WIDTH		10
#define RX_NORMAL_DESC3_CDA_INDEX		27
#define RX_NORMAL_DESC3_CDA_WIDTH		1
#define RX_NORMAL_DESC3_CTXT_INDEX		30
#define RX_NORMAL_DESC3_CTXT_WIDTH		1
#define RX_NORMAL_DESC3_ES_INDEX		15
#define RX_NORMAL_DESC3_ES_WIDTH		1
#define RX_NORMAL_DESC3_ETLT_INDEX		16
#define RX_NORMAL_DESC3_ETLT_WIDTH		4
#define RX_NORMAL_DESC3_FD_INDEX		29
#define RX_NORMAL_DESC3_FD_WIDTH		1
#define RX_NORMAL_DESC3_INTE_INDEX		30
#define RX_NORMAL_DESC3_INTE_WIDTH		1
#define RX_NORMAL_DESC3_L34T_INDEX		20
#define RX_NORMAL_DESC3_L34T_WIDTH		4
#define RX_NORMAL_DESC3_LD_INDEX		28
#define RX_NORMAL_DESC3_LD_WIDTH		1
#define RX_NORMAL_DESC3_OWN_INDEX		31
#define RX_NORMAL_DESC3_OWN_WIDTH		1
#define RX_NORMAL_DESC3_PL_INDEX		0
#define RX_NORMAL_DESC3_PL_WIDTH		14
#define RX_NORMAL_DESC3_RSV_INDEX		26
#define RX_NORMAL_DESC3_RSV_WIDTH		1

#define RX_DESC3_L34T_IPV4_TCP			1
#define RX_DESC3_L34T_IPV4_UDP			2
#define RX_DESC3_L34T_IPV4_ICMP			3
#define RX_DESC3_L34T_IPV6_TCP			9
#define RX_DESC3_L34T_IPV6_UDP			10
#define RX_DESC3_L34T_IPV6_ICMP			11

#define RX_CONTEXT_DESC3_TSA_INDEX		4
#define RX_CONTEXT_DESC3_TSA_WIDTH		1
#define RX_CONTEXT_DESC3_TSD_INDEX		6
#define RX_CONTEXT_DESC3_TSD_WIDTH		1

#define TX_PACKET_ATTRIBUTES_CSUM_ENABLE_INDEX	0
#define TX_PACKET_ATTRIBUTES_CSUM_ENABLE_WIDTH	1
#define TX_PACKET_ATTRIBUTES_TSO_ENABLE_INDEX	1
#define TX_PACKET_ATTRIBUTES_TSO_ENABLE_WIDTH	1
#define TX_PACKET_ATTRIBUTES_VLAN_CTAG_INDEX	2
#define TX_PACKET_ATTRIBUTES_VLAN_CTAG_WIDTH	1
#define TX_PACKET_ATTRIBUTES_PTP_INDEX		3
#define TX_PACKET_ATTRIBUTES_PTP_WIDTH		1

#define TX_CONTEXT_DESC2_MSS_INDEX		0
#define TX_CONTEXT_DESC2_MSS_WIDTH		15
#define TX_CONTEXT_DESC3_CTXT_INDEX		30
#define TX_CONTEXT_DESC3_CTXT_WIDTH		1
#define TX_CONTEXT_DESC3_TCMSSV_INDEX		26
#define TX_CONTEXT_DESC3_TCMSSV_WIDTH		1
#define TX_CONTEXT_DESC3_VLTV_INDEX		16
#define TX_CONTEXT_DESC3_VLTV_WIDTH		1
#define TX_CONTEXT_DESC3_VT_INDEX		0
#define TX_CONTEXT_DESC3_VT_WIDTH		16

#define TX_NORMAL_DESC2_HL_B1L_INDEX		0
#define TX_NORMAL_DESC2_HL_B1L_WIDTH		14
#define TX_NORMAL_DESC2_IC_INDEX		31
#define TX_NORMAL_DESC2_IC_WIDTH		1
#define TX_NORMAL_DESC2_TTSE_INDEX		30
#define TX_NORMAL_DESC2_TTSE_WIDTH		1
#define TX_NORMAL_DESC2_VTIR_INDEX		14
#define TX_NORMAL_DESC2_VTIR_WIDTH		2
#define TX_NORMAL_DESC3_CIC_INDEX		16
#define TX_NORMAL_DESC3_CIC_WIDTH		2
#define TX_NORMAL_DESC3_CPC_INDEX		26
#define TX_NORMAL_DESC3_CPC_WIDTH		2
#define TX_NORMAL_DESC3_CTXT_INDEX		30
#define TX_NORMAL_DESC3_CTXT_WIDTH		1
#define TX_NORMAL_DESC3_FD_INDEX		29
#define TX_NORMAL_DESC3_FD_WIDTH		1
#define TX_NORMAL_DESC3_FL_INDEX		0
#define TX_NORMAL_DESC3_FL_WIDTH		15
#define TX_NORMAL_DESC3_LD_INDEX		28
#define TX_NORMAL_DESC3_LD_WIDTH		1
#define TX_NORMAL_DESC3_OWN_INDEX		31
#define TX_NORMAL_DESC3_OWN_WIDTH		1
#define TX_NORMAL_DESC3_TCPHDRLEN_INDEX		19
#define TX_NORMAL_DESC3_TCPHDRLEN_WIDTH		4
#define TX_NORMAL_DESC3_TCPPL_INDEX		0
#define TX_NORMAL_DESC3_TCPPL_WIDTH		18
#define TX_NORMAL_DESC3_TSE_INDEX		18
#define TX_NORMAL_DESC3_TSE_WIDTH		1

#define TX_NORMAL_DESC2_VLAN_INSERT		0x2

/* MDIO undefined or vendor specific registers */
#ifndef MDIO_AN_COMP_STAT
#define MDIO_AN_COMP_STAT		0x0030
#endif

/* Bit setting and getting macros
 *  The get macro will extract the current bit field value from within
 *  the variable
 *
 *  The set macro will clear the current bit field value within the
 *  variable and then set the bit field of the variable to the
 *  specified value
 */
#define GET_BITS(_var, _index, _width)					\
	(((_var) >> (_index)) & ((0x1 << (_width)) - 1))

#define SET_BITS(_var, _index, _width, _val)				\
do {									\
	(_var) &= ~(((0x1 << (_width)) - 1) << (_index));		\
	(_var) |= (((_val) & ((0x1 << (_width)) - 1)) << (_index));	\
} while (0)

#define GET_BITS_LE(_var, _index, _width)				\
	((le32_to_cpu((_var)) >> (_index)) & ((0x1 << (_width)) - 1))

#define SET_BITS_LE(_var, _index, _width, _val)				\
do {									\
	(_var) &= cpu_to_le32(~(((0x1 << (_width)) - 1) << (_index)));	\
	(_var) |= cpu_to_le32((((_val) &				\
			      ((0x1 << (_width)) - 1)) << (_index)));	\
} while (0)

/* Bit setting and getting macros based on register fields
 *  The get macro uses the bit field definitions formed using the input
 *  names to extract the current bit field value from within the
 *  variable
 *
 *  The set macro uses the bit field definitions formed using the input
 *  names to set the bit field of the variable to the specified value
 */
#define XGMAC_GET_BITS(_var, _prefix, _field)				\
	GET_BITS((_var),						\
		 _prefix##_##_field##_INDEX,				\
		 _prefix##_##_field##_WIDTH)

#define XGMAC_SET_BITS(_var, _prefix, _field, _val)			\
	SET_BITS((_var),						\
		 _prefix##_##_field##_INDEX,				\
		 _prefix##_##_field##_WIDTH, (_val))

#define XGMAC_GET_BITS_LE(_var, _prefix, _field)			\
	GET_BITS_LE((_var),						\
		 _prefix##_##_field##_INDEX,				\
		 _prefix##_##_field##_WIDTH)

#define XGMAC_SET_BITS_LE(_var, _prefix, _field, _val)			\
	SET_BITS_LE((_var),						\
		 _prefix##_##_field##_INDEX,				\
		 _prefix##_##_field##_WIDTH, (_val))

/* Macros for reading or writing registers
 *  The ioread macros will get bit fields or full values using the
 *  register definitions formed using the input names
 *
 *  The iowrite macros will set bit fields or full values using the
 *  register definitions formed using the input names
 */
#define XGMAC_IOREAD(_pdata, _reg)					\
	ioread32((_pdata)->xgmac_regs + _reg)

#define XGMAC_IOREAD_BITS(_pdata, _reg, _field)				\
	GET_BITS(XGMAC_IOREAD((_pdata), _reg),				\
		 _reg##_##_field##_INDEX,				\
		 _reg##_##_field##_WIDTH)

#define XGMAC_IOWRITE(_pdata, _reg, _val)				\
	iowrite32((_val), (_pdata)->xgmac_regs + _reg)

#define XGMAC_IOWRITE_BITS(_pdata, _reg, _field, _val)			\
do {									\
	u32 reg_val = XGMAC_IOREAD((_pdata), _reg);			\
	SET_BITS(reg_val,						\
		 _reg##_##_field##_INDEX,				\
		 _reg##_##_field##_WIDTH, (_val));			\
	XGMAC_IOWRITE((_pdata), _reg, reg_val);				\
} while (0)

/* Macros for reading or writing MTL queue or traffic class registers
 *  Similar to the standard read and write macros except that the
 *  base register value is calculated by the queue or traffic class number
 */
#define XGMAC_MTL_IOREAD(_pdata, _n, _reg)				\
	ioread32((_pdata)->xgmac_regs +					\
		 MTL_Q_BASE + ((_n) * MTL_Q_INC) + _reg)

#define XGMAC_MTL_IOREAD_BITS(_pdata, _n, _reg, _field)			\
	GET_BITS(XGMAC_MTL_IOREAD((_pdata), (_n), _reg),		\
		 _reg##_##_field##_INDEX,				\
		 _reg##_##_field##_WIDTH)

#define XGMAC_MTL_IOWRITE(_pdata, _n, _reg, _val)			\
	iowrite32((_val), (_pdata)->xgmac_regs +			\
		  MTL_Q_BASE + ((_n) * MTL_Q_INC) + _reg)

#define XGMAC_MTL_IOWRITE_BITS(_pdata, _n, _reg, _field, _val)		\
do {									\
	u32 reg_val = XGMAC_MTL_IOREAD((_pdata), (_n), _reg);		\
	SET_BITS(reg_val,						\
		 _reg##_##_field##_INDEX,				\
		 _reg##_##_field##_WIDTH, (_val));			\
	XGMAC_MTL_IOWRITE((_pdata), (_n), _reg, reg_val);		\
} while (0)

/* Macros for reading or writing DMA channel registers
 *  Similar to the standard read and write macros except that the
 *  base register value is obtained from the ring
 */
#define XGMAC_DMA_IOREAD(_channel, _reg)				\
	ioread32((_channel)->dma_regs + _reg)

#define XGMAC_DMA_IOREAD_BITS(_channel, _reg, _field)			\
	GET_BITS(XGMAC_DMA_IOREAD((_channel), _reg),			\
		 _reg##_##_field##_INDEX,				\
		 _reg##_##_field##_WIDTH)

#define XGMAC_DMA_IOWRITE(_channel, _reg, _val)				\
	iowrite32((_val), (_channel)->dma_regs + _reg)

#define XGMAC_DMA_IOWRITE_BITS(_channel, _reg, _field, _val)		\
do {									\
	u32 reg_val = XGMAC_DMA_IOREAD((_channel), _reg);		\
	SET_BITS(reg_val,						\
		 _reg##_##_field##_INDEX,				\
		 _reg##_##_field##_WIDTH, (_val));			\
	XGMAC_DMA_IOWRITE((_channel), _reg, reg_val);			\
} while (0)

/* Macros for building, reading or writing register values or bits
 * within the register values of XPCS registers.
 */
#define XPCS_IOWRITE(_pdata, _off, _val)				\
	iowrite32(_val, (_pdata)->xpcs_regs + (_off))

#define XPCS_IOREAD(_pdata, _off)					\
	ioread32((_pdata)->xpcs_regs + (_off))

/* Macros for building, reading or writing register values or bits
 * using MDIO.  Different from above because of the use of standardized
 * Linux include values.  No shifting is performed with the bit
 * operations, everything works on mask values.
 */
#define XMDIO_READ(_pdata, _mmd, _reg)					\
	((_pdata)->hw_if.read_mmd_regs((_pdata), 0,			\
		MII_ADDR_C45 | (_mmd << 16) | ((_reg) & 0xffff)))

#define XMDIO_READ_BITS(_pdata, _mmd, _reg, _mask)			\
	(XMDIO_READ((_pdata), _mmd, _reg) & _mask)

#define XMDIO_WRITE(_pdata, _mmd, _reg, _val)				\
	((_pdata)->hw_if.write_mmd_regs((_pdata), 0,			\
		MII_ADDR_C45 | (_mmd << 16) | ((_reg) & 0xffff), (_val)))

#define XMDIO_WRITE_BITS(_pdata, _mmd, _reg, _mask, _val)		\
do {									\
	u32 mmd_val = XMDIO_READ((_pdata), _mmd, _reg);			\
	mmd_val &= ~_mask;						\
	mmd_val |= (_val);						\
	XMDIO_WRITE((_pdata), _mmd, _reg, mmd_val);			\
} while (0)

/*
 * TEST_XGMAC_READ macro returns 0 if the reg register can be read.
 */
/* xPSC access */
#define XGMAC_XPCS_BASE				0xbf05d000
#define XGMAC_BASE				0xbf054000

#define APB_ADDR				(XGMAC_XPCS_BASE + (0xFF << 2))

#define DEVICE_PMA				0x1
#define DEVICE_PCS				0x3
#define DEVICE_AN				0x7
#define DEVICE_Vendor_Specific			0x1E

#define SR_XS_or_PCS_MMD_Control1		0x0
#define SR_PCS_Control2				0x7
#define VR_XS_PMA_MII_Ent_Gen5_Gen_Ctl		0x809C
#define VR_XS_or_PCS_MMD_Digital_Ctl1		0x8000
#define SR_XS_or_PCS_MMD_Status1		0x1
#define SR_AN_MMD_Control			0x0
#define SR_PMA_MMD_Control1			0x0

#define SR_XS_or_PCS_MMD_Control1_RST			(1 << 15)
#define SR_PCS_Control2_PCS_TYPE_SEL_MASK		0x3
#define SR_PCS_Control2_PCS_TYPE_SEL_KX4		0x1
#define SR_PCS_Control2_PCS_TYPE_SEL_KR			0x0
#define VR_XS_PMA_MII_Ent_Gen5_Gen_Ctl_LANE_MODE_MASK	0xF
#define VR_XS_PMA_MII_Ent_Gen5_Gen_Ctl_LANE_MODE_KX4	4
#define VR_XS_PMA_MII_Ent_Gen5_Gen_Ctl_LINK_WIDTH_MASK	0x7
#define VR_XS_PMA_MII_Ent_Gen5_Gen_Ctl_LINK_WIDTH_SHIFT	8
#define VR_XS_PMA_MII_Ent_Gen5_Gen_Ctl_LINK_WIDTH_4	2
#define VR_XS_or_PCS_MMD_Digital_Ctl1_VR_RST		(1 << 15)
#define SR_XS_or_PCS_MMD_Status1_RLU			(1 << 2)
#define SR_AN_MMD_Control_AN_EN				(1 << 12)
#define SR_PMA_MMD_Control1_LB				(1 << 0)

#define FULL_XPCS_ADDR(device, offset)		((device << 16) | offset)
#define XPCS_ADDR_LOW_MASK			0xFF

#define SR_Ctl_MMD_PMA_Dev_Id_1			0x0000
#define SR_Ctl_MMD_PMA_Dev_Id_2			0x0001
#define SR_Ctl_MMD_Dev_Id_1			0x0002
#define SR_Ctl_MMD_Dev_Id_2			0x0003
#define SR_Ctl_MMD_PCS_Dev_Id_1			0x0004
#define SR_Ctl_MMD_PCS_Dev_Id_2			0x0005
#define SR_Ctl_MMD_AN_Dev_Id_1			0x0006
#define SR_Ctl_MMD_AN_Dev_Id_2			0x0007
#define SR_Ctl_MMD_Package_Id_1			0x00E
#define SR_Ctl_MMD_Package_Id_2			0x00F

#define SR_Ctl_MMD_REG_1_VAL			0xa294
#define SR_Ctl_MMD_PMA_Dev_Id_2_VAL		0x4c00
#define SR_Ctl_MMD_Dev_Id_2_VAL 		0x4c11
#define SR_Ctl_MMD_PCS_Dev_Id_2_VAL 		0x4c22
#define SR_Ctl_MMD_AN_Dev_Id_2_VAL 		0x4c33
#define SR_Ctl_MMD_Package_Id_2_VAL		0x4c44


#define READ_MEMORY_REG(r)       (*((volatile uint32_t *) (r)))
#define WRITE_MEMORY_REG(r, v)   (*((volatile uint32_t *) (r)) = v)

#define READ_XPCS_REG(device, offset)	({	\
		WRITE_MEMORY_REG(APB_ADDR, FULL_XPCS_ADDR(device, offset) >> 8); \
		READ_MEMORY_REG(((FULL_XPCS_ADDR(device, offset) & XPCS_ADDR_LOW_MASK) << 2) + XGMAC_XPCS_BASE );})

#define WRITE_XPCS_REG(device, offset, val)	({	\
		WRITE_MEMORY_REG(APB_ADDR, FULL_XPCS_ADDR(device, offset) >> 8); \
		WRITE_MEMORY_REG(((FULL_XPCS_ADDR(device, offset) & XPCS_ADDR_LOW_MASK) << 2) + XGMAC_XPCS_BASE, val);})

#define MAX_POLLING_TIMES 100000

/* DMA Registers Map */
#define DMA_Mode				(XGMAC_BASE + 0x3000)
#define DMA_SysBus_Mode				(XGMAC_BASE + 0x3004)
#define DMA_Interrupt_Status			(XGMAC_BASE + 0x3008)
#define AXI_ARCache_Control			(XGMAC_BASE + 0x3010)
#define AXI_AWCache_Control			(XGMAC_BASE + 0x3018)
#define DMA_Debug_Status0			(XGMAC_BASE + 0x3020)
#define DMA_Debug_Status1			(XGMAC_BASE + 0x3024)
#define DMA_Debug_Status2			(XGMAC_BASE + 0x3028)
#define DMA_Debug_Status3			(XGMAC_BASE + 0x302C)
#define DMA_Debug_Status4			(XGMAC_BASE + 0x3030)

#define DMA_CH_OFFSET				0x80

/* Channel n Registers */
#define DMA_CH_Control(n)			(XGMAC_BASE + (n) * DMA_CH_OFFSET + 0x3100)
#define DMA_CH_Tx_Control(n)			(XGMAC_BASE + (n) * DMA_CH_OFFSET + 0x3104)
#define DMA_CH_Rx_Control(n)			(XGMAC_BASE + (n) * DMA_CH_OFFSET + 0x3108)
#define DMA_CH_TxDesc_List_HAddress(n)		(XGMAC_BASE + (n) * DMA_CH_OFFSET + 0x3110)
#define DMA_CH_TxDesc_List_LAddress(n)		(XGMAC_BASE + (n) * DMA_CH_OFFSET + 0x3114)
#define DMA_CH_RxDesc_List_HAddress(n)		(XGMAC_BASE + (n) * DMA_CH_OFFSET + 0x3118)
#define DMA_CH_RxDesc_List_LAddress(n)		(XGMAC_BASE + (n) * DMA_CH_OFFSET + 0x311C)
#define DMA_CH_TxDesc_Tail_LPointer(n)		(XGMAC_BASE + (n) * DMA_CH_OFFSET + 0x3124)
#define DMA_CH_RxDesc_Tail_LPointer(n)		(XGMAC_BASE + (n) * DMA_CH_OFFSET + 0x312C)
#define DMA_CH_TxDesc_Ring_Length(n)		(XGMAC_BASE + (n) * DMA_CH_OFFSET + 0x3130)
#define DMA_CH_RxDesc_Ring_Length(n)		(XGMAC_BASE + (n) * DMA_CH_OFFSET + 0x3134)
#define DMA_CH_Interrupt_Enable(n)		(XGMAC_BASE + (n) * DMA_CH_OFFSET + 0x3138)
#define DMA_CH_Rx_Interrupt_Watchdog_Timer(n)	(XGMAC_BASE + (n) * DMA_CH_OFFSET + 0x313C)
#define DMA_CH_Current_App_TxDesc_L(n)		(XGMAC_BASE + (n) * DMA_CH_OFFSET + 0x3144)
#define DMA_CH_Current_App_RxDesc_L(n)		(XGMAC_BASE + (n) * DMA_CH_OFFSET + 0x314C)
#define DMA_CH_Current_App_TxBuffer_H(n)	(XGMAC_BASE + (n) * DMA_CH_OFFSET + 0x3150)
#define DMA_CH_Current_App_TxBuffer_L(n)	(XGMAC_BASE + (n) * DMA_CH_OFFSET + 0x3154)
#define DMA_CH_Current_App_RxBuffer_H(n)	(XGMAC_BASE + (n) * DMA_CH_OFFSET + 0x3158)
#define DMA_CH_Current_App_RxBuffer_L(n)	(XGMAC_BASE + (n) * DMA_CH_OFFSET + 0x315C)
#define DMA_CH_Status(n)			(XGMAC_BASE + (n) * DMA_CH_OFFSET + 0x3160)

/* MTL Registers Map */
#define MTL_Operation_Mode			(XGMAC_BASE + 0x1000)
#define MTL_Debug_Control			(XGMAC_BASE + 0x1008)
#define MTL_Debug_Status			(XGMAC_BASE + 0x100C)
#define MTL_FIFO_Debug_Data			(XGMAC_BASE + 0x1010)
#define MTL_Interrupt_Status			(XGMAC_BASE + 0x1020)
#define MTL_RxQ_DMA_Map0			(XGMAC_BASE + 0x1030)
#define MTL_RxQ_DMA_Map1			(XGMAC_BASE + 0x1034)
#define MTL_RxQ_DMA_Map2			(XGMAC_BASE + 0x1038)
#define MTL_TC_Prty_Map0			(XGMAC_BASE + 0x1040)
#define MTL_TC_Prty_Map1			(XGMAC_BASE + 0x1044)

#define MTL_Q_OFFSET				0x80
/* MTL Traffic Class/Queue n Registers */
#define MTL_TxQ_Operation_Mode(n)		(XGMAC_BASE + (n) * MTL_Q_OFFSET + 0x1100)
#define MTL_TxQ_Underflow(n)			(XGMAC_BASE + (n) * MTL_Q_OFFSET + 0x1104)
#define MTL_TxQ_Debug(n)			(XGMAC_BASE + (n) * MTL_Q_OFFSET + 0x1108)
#define MTL_TC_ETS_Control(n)			(XGMAC_BASE + (n) * MTL_Q_OFFSET + 0x1110)
#define MTL_TC_ETS_Status(n)			(XGMAC_BASE + (n) * MTL_Q_OFFSET + 0x1114)
#define MTL_TC_Quantum_Weight(n)		(XGMAC_BASE + (n) * MTL_Q_OFFSET + 0x1118)
#define MTL_RxQ_Operation_Mode(n)		(XGMAC_BASE + (n) * MTL_Q_OFFSET + 0x1140)
#define MTL_RxQ_Missed_Pkt_Overflow_Cnt(n)	(XGMAC_BASE + (n) * MTL_Q_OFFSET + 0x1144)
#define MTL_RxQ_Debug(n)			(XGMAC_BASE + (n) * MTL_Q_OFFSET + 0x1148)
#define MTL_RxQ_Control(n)			(XGMAC_BASE + (n) * MTL_Q_OFFSET + 0x114C)
#define MTL_RxQ_Flow_Control(n)			(XGMAC_BASE + (n) * MTL_Q_OFFSET + 0x1150)
#define MTL_Q_Interrupt_Enable(n)		(XGMAC_BASE + (n) * MTL_Q_OFFSET + 0x1170)
#define MTL_Q_Interrupt_Status(n)		(XGMAC_BASE + (n) * MTL_Q_OFFSET + 0x1174)

/* MAC Registers Map */
#define MAC_Tx_Configuration			(XGMAC_BASE + 0x0000)
#define MAC_Rx_Configuration			(XGMAC_BASE + 0x0004)
#define MAC_Packet_Filter			(XGMAC_BASE + 0x0008)
#define MAC_Watchdog_Timeout			(XGMAC_BASE + 0x000C)
#define MAC_Hash_Table_Reg0			(XGMAC_BASE + 0x0010)
#define MAC_Hash_Table_Reg1			(XGMAC_BASE + 0x0014)
#define MAC_Hash_Table_Reg2			(XGMAC_BASE + 0x0018)
#define MAC_Hash_Table_Reg3			(XGMAC_BASE + 0x001C)
#define MAC_Hash_Table_Reg4			(XGMAC_BASE + 0x0020)
#define MAC_Hash_Table_Reg5			(XGMAC_BASE + 0x0024)
#define MAC_Hash_Table_Reg6			(XGMAC_BASE + 0x0028)
#define MAC_Hash_Table_Reg7			(XGMAC_BASE + 0x002C)
#define MAC_VLAN_Tag				(XGMAC_BASE + 0x0050)
#define MAC_VLAN_Hash_Table			(XGMAC_BASE + 0x0058)
#define MAC_VLAN_Incl				(XGMAC_BASE + 0x0060)
#define MAC_Inner_VLAN_Incl			(XGMAC_BASE + 0x0064)
#define MAC_Rx_Eth_Type_Match			(XGMAC_BASE + 0x006C)
#define MAC_Q0_Tx_Flow_Ctrl			(XGMAC_BASE + 0x0070)
#define MAC_Q1_Tx_Flow_Ctrl			(XGMAC_BASE + 0x0074)
#define MAC_Q2_Tx_Flow_Ctrl			(XGMAC_BASE + 0x0078)
#define MAC_Q3_Tx_Flow_Ctrl			(XGMAC_BASE + 0x007C)
#define MAC_Q4_Tx_Flow_Ctrl			(XGMAC_BASE + 0x0080)
#define MAC_Q5_Tx_Flow_Ctrl			(XGMAC_BASE + 0x0084)
#define MAC_Q6_Tx_Flow_Ctrl			(XGMAC_BASE + 0x0088)
#define MAC_Q7_Tx_Flow_Ctrl			(XGMAC_BASE + 0x008C)
#define MAC_Rx_Flow_Ctrl			(XGMAC_BASE + 0x0090)
#define MAC_RxQ_Ctrl0				(XGMAC_BASE + 0x00A0)
#define MAC_RxQ_Ctrl1				(XGMAC_BASE + 0x00A4)
#define MAC_RxQ_Ctrl2				(XGMAC_BASE + 0x00A8)
#define MAC_RxQ_Ctrl3				(XGMAC_BASE + 0x00AC)
#define MAC_Interrupt_Status			(XGMAC_BASE + 0x00B0)
#define MAC_Interrupt_Enable			(XGMAC_BASE + 0x00B4)
#define MAC_RX_TX_Status			(XGMAC_BASE + 0x00B8)
#define MAC_PMT_Control_Status			(XGMAC_BASE + 0x00C0)
#define MAC_RWK_Packet_Filter			(XGMAC_BASE + 0x00C4)
#define MAC_LPI_Control_Status			(XGMAC_BASE + 0x00D0)
#define MAC_LPI_Timers_Control			(XGMAC_BASE + 0x00D4)
#define MAC_Version				(XGMAC_BASE + 0x0110)
#define MAC_Debug				(XGMAC_BASE + 0x0114)
#define MAC_HW_Feature0				(XGMAC_BASE + 0x011C)
#define MAC_HW_Feature1				(XGMAC_BASE + 0x0120)
#define MAC_HW_Feature2				(XGMAC_BASE + 0x0124)
#define MAC_GPIO_Control			(XGMAC_BASE + 0x0278)
#define MAC_GPIO_Status				(XGMAC_BASE + 0x027C)

/* MAC Address Registers */
#define MAC_Address_High(ch)			(XGMAC_BASE + 0x0300 + 8 * (ch))
#define MAC_Address_Low(ch)			(XGMAC_BASE + 0x0304 + 8 * (ch))

/* L3-L4 Function Registers */
#define MAC_L3_L4_Address_Control		(XGMAC_BASE + 0x0C00)
#define MAC_L3_L4_Data				(XGMAC_BASE + 0x0C04)
#define MAC_ARP_Address				(XGMAC_BASE + 0x0C10)
#define MAC_RSS_Control				(XGMAC_BASE + 0x0C80)
#define MAC_RSS_Address				(XGMAC_BASE + 0x0C88)
#define MAC_RSS_Data				(XGMAC_BASE + 0x0C8C)

/* IEEE 1588 Registers */
#define MAC_Timestamp_Control			(XGMAC_BASE + 0x0D00)
#define MAC_Sub_Second_Increment		(XGMAC_BASE + 0x0D04)
#define MAC_System_Time_Seconds			(XGMAC_BASE + 0x0D08)
#define MAC_System_Time_Nanoseconds		(XGMAC_BASE + 0x0D0C)
#define MAC_System_Time_Seconds_Update		(XGMAC_BASE + 0x0D10)
#define MAC_System_Time_Nanoseconds_Update	(XGMAC_BASE + 0x0D14)
#define MAC_Timestamp_Addend			(XGMAC_BASE + 0x0D18)
#define MAC_System_Time_Higher_Word_Seconds	(XGMAC_BASE + 0x0D1C)
#define MAC_Timestamp_Status			(XGMAC_BASE + 0x0D20)
#define MAC_TxTimestamp_Status_Nanoseconds	(XGMAC_BASE + 0x0D30)
#define MAC_TxTimestamp_Status_Seconds		(XGMAC_BASE + 0x0D34)
#define MAC_Auxiliary_Control			(XGMAC_BASE + 0x0D40)
#define MAC_Auxiliary_Timestamp_Nanoseconds	(XGMAC_BASE + 0x0D48)
#define MAC_Auxiliary_Timestamp_Seconds		(XGMAC_BASE + 0x0D4C)
#define MAC_Timestamp_Ingress_Asym_Corr		(XGMAC_BASE + 0x0D50)
#define MAC_Timestamp_Egress_Asym_Corr		(XGMAC_BASE + 0x0D54)
#define MAC_Timestamp_Ingress_Corr_Nanosecond	(XGMAC_BASE + 0x0D58)
#define MAC_Timestamp_Ingress_Corr_Subnanosecond	(XGMAC_BASE + 0x0D5C)
#define MAC_Timestamp_Egress_Corr_Nanosecond		(XGMAC_BASE + 0x0D60)
#define MAC_Timestamp_Egress_Corr_Subnanosecond		(XGMAC_BASE + 0x0D64)
#define MAC_PPS0_Target_Time_Seconds			(XGMAC_BASE + 0x0D80)
#define MAC_PPS0_Target_Time_Nanoseconds		(XGMAC_BASE + 0x0D84)
#define MAC_PPS0_Interval				(XGMAC_BASE + 0x0D88)
#define MAC_PPS0_Width					(XGMAC_BASE + 0x0D8C)
#define MAC_PPS1_Target_Time_Seconds			(XGMAC_BASE + 0x0D90)
#define MAC_PPS1_Target_Time_Nanoseconds		(XGMAC_BASE + 0x0D94)
#define MAC_PPS_Control					(XGMAC_BASE + 0x0D94)
#define MAC_PPS1_Interval				(XGMAC_BASE + 0x0D98)
#define MAC_PPS1_Width					(XGMAC_BASE + 0x0D9C)
#define MAC_PPS2_Target_Time_Seconds			(XGMAC_BASE + 0x0DA0)
#define MAC_PPS2_Target_Time_Nanoseconds		(XGMAC_BASE + 0x0DA4)
#define MAC_PPS2_Interval				(XGMAC_BASE + 0x0DA8)
#define MAC_PPS2_Width					(XGMAC_BASE + 0x0DAC)
#define MAC_PPS3_Target_Time_Seconds			(XGMAC_BASE + 0x0DB0)
#define MAC_PPS3_Target_Time_Nanoseconds		(XGMAC_BASE + 0x0DB4)
#define MAC_PPS3_Interval				(XGMAC_BASE + 0x0DB8)
#define MAC_PPS3_Width					(XGMAC_BASE + 0x0DBC)
#define MAC_PTO_Control					(XGMAC_BASE + 0x0DC0)
#define MAC_Source_Port_Identity0			(XGMAC_BASE + 0x0DC4)
#define MAC_Source_Port_Identity1			(XGMAC_BASE + 0x0DC8)
#define MAC_Source_Port_Identity2			(XGMAC_BASE + 0x0DCC)
#define MAC_Log_Message_Interval			(XGMAC_BASE + 0x0DD0)


/* Macros to read/write XGMAC registers. */
#define READ_XGMAC_REG(r)		READ_MEMORY_REG(r)
#define WRITE_XGMAC_REG(r, v)		WRITE_MEMORY_REG(r, v)

#endif
