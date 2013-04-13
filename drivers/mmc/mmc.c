/*
 * Copyright 2008, Freescale Semiconductor, Inc
 * Andy Fleming
 *
 * Based vaguely on the Linux code
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

#include <config.h>
#include <common.h>
#include <command.h>
#include <mmc.h>
#include <part.h>
#include <malloc.h>
#include <linux/list.h>
#include <mmc.h>
#include <div64.h>

extern int get_active_device();
#ifdef CONFIG_EMMC_4_4
static int bootaccess;
#endif


#ifdef CONFIG_CMD_MOVINAND
extern int init_raw_area_table (block_dev_desc_t * dev_desc);
#endif

static struct list_head mmc_devices;
static int cur_dev_num = -1;

struct mmc *mmc_global[3] = {NULL, NULL, NULL};
struct mmc *mmc_default = NULL;

int mmc_send_cmd(struct mmc *mmc, struct mmc_cmd *cmd, struct mmc_data *data)
{
        //printf("envi 33\n");
	return mmc->send_cmd(mmc, cmd, data);
}

int mmc_set_blocklen(struct mmc *mmc, int len)
{
	struct mmc_cmd cmd;

	cmd.opcode = MMC_CMD_SET_BLOCKLEN;
	cmd.resp_type = MMC_RSP_R1;
	cmd.arg = len;
	cmd.flags = 0;

	return mmc_send_cmd(mmc, &cmd, NULL);
}

struct mmc *find_mmc_device(int dev_num)
{
	struct mmc *m;
	struct list_head *entry;

	list_for_each(entry, &mmc_devices) {
		m = list_entry(entry, struct mmc, link);

		if (m->block_dev.dev == dev_num)
			return m;
	}

	printf("MMC Device %d not found\n", dev_num);

	return NULL;
}
#define MAX_BLOCKS 0xffff

static ulong
mmc_bread(int dev_num, ulong start, lbaint_t blkcnt, void *dst)
{
	struct mmc *host = find_mmc_device(dev_num);
	int err;
	struct mmc_cmd cmd;
	struct mmc_data data;

	if (!host)
		return 0;
#ifdef CONFIG_EMMC_4_4	
	if(!bootaccess && dev_num == EMMC_DEV)
		emmc_init(host);
	
#endif
	/* We always do full block reads from the card */
	err = mmc_set_blocklen(host, (1<<9));
	if (err) {
		printf("mmc_set_blocklen failed !!!!\n");
		return 0;
	}

#if 0
        if (blkcnt > 1)
                cmd.opcode = MMC_CMD_READ_MULTIPLE_BLOCK;
        else
                cmd.opcode = MMC_CMD_READ_SINGLE_BLOCK;

        if (host->high_capacity)
                cmd.arg = start;
        else
                cmd.arg = start * (1<<9);

        cmd.resp_type = MMC_RSP_R1;
        cmd.flags = 0;

        data.dest = dst;
        data.blocks = blkcnt;
        data.blocksize = (1<<9);
        data.flags = MMC_DATA_READ;

        err = mmc_send_cmd(host, &cmd, &data);
        if (err) {
                printf("mmc read failed\n");
                return err;
        }
#else
	//fix for multiple transfer limit (0xffff(65535) blocks)
	int DataToTransfer, CardAddress, BufferAddress;
	
	DataToTransfer 	= blkcnt;
	CardAddress 	= start;
	BufferAddress 	= dst;
	
	while(1){

		if(DataToTransfer >= MAX_BLOCKS){
			printf(" More than <%d> blocks, spliting the transfer\n", MAX_BLOCKS );
			printf( "Card Address <%x> Transfer Size <%x> Buff Address <%x>\n", CardAddress, MAX_BLOCKS, BufferAddress );

			cmd.opcode = MMC_CMD_READ_MULTIPLE_BLOCK;

			if (host->high_capacity)
				cmd.arg = CardAddress;
			else
				cmd.arg = CardAddress * (1<<9);

			cmd.resp_type = MMC_RSP_R1;
			cmd.flags = 0;

			data.dest = BufferAddress;
			data.blocks = MAX_BLOCKS;
			data.blocksize = (1<<9);
			data.flags = MMC_DATA_READ;

			err = mmc_send_cmd(host, &cmd, &data);
			if (err) {
				printf("mmc read failed\n");
				return err;
			}

			CardAddress = CardAddress + MAX_BLOCKS;
			DataToTransfer = DataToTransfer - MAX_BLOCKS;
			BufferAddress = BufferAddress + MAX_BLOCKS * (1<<9);
		}
		else if(DataToTransfer >= 1 ){

			if (DataToTransfer > 1)
				cmd.opcode = MMC_CMD_READ_MULTIPLE_BLOCK;
			else
				cmd.opcode = MMC_CMD_READ_SINGLE_BLOCK;

			if (host->high_capacity)
				cmd.arg = CardAddress;
			else
				cmd.arg = CardAddress * (1<<9);

			cmd.resp_type = MMC_RSP_R1;
			cmd.flags = 0;

			data.dest = BufferAddress;
			data.blocks = DataToTransfer;
			data.blocksize = (1<<9);
			data.flags = MMC_DATA_READ;

			err = mmc_send_cmd(host, &cmd, &data);
			if (err) {
				printf("mmc read failed\n");
				return err;
			}
			break;
		}
		else
			break;

	} /* End of while(1)*/
#endif
#ifdef CONFIG_EMMC_4_4
	if(!bootaccess && dev_num == EMMC_DEV)
		emmc_deinit(host);
#endif
	return blkcnt;
}

