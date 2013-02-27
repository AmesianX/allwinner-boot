/*
************************************************************************************************************************
*                                                         eGON
*                                         the Embedded GO-ON Bootloader System
*
*                             Copyright(C), 2006-2008, SoftWinners Microelectronic Co., Ltd.
*											       All Rights Reserved
*
* File Name : Boot0_C_part.c
*
* Author : Gary.Wang
*
* Version : 1.1.0
*
* Date : 2007.12.18
*
* Description :
*
* Others : None at present.
*
*
* History :
*
*  <Author>        <time>       <version>      <description>
*
* Gary.Wang       2007.11.09      1.1.0        build the file
*
* Gary.Wang       2007.12.18      1.1.0        remove "BT0_self_rcv"
*
************************************************************************************************************************
*/
#include "boot0_i.h"

#include <string.h>

extern const boot0_file_head_t  BT0_head;
extern __u32 super_standby_flag;
__u32 odt_status;

static void move_RW( void );
static void clear_ZI( void );
static int  check_bootid(void);
static void timer_init(void);
static void print_version(void);
static __u32 check_odt(int ms);
/*******************************************************************************
*��������: Boot0_C_part
*����ԭ�ͣ�void Boot0_C_part( void )
*��������: Boot0����C���Ա�д�Ĳ��ֵ�������
*��ڲ���: void
*�� �� ֵ: void
*��    ע:
*******************************************************************************/
void Boot0_C_part( void )
{
	__u32 status;
	__s32 dram_size;

	move_RW( );
	clear_ZI( );

	if(check_bootid())
    {
    	jump_to( FEL_BASE );
    }
    timer_init();
    UART_open( BT0_head.prvt_head.uart_port, (void *)BT0_head.prvt_head.uart_ctrl, 24*1000*1000 );
	odt_status = check_odt(5);
    if( BT0_head.prvt_head.enable_jtag )
    {
		jtag_init( (normal_gpio_cfg *)BT0_head.prvt_head.jtag_gpio );
    }
	msg("HELLO! BOOT0 is starting!\n");
	print_version();

	mmu_system_init(EGON2_DRAM_BASE, 4 * 1024, EGON2_MMU_BASE);
	mmu_enable();

	//for A20 super standby
	boot0_twi_init();

#ifdef CONFIG_AW_FPGA_PLATFORM
	dram_size=*((volatile unsigned int*)(0x8000-0x4));
	msg("sram data=%x\n",dram_size);
	if(dram_size==0x12345678)
	{
			msg("force jump to superstandby!\n");
			jump_to( 0x52000000 );
	}
#endif

	dram_size = init_DRAM(BT0_head.boot_head.platform[7]);                                // ��ʼ��DRAM

    boot0_twi_exit();

    if(dram_size)
	{
		msg("dram size =%d\n", dram_size);
	}
	else
	{
		msg("initializing SDRAM Fail.\n");
		mmu_disable( );
		jump_to( FEL_BASE );
	}

#ifdef CONFIG_AW_FPGA_PLATFORM
	dram_size=*((volatile unsigned int*)(0x7000-0x4));
	msg("sram data=%x\n",dram_size);
	if(dram_size==0x12345678)
	{
			msg("force jump to superstandby!\n");
			jump_to( 0x52000000 );
	}
#endif

	msg("%x\n", *(volatile int *)0x52000000);
	msg("super_standby_flag = %d\n", super_standby_flag);
	if(1 == super_standby_flag)
	{
		//tmr_enable_watchdog();
		//disable_icache();
		jump_to( 0x52000000 );
	}

	#if SYS_STORAGE_MEDIA_TYPE == SYS_STORAGE_MEDIA_NAND_FLASH
		status = load_Boot1_from_nand( );         // ����Boot1
	#elif SYS_STORAGE_MEDIA_TYPE == SYS_STORAGE_MEDIA_SPI_NOR_FLASH
		status = load_boot1_from_spinor( );         // ����Boot1
	#elif SYS_STORAGE_MEDIA_TYPE == SYS_STORAGE_MEDIA_SD_CARD
		status = load_boot1_from_sdmmc( (char *)BT0_head.prvt_head.storage_data );  // ����boot1
	#else
		#error The storage media of Boot1 has not been defined.
	#endif


	msg("Ready to disable icache.\n");

	mmu_disable( );                               // disable instruction cache

	if( status == OK )
	{
//		restart_watch_dog( );                     // restart watch dog
		//��תboot1֮ǰ����dram�Ĵ�Сд��ȥ
		//set_dram_size(dram_size );
		//��ת֮ǰ�������е�dram����д��boot1��
		#if SYS_STORAGE_MEDIA_TYPE == SYS_STORAGE_MEDIA_SD_CARD
		set_dram_para((void *)&BT0_head.prvt_head.dram_para);
        #endif
        
        #if SYS_STORAGE_MEDIA_TYPE == SYS_STORAGE_MEDIA_NAND_FLASH
        set_nand_good_block_ratio_para((void *)&BT0_head.prvt_head.storage_data);
        #endif
        
		msg("Succeed in loading Boot1.\n"
		    "Jump to Boot1.\n");
		jump_to( BOOT1_BASE );                    // �������Boot1�ɹ�����ת��Boot1��ִ��
	}
	else
	{
//		disable_watch_dog( );                     // disable watch dog

		msg("Fail in loading Boot1.\n"
		    "Jump to Fel.\n");
		jump_to( FEL_BASE );                      // �������Boot1ʧ�ܣ�������Ȩ����Fel
	}
}
/*******************************************************************************
*��������: set_pll
*����ԭ�ͣ�void set_pll( void )
*��������: Boot0����C���Ա�д�� ����CPUƵ��
*��ڲ���: void
*�� �� ֵ: void
*��    ע:
*******************************************************************************/
void set_pll( void )
{
	__u32 reg_val, i;

	//�л���24M
	CCMU_REG_AHB_APB = 0x00010010;
	//����PLL1��384M
	reg_val = (0x21005000) | (0x80000000);
	CCMU_REG_PLL1_CTRL = reg_val;
	//��ʱ
	for(i=0;i<200;i++);
	//�л���PLL1
	reg_val = CCMU_REG_AHB_APB;
	reg_val &= ~(3 << 16);
	reg_val |=  (2 << 16);
	CCMU_REG_AHB_APB = reg_val;
	//��DMA
	CCMU_REG_AHB_MOD0 |= 1 << 6;

	//��PLL6�����õ�Ĭ��Ƶ��600M
	reg_val = CCMU_REG_PLL6_CTRL;
	reg_val |= 1<<31;
	CCMU_REG_PLL6_CTRL = reg_val;

	return ;
}


