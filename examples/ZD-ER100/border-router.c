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
#include <string.h>
#include <syslog.h>
#include "kvconf.h"


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "MQTTClient.h"
#if !defined(WIN32)
#include <unistd.h>
#else
#include <windows.h>
#endif
#include "protocol.h"
#include "device-description.h"
#include "mqtt-config.h"
#include "net/netstack.h"
#include "net/ipv6/uip.h"
#include "log.h"
#include "net/net-debug.h"
#include "uip-ds6.h"
//#include "sicslowpan.h"
/*---------------------------------------------------------------------------*/
/* Log configuration */
#include "sys/log.h"
#include "sicslowpan.h"
#define LOG_MODULE "RPL BR"
#define LOG_LEVEL LOG_LEVEL_INFO

#define DISCONNECT "out"
#define FW_ENABLE 1 //是否转发到服务器
#define PRINTF_UDP_DATA 1
/*---------------------------------------------------------------------------*/
int CONNECT = 1;
volatile MQTTClient_deliveryToken deliveredtoken;
//声明一个MQTTClient
MQTTClient client = NULL;
MQTTClient_message pubmsg = MQTTClient_message_initializer;
//声明消息token
MQTTClient_deliveryToken token;
char username[50]; //添加的用户名
char password[50]; //添加的密码
char clientID[50]; //客户端id
char pubTopic[50]; //推送的topic
char subTopic[50]; //订阅的topic
char serverIP[50];
//初始化MQTT Client选项
MQTTClient_connectOptions conn_opts = MQTTClient_connectOptions_initializer;
uint32_t second = 0;
/*---------------------------------------------------------------------------*/
#define DEBUG 1
#if DEBUG
#undef PRINTF
#define PRINTF(...) printf(__VA_ARGS__)
#else
#define PRINTF(...)
#endif
/*---------------------------------------------------------------------------*/
#define WITH_SERVER_REPLY 0
#define UDP_CLIENT_PORT 8765
#define UDP_SERVER_PORT 5678
static struct simple_udp_connection udp_conn;
void mqtt_publish_data(uint8_t *data, uint16_t len);
process_event_t br_start_event = 0;
PROCESS(udp_server_process, "UDP server");
/*---------------------------------------------------------------------------*/
static void
udp_rx_callback(struct simple_udp_connection *c,
                const uip_ipaddr_t *sender_addr,
                uint16_t sender_port,
                const uip_ipaddr_t *receiver_addr,
                uint16_t receiver_port,
                const uint8_t *data,
                uint16_t datalen)
{
#if PRINTF_UDP_DATA
  int rssi = sicslowpan_get_last_rssi();
  printf("Received data (%d),rssi %d dbm", datalen,rssi);
  for (uint8_t i = 0; i < datalen; i++)
  printf("%02x", data[i]);
  printf("from ");
  LOG_PRINT_6ADDR(sender_addr);
  printf("\n");
#endif
/*转发到服务器*/
#if FW_ENABLE
  if (check_data((uint8_t *)data) > 0)
  {
    printf("转发数据(%d)",datalen);
    for(uint8_t i = 0;i < datalen;i++)
    printf("%02x",data[i]);
    printf("到%s\n",serverIP);
    mqtt_publish_data((uint8_t *)data, datalen);
  }
  else
  {
    printf("%s\n", data);
  }
#endif
}
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(udp_server_process, ev, data)
{
  PROCESS_BEGIN();
  /* Initialize DAG root */
  printf("udp server process\n");
  /* Initialize UDP connection */
  simple_udp_register(&udp_conn, UDP_SERVER_PORT, NULL,
                      UDP_CLIENT_PORT, udp_rx_callback);
  PROCESS_END();
}

void delivered(void *context, MQTTClient_deliveryToken dt)
{
  PRINTF("Message with token value %d delivery confirmed\n", dt);
  deliveredtoken = dt;
}

