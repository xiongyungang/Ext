#include "stdafx.h"
#include "fcs16.h"

unsigned short pppfcs16(unsigned short fcs, unsigned char* cp, int len)

{
    while (len--)
        fcs = (fcs >> 8) ^ fcstab[(fcs ^ *cp++) & 0xff];

    return (fcs);
}

//fcsĞ£Ñé
void addfcs(unsigned char* pFrame, int inlen, int &outlen)
{
    unsigned short trialfcs = pppfcs16( PPPINITFCS16, pFrame, inlen);
    trialfcs ^= 0xffff;                 /* complement */
    pFrame[inlen] = (trialfcs & 0x00ff);      /* least significant byte first */
    pFrame[inlen+1] = ((trialfcs >> 8) & 0x00ff);
    outlen = inlen + 2;
}