/*
 * clk-baikal.c - Baikal Electronics clock driver.
 *
 * Copyright (C) 2015  Baikal Electronics JSC
 * Dmitry Dunaev <dmitry.dunaev@baikalelectronics.ru>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */
#include <linux/module.h>
#include <linux/spinlock.h>
#include <linux/io.h>
#include <linux/of.h>
#include <linux/clkdev.h>
#include <linux/clk-provider.h>
#include <linux/of_address.h>
#include <linux/delay.h>
#include <asm/setup.h>

#define BE_MODULE_VERSION	"1.02"

#define BE_CLK_ENABLE_MASK 		(1 << 0)
#define BE_CLK_RESET_MASK 		(1 << 1)
#define BE_CLK_SET_MASK 		(1 << 2)
#define BE_CLK_BYPASS_MASK 		(1 << 30)
#define BE_CLK_LOCK_MASK 		(1 << 31)

#define BE_CLKR_SHFT			2
#define BE_DIV_SHFT				4
#define BE_CLKF_SHFT			8
#define BE_CLKOD_SHFT			21

#define BE_CLK_DIV_MAX_WIDTH	17
#define BE_CLK_DIV_MASK			(((1 << BE_CLK_DIV_MAX_WIDTH) - 1) \
 									<< BE_DIV_SHFT)

#define BE_RD_CLKR(src)			(((src) & 0x000000fc) >> BE_CLKR_SHFT)
#define BE_RD_CLKF(src)			(((src) & 0x001FFF00) >> BE_CLKF_SHFT)
#define BE_RD_CLKOD(src)		(((src) & 0x01E00000) >> BE_CLKOD_SHFT)

/*
 * Common functions
 */
static DEFINE_SPINLOCK(clk_lock);

static inline unsigned int be_clk_read(void *csr)
{
        return readl_relaxed(csr);
}

static inline void be_clk_write(unsigned int data, void *csr)
{
        return writel_relaxed(data, csr);
}

struct be_clk_pll_params {
	int nr;
	int nf;
	int od;
};

struct be_clk_pll {
	struct clk_hw   hw;
	const char      *name;
	void __iomem    *reg;
	spinlock_t      *lock;
	struct be_clk_pll_params params;
};

#define to_be_clk_pll(_hw) container_of(_hw, struct be_clk_pll, hw)

static int be_clk_pll_is_enabled(struct clk_hw *hw)
{
	struct be_clk_pll *pllclk = to_be_clk_pll(hw);
	unsigned int data;
	/* Read pll ctrl reg */
	data = be_clk_read(pllclk->reg);
	/* Debug info */
	pr_debug("%s pll %sabled\n", pllclk->name,
			data & BE_CLK_ENABLE_MASK ? "di" : "en");
	/* Check enable bit */
	return data & BE_CLK_ENABLE_MASK ? 0 : 1;
}

static unsigned long be_clk_pll_recalc_rate(struct clk_hw *hw,
                                unsigned long parent_rate)
{
	struct be_clk_pll *pllclk = to_be_clk_pll(hw);
	unsigned long fref;
	unsigned long fout;
	unsigned int pll;
	unsigned int nr;
	unsigned int nf;
	unsigned int od;
	/* Read pll ctrl reg */
	pll = be_clk_read(pllclk->reg);
	/* Fetch pll parameters */
	nr = BE_RD_CLKR(pll) + 1;
	nf = BE_RD_CLKF(pll) + 1;
	od = BE_RD_CLKOD(pll) + 1;
	/* Ref dividers */
	fref = parent_rate / nr / od;
	/* pll multiplier */
	fout = fref * nf;
	/* Debug info */
	pr_debug("%s pll recalc rate %ld parent %ld\n", pllclk->name,
		fout, parent_rate);
	/* Return freq */
	return fout;
}

