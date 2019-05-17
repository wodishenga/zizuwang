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
#include "water_sensor.h"
#include "net/netstack.h"
#include "net/ipv6/simple-udp.h"

#define ENABLE_UDP 1
#define LOG_MODULE "waterSensor"
#define LOG_LEVEL LOG_LEVEL_INFO

#if ENABLE_UDP
#define WITH_SERVER_REPLY  1
#define UDP_CLIENT_PORT	8765
#define UDP_SERVER_PORT	5678

struct simple_udp_connection udp_conn;
#endif


#if RPL_CONF_DEFAULT_LEAF_ONLY!=1
#error "lpm downward only for leaf node"
#endif

#define REPORT_PERIOD CLOCK_SECOND *3600*24
PROCESS(app_process, "application process");
PROCESS(report_process, "report process");


/*节点mac地址*/
u8 mac_addr[8];

/*电池电量*/
u16 voltage = 0;

/*发送和接收缓存*/
u8 ZDST_TX_BUF[128] ;
u8 ZDST_RX_BUF[128] ;

/*传感器状态*/
Sensor_Info Sensor;

/*告警发送次数*/
int warningSentCount = 0;

/*是否告警*/
extern bool warning; 
/*是否恢复正常状态*/
extern bool isNormal;

/*一开始的状态为初始化上报*/
u8 eventType  = DEVINIT;

u16 CommunicationPacketmark = 1;
/*电池信息上报flag*/
u8 Normal_Send_Flag = false;

int sendCount = 0;
/*固件版本*/
u8 Firmware_Version[25] = "ZDST-WATER-CC1310-Ver001";

/*硬件版本*/
u8 Hardware_Version[25] = "CC1310_MOUDLE_VER1_0";

u8 chr2hex(u8 chr)
{
	if(chr>='0'&&chr<='9')return chr-'0';
	if(chr>='A'&&chr<='F')return (chr-'A'+10);
	if(chr>='a'&&chr<='f')return (chr-'a'+10);
	return 0;
}

u8 hex2chr(u8 hex)
{
	if(hex<=9)return hex+'0';
	if(hex>=10&&hex<=15)return (hex-10+'A');
	return '0';
}

void hex2byte(uint16_t hex, uint8_t *bytes,uint8_t state)
{
	if(state == 1)//高字节在前
	{
		bytes[0] = hex >> 8;
		bytes[1] = hex & 0xff;
	}
	else if(state == 0)//低字节在前
	{
		bytes[1] = hex >> 8;
		bytes[0] = hex & 0xff;
	}
}

u16 HexTo2ASC(u8 hex)
{
	return (hex2chr( (hex>>4) ) <<8) | (hex2chr( (hex&0x0F) )) ;
}

u32 HexTo4ASC(u16 hex)
{
	u32 temp;
	temp = (HexTo2ASC(hex>>8));
	return ((temp <<16) | HexTo2ASC(hex&0x00FF));
}

u8 ChkSum8(u8 *sdata , u16 len)
{
	u16 acc=0;
	for (acc = 0; len > 0; len --)
	{
		acc += *sdata;
		++sdata;
	}
	return (u8)acc;
}


static void GPIO_init(void)
{
	/*测电池电压时候把DIO27设置成输出低电平，不测的时候设置输出高电平*/
	ti_lib_ioc_pin_type_gpio_output(27);
	GPIO_setDio(IOID_27);
        ti_lib_ioc_pin_type_gpio_output(25);
        GPIO_setDio(IOID_25);

}
/*内部寄存器读取电池电压*/
static int getBatteryVoltage(void)
{
	ti_lib_aon_batmon_enable();
	int result = (int)ti_lib_aon_batmon_battery_voltage_get();
	result =  (result * 125) >> 5;
	return result;
}



