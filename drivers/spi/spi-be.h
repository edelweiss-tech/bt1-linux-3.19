#ifndef BE_SPI_HEADER_H
#define BE_SPI_HEADER_H

#include <linux/io.h>
#include <linux/scatterlist.h>
#include <asm/delay.h>

/* Control register offsets */
#define BE_CTRL_CSR			0x00
#define BE_CTRL_MAR			0x04
#define BE_CTRL_DRID		0x08
#define BE_CTRL_VID			0x0c

#define CTRL_BOOT_MODE 		((1 << 1) | (1 << 0))
#define CTRL_SPI_RDA 		(1 << 8)
#define CTRL_SRAM_BSAB		(1 << 0)

/* SPI register offsets */
#define BE_SPI_CTRL0			0x00
#define BE_SPI_CTRL1			0x04
#define BE_SPI_SSIENR			0x08
#define BE_SPI_MWCR			0x0c
#define BE_SPI_SER			0x10
#define BE_SPI_BAUDR			0x14
#define BE_SPI_TXFLTR			0x18
#define BE_SPI_RXFLTR			0x1c
#define BE_SPI_TXFLR			0x20
#define BE_SPI_RXFLR			0x24
#define BE_SPI_SR			0x28
#define BE_SPI_IMR			0x2c
#define BE_SPI_ISR			0x30
#define BE_SPI_RISR			0x34
#define BE_SPI_TXOICR			0x38
#define BE_SPI_RXOICR			0x3c
#define BE_SPI_RXUICR			0x40
#define BE_SPI_MSTICR			0x44
#define BE_SPI_ICR			0x48
#define BE_SPI_DMACR			0x4c
#define BE_SPI_DMATDLR			0x50
#define BE_SPI_DMARDLR			0x54
#define BE_SPI_IDR			0x58
#define BE_SPI_VERSION			0x5c
#define BE_SPI_DR			0x60

/* Bit fields in CTRLR0 */
#define SPI_DFS_OFFSET			0

#define SPI_FRF_OFFSET			4
#define SPI_FRF_SPI			0x0
#define SPI_FRF_SSP			0x1
#define SPI_FRF_MICROWIRE		0x2
#define SPI_FRF_RESV			0x3

#define SPI_MODE_OFFSET			6
#define SPI_SCPH_OFFSET			6
#define SPI_SCOL_OFFSET			7

#define SPI_TMOD_OFFSET			8
#define SPI_TMOD_MASK			(0x3 << SPI_TMOD_OFFSET)
#define	SPI_TMOD_TR			0x0		/* xmit & recv */
#define SPI_TMOD_TO			0x1		/* xmit only */
#define SPI_TMOD_RO			0x2		/* recv only */
#define SPI_TMOD_EPROMREAD		0x3		/* eeprom read mode */

#define SPI_SLVOE_OFFSET		10
#define SPI_SRL_OFFSET			11
#define SPI_CFS_OFFSET			12

/* Bit fields in SR, 7 bits */
#define SR_MASK				0x7f		/* cover 7 bits */
#define SR_BUSY				(1 << 0)
#define SR_TF_NOT_FULL			(1 << 1)
#define SR_TF_EMPT			(1 << 2)
#define SR_RF_NOT_EMPT			(1 << 3)
#define SR_RF_FULL			(1 << 4)
#define SR_TX_ERR			(1 << 5)
#define SR_DCOL				(1 << 6)

/* Bit fields in ISR, IMR, RISR, 7 bits */
#define SPI_INT_TXEI			(1 << 0)
#define SPI_INT_TXOI			(1 << 1)
#define SPI_INT_RXUI			(1 << 2)
#define SPI_INT_RXOI			(1 << 3)
#define SPI_INT_RXFI			(1 << 4)
#define SPI_INT_MSTI			(1 << 5)

/* TX RX interrupt level threshold, max can be 256 */
#define SPI_INT_THRESHOLD		32

enum be_ssi_type {
	SSI_MOTO_SPI = 0,
	SSI_TI_SSP,
	SSI_NS_MICROWIRE,
};

struct be_spi;

struct be_spi {
	struct spi_master	*master;
	struct spi_device	*cur_dev;
	enum be_ssi_type	type;
	char			name[16];

	void __iomem		*ctrl;
	void __iomem		*regs;
	unsigned long		paddr;
	int			irq;
	u32			fifo_len;	/* depth of the FIFO buffer */
	u32			max_freq;	/* max bus freq supported */