int msgarrvd(void *context, char *topicName, int topicLen, MQTTClient_message *message)
{
  int i;
  uint8_t *payloadptr;
  PRINTF("Message arrived ");
  PRINTF("     topic: %s (%d)", topicName, message->payloadlen);
  PRINTF("   message: ");
  payloadptr = message->payload;
  //uint16_t len = message->payloadlen;
  if (strcmp((char *)payloadptr, DISCONNECT) == 0)
  {
    PRINTF(" \n out!!");
    CONNECT = 0;
  }
  for (uint16_t i = 0; i < message->payloadlen; i++)
  printf("%02x", payloadptr[i]);
  printf("\n");
  frame_t frame;
  iotPasre(payloadptr,&frame);
  if (check_data((uint8_t *)payloadptr) > 0)
  {
    /* send back the same string to the client as an echo reply */
    if(isSendToMe(frame.app.deviceID))//发给我的
    {
      printf("this is send to me\n");
      if(frame.app.commond == 0x32)
      {
          payloadptr[21] = 0x00;
          mqtt_publish_data(payloadptr,message->payloadlen);
      }
      else
      {
          SendAckToServer((uint8_t *)payloadptr);
          mqtt_publish_data(payloadptr,24);
      }
      frame_t sendframe;
      printf("br-------------------------->server\n");
      iotPasre(payloadptr,&sendframe);
      printf("send-------------------------->compele\n");
    }
    else
    {
      uip_ipaddr_t sender_addr;
      uip_ip6addr(&sender_addr, UIP_DS6_DEFAULT_PREFIX, 0, 0, 0, 0, 0, 0, 0);
      for (uint8_t i = 0; i < 8; i++)
       sender_addr.u8[i + 8] = payloadptr[11 + i];
      if (sender_addr.u8[8] & 0x02)
       sender_addr.u8[8] &= ~0x02;
      else
       sender_addr.u8[8] |= 0x02;
      uint16_t datalen = message->payloadlen;
      PRINTF("Sending response from server (%d) token (%d)", datalen,token);
      printf("to ");
      LOG_PRINT_6ADDR(&sender_addr);
      printf("\n");
      simple_udp_sendto(&udp_conn, payloadptr, datalen, &sender_addr);
    }

  }
  else
  {
    for (i = 0; i < message->payloadlen; i++)
    {
      PRINTF("0x%02x ", *payloadptr++);
    }
    PRINTF("\n");
  }
  MQTTClient_freeMessage(&message);
  MQTTClient_free(topicName);
  return 1;
}
void connlost(void *context, char *cause)
{
  PRINTF("\nConnection lost\n");
  PRINTF("     cause: %s\n", cause);
}

void mqtt_publish_data(uint8_t *data, uint16_t len)
{
  if (MQTTClient_isConnected(client) != true)
  {
    printf("mqtt 连接断开\n");
    return;
  }
  pubmsg.payload = data;
  pubmsg.payloadlen = len;
  pubmsg.qos = QOS;
  pubmsg.retained = 0;
  if (MQTTClient_publishMessage(client, pubTopic, &pubmsg, &token) != MQTTCLIENT_SUCCESS) //发送失败
  {
    printf("发送失败\n");
  }
  printf("Waiting for up to %d seconds for publication of %s on topic %s for client with ClientID: %s\n",
  (int)(TIMEOUT / 1000), (char *)pubmsg.payload, pubTopic, clientID);
  MQTTClient_waitForCompletion(client, token, TIMEOUT);
  printf("Message with delivery token %d delivered\n", token);
}

void mqtt_subscribing_process(uint8_t *data)
{
  static int rc;
  //使用参数创建一个client，并将其赋值给之前声明的client
  setenv("MQTT_C_CLIENT_TRACE", "../../log/client.log", 1); // same as 'stdout'
  setenv("MQTT_C_CLIENT_TRACE_LEVEL", "PROTOCOL", 1);       //ERROR, PROTOCOL, MINIMUM, MEDIUM and MAXIMUM
  get_mqtt_username(DEVICE_ADDRESS, username);
  get_mqtt_password(DEVICE_ADDRESS, password);
  get_mqtt_clientid(DEVICE_ADDRESS, clientID);
  get_mqtt_pubTopic(DEVICE_ADDRESS, pubTopic);
  get_mqtt_subTopic(DEVICE_ADDRESS, subTopic);
  printf("username %s\n", username);
  printf("password %s\n", password);
  printf("clientID %s\n", clientID);
  printf("pubTopic %s\n", pubTopic);
  printf("subTopic %s\n", subTopic);
  MQTTClient_create(&client, serverIP, clientID,
                    MQTTCLIENT_PERSISTENCE_NONE, NULL);
  conn_opts.keepAliveInterval = 30;
  conn_opts.cleansession = 1;
  conn_opts.username = username; //将用户名写入连接选项中
  conn_opts.password = password; //将密码写入连接选项中

  //使用MQTTClient_connect将client连接到服务器，使用指定的连接选项。成功则返回MQTTCLIENT_SUCCESS
  MQTTClient_setCallbacks(client, NULL, connlost, msgarrvd, delivered);
  if ((rc = MQTTClient_connect(client, &conn_opts)) != MQTTCLIENT_SUCCESS)
  {
    PRINTF("Failed to connect, return code %d\n", rc);
    return;
  }
  pubmsg.payload = data;
  pubmsg.payloadlen = sizeof(data);
  pubmsg.qos = QOS;
  pubmsg.retained = 0;
  PRINTF("Connected to server addr:%s\n", serverIP);
  PRINTF("Subscribing to topic %s for client %s using QoS%d\n",subTopic, clientID, QOS);
  MQTTClient_subscribe(client, subTopic, QOS);
}
void mqtt_distconnect()
{
  MQTTClient_unsubscribe(client, subTopic);
  PRINTF("mqtt disconnect\n");
  MQTTClient_disconnect(client, 10000);
  MQTTClient_destroy(&client);  
}
/*
 *IPV6地址转MAC地址
 */
