#ifndef __PA_H__
#define __PA_H__
#define RGB_RED IOID_6
#define RGB_GREEN IOID_7
#define RGB_BLUE IOID_8
void pa_set_radio_tx();
void pa_set_radio_rx();
void pa_set_radio_sleep();
void init_pa();

#endif