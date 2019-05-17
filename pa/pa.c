#include "ti-lib.h"
#include "../arch/cpu/cc26x0-cc13x0/lib/cc13xxware/driverlib/ioc.h"
#include "stdio.h"
#include "lpm.h"
#include "pa.h"
#include "contiki-net.h"
#include "smartrf-settings.h"
#define CTX IOID_1
#define CSD IOID_30


/*
CTX : LOW  & CSD :HIGH ->RX
CTX : HIGN & CSD :HIGH ->TX
CTX : X    & CSD :LOW  ->SLEEP
*/
/*
void wakeup_handle()
{
  pa_set_radio_rx();
  ti_lib_gpio_write_dio(RGB_YELLOW, 1);
}

void shutdown_handle(uint8_t mode)
{
  pa_set_radio_sleep();
  ti_lib_gpio_write_dio(RGB_YELLOW, 0);
}

LPM_MODULE(pa_module, NULL, shutdown_handle, wakeup_handle, LPM_DOMAIN_NONE);
*/
/*---------------------------------------------------------------------------*/
#define DEBUG 0
#if DEBUG
#undef PRINTF
#define PRINTF(...) printf(__VA_ARGS__)
#else
#define PRINTF(...)
#endif
/*---------------------------------------------------------------------------*/

void pa_set_radio_tx()
{
  PRINTF("PA set to TX\n");
    /* Adjust RF Front End and Bias based on the board */
  smartrf_settings_cmd_prop_radio_div_setup.config.frontEndMode =
    1;
  smartrf_settings_cmd_prop_radio_div_setup.config.biasMode =
    1;
/*  ti_lib_ioc_port_configure_set(CTX, IOC_PORT_GPIO, IOC_IOMODE_NORMAL | IOC_IOPULL_UP);
  ti_lib_ioc_port_configure_set(CSD, IOC_PORT_GPIO, IOC_IOMODE_NORMAL | IOC_IOPULL_UP);
  ti_lib_gpio_get_output_enable_dio(CTX);
  ti_lib_gpio_get_output_enable_dio(CSD);
  ti_lib_gpio_write_dio(CTX, 1);
  ti_lib_gpio_write_dio(CSD, 1);*/
  ti_lib_gpio_set_dio(CTX);
  ti_lib_gpio_set_dio(CSD);
}

void pa_set_radio_rx()
{
  PRINTF("PA set to rx\n");
  ti_lib_gpio_clear_dio(CTX);
  ti_lib_gpio_set_dio(CSD);
  /* Adjust RF Front End and Bias based on the board */
  smartrf_settings_cmd_prop_radio_div_setup.config.frontEndMode =
    2;
  smartrf_settings_cmd_prop_radio_div_setup.config.biasMode =
    1;
printf("bias = %d,frontEndMode=%d\n",
smartrf_settings_cmd_prop_radio_div_setup.config.biasMode
,  smartrf_settings_cmd_prop_radio_div_setup.config.frontEndMode
);
  //ti_lib_ioc_port_configure_set(CTX, IOC_PORT_GPIO, IOC_IOMODE_NORMAL | IOC_IOPULL_UP);
  //ti_lib_ioc_port_configure_set(CSD, IOC_PORT_GPIO, IOC_IOMODE_NORMAL | IOC_IOPULL_UP);
  //ti_lib_gpio_get_output_enable_dio(CTX);
  //ti_lib_gpio_get_output_enable_dio(CSD);
  //ti_lib_gpio_write_dio(CTX, 0);
  //ti_lib_gpio_write_dio(CSD, 1);
}

void pa_set_radio_sleep()
{
  ti_lib_gpio_write_dio(CSD, 0);
}
void init_pa()
{
  printf("PA Init CTX=%d,CSD%d\n", CTX, CSD);
  ti_lib_ioc_port_configure_set(CTX, IOC_PORT_GPIO, IOC_IOMODE_NORMAL | IOC_IOPULL_UP);
  ti_lib_ioc_port_configure_set(CSD, IOC_PORT_GPIO, IOC_IOMODE_NORMAL | IOC_IOPULL_UP);
  ti_lib_ioc_port_configure_set(RGB_RED, IOC_PORT_GPIO, IOC_IOMODE_NORMAL | IOC_IOPULL_UP);
  ti_lib_ioc_port_configure_set(RGB_BLUE, IOC_PORT_GPIO, IOC_IOMODE_NORMAL | IOC_IOPULL_UP);
 // ti_lib_ioc_port_configure_set(RGB_GREEN, IOC_PORT_GPIO, IOC_IOMODE_NORMAL | IOC_IOPULL_UP);
  ti_lib_ioc_pin_type_gpio_output(CTX);
  ti_lib_ioc_pin_type_gpio_output(CSD);
  /*ti_lib_gpio_write_dio(RGB_RED, 0);
  ti_lib_gpio_write_dio(RGB_BLUE, 1);
  ti_lib_gpio_write_dio(RGB_GREEN, 1);
  */
  pa_set_radio_rx();
  //while(1);
 // while(1);
  /*
  NETSTACK_RADIO.send("caonima",5);
  */
  	/* Register ourselves with the LPM module */
	//lpm_register_module(&pa_module);
  pa_set_radio_rx();
}