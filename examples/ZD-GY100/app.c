/*
 * Copyright (c) 2006, Swedish Institute of Computer Science.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the Institute nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE INSTITUTE AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE INSTITUTE OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 * This file is part of the Contiki operating system.
 *
 */

/**
 * \file
 *         A very simple Contiki application showing how Contiki programs look
 * \author
 *         Adam Dunkels <adam@sics.se>
 */

//https://e2e.ti.com/support/wireless-connectivity/bluetooth/f/538/t/557767

#include "contiki.h"
#include "ti-lib.h"
#include "lpm.h"
#include "driverlib/prcm.h"
#include "dev/gpio-hal.h"
#include <stdio.h> 
#include "net/routing/routing.h"
#include "random.h"
#include "net/netstack.h"
#include "net/ipv6/simple-udp.h"
#include "sys/clock.h"
#include "sys/log.h"
#include "app.h"

#include "lpm.h"

#include "net/netstack.h"
#include "net/ipv6/simple-udp.h"


#define ENABLE_UDP 1
#define LOG_MODULE "SMOKE"
#define LOG_LEVEL LOG_LEVEL_INFO

/*i2c从机地址*/
#define SLAVE_ADDRESS 0x48

#if ENABLE_UDP
#define WITH_SERVER_REPLY 1
#define UDP_CLIENT_PORT 8765
#define UDP_SERVER_PORT 5678

static struct simple_udp_connection udp_conn;
#endif


static gpio_hal_event_handler_t Interrupt_handler; //定义SCL下降沿中断handler

static void i2c_slave_init(void);
static void SCLInterruptHandler(void);
static void Alarm_handler(gpio_hal_pin_mask_t pin_mask);


#if RPL_CONF_DEFAULT_LEAF_ONLY!=1
#error "lpm downward only for leaf node"
#endif

#define REPORT_PERIOD CLOCK_SECOND *3600*24
PROCESS(app_process, "application process");
PROCESS(report_process, "report process");
bool smokeflag = true;


/*节点mac地址*/
u8 mac_addr[8];

/*mac层RSSI值*/
extern signed char mac_rssi;

u8 Smk_status = 0;

/*发送消息的次数*/
u16 sendCount = 0;

/*告警状态*/
u8 sta_Smk_Now = 0;
u8 sta_Smk_Ago = 0;

/*自检状态*/
u8 sta_Chk_Now = 0;
u8 sta_Chk_Ago = 0;

/*告警延时计数器*/
u16 smk_delay_cnt = 0;
/*告警延时计数器*/
u16 smk_fire_cnt = 0;


/*是否传递消息*/
bool smk_delay_msk = true;

/*自检延时计数器*/
u16 smk_check_cnt = 0;

/*一开始的状态为初始化上报*/
u8 Accord_Type = DEVINIT;

/*电池信息上报flag*/
u8 Normal_Send_Flag = false;

/*通信标识*/
u16 CommunicationPacketmark = 1;

/*固件版本*/
u8 Firmware_Version[25] = "ZDST-Smk-CC1310-Ver001";

/*硬件版本*/
u8 Hardware_Version[25] = "CC1310_MOUDLE_VER1_0";

/*发送和接收缓存*/
u8 ZDST_TX_BUF[128];
u8 ZDST_RX_BUF[128];

u16 i2cDataTxArray[4] = {0x03, 0x03, 0x06, 0xf9};

u16 i2cDataArray[10];
u16 i2cDataLen;
u8 i2cDataFlag = 0;
/*电池电量*/
u8 smk_vol = 255;

/*烟感信息结构体*/
Smk_Info smk;

u8 chr2hex(u8 chr)
{
	if (chr >= '0' && chr <= '9')
		return chr - '0';
	if (chr >= 'A' && chr <= 'F')
		return (chr - 'A' + 10);
	if (chr >= 'a' && chr <= 'f')
		return (chr - 'a' + 10);
	return 0;
}

u8 hex2chr(u8 hex)
{
	if (hex <= 9)
		return hex + '0';
	if (hex >= 10 && hex <= 15)
		return (hex - 10 + 'A');
	return '0';
}

u16 HexTo2ASC(u8 hex)
{
	return (hex2chr((hex >> 4)) << 8) | (hex2chr((hex & 0x0F)));
}

u32 HexTo4ASC(u16 hex)
{
	u32 temp;
	temp = (HexTo2ASC(hex >> 8));
	return ((temp << 16) | HexTo2ASC(hex & 0x00FF));
}

u8 ChkSum8(u8 *sdata, u16 len)
{
	u16 acc = 0;
	for (acc = 0; len > 0; len--)
	{
		acc += *sdata;
		++sdata;
	}
	return (u8)acc;
}

