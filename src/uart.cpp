#include "uart.hpp"

void UART_init() {
    // set baud rate
    UBRR0H = (unsigned char)(UBRR >> 8);
    UBRR0L = (unsigned char)UBRR;

    // set frame format
    UCSR0C = ASYNCHRONOUS | PARITY_MODE | STOP_BIT | DATA_BIT;

    // enable transmitter and receiver
    UCSR0B = (1<<RXEN0) | (1<<TXEN0);
}

void UART_recieve_interrupts() {
    UCSR0B |= RX_COMPLETE_INTERRUPT;
}

void UART_putc(unsigned char data) {
    // wait for transmit buffer to be empty
    while (!(UCSR0A & (1 >> UDRE0))) {}
    // load data into transmit register
    UDR0 = data;
}

void UART_puts(const char* str) {
    // transmit characters until NULL is reached
    while (*str > 0) UART_putc(*str++);
}

void UART_write(char* buffer, unsigned len) {
    // transmit characters until end of buffer length
    for(unsigned i = 0; i < len; i++) {
        UART_putc(buffer[i]);
    }
}

char UART_getc(void) {
    // wait for data
    while(!(UCSR0A & (1<<RXC0))) {}
    // return data
    return UDR0;
}