static ulong
mmc_bwrite(int dev_num, ulong start, lbaint_t blkcnt, const void*src)
{
	struct mmc *host = find_mmc_device(dev_num);
	struct mmc_cmd cmd;
	struct mmc_data data;
	int err;

	if (!host)
		return 0;

#ifdef CONFIG_EMMC_4_4
	if(!bootaccess && dev_num == EMMC_DEV)
		emmc_init(host);
#endif
	err = mmc_set_blocklen(host, (1<<9));
	if (err) {
		printf("set write bl len failed\n");
		return err;
	}
	if (blkcnt > 1)
		cmd.opcode = MMC_CMD_WRITE_MULTIPLE_BLOCK;
	else
		cmd.opcode = MMC_CMD_WRITE_SINGLE_BLOCK;

	if (host->high_capacity)
		cmd.arg = start;
	else
		cmd.arg = start * (1<<9);

	cmd.resp_type = MMC_RSP_R1;
	cmd.flags = 0;

	data.src = src;
	data.blocks = blkcnt;
	data.blocksize = (1<<9);
	data.flags = MMC_DATA_WRITE;

	err = mmc_send_cmd(host, &cmd, &data);
	if (err) {
		printf("mmc write failed\n");
		return err;
	}
#ifdef CONFIG_EMMC_4_4
	if(!bootaccess && dev_num == EMMC_DEV)
		emmc_deinit(host);
#endif

#if defined(CONFIG_VOGUES)
	mmc_bread(dev_num, start, blkcnt, src);
#endif
	return blkcnt;
}

ulong movi_write(ulong start, lbaint_t blkcnt, void *src)
{
	int boot_device = get_active_device();
	if(boot_device < 0){
		printf("\n ERROR!! wrong boot device.");
		while(1);
	}
	return mmc_bwrite(boot_device, start, blkcnt, src);
}

ulong movi_read(ulong start, lbaint_t blkcnt, void *dst)
{
	int boot_device = get_active_device();
	if(boot_device < 0){
		printf("\n ERROR!! wrong boot device.");
		while(1);
	}
	return mmc_bread(boot_device, start, blkcnt, dst);
}

int mmc_go_idle(struct mmc* host)
{
	struct mmc_cmd cmd;
	int err;

	udelay(1000);

	cmd.opcode = MMC_CMD_GO_IDLE_STATE;
	cmd.arg = 0;
	cmd.resp_type = MMC_RSP_NONE;
	cmd.flags = 0;

	err = mmc_send_cmd(host, &cmd, NULL);

	if (err)
		return err;

	udelay(2000);

	return 0;
}

int mmc_send_app_op_cond(struct mmc *host)
{
	int timeout = 100;
	int err;
	struct mmc_cmd cmd;

	do {
		cmd.opcode = MMC_CMD_APP_CMD;
		cmd.resp_type = MMC_RSP_R1;
		cmd.arg = 0;
		cmd.flags = 0;

		err = mmc_send_cmd(host, &cmd, NULL);

		if (err)
			return err;

		cmd.opcode = SD_APP_OP_COND;
		cmd.resp_type = MMC_RSP_R3;
		cmd.arg = host->voltages;

		if (host->version == SD_VERSION_2)
			cmd.arg |= OCR_HCS;

		err = mmc_send_cmd(host, &cmd, NULL);

		if (err)
			return err;

		udelay(10000);
	} while ((!(cmd.resp[0] & OCR_BUSY)) && timeout--);

	if (timeout <= 0)
		return UNUSABLE_ERR;

	if (host->version != SD_VERSION_2)
		host->version = SD_VERSION_1_0;

	host->ocr = cmd.resp[0];
	host->high_capacity = ((host->ocr & OCR_HCS) == OCR_HCS);
	host->rca = 0;

	return 0;
}

int mmc_send_op_cond(struct mmc *mmc)
{
	int timeout = 1000;
	struct mmc_cmd cmd;
	int err;

	/* Some cards seem to need this */
	mmc_go_idle(mmc);

	do {
		cmd.opcode = MMC_CMD_SEND_OP_COND;
		cmd.resp_type = MMC_RSP_R3;
		cmd.arg = OCR_HCS | mmc->voltages;
		cmd.flags = 0;

		err = mmc_send_cmd(mmc, &cmd, NULL);
                //printf("envi 3+++\n");
		if (err)
			return err;
	
		udelay(1000);
	} while (!(cmd.resp[0] & OCR_BUSY) && timeout--);

	if (timeout <= 0)
		return UNUSABLE_ERR;

	mmc->version = MMC_VERSION_UNKNOWN;
	mmc->ocr = cmd.resp[0];

	mmc->high_capacity = ((mmc->ocr & OCR_HCS) == OCR_HCS);
	mmc->rca = 0;

	return 0;
}

int mmc_send_ext_csd(struct mmc *mmc, u8 *ext_csd)
{
	struct mmc_cmd cmd;
	struct mmc_data data;
	int err;

	/* Get the Card Status Register */
	cmd.opcode = MMC_CMD_SEND_EXT_CSD;
	cmd.resp_type = MMC_RSP_R1;
	cmd.arg = 0;
	cmd.flags = 0;

	data.dest = ext_csd;
	data.blocks = 1;
	data.blocksize = 512;
	data.flags = MMC_DATA_READ;

	err = mmc_send_cmd(mmc, &cmd, &data);

	return err;
}

int mmc_set_relative_addr(struct mmc *host)
{
	int err;
	struct mmc_cmd cmd;

	cmd.opcode = MMC_SET_RELATIVE_ADDR;
	cmd.arg = host->rca << 16;
	cmd.flags = MMC_RSP_R1;

	err = mmc_send_cmd(host, &cmd, NULL);

	return err;
}

int mmc_switch(struct mmc *host, u8 set, u8 index, u8 value)
{
	int ret;
	struct mmc_cmd cmd;

	cmd.opcode = MMC_CMD_SWITCH;
	cmd.resp_type = MMC_RSP_R1b;
	cmd.arg = (MMC_SWITCH_MODE_WRITE_BYTE << 24) |
		(index << 16) |
		(value << 8);
	cmd.flags = 0;

	ret = mmc_send_cmd(host, &cmd, NULL);

	#ifdef CONFIG_EMMC_4_4
	//check status after sending R1b type commands.
	do{
           cmd.opcode = MMC_CMD_SEND_STATUS;
           cmd.resp_type =  MMC_RSP_R1;
           cmd.arg = host->rca << 16;
           ret = mmc_send_cmd(host,&cmd, NULL);
        }while(((cmd.resp[0] & 0x00001E00) >> 9) == 7);
	#endif
	
	return ret;
}

