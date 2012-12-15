//Initial emmc driver for enabling boot from emmc.
//manoj.kh@samsung.com
//copiously use code from linux mshci driver :)
//


#include <config.h>
#include <common.h>
#include <mmc.h>
#include <part.h>
#include <regs.h>
#include <malloc.h>
#include <s5p_emmc.h>
#include <asm/io.h>

DECLARE_GLOBAL_DATA_PTR; //seems universal in u-boot.
 
#define emmc_dbg //printf
#define emmc_data_dbg //printf 
#define emmc_resp_dbg //printf 
#define emmc_fn //printf("[- %s]:%d\n",__func__,__LINE__)

#define MAX_EMMC_CHANNELS 1 //move to machine header.

#ifndef mdelay
#define mdelay(x)       udelay(1000*x)
#endif

struct mmc EMMC[MAX_EMMC_CHANNELS];
struct mshci_host emmc_host[MAX_EMMC_CHANNELS];

static int initialized;
static int mshci_writeto_fifo(struct mshci_host *host, struct mmc_data *data)
{
	int transfersize = (data->blocks * (data->blocksize>>2));
	int i,count = 0;
	u32 *p;
	p = (u32*)(data->src);
	int loopcount = 1000; //1ms
	emmc_dbg("\n emmc: writing data %d bytes",transfersize * 4);

#if 1		
	while(readl(host->ioaddr + MSHCI_STATUS) & (DATA_BUSY /*| DATA_MCBUSY*/)){
		emmc_dbg("\n emmc :  card busy");
	} 
#endif
	do{
		if(readl(host->ioaddr + MSHCI_RINTSTS) & INTMSK_TXDR){
			for(i=0; i < TX_WMARK; i++){
				writel(*p, host->ioaddr + MSHCI_FIFODAT);
				p++;
				count += 4;
			}
		transfersize -= TX_WMARK;
		writel(readl(host->ioaddr + MSHCI_RINTSTS) | INTMSK_TXDR,host->ioaddr + MSHCI_RINTSTS); 
		}

	}while(transfersize >= TX_WMARK);
	
	if(transfersize < TX_WMARK && transfersize !=0){
		while(loopcount > 0){
			if(readl(host->ioaddr + MSHCI_RINTSTS) & INTMSK_DTO){
				for(i=0;i<transfersize;i++){
					writel(*p, host->ioaddr + MSHCI_FIFODAT);
					p++;
					count += 4;
				}
				writel(readl(host->ioaddr + MSHCI_RINTSTS) | INTMSK_TXDR,host->ioaddr + MSHCI_RINTSTS); 
				writel(readl(host->ioaddr + MSHCI_RINTSTS) | INTMSK_DTO,host->ioaddr + MSHCI_RINTSTS); 
			}

		}	
		

	}	

	emmc_dbg("\n tx bytes written 0x%x \n",count);

//	writel(readl(host->ioaddr + MSHCI_RINTSTS) | INTMSK_ALL,host->ioaddr + MSHCI_RINTSTS);
	return 0; 

}






