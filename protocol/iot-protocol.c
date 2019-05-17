#include "iot-protocol.h"
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

#include "net/netstack.h"
#include "net/ipv6/uip.h"

#include "net/linkaddr.h"
#include "sys/log.h"
#define START '&'
#define END '!'
#define PROTOCOL_TYPE 'D'
#define FRAME_MAX_LEN 1024
/* Log configuration */
#include "sys/log.h"
#define LOG_MODULE "Main"
#define LOG_LEVEL LOG_LEVEL_MAIN
uint8_t sn = 0;
/*---------------------------------------------------------------------------*/
#define DEBUG 1
#if DEBUG
#define PRINTF(...) printf(__VA_ARGS__)
#else
#define PRINTF(...)
#endif
typedef struct frame_t
{
	/*接入层*/
    uint8_t start;//起始标志
    char protocolType;//协议类型
    char len[4];//数据长度，ascii,长度0x0a=10= 0x30 0x30 0x30 0x3a
	char chsum[2];//校验码，0x0a=0x30 0x3a
	/*应用层*/
	uint16_t deviceType;
	uint8_t deviceAddr[9];
	uint16_t sn;
	uint8_t crossFlag;//交互标识
	uint8_t commond;
	uint8_t playload[FRAME_MAX_LEN];
	char end;
}frame_t;

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
void get_node_mac_addr(uint8_t *node_mac_addr)
{
    node_mac_addr[0] = 0x01;
    for(uint8_t i = 0;i < 8;i++)
    node_mac_addr[i + 1] = linkaddr_node_addr.u8[i];
    printf("linkaddr_node_addr addr = ");
    for(uint8_t i = 0;i < 9;i++)
    printf("%02x ",node_mac_addr[i]);
    printf("\n");
    printf("Link-layer address: ");
    LOG_PRINT_LLADDR(&linkaddr_node_addr);
    printf("\n");

}
void get_device_addr(uint8_t *device_addr)
{
	get_node_mac_addr(device_addr);
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
/*
 *
 */
void convert_len_to_assi(uint16_t len, char *assi_len)
{
	uint8_t i = 0;
	assi_len[0] = hex2ascii(len >> 8) >> 8;
	assi_len[1] = hex2ascii(len >> 8) & 0xff;
	assi_len[2] = hex2ascii(len & 0xff) >> 8;
	assi_len[3] = hex2ascii(len & 0xff) & 0xff;
}

void hex2byte(uint16_t hex, uint8_t *bytes)
{
	bytes[0] = hex >> 8;
	bytes[1] = hex & 0xff;
}
/*打包数据到buf*/
uint16_t encode(uint8_t *buf, uint8_t *srcAddr, uint8_t *msg, uint16_t msgLen, uint8_t cmd)
{
	uint16_t len = msgLen + DEVICE_LEN + 2 + 4 + 8 + 1; //总长度 2 + 4 + 8 + 9 = 26 - 6 = 20
	uint8_t pos = 0;
	buf[pos++] = START;
	buf[pos++] = PROTOCOL_TYPE;
	convert_len_to_assi(len - 4 - 2, &buf[pos]);
	pos = pos + 4;
	pos = pos + 2;
	hex2byte(DEVICE_TYPE, &buf[pos]);
	pos = pos + 2;
	get_device_addr(DEVICE_ADDRESS);
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
	printf_parse(buf);
	return len;
}
uint8_t ascii2hex(char ascii)
{
	uint8_t hex = 0;
	if (ascii < 'F' && ascii > 'A')
		hex = ascii - 'A' + 10;
	if (ascii < 'f' && ascii > 'a')
		hex = ascii - 'a' + 10;
	if (ascii < '9' && ascii > '0')
		hex = ascii - '0';
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
void printf_parse(uint8_t *data)
{
	uint16_t msg_len = 0; //实体层数据单元长度
	uint8_t i;
	uint16_t pos = 0;
	printf("{");
	printf("\n");
	printf("\"起始标志\":\"%c\"\n", data[pos++]);
	printf("\"协议类型\":\"%c\"\n", data[pos++]);
	msg_len = get_len(data) - 18;
	printf("\"数据单元长度\":\"%d\"\n", get_len(data));
	pos = pos + 4;
	printf("\"校验和\":\"0x%04x\"\n", byte2hex(&data[pos]));
	pos = pos + 2;
	printf("\"设备类型\":\"0x%04x\"\n", byte2hex(&data[pos]));
	pos = pos + 2;
	printf("\"设备地址\":\"");
	for (i = 0; i < 9; i++)
	{
		printf("%02x", data[i + pos]);
		if (i != 8)
			printf(":");
	}
	pos = pos + 9;
	printf("\"\n");
	printf("\"通信标识\":\"0x%04x\"\n", byte2hex(&data[pos]));
	pos = pos + 2;
	printf("\"交互标识\":\"0x%02x\"\n", data[pos++]);
	printf("\"命令单元\":\"0x%02x\"\n", data[pos++]);
	printf("\"实体层数据单元(%d)\":\"",msg_len);
	for (i = 0; i < msg_len; i++)
	{
		if (i % 9 == 0)
			printf("\n");
		printf("0x%02x", data[pos + i]);
		if (i != msg_len - 1)
			printf(",");
	}
	pos = pos + msg_len;
	printf("\"");
	printf("\n");
	printf("\"结束标识\":\"%c\"\n", data[pos]);
	printf("}");
	printf("\n");
}

uint16_t test(uint8_t *buf,uint8_t *msg,uint16_t len)
{
    printf("iot-protocol test \n");
    return encode(buf, DEVICE_ADDRESS, msg, len, 0x01);//主动告警
}