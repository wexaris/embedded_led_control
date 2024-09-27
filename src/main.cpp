#ifndef F_CPU // Assume default
#define F_CPU 16000000UL
#endif

#include "uart.hpp"

#include <avr/interrupt.h>
#include <avr/io.h>
#include <util/delay.h>

#include <string.h>

///////////////////////////////////////
/// Set LED pins during build

#ifndef LED_PIN_0
#define LED_PIN_0 0
#endif

#ifndef LED_PIN_1
#define LED_PIN_1 1
#endif

#if (LED_PIN_0 < 0 || LED_PIN_0 > 3)
#error "invalid LED_PIN_0 value; must be in range [0-3]"
#endif

#if (LED_PIN_1 < 0 || LED_PIN_1 > 3)
#error "invalid LED_PIN_1 value; must be in range [0-3]"
#endif

///////////////////////////////////////
/// Define PORT_B and its pins

#define PORT_B_DIR DDRB // Port B direction
#define PORT_B_VAL PORTB // Port B data

#define PIN_0 DDB5
#define PIN_1 DDB4
#define PIN_2 DDB3
#define PIN_3 DDB2
#define PIN_4 DDB1
#define PIN_5 DDB0

/// Resolve a pin ID to a pin.
char resolve_pin_id(char pin_id) {
    switch (pin_id) {
        case 0: return PIN_0;
        case 1: return PIN_1;
        case 2: return PIN_2;
        case 3: return PIN_3;
        // case 4: return PIN_4;
        // case 5: return PIN_5;
        default: return -1;
    }
}

/// Resolve a LED ID to a pin.
char resolve_led_pin(char led_id) {
    switch (led_id) {
        case 0: return resolve_pin_id(LED_PIN_0);
        case 1: return resolve_pin_id(LED_PIN_1);
        default: return -1;
    }
}

///////////////////////////////////////
/// Define PORT_B and its pins

/// Set a pin to work as output.
void set_pin_output(char pin) {
    // since all out pins are on PORT_B, we don't need to do extra resolution
    PORT_B_DIR |= _BV(pin);
}

void set_pin_high(char pin) { PORT_B_VAL |=  _BV(pin); }
void set_pin_low(char pin)  { PORT_B_VAL &= ~_BV(pin); }

/// Start a timer with a certain delay.
void timer_start(unsigned time) {
    TIMSK1 = (1 << TOIE1); // enable overflow interrupt
    TCNT1 = 65536 - (time / 0.064); // set timer
}

/// Stop the timer.
void timer_stop() {
    TIMSK1 = 0; // disable trigger interrupts
    TCNT1 = 0; // reset time to 0
}

void timer_init() {
    TCCR1A = 0x00; // set normal counter mode
    TCCR1B = (1<<CS10) | (1<<CS12); // set 1024 pre-scaler
    timer_stop();
}


int main() {
    // set pins to output
    set_pin_output(PIN_0);
    set_pin_output(PIN_1);
    set_pin_output(PIN_2);
    set_pin_output(PIN_3);

    UART_init();
    UART_recieve_interrupts();
    timer_init();
    sei(); // enable interrupts

    while (true) {}
    return 0;
}

char MSG_Buffer[312];
unsigned MSG_BufferIdx = 0;
volatile char ACTIVE_LED_PIN = 0;

void clear_buffer() {
    memset(&MSG_Buffer[0], 0, sizeof(MSG_Buffer));
    MSG_BufferIdx = 0;
}

void msg_ok()    { UART_puts("OK\r\n"); clear_buffer(); }
void msg_error() { UART_puts("ERROR\r\n"); clear_buffer(); }

ISR (USART_RX_vect) {
    char recv = UDR0;

    // keep collecting the command buffer
    MSG_Buffer[(unsigned)MSG_BufferIdx++] = recv;

    // all commands should end with a '\r'
    if (recv == '\r') {

        if (!strncmp(&MSG_Buffer[0], "set-led ", 8) && MSG_BufferIdx > 11) {
            // read the given LED
            char led_id = MSG_Buffer[8] - '0';

            // our we only have LED_0 and LED_1
            if (led_id != 0 && led_id != 1) {
                msg_error();
                return;
            }

            // check for comma
            if (MSG_Buffer[9] != ',') {
                msg_error();
                return;
            }

            unsigned time = 0, idx = 10;
            // ensure a value is given
            if (MSG_Buffer[idx] < '0' && '9' < MSG_Buffer[idx]) {
                msg_error();
                return;
            }
            // parse value
            while (MSG_Buffer[idx] >= '0' && MSG_Buffer[idx] <= '9') {
                time = 10 * time + (MSG_Buffer[idx++] - '0');
            }

            // ensure message ended where expected
            if (MSG_Buffer[idx] != '\r') {
                msg_error();
                return;
            }

            // turn off any previously active LED
            if (ACTIVE_LED_PIN >= 0) {
                set_pin_low(ACTIVE_LED_PIN);
                ACTIVE_LED_PIN = -1;
            }

            // turn on the LED
            ACTIVE_LED_PIN = resolve_led_pin(led_id);
            set_pin_high(ACTIVE_LED_PIN);

            timer_start(time); // start expiry timer

            msg_ok();
        }
        else if (!strncmp(&MSG_Buffer[0], "echo ", 5) && MSG_BufferIdx > 9) {
            unsigned len = 0, idx = 5;
            // ensure a value is given
            if (MSG_Buffer[idx] < '0' && '9' < MSG_Buffer[idx]) {
                msg_error();
                return;
            }
            // parse value
            while (MSG_Buffer[idx] >= '0' && MSG_Buffer[idx] <= '9') {
                len = 10 * len + (MSG_Buffer[idx++] - '0');
            }

            // check for acceptable length
            if (len > 300) {
                msg_error();
                return;
            }

            // check for comma
            if (MSG_Buffer[idx++] != ',') {
                msg_error();
                return;
            }

            // check if data is incomplete
            if (MSG_BufferIdx < idx + len) {
                // can't know if input was malformed or there's more coming,
                // so keep waiting..
                return;
            }

            // ensure data ended where expected
            if (MSG_Buffer[idx + len] != '\r') {
                msg_error();
                return;
            }

            UART_puts("data: ");
            UART_write(&MSG_Buffer[idx], len);
            UART_puts("\r\n");

            msg_ok();
        }
        else { // unrecognised command
            msg_error();
        }
    }
}

/// Disable LED after given timer has expired.
ISR (TIMER1_OVF_vect) {
    // save and reset active pin
    char active_pin = ACTIVE_LED_PIN;
    ACTIVE_LED_PIN = -1;

    TIMSK1 = 0; // disable timer interrupt

    // turn off the LED
    set_pin_low(active_pin);

    // write message
    UART_puts("led-off: ");
    UART_putc('0' + active_pin);
    UART_puts("\r\n");
}
