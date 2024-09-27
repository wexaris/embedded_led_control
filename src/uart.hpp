#pragma once
#include <avr/io.h>

#ifndef UART_BAUD
#define UART_BAUD 9600
#endif

#define UBRR ((F_CPU / (16UL * UART_BAUD)) - 1)

#define ASYNCHRONOUS (0<<UMSEL00) // Mode Selection

#define DISABLED    (0<<UPM00)
#define EVEN_PARITY (2<<UPM00)
#define ODD_PARITY  (3<<UPM00)
#define PARITY_MODE  DISABLED // Parity Bit Selection

#define ONE_BIT (0<<USBS0)
#define TWO_BIT (1<<USBS0)
#define STOP_BIT ONE_BIT      // Stop Bit Selection

#define FIVE_BIT  (0<<UCSZ00)
#define SIX_BIT   (1<<UCSZ00)
#define SEVEN_BIT (2<<UCSZ00)
#define EIGHT_BIT (3<<UCSZ00)
#define DATA_BIT   EIGHT_BIT  // Data Bit Selection

#define RX_COMPLETE_INTERRUPT         (1<<RXCIE0)
#define DATA_REGISTER_EMPTY_INTERRUPT (1<<UDRIE0)

void UART_init();
void UART_recieve_interrupts();

/// Write a single character.
void UART_putc(unsigned char data);
/// Write a const string, expecting it to end with a NULL character.
void UART_puts(const char* str);
/// Write a buffer of a certain length.
void UART_write(char* buffer, unsigned len);

/// Write a single character.
char UART_getc(void);
