
#ifndef TEMPERATURE_H
#define TEMPERATURE_H

void TemperatureSensor_Init(void);
void TemperatureSensor_Task(void);

extern QueueHandle_t TemperatureSensor_Queue;
#endif