	u16			bus_num;
	u16			num_cs;		/* supported slave numbers */

	/* Message Transfer pump */
	struct tasklet_struct	pump_transfers;

	/* Current message transfer state info */
	struct spi_message	*cur_msg;
	struct spi_transfer	*cur_transfer;
	struct chip_data	*cur_chip;
	struct chip_data	*prev_chip;
	size_t			len;
	void			*tx;
	void			*tx_end;
	void			*rx;
	void			*rx_end;
	size_t			rx_map_len;
	size_t			tx_map_len;
	u8			n_bytes;	/* current is a 1/2 bytes op */
	u8			max_bits_per_word;	/* maxim is 16b */
	void			(*cs_control)(u32 command);
	/* Bus interface info */
	void			*priv;
#ifdef CONFIG_DEBUG_FS
	struct dentry *debugfs;
#endif
};

static inline u32 be_ctrl_readl(struct be_spi *bes, u32 offset)
{
	return __raw_readl(bes->ctrl + offset);
}

static inline void be_ctrl_writel(struct be_spi *bes, u32 offset, u32 val)
{
	__raw_writel(val, bes->ctrl + offset);
}

static inline int ctrl_boot_mode(struct be_spi *bes)
{
	return be_ctrl_readl(bes, BE_CTRL_CSR) & CTRL_BOOT_MODE;
}

static inline int ctrl_get_spi_mode(struct be_spi *bes)
{
	return (be_ctrl_readl(bes, BE_CTRL_CSR) & CTRL_SPI_RDA) ? 1: 0;
}

static inline void ctrl_set_spi_mode(struct be_spi *bes, int mode)
{
	be_ctrl_writel(bes, BE_CTRL_CSR, (mode ? CTRL_SPI_RDA : 0));
}

static inline void ctrl_release_sram(struct be_spi *bes)
{
	be_ctrl_writel(bes, BE_CTRL_MAR, 0);
}

static inline u32 be_readl(struct be_spi *bes, u32 offset)
{
	return __raw_readl(bes->regs + offset);
}

static inline void be_writel(struct be_spi *bes, u32 offset, u32 val)
{
	__raw_writel(val, bes->regs + offset);
}

static inline void spi_enable_chip(struct be_spi *bes, int enable)
{
	be_writel(bes, BE_SPI_SSIENR, (enable ? 1 : 0));
}

static inline void spi_set_clk(struct be_spi *bes, u16 div)
{
	be_writel(bes, BE_SPI_BAUDR, div);
}

static inline void spi_chip_sel(struct be_spi *bes, struct spi_device *spi,
		int active)
{
	u16 cs = spi->chip_select;

	if (bes->cs_control)
		bes->cs_control(active);

	pr_info("SPI: cs=%d active=%d\n", cs, active);

	if (active)
		be_writel(bes, BE_SPI_SER, 1 << cs);
}

/* Reset flash chip */
static inline void spi_chip_reset(struct be_spi *bes)
{
	u32 val = be_readl(bes, BE_SPI_SER);
	be_writel(bes, BE_SPI_SER, 0);
	udelay(100);
	be_writel(bes, BE_SPI_SER, val);
}

/* Disable IRQ bits */
static inline void spi_mask_intr(struct be_spi *bes, u32 mask)
{
	u32 new_mask;

	new_mask = be_readl(bes, BE_SPI_IMR) & ~mask;
	be_writel(bes, BE_SPI_IMR, new_mask);
}

/* Enable IRQ bits */
static inline void spi_umask_intr(struct be_spi *bes, u32 mask)
{
	u32 new_mask;

	new_mask = be_readl(bes, BE_SPI_IMR) | mask;
	be_writel(bes, BE_SPI_IMR, new_mask);
}

/*
 * Each SPI slave device to work with be_api controller should
 * has such a structure claiming its working mode (PIO/DMA etc),
 * which can be save in the "controller_data" member of the
 * struct spi_device
 */
struct be_spi_chip {
	u8 poll_mode;	/* 0 for contoller polling mode */
	u8 type;	/* SPI/SSP/Micrwire */
	void (*cs_control)(u32 command);
};

extern int be_spi_add_host(struct device *dev, struct be_spi *bes);
extern void be_spi_remove_host(struct be_spi *bes);
extern int be_spi_suspend_host(struct be_spi *bes);
extern int be_spi_resume_host(struct be_spi *bes);
extern void be_spi_xfer_done(struct be_spi *bes);

#endif /* BE_SPI_HEADER_H */
