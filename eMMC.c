#include <stdio.h>
#include "xparameters.h"
#include "xil_exception.h"
#include "xsdps.h"
#include "xil_printf.h"
static XSdPs ps7_EMMC;
XSdPs_Config * EMMC_Config;
u8 WR_Buf[1024];
u8 RD_Buf[1024];
u8 Emmc_ExtCsd[1024];
int main()
{
u32 i;
s32 Status;
u32 Buffer_size=20;
u8 SD_ERROR=0;
EMMC_Config= XSdPs_LookupConfig(XPAR_PS7_SD_1_DEVICE_ID);
Status = XSdPs_CfgInitialize(&ps7_EMMC, EMMC_Config, EMMC_Config->BaseAddress);
if (Status != XST_SUCCESS)
{
print("EMMC Config failed !\n\r");
return XST_FAILURE;
}
Status=XSdPs_MmcCardInitialize(&ps7_EMMC);
if (Status != XST_SUCCESS)
{
print("EMMC Config failed !\n\r");
return XST_FAILURE;
}
Status=XSdPs_Change_ClkFreq(&ps7_EMMC,50000000);
Status=XSdPs_Select_Card(&ps7_EMMC);
Status=XSdPs_SetBlkSize(&ps7_EMMC,XSDPS_BLK_SIZE_512_MASK);
Status=XSdPs_Get_Mmc_ExtCsd(&ps7_EMMC,Emmc_ExtCsd);
for(i=0;i<Buffer_size;i++)
{
WR_Buf[i]=i+1;
}
Status = XSdPs_WritePolled(&ps7_EMMC, 0x05, 2, WR_Buf);
Status = XSdPs_ReadPolled(&ps7_EMMC, 0x05, 2, RD_Buf);
//-----------------------check data-----------------------------------------------
for(i=0; i<Buffer_size;i++)
{
xil_printf("%d\n",RD_Buf[i]);
}
if(SD_ERROR)
xil_printf("EMMC Data checked Error\r\n");
else
xil_printf("EMMC Data checked Successfully\r\n");
return 0;
}