int mmc_change_freq(struct mmc *mmc)
{
	u8 ext_csd[512];
	char cardtype;
	int err;

	mmc->card_caps = 0;

	/* Only version 4 supports high-speed */
	if (mmc->version < MMC_VERSION_4)
		return 0;

	mmc->card_caps |= MMC_MODE_4BIT;

	err = mmc_send_ext_csd(mmc, ext_csd);

	if (err)
		return err;

	if (ext_csd[212] || ext_csd[213] || ext_csd[214] || ext_csd[215])
		mmc->high_capacity = 1;

	cardtype = ext_csd[196] & 0xf;

	err = mmc_switch(mmc, EXT_CSD_CMD_SET_NORMAL, EXT_CSD_HS_TIMING, 1);
	if (err)
		return err;

	/* Now check to see that it worked */
	err = mmc_send_ext_csd(mmc, ext_csd);

	if (err)
		return err;

	/* No high-speed support */
	if (!ext_csd[185])
		return 0;

	/* High Speed is set, there are two types: 52MHz and 26MHz */
	if (cardtype & MMC_HS_52MHZ)
		mmc->card_caps |= MMC_MODE_HS_52MHz | MMC_MODE_HS;
	else
		mmc->card_caps |= MMC_MODE_HS;

	return 0;
}

int sd_switch(struct mmc *mmc, int mode, int group, u8 value, u8 *resp)
{
	struct mmc_cmd cmd;
	struct mmc_data data;

	/* Switch the frequency */
	cmd.opcode = SD_SWITCH;
	cmd.resp_type = MMC_RSP_R1;
	cmd.arg = (mode << 31) | 0xffffff;
	cmd.arg &= ~(0xf << (group * 4));
	cmd.arg |= value << (group * 4);
	cmd.flags = 0;

	data.dest = resp;
	data.blocksize = 64;
	data.blocks = 1;
	data.flags = MMC_DATA_READ;

	return mmc_send_cmd(mmc, &cmd, &data);
}


int sd_change_freq(struct mmc *mmc)
{
	int err;
	struct mmc_cmd cmd;
	uint scr[2];
	uint switch_status[16];
	struct mmc_data data;
	int timeout;

	mmc->card_caps = 0;

	/* Read the SCR to find out if this card supports higher speeds */
	cmd.opcode = MMC_CMD_APP_CMD;
	cmd.resp_type = MMC_RSP_R1;
	cmd.arg = mmc->rca << 16;
	cmd.flags = 0;

	err = mmc_send_cmd(mmc, &cmd, NULL);

	if (err)
		return err;

	cmd.opcode = SD_APP_SEND_SCR;
	cmd.resp_type = MMC_RSP_R1;
	cmd.arg = 0;
	cmd.flags = 0;

	timeout = 3;

retry_scr:
	data.dest = (u8 *)&scr;
	data.blocksize = 8;
	data.blocks = 1;
	data.flags = MMC_DATA_READ;

	err = mmc_send_cmd(mmc, &cmd, &data);

	if (err) {
		if (timeout--)
			goto retry_scr;

		return err;
	}

	mmc->scr[0] = __be32_to_cpu(scr[0]);
	mmc->scr[1] = __be32_to_cpu(scr[1]);

	switch ((mmc->scr[0] >> 24) & 0xf) {
		case 0:
			mmc->version = SD_VERSION_1_0;
			break;
		case 1:
			mmc->version = SD_VERSION_1_10;
			break;
		case 2:
			mmc->version = SD_VERSION_2;
			break;
		default:
			mmc->version = SD_VERSION_1_0;
			break;
	}

	/* Version 1.0 doesn't support switching */
	if (mmc->version == SD_VERSION_1_0)
		return 0;

	timeout = 4;
	while (timeout--) {
		err = sd_switch(mmc, SD_SWITCH_CHECK, 0, 1,
				(u8 *)&switch_status);

		if (err)
			return err;

		/* The high-speed function is busy.  Try again */
		if (!(__be32_to_cpu(switch_status[7]) & SD_HIGHSPEED_BUSY))
			break;
	}

//	if (mmc->scr[0] & SD_DATA_4BIT)
		mmc->card_caps |= MMC_MODE_4BIT;

	/* If high-speed isn't supported, we return */
	if (!(__be32_to_cpu(switch_status[3]) & SD_HIGHSPEED_SUPPORTED))
		return 0;

	err = sd_switch(mmc, SD_SWITCH_SWITCH, 0, 1, (u8 *)&switch_status);

	if (err)
		return err;

	if ((__be32_to_cpu(switch_status[4]) & 0x0f000000) == 0x01000000) {
		mmc->card_caps |= MMC_MODE_HS;
		mmc->clock = 50000000;
	} else {
		mmc->clock = 25000000;
	}

	return 0;
}

static const unsigned int tran_exp[] = {
	10000,		100000,		1000000,	10000000,
	0,		0,		0,		0
};

static const unsigned char tran_mant[] = {
	0,	10,	12,	13,	15,	20,	25,	30,
	35,	40,	45,	50,	55,	60,	70,	80,
};

static const unsigned int tacc_exp[] = {
	1,	10,	100,	1000,	10000,	100000,	1000000, 10000000,
};

static const unsigned int tacc_mant[] = {
	0,	10,	12,	13,	15,	20,	25,	30,
	35,	40,	45,	50,	55,	60,	70,	80,
};

#define UNSTUFF_BITS(resp,start,size)					\
	({								\
		const int __size = size;				\
		const u32 __mask = (__size < 32 ? 1 << __size : 0) - 1;	\
		const int __off = 3 - ((start) / 32);			\
		const int __shft = (start) & 31;			\
		u32 __res;						\
									\
		__res = resp[__off] >> __shft;				\
		if (__size + __shft > 32)				\
			__res |= resp[__off-1] << ((32 - __shft) % 32);	\
		__res & __mask;						\
	})

/*
 * Given a 128-bit response, decode to our card CSD structure. for SD
 * every
 */