static void GPIO_init(void)
{
	ti_lib_ioc_pin_type_gpio_output(27);
	GPIO_setDio(IOID_27);
	ti_lib_ioc_pin_type_gpio_output(25);
	GPIO_setDio(25);
}


/*SCL引脚中断函数*/
static void Alarm_handler(gpio_hal_pin_mask_t pin_mask)
{
	/*触发中断后，设置flag让cpu退出休眠模式*/
	smokeflag = false;
	/*中断触发后，立即进行I2C初始化，接收数据*/
	i2c_slave_init();
}

/*SCL下降沿触发中断初始化函数*/
static void SCLInterruptHandler(void)
{
	Interrupt_handler.pin_mask |= gpio_hal_pin_to_mask(BOARD_IOID_SCL); //使用gpio 6 作为中断判断
	Interrupt_handler.handler = Alarm_handler;
	gpio_hal_arch_pin_set_input(BOARD_IOID_SCL);
    gpio_hal_arch_pin_cfg_set(BOARD_IOID_SCL, GPIO_HAL_PIN_CFG_EDGE_FALLING | GPIO_HAL_PIN_CFG_INT_ENABLE | GPIO_HAL_PIN_CFG_PULL_UP);
    gpio_hal_arch_interrupt_enable(BOARD_IOID_SCL);
    gpio_hal_register_handler(&Interrupt_handler);

}

static void
I2CReciveCB(void)
{
	static u8  i = 0;
	static u8  j = 0;
	u32 status = 0 ;
	u32 slaveStatus = 0;
	
	status= I2CSlaveIntStatus(I2C0_BASE, 0);
	I2CSlaveIntClear(I2C0_BASE, status);
	if(status & I2C_SLAVE_INT_DATA)
	{
		slaveStatus = I2CSlaveStatus(I2C0_BASE);
		if(slaveStatus & I2C_SLAVE_ACT_RREQ)
		{
			i2cDataArray[i++] = I2CSlaveDataGet(I2C0_BASE);
		}
		if(slaveStatus & I2C_SLAVE_ACT_TREQ)
		{
			I2CSlaveDataPut(I2C0_BASE, i2cDataTxArray[j++]);
		}
	}
	else if(status & I2C_SLAVE_INT_STOP)
	{
		i2cDataLen = i;
		i = 0;
		j = 0;
		i2cDataFlag = 1;
		smokeflag = true;
	}
}

static void
i2c_slave_init(void)
{
	
	bool int_disabled = ti_lib_int_master_disable();
 
	ti_lib_prcm_power_domain_on(PRCM_DOMAIN_PERIPH);
	  
	while((ti_lib_prcm_power_domain_status(PRCM_DOMAIN_PERIPH)
	         != PRCM_DOMAIN_POWER_ON));
	
	ti_lib_prcm_power_domain_on(PRCM_DOMAIN_SERIAL);
	while((ti_lib_prcm_power_domain_status(PRCM_DOMAIN_SERIAL)
	        != PRCM_DOMAIN_POWER_ON));

	ti_lib_prcm_peripheral_run_enable(PRCM_PERIPH_I2C0);
	ti_lib_prcm_peripheral_sleep_enable(PRCM_PERIPH_I2C0);
	ti_lib_prcm_peripheral_deep_sleep_enable(PRCM_PERIPH_I2C0);
	ti_lib_prcm_load_set();
	while(!ti_lib_prcm_load_get());

	ti_lib_i2c_slave_disable(I2C0_BASE);

	ti_lib_ioc_io_port_pull_set(BOARD_IOID_SDA, IOC_NO_IOPULL);
	ti_lib_ioc_io_port_pull_set(BOARD_IOID_SCL, IOC_NO_IOPULL);
	ti_lib_ioc_pin_type_i2c(I2C0_BASE, BOARD_IOID_SDA, BOARD_IOID_SCL);

	ti_lib_i2c_slave_init(I2C0_BASE,  0x48);
	ti_lib_i2c_int_register(I2C0_BASE, I2CReciveCB);
	ti_lib_i2c_slave_int_clear(I2C0_BASE, I2C_SLAVE_INT_START | I2C_SLAVE_INT_STOP | I2C_SLAVE_INT_DATA);
	ti_lib_i2c_slave_int_enable(I2C0_BASE, I2C_SLAVE_INT_START | I2C_SLAVE_INT_STOP | I2C_SLAVE_INT_DATA);

	ti_lib_int_enable(INT_I2C_IRQ);

	if(!int_disabled) {
		ti_lib_int_master_enable();
	}
	 
}

