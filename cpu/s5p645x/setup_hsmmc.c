#include <common.h>
#include <regs.h>
#include <asm/io.h>
#include <mmc.h>
#include <s3c_hsmmc.h>

void setup_hsmmc_clock(void)
{
	u32 tmp;
	u32 clock;
	u32 i;

#ifdef OM_PIN
	if(OM_PIN == SDMMC_CHANNEL0) {
		printf("SDMMC_CHANNEL0\n");
                printf("envi a\n");
		/* MMC0 clock src = SCLKMPLL */
		tmp = CLK_SRC0_REG & ~(0x3<<18);
		CLK_SRC0_REG = tmp | (0x1<<18);

		/* MMC0 clock div */
		tmp = CLK_DIV1_REG & ~(0xf<<0);
		clock = get_MPLL_CLK() / (((CLK_DIV0_REG >> 4) & 1) + 1) / 1000000;
		for(i=0; i<0xf; i++)
		{
			//if((clock / (i+1)) <= 50) {
			if((clock / (i+1)) <= 21) {
				CLK_DIV1_REG = tmp | i<<0;
				break;
			}
		}
		printf("MPLL_CON_REG: %X \n", MPLL_CON_REG);
		printf("CLK_DIV0_REG: %X \n", CLK_DIV0_REG);
		printf("CLK_DIV1_REG: %X \n", CLK_DIV1_REG);
		return ;
	} else if (OM_PIN == SDMMC_CHANNEL1) {
		printf("SDMMC_CHANNEL1\n");
                 printf("envi b\n");
		/* MMC1 clock src = SCLKMPLL */
		tmp = CLK_SRC0_REG & ~(0x3<<20);
		CLK_SRC0_REG = tmp | (0x1<<20);

		/* MMC1 clock div */
		tmp = CLK_DIV1_REG & ~(0xf<<4);
		clock = get_MPLL_CLK() / (((CLK_DIV0_REG >> 4) & 1) + 1) / 1000000;
		for(i=0; i<0xf; i++)
		{
			//if((clock / (i+1)) <= 50) {
			if((clock / (i+1)) <= 21) {
				CLK_DIV1_REG = tmp | i<<4;
				break;
			}
		}
		printf("MPLL_CON_REG: %X \n", MPLL_CON_REG);
		printf("CLK_DIV0_REG: %X \n", CLK_DIV0_REG);
		printf("CLK_DIV1_REG: %X \n", CLK_DIV1_REG);
		return ;
	}
#endif

#ifdef USE_MMC0
	printf("SDMMC_CHANNEL0\n");
         printf("envi c\n");
	/* MMC0 clock src = SCLKMPLL */
	tmp = CLK_SRC0_REG & ~(0x3<<18);
	CLK_SRC0_REG = tmp | (0x1<<18);

	/* MMC0 clock div */
	tmp = CLK_DIV1_REG & ~(0xf<<0);
	clock = get_MPLL_CLK() / (((CLK_DIV0_REG >> 4) & 1) + 1) / 1000000;
	for(i=0; i<0xf; i++)
	{
		if((clock / (i+1)) <= 50) {
		//if((clock / (i+1)) <= 21) {
			CLK_DIV1_REG = tmp | i<<0;
			break;
		}
	}
	printf("MPLL_CON_REG: %X \n", MPLL_CON_REG);
	printf("CLK_DIV0_REG: %X \n", CLK_DIV0_REG);
	printf("CLK_DIV1_REG: %X \n", CLK_DIV1_REG);
#endif

#ifdef USE_MMC1
         printf("envi d\n");
	printf("SDMMC_CHANNEL1\n");
	/* MMC1 clock src = SCLKMPLL */
	tmp = CLK_SRC0_REG & ~(0x3<<20);
	CLK_SRC0_REG = tmp | (0x1<<20);
	
#ifdef CONFIG_EMMC_4_4
	CLK_SRC0_REG = CLK_SRC0_REG & ~(7<<6) | (0x1 << 6);
#endif

	/* MMC1 clock div */
	tmp = CLK_DIV1_REG & ~(0xf<<4);
#ifdef CONFIG_EMMC_4_4
	tmp &= ~(0xf<<28);
#endif
	clock = get_MPLL_CLK() / (((CLK_DIV0_REG >> 4) & 1) + 1) / 1000000;
	for(i=0; i<0xf; i++)
	{
		if((clock / (i+1)) <= 50) {
		//if((clock / (i+1)) <= 21) {
			CLK_DIV1_REG = tmp | i<<4;
#ifdef CONFIG_EMMC_4_4
			CLK_DIV1_REG |= i<<28;
#endif
			break;
		}
	}
	printf("MPLL_CON_REG: %X \n", MPLL_CON_REG);
	printf("CLK_DIV0_REG: %X \n", CLK_DIV0_REG);
	printf("CLK_DIV1_REG: %X \n", CLK_DIV1_REG);
#endif

#ifdef USE_MMC2
	/* MMC2 clock src = SCLKMPLL */
	tmp = CLK_SRC0_REG & ~(0x3<<22);
	CLK_SRC0_REG = tmp | (0x1<<22);

	/* MMC2 clock div */
	tmp = CLK_DIV1_REG & ~(0xf<<8);
	clock = get_MPLL_CLK()/1000000;
	for(i=0; i<0xf; i++)
	{
		//if((clock / (i+1)) <= 50) {
		if((clock / (i+1)) <= 21) {
			CLK_DIV1_REG = tmp | i<<8;
			break;
		}
	}
#endif
}

