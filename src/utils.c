#include "utils.h"
#include <util/delay.h>

#define DEBUG 0x0001

#ifdef DEBUG
    // Function to initialize UART
    void uart_init(uint32_t baud) {
        // Calculate the UBRR value based on F_CPU and desired baud
        // Formula: UBRR = (F_CPU / (16 * baud)) - 1
        uint16_t ubrr_value = (F_CPU / (16 * baud)) - 1;
        
        // Set baud rate registers
        UBRR0H = (uint8_t)(ubrr_value >> 8);
        UBRR0L = (uint8_t)ubrr_value;

        // Enable transmitter
        UCSR0B = (1 << TXEN0);

        // Set frame format: 8-bit data, 1 stop bit, no parity (8N1)
        UCSR0C = (1 << UCSZ01) | (1 << UCSZ00);
    }

    // Function to send a single character
    void uart_putchar(char c) {
        // Wait for empty transmit buffer (UDRE0 bit becomes 1)
        while (!(UCSR0A & (1 << UDRE0)));
        // Put data into buffer, sends the data
        UDR0 = c;
    }

    // Function to send a full string
    /**
     * When you define a string in C like this:
    const char* s = "Hi";
    The compiler actually stores three bytes in memory, not two:
    'H' (ASCII 72)
    'i' (ASCII 105)
    '\0' (The Null Terminator, ASCII 0)
    Also const declaration - unchangable in flash
    without const will be in SRAM. takin up space
    */
    void uart_print(char* s) {
        while (*s) {
            uart_putchar(*s++);
        }
    }

    /**
     * @brief Sends an 8-bit register in Hex format (e.g., PORTB: 0x25)
     */
    void uart_debug_reg8(const char* name, uint8_t val) {
        const char hex_chars[] = "0123456789ABCDEF";
        
        uart_print((char*)name); // Cast to match your function signature
        uart_print(": 0x");
        
        uart_putchar(hex_chars[(val >> 4) & 0x0F]); // High nibble
        uart_putchar(hex_chars[val & 0x0F]);        // Low nibble
        uart_print("\r\n");
    }

    /**
     * @brief Sends a 16-bit register in Hex format (e.g., TCNT1: 0xABCD)
     */
    void uart_debug_reg16(const char* name, uint16_t val) {
        const char hex_chars[] = "0123456789ABCDEF";
        
        uart_print((char*)name);
        uart_print(": 0x");
        
        uart_putchar(hex_chars[(val >> 12) & 0x0F]); // Highest
        uart_putchar(hex_chars[(val >> 8) & 0x0F]);
        uart_putchar(hex_chars[(val >> 4) & 0x0F]);
        uart_putchar(hex_chars[val & 0x0F]);         // Lowest
        uart_print("\r\n");
    }

    /**
     * @brief Sends an 8-bit register in binary (e.g., TCCR1A: 01000000)
     */
    void uart_debug_binary8(const char* name, uint8_t val) {
        uart_print((char*)name);
        uart_print(": ");
        
        for (int8_t i = 7; i >= 0; i--) {
            if (val & (1 << i)) {
                uart_putchar('1');
            } else {
                uart_putchar('0');
            }
        }
        uart_print("\r\n");
    }

    /**
     * @brief Sends a 16-bit register in binary with a separator (e.g., TCNT1: 00001111_01010101)
     */
    void uart_debug_binary16(const char* name, uint16_t val) {
        uart_print((char*)name);
        uart_print(": ");
        
        for (int8_t i = 15; i >= 0; i--) {
            // Send '1' or '0' based on the bit state
            if (val & (uint16_t)(1 << i)) {
                uart_putchar('1');
            } else {
                uart_putchar('0');
            }

            // Add an underscore separator between the High and Low bytes
            if (i == 8) {
                uart_putchar('_');
            }
        }
        uart_print("\r\n");
    }
#else
    //In non-Debug mode the functions are empty
    #define uart_init(baud)           ((void)0)
    #define uart_putchar(c)           ((void)0)
    #define uart_print(s)             ((void)0)
    #define uart_debug_reg8(n, v)     ((void)0)
    #define uart_debug_reg16(n, v)    ((void)0)
    #define uart_debug_binary8(n, v)  ((void)0)
    #define uart_debug_binary16(n, v) ((void)0)
#endif