static int sd_decode_csd(struct mmc *host)
{
	struct mmc_csd csd_org, *csd;
	unsigned int e, m, csd_struct;
	u32 *resp = host->csd;

	csd = &csd_org;

	csd_struct = UNSTUFF_BITS(resp, 126, 2);

	switch (csd_struct) {
	case 0:
		m = UNSTUFF_BITS(resp, 115, 4);
		e = UNSTUFF_BITS(resp, 112, 3);
		csd->tacc_ns	 = (tacc_exp[e] * tacc_mant[m] + 9) / 10;
		csd->tacc_clks	 = UNSTUFF_BITS(resp, 104, 8) * 100;

		m = UNSTUFF_BITS(resp, 99, 4);
		e = UNSTUFF_BITS(resp, 96, 3);
		csd->max_dtr	  = tran_exp[e] * tran_mant[m];
		csd->cmdclass	  = UNSTUFF_BITS(resp, 84, 12);

		e = UNSTUFF_BITS(resp, 47, 3);
		m = UNSTUFF_BITS(resp, 62, 12);
		csd->capacity	  = (1 + m) << (e + 2);

		csd->read_blkbits = UNSTUFF_BITS(resp, 80, 4);
		csd->read_partial = UNSTUFF_BITS(resp, 79, 1);
		csd->write_misalign = UNSTUFF_BITS(resp, 78, 1);
		csd->read_misalign = UNSTUFF_BITS(resp, 77, 1);
		csd->r2w_factor = UNSTUFF_BITS(resp, 26, 3);
		csd->write_blkbits = UNSTUFF_BITS(resp, 22, 4);
		csd->write_partial = UNSTUFF_BITS(resp, 21, 1);
		host->read_bl_len = (1<<9);
		host->write_bl_len = (1<<9);
		host->capacity = csd->capacity<<(csd->read_blkbits - 9);
		break;
	case 1:
		/*
		 * This is a block-addressed SDHC card. Most
		 * interesting fields are unused and have fixed
		 * values. To avoid getting tripped by buggy cards,
		 * we assume those fixed values ourselves.
		 */

		csd->tacc_ns	 = 0; /* Unused */
		csd->tacc_clks	 = 0; /* Unused */

		m = UNSTUFF_BITS(resp, 99, 4);
		e = UNSTUFF_BITS(resp, 96, 3);
		csd->max_dtr	  = tran_exp[e] * tran_mant[m];
		csd->cmdclass	  = UNSTUFF_BITS(resp, 84, 12);

		m = UNSTUFF_BITS(resp, 48, 22);
		csd->capacity     = (1 + m) << 10;

		csd->read_blkbits = 9;
		csd->read_partial = 0;
		csd->write_misalign = 0;
		csd->read_misalign = 0;
		csd->r2w_factor = 4; /* Unused */
		csd->write_blkbits = 9;
		csd->write_partial = 0;

		host->high_capacity = 1;
		host->read_bl_len = (1<<9);
		host->write_bl_len = (1<<9);
		host->capacity = csd->capacity;
		break;
	default:
		printf("unrecognised CSD structure version %d\n"
			, csd_struct);
		return -1;
	}

	return 0;
}

#if 0
/*
 * Given the decoded CSD structure, decode the raw CID to our CID structure.
 */
static int mmc_decode_cid(struct mmc_card *card)
{
	u32 *resp = card->raw_cid;

	/*
	 * The selection of the format here is based upon published
	 * specs from sandisk and from what people have reported.
	 */
	switch (card->csd.mmca_vsn) {
	case 0: /* MMC v1.0 - v1.2 */
	case 1: /* MMC v1.4 */
		card->cid.manfid	= UNSTUFF_BITS(resp, 104, 24);
		card->cid.prod_name[0]	= UNSTUFF_BITS(resp, 96, 8);
		card->cid.prod_name[1]	= UNSTUFF_BITS(resp, 88, 8);
		card->cid.prod_name[2]	= UNSTUFF_BITS(resp, 80, 8);
		card->cid.prod_name[3]	= UNSTUFF_BITS(resp, 72, 8);
		card->cid.prod_name[4]	= UNSTUFF_BITS(resp, 64, 8);
		card->cid.prod_name[5]	= UNSTUFF_BITS(resp, 56, 8);
		card->cid.prod_name[6]	= UNSTUFF_BITS(resp, 48, 8);
		card->cid.hwrev		= UNSTUFF_BITS(resp, 44, 4);
		card->cid.fwrev		= UNSTUFF_BITS(resp, 40, 4);
		card->cid.serial	= UNSTUFF_BITS(resp, 16, 24);
		card->cid.month		= UNSTUFF_BITS(resp, 12, 4);
		card->cid.year		= UNSTUFF_BITS(resp, 8, 4) + 1997;
		break;

	case 2: /* MMC v2.0 - v2.2 */
	case 3: /* MMC v3.1 - v3.3 */
	case 4: /* MMC v4 */
		card->cid.manfid	= UNSTUFF_BITS(resp, 120, 8);
		card->cid.oemid		= UNSTUFF_BITS(resp, 104, 16);
		card->cid.prod_name[0]	= UNSTUFF_BITS(resp, 96, 8);
		card->cid.prod_name[1]	= UNSTUFF_BITS(resp, 88, 8);
		card->cid.prod_name[2]	= UNSTUFF_BITS(resp, 80, 8);
		card->cid.prod_name[3]	= UNSTUFF_BITS(resp, 72, 8);
		card->cid.prod_name[4]	= UNSTUFF_BITS(resp, 64, 8);
		card->cid.prod_name[5]	= UNSTUFF_BITS(resp, 56, 8);
		card->cid.serial	= UNSTUFF_BITS(resp, 16, 32);
		card->cid.month		= UNSTUFF_BITS(resp, 12, 4);
		card->cid.year		= UNSTUFF_BITS(resp, 8, 4) + 1997;
		break;

	default:
		printk(KERN_ERR "%s: card has unknown MMCA version %d\n",
			mmc_hostname(card->host), card->csd.mmca_vsn);
		return -EINVAL;
	}

	return 0;
}
#endif

/*
 * Given a 128-bit response, decode to our card CSD structure.
 */
