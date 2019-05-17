/*
 * Copyright (c) 2010, Swedish Institute of Computer Science.
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
 */

#ifndef PROTOCOL_H_
#define PROTOCOL_H_
#include "device-description.h"

#define CHANGED_REPORT 1 //拓扑变更上报 0x41
#define PROTOCOL_TYPE 'D'

/*监控量列表*/
typedef struct attr_t
{
    uint16_t key;
    uint16_t len;
	void* data;
	char* name;
	uint8_t type;
}attrList_t;

/*
 *交互标志	含义
 *0x00	通信请求的执行结果，表示执行正常
 *0x01	通信请求的执行结果，表示设备忙，无法处理命令请求
 *0x02	无效的命令
 *0x03	长度错
 *0x04	校验错, 仅在调试时使用，正式产品中不返回此信息
 *0x80	正常的命令请求
 */
typedef struct monitor_t
{
	uint16_t *key;
	void *value;
} monitor_t;
/*---------------------------------------------------------------------------*/
typedef struct app_header_t //应用层
{
#if PROTOCOL_TYPE == 'A' || PROTOCOL_TYPE == 'B' || PROTOCOL_TYPE == 'C'
	uint8_t deviceType;
#elif PROTOCOL_TYPE == 'D'
	uint16_t deviceType;
#endif
	uint8_t deviceID[9]; //设备地址，长度由第一个字节决定
	uint16_t sequence;		//通信包标识，帧序号
	uint8_t sign;			//交互标志
	uint8_t commond;		//命令单元
} app_header_t;
/*---------------------------------------------------------------------------*/
typedef struct physical_header_t //实体层
{
	monitor_t monitor[1024]; //实体层数据单元
} physical_header_t;
/*---------------------------------------------------------------------------*/
typedef struct frame_t //接入层
{
	uint8_t start;
	char protocolType;
	char len[4];
	char checkSum[2];
	app_header_t app;
	uint8_t playload[1024];
	uint8_t end;
} frame_t;
/*---------------------------------------------------------------------------*/

typedef enum
{
    NORMAL_REQUEST = 0x00, //通信请求的执行结果，表示执行正常
    BUSY,                  //通信请求的执行结果，表示设备忙，无法处理命令请求
    UNAVAILABLE,           //无效的命令
    ERROR_LEN,             //长度错误
    ERROR_CHECKSUM,        //校验和错误
    NORMAL_CMD_REQUEST     //正常的命令请求
} COMUNICATION_SIGN;       //交互标识

typedef enum
{
    PROHIBIT = 0x00,     //禁止使用
    INIT_REPORT,         //初始化上报
    CONFIG_CHANGE_REPORT //设备配置变更上报
} CMD_TYPE;
uint16_t get_len(uint8_t *pData);
void test_protocol();
void printf_parse(uint8_t *data);
int check_data(uint8_t *pData);
void encode(uint8_t *buf, uint8_t *srcAddr, uint8_t *msg, uint16_t msgLen,uint8_t cmd);
void fill_msg(uint8_t *buf,uint16_t len,uint8_t sendStatus);
void uip_addr_to_mac_addr1(const uip_ipaddr_t *uip_ipaddr, uint8_t *uip_macaddr);
void report_disconnect(uip_ipaddr_t *node_addr);
int get_mqtt_password(uint8_t *deviceID,char* passwordMd5);
int get_mqtt_clientid(uint8_t *deviceID,char* client);
int get_mqtt_username(uint8_t *deviceID,char* username);
int get_mqtt_clientid(uint8_t *deviceID,char* client);
int get_mqtt_pubTopic(uint8_t *deviceID,char* pubTopic);
int get_mqtt_subTopic(uint8_t *deviceID,char* subTopic);
void get_device_addr(uint8_t *device_addr);
void hex2string(char *string,uint8_t *hex,uint16_t len,uint8_t flag);
void iotPasre(uint8_t *pData,frame_t *frame);
int isSendToMe(uint8_t *pData);

void SendAckToServer(uint8_t *pData);
void deallocate_report();
void add_report();
#endif /* PROJECT_CONF_H_ */
