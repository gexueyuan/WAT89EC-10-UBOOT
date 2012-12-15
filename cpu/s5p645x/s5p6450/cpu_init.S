#include <config.h>
#include <s5p6450.h>

	.globl mem_ctrl_asm_init
mem_ctrl_asm_init:

	ldr	r0, =ELFIN_DMC0_BASE
	ldr	r1, =0x71101008			
	str	r1, [r0, #PHYCONTROL0_OFFSET]
	ldr	r1, =0x000084F6			
	str	r1, [r0, #PHYCONTROL1_OFFSET]
	ldr	r1, =0x00000000			
	str	r1, [r0, #PHYCONTROL2_OFFSET]
	ldr	r1, =0x7110100A			
	str	r1, [r0, #PHYCONTROL0_OFFSET]
	ldr	r1, =0x7110100B				
	str	r1, [r0, #PHYCONTROL0_OFFSET]

	mov	r1, #0x10000
1:	subs	r1, r1, #1
	bne	1b

	ldr	r1, =0x0F002010				
	str	r1, [r0, #CONCONTROL_OFFSET]
	ldr	r1, =0x00202440				
	str	r1, [r0, #MEMCONTROL_OFFSET]
	ldr	r1, =0x20F01313				
	str	r1, [r0, #MEMCONFIG0_OFFSET]
	ldr	r1, =0x30F01313			
	str	r1, [r0, #MEMCONFIG1_OFFSET]
	ldr	r1, =0xF0000000				
	str	r1, [r0, #PRECHCONFIG_OFFSET]
	ldr	r1, =0xFFFF00FF				
	str	r1, [r0, #PWRDNCONFIG_OFFSET]

#if defined(CONFIG_CLK_667_166_83) 
	ldr     r1, =0x0000050E
        str     r1, [r0, #TIMINGAREF_OFFSET]
        ldr     r1, =0x16233288                
        str     r1, [r0, #TIMINGROW_OFFSET]
        ldr     r1, =0x23230204           
        str     r1, [r0, #TIMINGDATA_OFFSET]
        ldr     r1, =0x08C80232
        str     r1, [r0, #TIMINGPOWER_OFFSET]

#elif defined(CONFIG_CLK_533_133_66)

	ldr	r1, =0x0000040D				
	str	r1, [r0, #TIMINGAREF_OFFSET]
	ldr	r1, =0x16233288		
	str	r1, [r0, #TIMINGROW_OFFSET]
	ldr	r1, =0x23230204				
	str	r1, [r0, #TIMINGDATA_OFFSET]
	ldr	r1, =0x08C80232				
	str	r1, [r0, #TIMINGPOWER_OFFSET]
#endif

	mov	r1, #0x10000
1:	subs	r1, r1, #1
	bne	1b

/*****************************************************************/
	ldr	r1, =0x07000000				@DirectCmd	chip0 Deselect
	str	r1, [r0, #DIRECTCMD_OFFSET]	

	ldr	r1, =0x01000000				@DirectCmd	chip0 PALL
	str	r1, [r0, #DIRECTCMD_OFFSET]
	
	ldr	r1, =0x00020000				@DirectCmd	chip0 EMRS2
	str	r1, [r0, #DIRECTCMD_OFFSET]
	
	ldr	r1, =0x00030000				@DirectCmd	chip0 EMRS3
	str	r1, [r0, #DIRECTCMD_OFFSET]
	
	ldr	r1, =0x00010400				@DirectCmd	chip0 EMRS1 (MEM DLL on, DQS# disable)
	str	r1, [r0, #DIRECTCMD_OFFSET]
	
	ldr	r1, =0x00000532				@DirectCmd	chip0 MRS (MEM DLL reset) CL=4, BL=4
	str	r1, [r0, #DIRECTCMD_OFFSET]
	
	ldr	r1, =0x01000000				@DirectCmd	chip0 PALL
	str	r1, [r0, #DIRECTCMD_OFFSET]
	
	ldr	r1, =0x05000000				@DirectCmd	chip0 REFA
	str	r1, [r0, #DIRECTCMD_OFFSET]
	
	ldr	r1, =0x05000000				@DirectCmd	chip0 REFA
	str	r1, [r0, #DIRECTCMD_OFFSET]
	
	ldr	r1, =0x00000432				@DirectCmd	chip0 MRS (MEM DLL unreset)
	str	r1, [r0, #DIRECTCMD_OFFSET]
	
	ldr	r1, =0x00010780				@DirectCmd	chip0 EMRS1 (OCD default)
	str	r1, [r0, #DIRECTCMD_OFFSET]
	
	ldr	r1, =0x00010000				@DirectCmd	chip0 EMRS1 (OCD exit, Differential DQS Enable)
	str	r1, [r0, #DIRECTCMD_OFFSET]

/*****************************************************************/

	ldr	r1, =0x0F0020B0				
	str	r1, [r0, #CONCONTROL_OFFSET]

	mov	pc, lr

