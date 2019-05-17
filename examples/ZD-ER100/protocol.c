/*
 * Copyright (c) 201, RISE SICS
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
#include "contiki.h"
/* Log configuration */
#include "sys/log.h"
#include "rpl-dag-root.h"
#include "contiki.h"
#include "contiki-net.h"
#include "net/routing/rpl-lite/rpl.h"
#include "net/ipv6/uip-ds6-route.h"
#include "net/ipv6/uip-sr.h"
/* Log configuration */
#include "sys/log.h"
#include "stdlib.h"

#include <stdio.h>
#include "protocol.h"

#include "net/netstack.h"
#include "net/ipv6/uip.h"
#include "md5.h"
extern void mqtt_publish_data(uint8_t *data, uint16_t len);
#define START '&'
#define END '!'
#define PROTOCOL_TYPE 'D'
uint8_t msgBuffer[1024] = {0};
static uint16_t msgIndex = 0;
uint8_t sn = 0;
/*---------------------------------------------------------------------------*/
#define DEBUG 1
#if DEBUG
#define PRINTF(...) printf(__VA_ARGS__)
#else
#define PRINTF(...)
#endif

uint16_t panID1 = 0xaaaa;
attrList_t attrList[] = 
{
	{
		0x01b9,
		9,
		(void * )(&panID1),
		"网络编号",
		1
	}

};

attrList_t* findAttr(uint16_t key)
{
    for(uint8_t i = 0;i < sizeof(attrList);i++)
    {
		if(key == attrList[i].key)
		{
			printf("find %s in list \n",attrList[i].name);
			return &attrList[i];
		}
	}
	printf("没找到key = 0x%04x,%d",key,key);
	return 0;
}
/*
 *IPV6地址转MAC地址
 */
void uip_addr_to_mac_addr1(const uip_ipaddr_t *uip_ipaddr, uint8_t *uip_macaddr)
{
	uint8_t i = 0;
	for (i = 0; i < 8; i++)
		uip_macaddr[i] = uip_ipaddr->u8[i + 8];
	if (uip_macaddr[0] & 0x02)
		uip_macaddr[0] &= ~0x02;
	else
		uip_macaddr[0] |= 0x02;
}

//字节序检查
//return >0大端序，<0 小端序
void get_root_mac_addr(uint8_t *root_mac_addr)
{
	static uip_ipaddr_t root_ipaddr;
	NETSTACK_ROUTING.get_root_ipaddr(&root_ipaddr);
	uip_addr_to_mac_addr1((const uip_ipaddr_t *)&root_ipaddr, root_mac_addr);
}
void get_device_addr(uint8_t *device_addr)
{
	get_root_mac_addr(device_addr);
}
int check_endian()
{
	int n = 1;
	char ch = (char)n;
	if (ch)
	{
		return -1;
	}
	else
	{
		return 1;
	}
}
uint16_t byte2hex(uint8_t *pData)
{
	uint16_t hex = 0;
	if (check_endian() < 0) //小端序,高字节在前，低字节在后
	{
		hex |= pData[0] << 8;
		hex |= pData[1];
	}
	else //大端序,高字节在前，低字节在后
	{
		hex |= pData[1] << 8;
		hex |= pData[0];
	}
	return hex;
}
uint16_t hex2ascii(uint8_t data_hex)
{
	uint8_t data_ASCII_H;
	uint8_t data_ASCII_L;
	uint16_t data_ASCII;
	data_ASCII_H = ((data_hex >> 4) & 0x0F);
	data_ASCII_L = data_hex & 0x0F;
	if (data_ASCII_H <= 9)
	{
		data_ASCII_H += 0x30;
	}
	else if ((data_ASCII_H >= 10) && (data_ASCII_H <= 15))
	{
		data_ASCII_H += 0x37;
	}
	if (data_ASCII_L <= 9)
	{
		data_ASCII_L += 0x30;
	}
	else if ((data_ASCII_L >= 10) && (data_ASCII_L <= 15))
	{
		data_ASCII_L += 0x37;
	}
	data_ASCII = (((data_ASCII_H & 0x00FF) << 8) | data_ASCII_L);
	return data_ASCII;
}
/*
 *掉线上报
 */
