/*
 *
 *
 */
#ifndef __ASM_MIPS_BOARDS_BAIKAL_H
#define __ASM_MIPS_BOARDS_BAIKAL_H

#include <asm/addrspace.h>
#include <asm/io.h>

/*
 * GCMP Specific definitions
 */
#define GCMP_BASE_ADDR			0x1fbf8000
#define GCMP_ADDRSPACE_SZ		(256 * 1024)

/*
 * GIC Specific definitions
 */
#define GIC_BASE_ADDR			0x1bdc0000
#define GIC_ADDRSPACE_SZ		(128 * 1024)

/*
 * CPC Specific definitions
 */
#define CPC_BASE_ADDR			0x1bde0000
#define CPC_ADDRSPACE_SZ		(24 * 1024)


#endif /* __ASM_MIPS_BOARDS_BAIKAL_H */