static __u32 check_odt(int ms)
{
#if 0
	__u32 value, tmp_cfg, tmp_pull;
	__u32 t1, t2;

	value = *(volatile __u32 *)(0x01c20828);
	tmp_cfg = value;

	value &= ~(0x07 << 24);
	*(volatile __u32 *)(0x01c20828) = value;

	value = *(volatile __u32 *)(0x01c20840);
	tmp_pull = value;
	value &= ~(0x03 << 28);
	value |=  (0x01 << 28);
	*(volatile __u32 *)(0x01c20840) = value;

	t1 = *(volatile unsigned int *)(0x01c20C00 + 0x84);
	t2 = t1 + ms;
	do
	{
		t1 = *(volatile unsigned int *)(0x01c20C00 + 0x84);
	}
	while(t2 >= t1);

	value = (*(volatile __u32 *)(0x01c20834)) & (1 << 14);

	*(volatile __u32 *)(0x01c20840) = tmp_pull;
	*(volatile __u32 *)(0x01c20828) = tmp_cfg;

	return value;
#endif
	return 0;
}

static __s32 check_bootid(void)
{
#if 0
	__u32 reg = 0x01c23800;
	__u32 value, i;

	for(i=0;i<4;i++)
	{
		value = *(volatile __u32 *)(reg + 0x10 + (i<<2));
		if(value)
		{
			return -1;
		}
	}
#endif
	return 0;
}

static void timer_init(void)
{
	*(volatile unsigned int *)(0x01c20000 + 0x144) |= (1U << 31);
	*(volatile unsigned int *)(0x01c20C00 + 0x80 )  = 1;
	*(volatile unsigned int *)(0x01c20C00 + 0x8C )  = 0x2EE0;
	*(volatile unsigned int *)(0x01c20C00 + 0x84 )  = 0;
}

static void print_version(void)
{
	msg("boot0 version : %s\n", BT0_head.boot_head.platform + 2);

	return;
}

static void move_RW( void )
{
	__u8    *psrc8;
	__u8    *pdst8;
	__u32   *psrc32;
	__u32   *pdst32;
	__u32   size;
	__u32   N;

	extern unsigned char Load$$Boot0_RW_ZI$$Base;
	extern unsigned char Image$$Boot0_RW_ZI$$Base;
	extern unsigned char Image$$Boot0_RW_ZI$$Length;

	size = (__u32) &Image$$Boot0_RW_ZI$$Length;
	psrc32  = (__u32 *)&Load$$Boot0_RW_ZI$$Base;
	pdst32  = (__u32 *)&Image$$Boot0_RW_ZI$$Base;

	N = size >> 4;
	while( N-- > 0 )
	{
		*pdst32++ = *psrc32++;
		*pdst32++ = *psrc32++;
		*pdst32++ = *psrc32++;
		*pdst32++ = *psrc32++;
	}

	N = size & ( ( 1 << 4 ) - 1 );
	psrc8 = (__u8 *)psrc32;
	pdst8 = (__u8 *)pdst32;
	while( N--)
	{
		*pdst8++ = *psrc8++;
	}
}



static void clear_ZI( void )
{
	__u8  *p8;
	__u32 *p32;
	__u32 size;
	__u32 N;

	extern unsigned char Image$$Boot0_RW_ZI$$ZI$$Base;
	extern unsigned char Image$$Boot0_RW_ZI$$ZI$$Length;

	size = (__u32)  &Image$$Boot0_RW_ZI$$ZI$$Length;
	p32  = (__u32 *)&Image$$Boot0_RW_ZI$$ZI$$Base;

	N = size >> 4;
	while( N-- > 0 )
	{
		*p32++ = 0;
		*p32++ = 0;
		*p32++ = 0;
		*p32++ = 0;
	}

	N = size & ( ( 1 << 4 ) - 1 );
	p8 = (__u8 *)p32;
	while( N--)
	{
		*p8++ = 0;
	}
}
