
#include <common.h>
#include <s5p6450.h>

#include <movi.h>
#include <asm/io.h>
#include <regs.h>

#if defined(CONFIG_SECURE_BOOT)
#include <secure_boot.h>
#endif

#include "msh.h"


#define	MSH_BASE	0xED500000

#define ISRAM_ADDRESS	0xD0020000
#define BL1_ADDRESS	ISRAM_ADDRESS 

#define MSH_MPLL_SRC	55500000
#define BL1_SIZE 	0x2800
#define BL2_SIZE 	0x2800
#define BL_SIZE 	0x5000
#define _DRAM_BaseAddress 0x20000000

#define	MshOutp32(Offset, Data)		Outp32((MSH_BASE + Offset), Data)
#define	MshInp32(Offset)			Inp32((MSH_BASE + Offset))

#define ARMCLK_FREQ    47*1000*1000
#define HCLK166_FREQ   ARMCLK_FREQ/3
#define PCLK83_FREQ    HCLK166_FREQ/2
#define HCLK133_FREQ   ARMCLK_FREQ/3 
#define PCLK66_FREQ    HCLK133_FREQ/2

#define Outp32(addr, data) (*(volatile u32 *)(addr) = (data))
#define Outp16(addr, data) (*(volatile u16 *)(addr) = (data))
#define Outp8(addr, data)  (*(volatile u8 *)(addr) = (data))

#define Inp32(addr) ((*(volatile u32 *)(addr)))
#define Inp16(addr) ((*(volatile u16 *)(addr)))
#define Inp8(addr)  ((*(volatile u8 *)(addr)))
#define printf 

//static volatile MSH sMSH;
//static volatile u32 g_uIntFlag = 0;
//

//static volatile MSH *oMSH = &sMSH;
#define oMSH ((MSH*)0xd0024000)
void MSH_WaitForCardReady(void)
{
	while(1)
	{
		if(MshInp32(rMSH_STATUS) & (u32)(0x1<<9))
		{
			printf("\n Card is busy now ");
		}
		else
		{
			printf("\n Card is not busy now ");
			break;
		}
	}
}

void MSH_SetBlkSize(u32 uBlkSize)
{
printf("===%s\n",__func__);
	if(uBlkSize > 0xFFFF)
	{
		MshOutp32(rMSH_BLKSIZ, MSH_BlockLength);
	}
	else
	{
		MshOutp32(rMSH_BLKSIZ, uBlkSize);
	}
}

void MSH_SetBytCnt(u32 uBytCnt)
{
printf("===%s\n",__func__);
	MshOutp32(rMSH_BYTCNT, uBytCnt);
}

void MSH_ReadFromFIFO(u32 uTransferCount, void * uDstAddr)
{
printf("===%s\n",__func__);
	u32 uForLoop;
	volatile u32 * uBufAddr;
	uBufAddr = (u32*)uDstAddr;
	
	for(uForLoop=0; uForLoop< (uTransferCount); uForLoop++)
	{
		//Outp32( (u32 *)(uBufAddr++) , MshInp32(rMSH_DATA));
		*uBufAddr++ = MshInp32(rMSH_DATA);
	}

	oMSH->uBufferAddr = (u32) uBufAddr;
}


void MSH_ReadOperation(u32 uStBlock, u32 uNumofBlocks, void * uDstAddr)
{
	u32 uForLoop = 0;
	volatile u32 * uBufAddr;
	u32 uStartBlock;
	
	uBufAddr = uDstAddr;

	uStartBlock = uStBlock;

	// [CheckPoint]
	MSH_SetBytCnt( (MSH_BlockLength * uNumofBlocks));
	MSH_SetBlkSize(MSH_BlockLength);

	// 2010.05.07
	oMSH->uBufferAddr = (u32)uDstAddr;
	oMSH->uTotalTransferSize = ((MSH_BlockLength>>2)*uNumofBlocks);

	// Command!!!!
	MSH_ClearCmdReg();
	MSH_CmdArgument(uStartBlock);
	MSH_SetCmd_CmdOption(MSH_NoStopAbortCmd, MSH_WaitPrvCmd, MSH_SendAutoStop); // It is original~!
	MSH_SetCmd_RWOption(MSH_BlockTransfer, MSH_Read_from_Card, MSH_DataTransfer);
	MSH_SetCmd_RspOption(MSH_SkipRspCRC, MSH_ShortLength, MSH_ResponseExpected);
	if(uNumofBlocks == 1)
	{
		MSH_SetCmd_IssueCmd(17);
	}
	else if(uNumofBlocks > 1)
	{
		MSH_SetCmd_IssueCmd(18);
	}

	
	MSH_Wait4CmdDone();
	//MSH_WaitForRINTandClear(MSH_CmdDone);

	//MSH_ReadData(oMSH->uWMark_Receive, uNumofBlocks, (u32 *)oMSH->uBufferAddr);
	MSH_ReadData_Performance(oMSH->uWMark_Receive, uNumofBlocks, (u32 *)oMSH->uBufferAddr);


	MSH_Clear_RINTSTS(MSH_AutoCmdDone);

}