/*协议数据封装函数*/
u16 protocolPackingFunc(u8 type)
{
	u8 i = 0;
	u8 check = 0;
	u16 length = 24; 							    //除去数据单元协议包的长度
	u16 index = 0;


	memset(ZDST_TX_BUF,0, sizeof(ZDST_TX_BUF));
	MSG_S->Head = MSG_HEAD;								//起始标志('&')
	MSG_S->Protocol_id = PROTOCOL_ID;					//协议类型('D')

	MSG_S->Data_length = HexTo4ASC(0x00000000); 		//数据长度
	MSG_S->Schksum = HexTo2ASC(0x0000);   				//校验单元

	MSG_S->Term_type = htons(TERM_TYPE);    			//终端类型
	MSG_S->Term_addr[0] = 0x01;       					//设备厂商标识
	for(i=0; i<8 ; i++)
	{
		MSG_S->Term_addr[i+1] = mac_addr[i];            //8位mac地址
	}

	MSG_S->CommunicationPacket_mark = htons(CommunicationPacketmark); 	  //通信标识
	MSG_S->Communication_mark = MSG_DEMAND; 	          //交互标识
	CommunicationPacketmark++;

	MSG_S->Command_unit = type;  						  //命令单元
	switch(type)
	{
		case DEVALARM:
		case DEVINIT:
		case DEVPOLL:
			/*初始化以及定期数据上报*/
			MSG_S->Dataload[index++] = 0x10;
			MSG_S->Dataload[index++] = 0x00;
			MSG_S->Dataload[index++] = 24;

			/*设备状态*/
			MSG_S->Dataload[index++] = 0x16;
			MSG_S->Dataload[index++] = 0x00;
			MSG_S->Dataload[index++] = (u8)Sensor.status;

			/*电池电量*/
			MSG_S->Dataload[index++] = 0xb1;
			MSG_S->Dataload[index++] = 0x01;
			hex2byte(voltage, &MSG_S->Dataload[index],0);
			index += 2;

			/*固件版本*/
			MSG_S->Dataload[index++] = 0x36;
			MSG_S->Dataload[index++] = 0x00;

			for(i=0; i<25 ; i++)
			{
				MSG_S->Dataload[i+index] = Firmware_Version[i];
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


	MSG_S->Data_length = htonl(HexTo4ASC(length-6));
	ZDST_TX_BUF[length-1] = MSG_END;
	check = ChkSum8((u8 *)(ZDST_TX_BUF+8),length-9);
	MSG_S->Schksum = htons(HexTo2ASC(check));

	return length;

}


/*传感器事件处理函数*/
void sensorEventHandler(void)
{

	u16 length,i;

	uip_ipaddr_t dest_ipaddr;

	if(!NETSTACK_ROUTING.node_is_reachable() || !NETSTACK_ROUTING.get_root_ipaddr(&dest_ipaddr))
	{
		return;
	}
        /*告警清除*/
        if(isNormal)
        {
                Sensor.status = 0;        //状态正常：0
                eventType =  DEVALARM;     
                isNormal = false;
        }

	/*发生告警*/
	if(warning)
	{
		Sensor.status = 1;       //告警状态
		eventType = DEVALARM;    //命令单元置为主动告警
		warningSentCount++;
		if(warningSentCount > 3)  //每次告警最多发3次数据
		{
			eventType = NONE;
		}
	}

	/*定时上报电量信息*/
	if(Normal_Send_Flag && eventType != DEVALARM)
	{
		Sensor.status = 0;        //状态正常：0
		eventType =  DEVPOLL;     //巡检事件
		Normal_Send_Flag = false; 
	}

	if(eventType != 0)
	{
		/*打包数据*/
		length = protocolPackingFunc(eventType);
		printf("eventType = %d, length=%d\n ", eventType,length);
		if(NETSTACK_ROUTING.node_is_reachable() && NETSTACK_ROUTING.get_root_ipaddr(&dest_ipaddr)) {
			/* Send to DAG root */
			LOG_INFO("Sending request :");
			for(i=0;i<length;i++)
			{
				printf("%02x",ZDST_TX_BUF[i]);
			}
			LOG_INFO_(" to ");
			LOG_INFO_6ADDR(&dest_ipaddr);
			LOG_INFO_("\n");

			simple_udp_sendto(&udp_conn, &ZDST_TX_BUF,length, &dest_ipaddr);
			eventType = NONE;
		}
		else {
			LOG_INFO("Not reachable yet\n");
		}
	}

}


PROCESS_THREAD(app_process, ev, data)
{
	static struct etimer timer;

	CommunicationPacketmark = 1;

	memcpy(mac_addr, &linkaddr_node_addr, 8);
	GPIO_init();
	/*水浸中断GPIO初始化*/
	Water_sensor_hal_init();
	/*设备一启动就检查下GPIO的状态，如果不为0，则说明传感器故障，状态置为-3*/
	if(gpio_hal_arch_read_pin(26) == 0) {
		Sensor.status = -3;
	} else {
		Sensor.status = 0;
	}

	PROCESS_BEGIN();

	etimer_set(&timer, CLOCK_SECOND * 1);

#if ENABLE_UDP
	simple_udp_register(&udp_conn, UDP_CLIENT_PORT, NULL,
			UDP_SERVER_PORT, NULL);
#endif

	while(1) {
		sensorEventHandler();
		PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&timer));
		etimer_reset(&timer);
	}

	PROCESS_END();
}


/*定时开启上报flag，发送电量信息*/
PROCESS_THREAD(report_process, ev, data)
{
	static struct etimer timer;

	PROCESS_BEGIN();

	etimer_set(&timer, REPORT_PERIOD);  

	while(1) {
		voltage = getBatteryVoltage();
		printf("voltage = %d\n", voltage);
		PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&timer));
		Normal_Send_Flag = true;
		etimer_reset(&timer);
	}

	PROCESS_END();
}