/*协议数据封装函数*/
u16 protocolPackingFunc(u8 type)
{
	u8 i = 0;
	u8 check = 0;
	u16 length = 24;
	u16 index = 0;

	memset(ZDST_TX_BUF, 0, sizeof(ZDST_TX_BUF));
	MSG_S->Head = MSG_HEAD;						//起始标志('&')
	MSG_S->Protocol_id = PROTOCOL_ID;			// 协议类型('D')
	MSG_S->Data_length = HexTo4ASC(0x00000000); //数据长度
	MSG_S->Schksum = HexTo2ASC(0x0000);			//校验单元
	MSG_S->Term_type = htons(TERM_TYPE);		//终端类型
	MSG_S->Term_addr[0] = 0x01;					//设备厂商标识
	for (i = 0; i < 8; i++)
	{
		MSG_S->Term_addr[i + 1] = mac_addr[i];
	}
	MSG_S->CommunicationPacket_mark = htons(CommunicationPacketmark); //通信标识
	MSG_S->Communication_mark = MSG_DEMAND;			 //交互标识
	CommunicationPacketmark++;

	MSG_S->Command_unit = type; //命令单元
	switch (type)
	{
	case DEVALARM:
		if (smk.status.fire)
			Smk_status = 8;
		else if (smk.status.chek)
			Smk_status = 8;
		else if (smk.status.faut)
			Smk_status = -3;
		else
			Smk_status = 0;
		/*烟感状态*/
		MSG_S->Dataload[index++] = 0x16;
		MSG_S->Dataload[index++] = 0x00;
		MSG_S->Dataload[index++] = (u8)Smk_status;
		/*信号强度*/
                MSG_S->Dataload[index++] = 0xba;
                MSG_S->Dataload[index++] = 0x01;
                MSG_S->Dataload[index++] = mac_rssi;

		length += index;
		break;

	case DEVINIT:
	case DEVPOLL:
		/*上报周期*/
		MSG_S->Dataload[index++] = 0x10;
		MSG_S->Dataload[index++] = 0x00;
		MSG_S->Dataload[index++] = 24; //Sys.Normal_send_time;

		if (smk.status.fire)
			Smk_status = 8;
		else if (smk.status.chek)
			Smk_status = 8;
		else if (smk.status.faut)
			Smk_status = -3;
		else
			Smk_status = 0;
		/*烟感状态*/
		MSG_S->Dataload[index++] = 0x16;
		MSG_S->Dataload[index++] = 0x00;
		MSG_S->Dataload[index++] = (u8)Smk_status;

		/*信号强度*/
		MSG_S->Dataload[index++] = 0xba;
		MSG_S->Dataload[index++] = 0x01;
		MSG_S->Dataload[index++] = mac_rssi;

		/*电池电量*/
		MSG_S->Dataload[index++] = 0x8a;
		MSG_S->Dataload[index++] = 0x00;
		MSG_S->Dataload[index++] = (u8)(((float)smk_vol * 3.0) / 25.5);
		/*软件版本*/
		MSG_S->Dataload[index++] = 0x36;
		MSG_S->Dataload[index++] = 0x00;

		for (i = 0; i < 25; i++)
		{
			MSG_S->Dataload[i + index] = Firmware_Version[i];
		}
		index += 25;
		
		/*硬件版本*/
		MSG_S->Dataload[index++] = 0xb4;
		MSG_S->Dataload[index++] = 0x01;
		
		for(i=0; i<25 ; i++)
		{
			MSG_S->Dataload[i+index] = Hardware_Version[i];
		}
		index += 25;
		length += index;
		break;
	}

	MSG_S->Data_length = htonl(HexTo4ASC(length - 6));
	ZDST_TX_BUF[length - 1] = MSG_END;
	check = ChkSum8((u8 *)(ZDST_TX_BUF + 8), length - 9);
	MSG_S->Schksum = htons(HexTo2ASC(check));

	return length;
}