void report_disconnect(uip_ipaddr_t *node_addr)
{
	uint8_t i = 0;
	static uint8_t buffer[100];
	uint8_t msg[50];
	static uint8_t mac_addr[9] = {0};
	for (i = 0; i < 8; i++)
		mac_addr[i + 1] = node_addr->u8[i + 8];
	if (mac_addr[1] & 0x02)
		mac_addr[1] &= ~0x02;
	else
		mac_addr[1] |= 0x02;
	msg[0] = 0x02;
	memcpy(&msg[1], mac_addr, 9);
	encode(buffer, DEVICE_ADDRESS, msg, 10, 0x40);
}
uint8_t ascii2hex(char ascii)
{
	uint8_t hex = 0;
	if (ascii <= 'F' && ascii >= 'A')
	{
		hex = ascii - 'A' + 10;
		return hex;
	}
	if (ascii <= 'f' && ascii >= 'a')
	{
		hex = ascii - 'a' + 10;
		return hex;
	}
	if (ascii <= '9' && ascii >= '0')
	{
		hex = ascii - '0';
		return hex;
	}
	return hex;
}
/*
 *长度用ASII码
 */
uint16_t get_len(uint8_t *pData)
{
	uint16_t len = 0;
	len |= ascii2hex(pData[2]) << 12; //0x31
	len |= ascii2hex(pData[3]) << 8;  //0x32
	len |= ascii2hex(pData[4]) << 4;  //0x33
	len |= ascii2hex(pData[5]);		  //0x34
	return len;
}
void convert_len_to_assi(uint16_t len, char *assi_len)
{
	assi_len[0] = hex2ascii(len >> 8) >> 8;
	assi_len[1] = hex2ascii(len >> 8) & 0xff;
	assi_len[2] = hex2ascii(len & 0xff) >> 8;
	assi_len[3] = hex2ascii(len & 0xff) & 0xff;
}
/*
 *计算校验和
 */
uint16_t check_sum(uint8_t *data, uint32_t len)
{
	uint8_t i;
	uint16_t checkSum = 0;
	uint16_t asciiChkSum = 0;
	for (i = 0; i < len; i++)
		checkSum = data[i] + checkSum;
	asciiChkSum = hex2ascii(checkSum & 0xff);
	return asciiChkSum;
}
void fill_msg(uint8_t *buf, uint16_t len, uint8_t sendStatus)
{
	static uint8_t buffer[1024];
	memcpy(&msgBuffer[msgIndex], buf, len);
	msgIndex = msgIndex + len;
	if (sendStatus)
	{
		encode(buffer, DEVICE_ADDRESS, msgBuffer, msgIndex + 1, 0x40);
		memset(msgBuffer, 0, sizeof(msgBuffer));
		msgIndex = 0;
	}
}
/*
 *检查数据正确性
 */