static int mmc_decode_csd(struct mmc *host)
{
	struct mmc_csd csd_org, *csd;
	unsigned int e, m, csd_struct;
	u32 *resp = host->csd;

	csd = &csd_org;

	/*
	 * We only understand CSD structure v1.1 and v1.2.
	 * v1.2 has extra information in bits 15, 11 and 10.
	 */
	csd_struct = UNSTUFF_BITS(resp, 126, 2);
	//if (csd_struct != 1 && csd_struct != 2) {
	if (csd_struct != 1 && csd_struct != 2 && csd_struct != 3) {
		printf("unrecognised CSD structure version %d\n",
			csd_struct);
		return -1;
	}

	csd->mmca_vsn	 = UNSTUFF_BITS(resp, 122, 4);
	switch (csd->mmca_vsn) {
	case 0:
		host->version = MMC_VERSION_1_2;
		break;
	case 1:
		host->version = MMC_VERSION_1_4;
		break;
	case 2:
		host->version = MMC_VERSION_2_2;
		break;
	case 3:
		host->version = MMC_VERSION_3;
		break;
	case 4:
		host->version = MMC_VERSION_4;
		break;
	}

	m = UNSTUFF_BITS(resp, 115, 4);
	e = UNSTUFF_BITS(resp, 112, 3);
	csd->tacc_ns	 = (tacc_exp[e] * tacc_mant[m] + 9) / 10;
	csd->tacc_clks	 = UNSTUFF_BITS(resp, 104, 8) * 100;

	m = UNSTUFF_BITS(resp, 99, 4);
	e = UNSTUFF_BITS(resp, 96, 3);
	csd->max_dtr	  = tran_exp[e] * tran_mant[m];
	csd->cmdclass	  = UNSTUFF_BITS(resp, 84, 12);

	e = UNSTUFF_BITS(resp, 47, 3);
	m = UNSTUFF_BITS(resp, 62, 12);
	csd->capacity	  = (1 + m) << (e + 2);
	csd->read_blkbits = UNSTUFF_BITS(resp, 80, 4);
	csd->read_partial = UNSTUFF_BITS(resp, 79, 1);
	csd->write_misalign = UNSTUFF_BITS(resp, 78, 1);
	csd->read_misalign = UNSTUFF_BITS(resp, 77, 1);
	csd->r2w_factor = UNSTUFF_BITS(resp, 26, 3);
	csd->write_blkbits = UNSTUFF_BITS(resp, 22, 4);
	csd->write_partial = UNSTUFF_BITS(resp, 21, 1);

	host->read_bl_len = 1 << csd->read_blkbits;
	host->write_bl_len = 1 << csd->write_blkbits;
	host->capacity = csd->capacity;
	host->clock = 20000000;

	return 0;
}

/*
 * Read and decode extended CSD.
 */
static int mmc_read_ext_csd(struct mmc *host)
{
	int err;
	u8 *ext_csd;
	unsigned int ext_csd_struct;

	if (host->version < (MMC_VERSION_4 | MMC_VERSION_MMC))
		return 0;

	/*
	 * As the ext_csd is so large and mostly unused, we don't store the
	 * raw block in mmc_card.
	 */
	ext_csd = malloc(512);
	if (!ext_csd) {
		printf("could not allocate a buffer to "
			"receive the ext_csd.\n");
		return -1;
	}

	err = mmc_switch(host, EXT_CSD_CMD_SET_NORMAL, EXT_CSD_HS_TIMING, 0);
	if (err)
		return err;

	err = mmc_switch(host, EXT_CSD_CMD_SET_NORMAL,
				EXT_CSD_BUS_WIDTH,
				EXT_CSD_BUS_WIDTH_1);	
	if (err)
		return err;
	

	err = mmc_send_ext_csd(host, ext_csd);
	if (err) {
		/*
		 * High capacity cards should have this "magic" size
		 * stored in their CSD.
		 */
		if (host->capacity == (4096 * 512)) {
			printf("unable to read EXT_CSD "
				"on a possible high capacity card. "
				"Card will be ignored.\n");
		} else {
			printf("unable to read "
				"EXT_CSD, performance might suffer.\n");
			err = 0;
		}

		goto out;
	}

	ext_csd_struct = ext_csd[EXT_CSD_REV];
	
	if (ext_csd_struct > 5) {
		printf("unrecognised EXT_CSD structure "
			"version %d\n", ext_csd_struct);
		err = -1;
		goto out;
	}

	if (ext_csd_struct >= 2) {
		host->ext_csd.sectors =
			ext_csd[EXT_CSD_SEC_CNT + 0] << 0 |
			ext_csd[EXT_CSD_SEC_CNT + 1] << 8 |
			ext_csd[EXT_CSD_SEC_CNT + 2] << 16 |
			ext_csd[EXT_CSD_SEC_CNT + 3] << 24;
		if (host->ext_csd.sectors) {
			host->high_capacity = 1;
			host->capacity = host->ext_csd.sectors;
		}
	}

	switch (ext_csd[EXT_CSD_CARD_TYPE]) {
	case EXT_CSD_CARD_TYPE_52 | EXT_CSD_CARD_TYPE_26:
		host->ext_csd.hs_max_dtr = 26000000;
		host->clock = 26000000;
                //printf("envi read extend card\n");
		break;
	case EXT_CSD_CARD_TYPE_26:
		host->ext_csd.hs_max_dtr = 26000000;
		host->clock = 26000000;
		break;
	// 20100713
	case 7:
		host->ext_csd.hs_max_dtr = 26000000;
		host->clock = 26000000;
		break;
	default:
		/* MMC v4 spec says this cannot happen */
		printf("card is mmc v4 but doesn't "
			"support any high-speed modes.\n");
		goto out;
	}

out:
	free(ext_csd);

	return err;
}

void mmc_set_ios(struct mmc *mmc)
{
	mmc->set_ios(mmc);
}

void mmc_set_clock(struct mmc *mmc, uint clock)
{
	if (clock > mmc->f_max)
		clock = mmc->f_max;

	if (clock < mmc->f_min)
		clock = mmc->f_min;

	mmc->clock = clock;

	mmc_set_ios(mmc);
}

void mmc_set_bus_width(struct mmc *host, uint width)
{
	host->bus_width = width;

	mmc_set_ios(host);
}

