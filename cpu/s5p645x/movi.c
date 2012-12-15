#include <common.h>
#include <s5p6450.h>

#include <movi.h>
#include <asm/io.h>
#include <regs.h>
#include <mmc.h>
#if defined(CONFIG_SECURE_BOOT)
#include <secure_boot.h>
#endif

extern raw_area_t raw_area_control;

typedef u32 (*copy_sd_mmc_to_mem) \
	(u32 start_block, u16 block_size, u32* dest_addr);


void movi_bl2_copy(void)
{
	unsigned long bl2_addr; 
	copy_sd_mmc_to_mem copy_bl2;

	if(CHIP_VER == 1)	
		bl2_addr = (0x00002360);
	else
		bl2_addr = (0x00001908);
       
	
	copy_bl2 = (copy_sd_mmc_to_mem)(bl2_addr);

	copy_bl2(MOVI_BL2_POS, MOVI_BL2_BLKCNT, CFG_PHY_UBOOT_BASE);

#if defined(CONFIG_SECURE_BOOT)
 unsigned int rv;
        /* do integrity check */
        rv = Check_Signature( (SB20_CONTEXT *)SECURE_BOOT_CONTEXT_ADDR,
                              (unsigned char *)(0x27e00000),(1024*512-256),
                              (unsigned char *)((0x27e00000)+(1024*512-256)), 256 );

        if (rv != 0) {
                while(1);
        }
       
#endif
}

void print_movi_bl2_info (void)
{
	printf("%d, %d, %d\n", MOVI_BL2_POS, MOVI_BL2_BLKCNT, MOVI_ENV_BLKCNT);
}

void movi_write_env(ulong addr)
{
	movi_write(raw_area_control.image[2].start_blk, 
		raw_area_control.image[2].used_blk, addr);
}

void movi_read_env(ulong addr)
{
	movi_read(raw_area_control.image[2].start_blk,
		raw_area_control.image[2].used_blk, addr);
}

void movi_write_bl1(ulong addr,member_t *image)
{
	int i;
	ulong checksum;
	ulong src;
	ulong tmp;

	src = addr;
	
	for(i = 0, checksum = 0;i < (14 * 1024) - 4;i++)
	{
		checksum += *(u8*)addr++;
	}

	tmp = *(ulong*)addr;
	*(ulong*)addr = checksum;
	
	movi_write(image[1].start_blk,image[1].used_blk, src);

	*(ulong*)addr = tmp;
}

