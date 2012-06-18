/*
**********************************************************************************************************************
*											        eGon
*						           the Embedded GO-ON Bootloader System
*									       eGON mod demo sub-system
*
*						  Copyright(C), 2006-2010, SoftWinners Microelectronic Co., Ltd.
*                                           All Rights Reserved
*
* File    : sdmmc_for_boot0.c
*
* By      : Neil
*
* Version : V1.00
*
* Date	  :
*
* Descript:
**********************************************************************************************************************
*/
#include  "sdmmc_for_boot0.h"
#include  "sdcard_op.h"
#include  "osal/sdmmc_osal_boot0.h"

smc_idma_des * pdes = NULL;
static  __s32  gpio_no, line_count;
/*
************************************************************************************************************
*
*                                             SDMMC_PhyInit
*
*    �������ƣ�
*
*    �����б�
*
*    ����ֵ  ��
*
*    ˵��    ��
*
*
************************************************************************************************************
*/
__s32 SDMMC_PhyInit(__u32 card_no, __u32 speed_mode, __u32 line_sel)
{
    __u32 sysclock, sdcard_clock;

	line_count = line_sel;
	gpio_no  = (card_no >> 16) & 0xffff;
	card_no &= 0xffff;
	//�򿪿���AHBʱ��
    SDMMC_OpenAHBClock(card_no);
    //��ȡϵͳʱ��Ƶ��
    sysclock = SDMMC_GetCmuClk();
    //���ÿ�ʱ�ӣ����䵽corepll��
    sdcard_clock = SDMMC_OpenClock(50, sysclock, card_no);
    if(!sdcard_clock)
    {
        OSAL_printf("unable set card clock\n");

        return -1;
    }
    //����GPIO
    SDMMC_SetPinOn(gpio_no, line_count);
    //���ÿ��ĳ�ʼ������
    if(SDMMC_Init(card_no, sdcard_clock, speed_mode, line_sel) == -1)
    {
        OSAL_printf("fail to init sdmmc card\n");

        return -1;
    }
    pdes = (smc_idma_des*)0x40100000;   //set 1M offset as imda descriptor base address

    return 0;
}
/*
************************************************************************************************************
*
*                                             SDMMC_PhyExit
*
*    �������ƣ�
*
*    �����б�
*
*    ����ֵ  ��
*
*    ˵��    ��
*
*
************************************************************************************************************
*/
__s32 SDMMC_PhyExit(__u32 card_no)
{
	card_no &= 0xffff;
    //�رտ�
    SDMMC_Exit(card_no);
    //�ر�GPIO
    SDMMC_SetPinOff(gpio_no, line_count);
    //�رտ���ģ��ʱ��
    SDMMC_CloseClock(card_no);
    //�رտ���AHBʱ��
    SDMMC_CloseAHBClock(card_no);

    return 0;
}
/*
************************************************************************************************************
*
*                                             SDMMC_PhyRead
*
*    �������ƣ�
*
*    �����б�
*
*    ����ֵ  ��
*
*    ˵��    ��
*
*
************************************************************************************************************
*/
__s32 SDMMC_PhyRead(__u32 start_sector, __u32 nsector, void *buf, __u32 card_no)
{
	card_no &= 0xffff;

    return SDMMC_Read(start_sector, nsector, buf, card_no);
}
