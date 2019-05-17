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

#ifndef MQTT_CONFIG_H_
#define MQTT_CONFIG_H_

#define MQTT_CONN_ADDR 6
/**********************************************************************/
#if MQTT_CONN_ADDR == 1
#define MQTT_NAME "localhost"
#define ADDRESS "tcp://localhost:1883"          //更改此处地址
#define CLIENTID "aaabbbccc"                    //更改此处客户端ID
#define TOPIC "/ROUTING/1/888888888888888/pub"  //更改发送的话题
#define TOPIC1 "/ROUTING/1/888888888888888/sub" //更改发送的话题
#define PAYLOAD "Hello Man, Can you see me ?!"  //更改信息内容
#define QOS 1
#define TIMEOUT 10000L
#define USERNAME ""
#define PASSWORD ""
/**********************************************************************/
#elif MQTT_CONN_ADDR == 2
#define MQTT_NAME "m2c.eclipse.org"
#define ADDRESS "m2m.eclipse.org"              //更改此处地址
#define CLIENTID "MQTT_FX_Client"              //更改此处客户端ID
#define SUB_CLIENTID "aaabbbccc_sub"           //更改此处客户端ID
#define TOPIC "/ROUTING/1/888888888888888/pub" //更改发送的话题
#define PAYLOAD "Hello Man!"                   //
#define QOS 1
#define TIMEOUT 10000L
#define USERNAME ""
#define PASSWORD ""
/**********************************************************************/
#elif MQTT_CONN_ADDR == 3//周会明
#define MQTT_NAME "zhouhuiming"
#define ADDRESS "172.16.0.20:18884"            //更改此处地址
#define CLIENTID "0100124b0013a4be48_client"               //更改此处客户端ID
#define SUB_CLIENTID "0100124b0013a4be48_client"           //更改此处客户端ID
#define TOPIC "/ROUTING/1/0100124b0013a4be48/pub" //发送的主题
#define TOPIC1 "/ROUTING/1/0100124b0013a4be48/sub" //订阅的主题
#define GLOBAL_TOPIC "/ROUTING/sub"
#define PAYLOAD "Hello Man!"                   
#define QOS 1
#define TIMEOUT 10000L
#define USERNAME "0100124b0013a4be48_account"
#define PASSWORD "400afeb6790df0a3a4ef91688c462176"
//#define TOPIC "/ROUTING/1/888888888888888/pub" //更改发送的话题
//#define TOPIC1 "/ROUTING/1/888888888888888/sub" //更改发送的话题
/**********************************************************************/
/**********************************************************************/
#elif MQTT_CONN_ADDR == 4//蔡泽林
#define MQTT_NAME "caizelin"
#define ADDRESS "172.16.0.19:18884"            //更改此处地址
#define CLIENTID "0100124b0013a4be48_client"               //更改此处客户端ID
#define SUB_CLIENTID "0100124b0013a4be48_client"           //更改此处客户端ID
#define TOPIC "/ROUTING/1/0100124b0013a4be48/pub" //发送的主题
#define TOPIC1 "/ROUTING/1/0100124b0013a4be48/sub" //订阅的主题
#define GLOBAL_TOPIC "/ROUTING/sub"
#define PAYLOAD "Hello Man!"                   
#define QOS 1
#define TIMEOUT 10000L
#define USERNAME "0100124b0013a4be48_account"
#define PASSWORD "e9ff2f52cdb091b1087699e7bc3291cb"
//#define TOPIC "/ROUTING/1/888888888888888/pub" //更改发送的话题
//#define TOPIC1 "/ROUTING/1/888888888888888/sub" //更改发送的话题
/**********************************************************************/

/**********************************************************************/
#elif MQTT_CONN_ADDR == 5//测试环境
#define MQTT_NAME "test"
#define ADDRESS "192.168.1.234:18884"            //更改此处地址
#define CLIENTID "0100124B0013A4BE48_client"               //更改此处客户端ID
#define SUB_CLIENTID "0100124B0013A4BE48_client"           //更改此处客户端ID
#define TOPIC "/ROUTING/1/0100124B0013A4BE48/pub" //发送的主题
#define TOPIC1 "/ROUTING/1/0100124B0013A4BE48/sub" //订阅的主题
#define GLOBAL_TOPIC "/ROUTING/sub"
#define PAYLOAD "Hello Man!"                   
#define QOS 1
#define TIMEOUT 10000L
#define USERNAME "0100124B0013A4BE48_account"
#define PASSWORD "400afeb6790df0a3a4ef91688c462176"
/**********************************************************************/

/**********************************************************************/
#elif MQTT_CONN_ADDR == 6//测试环境
#define MQTT_NAME "test"
#define ADDRESS "wlw3mqtt.xiaofangyj.cn:18884"            //更改此处地址
#define CLIENTID "0100124B0013A4BE48_client"               //更改此处客户端ID
#define SUB_CLIENTID "0100124B0013A4BE48_client"           //更改此处客户端ID
#define TOPIC "/ROUTING/1/0100124B0013A4BE48/pub" //发送的主题
#define TOPIC1 "/ROUTING/1/0100124B0013A4BE48/sub" //订阅的主题
#define GLOBAL_TOPIC "/ROUTING/sub"
#define PAYLOAD "Hello Man!"                   
#define QOS 1
#define TIMEOUT 10000L
#define USERNAME "0100124B0013A4BE48_account"
#define PASSWORD "400afeb6790df0a3a4ef91688c462176"
/**********************************************************************/
#endif

#endif