static int mshci_read_fifo(struct mshci_host *host, struct mmc_data *data)
{

	int transfersize = (data->blocks * (data->blocksize>>2));
	int i, count = 0;
	u32 *p;
	p  = (u32*)(data->dest);
	
	emmc_dbg("\n emmc: reading data %d bytes",transfersize * 4);
#if 1		
	while(readl(host->ioaddr + MSHCI_STATUS) & (DATA_BUSY /*| DATA_MCBUSY*/| FSM_WAIT)){
		emmc_dbg("\n emmc :  card busy");
	} 
#endif
	do{
		if(readl(host->ioaddr + MSHCI_RINTSTS/*MSHCI_STATUS*/ ) & INTMSK_RXDR /* FIFO_RXWTRMARK*/){
			for(i=0;i < RX_WMARK; i++){
				*p++ = readl(host->ioaddr + MSHCI_FIFODAT);
				count += 4;
				emmc_data_dbg("\nd:0x%x",*(p - 1));
			}
			
			transfersize -= (RX_WMARK);		
			writel(readl(host->ioaddr + MSHCI_RINTSTS) | INTMSK_RXDR,host->ioaddr + MSHCI_RINTSTS); 
		}
		else{
		
			emmc_dbg("\n emmc :data 0x%x, status 0x%x,int 0x%x\n",transfersize,readl(host->ioaddr + MSHCI_STATUS),readl(host->ioaddr + MSHCI_RINTSTS));
			udelay(1);
			//return -1;
		}	
	}while(transfersize > (RX_WMARK ));
	
	emmc_dbg("\n emmc: initial transfer done. bytes remaining : 0x%x",transfersize);


	if((transfersize <= (RX_WMARK + 1)) && transfersize != 0){  //wont get interrupt if data in fifo is lesser than wmark.
		//so just read that much data, and clear rintsts.
		i = 0;
		while(i < transfersize){
		/*for(i=0; i < transfersize; i++){*/
			if(!(readl(host->ioaddr + MSHCI_STATUS)& FIFO_EMPTY)){
			count += 4;
			i++ ;
			*p++ = readl(host->ioaddr + MSHCI_FIFODAT);
			emmc_data_dbg("\nd:0x%x",*(p - 1));
			}else{
				emmc_data_dbg("\n emmc status register 0x%x",readl(host->ioaddr + MSHCI_STATUS));
			}
			//if(i >= transfersize)
			//	break;
		}
		writel(readl(host->ioaddr + MSHCI_RINTSTS) | INTMSK_ALL,host->ioaddr + MSHCI_RINTSTS); 
	} 	 

	//writel(readl(host->ioaddr + MSHCI_RINTSTS) | INTMSK_ALL,host->ioaddr + MSHCI_RINTSTS); 
	emmc_dbg("\n>>>>>>>bytes read %d \n",count);

	return 0;		

}



struct descriptor_table{
	u16 attribute;
	u16 length;
	u32 address;
}idmac;


static void mshci_prepare_data(struct mshci_host *host,struct mmc_data *data)
{

	emmc_fn;

	//Point of being absurd!!
#if 0 //forget adma. Read from fifo.

	idmac.attribute = 0x27; //referring c110 descriptor table
	idmac.length = data->blocks * (MSHCI_MAKE_BLKSZ(7, data->blocksize));
	idmac.address = virt_to_phys((u32)data->dest);


	writel(virt_to_phys(&idmac), host->ioaddr + MSHCI_DBADDR); //not sure at all!! :(

	writew(MSHCI_MAKE_BLKSZ(7, data->blocksize), host->ioaddr + MSHCI_BLKSIZ); //1 page?
	writew(data->blocks * (MSHCI_MAKE_BLKSZ(7, data->blocksize)), host->ioaddr + MSHCI_BYTCNT);

	writel((readl(host->ioaddr + MSHCI_CTRL) |
                                        ENABLE_IDMAC|DMA_ENABLE),host->ioaddr + MSHCI_CTRL);
        writel((readl(host->ioaddr + MSHCI_BMOD) |
                                        (BMOD_IDMAC_ENABLE|BMOD_IDMAC_FB)),
                                        host->ioaddr + MSHCI_BMOD);
	#if 0
        writel(host->ioaddr + INTMSK_IDMAC_ERROR, host->ioaddr + MSHCI_IDINTEN);
	#endif

#endif

	//1)set block size, allow max 512 only :P
	if(data->blocksize > MSHCI_BLOCK_SIZE){
		printf("\n emmc : Block size > 512 not supported! may lose data ");
		writel(MSHCI_BLOCK_SIZE, host->ioaddr + MSHCI_BLKSIZ);
	}
	else{
		writel(data->blocksize,host->ioaddr + MSHCI_BLKSIZ); 
	}
		
	emmc_dbg("\n emmc : block size 0x%x", readl(host->ioaddr + MSHCI_BLKSIZ));

	//2) set byte count
	emmc_dbg("\n emmc : block count 0x%x",data->blocks);
	
	writel((data->blocks * data->blocksize), host->ioaddr + MSHCI_BYTCNT);
	emmc_dbg("\n emmc : byte count 0x%x", readl(host->ioaddr + MSHCI_BYTCNT));

	//3) set burst size to one transaction, as we are simply reading from fifo.
	writel((readl(host->ioaddr + MSHCI_FIFOTH) & ~MSIZE_MASK | MSIZE_1),host->ioaddr + MSHCI_FIFOTH); 

	//set rx water mark
	writel((readl(host->ioaddr + MSHCI_FIFOTH) & ~(0xFFF << 16) | MRX_WMARK),host->ioaddr + MSHCI_FIFOTH); 
	//set tx water mark
	writel((readl(host->ioaddr + MSHCI_FIFOTH) & ~(0xFFF) | MTX_WMARK),host->ioaddr + MSHCI_FIFOTH); 

	emmc_dbg("\n emmc : FIFOTH (register 0x4c) 0x%x",readl(host->ioaddr + MSHCI_FIFOTH));

}