int check_data(uint8_t *pData)
{
	//	uint16_t checkSum;
	//	uint16_t len;
	//	len = get_len(pData) + 4 + 2;
	if (pData[0] != START)
	{
		PRINTF("数据起始标志不正确，起始字节为0x%02x，不等于'&'\n", pData[0]);
		return -1;
	}
	uint16_t len = get_len(pData);
	if (len > 1024 || len < 10)
	{
		PRINTF("长度 = %d,长度不正确\n", len);
		return -5;
	}
	return 1;
}
void hex2byte(uint16_t hex, uint8_t *bytes)
{
	bytes[0] = hex >> 8;
	bytes[1] = hex & 0xff;
}
/*打包数据到buf*/
void encode(uint8_t *buf, uint8_t *srcAddr, uint8_t *msg, uint16_t msgLen, uint8_t cmd)
{
	uint16_t len = msgLen + DEVICE_LEN + 2 + 4 + 8 + 1; //总长度 2 + 4 + 8 + 9 = 26 - 6 = 20
	uint8_t pos = 0;
	buf[pos++] = START;
	buf[pos++] = PROTOCOL_TYPE;
	convert_len_to_assi(len - 4 - 2, (char *)&buf[pos]);
	pos = pos + 4;
	pos = pos + 2;
	hex2byte(DEVICE_TYPE, &buf[pos]);
	pos = pos + 2;
	DEVICE_ADDRESS[0] = 0x01;
	get_device_addr(&DEVICE_ADDRESS[1]);
	memcpy(&buf[pos], DEVICE_ADDRESS, sizeof(DEVICE_ADDRESS));
	pos = pos + DEVICE_LEN;
	hex2byte(sn++, &buf[pos]);
	pos = pos + 2;
	buf[pos++] = 0x80; //交互标识
	buf[pos++] = cmd;  //命令单元
	memcpy(&buf[pos], msg, msgLen);
	pos = pos + msgLen;
	buf[pos] = END; //结束标志
	hex2byte(check_sum(&buf[8], len - 6 - 2 - 1), &buf[6]);
	for (uint8_t i = 0; i < len; i++)
	{
		printf("%02x ", buf[i]);
	}
	printf("\n");
	//printf_parse(buf);
	mqtt_publish_data(buf, len);
	return;
}
void parse(uint8_t *data)
{
	if (check_data(data) < 0)
		return;
}
void printf_parse(uint8_t *data)
{
	uint16_t msgLen = 0; //实体层数据单元长度
	uint8_t i;
	uint16_t pos = 0;
	if (data[0] != '&')
		return;
	printf("{");
	printf("\n");
	printf("\"起始标志\":\"%c\"\n", data[pos++]);
	printf("\"协议类型\":\"%c\"\n", data[pos++]);
	msgLen = get_len(data) - 18;
	printf("\"数据单元长度\":\"%d\"\n", get_len(data));
	if (msgLen > 200)
		while (1)
			;
	pos = pos + 4;
	printf("\"校验和\":\"0x%04x\"\n", byte2hex(&data[pos]));
	pos = pos + 2;
	printf("\"设备类型\":\"0x%04x\"\n", byte2hex(&data[pos]));
	pos = pos + 2;
	printf("\"设备地址\":\"");
	for (i = 0; i < 9; i++)
	{
		printf("%02x", data[i + pos]);
	}
	pos = pos + 9;
	printf("\"\n");
	printf("\"通信标识\":\"0x%04x\"\n", byte2hex(&data[pos]));
	pos = pos + 2;
	printf("\"交互标识\":\"0x%02x\"\n", data[pos++]);
	printf("\"命令单元\":\"0x%02x\"\n", data[pos++]);
	printf("\"实体层数据单元(%d)\":\"", msgLen);
	for (i = 0; i < msgLen; i++)
	{
		if (i % 9 == 0)
			printf("\n");
		printf("0x%02x", data[pos + i]);
		if (i != msgLen - 1)
			printf(",");
	}
	pos = pos + msgLen;
	printf("\"");
	printf("\n");
	printf("\"结束标识\":\"%c\"\n", data[pos]);
	printf("}");
	printf("\n");
}

char hex2char(uint8_t hex, uint8_t flag)
{
	if (hex <= 9)
		return hex + '0';
	if (hex >= 10 && hex <= 15)
	{
		if (flag)
			return (hex - 10 + 'A');
		else
			return (hex - 10 + 'a');
	}
	return '0';
}
void hex2string(char *string, uint8_t *hex, uint16_t len, uint8_t flag)
{
	for (uint8_t i = 0; i < len; i++)
	{
		string[i * 2] = hex2char(hex[i] >> 4 & 0x0f, flag);
		string[i * 2 + 1] = hex2char(hex[i] & 0x0f, flag);
	}
	string[len * 2] = '\0';
}
int get_mqtt_password(uint8_t *deviceID, char *passwordMd5)
{
	uint8_t idLen = 0;
	uint8_t password[30] = "0";
	uint8_t md51[16];
	if (deviceID[0] == 0x01) //ti设备，设备ID长度是9个字节
	{
		idLen = 9;
		hex2string((char *)password, deviceID, idLen, 1);
		strcat((char *)password, "_zdst666");
		md5(password, strlen((const char *)password), (uint8_t *)md51);
		hex2string((char *)passwordMd5, md51, 16, 0);
		return 1;
	}
	return -1;
}
int get_mqtt_username(uint8_t *deviceID, char *username)
{
	uint8_t idLen = 0;
	if (deviceID[0] == 0x01) //ti设备，设备ID长度是9个字节
	{
		idLen = 9;
		hex2string((char *)username, deviceID, idLen, 1);
		strcat((char *)username, "_account");
		return 1;
	}
	return -1;
}
int get_mqtt_clientid(uint8_t *deviceID, char *client)
{
	uint8_t idLen = 0;
	if (deviceID[0] == 0x01) //ti设备，设备ID长度是9个字节
	{
		idLen = 9;
		hex2string((char *)client, deviceID, idLen, 1);
		strcat((char *)client, "_client");
		return 1;
	}
	return -1;
}