/*
 * this will set the GPIO for hsmmc ch0
 * GPG0[0:6] = CLK, CMD, DAT[0:3], CDn
 */
void setup_hsmmc0_cfg_gpio(void)
{
	ulong reg;

        reg =  readl(ELFIN_GPIO_BASE+SPCON_OFFSET);
	reg &= ~(1 << 26);
        writel(reg,(ELFIN_GPIO_BASE+SPCON_OFFSET));
	printf("SPCON_REG: %X \n", reg);
     //   writel(0xbbcd1500,SPCON_OFFSET);
#ifdef OM_PIN
	if(OM_PIN == SDMMC_CHANNEL0) {
		/* 7 pins will be assigned - GPG0[0:6] = CLK, CMD, DAT[0:3], CDn */
		reg = readl(GPGCON) & 0xf0000000;
		writel(reg | 0x02222222, GPGCON);
		
		reg = readl(GPGPUD) & 0xffffc000;
		writel(reg | 0x00002000, GPGPUD);

		writel(reg | 0x0, GPGPUD);
		return ;
	} else if (OM_PIN == SDMMC_CHANNEL1) {
		reg = readl(GPHCON0) & 0x00000000;
		writel(reg | 0x22222222, GPHCON0);
	
		reg = readl(GPHCON1) & 0xFFFFFF00;
		writel(reg | 0x00000022, GPHCON1);

		writel(reg | 0x0, GPHPUD);
		return ;
	}
#endif

#ifdef USE_MMC0
	printf("GPIO_MMC0\n");
	/* 7 pins will be assigned - GPG0[0:6] = CLK, CMD, DAT[0:3], CDn */
	reg = readl(GPGCON) & 0xf0000000;
	writel(reg | 0x02222222, GPGCON);
	reg = readl(GPGPUD) & 0xffffc000;
	writel(reg | 0x00002000, GPGPUD);
	writel(reg | 0x0, GPGPUD);
#endif

#ifdef USE_MMC1
	printf("GPIO_MMC1\n");

	reg = readl(GPHCON0) & 0x00000000;
        writel(reg | 0x22222222, GPHCON0);

        reg = readl(GPHCON1) & 0xFFFFFF00;
        writel(reg | 0x00000022, GPHCON1);

        writel(0x0, GPHPUD); //PUD disabled

#endif
#ifdef USE_MMC2
	/* 7 pins will be assigned - GPG0[0:6] = CLK, CMD, DAT[0:3], CDn */
	reg = readl(GPCCON) & 0xff00ffff;
	writel(reg | 0x00330000, GPCCON);
	reg = readl(GPHCON0) & 0x00ffffff;
	writel(reg | 0x33000000, GPCCON);
	reg = readl(GPHCON1) & 0xff;
	writel(reg | 0x33, GPCCON);


	reg = readl(GPCPUD) & 0xfffff0ff;
	writel(reg, GPCPUD);
	reg = readl(GPCPUD) & 0xfffc0fff;
	writel(reg, GPCPUD);
#endif
}

void setup_sdhci0_cfg_card(struct sdhci_host *host)
{
	u32 ctrl2;
	u32 ctrl3;

	/* don't need to alter anything acording to card-type */
	writel(S3C64XX_SDHCI_CONTROL4_DRIVE_9mA, host->ioaddr + S3C64XX_SDHCI_CONTROL4);

	ctrl2 = readl(host->ioaddr + S3C_SDHCI_CONTROL2);

	ctrl2 |= (S3C64XX_SDHCI_CTRL2_ENSTAASYNCCLR |
		  S3C64XX_SDHCI_CTRL2_ENCMDCNFMSK |
		  //S3C_SDHCI_CTRL2_ENFBCLKTX |
		  S3C_SDHCI_CTRL2_ENFBCLKRX |		  
		  S3C_SDHCI_CTRL2_DFCNT_NONE |
		  S3C_SDHCI_CTRL2_ENCLKOUTHOLD);

	/*
	if(host->mmc->clock == 52*1000000)
		ctrl3 = 0;
	else
		ctrl3 = S3C_SDHCI_CTRL3_FCSEL0 | S3C_SDHCI_CTRL3_FCSEL1;
	*/
	ctrl3 = 0x7f5f3f1f
			| S3C_SDHCI_CTRL3_FCSEL0
			| S3C_SDHCI_CTRL3_FCSEL1;

	writel(ctrl2, host->ioaddr + S3C_SDHCI_CONTROL2);
	writel(ctrl3, host->ioaddr + S3C_SDHCI_CONTROL3);
}

