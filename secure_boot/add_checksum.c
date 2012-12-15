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
	char		*Buf, *a;
	int		BufLen;
	int		nbytes, fileLen;
	unsigned int	checksum;
	int		i;

//////////////////////////////////////////////////////////////
	if (argc != 3)
	{
		printf("Usage: add_checksum <source file> <destination file>\n");
		return -1;
	}

	
//////////////////////////////////////////////////////////////
	fp = fopen(argv[1], "rb");
	if( fp == NULL)
	{
		printf("source file open error\n");
		return -1;
	}

	fseek(fp, 0L, SEEK_END);
	fileLen = ftell(fp);
	fseek(fp, 0L, SEEK_SET);

//////////////////////////////////////////////////////////////
	BufLen = fileLen;
	Buf = (char *)malloc(BufLen);
	memset(Buf, 0x00, BufLen);

	nbytes = fread(Buf, 1, BufLen, fp);

	if ( nbytes != BufLen )
	{
		printf("source file read error\n");
		free(Buf);
		fclose(fp);
		return -1;
	}

	fclose(fp);

//////////////////////////////////////////////////////////////
	a = Buf;
	for(i = 0, checksum = 0; i < BufLen - 4; i++)
		checksum += (0x000000FF) & *a++;

	a = Buf + BufLen - 4;	
	*( (unsigned int *)a ) = checksum;

//////////////////////////////////////////////////////////////
        fp = fopen(argv[2], "wb");
        if (fp == NULL)
        {
                printf("destination file open error\n");
                free(Buf);
                return -1;
        }

        a       = Buf;
        nbytes  = fwrite( a, 1, BufLen, fp);

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