void uip_addr_to_mac_addr(const uip_ipaddr_t *uip_ipaddr, uint8_t *uip_macaddr)
{
  uint8_t i = 0;
  for (i = 0; i < 8; i++)
    uip_macaddr[i] = uip_ipaddr->u8[i + 8];
  if (uip_macaddr[0] & 0x02)
    uip_macaddr[0] &= ~0x02;
  else
    uip_macaddr[0] |= 0x02;
}
/*---------------------------------------------------------------------------*/
//发送拓扑到服务器
void update_links_to_server(const char *str)
{
  uint8_t buffer[1024];
  uip_ipaddr_t child_ipaddr;
  uip_ipaddr_t parent_ipaddr;
  uint8_t child_mac_addr[8];
  uint8_t parent_mac_addr[8];
  uip_sr_node_t *link;
  uip_ipaddr_t root_ipaddr;
  uint8_t root_mac_addr[8];
  uint8_t i = 0;
  uint8_t msg[1024];
  uint16_t msgLen = 0;
  printf("拓扑上报\n");
  if (rpl_dag_root_is_root())
  {
    NETSTACK_ROUTING.get_root_ipaddr(&root_ipaddr);
    uip_addr_to_mac_addr((const uip_ipaddr_t *)&root_ipaddr, root_mac_addr);
    PRINTF("[root mac Addr ]  ");
    for (i = 0; i < sizeof(root_mac_addr); i++)
    {
      PRINTF("%02x", root_mac_addr[i]);
      if (i != sizeof(root_mac_addr) - 1)
        PRINTF(":");
    }
    PRINTF("\n");
    PRINTF("[root ip addr]  ");
    LOG_PRINT_6ADDR(&root_ipaddr);
    PRINTF("\n");
    if (uip_sr_num_nodes() > 0)
    {
      /* Our routing links */
      PRINTF("links: %u routing links in total (%s)\n", uip_sr_num_nodes(), str);
      link = uip_sr_node_head();
      while (link != NULL)
      {
        char buf[100];
        if (link->parent != NULL) //不是根节点，如果是根节点的话，父亲为空
        {
          NETSTACK_ROUTING.get_sr_node_ipaddr(&child_ipaddr, link);
          NETSTACK_ROUTING.get_sr_node_ipaddr(&parent_ipaddr, link->parent);
          uip_addr_to_mac_addr((const uip_ipaddr_t *)&child_ipaddr, child_mac_addr);
          uip_addr_to_mac_addr((const uip_ipaddr_t *)&parent_ipaddr, parent_mac_addr);
          msg[msgLen++] = 0x01;
          memcpy(&msg[msgLen], child_mac_addr, sizeof(child_mac_addr));
          msgLen = msgLen + sizeof(child_mac_addr);
          msg[msgLen++] = 0x01;
          memcpy(&msg[msgLen], parent_mac_addr, sizeof(parent_mac_addr));
          msgLen = msgLen + sizeof(parent_mac_addr);
          PRINTF("child mac addr:");
          for (uint8_t i = 0; i < 8; i++)
            PRINTF("%02x ", child_mac_addr[i]);
          PRINTF("(parent :");
          for (uint8_t i = 0; i < 8; i++)
            PRINTF("%02x ", parent_mac_addr[i]);
          PRINTF(")\n");
        }
        uip_sr_link_snprint(buf, sizeof(buf), link);
        PRINTF("links: %s\n", buf);
        link = uip_sr_node_next(link);
      }
      PRINTF("links: end of list\n");
      encode(buffer, DEVICE_ADDRESS, msg, msgLen, 0x41);
    }
    else
    {
      PRINTF("No routing links\n");
    }
  }
}
/*---------------------------------------------------------------------------*/