/*事件处理函数*/
void sensorEventHandler(void)
{

	u16 length, i;

	uip_ipaddr_t dest_ipaddr;
	
	/*是否自检*/
	if (smk.status.chek != sta_Chk_Ago)
	{
		sta_Chk_Now = smk.status.chek;
		sta_Chk_Ago = sta_Chk_Now;
		/*发生自检,则命令单元为DEVALARM,smk_delay_msk标志位置为1发送数据*/
		Accord_Type = DEVALARM;
		smk_delay_msk = true;
		if (smk.status.chek)
		{
			LOG_INFO("自检\r\n");
		}
		else
		{
			/*发送3次自检信息后,不要再发送自检信息了*/
			LOG_INFO("自检后恢复到正常状态!!!\r\n");
			Accord_Type = DEVALARM;
		}
	}

	/*是否发生告警*/
	if (smk.status.fire != sta_Smk_Ago)
	{
		sta_Smk_Now = smk.status.fire;
		sta_Smk_Ago = sta_Smk_Now;
		/*发生告警,则数据类型为DEVALARM,smk_delay_msk标志位置为1发送数据*/
		Accord_Type = DEVALARM;
		smk_delay_msk = true;
		if (smk.status.fire)
		{
			LOG_INFO("火灾发生,告警!!!\r\n");
		}
		else
		{
			/*发送3次告警信息后,不要再发送告警信息了*/
			LOG_INFO("告警完毕,恢复到正常!!!\r\n");
			Accord_Type = DEVALARM;
		}
	}

	/*巡检*/
	if (Normal_Send_Flag && Accord_Type != DEVALARM)
	{
		Accord_Type = DEVPOLL;
		Normal_Send_Flag = false;
		smk_delay_msk = true;
		LOG_INFO("发送电量信息!!!\r\n");
	}

	if (smk.status.fire)
	{
		if (++smk_fire_cnt > 120)
		{
			smk_fire_cnt = 0;
			smk.status.fire = false;
			Accord_Type = NONE;
		}
	}

	if (smk.status.chek)
	{
		if (++smk_check_cnt > 120)
		{
			smk_check_cnt = 0;
			smk.status.chek = false;
			Accord_Type = NONE;
		}
	}
	if (smk_delay_cnt)
	{
		smk_delay_cnt--;
	}
	else
	{
		smk_delay_msk = true;
	}

	if (Accord_Type != 0 && smk_delay_msk)
	{
		smk_delay_msk = false;
		smk_delay_cnt = 6;
		/*打包数据*/
		length = protocolPackingFunc(Accord_Type);
		printf("Accord_Type = %d, length=%d\n ", Accord_Type, length);
		if (NETSTACK_ROUTING.node_is_reachable() && NETSTACK_ROUTING.get_root_ipaddr(&dest_ipaddr))
		{
			/* Send to DAG root */
			LOG_INFO("Sending request :");
			for (i = 0; i < length; i++)
			{
				printf("%02x", ZDST_TX_BUF[i]);
			}
			LOG_INFO_(" to ");
			LOG_INFO_6ADDR(&dest_ipaddr);
			LOG_INFO_("\n");
			simple_udp_sendto(&udp_conn, &ZDST_TX_BUF, length, &dest_ipaddr);
			Accord_Type = NONE;
		}
		else
		{
			LOG_INFO("Not reachable yet\n");
		}
	}
}


PROCESS_THREAD(app_process, ev, data)
{
	static struct etimer timer;
	CommunicationPacketmark = 1;
	memcpy(mac_addr, &linkaddr_node_addr, 8);
	/*注册SCL中断函数*/
	SCLInterruptHandler();

	GPIO_init();
	u8 i = 0;
	PROCESS_BEGIN(); 
  
	etimer_set(&timer, CLOCK_SECOND * 1);
#if ENABLE_UDP
	simple_udp_register(&udp_conn, UDP_CLIENT_PORT, NULL,
						  UDP_SERVER_PORT, NULL);
#endif

	while(1) {
			if(i2cDataFlag)
			{
				printf("I2C ReciveData :\r\n");
				for(i = 0; i < i2cDataLen; i++)printf("%02x",i2cDataArray[i]);
				printf("\ndatalen = %d, smk.byte = %02x, smk_vol = %02x\n",i2cDataLen, i2cDataArray[0],i2cDataArray[1]);
				i2cDataFlag = 0;
				i2cDataLen = 0;
				smk.byte = i2cDataArray[0];//探测器状态
				smk_vol =  i2cDataArray[1]; //电量
				if(smk.status.chek)smk_check_cnt = 0;
				if(smk.status.fire)smk_fire_cnt = 0;
				memset(i2cDataArray, 0 ,10);
			}
		sensorEventHandler();
		PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&timer));
		etimer_reset(&timer);
  }

  PROCESS_END();
}


/*定时开启上报flag，发送电量信息*/
PROCESS_THREAD(report_process, ev, data)
{
	static struct etimer rtimer;

	PROCESS_BEGIN();

	etimer_set(&rtimer, REPORT_PERIOD );

	while (1)
	{
		PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&rtimer));
		Normal_Send_Flag = true;
		etimer_reset(&rtimer);
	}

	PROCESS_END();
}



