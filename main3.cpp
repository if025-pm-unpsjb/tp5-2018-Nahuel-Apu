#include "FreeRTOS.h"
#include "task.h"
#include "mbed.h"
#include "semphr.h"

struct Tasks
{
  char name[10];
  TickType_t ci;
  TickType_t ti;
  TickType_t di;
  UBaseType_t priority;
};

Tasks tareas[3] =
  {
    { "t1", 100, 4000, 4000, 4 },
    { "t2", 100, 5000, 5000, 3 },
    { "t3", 4000, 12000, 12000, 2 } };

SemaphoreHandle_t xSemaphore = 0;

Serial pc (USBTX, USBRX);

//------------------------------------------------------------------------------

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

/* Esta función accede a un recurso compartido. */

void access_shared_resource(TickType_t retardo) {

  eatCpu (retardo);

}

void
thread (void* params)
{
  TickType_t prev = 0;
  int i = (int) params;
  TickType_t inicio = 0;
  TickType_t final = 0;
  int inst = 0;
  while (1)
    {

      if(xSemaphoreTake(xSemaphore, portMAX_DELAY)== pdTRUE){
          pc.printf ("La tarea %d accede al recurso\n\r", i+1);
          access_shared_resource(tareas[i].ci);

          xSemaphoreGive(xSemaphore);
      }

      vTaskDelayUntil (&prev, tareas[i].ti);
      inst++;

    }
}

int
main ()
{
  xSemaphore = xSemaphoreCreateMutex();

  for (int i = 0; i < 3; i++)
    {
      xTaskCreate (thread, tareas[i].name, 256, (void *) i, tareas[i].priority,
                   NULL);
    }
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