static unsigned int mshci_set_transfer_mode(struct mshci_host *host,struct mmc_data *data)
{
	unsigned int ret = 0;

	emmc_fn;
	/* this cmd has data to transmit */
        ret |= CMD_DATA_EXP_BIT;

        if (data->flags & MMC_DATA_WRITE)
                ret |= CMD_RW_BIT;

        return ret;


}


static int  s5p_emmc_send_command(struct mmc *emmc,  struct mmc_cmd *cmd,
                        struct mmc_data *data)
{
	int ret,i;
	unsigned int flags = 0, intsts;
	struct mshci_host *host = emmc->priv;
	unsigned int status;
	unsigned int ctrl = readl(host->ioaddr + MSHCI_CTRL);
	int timeout;
		

	if (data)
		mshci_prepare_data(host,data);		

	emmc_dbg("\n[emmc:command 0x%x][Argument 0x%x]\n",cmd->opcode,cmd->arg);

	writel(cmd->arg, host->ioaddr + MSHCI_CMDARG);

	if(data)
		flags |= mshci_set_transfer_mode(host,data);
	
	emmc_resp_dbg("Command 0x%x, resp_type 0x%x,flags 0x%x\n",cmd->opcode,cmd->resp_type,cmd->flags);

	
	if ((cmd->resp_type & MMC_RSP_136) && (cmd->resp_type & MMC_RSP_BUSY)) {
                printf( "emmc:Unsupported response type!\n");
                return -1;
        }
	

        if (cmd->resp_type & MMC_RSP_PRESENT) {
                flags |= CMD_RESP_EXP_BIT;
                if (cmd->resp_type & MMC_RSP_136)
                        flags |= CMD_RESP_LENGTH_BIT;
        }
        if (cmd->resp_type & MMC_RSP_CRC)
                flags |= CMD_CHECK_CRC_BIT;
      
	flags |= (cmd->opcode | CMD_STRT_BIT | CMD_WAIT_PRV_DAT_BIT);  //0:5 bits cmd index, 
								       //then flags. Then settings to transmit command.

	if(!initialized){
		
	//	flags |= CMD_SEND_INIT_BIT;
		initialized = 1; 
	}
        ret = readl(host->ioaddr + MSHCI_CMD);
        if (ret & CMD_STRT_BIT)
                printf("emmc : CMD busy. current cmd %d. last cmd reg 0x%x\n",
                        cmd->opcode, ret);

	//while(readl(host->ioaddr + MSHCI_STATUS) & (DATA_BUSY | DATA_MCBUSY)){
	//	emmc_dbg("\n emmc :  card busy");
	//}
	
        writel(flags, host->ioaddr + MSHCI_CMD);
	writel((readl(host->ioaddr + MSHCI_CTRL) | INT_ENABLE),host->ioaddr + MSHCI_CTRL);
	//At this point, command has been sent.  
	
