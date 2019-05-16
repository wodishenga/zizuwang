/*
 * Copyright (c) 2016, University of Bristol - http://www.bristol.ac.uk
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
/*---------------------------------------------------------------------------*/
/**
 * \addtogroup cc26xx-adc-sensor
 * @{
 *
 * \file
 * Driver for the CC13xx/CC26xx ADC
 */
/*---------------------------------------------------------------------------*/
#include "contiki.h"
#include "lib/sensors.h"
#include "dev/adc-sensor.h"
#include "sys/timer.h"
#include "lpm.h"
#include "dev/gpio-hal.h"
#include "board.h"
#include "ti-lib.h"
#include "driverlib/aux_adc.h"
#include "aux-ctrl.h"
#include <stdio.h>
#include <stdbool.h>
#include <math.h>
#include "app.h"
/*---------------------------------------------------------------------------*/
 uint8_t adcChannel = ADC_COMPB_IN_AUXIO2;

 aux_consumer_module_t adc_aux_pressure = {
  .clocks = AUX_WUC_ADI_CLOCK | AUX_WUC_ANAIF_CLOCK | AUX_WUC_SMPH_CLOCK
};
/*---------------------------------------------------------------------------*/
extern u16 voltage ;

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

void openADC(void)
{
	aux_ctrl_register_consumer(&adc_aux_pressure);
	
	ti_lib_aux_adc_select_input(adcChannel);  

    
}

void closeADC(void)
{
	ti_lib_aux_adc_disable();

	aux_ctrl_unregister_consumer(&adc_aux_pressure);
	
	ti_lib_ioc_pin_type_gpio_input(ADC_COMPB_IN_AUXIO2);
	ti_lib_ioc_io_port_pull_set(ADC_COMPB_IN_AUXIO2, IOC_IOPULL_DOWN);
}
 int
getWaterPressure(void)
{

    static int val, adj_val,pressure;
	aux_ctrl_register_consumer(&adc_aux_pressure);
	
	ti_lib_aux_adc_select_input(adcChannel);

	ti_lib_aux_adc_enable_sync(AUXADC_REF_VDDS_REL, AUXADC_SAMPLE_TIME_21P3_US,
								   AUXADC_TRIGGER_MANUAL);

    ti_lib_aux_adc_gen_manual_trigger();
    val = ti_lib_aux_adc_read_fifo();
	
    adj_val = ti_lib_aux_adc_adjust_value_for_gain_and_offset(
        val,
        ti_lib_aux_adc_get_adjustment_gain(AUXADC_REF_VDDS_REL),
        ti_lib_aux_adc_get_adjustment_offset(AUXADC_REF_VDDS_REL));

	//adj_mv = ti_lib_aux_adc_value_to_microvolts(AUXADC_FIXED_REF_VOLTAGE_NORMAL, adj_val);
	
  // printf("val = %d, adj_val = %d adj_mv=%d \n", val,adj_val,adj_mv);

   ti_lib_aux_adc_disable();
   
	   aux_ctrl_unregister_consumer(&adc_aux_pressure);
	   
	   ti_lib_ioc_pin_type_gpio_input(ADC_COMPB_IN_AUXIO2);
	   ti_lib_ioc_io_port_pull_set(ADC_COMPB_IN_AUXIO2, IOC_IOPULL_DOWN);

	

	pressure = (adj_val*3600/4096);
	//printf("pressure = %d\n", pressure);

    return pressure;
}


