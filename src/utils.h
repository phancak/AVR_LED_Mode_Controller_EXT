#ifndef UTILS_H
#define UTILS_H

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

#define SYSTEM_STATUS_OK 1

void uart_init(uint32_t baud);
void uart_putchar(char c);
void uart_print(char* s);
void uart_debug_reg8(const char* name, uint8_t val);
void uart_debug_reg16(const char* name, uint16_t val);
void uart_debug_binary8(const char* name, uint8_t val) ;
void uart_debug_binary16(const char* name, uint16_t val);

void range_finder_get();

#endif