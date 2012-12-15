#ifndef __msh_H__
#define __msh_H__

#ifdef __cplusplus
extern "C" 
{
#endif

#define S5P6450




typedef unsigned int        U32;
typedef unsigned short      U16;
typedef unsigned char       U8;
typedef signed   int        S32;
typedef signed   short      S16;
typedef signed   char       S8;

#ifndef __cplusplus
typedef unsigned char        bool;
#endif

#undef FALSE
#undef TRUE

#define FALSE 0
#define TRUE 1
#define false 0
#define true 1
#define PASS	1
#define FAIL	0




// ___________________________________________________________________________ [Register Map ]

#define		rMSH_CTRL			0x00	// Control 
#define		rMSH_PWREN		0x04	// Power Enable 

#define		rMSH_CLKDIV		0x08	// Clock Divider 
#define		rMSH_CLKSRC		0x0C	// Clock Source 
#define		rMSH_CLKENA		0x10	// Clock Enable 

#define		rMSH_TMOUT		0x14	// Time Out 

#define		rMSH_CTYPE			0x18	// Card Type
#define		rMSH_BLKSIZ		0x1C	// Block Size
#define		rMSH_BYTCNT		0x20	// Byte Count

#define		rMSH_INTMASK		0x24	// Interrupt Mask

#define		rMSH_CMDARG		0x28	// Command Argument
#define		rMSH_CMD			0x2C	// Command

#define		rMSH_RESP_0		0x30	// Response 0
#define		rMSH_RESP_1		0x34	// Response 1
#define		rMSH_RESP_2		0x38	// Response 2
#define		rMSH_RESP_3		0x3C	// Response 3

#define		rMSH_MINTSTS		0x40	// Masked interrupt status
#define		rMSH_RINTSTS		0x44 	// Raw interrupt status
#define		rMSH_STATUS		0x48	// Status

#define		rMSH_FIFOTH		0x4C	// FIFO threshold
#define		rMSH_CDETECT		0x50 	// Card Detect
#define		rMSH_WRTPRT		0x54	// Write Protect
#define		rMSH_GPIO			0x58	// GPIO

#define		rMSH_TCBCNT		0x5C	// Transferred CIU(Card Interface Unit) card byte count
#define		rMSH_TBBCNT		0x60	// Transferred host/DMA to/from BIU(Bus Interface Unit)-FIFO byte count

#define		rMSH_DEBNCE		0x64	// Card Detect Debounce

#define		rMSH_USRID			0x68	// User ID
#define		rMSH_VERID			0x6C	// IP version ID 

#define		rMSH_HCON			0x70	// Hardware Configuration

#define		rMSH_UHS_REG		0x74	//UHS-1

#define		rMSH_BMOD			0x80	// Bus Mode Register; controls the Host Interface Mode
#define		rMSH_PLDMND		0x84	// Poll Demand

#define		rMSH_DBADDR		0x88	// Descriptor List Base Address

#define		rMSH_IDSTS			0x8C	// Internal DMAC Status
#define		rMSH_IDINTEN		0x90	// Internal DMAC Interrupt Enable

#define		rMSH_DSCADDR		0x94	// Current Host Descriptor Address
#define		rMSH_BUFADDR		0x98	// Current Host Buffer Address

#define		rMSH_WAKEUPCON	0xA0	// Wake-up control register
#define		rMSH_CLOCKCON		0xA4	// Clock (delay) control register

#define		rMSH_DATA			0x100	// Data FIFO read/write

//''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''[Register Map ]




// ___________________________________________________________________________ [Define Area ]

#define		MSH_start_cmd				0x80000000	// CMD(Command Register, Addr Offset : 0x2C) [31]


#define		MSH_IdentifySpeed			400000		// 400kHz



/***** SD spec 2.0
 -> Default mode        : Variable clock rate 0 ~ 25MHz
 -> High-Speed mode : Variable clock rate 0 ~ 50MHz
*/
#define		MSH_NormalSpeed_SD		25000000	// 25MHz
#define		MSH_HighSpeed_SD			50000000	// 50MHz

/***** MMC 
 -> Version 4.0, 4.1, 4.2 Default mode : 0 ~ 20MHz
 -> Version 4.3 4.4         Default mode : 0 ~ 26MHz
 -> High-Speed mode (optional, depend on the spec) : 0 ~ 52MHz
*/
#define		MSH_NormalSpeed_MMC		20000000
#define		MSH_HighSpeed_MMC		52000000


#define		MSH_BlockLength			512
#define		MSH_FIFO_SIZE				32

#define		MSH_RxWMark_Limit			(MSH_FIFO_SIZE -2)
#define		MSH_TxWMark_Limit			(1)	// 4
#define		MSH_RxWMark				(MSH_FIFO_SIZE/2) //0x10
#define		MSH_TxWMark				(MSH_FIFO_SIZE/2) //0x10

//#define	IDMAC_MaxSize				15 // It is real.
//#define	MSH_IDMAC_5kBytesSize			10 
#define		MSH_IDMAC_4kBytesSize			8 // reorganize for easyly


#if defined( S5PC200 ) 
//temp
#define		MSH_uDES_A_0				0x41000000
#define		MSH_uDES_A_1				0x41000004
#define		MSH_uDES_A_2				0x41000008
#define		MSH_uDES_A_3				0x4100000C
#define		MSH_uDES_B_0				0x41000010
#define		MSH_uDES_B_1				0x41000014
#define		MSH_uDES_B_2				0x41000018
#define		MSH_uDES_B_3				0x4100001C
#define		MSH_uDES_BaseAddr			0x41000000

#define		MSH_uReadBufferAddr		0x42000000
#define		MSH_uWriteBufferAddr		0x43000000

#define		MSH_uBL_ReadBufAddr		0x42700000
#define		MSH_uDI_ReadBufAddr		0x42A00000

#define		MSH_uBL_WriteBufAddr		0x43700000
#define		MSH_uDI_WriteBufAddr		0x43A00000

#elif defined ( S5P6450 )

//temp
#define		MSH_uDES_A_0				0x24000000
#define		MSH_uDES_A_1				0x24000004
#define		MSH_uDES_A_2				0x24000008
#define		MSH_uDES_A_3				0x2400000C
#define		MSH_uDES_B_0				0x24000010
#define		MSH_uDES_B_1				0x24000014
#define		MSH_uDES_B_2				0x24000018
#define		MSH_uDES_B_3				0x2400001C
#define		MSH_uDES_BaseAddr			0x24000000

#define		MSH_uReadBufferAddr		0x22000000
#define		MSH_uWriteBufferAddr		0x23000000

#define		MSH_uBL_ReadBufAddr		0x22700000
#define		MSH_uDI_ReadBufAddr		0x22A00000

#define		MSH_uBL_WriteBufAddr		0x23700000
#define		MSH_uDI_WriteBufAddr		0x23A00000

#else 
#error 
#endif			




//''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''[Define Area ]



// enum for Register Map!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
// enum for Register Map!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

// Register Map : [CTRL] offset : 0x0
typedef enum
{
	MSH_CTRL_UseIDMAC		= ( 0x1 << 25 ),
	MSH_CTRL_EnableExtDMA		= ( 0x1 << 5 ),
	MSH_CTRL_EnableINT		= ( 0x1 << 4 ),
	MSH_CTRL_DMAReset			= ( 0x1 << 2 ),
	MSH_CTRL_FIFOReset		= ( 0x1 << 1 ),
	MSH_CTRL_ControllerReset	= ( 0x1 << 0 ),	
}eMSH_CTRLReg;




// Register Map : [RESP 0~3] offset : 0x30
typedef enum
{
	MSH_Response_0		= rMSH_RESP_0,
	MSH_Response_1		= rMSH_RESP_1,
	MSH_Response_2		= rMSH_RESP_2,
	MSH_Response_3		= rMSH_RESP_3,
}eMSH_Response;






// enum for Register Map!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!


typedef enum
{
	MSH_STATUS_CardPresent	= ( 0x1 << 8 ),

	MSH_STATUS_FifoFull		= ( 0x1 << 3 ),
	MSH_STATUS_FifoEmpty		= ( 0x1 << 2 ),
	MSH_STATUS_ReachedTxWM 	= ( 0x1 << 1 ),
	MSH_STATUS_ReachedRxWM 	= ( 0x1 << 0 ),
}eMSH_STATUSReg;



// related with RegisterMap
typedef enum
{
	MSH_WAKEUP_Status			= 4,
	MSH_WAKEUP_SDCDSel		= 3,
	MSH_WAKEUP_Remove		= 2,
	MSH_WAKEUP_Insert			= 1,
	MSH_WAKEUP_Interrupt		= 0,	
}eMSH_WAKEUPCONReg;

typedef enum
{
	MSH_WAKEUP_NotOccurred	= 0,
	MSH_WAKEUP_Occurred		= 1,
}eMSH_StaWakeup;


// _____[ BMOD(Bus Mode)  register ]_____

typedef enum
{
	MSH_BMOD_Enable		= ( 0x1 << 7 ),
	MSH_BMOD_FixedBurst	= ( 0x1 << 1 ),
	MSH_BMOD_SWReset		= ( 0x1 << 0 ),
}eMSH_BMOD;



// _____[ IDINTEN (Internal DMAC INTerrupt Enable) register ]_____

typedef enum
{
	MSH_IDINTEN_All			= ( 0x337 ),
	MSH_IDINTEN_AIS			= ( 0x1 << 9 ), // Abnormal Interrupt Summary Enable.
	MSH_IDINTEN_NIS			= ( 0x1 << 8 ), //    Normal Interrupt Summary Enable.
	MSH_IDINTEN_CES			= ( 0x1 << 5 ), // Card Error Summary Interrupt Enable.
	MSH_IDINTEN_DU			= ( 0x1 << 4 ), // Descriptor Unavailable Interrupt Enable.
	MSH_IDINTEN_FBE			= ( 0x1 << 2 ), // Fatal Bus Error Enable.
	MSH_IDINTEN_ReceiveINT	= ( 0x1 << 1 ), // Receive Interrup Enable.
	MSH_IDINTEN_TransmitINT	= ( 0x1 << 0 ), // Transmit Interrup Enable.
}eMSH_IDINTEN;





// _____[ CMD register ]_____

typedef enum // CMD(Command Register, Addr Offset : 0x2C) [21]
{
	MSH_NormalCmdSeq		= (0x0 << 21),
	MSH_UpdateClkReg		= (0x1 << 21),
}eMSH_UpdateClkReg;



// _____[ FIFOTH ]_____

typedef enum 
{
	MSH_MSize_1	= (0x0 << 28),
	MSH_MSize_4	= (0x1 << 28),
	MSH_MSize_8	= (0x2 << 28),
	MSH_MSize_16	= (0x3 << 28),
	MSH_MSize_32	= (0x4 << 28),
	MSH_MSize_64	= (0x5 << 28),
	MSH_MSize_128	= (0x6 << 28),
	MSH_MSize_256	= (0x7 << 28),
}eMSH_MSize;



//(eMSH_BootMode eBootMode, eMSH_ExpectBootAck eBootAck, eMSH_EnableBoot eEnableBoot);
// [27 : 24]
// void MSH_SetCmd_BootOption()
typedef enum
{
	MSH_MandatoryBoot		= (0x0 << 27),
	MSH_AlternateBoot		= (0x1 << 27),
}eMSH_BootMode;

typedef enum
{
	MSH_NoBootAck			= (0x0 << 25),
	MSH_ExpectedBootAck	= (0x1 << 25),
}eMSH_ExpectBootAck;


typedef enum
{
	MSH_DisableBoot		= (0x1 << 26), // !!!!! 
	MSH_EnableBoot			= (0x1 << 24),
}eMSH_EnableBoot;


// [14 : 12]
// void MSH_SetCmd_CmdOption()
typedef enum	
{
	MSH_NoStopAbortCmd	= (0x0 <<14),
	MSH_StopAbortCmd		= (0x1 <<14),
}eMSH_StopAbortCmd;

typedef enum
{
	MSH_SendCmdAtOnce	= (0x0 <<13),
	MSH_WaitPrvCmd		= (0x1 <<13),
}eMSH_WaitPrvCmd;

typedef enum
{
	MSH_NotAutoStop		= (0x0 <<12),
	MSH_SendAutoStop		= (0x1 <<12),
}eMSH_SendAutoStop;

// [11 : 09]
// void MSH_SetCmd_RWOption()
typedef enum
{
	MSH_BlockTransfer		= (0x0 <<11),
	MSH_StreamTransfer		= (0x1 <<11),
}eMSH_TransMode;

typedef enum 
{
	MSH_Read_from_Card	= (0x0 << 10),
	MSH_Write_to_Card		= (0x1 << 10),
}eMSH_nRW;

typedef enum 
{
	MSH_NoDataTransfer		= (0x0 << 9),
	MSH_DataTransfer		= (0x1 << 9),
}eMSH_DataExpected;

// [08 : 06]
// void MSH_SetCmd_RspOption()
typedef enum
{
	MSH_SkipRspCRC			= (0x0 << 8),
	MSH_CheckRspCRC		= (0x1 << 8),
}eMSH_CheckRspCRC;

typedef enum
{
	MSH_ShortLength		= (0x0 << 7),
	MSH_LongLength			= (0x1 << 7),
}eMSH_RspLength;

typedef enum
{
	MSH_NoResponse		= (0x0 << 6),
	MSH_ResponseExpected	= (0x1 << 6),
}eMSH_RspExpected;





// _____[ Interrupt ]_____
typedef enum
{
	MSH_AllMaskINT			= 0,

//
	MSH_CardDetect			= (0x1 << 0),
	MSH_RespErr			= (0x1 << 1),
	MSH_CmdDone			= (0x1 << 2),
	MSH_TransferOver		= (0x1 << 3),
//
	MSH_TXDataReq			= (0x1 << 4),
	MSH_RXDataReq			= (0x1 << 5),
	MSH_RespCRCErr			= (0x1 << 6),	
	MSH_DataCRCErr			= (0x1 << 7),

//
	MSH_RespTimeOut		= (0x1 << 8),
	MSH_BootAckRec			= (0x1 << 8),

	MSH_DataReadTimeOut	= (0x1 << 9),
	MSH_BootDataStart		= (0x1 << 9),
	
	MSH_DataStarvation		= (0x1 << 10),
	MSH_VoltSwitchInt		= (0x1 << 10),
	MSH_FIFOErr			= (0x1 << 11),

//
	MSH_HwLockedWriteErr	= (0x1 << 12),
	MSH_StartBitErr			= (0x1 << 13),
	MSH_AutoCmdDone		= (0x1 << 14),
	MSH_EndBitErr			= (0x1 << 15),
	MSH_WriteNoCRC			= (0x1 << 15),

	MSH_AllEnableINT		= 0xFFFF,
}eMSH_INTERRUPT;




// ___________________________________________________________________________ [etc ENUM Area ]

typedef enum
{
	MMC44_1bitDataBus			= 0x0,
	MMC44_4bitsDataBus			= 0x1,
	MMC44_8bitsDataBus			= 0x2,
	MMC44_4bitsDataBus_DDR	= 0x5,
	MMC44_8bitsDataBus_DDR	= 0x6,
}eMSH_MMC_BusWidth;

typedef enum
{
	MSH_Identify_400	= 0x1,
	MSH_NormalSpeed	= 0x2,
	MSH_HighSpeed		= 0x4,
	MSH_Bypass			= 0x8,
	MSH_GetFromStruct	= 0xD1E,
}eMSH_ClkSpeed;



// _____[ SPCON Reg. in GPIO, 0xE030_81A0 ]_____

typedef enum
{
	MSH_DS_2mA		= (0x0 << 26),
	MSH_DS_4mA		= (0x1 << 26),
	MSH_DS_8mA		= (0x2 << 26),
	MSH_DS_12mA		= (0x3 << 26),
}eMSH_DriveStrength; 


//''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''[etc ENUM Area ]




// ___________________________________________________________________________ [Struct Area ]


typedef enum
{
	MSH_DeviceType_Absent	= 0x00,
	MSH_DeviceType_SD		= 0x01,
	MSH_DeviceType_MMC	= 0x02,
	MSH_DeviceType_eMMC	= 0x04,
	MSH_DeviceType_SDIO	= 0x08,
}eMSH_DeviceType;

typedef enum
{
	MSH_CardSlot_1	 = 0x01,
	MSH_CardSlot_2	 = 0x02,
	MSH_CardSlot_3	 = 0x04,
	MSH_CardSlot_4	 = 0x08,
}eMSH_CardSlot;

typedef enum // CLKSRC(Clock Source Reg. Addr Offset : 0x0C)
{
	MSH_ClkDivider_0	= 0x0,
	MSH_ClkDivider_1	= 0x1,
	MSH_ClkDivider_2	= 0x2,
	MSH_ClkDivider_3	= 0x3,	
}eMSH_SelectDivider;

typedef enum
{
	MSH_ClkSrc_FIN		= 0x0,
	MSH_ClkSrc_HCLK	= 0x1,
	MSH_ClkSrc_PCLK	= 0x2,
	MSH_ClkSrc_MPLL	= 0x4,
	MSH_ClkSrc_EPLL	= 0x8,
}eMSH_ClkSrc;

typedef enum
{
	MSH_1bit_Mode			= ( 0x0 << 0),
	MSH_4bits_Mode			= ( 0x1 << 0),
	MSH_8bits_Mode			= ( 0x1 << 16),
}eMSH_BusWidth;

typedef enum
{
	MSH_BlockAddrFmt	= 0x1,
	MSH_ByteAddrFmt	= 0x2,
}eMSH_AddrFmt;

#if 0
typedef enum
{
	BlockTransfer		= 0x0,
	StreamTransfer		= 0x1,
}eMSH_DataTransfer;
#endif

typedef	enum
{
	MSH_SetStructFlag_ON	=0x0,		
	MSH_SetStructFlag_OFF	=0x1,
}eMSH_SetStructFlag;

#if 0
typedef enum
{
	MSH_CH_0	= 0,
	MSH_CH_1	= 1, 
	MSH_CH_2	= 2,
}eMSH_CH;
#endif

// _____[ UHS Register ]_____

typedef enum
{
	MSH_NonDDR_Mode	= ( 0x0 << 16 ),
	MSH_4bitsDDR_Mode	= ( 0x1 << 16 ),
}eMSH_UHS;


typedef enum
{
	MSH_EventDriven_Polling		= 0x1,
	MSH_EventDriven_Interrupt	= 0x2,
}eMSH_EventDrivenMode;

typedef enum
{
	MSH_NotUsingIDMA		= (0x0 << 25),
	MSH_UsingIDMA			= (0x1 << 25),
}eMSH_UsingIDMAorNot;


// - Data Transfer Cmd       :  Use TimeOut RINT (You can use ISR in Interrupt Mode)
// - in Boot Mode                : Can Not use TimeOut RINT (If you want? You must use IDMAC !!)
// - Non Data Transfer Cmd : Can Not use TimeOut RINT (Not occur TimeOut RINT !!!) 
typedef enum
{
	MSH_NotUseTORINT_InPollingMode	= 0x0,
	MSH_UseTORINT_InPollingMode 		= 0x1,
}eMSH_UseTORINT_InPollingMode;


typedef struct
{
	//eMSH_CH				eCH;
	volatile eMSH_CardSlot			eCardSlot;		// fixed at Vega-1
	volatile eMSH_SelectDivider		eDivider;		// fixed at Vega-1
	volatile eMSH_DeviceType		eDeviceType;	// determined by code. default is Absent
	volatile eMSH_ClkSrc			eClkSrc;
	volatile eMSH_BusWidth			eBusWidth;
	volatile eMSH_AddrFmt			eAddrFmt;
	volatile u32					uCardNum;		// determined by code

	volatile eMSH_EventDrivenMode	eEventMode;
	volatile eMSH_UsingIDMAorNot	eUsingIDMAorNot;
	volatile u32					uOccurredInt;
		
	volatile eMSH_UHS				eUHS;
	volatile u32 					uSetDividerValue;
	
	volatile eMSH_MSize			eMSize;				// [30:28]
	volatile u32					uWMark_Receive;	// [27:16]
	volatile u32					uWMark_Transmit;	// [11:00]

	volatile u32					uTotalTransferSize;
	volatile u32					uBufferAddr;
	volatile u32					uSectorCount;

	volatile eMSH_UseTORINT_InPollingMode	eUseTORINT;
	volatile eMSH_SetStructFlag				eSetStructFlag;
	
	//eMSH_DataTransfer	eDataTransfer;	
}MSH;


//''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''[Struct Area ]
bool  LoadBL1FromEmmc44Ch3(void);
bool LoadBL2FromEmmc44Ch3(u32 uNumofBlocks_BL2, u32* uDstAddr_BL2);
bool LoadDramImgFromEmmc44Ch3(u32 uNumofBlocks_DramImage, u32* uDstAddr_DramImage);


void MSH_SetDriveStrength_GPIO(eMSH_DriveStrength eDS);

void MSH_Init_eMMC(void);

void MSH_SetController_eMMC(void);

bool MSH_ReadBlocks_eMMC(u32 uStBlock, u32 uNumofBlocks_BL1, u32 uDstAddr_BL1,
							u32 uNumofBlocks_BL2, u32 uDstAddr_BL2,
							u32 uNumofBlocks_DramImage, u32 uDstAddr_DramImage);

bool MSH_ReadFromFIFO_eMMC(u32 uRxWMark, u32 uNumofBlocks, void * uDstAddr);


void MSH_EndBootOp_eMMC(void);

void MSH_WaitForCardReady(void);


// ###################################################### [ Card Identification Area] 
// ###################################################### [ Card Identification Area] 
// ###################################################### [ Card Identification Area] 

void MSH_InitIP(void);


//bool MSH_CardIdentification(void);

u32 MSH_GetCardSize(void);

bool MSH_OpenMedia(void);

bool MSH_OpenMediaWithMode(eMSH_BusWidth eBusWidth, eMSH_UHS eUHS,
									eMSH_EventDrivenMode eEventMode, eMSH_UsingIDMAorNot eUsingIDMAorNot,
									eMSH_MSize eMSize, u32 uWMark_Receive, u32 uWMark_Transmit);

void MSH_SetHostController(void);

bool MSH_Step1_IdentifyCard(void);
void MSH_Step2_GoStandbyState(void);
void MSH_Step3_GoTransferState(void);
void MSH_Step4_SetBusWidth(void);
void MSH_Step5_SET_BLOCKLEN(void);

bool MSH_IdentifySD(void);

bool MSH_IdentifyMMC(void);



// ###################################################### [ Access Function Area ] 
// ###################################################### [ Access Function Area ] 
// ###################################################### [ Access Function Area ] 

void MSH_SetBlkSize(u32 uBlkSize);

void MSH_SetBytCnt(u32 uBytCnt);



// ###################################################### [ used IDMAC ] 
// ###################################################### [ used IDMAC ] 

// Chained - setting
void MSH_SetDES_ChainedSingleDES(u32 uNumofBlocks, u32 uBufferAddr);
void MSH_SetDES_ChainedMultiDES(u32 uNumofBlocks, u32 uBufferAddr);
void MSH_SetDES_ChainedMultiDES_4kBytes(u32 uNumof4kBytes, u32 uBufferAddr);


void MSH_WriteBlocksByIDMAC_ChainedMultiDES_4kBytes(u32 uStBlock, u32 uNumof4kBytes, void * uSrcAddr);
void MSH_ReadBlocksByIDMAC_ChainedMultiDES_4kBytes(u32 uStBlock, u32 uNumof4kBytes, void * uDstAddr);



// Read
void MSH_ReadBlocksByIDMAC_ChainedSingleDES(u32 uStBlock, u32 uNumofBlocks, void * uDstAddr);
void MSH_ReadBlocksByIDMAC_ChainedMultiDES(u32 uStBlock, u32 uNumofBlocks, void * uDstAddr);

// Write
void MSH_WriteBlocksByIDMAC_ChainedSingleDES(u32 uStBlock, u32 uNumofBlocks, void * uDstAddr);
void MSH_WriteBlocksByIDMAC_ChainedMultiDES(u32 uStBlock, u32 uNumofBlocks, void * uSrcAddr);

// Dual-Buffer - setting
void MSH_SetDES_DualBufSingleDES(u32 uNumofBlocks, u32 uBufferAddr);
void MSH_SetDES_DualBufMultiDES(u32 uNumofBlocks, u32 uBufferAddr);

// Read
void MSH_ReadBlocksByIDMAC_DualBufSingleDES(u32 uStBlock, u32 uNumofBlocks, void * uDstAddr);
void MSH_ReadBlocksByIDMAC_DualBufMultiDES(u32 uStBlock, u32 uNumofBlocks, void * uDstAddr);

// Write
void MSH_WriteBlocksByIDMAC_DualBufSingleDES(u32 uStBlock, u32 uNumofBlocks, void * uSrcAddr);
void MSH_WriteBlocksByIDMAC_DualBufMultiDES(u32 uStBlock, u32 uNumofBlocks, void * uSrcAddr);


// Read or Write Operation by IDMAC
void MSH_ReadOperationByIDMAC(u32 uStBlock, u32 uNumofBlocks, void * uDstAddr);
void MSH_WriteOperationByIDMAC(u32 uStBlock, u32 uNumofBlocks, void * uSrcAddr);


// ###################################################### [ Read Functions Area ] 

bool MSH_ReadBlock(u32 uStBlock, void * uDstAddr);
bool MSH_ReadBlocks(u32 uStBlock, u32 uNumofBlocks, void * uDstAddr);
void MSH_ReadOperation(u32 uStBlock, u32 uNumofBlocks, void * uDstAddr);

bool MSH_ReadData(u32 uRxWMark, u32 uNumofBlocks, void * uDstAddr);
bool MSH_ReadData_Performance(u32 uRxWMark, u32 uNumofBlocks, void * uDstAddr);

void MSH_ReadFromFIFO(u32 uTransferCount, void * uDstAddr);


// ###################################################### [ Write Functions Area ] 

bool MSH_WriteBlock(u32 uStBlock, void * uDstAddr);
bool MSH_WriteBlocks(u32 uStBlock, u32 uNumofBlocks, void * uDstAddr);
void MSH_WriteOperation(u32 uStBlock, u32 uNumofBlocks, void * uSrcAddr);
bool MSH_WriteData(u32 uTxWMark, u32 uNumofBlocks, void * uSrcAddr);
void MSH_WriteToFIFO(u32 uTransferCount, void * uSrcAddr);



// ###################################################### [ related with Spec~! ] 

void MSH_EnabledHardWareRST(void);

void MSH_SetEnhancedAreaForMMC441(void);


bool MSH_GO_IDLE_STATE_byCMD0(void);

bool MSH_GO_PRE_IDLE_STATE_byCMD0(void);

void MSH_SetMmcHSTiming(void);

void MSH_SendWakeUp(void);
	

// ###################################################### [ related with Structure ] 
// ###################################################### [ related with Structure ] 
// ###################################################### [ related with Structure ] 

void MSH_oDisp_Information(void);

void MSH_oSet_SetStruct(void);
//void	MSH_oSet_SetStruct_iROM(void);


void MSH_oSet_BusWidth(eMSH_BusWidth eBusWidth);

void MSH_oSet_EventDrivenMode(eMSH_EventDrivenMode eEventMode);

void MSH_oSet_UsingIDMAorNot(eMSH_UsingIDMAorNot eUsingIDMAorNot);

void MSH_oSet_UseTORINT_InPollingMode(eMSH_UseTORINT_InPollingMode eUseTORINT);

// [30:28]
void MSH_oSet_MSize(eMSH_MSize eMSize);

// [27:16]
void MSH_oSet_WMark_Receive(u32 uRX_WMark);

// [11:00]
void MSH_oSet_WMark_Transmit(u32 uTX_WMark);

void MSH_oSet_DDRMode(eMSH_UHS eUHS);


void MSH_oSet_ClkSource(eMSH_ClkSrc eClkSrc);

void MSH_oSet_uSetDividerValue(u32 uDividerValue);

//void MSH_oSet_DataTransfer(eMSH_DataTransfer eDataTransfer);

void MSH_oSet_SetStructFlag_ON(void);
void MSH_oSet_SetStructFlag_OFF(void);

void MSH_oSet_DeviceType(eMSH_DeviceType eDeviceType);
void MSH_oSet_CardSlot(eMSH_CardSlot eCardSlot);
void MSH_oSet_SelectDivider(eMSH_SelectDivider eDivider);
void MSH_oSet_CardNum(u32 uCardNum);


// Get~ Inform from Structure!!!!
eMSH_BusWidth MSH_oGet_BusWidth(void);

eMSH_EventDrivenMode MSH_oGet_EventDrivenMode(void);

eMSH_UsingIDMAorNot MSH_oGet_UsingIDMAorNot(void);

eMSH_UseTORINT_InPollingMode MSH_oGet_UseTORINT_InPollingMode(void);

// [30:28]
eMSH_MSize MSH_oGet_MSize(void);

// [27:16]
u32 MSH_oGet_WMark_Receive(void);

// [11:00]
u32 MSH_oGet_WMark_Transmit(void);

eMSH_UHS MSH_oGet_DDRMode(void);

eMSH_DeviceType MSH_oGet_DeviceType(void);

// ###################################################### [ Basic Function Area ] 
// ###################################################### [ Basic Function Area ] 
// ###################################################### [ Basic Function Area ] 

void MSH_SetBusWidth(eMSH_BusWidth eBusWidth);


void MSH_TimeOutFunc(u32 uFreqMhz, u32 uMicroSec);

bool MSH_WaitForRINTandClear(eMSH_INTERRUPT eIntSrc);


void Isr_MSH(void);
void Isr_MSH_FIFO(void);
void Isr_MSH_WakeupInterrupt(void);

void __Isr_MSH(void);
u32 MSH_FindingTheBit(u32 uNameofReg);


void MSH_AllClear_IDSTS(void);

void MSH_AllClear_RINTSTS(void);

void MSH_Clear_RINTSTS(eMSH_INTERRUPT eIntSrc);

bool MSH_Wait4Card2TransferState(void);


/* ________________ [[ Set Card Type for "CTYPE reg." ]] ________________*/
//void MSH_SetCTYPE_BusWidth(void);

bool MSH_Wait4CmdDone(void);

u32 MSH_GetResponse(eMSH_Response eResponse);


// ###################################################### [ Issue Command Functions Area ] 

void MSH_ClearCmdReg(void);

void MSH_CmdArgument(u32 uCmdArg);

// [27 : 24]
void MSH_SetCmd_BootOption(eMSH_BootMode eBootMode, eMSH_ExpectBootAck eBootAck, eMSH_EnableBoot eEnableBoot);

// [14 : 12]
void MSH_SetCmd_CmdOption(eMSH_StopAbortCmd eStopAbortCmd, eMSH_WaitPrvCmd eWaitPrvCmd, eMSH_SendAutoStop eSendAutoStop);

// [11 : 09]
void MSH_SetCmd_RWOption(eMSH_TransMode eTransMode, eMSH_nRW eReaWrite, eMSH_DataExpected eDataExpected);

// [08 : 06]
void MSH_SetCmd_RspOption(eMSH_CheckRspCRC eCheckRspCRC, eMSH_RspLength eRspLength, eMSH_RspExpected eRspExpected);

// [05:00]
bool MSH_SetCmd_IssueCmd(u32 CmdIndex);

void MSH_SetCmd_UpdateClockReg(void);

void MSH_SetCLKforMode(eMSH_ClkSpeed eClkSpeed);

void MSH_SetCLKDividerValue(u32 uDividerValue);



/* ________________ [[ Wakeup Status ]] ________________*/
// WAKEUPCON [4]
eMSH_StaWakeup MSH_GetWakeupCon_StaWakeup(void);
// WAKEUPCON [4]
void MSH_SetWakeupCon_ClearWakeupSta(void);

/* ________________ [[ SDCD Selection ]] ________________*/
// WAKEUPCON [3]
void MSH_SetWakeupCon_nSDCD(void);
void MSH_SetWakeupCon_nDAT3 (void);

/* ________________ [[ Wakeup-Remove ]] ________________*/
// WAKEUPCON [2]
void MSH_SetWakeupCon_EnRemove(void);
void MSH_SetWakeupCon_DisRemove(void);

/* ________________ [[ Wakeup-Insert ]] ________________*/
// WAKEUPCON [1]
void MSH_SetWakeupCon_EnInsert(void);
void MSH_SetWakeupCon_DisInsert(void);


/* ________________ [[ Wakeup-Interrupt ]] ________________*/
// WAKEUPCON [0]
void MSH_SetWakeupCon_EnInterrupt(void);
void MSH_SetWakeupCon_DisInterrupt(void);


/* ________________ [[ IDMAC ]] ________________*/

// BMOD[0] (offset : 0x80)
void MSH_Reset_InternalDMAC(void);

// BMOD[1] (offset : 0x80)
void MSH_SetBMOD_EnableIDMAC(void);
void MSH_SetBMOD_DisableIDMAC(void);

// BMOD[7] (offset : 0x80)
void MSH_SetBMOD_FixedBurst(void);
void MSH_SetBMOD_NotFixedBurst(void);

// IDINTEN (offset : 0x90)
void MSH_AllEnable_IDMAC_Interrupt(void);
void MSH_AllMask_IDMAC_Interrupt(void);













/* ________________ [[ Set DDR Mode ]] ________________*/

void MSH_SetUHS_DdrMode(void);
void MSH_SetUHS_NonDdrMode(void);


/* ________________ [[ FIFO Threshold ]] ________________*/

void MSH_SetFIFOTH_MSize(eMSH_MSize eMSize);

void MSH_SetFIFOTH_RxWMark(u32 uRX_WMark);

void MSH_SetFIFOTH_TxWMark(u32 uTX_WMark);


/* ________________ [[ Interrupts ]] ________________*/
// CTRL [04]
void MSH_SetCTRL_EnableINT(void);
void MSH_SetCTRL_DisableINT(void);

/* ________________ [[ DMA ]] ________________*/
// CTRL [05]
void MSH_SetCTRL_EnableExtDMA(void);
void MSH_SetCTRL_DisableExtDMA(void);

/* ________________ [[ IDMAC ]] ________________*/
// CTRL [25]
void MSH_SetCTRL_UsingIDMAC(void);
void MSH_SetCTRL_NotUsingIDMAC(void);


/* ________________ [[ Reset Functions ]] ________________*/

void MSH_ResetAll(void);

void MSH_Reset_DmaFifoCntl(void);
	
void MSH_Reset_DMA(void);

void MSH_Reset_FIFO(void);

void MSH_Reset_Controller(void);

void MSH_ClearRegister(u32 rDefinedReg);


/* ________________ [[ Interrupt ]] ________________*/

void MSH_AllMask_Interrupt(void);

void MSH_AllEnable_Interrupt(void);

void MSH_Mask_SelectedInterrupt(eMSH_INTERRUPT eIntSrc);

void MSH_Enable_SelectedInterrupt(eMSH_INTERRUPT eIntSrc);
//void MSH_SelectedEnableInterrupt(eMSH_INTERRUPT eIntSrc);


/* ________________ [[ Power Enable / Disable ]] ________________*/

void MSH_Enable_POWER(void);

void MSH_Disable_POWER(void);


/* ________________ [[ Clock Enable / Disable ]] ________________*/

void MSH_Enable_CLOCK(void);

void MSH_Disable_CLOCK(void);


/* ________________ [[ Set / Get Register ]] ________________*/
void MSH_SetRegister(u32 uNameofReg, u32 uValue);
u32 MSH_GetRegister(u32 uNameofReg);


/* ________________ [[ Set GPIO for MobileStorageHost ]] ________________*/

void MSH_SetGPIO(void);




#ifdef __cplusplus
}
#endif

#endif //__MobileStorageHost_H__




