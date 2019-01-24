/*
 * Copyright (c) 2018, Texas Instruments Incorporated - http://www.ti.com/
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
/**
 * \addtogroup srf06-peripherals
 * @{
 *
 * \file
 *        Driver for the SmartRF06 EB ALS sensor.
 * \author
 *        Edvard Pettersen <e.pettersen@ti.com>
 */
/*---------------------------------------------------------------------------*/
#include "contiki.h"
#include "dev/gpio-hal.h"
#include "lib/sensors.h"
#include "sys/timer.h"

#include "als-sensor.h"
/*---------------------------------------------------------------------------*/
#include <Board.h>

#include <ti/drivers/ADC.h>
/*---------------------------------------------------------------------------*/
#include <stdint.h>
/*---------------------------------------------------------------------------*/
static ADC_Handle adc_handle;
/*---------------------------------------------------------------------------*/
static int
init(void)
{
  ADC_Params adc_params;
  ADC_Params_init(&adc_params);

  adc_handle = ADC_open(Board_ADCALS, &adc_params);
  if(adc_handle == NULL) {
    return 0;
  }

  return 1;
}
/*---------------------------------------------------------------------------*/
static int
config(int type, int enable)
{
  switch(type) {
  case SENSORS_HW_INIT:
    return init();

  case SENSORS_ACTIVE:
    gpio_hal_arch_pin_set_output(Board_ALS_PWR);
    gpio_hal_arch_pin_set_input(Board_ALS_OUT);

    if(enable) {
      gpio_hal_arch_set_pin(Board_ALS_PWR);
      clock_delay_usec(2000);
    } else {
      gpio_hal_arch_clear_pin(Board_ALS_PWR);
    }
    break;

  default:
    break;
  }
  return 1;
}
/*---------------------------------------------------------------------------*/
static int
value(int type)
{

  uint16_t adc_value = 0;
  int_fast16_t res = ADC_convert(adc_handle, &adc_value);
  if(res != ADC_STATUS_SUCCESS) {
    return -1;
  }

  return (int)adc_value;
}
/*---------------------------------------------------------------------------*/
static int
status(int type)
{
  return 1;
}
/*---------------------------------------------------------------------------*/
SENSORS_SENSOR(als_sensor, ALS_SENSOR, value, config, status);
/*---------------------------------------------------------------------------*/
/** @} */
