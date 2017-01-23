/* 
 * Baikal-T SOC platform support code.
 *
 * Copyright (C) 2014  Baikal Electronics OJSC
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

#include <linux/init.h>
#include <linux/types.h>
#include <linux/interrupt.h>
#include <linux/sched.h>
#include <linux/irq.h>
#include <linux/irqchip.h>
#include <linux/of.h>
#include <linux/log2.h>

#include <asm/mips-boards/baikal.h> /* Base GIC and GCR addresses */
#include <asm/irq_cpu.h>			/* MIPS CPU IRQ subsystem init */
#include <asm/irq_regs.h>			/* Registers in IRQ mode */
#include <asm/setup.h>				/* Exception handlers */
#include <asm/rtlx.h>				/* Strange block */
#include <linux/irqchip/mips-gic.h>	/* GIC definitions */

#include "common.h"					/* Common Baikal definitions */

/* Perfomance counters support */
int get_c0_perfcount_int(void)
{
	return gic_get_c0_perfcount_int();
}
EXPORT_SYMBOL_GPL(get_c0_perfcount_int);

void __init arch_init_irq(void)
{
	/* Init complete with devicetree */
	if (of_have_populated_dt())
		irqchip_init();
}

asmlinkage void plat_irq_dispatch(void)
{
	/* Nothing to do here */
}
