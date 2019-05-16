/*
 * water_sensor.c
 *
 *  Created on: 2019年2月28日
 *      Author: ASUS
 */
#include "water_sensor.h"
#include "dev/gpio-hal.h"
#include "contiki.h"
#include "app.h"
static gpio_hal_event_handler_t Water_Sensor_Interrupt_handler; //定义中断handler
bool warning = false;
extern u8 eventType;
extern Sensor_Info Sensor;
extern int warningSentCount;
bool isNormal = false;
static void Water_Sensor_Alarm_handler(gpio_hal_pin_mask_t pin_mask)
{
	if(gpio_hal_arch_read_pin(26) == 0){
		printf("发生水浸事故!\r\n");
		warning = true;
	}else{
		printf("水浸事故已解决！\r\n");
		warning = false;
		eventType = 0;
		isNormal = true;
		Sensor.status = 0;
		warningSentCount = 0;
	}
}


void Water_sensor_hal_init(void)
{
	Water_Sensor_Interrupt_handler.pin_mask |= gpio_hal_pin_to_mask(26); //使用gpio 6 作为中断判断
	Water_Sensor_Interrupt_handler.handler = Water_Sensor_Alarm_handler;
	gpio_hal_arch_pin_set_input(26);
	gpio_hal_arch_pin_cfg_set(26, GPIO_HAL_PIN_CFG_EDGE_FALLING | GPIO_HAL_PIN_CFG_INT_ENABLE | GPIO_HAL_PIN_CFG_PULL_NONE);
	gpio_hal_arch_interrupt_enable(26);
	gpio_hal_register_handler(&Water_Sensor_Interrupt_handler);


}
