/*
 * Copyright (c) 2016, George Oikonomou - http://www.spd.gr
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the copyright holder nor the names of its
 *    contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE
 * COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
 * OF THE POSSIBILITY OF SUCH DAMAGE.
 */
/*---------------------------------------------------------------------------*/
#include "contiki.h"
#include "sensniff.h"
#include "dev/radio.h"
#include "net/netstack.h"
#include "net/packetbuf.h"
#include "sys/process.h"
#include "sys/ctimer.h"
#include "lib/ringbuf.h"

#include "ti-lib.h"
#include "net/mac/framer/framer-802154.h"
#include "net/mac/framer/frame802154.h"
#include "net/mac/llsec802154.h"

#include "lib/random.h"


#define LOG_MODULE "seniff"
#define LOG_LEVEL LOG_LEVEL_INFO


#include SENSNIFF_IO_DRIVER_H

#include <string.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "sys/log.h"

/*---------------------------------------------------------------------------*/
PROCESS(sensniff_process, "sensniff process");


AUTOSTART_PROCESSES(&sensniff_process);
/*---------------------------------------------------------------------------*/
/* Timeout handling for incoming characters. */
#define TIMEOUT (CLOCK_SECOND >> 1)
static struct ctimer ct;
/*---------------------------------------------------------------------------*/
#define STATE_WAITING_FOR_MAGIC     0x00
#define STATE_WAITING_FOR_VERSION   0x01
#define STATE_WAITING_FOR_CMD       0x02
#define STATE_WAITING_FOR_LEN_1     0x03
#define STATE_WAITING_FOR_LEN_2     0x04
#define STATE_WAITING_FOR_DATA      0x05

static uint8_t state;
static uint8_t in_ct;
/*---------------------------------------------------------------------------*/
#define CMD_FRAME               0x00
#define CMD_CHANNEL             0x01
#define CMD_CHANNEL_MIN         0x02
#define CMD_CHANNEL_MAX         0x03
#define CMD_ERR_NOT_SUPPORTED   0x7F
#define CMD_GET_CHANNEL         0x81
#define CMD_GET_CHANNEL_MIN     0x82
#define CMD_GET_CHANNEL_MAX     0x83
#define CMD_SET_CHANNEL         0x84
/*---------------------------------------------------------------------------*/
#define PROTOCOL_VERSION           2
/*---------------------------------------------------------------------------*/
#define BUFSIZE 32

static struct ringbuf rxbuf;

typedef struct cmd_in_s {
  uint8_t cmd;
  uint16_t len;
  uint8_t data;
} cmd_in_t;

static cmd_in_t command;

uint8_t cmd_buf[BUFSIZE];
/*---------------------------------------------------------------------------*/
static void
reset_state(void *byte)
{
  state = STATE_WAITING_FOR_MAGIC;
  in_ct = 0;
  memset(&command, 0, sizeof(command));
}

/*---------------------------------------------------------------------------*/
static void
set_channel(uint8_t channel)
{
  if(NETSTACK_RADIO.set_value(RADIO_PARAM_CHANNEL, channel) ==
     RADIO_RESULT_OK) {
    return;
  }
}
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
uint8_t cmd[2] = {0};
//static int cmdIndex = 0;

static int
char_in(unsigned char c)
{
  /* Bump the timeout counter */
  ctimer_set(&ct, TIMEOUT, reset_state, NULL);
  
  if(c>=0 && c<=32)
  	{
  		set_channel(c);
  	}

  return 1;
}

/*---------------------------------------------------------------------------*/
linkaddr_t sendaddr;

static uint8_t mac_addr[8];

static int
parseFrame(void)
{
	frame802154_t frame;
	int hdr_len;

	hdr_len = frame802154_parse(packetbuf_dataptr(), packetbuf_datalen(), &frame);

	if(hdr_len && packetbuf_hdrreduce(hdr_len)) {
	packetbuf_set_attr(PACKETBUF_ATTR_FRAME_TYPE, frame.fcf.frame_type);

	if(frame.fcf.dest_addr_mode) {
		if(frame.dest_pid != frame802154_get_pan_id() &&
			frame.dest_pid != FRAME802154_BROADCASTPANDID) {
	        /* Packet to another PAN */
	  }
	if(!frame802154_is_broadcast_addr(frame.fcf.dest_addr_mode, frame.dest_addr)) {
	        packetbuf_set_addr(PACKETBUF_ADDR_RECEIVER, (linkaddr_t *)&frame.dest_addr);
	      }
	}
	packetbuf_set_addr(PACKETBUF_ADDR_SENDER, (linkaddr_t *)&frame.src_addr);
	
	uint16_t panid = frame.dest_pid;
	if(panid != 0)
		{
			sensniff_io_byte_out(panid >> 8);
			sensniff_io_byte_out(panid & 0xFF);

			linkaddr_copy(&sendaddr, packetbuf_addr(PACKETBUF_ADDR_SENDER));
			memcpy(mac_addr, &sendaddr, 8);
			for(int i =0;i<8;i++){
		 	sensniff_io_byte_out(mac_addr[i]);
			}
			sensniff_io_byte_out(packetbuf_attr(PACKETBUF_ATTR_RSSI) & 0xFF);
			uint8_t a = 0x0a;
			sensniff_io_byte_out(a);
	 	}	
    return hdr_len;
  }
  return FRAMER_FAILED;
}


void
sensniff_output_frame()
{
  parseFrame();
}

/*---------------------------------------------------------------------------*/
PROCESS_THREAD(sensniff_process, ev, data)
{
  PROCESS_BEGIN();

  ti_lib_uart_fifo_enable(UART0_BASE);

  /* Turn off RF frame filtering and H/W ACKs */
  if(NETSTACK_RADIO.set_value(RADIO_PARAM_RX_MODE, 0) != RADIO_RESULT_OK) {
    printf("sensniff: Error setting RF in promiscuous mode\n");
    PROCESS_EXIT();
  }
  set_channel(1);

  /* Initialise the ring buffer */
  ringbuf_init(&rxbuf, cmd_buf, sizeof(cmd_buf));

  /* Initialise the state machine */
  reset_state(NULL);

  /* Register for char inputs with the character I/O peripheral */
  sensniff_io_set_input(&char_in);

  while(1) {
    PROCESS_YIELD();
  }

  PROCESS_END();
}

