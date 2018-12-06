#include "FreeRTOS.h"
#include "task.h"
#include "mbed.h"
#include "queue.h"
#include "semphr.h"

QueueHandle_t xQueue2, xQueue4, xQueue5;
SemaphoreHandle_t xSemaphore = 0;

Serial pc (USBTX, USBRX);

void
eatCpu (TickType_t ticks)
{
  TickType_t cnt = 0;
  TickType_t prev = xTaskGetTickCount ();
  while (true)
    {
      if (prev < xTaskGetTickCount ())
        {
          cnt++;
          prev = xTaskGetTickCount ();
        }
      if (cnt > ticks)
        {
          break;
        }

    }
}

void
thread1 (void* params)
{
  TickType_t prev = 0;
  int msg = 1;
  while (1)
    {

      eatCpu (1000);

      if ( xQueueSend(xQueue2 , (void * ) &msg, portMAX_DELAY) == pdTRUE)
        {
          if (xSemaphoreTake(xSemaphore, portMAX_DELAY) == pdTRUE)
            {
              pc.printf ("Mensaje enviado de t1 a t2.\n\r");
              xSemaphoreGive(xSemaphore);
            }
        }

      vTaskDelayUntil (&prev, 4000);

    }
}

void
thread2 (void* params)
{
  TickType_t prev = 0;
  int msg, recibido;
  while (1)
    {

      if (xQueueReceive (xQueue2, &recibido, portMAX_DELAY))
        {
          if (xSemaphoreTake(xSemaphore, portMAX_DELAY) == pdTRUE)
            {
              pc.printf ("t2: mensaje recibido de t1.\n\r");
              xSemaphoreGive(xSemaphore);
            }
        }

      eatCpu (1000);
      msg = 2;
      if ( xQueueSend(xQueue5 , (void * ) &msg, portMAX_DELAY) == pdTRUE)
        {

          if (xSemaphoreTake(xSemaphore, portMAX_DELAY) == pdTRUE)
            {
              pc.printf ("Mensaje enviado de t2 a t5.\n\r");
              xSemaphoreGive(xSemaphore);
            }
        }

      msg = 4;
      if ( xQueueSend(xQueue4 , (void * ) &msg, portMAX_DELAY) == pdTRUE)
        {

          if (xSemaphoreTake(xSemaphore, portMAX_DELAY) == pdTRUE)
            {
              pc.printf ("Mensaje enviado de t2 a t4.\n\r");
              xSemaphoreGive(xSemaphore);
            }
        }

      vTaskDelayUntil (&prev, 4000);

    }
}

void
thread3 (void* params)
{
  TickType_t prev = 0;
  int msg = 3;
  while (1)
    {

      eatCpu (1000);

      if ( xQueueSend(xQueue5 , (void * ) &msg, portMAX_DELAY) == pdTRUE)
        {
          if (xSemaphoreTake(xSemaphore, portMAX_DELAY) == pdTRUE)
            {
              pc.printf ("Mensaje enviado de t3 a t5.\n\r");
              xSemaphoreGive(xSemaphore);
            }
        }

      vTaskDelayUntil (&prev, 4000);

    }
}

void
thread4 (void* params)
{
  TickType_t prev = 0;
  int msg;
  while (1)
    {

      if (xQueueReceive (xQueue4, &msg, portMAX_DELAY))
        {
          if (xSemaphoreTake(xSemaphore, portMAX_DELAY) == pdTRUE)
            {
              pc.printf ("t4: mensaje recibido de t2.\n\r");
              xSemaphoreGive(xSemaphore);
            }
        }
      eatCpu (1000);

      vTaskDelayUntil (&prev, 4000);

    }
}

void
thread5 (void* params)
{
  TickType_t prev = 0;
  int msg;
  int *cont;
  while (1)
    {

      if (xQueueReceive (xQueue5, &msg, portMAX_DELAY))
        {
          if (xSemaphoreTake(xSemaphore, portMAX_DELAY) == pdTRUE)
            {
              cont = &msg;
              if (*cont == 2)
                pc.printf ("t5: mensaje recibido de t2.\n\r");

              else
                pc.printf ("t5: mensaje recibido de t3.\n\r");

            }
          xSemaphoreGive(xSemaphore);
        }
    }

  eatCpu (1000);

  vTaskDelayUntil (&prev, 4000);
}

int
main ()
{
  xQueue2 = xQueueCreate(3, sizeof(int));
  xQueue4 = xQueueCreate(3, sizeof(int));
  xQueue5 = xQueueCreate(3, sizeof(int));
  xSemaphore = xSemaphoreCreateMutex();

  xTaskCreate (thread1, "t1", 256, NULL, 2, NULL);
  xTaskCreate (thread2, "t2", 256, NULL, 2, NULL);
  xTaskCreate (thread3, "t3", 256, NULL, 2, NULL);
  xTaskCreate (thread4, "t4", 256, NULL, 2, NULL);
  xTaskCreate (thread5, "t5", 256, NULL, 2, NULL);
  vTaskStartScheduler ();
  for (;;)
    ;
}

void
vConfigureTimerForRunTimeStats (void)
{
  const unsigned long TCR_COUNT_RESET = 2, CTCR_CTM_TIMER = 0x00,
      TCR_COUNT_ENABLE = 0x01;

  /* Power up and feed the timer with a clock. */
  LPC_SC->PCONP |= 0x02UL;
  LPC_SC->PCLKSEL0 = (LPC_SC->PCLKSEL0 & (~(0x3 << 2))) | (0x01 << 2);

  /* Reset Timer 0 */
  LPC_TIM1->TCR = TCR_COUNT_RESET;

  /* Just count up. */
  LPC_TIM1->CTCR = CTCR_CTM_TIMER;

  /* Prescale to a frequency that is good enough to get a decent resolution,
   but not too fast so as to overflow all the time. */
  LPC_TIM1->PR = ( configCPU_CLOCK_HZ / 10000UL) - 1UL;

  /* Start the counter. */
  LPC_TIM1->TCR = TCR_COUNT_ENABLE;
}
