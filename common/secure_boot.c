#include <secure_boot.h>

/* int Verify_PSS_RSASignature2 (
 *	unsigned char	*rsaPublicKey,
 *	int		rsaPublicKeyLen,
 *	unsigned char	*msg,
 *	int		msgLen,
 *	unsigned char	*signature,
 *	int		signatureLen);
 */	
#define	macro_Verify_PSS_RSASignature2(BASE_FUNC_PTR,a,b,c,d,e,f) \
                        (((int(*)(unsigned char *, int, unsigned char *, int, unsigned char*, int))\
                        (*((unsigned int *)(BASE_FUNC_PTR + 48))))\
                        ((a),(b),(c),(d),(e),(f)))


/* Verify integrity of BL2(or OS) Image. */
int Check_Signature (SB20_CONTEXT *sb20_Context, unsigned char *codeImage, int codeImageLen,
                     unsigned char *signedData, int signedDataLen )
{
	unsigned int	rv;
	unsigned int	SBoot_BaseFunc_ptr;

	SBoot_BaseFunc_ptr = (unsigned int)sb20_Context->func_ptr_BaseAddr;

        /* 1. signature verification */
	rv = macro_Verify_PSS_RSASignature2(SBoot_BaseFunc_ptr, (unsigned char *)&(sb20_Context->stage2PubKey),
                                            sizeof(SB20_RSAPubKey), codeImage, codeImageLen, signedData, signedDataLen );
	if ( rv != SB_OK )
		return rv;
	
	return SB_OK;
}

