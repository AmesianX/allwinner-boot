/*
**********************************************************************************************************************
*                                                    ePDK
*                                    the Easy Portable/Player Develop Kits
*                                              eBIOS Sub-System
*
*                                   (c) Copyright 2007-2009, Steven.ZGJ.China
*                                             All Rights Reserved
*
* Moudle  : iis bios module
* File    : iisc_private.h
*
* By      : Steven
* Version : v1.00
* Date    : 2008-8-19 18:38:16
**********************************************************************************************************************
*/
#ifndef _SW_IIC_H_
#define _SW_IIC_H_

#include "axp_i.h"

#define CFG_SW_IIC_MAX      4
#define CFG_SW_IIC_CTL0	   TWIC0_REGS_BASE	      /* iic0 */
#define CFG_SW_IIC_CTL1	   TWIC1_REGS_BASE	      /* iic1 */
#define CFG_SW_IIC_CTL2	   TWIC2_REGS_BASE	      /* iic2 */

#define SW_IIC_READ        (1)
#define SW_IIC_WRITE       (0)

struct sw_iic
{
    volatile unsigned int addr;        /* slave address     */
    volatile unsigned int xaddr;       /* extend address    */
    volatile unsigned int data;        /* data              */
    volatile unsigned int ctl;         /* control           */
    volatile unsigned int status;      /* status            */
    volatile unsigned int clk;         /* clock             */
    volatile unsigned int srst;        /* soft reset        */
    volatile unsigned int eft;         /* enhanced future   */
};

typedef struct  _twi_arg
{
    __u32    slave_addr;             //���豸��ַ
    __u32    slave_addr_flag;        //�����õ�ַ���  0��7bit�� 1��10bit
	__u8    *byte_addr;              //��Ҫ��д�������ڴ��豸�еĵ�ַ
	__u32    byte_addr_width;        //�����õ�ַ���  0��1��8 bit; ���������ִ����ֽ���
	__u32    byte_count;             //һ�ζ�дҪ��ɵ��ֽ���
	__u32    if_restart;             //�Ƿ���Ҫ����restart����
	__u8    *data;                   //���ݵĵ�ַ
}
_twi_arg_t;



typedef volatile struct sw_iic *sw_iic_t;

enum  _sw_iic_operation_status
{
    SW_IIC_OPERATION_NO_ERR         = 0,
    SW_IIC_OPERATION_START_NO_INT   = -1

};

extern  __s32 boot0_twi_init(void);
extern  __s32 boot0_twi_exit(void);


extern  __s32  BOOT_TWI_Read(__u32 arg1, __u8 *arg2, __u8 *arg3);
extern  __s32  BOOT_TWI_Write(__u32 arg1, __u8 *arg2, __u8 *arg3);

#endif /* _SW_IIC_H_ */

/*end of sw_iic.h*/