int mmc_startup(struct mmc *host)
{
	int err;
	struct mmc_cmd cmd;

	/* Put the Card in Identify Mode */
	cmd.opcode = MMC_ALL_SEND_CID;
	cmd.resp_type = MMC_RSP_R2;
	cmd.arg = 0;
	cmd.flags = 0;

	err = mmc_send_cmd(host, &cmd, NULL);
	if (err)
		return err;

	memcpy(host->cid, cmd.resp, 16);

	/*
	 * For MMC cards, set the Relative Address.
	 * For SD cards, get the Relatvie Address.
	 * This also puts the cards into Standby State
	 */
	#ifdef CONFIG_EMMC_4_4
	 if(!IS_SD(host) && (host->block_dev.dev == EMMC_DEV)){
                host->rca = 0x0001;//(cmd.resp[0]>>32) & 0xffffffff ;
        }
	#endif

	cmd.opcode = SD_SEND_RELATIVE_ADDR;
	cmd.arg = host->rca << 16;
	cmd.resp_type = MMC_RSP_R6;
	cmd.flags = 0;

	err = mmc_send_cmd(host, &cmd, NULL);

	if (err)
		return err;

	if (IS_SD(host))
		host->rca = (cmd.resp[0] >> 16) & 0xffff;

	/* Get the Card-Specific Data */
	cmd.opcode = MMC_SEND_CSD;
	cmd.resp_type = MMC_RSP_R2;
	cmd.arg = host->rca << 16;
	cmd.flags = 0;

	err = mmc_send_cmd(host, &cmd, NULL);

	if (err)
		return err;

	memcpy(host->csd, cmd.resp, 4*4);
	if (IS_SD(host))
		sd_decode_csd(host);
	else
		mmc_decode_csd(host);

	/* Select the card, and put it into Transfer Mode */
	cmd.opcode = MMC_CMD_SELECT_CARD;
	cmd.resp_type = MMC_RSP_R1b;
	cmd.arg = host->rca << 16;
	cmd.flags = 0;
	err = mmc_send_cmd(host, &cmd, NULL);
	if (err)
		return err;

	if (IS_SD(host)) {
		err = sd_change_freq(host);
	}
	else {
		err = mmc_read_ext_csd(host);
		if (err)
			return err;
		err = mmc_change_freq(host);
	}

	if (err)
		return err;

	/* Restrict card's capabilities by what the host can do */
	host->card_caps &= host->host_caps;

	if (IS_SD(host)) {
		if (host->card_caps & MMC_MODE_4BIT) {
			cmd.opcode = MMC_CMD_APP_CMD;
			cmd.resp_type = MMC_RSP_R1;
			cmd.arg = host->rca << 16;
			cmd.flags = 0;

			err = mmc_send_cmd(host, &cmd, NULL);
			if (err)
				return err;

			cmd.opcode = SD_APP_SET_BUS_WIDTH;
			cmd.resp_type = MMC_RSP_R1;
			cmd.arg = 2;
			cmd.flags = 0;
			err = mmc_send_cmd(host, &cmd, NULL);
			if (err)
				return err;

			mmc_set_bus_width(host, 4);
		}

		if (host->card_caps & MMC_MODE_HS)
			host->clock = 50000000;
		else
			host->clock = 25000000;
	} else {
		if (host->card_caps & MMC_MODE_4BIT) {
			/* Set the card to use 4 bit*/
			err = mmc_switch(host, EXT_CSD_CMD_SET_NORMAL,
					EXT_CSD_BUS_WIDTH,
					EXT_CSD_BUS_WIDTH_4);

			if (err)
				return err;

			mmc_set_bus_width(host, 4);
		} else if (host->card_caps & MMC_MODE_8BIT) {
			/* Set the card to use 8 bit*/
			err = mmc_switch(host, EXT_CSD_CMD_SET_NORMAL,
					EXT_CSD_BUS_WIDTH,
					EXT_CSD_BUS_WIDTH_8);

			if (err)
				return err;

			mmc_set_bus_width(host, 8);
		} else {
			mmc_set_bus_width(host, 1);
		}
	}

	mmc_set_ios(host);

	/* fill in device description */
	host->block_dev.lun = 0;
	host->block_dev.type = 0;
	host->block_dev.blksz = (1<<9);
	host->block_dev.lba = host->capacity;
	sprintf(host->block_dev.vendor, "Man %06x Snr %08x", host->cid[0] >> 8,
			(host->cid[2] << 8) | (host->cid[3] >> 24));
	sprintf(host->block_dev.product, "%c%c%c%c%c", host->cid[0] & 0xff,
			(host->cid[1] >> 24), (host->cid[1] >> 16) & 0xff,
			(host->cid[1] >> 8) & 0xff, host->cid[1] & 0xff);
	sprintf(host->block_dev.revision, "%d.%d", host->cid[2] >> 28,
			(host->cid[2] >> 24) & 0xf);
	init_part(&host->block_dev);

#ifdef CONFIG_CMD_MOVINAND
	init_raw_area_table(&host->block_dev);
#endif

	return 0;
}

int mmc_send_if_cond(struct mmc *host)
{
	struct mmc_cmd cmd;
	int err;
	static const u8 test_pattern = 0xAA;

	/*
	 * To support SD 2.0 cards, we must always invoke SD_SEND_IF_COND
	 * before SD_APP_OP_COND. This command will harmlessly fail for
	 * SD 1.0 cards.
	 */
	cmd.opcode = SD_SEND_IF_COND;
	cmd.arg = ((host->voltages & 0xFF8000) != 0) << 8 | test_pattern;
	cmd.resp_type = MMC_RSP_R7;
	cmd.flags = 0;

	err = mmc_send_cmd(host, &cmd, NULL);

	if (err)
		return err;

	if ((cmd.resp[0] & 0xff) != 0xaa)
		return UNUSABLE_ERR;
	else
		host->version = SD_VERSION_2;

	return 0;
}

