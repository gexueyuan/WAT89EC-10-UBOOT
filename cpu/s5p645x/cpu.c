/*
 * (C) Copyright 2004 Texas Insturments
 *
 * (C) Copyright 2002
 * Sysgo Real-Time Solutions, GmbH <www.elinos.com>
 * Marius Groeger <mgroeger@sysgo.de>
 *
 * (C) Copyright 2002
 * Gary Jennejohn, DENX Software Engineering, <gj@denx.de>
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

/*
 * CPU specific code
 */

#include <common.h>
#include <command.h>
#include <regs.h>

#ifdef CONFIG_USE_IRQ
DECLARE_GLOBAL_DATA_PTR;
#endif
static int mmc_device_num;
static int boot_device_num;

/* read co-processor 15, register #1 (control register) */
static unsigned long read_p15_c1 (void)
{
	unsigned long value;

	__asm__ __volatile__(
						"mrc	p15, 0, %0, c1, c0, 0   @ read control reg\n"
						: "=r" (value)
						:
						: "memory");
	return value;
}

/* write to co-processor 15, register #1 (control register) */
static void write_p15_c1 (unsigned long value)
{
	__asm__ __volatile__(
						"mcr	p15, 0, %0, c1, c0, 0   @ write it back\n"
						:
						: "r" (value)
						: "memory");

	read_p15_c1 ();
}

static void cp_delay (void)
{
	volatile int i;

	/* Many OMAP regs need at least 2 nops  */
	for (i = 0; i < 100; i++);
}

/* See also ARM Ref. Man. */
#define C1_MMU		(1<<0)		/* mmu off/on */
#define C1_ALIGN	(1<<1)		/* alignment faults off/on */
#define C1_DC		(1<<2)		/* dcache off/on */
#define C1_WB		(1<<3)		/* merging write buffer on/off */
#define C1_BIG_ENDIAN	(1<<7)	/* big endian off/on */
#define C1_SYS_PROT	(1<<8)		/* system protection */
#define C1_ROM_PROT	(1<<9)		/* ROM protection */
#define C1_BRANCH	(1<<11)		/* branch prediction off/on */
#define C1_IC		(1<<12)		/* icache off/on */
#define C1_HIGH_VECTORS	(1<<13)	/* location of vectors: low/high addresses */
#define RESERVED_1	(0xf << 3)	/* must be 111b for R/W */

int cpu_init (void)
{
	/*
	 * setup up stacks if necessary
	 */
#ifdef CONFIG_USE_IRQ
	IRQ_STACK_START = _armboot_start - CFG_MALLOC_LEN - CFG_GBL_DATA_SIZE - 4;
	FIQ_STACK_START = IRQ_STACK_START - CONFIG_STACKSIZE_IRQ;
#endif
	return 0;
}

int cleanup_before_linux (void)
{
	/*
	 * this function is called just before we call linux
	 * it prepares the processor for linux
	 *
	 * we turn off caches etc ...
	 */

	unsigned long i;

	disable_interrupts ();

#ifdef CONFIG_LCD
	{
		extern void lcd_disable(void);
		extern void lcd_panel_disable(void);

		lcd_disable(); /* proper disable of lcd & panel */
		lcd_panel_disable();
	}
#endif

	/* turn off I/D-cache */
	asm ("mrc p15, 0, %0, c1, c0, 0":"=r" (i));
	i &= ~(C1_DC | C1_IC);
	asm ("mcr p15, 0, %0, c1, c0, 0": :"r" (i));

	/* flush I/D-cache */
	i = 0;
	asm ("mcr p15, 0, %0, c7, c7, 0": :"r" (i));  /* invalidate both caches and flush btb */
	asm ("mcr p15, 0, %0, c7, c10, 4": :"r" (i)); /* mem barrier to sync things */
	return(0);
}


/* * reset the cpu by setting up the watchdog timer and let him time out */
void reset_cpu(ulong ignored)
{
	printf("reset... \n\n\n");

#if defined(CONFIG_S5P6450)
	SW_RST_REG = 0x6450;
#endif

	/* loop forever and wait for reset to happen */
	while (1)
	{
		if (serial_tstc())
		{
			serial_getc();
			break;
		}
	}
	/*NOTREACHED*/
}

int do_reset (cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
	disable_interrupts ();
	reset_cpu (0);
	/*NOTREACHED*/
	return(0);
}

void icache_enable (void)
{
	ulong reg;

	reg = read_p15_c1 ();	/* get control reg. */
	cp_delay ();
	write_p15_c1 (reg | C1_IC);
}

void icache_disable (void)
{
	ulong reg;

	reg = read_p15_c1 ();
	cp_delay ();
	write_p15_c1 (reg & ~C1_IC);
}

int icache_status (void)
{
	return(read_p15_c1 () & C1_IC) != 0;
}

/* It makes no sense to use the dcache if the MMU is not enabled */
void dcache_enable (void)
{
	ulong reg;

	reg = read_p15_c1 ();
	cp_delay ();
	write_p15_c1 (reg | C1_DC);
}

void dcache_disable (void)
{
	ulong reg;

	reg = read_p15_c1 ();
	cp_delay ();
	reg &= ~C1_DC;
	write_p15_c1 (reg);
}

int dcache_status (void)
{
	return (read_p15_c1 () & C1_DC) != 0;
}

void branch_enable (void)
{
	ulong reg;

	reg = read_p15_c1 ();
	cp_delay ();
	write_p15_c1 (reg | C1_BRANCH);
}

void branch_disable (void)
{
	ulong reg;

	reg = read_p15_c1 ();
	cp_delay ();
	reg &= ~C1_BRANCH;
	write_p15_c1 (reg);
}

int branch_status (void)
{
	return (read_p15_c1 () & C1_BRANCH) != 0;
}

int set_active_device(int device_number)
{
	mmc_device_num = device_number;
}
int get_active_device()
{
	return mmc_device_num;
}
int get_boot_device()
{
	return boot_device_num;
}

int get_boot_config(){
	
	if(INF_REG3_REG == 0){
		return 1; //1 or 0
	}
	else if(INF_REG3_REG == 2){
		return 2;
	}
	else {//wrong 
#ifdef CFG_FASTBOOT_SDMMCBSP
		return -1;
#else
		return 1;
#endif
	}
}
/*
 * Initializes on-chip MMC controllers.
 * to override, implement board_mmc_init()
 */
int cpu_mmc_init(bd_t *bis)
{
	int ret;
	mmc_device_num = -1;
#ifdef CONFIG_S3C_HSMMC
	boot_device_num = get_boot_config();
	if(boot_device_num < 0){
		printf("\n ERR: Cannot get boot device!");	
		while(1);
	}
	printf("\n Booting from device no: %d\n",boot_device_num);
	mmc_device_num = boot_device_num;
	setup_hsmmc_clock();
	setup_hsmmc0_cfg_gpio();

	ret = smdk_s3c_hsmmc_init();
#ifdef CONFIG_EMMC_4_4
	 smdk_s5p_emmc_init();
#endif
	return ret;
#else
	return 0;
#endif
}

