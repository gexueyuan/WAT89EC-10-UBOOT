/*
 * Copyright (c) 2010 Samsung Electronics Co., Ltd.
 *              http://www.samsung.com/
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

int main (int argc, char *argv[])
{
	FILE		*fp;

	unsigned char	*Buf;
	int		BufLen;
	int		nbytes, fileLen, toBeReadLen;

	int		i;

//////////////////////////////////////////////////////////////
	if (argc != 4)
	{
		printf("Usage: size_adjust <source file> <destination file> <size> \n");
		return -1;
	}

//////////////////////////////////////////////////////////////
	BufLen = atoi(argv[3]);
	Buf = (char *)malloc(BufLen);
	memset(Buf, 0x00, BufLen);
	
//////////////////////////////////////////////////////////////
	fp = fopen(argv[1], "rb");
	if( fp == NULL)
	{
		printf("source file open error\n");
		free(Buf);
		return -1;
	}

	fseek(fp, 0L, SEEK_END);
	fileLen = ftell(fp);
	fseek(fp, 0L, SEEK_SET);

	memset(Buf, 0x00, BufLen);

	if (BufLen <= fileLen)
		toBeReadLen = BufLen;
	else
		toBeReadLen = fileLen;

	nbytes = fread(Buf, 1, toBeReadLen, fp);

	if ( nbytes != toBeReadLen )
	{
		printf("source file read error\n");
		free(Buf);
		fclose(fp);
		return -1;
	}

	fclose(fp);

//////////////////////////////////////////////////////////////
	fp = fopen(argv[2], "wb");
	if (fp == NULL)
	{
		printf("destination file open error\n");
		free(Buf);
		return -1;
	}

	nbytes	= fwrite( Buf, 1, BufLen, fp);

	if ( nbytes != BufLen )
	{
		printf("destination file write error\n");
		free(Buf);
		fclose(fp);
		return -1;
	}

	free(Buf);
	fclose(fp);

	return 0;
}