bool MSH_ReadData_Performance(u32 uRxWMark, u32 uNumofBlocks, void * uDstAddr)
{
	u32 uTime;
	u32 uTotalTime_NOP = 0;
	u32 uFlag_NOP;
	
	oMSH->uBufferAddr = (u32)uDstAddr;
	do
	{
		// occured RX Data Requeset~! 
		if ( (MshInp32(rMSH_RINTSTS) & MSH_RXDataReq) )
		{
			MSH_ReadFromFIFO(uRxWMark, (u32 *) oMSH->uBufferAddr);
			oMSH->uTotalTransferSize = oMSH->uTotalTransferSize - uRxWMark;
			MSH_Clear_RINTSTS(MSH_RXDataReq);
		}
	}
	while(oMSH->uTotalTransferSize > uRxWMark);

	// read Remained Data
	if ((oMSH->uTotalTransferSize < MSH_FIFO_SIZE) && (oMSH->uTotalTransferSize!=0))
	{
		MSH_ReadFromFIFO(oMSH->uTotalTransferSize, (u32 *) oMSH->uBufferAddr);
			
		oMSH->uOccurredInt = oMSH->uOccurredInt & (~(u32)(MSH_TransferOver));

		MSH_Clear_RINTSTS(MSH_RXDataReq); // clear RINT - Rx
		MSH_Clear_RINTSTS(MSH_TransferOver); // clear RINT - Transfer over

		////cprintf("\n rMSH_STATUS : %x", MshInp32(rMSH_STATUS));
	}
	return PASS;
}

#if 1
bool MSH_ReadData(u32 uRxWMark, u32 uNumofBlocks, void * uDstAddr)
{
	u32 uDelay;
	volatile u32 uNop;
	u32 uFreqMhz = 333;
	u32 uMicroSec = 10000; // 10ms

	u32 uForLoop = 0;
	//u32 * uBufAddr;
	//u32 uTotalSize;

	//oMSH->uBufferAddr = (u32)uDstAddr;
	//oMSH->uTotalTransferSize = ((MSH_BlockLength>>2)*uNumofBlocks);
	//uBufAddr = uDstAddr;
	//uTotalSize = ((MSH_BlockLength>>2)*uNumofBlocks);
	
	oMSH->uBufferAddr = (u32)uDstAddr;

		do
		{
			// occured RX Data Requeset~! 
			if ( (MshInp32(rMSH_RINTSTS) & MSH_RXDataReq) )
			{
				MSH_ReadFromFIFO(uRxWMark, (u32 *) oMSH->uBufferAddr);
				#if 0
				for(uForLoop=0; uForLoop< (uRxWMark); uForLoop++)
				{
					Outp32( (u32 *)(uBufAddr++) , MshInp32(rMSH_DATA));
				}
				#endif
				oMSH->uTotalTransferSize = oMSH->uTotalTransferSize - uRxWMark;
				MSH_Clear_RINTSTS(MSH_RXDataReq);
			}

			// wait for Card
			else 
			{
			}
		}
		while(oMSH->uTotalTransferSize > uRxWMark);


		//MSH_WaitForCardReady(); //2010.05.07

		// read Remained Data
		if ((oMSH->uTotalTransferSize < MSH_FIFO_SIZE) && (oMSH->uTotalTransferSize!=0))
		{
			// timeout
			for(uDelay = 0; uDelay < (uFreqMhz * uMicroSec); uDelay++)
			{

				if ( (MshInp32(rMSH_RINTSTS) & MSH_TransferOver) || (oMSH->uOccurredInt & MSH_TransferOver) == MSH_TransferOver)
				{

					MSH_ReadFromFIFO(oMSH->uTotalTransferSize, (u32 *) oMSH->uBufferAddr);
					
					oMSH->uOccurredInt = oMSH->uOccurredInt & (~(u32)(MSH_TransferOver));

					MSH_Clear_RINTSTS(MSH_RXDataReq); // clear RINT - Rx
					MSH_Clear_RINTSTS(MSH_TransferOver); // clear RINT - Transfer over

					while(1)
					{
						if(MshInp32(rMSH_STATUS) & (u32)(0x1<<9))
						{
							//cprintf("\n Ri");
						}
						else
						{
							break;
						}
					}
					return PASS;	
					
				}
							
				else
				{
				}
			}

			// Time Out!!!!!!
			MSH_AllClear_RINTSTS();
			return FAIL;

		}


		MSH_Clear_RINTSTS(MSH_RXDataReq); // clear RINT - Rx
		MSH_Clear_RINTSTS(MSH_TransferOver); // clear RINT - Transfer over

		//cprintf("\n rMSH_STATUS : %x", MshInp32(rMSH_STATUS));

		while(1)
		{

			if(MshInp32(rMSH_STATUS) & (u32)(0x1<<9))
			{
				//cprintf("\n Ro");
			}
			else
			{
				break;
			}
		}
		
		return PASS;
}

