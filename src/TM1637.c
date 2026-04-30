#include "TM1637.h"
#include <util/delay.h>
#include "utils.h"

static const uint8_t segment_map[] = {
    0x3f, 0x06, 0x5b, 0x4f, 0x66, 0x6d, 0x7d, 0x07, 0x7f, 0x6f
};

/**
 * @brief Generates START condition by ticking DIO LOW when CLK is HIGH
 * @modifies TM_PORT
 */
static void tm1637_start(void) {
    TM_PORT |= (1 << TM_DIO) | (1 << TM_CLK);
    _delay_us(2);
    TM_PORT &= ~(1 << TM_DIO);
}

/**
 * @brief Generates STOP condition by ticking DIO HIGH when CLK is HIGH
 * @modifies TM_PORT
 */
static void tm1637_stop(void) {
    TM_PORT &= ~(1 << TM_CLK); //CLK LOW
    _delay_us(2);
    TM_PORT &= ~(1 << TM_DIO); //DIO LOW
    _delay_us(2);
    TM_PORT |= (1 << TM_CLK); //CLK HIGH
    _delay_us(2);
    TM_PORT |= (1 << TM_DIO); //DIO HIGH
}

void tm1637_write_byte(uint8_t data) {
    // Send 8 bits (LSB first)
    for (uint8_t i = 0; i < 8; i++) {
		//CLK LOW, Set DIO
        TM_PORT &= ~(1 << TM_CLK);
        if (data & 0x01) {
            TM_PORT |= (1 << TM_DIO);
        } else {
            TM_PORT &= ~(1 << TM_DIO);
        }
        _delay_us(2);
        TM_PORT |= (1 << TM_CLK); //CLK HIGH
        _delay_us(2);
        data >>= 1;
    }

    //ACK DETECTION ON DIO (PC1)
    TM_PORT &= ~(1 << TM_CLK);  //CLK LOW
    TM_DDR &= ~(1 << TM_DIO);   //Set DIO (PC1) as Input
    TM_PORT &= ~(1 << TM_DIO);  //DIO output LOW - Releases DIO Line
    _delay_us(2);

    TM_PORT |= (1 << TM_CLK);  //CLK HIGH
    _delay_us(2);
    
    //The ACK on DIO pin not read (asssumed)
    
    TM_PORT &= ~(1 << TM_CLK); //CLK LOW
    TM_DDR |= (1 << TM_DIO);   //Set DIO (PC1) to Output
    _delay_us(2);
}

void tm1637_init(void) {
    TM_DDR |= (1 << TM_DIO) | (1 << TM_CLK); //Set PC1 and PC2 as Outputs
    TM_PORT |= (1 << TM_DIO) | (1 << TM_CLK); //DIO output HIGH
    
    //Turn on display, max brightness
    tm1637_start();
    tm1637_write_byte(0x8F); 
    tm1637_stop();
}

void tm1637_show_number(uint16_t num) {
    uint8_t digits[4];
    digits[0] = segment_map[(num / 1000) % 10];
    digits[1] = segment_map[(num / 100) % 10];
    digits[2] = segment_map[(num / 10) % 10];
    digits[3] = segment_map[num % 10];

    //Write Data Command (Auto-increment)
    tm1637_start();
    tm1637_write_byte(0x40);
    tm1637_stop();

    //Set Address Command (Start at C0)
    tm1637_start();
    tm1637_write_byte(0xC0);
    for (uint8_t i = 0; i < 4; i++) {
        tm1637_write_byte(digits[i]);
    }
    tm1637_stop();

	//uart_debug_binary16("tm1637_show_number:", 0x00);

	range_finder_get(); //Starts the next USF distance mesurement, aftter display finished
}