	//Now,wait till we get interrupt that command has been successfully sent, and response recieved.
	//
	for(i = 0; i < 100000; i++){
		intsts = readl(host->ioaddr + MSHCI_MINTSTS);
		//as of now check only for command done
		if(intsts & CMD_STATUS){
			//emmc_dbg("emmc: Command %d interrupt arrived\n",cmd->opcode);
			//if(!data) //data not there, we can clear cmd interrupt
				writel(intsts,host->ioaddr + MSHCI_RINTSTS);
			break; //at this point, we have got cmd done interrupt.
		} 
	
	}
	if (intsts & INTMSK_RTO){
          	printf("\nemmc : command %d response timed out",cmd->opcode);
		return TIMEOUT;
	}
        else if (intsts & (INTMSK_RCRC | INTMSK_RE)){
		printf("\nemmc : command %d response error!!",cmd->opcode);
                return -1; //not handling crc error.
	}
	if(i == 100000){ //unfortunately, we did not get the interrupt :(
		printf("\nemmc : interrupt not recieved after sending command %d!!",cmd->opcode);
		return TIMEOUT;
	}
	//if it gets here, means cmd done interrupt is recieved. Now handle it.
	//
//	#ifdef HANDLE_RESPONSE //as of now dont bother about response :P
	if (cmd->resp_type & MMC_RSP_PRESENT){ 			
		 if (cmd->resp_type & MMC_RSP_136) {
                        /*
 *                          * response data are overturned.
 *             			                                      */
			 for (i = 0;i < 4;i++) {
                                cmd->resp[0] = readl(host->ioaddr + MSHCI_RESP3);
                                cmd->resp[1] = readl(host->ioaddr + MSHCI_RESP2);
                                cmd->resp[2] = readl(host->ioaddr + MSHCI_RESP1);
                                cmd->resp[3] = readl(host->ioaddr +  MSHCI_RESP0);
                        }
                } else {
                        cmd->resp[0] = readl(host->ioaddr + MSHCI_RESP0);
                }
        }
	
	emmc_resp_dbg("\n response[0] 0x%x\nresponse[1] 0x%x\n response[2] 0x%x\nresponse[3] 0x%x",cmd->resp[0],cmd->resp[1],cmd->resp[2],cmd->resp[3]);



		if (cmd->resp_type & MMC_RSP_BUSY){
			emmc_resp_dbg("\n emmc : resp_busy");
			for(i = 0; i < 0x100000; i ++){
				emmc_resp_dbg("\nstatus[0x%x] Rintsts[0x%x]",readl(host->ioaddr + MSHCI_STATUS),readl(host->ioaddr + MSHCI_RINTSTS));
				if(!(readl(host->ioaddr + MSHCI_STATUS) & (DATA_MCBUSY | DATA_BUSY))) //not sure whether to check 
					break;						//this bit or DTO bit.		
			}
			emmc_resp_dbg("\n emmc : resp_busy end");

			if(i == 0x100000){
				printf("\nemmc : Card is not responding!");
				return TIMEOUT; 
			}	
		}	




	

	if(data){
#if 0		
	if(readl(host->ioaddr + MSHCI_STATUS) & (DATA_BUSY | DATA_MCBUSY)){
		printf("\n emmc :  card busy, CMD%d, status[0x%x]",cmd->opcode,readl(host->ioaddr + MSHCI_STATUS));
		mdelay(100);
		if(readl(host->ioaddr + MSHCI_STATUS) & (DATA_BUSY | DATA_MCBUSY)){//after reset again check
			return TIMEOUT;
		}
	} 
#endif
		if(data->flags & MMC_DATA_WRITE){
			emmc_dbg("\nemmc: DATA WRITE\n");
			mshci_writeto_fifo(host,data);	
		}
		else if(data->flags & MMC_DATA_READ){
			mshci_read_fifo(host, data);
		}
		else{
			printf("\n emmc: Error, no write or read set for data!");
		}			
                timeout--;
                mdelay(1);
		}
	#if 0	
		writel(ctrl | FIFO_RESET | CTRL_RESET, host->ioaddr + MSHCI_CTRL);
		timeout = 100;
		while (readl(host->ioaddr + MSHCI_CTRL) & (FIFO_RESET | CTRL_RESET)) {
		printf("!\n "); 
                if (timeout == 0) {
                        emmc_dbg("emmc !! reset did not happen!!\n");
                }
	}		
	#endif
	
//	#endif