int mmc_register(struct mmc *mmc)
{
	/* Setup the universal parts of the block interface just once */
	mmc->block_dev.if_type = IF_TYPE_MMC;
	mmc->block_dev.dev = cur_dev_num++;
	mmc->block_dev.removable = 1;
	mmc->block_dev.block_read = mmc_bread;
	mmc->block_dev.block_write = mmc_bwrite;

	INIT_LIST_HEAD (&mmc->link);

	list_add_tail (&mmc->link, &mmc_devices);

	return 0;
}

block_dev_desc_t *mmc_get_dev(int dev)
{
	struct mmc *mmc = find_mmc_device(dev);

	return mmc ? &mmc->block_dev : NULL;
}

int mmc_init(struct mmc *host)
{
	int err;

	err = host->init(host);
        //printf("envi 1\n");

	if (err)
		return err;

	/* Reset the Card */
	err = mmc_go_idle(host);
         //printf("envi 2\n");
	if (err)
		return err;

	#ifdef CONFIG_EMMC_4_4 //controller is not fully sd 2.0 compliant. FSMs get busy with sd commands.
	if(host->block_dev.dev == EMMC_DEV){
		err = mmc_send_op_cond(host); 
		if (err){
			return UNUSABLE_ERR;
		}
			
	}
	else{
	#endif

	/* Test for SD version 2 */
	err = mmc_send_if_cond(host);

	/* Now try to get the SD card's operating condition */
	err = mmc_send_app_op_cond(host);

	/* If the command timed out, we check for an MMC card */
	if (err == TIMEOUT) {
		err = mmc_send_op_cond(host);
                 //printf("envi 3\n");
		if (err) {
			return UNUSABLE_ERR;
		}
                //printf("envi 3--\n");
	}
	else if(err) {
                 //printf("envi 4\n");
		return UNUSABLE_ERR;
	}
	#ifdef CONFIG_EMMC_4_4
	}
	#endif
         //printf("envi 5\n");
	return mmc_startup(host);
}

/*
 * CPU and board-specific MMC initializations.  Aliased function
 * signals caller to move on
 */
static int __def_mmc_init(bd_t *bis)
{
	return -1;
}

int cpu_mmc_init(bd_t *bis) __attribute__((weak, alias("__def_mmc_init")));
int board_mmc_init(bd_t *bis) __attribute__((weak, alias("__def_mmc_init")));

void print_mmc_devices(char separator)
{
	struct mmc *m;
	struct list_head *entry;

	list_for_each(entry, &mmc_devices) {
		m = list_entry(entry, struct mmc, link);

		m->init(m);
		
		printf("%s_dev%d", m->name, m->block_dev.dev);

		if (entry->next != &mmc_devices)
			printf("%c ", separator);
	}

	printf("\n");
}

int mmc_initialize(bd_t *bis)
{
	struct mmc *mmc;
	int err;
	int mmc_boot_device;
	
	INIT_LIST_HEAD (&mmc_devices);
	cur_dev_num = 0;

	if (board_mmc_init(bis) < 0) {
		cpu_mmc_init(bis);
	}

#if defined(DEBUG_S3C_HSMMC)
	print_mmc_devices(',');
#endif

#ifdef CONFIG_EMMC_4_4
	mmc = find_mmc_device(EMMC_DEV);
	if(mmc)
		err = emmc_init(mmc); //initialize emmc and init partition table too.
	if(err){
		printf("emmc init failed %d\n",err);
	}
	else{
		printf("\nemmc %dMB\n", ((mmc->capacity)/(1024*1024)*512));	
	}
#endif
//	mmc_boot_device = get_active_device();
	mmc = find_mmc_device(0);
	if (mmc) {
		err = mmc_init(mmc);
		if (err) {
			if(EMMC_DEV != get_boot_device()){
				set_active_device(0);	
			}
			
			printf("Card init fail!\n");
			return err;
		}
		printf("SD/MMC : %dMB\n", (mmc->capacity/(1024*1024/mmc->read_bl_len)));
	}
	return 0;
}

#if defined (CONFIG_EMMC)
int emmc_boot_partition_size_change(struct mmc *mmc, u32 bootsize, u32 rpmbsize)
{
	int err;
	struct mmc_cmd cmd;

	/* Only use this command for raw eMMC moviNAND */
	/* Enter backdoor mode */
	cmd.opcode = 62;
	cmd.resp_type = MMC_RSP_R1b;
	cmd.arg = 0xefac62ec;
	cmd.flags = 0;
	
	err = mmc_send_cmd(mmc, &cmd, NULL);
	udelay(1000);
	if (err)
		return err;
	
	/* Boot partition changing mode */
	cmd.opcode = 62;
	cmd.resp_type = MMC_RSP_R1b;
	cmd.arg = 0xcbaea7;
	cmd.flags = 0;
	
	err = mmc_send_cmd(mmc, &cmd, NULL);
	udelay(1000);
	if (err)
		return err;

	bootsize = ((bootsize*1024)/128);

	/* Arg: boot partition size */
	cmd.opcode = 62;
	cmd.resp_type = MMC_RSP_R1b;
	cmd.arg = bootsize;
	cmd.flags = 0;

	err = mmc_send_cmd(mmc, &cmd, NULL);
	udelay(1000);
	if (err)
		return err;

	rpmbsize = ((rpmbsize*1024)/128);

	/* Arg: RPMB partition size */
	cmd.opcode = 62;
	cmd.resp_type = MMC_RSP_R1b;
	cmd.arg = rpmbsize;
	cmd.flags = 0;

	err = mmc_send_cmd(mmc, &cmd, NULL);
	udelay(1000);
	if (err)
		return err;



	return 0;	
}
#endif
#ifdef CONFIG_EMMC_4_4
int emmc_boot_open(struct mmc *mmc)
{
	int err;
	struct mmc_cmd cmd;
	u8 ext_csd[512];	
	bootaccess = 1;
	/* Boot ack enable, boot partition enable , boot partition access */
	cmd.opcode = MMC_CMD_SWITCH;
	cmd.resp_type = MMC_RSP_R1b;
	cmd.arg = ((3<<24)|(179<<16)|(((1<<6)|(1<<3)|(1<<0))<<8));
	cmd.flags = 0;

	err = mmc_send_cmd(mmc, &cmd, NULL);
	udelay(1000);
	if (err)
		return err;

	
        do{
           cmd.opcode = MMC_CMD_SEND_STATUS;
           cmd.resp_type =  MMC_RSP_R1;
           cmd.arg = mmc->rca << 16;
           err = mmc_send_cmd(mmc,&cmd, NULL);
           printf("\nCMD13 response 0x%x\n",cmd.resp[0]);
        }while(((cmd.resp[0] & 0x00001E00) >> 9) == 7);



	/* 4bit transfer mode at booting time. */
	cmd.opcode = MMC_CMD_SWITCH;
	cmd.resp_type = MMC_RSP_R1b;
	cmd.arg = ((3<<24)|(177<<16)|((1<<0)<<8));
	cmd.flags = 0;

	err = mmc_send_cmd(mmc, &cmd, NULL);
	udelay(1000);
	if (err)
		return err;

	err = mmc_send_ext_csd(mmc, ext_csd);	

	return 0;
}

