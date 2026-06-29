/*
Heart Beat con CTC comp A y Check Pin PD5 con COMPB

Se requiere implementar un sistema de señalización tipo “heartbeat” utilizando CTC A del microcontrolador ATmega328P.

1) La señal debe generarse mediante interrupciones por match CTC y debe alternar el estado del pin PC4 aproximadamente cada 100 ms. 

2) El sistema debe operar sin bloqueos y delegar el control de tiempo a una interrupción.
 
3) Generar una señal periódica en el pin PC4 utilizando Timer0 en modo normal con interrupciones, permitiendo que el main quede libre para otras tareas.

4) Genera por medio de CTC COMPB una señal de toggle de led que se enciente sera naturalmente el pin PD5 asociado al registro toggle de OC0B


El sistema genera una señal visible tipo heartbeat en el pin PC4 que indica que el microcontrolador está funcionando correctamente a la par con el COMPB con hearbeat del doble del tiempo, esto no tiene sentido sino solo para probar el PD5. 

*/

#include <stdint.h>

/*==============================
  Registros
==============================*/

volatile uint8_t* DDRC_ADDR   = (uint8_t*)0x27;
volatile uint8_t* PORTC_ADDR  = (uint8_t*)0x28;

volatile uint8_t* DDRD_ADDR   = (uint8_t*)0x2A;

volatile uint8_t* TCCR0A_ADDR = (uint8_t*)0x44;
volatile uint8_t* TCCR0B_ADDR = (uint8_t*)0x45;
volatile uint8_t* TCNT0_ADDR  = (uint8_t*)0x46;
volatile uint8_t* OCR0A_ADDR  = (uint8_t*)0x47;
volatile uint8_t* OCR0B_ADDR  = (uint8_t*)0x48;

volatile uint8_t* TIFR0_ADDR  = (uint8_t*)0x35;
volatile uint8_t* TIMSK0_ADDR = (uint8_t*)0x6E;

/*==============================
  Macros
==============================*/

#define BIT(n) (1U << (n))

#define PC4_MASK BIT(4)
#define PD5_MASK BIT(5)

/* TCCR0A */
#define WGM01_MASK  BIT(1)
#define COM0B0_MASK BIT(4)

/* TIMSK0 */
#define OCIE0A_MASK BIT(1)

/* TIFR0 */
#define OCF0A_MASK  BIT(1)

/* TCCR0B */
#define CS01_MASK BIT(1)
#define CS00_MASK BIT(0)

/*==============================
  Configuración temporal

  F_CPU = 16 MHz
  Prescaler = 64

  Tick = 4 us

  OCR0A = 249

  (249+1)*4us = 1 ms
==============================*/

#define OCR0A_VALUE 249U

/*==============================
  Variables
==============================*/

volatile uint16_t heartbeat_counter = 0;
volatile uint8_t heartbeat_flag = 0;

/*==============================
  Timer0 CTC
==============================*/

static void timer0_ctc_init(void)
{
    *TCCR0A_ADDR = 0x00;
    *TCCR0B_ADDR = 0x00;

    /*
       COM0B0 = 1
       Toggle OC0B (PD5)
    */
    *TCCR0A_ADDR |= COM0B0_MASK;

    /*
       CTC Mode
    */
    *TCCR0A_ADDR |= WGM01_MASK;

    *TCNT0_ADDR = 0;

    /*
       TOP
    */
    *OCR0A_ADDR = OCR0A_VALUE;

    /*
       Compare B
       Evento intermedio
    */
    *OCR0B_ADDR = 125;

    *TIFR0_ADDR |= OCF0A_MASK;

    /*
       COMPA interrupt
    */
    *TIMSK0_ADDR |= OCIE0A_MASK;

    /*
       Prescaler 64
    */
    *TCCR0B_ADDR |= (CS01_MASK | CS00_MASK);

    __asm__ __volatile__("sei");
}

/*==============================
  TIMER0_COMPA
==============================*/

void __vector_14(void) __attribute__((signal));

void __vector_14(void)
{
    heartbeat_counter++;

    if (heartbeat_counter >= 100)
    {
        heartbeat_counter = 0;
        heartbeat_flag = 1;
    }
}

/*==============================
  Heartbeat
==============================*/

static void heartbeat_task(void)
{
    *PORTC_ADDR ^= PC4_MASK;
}

/*==============================
  Main
==============================*/

int main(void)
{
    /*
       PC4 heartbeat
    */
    *DDRC_ADDR |= PC4_MASK;

    /*
       PD5 = OC0B
    */
    *DDRD_ADDR |= PD5_MASK;

    timer0_ctc_init();

    while (1)
    {
        if (heartbeat_flag)
        {
            heartbeat_flag = 0;
            heartbeat_task();
        }

        /*
           Otras tareas
        */
    }
}