#endif



void MSH_AllClear_RINTSTS(void)
{
	volatile u32 uStatus;

	oMSH->uOccurredInt = 0;
	uStatus = MshInp32(rMSH_RINTSTS);
	MshOutp32(rMSH_RINTSTS, uStatus);
}

void MSH_Clear_RINTSTS(eMSH_INTERRUPT eIntSrc)
{
	//oMSH->uOccurredInt = oMSH->uOccurredInt & (~(u32)(eIntSrc));
	MshOutp32(rMSH_RINTSTS, eIntSrc);
}



bool MSH_Wait4Card2TransferState(void)
{
printf("===%s\n",__func__);
	volatile u32 uCardNum = 1;
	volatile u32 uStatus;
	do
	{
		MSH_ClearCmdReg();
	 	MSH_CmdArgument((uCardNum << 16));
		MSH_SetCmd_CmdOption(MSH_NoStopAbortCmd, MSH_SendCmdAtOnce, MSH_NotAutoStop);
		MSH_SetCmd_RWOption(MSH_BlockTransfer, MSH_Read_from_Card, MSH_NoDataTransfer);
		MSH_SetCmd_RspOption(MSH_SkipRspCRC, MSH_ShortLength, MSH_ResponseExpected);
		MSH_SetCmd_IssueCmd(13);

		MSH_Wait4CmdDone();

		uStatus = MSH_GetResponse(MSH_Response_0);
		uStatus = ((uStatus >> 9) & 0xf);
	}
	while(uStatus ==7);
	
	return (uStatus == 4)? PASS : FAIL;
}



/* ________________ [[ Wait for Command Done ]] ________________*/

bool MSH_Wait4CmdDone(void)
{

		// worst case
		if ( MSH_CmdDone == (oMSH->uOccurredInt & MSH_CmdDone) )
		{
			oMSH->uOccurredInt = oMSH->uOccurredInt & (~(u32)(MSH_CmdDone));
			MSH_Clear_RINTSTS(MSH_CmdDone);
			return PASS;
		}
		
		do
		{
			if ( (MshInp32(rMSH_RINTSTS) & (MSH_DataReadTimeOut | MSH_DataStarvation)) )
			{
				MSH_AllClear_RINTSTS(); //
				return FAIL;
			}
		}while(!(MshInp32(rMSH_RINTSTS) & MSH_CmdDone));

		MSH_Clear_RINTSTS(MSH_CmdDone);
		//MSH_AllClear_RINTSTS();
		return PASS;

	return PASS;
}



/* ________________ [[ Get Resopnse ]] ________________*/

u32 MSH_GetResponse(eMSH_Response eResponse)
{
	volatile u32 uConfigReg;
	uConfigReg = MshInp32(eResponse);
	return uConfigReg;	
}



// ###################################################### [ Issue Command Functions Area ] 

void MSH_ClearCmdReg(void)
{
	MshOutp32(rMSH_CMD, 0);
}

void MSH_CmdArgument(u32 uCmdArg)
{
	MshOutp32(rMSH_CMDARG, uCmdArg);
}