int emmc_boot_close(struct mmc *mmc)
{
	int err;
	struct mmc_cmd cmd;

	bootaccess = 0;
	/* Boot ack enable, boot partition enable , boot partition access */
	cmd.opcode = MMC_CMD_SWITCH;
	cmd.resp_type = MMC_RSP_R1b;
	cmd.arg = ((3<<24)|(179<<16)|(((1<<6)|(1<<3)|(0<<0))<<8));
	cmd.flags = 0;

	err = mmc_send_cmd(mmc, &cmd, NULL);
	udelay(1000);
	if (err)
		return err;

	return 0;
}

int emmc_deinit(struct mmc *host){

	int err;	
	bootaccess = 0;
        err = mmc_go_idle(host);

        if (err)
        err = mmc_go_idle(host);
	return err;
}


int emmc_init(struct mmc *host)
{

        struct mmc_cmd cmd;
	u8 ext_csd[512];	
        int err = 0;
	unsigned long cap;
	static int table_created = 0;
	if(!table_created){
        	err = host->init(host);
        	if(err){
                	printf("\nemmc: host init failed");
                	return err;
        	}
	}

	bootaccess = 0;
        err = mmc_go_idle(host);

        if (err)
        err = mmc_go_idle(host);

        host->clock = 400000; //do a lot of hardcoding as of now
        host->bus_width = 1; //-do-
        mmc_set_ios(host);
	err = mmc_send_op_cond(host);
        if (err) {
          return UNUSABLE_ERR;
        }
        cmd.opcode = MMC_ALL_SEND_CID;
        cmd.resp_type = MMC_RSP_R2;
        cmd.arg = 0;
        cmd.flags = 0;

        err = mmc_send_cmd(host, &cmd, NULL);
        if (err)
                return err;
        memcpy(host->cid, cmd.resp, 16);
        host->rca = 0x0001;//(cmd.resp[0]>>32) & 0xffffffff; //spec says relative addrs shd be shorter than cid

	cmd.opcode = SD_SEND_RELATIVE_ADDR;
        cmd.arg = host->rca << 16;
        cmd.resp_type = MMC_RSP_R6;
        cmd.flags = 0;

        err = mmc_send_cmd(host, &cmd, NULL);
        if (err)
                return err;

        cmd.opcode = MMC_SEND_CSD;
        cmd.resp_type = MMC_RSP_R2;
        cmd.arg = host->rca << 16;
        cmd.flags = 0;

        err = mmc_send_cmd(host, &cmd, NULL);
        if (err)
                return err;
	memcpy(host->csd, cmd.resp, 4*4);

        cmd.opcode = MMC_CMD_SELECT_CARD;
        cmd.resp_type = MMC_RSP_R1b;
        cmd.arg = host->rca << 16;
        cmd.flags = 0;
        err = mmc_send_cmd(host, &cmd, NULL);
        if (err)
                return err;
         do{
                cmd.opcode = MMC_CMD_SEND_STATUS;
                cmd.resp_type =  MMC_RSP_R1;
                cmd.arg = host->rca << 16;
                err = mmc_send_cmd(host,&cmd, NULL);
        }while(((cmd.resp[0] & 0x00001E00) >> 9) == 7);

#if 1
	err = mmc_switch(host, EXT_CSD_CMD_SET_NORMAL,
				EXT_CSD_BUS_WIDTH,
				EXT_CSD_BUS_WIDTH_4);	
	if (err){
		printf("\nerr: emmc card switching to 4 bit %d",err);
		return err;
	}
	host->clock = 25000000;/*50000000*/; //do a lot of hardcoding as of now
        host->bus_width = 4; //-do-
        mmc_set_ios(host);
#endif
			
		if(!table_created)
		{
			table_created = 1;		
			mmc_decode_csd(host);
			err = mmc_send_ext_csd(host, ext_csd);
			if(err){
			printf("\n emmc:send ext csd failed %d\n",err);
			return err;
			}
			cap = (ext_csd[215]<<24 | ext_csd[214] << 16 | ext_csd[213]<<8 | ext_csd[212]);
			if(cap)
				host->capacity = cap; //if card is > 2GB, ext csd values SEC_COUNT Must be used to 
					       	      //calculate capacity. refer spec.

			/* fill in device description */
			host->block_dev.lun = 0;
			host->block_dev.type = 0;
			host->block_dev.blksz = (1<<9);
			host->block_dev.lba = host->capacity;
			sprintf(host->block_dev.vendor, "Man %06x Snr %08x", host->cid[0] >> 8,
					(host->cid[2] << 8) | (host->cid[3] >> 24));
			sprintf(host->block_dev.product, "%c%c%c%c%c", host->cid[0] & 0xff,
					(host->cid[1] >> 24), (host->cid[1] >> 16) & 0xff,
					(host->cid[1] >> 8) & 0xff, host->cid[1] & 0xff);
			sprintf(host->block_dev.revision, "%d.%d", host->cid[2] >> 28,
					(host->cid[2] >> 24) & 0xf);
			init_part(&host->block_dev);
#ifdef CONFIG_CMD_MOVINAND
			init_raw_area_table(&host->block_dev);
#endif
		}
        return err;

}
#endif //CONFIG_EMMC_4_4