static void be_clk_pll_setup(struct clk_hw *hw)
{
	struct be_clk_pll *pllclk = to_be_clk_pll(hw);
	unsigned int pll;
	unsigned int clkr;
	unsigned int clkf;
	unsigned int clkod;
	/* Check if no parameters set */
	if ((pllclk->params.nr < 0) &&
		(pllclk->params.nf < 0) &&
		(pllclk->params.od < 0))
		return;
	/* Get current value */
	pll = be_clk_read(pllclk->reg);
	pr_debug("%s clock PADDR base 0x%08lX clk value 0x%08X\n",
		pllclk->name, __pa(pllclk->reg), pll);
	/* Clock rate */
	if (pllclk->params.nr > 0)
		clkr = pllclk->params.nr - 1;
	else
		clkr = BE_RD_CLKR(pll);
	/* Clock ref */
	if (pllclk->params.nf > 0)
		clkf = pllclk->params.nf - 1;
	else
		clkf = BE_RD_CLKF(pll);
	/* Post divider */
	if ((pllclk->params.od > 0) && ((pllclk->params.od % 1) == 0))
		clkod = pllclk->params.od - 1;
	else
		clkod = BE_RD_CLKOD(pll);
	/* First set new values */
	pll = BE_CLK_ENABLE_MASK | (clkr << BE_CLKR_SHFT) |
		(clkf << BE_CLKF_SHFT) | (clkod << BE_CLKOD_SHFT);
	/* Write new values */
	be_clk_write(pll, pllclk->reg);
	/* Debug info */
	pr_debug("%s clock PADDR base 0x%08lX clk value 0x%08X\n",
		pllclk->name, __pa(pllclk->reg), pll);
	/* Reset the PLL */
	pll |= BE_CLK_RESET_MASK;
	/* Perform reset */
	be_clk_write(pll, pllclk->reg);
	/* Wait for sync */
	do {
		udelay(100);
		pll = be_clk_read(pllclk->reg);
	} while (!(pll & BE_CLK_LOCK_MASK));
}

const struct clk_ops be_clk_pll_ops = {
		.is_enabled = be_clk_pll_is_enabled,
		.recalc_rate = be_clk_pll_recalc_rate,
};

static struct clk *be_register_clk_pll(struct device *dev,
		const char *name, const char *parent_name, unsigned long flags,
		struct be_clk_pll_params *params, void __iomem *reg, spinlock_t *lock)
{
	struct be_clk_pll *pmuclk;
	struct clk *clk;
	struct clk_init_data init;
	/* allocate the APM clock structure */
	pmuclk = kzalloc(sizeof(*pmuclk), GFP_KERNEL);
	if (!pmuclk) {
		/* Error */
		pr_err("%s: could not allocate PMU clk\n", __func__);
		return ERR_PTR(-ENOMEM);
	}
	/* Set clock init parameters */
	init.name = name;
	init.ops = &be_clk_pll_ops;
	init.flags = flags;
	init.parent_names = parent_name ? &parent_name : NULL;
	init.num_parents = parent_name ? 1 : 0;
	/* Baikal pll parameters */
	pmuclk->name = name;
	pmuclk->reg = reg;
	pmuclk->lock = lock;
	pmuclk->hw.init = &init;
	pmuclk->params = *params;
	/* Setup clock */
	be_clk_pll_setup(&pmuclk->hw);
	/* Register the clock */
	clk = clk_register(dev, &pmuclk->hw);
	if (IS_ERR(clk)) {
		/* Error */
		pr_err("%s: could not register clk %s\n", __func__, name);
		/* Free allocated memory */
		kfree(pmuclk);
		return NULL;
	}
	/* Return clock */
	return clk;
}

static void be_pllclk_init(struct device_node *np)
{
	const char *clk_name = np->full_name;
	struct clk *clk;
	void *reg;
	struct be_clk_pll_params params;
	/* Get initial parameters */
	if (of_property_read_u32(np, "be,pll-ref", &params.nr))
				params.nr = -1;
	if (of_property_read_u32(np, "be,pll-mul", &params.nf))
				params.nf = -1;
	if (of_property_read_u32(np, "be,pll-vco", &params.od))
				params.od = -1;
	/* Remap ctrl reg mem */
	reg = of_iomap(np, 0);
	if (reg == NULL) {
		/* Error */
		pr_err("Unable to map CSR register for %s\n", np->full_name);
		return;
	}
	/* Get clock name */
	of_property_read_string(np, "clock-output-names", &clk_name);
	/* Register pll clock */
	clk = be_register_clk_pll(NULL, clk_name,
						of_clk_get_parent_name(np, 0),
						CLK_IS_ROOT, &params, reg, &clk_lock);
	if (!IS_ERR(clk)) {
		/* Addd clock provider */
		of_clk_add_provider(np, of_clk_src_simple_get, clk);
		/* Register clock device */
		clk_register_clkdev(clk, clk_name, NULL);
		/* Debug info */
		pr_debug("Add %s clock PLL\n", clk_name);
	}
}

struct be_dev_params {
	unsigned int 	width;		/* Divider width */
	unsigned int    nobypass; 	/* Disable clock div=1 */
};

struct be_clk {
	struct clk_hw   hw;
	const char      *name;
	spinlock_t      *lock;
	void __iomem    *reg;
	struct be_dev_params params;
};

#define to_be_clk(_hw) container_of(_hw, struct be_clk, hw)

