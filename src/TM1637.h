#ifndef TM1637_H
#define TM1637_H

#include <avr/io.h>

//Pin Mapping
#define TM_DDR   DDRC  //Direction dat register (LOW - Input, HIGH - Output)
#define TM_PORT  PORTC //Ouput data register (HIGH, LOW)
#define TM_PIN   PINC  //Input data register
#define TM_DIO   PC1   //Is first pin in GPIOC (1)
#define TM_CLK   PC2   //Is second pin in GPIOC (2)

//Function Prototypes
void tm1637_init(void);
void tm1637_show_number(uint16_t num);
void tm1637_write_byte(uint8_t data);

#endif
/*
#ifndef TM1637_H
#define TM1637_H

#include <avr/io.h>

// Define pins - adjust these to your modular PCB layout
#define TM_DDR  DDRD
#define TM_PORT PORTD
#define TM_PIN  PIND
#define TM_CLK  PD2
#define TM_DIO  PD3

//TM1637 Constants
#define TM1637_SIGNAL_PERIOD	((uint32_t)0x0012UL)       //Target 100KHz 0x00A0UL 0x0009UL
#define TM1637_1HALF_PERIOD		(TM1637_SIGNAL_PERIOD/2)  //First half of period length
#define TM1637_2HALF_PERIOD		(TM1637_SIGNAL_PERIOD-TM1637_1HALF_PERIOD) //Second half of period length

//TM1637 Commands
#define TM1637_COMMAND_AAA	((uint8_t)0x40UL) //Automatic address adding
#define TM1637_COMMAND_FA	((uint8_t)0x44UL) //Fix address
#define TM1637_COMMAND_C0H	((uint8_t)0xC0UL) //Display address C0H
#define TM1637_COMMAND_PW16	((uint8_t)0x80UL) //Pulse width is set as 1/16.
#define TM1637_COMMAND_DATA	((uint8_t)0x3FUL) //
#define TM1637_COMMAND_ON	((uint8_t)0x88UL) //Display ON
#define TM1637_COMMAND_OFF	((uint8_t)0x80UL) //Display OFF

// Public API
void tm1637_pin_init();
uint8_t tm1637_write_data_auto_increment(int8_t* digits);

#endif
*/