// [27 : 24]
void MSH_SetCmd_BootOption(eMSH_BootMode eBootMode, eMSH_ExpectBootAck eBootAck, eMSH_EnableBoot eEnableBoot)
{
printf("===%s\n",__func__);
	volatile u32 uConfigReg;

	uConfigReg = MshInp32(rMSH_CMD);
	uConfigReg = ( uConfigReg | eBootMode | eBootAck | eEnableBoot );

	MshOutp32(rMSH_CMD, uConfigReg);
}


// [14 : 12]
void MSH_SetCmd_CmdOption(eMSH_StopAbortCmd eStopAbortCmd, eMSH_WaitPrvCmd eWaitPrvCmd, eMSH_SendAutoStop eSendAutoStop)
{
printf("===%s\n",__func__);
	volatile u32 uConfigReg;

	uConfigReg = MshInp32(rMSH_CMD);
	uConfigReg = ( uConfigReg | eStopAbortCmd | eWaitPrvCmd | eSendAutoStop );

	MshOutp32(rMSH_CMD, uConfigReg);
}


// [11 : 09]
void MSH_SetCmd_RWOption(eMSH_TransMode eTransMode, eMSH_nRW eReaWrite, eMSH_DataExpected eDataExpected)
{
	volatile u32 uConfigReg;

	uConfigReg = MshInp32(rMSH_CMD);
	uConfigReg = ( uConfigReg | eTransMode | eReaWrite | eDataExpected );

	MshOutp32(rMSH_CMD, uConfigReg);
}


// [08 : 06]
void MSH_SetCmd_RspOption(eMSH_CheckRspCRC eCheckRspCRC, eMSH_RspLength eRspLength, eMSH_RspExpected eRspExpected)
{
	volatile u32 uConfigReg;

	uConfigReg = MshInp32(rMSH_CMD);
	uConfigReg = ( uConfigReg | eCheckRspCRC | eRspLength |eRspExpected );

	MshOutp32(rMSH_CMD, uConfigReg);
}


// [05:00]
bool MSH_SetCmd_IssueCmd(u32 CmdIndex)
{
	volatile u32 uConfigReg;

	MSH_AllClear_RINTSTS();

	uConfigReg = MshInp32(rMSH_CMD);
	uConfigReg = uConfigReg & (~(u32)(0x3F));
	uConfigReg = ( uConfigReg | MSH_start_cmd | MSH_NormalCmdSeq | (u8)(CmdIndex&0x3F) );
	MshOutp32(rMSH_CMD, uConfigReg);
		////cprintf("\n\n _____ Issued Command : CMD_%0d _____ \n", (u8)(CmdIndex&0x3F));

		do
		{
			if ( (MshInp32(rMSH_RINTSTS) & MSH_DataStarvation) )
			{
				MSH_AllClear_RINTSTS();
				return FAIL;
			}
		}while( (MshInp32(rMSH_CMD) & (MSH_start_cmd)) );

		return PASS;

}

void MSH_SetRegister(u32 uNameofReg, u32 uValue)
{
	MshOutp32(uNameofReg, uValue);
}

u32 MSH_GetRegister(u32 uNameofReg)
{
	return MshInp32(uNameofReg);
}


bool MSH_WaitForRINTandClear(eMSH_INTERRUPT eIntSrc)
{
        u32 uDelay;
        u32 uNop;
        u32 uFreqMhz = 533; //333
        u32 uMicroSec = 500000; // 50ms
        do
        {
	        for(uDelay = 0; uDelay < (uFreqMhz * uMicroSec); uDelay++)
                {
        	        if( MshInp32(rMSH_RINTSTS) & (u32)(eIntSrc) )
                        {
                	         MshOutp32(rMSH_RINTSTS, eIntSrc); // Clear RIN
                        	return PASS;
                        }
		}
                MSH_AllClear_RINTSTS();
                return FAIL;
	}while(!(MshInp32(rMSH_RINTSTS) & (u32)(eIntSrc)));

        MshOutp32(rMSH_RINTSTS, eIntSrc); // Clear RIN
        return PASS;
}