static int be_clk_enable(struct clk_hw *hw)
{
	struct be_clk *pclk = to_be_clk(hw);
	unsigned long flags = 0;
	unsigned int data;
	/* Lock clock */
	if (pclk->lock)
		spin_lock_irqsave(pclk->lock, flags);
	/* If clock valid */
	if (pclk->reg != NULL) {
		/* Debug info */
		pr_debug("%s clock enabled\n", pclk->name);
		/* Get CSR register */
		data = be_clk_read(pclk->reg);
		/* Enable the clock */
		data |= BE_CLK_ENABLE_MASK;
		/* Set CSR register */
		be_clk_write(data, pclk->reg);
		/* Debug info */
		pr_debug("%s clock PADDR base 0x%08lX clk value 0x%08X\n",
			pclk->name, __pa(pclk->reg), data);
	}
	/* Unlock clock */
	if (pclk->lock)
		spin_unlock_irqrestore(pclk->lock, flags);
	/* Return success */
	return 0;
}

static void be_clk_disable(struct clk_hw *hw)
{
	struct be_clk *pclk = to_be_clk(hw);
	unsigned long flags = 0;
	unsigned int data;
	/* Lock clock */
	if (pclk->lock)
		spin_lock_irqsave(pclk->lock, flags);
	/* If clock valid */
	if (pclk->reg != NULL) {
		/* Debug info */
		pr_debug("%s clock disabled\n", pclk->name);
		/* Get CSR register */
		data = be_clk_read(pclk->reg);
		/* Disable the clock */
		data &= ~BE_CLK_ENABLE_MASK;
		/* Set CSR register */
		be_clk_write(data, pclk->reg);
		/* Debug info */
		pr_debug("%s clock PADDR base 0x%08lX clk value 0x%08X\n",
			pclk->name, __pa(pclk->reg), data);
	}
	/* Unlock clock */
	if (pclk->lock)
		spin_unlock_irqrestore(pclk->lock, flags);
}

static int be_clk_is_enabled(struct clk_hw *hw)
{
	struct be_clk *pclk = to_be_clk(hw);
	unsigned int data = 0;

	/* If clock valid */
	if (pclk->reg != NULL) {
		/* Debug info */
		pr_debug("%s clock checking\n", pclk->name);
		/* Get CSR register */
		data = be_clk_read(pclk->reg);
		/* Debug info */
		pr_debug("%s clock PADDR base 0x%08lX clk value 0x%08X\n",
			pclk->name, __pa(pclk->reg), data);
		/* Debug info */
		pr_debug("%s clock is %sabled\n", pclk->name,
			data & BE_CLK_ENABLE_MASK ? "en" : "dis");
	}
	/* Enabled and not controlled */
	else
		return 1;
	return data & BE_CLK_ENABLE_MASK ? 1 : 0;
}

static unsigned long be_clk_recalc_rate(struct clk_hw *hw,
                                unsigned long parent_rate)
{
	struct be_clk *pclk = to_be_clk(hw);
	unsigned int data;

	/* If clock valid */
	if ((pclk->reg != NULL) &&
	    (pclk->params.width != 0)) {
		/* Get CSR register */
		data = be_clk_read(pclk->reg);
		/* Apply global mask and shift data */
		data = (data & BE_CLK_DIV_MASK) >> BE_DIV_SHFT;
		/* Apply divider width mask */
		data &= (1 << pclk->params.width) - 1;
		/* Debug info */
		pr_debug("%s clock recalc rate %ld parent %ld\n",
				pclk->name, parent_rate / data, parent_rate);
		return parent_rate / data;
	} else {
		pr_debug("%s clock recalc rate %ld parent %ld\n",
			pclk->name, parent_rate, parent_rate);
		return parent_rate;
	}
}

static int be_clk_set_rate(struct clk_hw *hw, unsigned long rate,
								unsigned long parent_rate)
{
	struct be_clk *pclk = to_be_clk(hw);
	unsigned long flags = 0;
	unsigned int data;
	unsigned int divider;
	/* Lock clock */
	if (pclk->lock)
		spin_lock_irqsave(pclk->lock, flags);
	/* If clock valid */
	if ((pclk->reg != NULL) &&
	    (pclk->params.width != 0)) {
		/* Let's compute the divider */
		if (rate > parent_rate)
			rate = parent_rate;
		/* Calc divider rounded down */
		divider = parent_rate / rate;
		/* Apply divider width mask */
		divider &= (1 << pclk->params.width) - 1;
		/* Why so may be ? */
		if (!divider)
			divider = 1;
		/* Check nobypass flag */
		if ((divider == 1) && pclk->params.nobypass)
			divider = 2;
		/* Get current state */
		data = be_clk_read(pclk->reg);
		/* Clear divide field */
		data &= ~BE_CLK_DIV_MASK;
		/* Set new divider */
		data |= divider << BE_DIV_SHFT;
		/* Set new value */
		be_clk_write(data, pclk->reg);
		/* Set restart pulse */
		data |= BE_CLK_SET_MASK;
		/* Restart divider */
		be_clk_write(data, pclk->reg);
		/* Debug info */
		pr_debug("%s clock set rate %ld\n", pclk->name,
				parent_rate / divider);
	} else {
		/* bypass mode */
		divider = 1;
	}
	/* Unlock clock */
	if (pclk->lock)
		spin_unlock_irqrestore(pclk->lock, flags);
	/* Return new rate */
	return parent_rate / divider;
}