/* Declare and auto-start this file's process */
PROCESS(mqtt_client_process, "Contiki-NG Border Router");
PROCESS_THREAD(mqtt_client_process, ev, data)
{
  static struct etimer timer;
  PROCESS_BEGIN();
  PRINTF("Mqtt Process\n");
  etimer_set(&timer, CLOCK_SECOND * 10);
  while (1)
  {
    get_device_addr(&DEVICE_ADDRESS[1]);
    DEVICE_ADDRESS[0] = 0x01;
    if(DEVICE_ADDRESS[3] != 0 && MQTTClient_isConnected(client) != true)//是否已经连上slip-radio
    {
        int channel = IEEE802154_DEFAULT_CHANNEL;
        uint32_t panID = IEEE802154_CONF_PANID;
        NETSTACK_RADIO.set_value(RADIO_PARAM_CHANNEL,channel);
        NETSTACK_RADIO.set_value(RADIO_PARAM_PAN_ID,panID);
        PRINTF("Setting channell to  %d\n",channel);
        PRINTF("Setting panID to  0x%04x\n",panID);
        mqtt_subscribing_process(data);
    }
    else if(MQTTClient_isConnected(client) == true)
    {
      etimer_set(&timer, CLOCK_SECOND * 60);
    }
    PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&timer));
    etimer_reset(&timer);
  }

  PROCESS_END();
}

/* Declare and auto-start this file's process */
PROCESS(contiki_ng_br, "Contiki-NG Border Router");
AUTOSTART_PROCESSES(&contiki_ng_br, &mqtt_client_process);
#define INTERVAL 60
#define TIME_INTERVAL (CLOCK_SECOND * INTERVAL)
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(contiki_ng_br, ev, data)
{
  PROCESS_BEGIN();
  uint8_t data[] = "adamsxiaomi";
  static struct etimer timer;
#if BORDER_ROUTER_CONF_WEBSERVER
  PROCESS_NAME(webserver_nogui_process);
  process_start(&webserver_nogui_process, NULL);
#endif /* BORDER_ROUTER_CONF_WEBSERVER */
  PRINTF("Contiki-NG Border Router started %s %s \n", __DATE__, __TIME__);
  int channel = IEEE802154_DEFAULT_CHANNEL;
  uint32_t panID = IEEE802154_CONF_PANID;
  NETSTACK_RADIO.set_value(RADIO_PARAM_CHANNEL,channel);
  NETSTACK_RADIO.set_value(RADIO_PARAM_PAN_ID,panID);
  PRINTF("Setting channell to  %d\n",channel);
  PRINTF("Setting panID to  0x%04x\n",panID);
  process_start(&udp_server_process, data);
  /* Setup a periodic timer that expires after 10 seconds. */
  etimer_set(&timer, CLOCK_SECOND * INTERVAL);
  char* key = "ip";
  kvpair* keyValues;
  getkvpairs("./br.conf", &keyValues);//读取配置文件
  printf("\033[32mgetValueBykey:key = %s; value = %s\033[0m\n", key, key2val(key, keyValues));//读取key
  strcpy(serverIP,key2val(key, keyValues));
  printf("serverIP : %s",serverIP);
  while (1)
  {
    second = second + INTERVAL;
    printf("root links(%u second):\r\n", second);
    char *str = "adamsxiaomi";
    if (rpl_dag_root_is_root())
    {
      if (uip_sr_num_nodes() > 0)
      {
        update_links_to_server("adams");
        //SendToNode(buf,dstaddr,msg,2,0x80);
        uip_sr_node_t *link;
        /* Our routing links */
        printf("links: %u routing links in total (%s)\n", uip_sr_num_nodes(), str);
        link = uip_sr_node_head();
        while (link != NULL)
        {
          char buf[100];
          uip_sr_link_snprint(buf, sizeof(buf), link);
          printf("links: %s\n", buf);
          link = uip_sr_node_next(link);
        }
        printf("links: end of list\n");
      }
      else
      {
        printf("No routing links\n");
      }

    }
    PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&timer));
    etimer_reset(&timer);
  }
  PROCESS_END();
}
