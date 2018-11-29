#include "FreeRTOS.h"
#include "task.h"
#include "mbed.h"

extern "C"{
void vApplicationIdleHook( void );
}

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
    { "t1", 1000, 4000, 4000, 3 },
    { "t2", 1000, 5000, 5000, 2 },
    { "t3", 3000, 6000, 6000, 1 } };

DigitalOut leds[3] =
  { LED1, LED2, LED3 };

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

//xTaskCallApplicationTaskHook

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
      inicio = xTaskGetTickCount ();
      eatCpu (tareas[i].ci);
      final = xTaskGetTickCount ();
      pc.printf ("Tarea %d [%d, %d, %d]\n\r", i + 1, inicio, final, inst);
      if(final > tareas[i].di * (inst +1 )){
          pc.printf ("Se supero el Vencimiento de la tarea %d en %d MS\n\r", i + 1, final - tareas[i].di * (inst +1 ));
      }
      vTaskDelayUntil (&prev, tareas[i].ti);
      inst++;

    }
}

int
main ()
{
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

void vApplicationIdleHook (void)
{
  pc.printf ("Me llamaron :D\n");
}