static long be_clk_round_rate(struct clk_hw *hw, unsigned long rate,
                                unsigned long *prate)
{
	struct be_clk *pclk = to_be_clk(hw);
	unsigned long parent_rate = *prate;
	unsigned int divider;
	/* If clock valid */
	if (pclk->reg) {
		/* Let's compute the divider */
		if (rate > parent_rate)
			rate = parent_rate;
		/* Calc divider rounded down */
		divider = parent_rate / rate;
	} else {
		divider = 1;
	}
	/* Return actual freq */
	return parent_rate / divider;
}

const struct clk_ops be_clk_ops = {
        .enable = be_clk_enable,
        .disable = be_clk_disable,
        .is_enabled = be_clk_is_enabled,
        .recalc_rate = be_clk_recalc_rate,
        .set_rate = be_clk_set_rate,
        .round_rate = be_clk_round_rate,
};

static struct clk *be_register_clk(struct device *dev,
				const char *name, const char *parent_name,
				struct be_dev_params *params, void __iomem *reg,
				spinlock_t *lock)
{
	struct be_clk *pmuclk;
	struct clk *clk;
	struct clk_init_data init;
	int rc;

	/* Allocate the APM clock structure */
	pmuclk = kzalloc(sizeof(*pmuclk), GFP_KERNEL);
	if (!pmuclk) {
		/* Error */
		pr_err("%s: could not allocate PMU clk\n", __func__);
		return ERR_PTR(-ENOMEM);
	}
	/* Setup clock init structure */
	init.name = name;
	init.ops = &be_clk_ops;
	init.flags = 0;
	init.parent_names = parent_name ? &parent_name : NULL;
	init.num_parents = parent_name ? 1 : 0;
	/* Setup IP clock structure */
	pmuclk->reg = reg;
	pmuclk->name = name;
	pmuclk->lock = lock;
	pmuclk->hw.init = &init;
	pmuclk->params = *params;

	/* Register the clock */
	clk = clk_register(dev, &pmuclk->hw);
	if (IS_ERR(clk)) {
		/* Error */
		pr_err("%s: could not register clk %s\n", __func__, name);
		/* Free memory */
		kfree(pmuclk);
		return clk;
	}

	/* Register the clock for lookup */
	rc = clk_register_clkdev(clk, name, NULL);
	if (rc != 0) {
		/* Error */
		pr_err("%s: could not register lookup clk %s\n",
			__func__, name);
	}
	return clk;
}

static void __init be_devclk_init(struct device_node *np)
{
	const char *clk_name = np->full_name;
	struct clk *clk;
	struct be_dev_params params;
	void *reg;
	int rc;

	/* Check if the entry is disabled */
	if (!of_device_is_available(np))
		return;

	/* Remap ctrl reg mem */
	reg = of_iomap(np, 0);
	if (reg == NULL) {
		/* Error */
		pr_err("Unable to map CSR register for %s\n", np->full_name);
		return;
	}
	/* Check nobypass property */
	params.nobypass = of_property_read_bool(np, "nobypass");
	/* Get divider width */
	if (of_property_read_u32(np, "divider-width", &params.width))
				params.width = BE_CLK_DIV_MAX_WIDTH;
	/* Get clock name */
	of_property_read_string(np, "clock-output-names", &clk_name);
	/* Register clock */
	clk = be_register_clk(NULL, clk_name, of_clk_get_parent_name(np, 0),
						&params, reg, &clk_lock);
	/* Check error */
	if (IS_ERR(clk))
		goto err;
	/* Debug error */
	pr_debug("Add %s clock\n", clk_name);
	/* Add clock provider */
	rc = of_clk_add_provider(np, of_clk_src_simple_get, clk);
	if (rc != 0)
		pr_err("%s: could register provider clk %s\n", __func__,
				np->full_name);
	return;
err:
	if (reg)
		iounmap(reg);
}
CLK_OF_DECLARE(be_pll_clock, "be,pmu-pll-clock", be_pllclk_init);
CLK_OF_DECLARE(be_dev_clock, "be,pmu-device-clock", be_devclk_init);

MODULE_VERSION(BE_MODULE_VERSION);
MODULE_AUTHOR("Dmitry Dunaev");
MODULE_DESCRIPTION("Baikal Electronics clock driver");
MODULE_LICENSE("Proprietary");