int get_mqtt_pubTopic(uint8_t *deviceID, char *pubTopic)
{
	uint8_t idLen = 0;
	if (deviceID[0] == 0x01) //ti设备，设备ID长度是9个字节
	{
		idLen = 9;
		strcpy((char *)pubTopic, "/ROUTING/1/");
		pubTopic = pubTopic + strlen("/ROUTING/1/");
		hex2string((char *)pubTopic, deviceID, idLen, 1);
		strcat((char *)pubTopic, "/pub");
		return 1;
	}
	return -1;
}

int get_mqtt_subTopic(uint8_t *deviceID, char *subTopic)
{
	uint8_t idLen = 0;
	if (deviceID[0] == 0x01) //ti设备，设备ID长度是9个字节
	{
		idLen = 9;
		strcpy((char *)subTopic, "/ROUTING/1/");
		subTopic = subTopic + strlen("/ROUTING/1/");
		hex2string((char *)subTopic, deviceID, idLen, 1);
		strcat((char *)subTopic, "/sub");
		return 1;
	}
	return -1;
}

//根据命令标识处理来自服务器的数据
void cmdProcess(uint8_t cmd,uint8_t *pData,uint16_t dataLen)
{
	uint16_t key;
	attrList_t *pAttr;
    switch(cmd)
	{
		case 0x32:
		printf("设置\n");
		while(dataLen > 2)
		{
		    key = *pData++ & 0xff;
		    key |= (*pData++ << 8 )& 0xff;
			dataLen = dataLen - 2;
		    pAttr = findAttr(key);
			if(dataLen > pAttr->len)
			dataLen = dataLen - pAttr->len;
			else
			{
				printf("长度错误");
			    return ;
			}
			memmove(pAttr->data,pData,pAttr->len);
			pData = pData + pAttr->len;
		}
		break;
		case 0x42:		//子设备列表下发
		printf("====================子设备列表下发======================\n");
		uint8_t index;
		index = 0;
		while(dataLen > 8)
		{
			index++;
	    	printf("设备%d :",index);
			for(uint8_t i = 0;i < 9;i++)
			{
				printf("%02X",*pData++);
			}
			printf("\n");
		}
		printf("=====================================================\n");
		break;
		case 0x43://子设备列表变更下发
		printf("子设备列表变更下发\n");
		printf("====================子设备列表变更下发======================\n");
		while(dataLen > 9)
		{
			if(*pData++ == 0x01)
			printf("新增设备");
			else if(*pData++ == 0x02)
			printf("删除设备");
			else
			printf("错误\n");
			return ;
			uint8_t index;
			index = 0;
	    	printf("设备%d :",index);
			for(uint8_t i = 0;i < 9;i++)
			{
				printf("%02X",*pData++);
			}
			printf("\n");
		}		
		printf("=====================================================\n");
		break;
		default:
		printf("无效命令字%d\n",cmd);
		break;
	}
}

