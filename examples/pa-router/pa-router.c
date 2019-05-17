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

#include "contiki.h"
#include "net/routing/routing.h"
#include "random.h"
#include "net/netstack.h"
#include "net/ipv6/simple-udp.h"
#include <stdio.h> /* For printf() */
#include "pa/pa.h"
#include "uip.h"
#include "ti-lib.h"
#include "sys/log.h"
#include "sicslowpan.h"
#define LOG_MODULE "PA"
#define LOG_LEVEL LOG_LEVEL_INFO
/*---------------------------------------------------------------------------*/
PROCESS(pa_process, "pa_process");
AUTOSTART_PROCESSES(&pa_process);
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(pa_process, ev, data)
{
  static struct etimer timer;
  uip_ipaddr_t dest_ipaddr;
  PROCESS_BEGIN();
  init_pa();
  printf("pa_process\n");
  /* Setup a periodic timer that expires after 10 seconds. */
  etimer_set(&timer, CLOCK_SECOND * 1);
  ti_lib_ioc_pin_type_gpio_output(IOID_6);
  ti_lib_ioc_pin_type_gpio_output(IOID_7);
  ti_lib_ioc_pin_type_gpio_output(IOID_8);
  //ti_lib_gpio_clear_dio(CTX);
  ti_lib_gpio_set_dio(IOID_6);
  ti_lib_gpio_set_dio(IOID_7);
  ti_lib_gpio_set_dio(IOID_8);
  while (1)
  {
    if (NETSTACK_ROUTING.node_is_reachable() && NETSTACK_ROUTING.get_root_ipaddr(&dest_ipaddr))
    {
      ti_lib_gpio_clear_dio(IOID_7);
      ti_lib_gpio_set_dio(IOID_6);
    }
    else
    {
      LOG_INFO("Not reachable yet\n");
      ti_lib_gpio_set_dio(IOID_7);
      ti_lib_gpio_clear_dio(IOID_6);
    }

    int rssi = sicslowpan_get_last_rssi();
    printf("rssi %d dbm\n", rssi);
    /* Wait for the periodic timer to expire and then restart the timer. */
    PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&timer));
    etimer_reset(&timer);
  }

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
