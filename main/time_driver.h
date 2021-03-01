#ifndef _TIME_DRIVER_H_
#define _TIME_DRIVER_H_

#include "freertos/queue.h"
#include "driver/timer.h"
#include "driver/periph_ctrl.h"


typedef struct {
    int type;  // the type of timer's event
    int timer_group;
    int timer_idx;
    uint64_t timer_counter_value;
} timer_event_t;

xQueueHandle timer_queue;
void time_driver_init(void);

#endif