	//now, what to do if data was also there.
	//
	#if 0
	if(data){
		intsts = mshci_readl(host, MSHCI_IDSTS);
                if (intmask) {
                        mshci_writel(host, intmask,MSHCI_IDSTS);
                        mshci_data_irq(host, intmask, INT_SRC_IDMAC);



	}
	#endif

	writel((readl(host->ioaddr + MSHCI_CTRL) & ~INT_ENABLE),host->ioaddr + MSHCI_CTRL);
	writel(readl(host->ioaddr + MSHCI_INTMSK)& ~INTMSK_CDETECT, host->ioaddr + MSHCI_INTMSK);

	mdelay(1);
	emmc_dbg("\n emmc:command[%d] status register 0x%x",cmd->opcode,readl(host->ioaddr + MSHCI_STATUS));
			
	return 0;	

} 


static void mshci_clock_onoff(struct mshci_host *host, unsigned char val)
{
        int loop_count = 100000;

	emmc_fn;

        if (val) {
                writel((0x1<<0),host->ioaddr +  MSHCI_CLKENA);
                writel(0,host->ioaddr +  MSHCI_CMD);
                writel(CMD_ONLY_CLK, host->ioaddr + MSHCI_CMD);
                do {
                        if (!(readl(host->ioaddr + MSHCI_CMD) & CMD_STRT_BIT))
                                break;
                        loop_count--;
                } while (loop_count);
        } else {
                writel((0x0<<0),host->ioaddr +  MSHCI_CLKENA);
                writel(0,host->ioaddr +  MSHCI_CMD);
                writel(CMD_ONLY_CLK,host->ioaddr + MSHCI_CMD);
                do {
                        if (!(readl(host->ioaddr + MSHCI_CMD) & CMD_STRT_BIT))
                                break;
                        loop_count--;
                } while (loop_count);
        }
        if (loop_count == 0) {
                printf("emmc: clock %s fail!",val? "ON":"OFF");
        }
}

static void mshci_set_clock(struct mshci_host *host,unsigned int clock)
{
	int loop_count;
	int div;
	emmc_fn;

//	clock = 400000; //as of now just use 400kHz
	   /* befor changing clock. clock needs to be off. */
        mshci_clock_onoff(host, CLK_DISABLE);
	host->max_clk = 47000000; //mkh: need to check!!

        if (clock >= host->max_clk) {
                div = 0;
        } else {
                for (div = 1;div < 255;div++) {
                        if ((host->max_clk / (div<<1)) <= clock)
                                break;
                }
	}

        writel(div, host->ioaddr + MSHCI_CLKDIV);
	emmc_dbg("\n emmc div 0x%x, register 0x%x \n",div,readl(host->ioaddr + MSHCI_CLKDIV));
	
	
	emmc_dbg("\n CMD 0x%x \n",readl(host->ioaddr + MSHCI_CMD));
        writel(0,host->ioaddr + MSHCI_CMD);

	emmc_dbg("\n RINTSTS 0x%x \n",readl(host->ioaddr + MSHCI_RINTSTS));

#if 1
        writel(CMD_ONLY_CLK,host->ioaddr + MSHCI_CMD);
        loop_count = 10000000;

        do {
                if (!(readl(host->ioaddr + MSHCI_CMD) & CMD_STRT_BIT))
                        break;
                loop_count--;
        } while(loop_count);

        if (loop_count == 0) {
                printf("emmc clock_set failed.\n");
        }
	else{
		emmc_dbg("emmc clock set success!! \n");
	}
        writel(readl(host->ioaddr + MSHCI_CMD)&(~CMD_SEND_CLK_ONLY),
                                        host->ioaddr + MSHCI_CMD);
#endif
        mshci_clock_onoff(host, CLK_ENABLE);


}


static void s5p_emmc_set_ios(struct mmc *emmc)
{
	struct mshci_host *host = emmc->priv;
	unsigned int ctrl;
	int timeout;
	emmc_fn;
	emmc_dbg("bus width %d ; clock %d\n",emmc->bus_width, emmc->clock);
	
        if (emmc->bus_width == 4 /*MMC_BUS_WIDTH_4*/) {
                writel((0x1<<0),host->ioaddr +  MSHCI_CTYPE);
        }else{
		// support only 1 bit in u-boot.
	}
	mshci_set_clock(host, emmc->clock); 
	

	#if 0 //support only these 2 while testing now.
	 else if (emmc->bus_width == MMC_BUS_WIDTH_8_DDR) {
                writel((0x1<<16),host->ioaddr +  MSHCI_CTYPE);
                writel((0x1<<16),host->ioaddr +  MSHCI_UHS_REG);
        } else if (emmc->bus_width == MMC_BUS_WIDTH_4_DDR) {
                writel((0x1<<0),host->ioaddr +  MSHCI_CTYPE);
                writel((0x1<<16),host->ioaddr + MSHCI_UHS_REG);
        } else {
                writel((0x0<<0),host->ioaddr +  MSHCI_CTYPE);
        }
	#endif


}



static void mshci_reset(struct mshci_host *host, unsigned char flags)
{
	unsigned int timeout = 100;
	unsigned int ctrl = readl(host->ioaddr + MSHCI_CTRL);
	emmc_dbg("\nmshci_ctrl 0x%x \n",ctrl);
	
	emmc_fn;

	/* Remember to check reset function! */


	//writel(0x0,host->ioaddr +  MSHCI_PWREN);
//	writel((readl(host->ioaddr + MSHCI_CTRL) | flags), host->ioaddr + MSHCI_CTRL);
	writel(0x0,host->ioaddr + MSHCI_CTRL);
        writel(0x0,host->ioaddr + MSHCI_BYTCNT);
        writel(0x0,host->ioaddr + MSHCI_CMDARG);
        writel(0x0,host->ioaddr + MSHCI_CMD);
        writel(0x0,host->ioaddr + MSHCI_UHS_REG);
        writel(0x0,host->ioaddr +  MSHCI_CTYPE);
        writel(readl(host->ioaddr + MSHCI_IDSTS),host->ioaddr + MSHCI_IDSTS);


#if 1  //Controller reset :(
	writel(ctrl | CTRL_RESET, host->ioaddr + MSHCI_CTRL);
	while (readl(host->ioaddr + MSHCI_CTRL) & CTRL_RESET) {
                if (timeout == 0) {
                        emmc_dbg("emmc control reset did not happen!!\n");
                        return;
                }
                timeout--;
                mdelay(1);
        }
#endif
	writel(ctrl | FIFO_RESET, host->ioaddr + MSHCI_CTRL);
	timeout = 100;
	while (readl(host->ioaddr + MSHCI_CTRL) & FIFO_RESET) {
                if (timeout == 0) {
                        emmc_dbg("emmc fifo reset did not happen!!\n");
                        return;
                }
                timeout--;
                mdelay(1);
        }
	writel(ctrl | DMA_RESET, host->ioaddr + MSHCI_CTRL);
	timeout = 100;
	while (readl(host->ioaddr + MSHCI_CTRL) & DMA_RESET) {
                if (timeout == 0) {
                        emmc_dbg("emmc DMA reset did not happen!!\n");
                        return;
                }
                timeout--;
                mdelay(1);
        }
	#if 0
	writel(ctrl | CTRL_RESET, host->ioaddr + MSHCI_CTRL);
	timeout = 100;
	while (readl(host->ioaddr + MSHCI_CTRL) & CTRL_RESET) {
	printf("!\n "); 
              if (timeout == 0) {
                 emmc_dbg("emmc fifo reset did not happen!!\n");
        	 return;
              }
              timeout--;
              mdelay(1);
	}	
	writel(ctrl | CTRL_RESET, host->ioaddr + MSHCI_CTRL);
		timeout = 100;
		while (readl(host->ioaddr + MSHCI_CTRL) & CTRL_RESET) {
		printf("!\n "); 
                if (timeout == 0) {
                        emmc_dbg("emmc fifo reset did not happen!!\n");
                        return;
                }
                timeout--;
                mdelay(1);
		}	
	#endif
       	//writel((0x1<<0),host->ioaddr +  MSHCI_CTYPE); //set 4bit width!! just to check
	//writel(0x1,host->ioaddr +  MSHCI_PWREN);
	//mdelay(100);
}


static int mshci_init(struct mshci_host *host)
{
	unsigned int intmask = 0;

	emmc_fn;
	
//	mshci_reset(host, CTRL_RESET | DMA_RESET | FIFO_RESET);
	
	//clear interrupts
	writel(INTMSK_ALL, host->ioaddr + MSHCI_RINTSTS);

	intmask |= (INTMSK_CDETECT | INTMSK_RE |
                INTMSK_CDONE | INTMSK_DTO | INTMSK_TXDR | INTMSK_RXDR |
                INTMSK_RCRC | INTMSK_DCRC | INTMSK_RTO | INTMSK_DRTO |
                INTMSK_HTO | INTMSK_FRUN | INTMSK_HLE | INTMSK_SBE |
                INTMSK_EBE);
	emmc_dbg("\n emmc int mask 0x%x\n",intmask);

	writel(intmask, host->ioaddr + MSHCI_INTMSK);
	 
}


static int s5p_emmc_init(struct mmc *emmc)
{
	

	struct mshci_host *host = emmc->priv;
	emmc_fn;
#if 0 //try resetting hardware to debug controller reset.
	writel(readl(host->ioaddr + HCLK_0_GATE_OFFSET) & ~(1<<16),host->ioaddr + HCLK_0_GATE_OFFSET);
	writel(readl(host->ioaddr + SCLK_0_GATE_OFFSET) & ~(1<<30),host->ioaddr + SCLK_0_GATE_OFFSET);
	writel(0x0,host->ioaddr +  MSHCI_PWREN);
	writel(0x22222222,ELFIN_GPIO_BASE + GPGPUD_OFFSET);
	mdelay(500);
	writel(0x55555555,ELFIN_GPIO_BASE + GPGPUD_OFFSET);
	mdelay(500);
	writel(0x00000000,ELFIN_GPIO_BASE + GPGPUD_OFFSET);
	writel(readl(host->ioaddr + HCLK_0_GATE_OFFSET) | (1<<16),host->ioaddr + HCLK_0_GATE_OFFSET);
	writel(readl(host->ioaddr + SCLK_0_GATE_OFFSET) | (1<<30),host->ioaddr + SCLK_0_GATE_OFFSET);

#endif
	mshci_reset(host, MSHCI_RESET_ALL);
	writel(0x1,host->ioaddr +  MSHCI_PWREN);
	host->version = 0;
	host->version = readw(host->ioaddr + MSHCI_VERID);
	mshci_init(host);
	mshci_set_clock(host,400000);
	initialized = 0;	
	
	return 0;
}


static int s5p_emmc_initialize(int channel)
{
	struct mmc *emmc;
	int ret;

	emmc_fn;
	emmc = &EMMC[channel];	
	//first finalize our functions
	emmc->priv = &emmc_host[channel];
	emmc->send_cmd = s5p_emmc_send_command;
	emmc->set_ios = s5p_emmc_set_ios;
	emmc->init = s5p_emmc_init;

	//set voltage, frequency etc.
	emmc->voltages = /*MMC_VDD_29_30 | MMC_VDD_30_31 |*/ MMC_VDD_32_33 | MMC_VDD_33_34; //kernel emmc driver gives these //''
	emmc->host_caps = MMC_MODE_4BIT;// | MMC_MODE_HS; //''
	emmc->f_min = 400000;
        emmc->f_max = 25000000;

        emmc_host[channel].clock = 0;

	emmc_host[channel].ioaddr = (void*)ELFIN_EMMC_44_BASE;
	
	//set gpio
	writel(readl(ELFIN_GPIO_BASE + GPGCON_OFFSET) & ~(3<<28) | (2<<28), ELFIN_GPIO_BASE + GPGCON_OFFSET);	
	writel(readl(ELFIN_GPIO_BASE + SPCON_OFFSET) | (3<<26) | (3<<18), ELFIN_GPIO_BASE + SPCON_OFFSET);	
	writel(0x00222222,ELFIN_GPIO_BASE + GPGCON1_OFFSET);
	writel(0x00000000,ELFIN_GPIO_BASE + GPGPUD_OFFSET);
	//writel(readl(ELFIN_GPIO_BASE + SPCON_OFFSET) | (0x3<<26),ELFIN_GPIO_BASE + SPCON_OFFSET);
	ret = mmc_register(emmc);
	return ret;
}

int smdk_s5p_emmc_init(void)
{
	int ret;

	ret = s5p_emmc_initialize(0); //now only one channel emmc.
	return ret;
}




























