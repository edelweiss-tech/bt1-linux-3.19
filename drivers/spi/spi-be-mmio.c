/*
 * Memory-mapped interface driver for Baikal SOC SPI boot controller
 *
 * Based on drivers/spi/spi-dw-mmio.c
 *
 * Copyright (c) 2015, Baikal Electronics OJSC.
 *
 */

#include <linux/clk.h>
#include <linux/err.h>
#include <linux/interrupt.h>
#include <linux/platform_device.h>
#include <linux/slab.h>
#include <linux/spi/spi.h>
#include <linux/scatterlist.h>
#include <linux/module.h>
#include <linux/of.h>
#include <linux/of_platform.h>

#include "spi-be.h"

#define DRIVER_NAME "be_spi_mmio"

struct be_spi_mmio {
	struct be_spi  bes;
	struct clk     *clk;
};

static int be_spi_mmio_probe(struct platform_device *pdev)
{
	struct be_spi_mmio *besmmio;
	struct be_spi *bes;
	struct resource *mem;
	int ret;

	besmmio = devm_kzalloc(&pdev->dev, sizeof(struct be_spi_mmio),
			GFP_KERNEL);
	if (!besmmio)
		return -ENOMEM;

	bes = &besmmio->bes;

	/* Get basic io resource and map it */
	mem = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	if (!mem) {
		dev_err(&pdev->dev, "No control iomem resource\n");
		return -EINVAL;
	}

	bes->ctrl = devm_ioremap_resource(&pdev->dev, mem);
	if (IS_ERR(bes->ctrl)) {
		dev_err(&pdev->dev, "Control region map failed\n");
		return PTR_ERR(bes->ctrl);
	}

	pr_debug("bes->ctrl = %p\n", bes->ctrl);

	/* Get basic io resource and map it */
	mem = platform_get_resource(pdev, IORESOURCE_MEM, 1);
	if (!mem) {
		dev_err(&pdev->dev, "No SPI iomem resource?\n");
		return -EINVAL;
	}

	bes->regs = devm_ioremap_resource(&pdev->dev, mem);
	if (IS_ERR(bes->regs)) {
		dev_err(&pdev->dev, "SPI region map failed\n");
		return PTR_ERR(bes->regs);
	}

	pr_debug("bes->regs = %p\n", bes->regs);

	besmmio->clk = devm_clk_get(&pdev->dev, NULL);
	if (IS_ERR(besmmio->clk))
		return PTR_ERR(besmmio->clk);
	ret = clk_prepare_enable(besmmio->clk);
	if (ret)
		return ret;

	/* Dump boot controller device ID */
	ret = be_ctrl_readl(bes, BE_CTRL_DRID);
	dev_info(&pdev->dev, "Component ID: %08X\n", ret);

	/* Dump boot controller vendor ID */
	ret = be_ctrl_readl(bes, BE_CTRL_VID);
	dev_info(&pdev->dev, "Vendor ID: %08X\n", ret);

	bes->bus_num =  of_alias_get_id(pdev->dev.of_node, "ssi");
	if (bes->bus_num < 0)
		bes->bus_num = 0;

	bes->max_freq = clk_get_rate(besmmio->clk);
	bes->num_cs = 1;

	ret = be_spi_add_host(&pdev->dev, bes);
	if (ret)
		goto out;

	platform_set_drvdata(pdev, besmmio);
	return 0;

out:
	clk_disable_unprepare(besmmio->clk);
	return ret;
}

static int be_spi_mmio_remove(struct platform_device *pdev)
{
	struct be_spi_mmio *besmmio = platform_get_drvdata(pdev);

	clk_disable_unprepare(besmmio->clk);
	be_spi_remove_host(&besmmio->bes);
	/* Clean SPI IO remap region */
	iounmap(besmmio->bes.regs);
	/* Clean Control IO remap region */
	iounmap(besmmio->bes.ctrl);
	/* Free memory */
	kfree(besmmio);

	return 0;
}

static const struct of_device_id be_spi_mmio_of_match[] = {
	{ .compatible = "be,apb-boot", },
	{ /* end of table */}
};
MODULE_DEVICE_TABLE(of, be_spi_mmio_of_match);

static struct platform_driver be_spi_mmio_driver = {
	.probe		= be_spi_mmio_probe,
	.remove		= be_spi_mmio_remove,
	.driver		= {
		.name	= DRIVER_NAME,
		.owner	= THIS_MODULE,
		.of_match_table = be_spi_mmio_of_match,
	},
};
module_platform_driver(be_spi_mmio_driver);

MODULE_AUTHOR("Dmitry Dunaev <dmitry.dunaev@baikalelectronics.ru>");
MODULE_DESCRIPTION("Memory-mapped I/O interface driver for BE SPI boot controller");
MODULE_ALIAS("platform:be_spi_dt");
MODULE_LICENSE("Proprinetary");