bool MSH_ReadFromFIFO_eMMC(u32 uRxWMark, u32 uNumofBlocks, void * uDstAddr)
{
        u32 uDelay;
        volatile u32 uNop;
        u32 uFreqMhz = 66;
        u32 uMicroSec = 10000; // 10ms
        u32 uForLoop;

        oMSH->uBufferAddr = (u32)uDstAddr;
        oMSH->uTotalTransferSize = ((MSH_BlockLength>>2)*uNumofBlocks);
        do
        {
                if ( (MshInp32(rMSH_RINTSTS) & MSH_RXDataReq) )
                {
                        MSH_ReadFromFIFO(uRxWMark, (u32 *) oMSH->uBufferAddr);
                        oMSH->uTotalTransferSize = oMSH->uTotalTransferSize - uRxWMark;
                        MSH_Clear_RINTSTS(MSH_RXDataReq);
                }
        }
        while(oMSH->uTotalTransferSize > uRxWMark);
        if ((oMSH->uTotalTransferSize <= uRxWMark) && (oMSH->uTotalTransferSize!=0))
        {
                MSH_ReadFromFIFO(oMSH->uTotalTransferSize, (u32 *) oMSH->uBufferAddr);
                MSH_Clear_RINTSTS(MSH_RXDataReq);
        }

        for(uDelay = 0; uDelay < (uFreqMhz * uMicroSec); uDelay++)
        {
                if(MshInp32(rMSH_STATUS) & (u32)(0x1<<9))
                {
                }
                else
                {
                        break;
                }
        }
        return true;
}



void MSH_EndBootOp_eMMC(void)
{

	volatile  u32 uTemp;

//Disable Boot Mode
        MSH_WaitForCardReady();
        MSH_SetBytCnt( 0 );
        MSH_ClearCmdReg();
        MSH_CmdArgument(0x0);
        MSH_SetCmd_BootOption(MSH_MandatoryBoot, MSH_NoBootAck, MSH_DisableBoot); // fixed argument
        MSH_SetCmd_CmdOption(MSH_NoStopAbortCmd, MSH_SendCmdAtOnce, MSH_NotAutoStop); // fixed argument
        MSH_SetCmd_RWOption(MSH_BlockTransfer, MSH_Read_from_Card, MSH_DataTransfer);
        MSH_SetCmd_RspOption(MSH_SkipRspCRC, MSH_ShortLength, MSH_NoResponse);
        MSH_SetCmd_IssueCmd(0);
        uTemp = MshInp32(rMSH_DATA);

//Delay is required for coming out of Boot mode
	uTemp = 0xffff;
	while(uTemp--);

}

void MSH_oSet_BusWidth(eMSH_BusWidth eBusWidth)
{
        oMSH->eBusWidth = eBusWidth;
}

void MSH_oSet_EventDrivenMode(eMSH_EventDrivenMode eEventMode)
{
        oMSH->eEventMode = eEventMode;
}


void MSH_oSet_UsingIDMAorNot(eMSH_UsingIDMAorNot eUsingIDMAorNot)
{
        oMSH->eUsingIDMAorNot = eUsingIDMAorNot;
}

void MSH_oSet_MSize(eMSH_MSize eMSize)
{
        oMSH->eMSize = eMSize;
}

void emmc_resume(void)
{
	volatile unsigned int x = 0xfff;

	if(INF_REG3_REG == 2){
		while(x--);
		MSH_EndBootOp_eMMC();
		x = 0xfff;
		while(x--);

		MSH_CmdArgument(0x0);
	        MSH_SetCmd_CmdOption(MSH_StopAbortCmd, MSH_SendCmdAtOnce, MSH_NotAutoStop);
	        MSH_SetCmd_RWOption(MSH_BlockTransfer, MSH_Read_from_Card, MSH_NoDataTransfer);
	        MSH_SetCmd_RspOption(MSH_SkipRspCRC, MSH_ShortLength, MSH_NoResponse);
        	MSH_SetCmd_IssueCmd(0);

		x = 0xfff;
		while(x--);
	}
}

void emmc_bl2_copy(void)
{
	volatile unsigned int x=0xfff,tmp = 0;
#if defined(CONFIG_SECURE_BOOT)
        unsigned int rv;
#endif

	while(x--);
	MSH_ReadFromFIFO_eMMC(MSH_RxWMark, 600, 0x27e00000);
	x=0xfff;
	while(x--);
	MSH_EndBootOp_eMMC();
	x = 0xfff;
	while(x--);
#if defined(CONFIG_SECURE_BOOT)
        /* do integrity check */
        rv = Check_Signature( (SB20_CONTEXT *)SECURE_BOOT_CONTEXT_ADDR,
                              (unsigned char *)(0x27e00000),(1024*300-256),
                              (unsigned char *)((0x27e00000)+(1024*300-256)), 256 );

        if (rv != 0) {
                while(1);
        }
	
	//while(1);

#endif


}

