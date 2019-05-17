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

#ifndef IOT_PROTOCOL_H_
#define IOT_PROTOCOL_H_
#include "stdint.h"
#define TEM_SENSOR
/*设备类型*/
/****************************************************/
#ifdef GATEWAY
uint8_t device_addr[9];
#define DEVICE_NAME "collect-gateway"//采集网关
#define DEVICE_TYPE 0x0100 
#define DEVICE_ADDRESS device_addr
#define DEVICE_LEN 9
#endif
/****************************************************/
#ifdef TEM_SENSOR
uint8_t device_addr[9];
#define DEVICE_NAME "tem_sensor"//温度传感器
#define DEVICE_TYPE 0x0102 
#define DEVICE_ADDRESS device_addr
#define DEVICE_LEN 9
#endif
uint16_t encode(uint8_t *buf, uint8_t *srcAddr, uint8_t *msg, uint16_t msgLen, uint8_t cmd);
uint16_t test(uint8_t *buf,uint8_t *msg,uint16_t len);
#endif