int isSendToMe(uint8_t *pData)
{
    for(uint8_t i = 0;i < 9;i++)
	{
	    if(pData[i] != device_addr[i])
        return 0;
	}
	return 1;
}
void printfArray(uint8_t *data,uint8_t len,char* name,char* format)
{
	printf("%s ",name);
    for(uint8_t i = 0;i < len;i++)
	printf(format,data[i]);
	printf("\n");
}
void iotPasre(uint8_t *pData,frame_t *frame)
{
    uint16_t pos = 0;
	uint16_t playloadLen = 0;
	playloadLen = get_len(pData) - 18;
    frame->start= pData[pos++];
    frame->protocolType = pData[pos++];
	for(uint8_t i = 0;i < 4;i++)
	frame->len[i] = pData[pos++];
	frame->checkSum[0] = pData[pos++];
	frame->checkSum[1] = pData[pos++];
	frame->app.deviceType = (pData[pos++] << 8) & 0xff;
	frame->app.deviceType |= pData[pos++] & 0xff;
	for(uint8_t i = 0;i < 9;i++)
	frame->app.deviceID[i] = pData[pos++];
	frame->app.sequence = (pData[pos++] << 8) & 0xff;
	frame->app.sequence |= pData[pos++] & 0xff;
	frame->app.sign = pData[pos++];
	frame->app.commond = pData[pos++];
	for(uint8_t i = 0;i < playloadLen;i++)
	frame->playload[i] = pData[pos++];
	frame->end = pData[pos++];
	printf("=========================================================================\n");
	printf("起始标志 %c\n",frame->start);
	printf("协议类型 %c\n",frame->protocolType);
	printfArray((uint8_t*)frame->len,4,"数据长度 0x","%c");
	printf("Msg length %d\n",playloadLen);
	printf("校验和 0x%c%c\n",frame->checkSum[0],frame->checkSum[1]);
	/*APP LAYER*/
	printf("设备类型 0x%04x\n",frame->app.deviceType);
	printf("帧序号 %d\n",frame->app.sequence);
	printfArray(frame->app.deviceID,9,"设备地址 ","%02x");
	printf("交互标识 0x%02x\n",frame->app.sign);
	printf("命令单元 0x%02x\n",frame->app.commond);
	printfArray(frame->playload,playloadLen,"数据单元 ","%02x");
	printf("结束标识 %c\n",frame->end);
	printf("=========================================================================\n");
}
void SendAckToServer(uint8_t *pData)
{
	//uint16_t len;
	//len = get_len(pData);
	pData[2] = 0x30; 
	pData[3] = 0x30;
	pData[4] = 0x31;
	pData[5] = 0x32;
    pData[21] = 0x00;//正常应答
    pData[23] = '!';//正常应答
    
}



/*打包数据到buf*/
void SendToNode(uint8_t *buf, uint8_t *dstAddr, uint8_t *msg, uint16_t msgLen, uint8_t cmd)
{
	uint16_t len = msgLen + DEVICE_LEN + 2 + 4 + 8 + 1; //总长度 2 + 4 + 8 + 9 = 26 - 6 = 20
	uint8_t pos = 0;
	buf[pos++] = START;
	buf[pos++] = PROTOCOL_TYPE;
	convert_len_to_assi(len - 4 - 2, (char *)&buf[pos]);
	pos = pos + 4;
	pos = pos + 2;
	hex2byte(DEVICE_TYPE, &buf[pos]);
	pos = pos + 2;
	DEVICE_ADDRESS[0] = 0x01;
	get_device_addr(&DEVICE_ADDRESS[1]);
	memcpy(&buf[pos], DEVICE_ADDRESS, sizeof(DEVICE_ADDRESS));
	pos = pos + DEVICE_LEN;
	hex2byte(sn++, &buf[pos]);
	pos = pos + 2;
	buf[pos++] = 0x80; //交互标识
	buf[pos++] = cmd;  //命令单元
	memcpy(&buf[pos], msg, msgLen);
	pos = pos + msgLen;
	buf[pos] = END; //结束标志
	printf("Send data To node\n");
	hex2byte(check_sum(&buf[8], len - 6 - 2 - 1), &buf[6]);
	for (uint8_t i = 0; i < len; i++)
	{
		printf("%02x ", buf[i]);
	}
	printf("\n");
	return;
}