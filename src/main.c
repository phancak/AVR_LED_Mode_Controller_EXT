#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include "TM1637.h"
#include "utils.h"

#ifndef F_CPU
#define F_CPU 8000000UL
#endif

// -----------------------------------------------------------------------------
// Candidate Assignment: AVR LED Mode Controller
// -----------------------------------------------------------------------------
// Objective
// Implement a small AVR C program that changes LED blinking behavior based on a
// switch input. Use the AVR GCC toolchain to build and Wokwi VSCode Extension
// to simulate the result.
//
// What is Wokwi?
// - Wokwi is a hardware simulator that lets you run embedded firmware without
//   a physical board.
// - Example: https://wokwi.com/projects/461302817329780737
// - In this assignment, you will compile your AVR program, then load the
//   generated .elf or .hex into Wokwi to verify LED behavior with the switch.
// - This means your submission can be validated consistently even without
//   real hardware.
//
// Target Platform
// - MCU: ATmega328P
// - Language: C
// - Toolchain: AVR GCC
// - Simulator: Wokwi VSCode Extension
//
// Functional Requirements
// 1. Switch input controls blinking mode.
// 2. If the switch is LEFT: blink RED LED every 500 ms.
// 3. If the switch is RIGHT: blink GREEN and BLUE LEDs every 500 ms.
// 4. Use an interrupt to detect switch changes and update the active mode.
//
// Note:
// 1. Reference build artifacts .elf and .hex are provided.
// 2. Complete main.c and simulate by editing wokwi.toml to point to your generated .elf or .hex file.
//
// Pin Mapping
// - Switch: pin 5
// - RED LED: pin 13
// - GREEN LED: pin 12
// - BLUE LED: pin 11
//
//
// Minimum Deliverables
// 1. Git repository with working implementation in this file.
//    - repo should include a README with build instructions and any notes.
//    - diagram.json should be included for Wokwi simulation.
//    - .elf or .hex file generated from your build process.
//    - source file used to build the .elf or .hex.
// 2. Behavior aligned with reference implementation according to requirements.
// 3. Build notes at the top of the file, including compiler flags and commands.
// 4. Testability support with test-build and release-build configurations.
// 5. Comments for any assumptions or tradeoffs.
//
// Build and Simulation (high level)
// 1. Build with AVR GCC and generate .elf or .hex.
// 2. Update wokwi.toml to point to your generated artifact.
// 3. Open the Wokwi diagram in VSCode and start the simulation.
// 4. Toggle the virtual switch and verify both required blinking modes.
//
// Evaluation Notes
// - The checklist above is a baseline.
// - You may add improvements or optimizations as long as core behavior remains
//   correct and requirements are met.
// - comment any assumptions and tradeoffs in your implementation
// - testability and maintainability will be considered in evaluation
//
// Helpful Resources
// - AVR GCC toolchain:
//   https://www.microchip.com/en-us/tools-resources/develop/microchip-studio/gcc-compilers
// - Wokwi VSCode getting started:
//   https://docs.wokwi.com/vscode/getting-started
// - ATmega328P quick reference:
//   https://github.com/amirbawab/AVR-cheat-sheet

//Project Global Variables
uint16_t timer1toggleTime = 62500; //LED on and off period based on 16Mhz clock with 256 prescaler
uint16_t readVariable = 0; //LED on and off period
uint8_t USF_timer0_echo_measure_prescalar = 0x40; //The timer0 prescalar used to mesure the time between the initial pulse and the echo 
uint8_t USF_hold_period = 0xFF; //Period between meassurements: we suggest to use over 60ms measurement cycle, in order to prevent trigger signal to the echo signal.
uint8_t USF_init_pulse_period = 100; //Timer 0 period, for USF, 10uS pulse to the trigger input to start the ranging
uint8_t USF_process_step = 0x00; //Ultrasonic range finder get phase (initial pulse or response count)
uint16_t USF_distance = 0x00; //Calculated distance

/**
 * @brief Reads the Output compare register TCNT1 SAFELY
 *        To avoid modification by interrupt
 * @param i target variable to be read from
 * @modifies Disables global interrupt for the time 
 */
void TIM16_ReadTCNT1(uint16_t *i)
{
    unsigned char sreg;
    /* Save global interrupt flag */
    sreg = SREG;
    cli(); // Disable interrupts
    /* Read TCNT1 into i */
    *i = TCNT1; //Reads the full 16 bit
    /* Restore global interrupt flag */
    SREG = sreg;
}

/**
 * @brief Writes the Output compare register TCNT1 SAFELY
 *        To avoid modification by interrupt
 * @param i target variable to be written from
 * @modifies Disables global interrupt for the time 
 */
void TIM16_WriteTCNT1(uint16_t *i)
{
    unsigned char sreg;
    /* Save global interrupt flag */
    sreg = SREG;cli(); // Disable interrupts
    
    /* Set TCNT1 to i */
    TCNT1 = *i;
    /* Restore global interrupt flag */
    SREG = sreg;
}

/**
 * @brief Reads the Output compare register OCR1A SAFELY
 *        To avoid modification by interrupt
 * @param i target variable to be read to
 * @modifies Disables global interrupt for the time 
 */
void TIM16_ReadOCR1A(uint16_t *i)
{
    unsigned char sreg;
    /* Save global interrupt flag */
    sreg = SREG;
    cli(); // Disable interrupts
    /* Read TCNT1 into i */
    *i = OCR1A; //Reads the full 16 bit
    /* Restore global interrupt flag */
    SREG = sreg;
}

/**
 * @brief Writes the Output compare register OCR1A SAFELY
 *        To avoid modification by interrupt
 * @param i target variable to be written from
 * @modifies Disables global interrupt for the time 
 */
void TIM16_WriteOCR1A(uint16_t *i)
{
    unsigned char sreg;
    /* Save global interrupt flag */
    sreg = SREG;
    cli(); // Disable interrupts
    /* Set TCNT1 to i */
    OCR1A = *i;
    /* Restore global interrupt flag */
    SREG = sreg;
}

/**
 * @brief Writes the Output compare register OCR1B SAFELY
 *        To avoid modification by interrupt
 * @param i target variable to be written from
 * @modifies Disables global interrupt for the time 
 */
void TIM16_WriteOCR1B(uint16_t *i)
{
    unsigned char sreg;
    /* Save global interrupt flag */
    sreg = SREG;
    cli(); // Disable interrupts
    /* Set TCNT1 to i */
    OCR1B = *i;
    /* Restore global interrupt flag */
    SREG = sreg;
}

/**
 * @brief Writes the Output compare register OCR1A SAFELY
 *        To avoid modification by interrupt
 * @param i target variable to be written from
 * @modifies Disables global interrupt for the time 
 */
void TIM16_WriteOCR0A(uint16_t *i)
{
    unsigned char sreg;
    /* Save global interrupt flag */
    sreg = SREG;
    cli(); // Disable interrupts
    /* Set TCNT1 to i */
    OCR0A = *i;
    /* Restore global interrupt flag */
    SREG = sreg;
}

/**
 * @brief Writes the Output compare register OCR1B SAFELY
 *        To avoid modification by interrupt
 * @param i target variable to be written from
 * @modifies Disables global interrupt for the time 
 */
void TIM16_WriteOCR0B(uint16_t *i)
{
    unsigned char sreg;
    /* Save global interrupt flag */
    sreg = SREG;
    cli(); // Disable interrupts
    /* Set TCNT1 to i */
    OCR0B = *i;
    /* Restore global interrupt flag */
    SREG = sreg;
}

/**
 * @brief Initializes the system clock with a 256 prescalar
 *        System clock: F_CPU/256
 * @modifies CLKPR Clock Prescale Register
 */
void system_clock_init(){
    //The CLKPCE bit must be written to logic one to enable change of the CLKPS bits. The CLKPCE bit is only
    //updated when the other bits in CLKPR are simultaneously written to zero
    CLKPR = (1 << CLKPCE); //Eanbles Makes CLKPR write (other bits in CLKPR zero)
    CLKPR = (uint8_t)0x00; //1 Clock Division Factor and CLKPCE set LOW, and must set CLKPCE LOW
}

void set_PB1_output(){
    DDRB |= (1 << DDB1); // 1. Set PB1 (OC1A) as output
}

void set_PB2_output(){
    DDRB |= (1 << DDB2); // 1. Set PB1 (OC1B) as output
}

/**
 * @brief Initializes registers for the ultrasonic range finder operation
 * @modifies TCCR0A, TCCR0B, OCR0A, OCR0B
 */
void range_finder_init(){
    DDRD |= (1 << DDD5); //Set PD5 (OC0B) as output, used to play sound
    DDRD |= (1 << DDD6); //Set PD6 (OC0A) as output, connected to the detector
    PORTD &= ~(1 << PORTD6); //Set PD6 (OC0A) output to LOW 

    //DEBUG
    uart_print("range_finder_init\r\n");
    uart_debug_binary8("DDRD", DDRD);
    uart_debug_binary8("PORTD", PORTD);
}

/**
 * @brief Requests response from the ultrasonic range finder
 *        A pulse of a certain length is sent to the range finder data
 *        pin and timer is started and runs until a response pin from the finder
 *        The time until the response is used to calculate the distance
 *        USF Measurement Steps:
 *        1. Set ouput LOW on the USF data pin, set timer to wait for a hold period, set ISR to run at the end of the set-up period
 * @modifies 
 */
void range_finder_get(){
    //The PD6 should be LOW now but it will be set low again
    DDRD |= (1 << DDD6); //Set PD6 (OC0A) as output, connected to the detector
    PORTD &= ~(1 << PORTD6); //Set PD6 (OC0A) output to LOW 

    TCNT0 = (uint8_t)0x00; //Timer/Counter Register - reset
    //Safe write here not needed - also to maintain core functionality
    OCR0A = USF_hold_period; //Sets the Output compare value OCR0A, pulse length sent to the range finder
    TIMSK0 |= (1 << OCIE0A); //Timer/Counter Interrupt Mask Register, OCIE0A: Timer/Counter0 Output Compare Match A Interrupt Enable

    USF_process_step = 0x01; //USF get process is in phase one - initial pulse

    sei(); //Global Interrupt Enable

    //Timer starts when we set clock source - 0b000 No clock source (Timer/Counter stopped).
    TCCR0B &= ~(0x7 << CS00); //Reset clock bits in register
    TCCR0B |= (0x5 << CS00); //Clock Select: clkI/O/1024 (From prescaler)
}

/**
 * @brief Initializes the timer 0 peripheral
 * @modifies TCCR0A, TCCR0B, OCR0A, OCR0B
 */
void timer0_init() {
    TCCR0A |= (1 << COM0A1); //Compare Match Output A Mode, Clear OC0A on Compare Match
    TCCR0A &= ~(1 << COM0A0); //Compare Match Output A Mode
    TCCR0A &= ~(1 << COM0B1); //Compare Match Output A Mode, Normal port operation, OC0B disconnected.
    TCCR0A &= ~(1 << COM0B0); //Compare Match Output A Mode

    TCCR0A &= ~(1 << WGM10); //Waveform Generation Mode Bit, CTC
    TCCR0A |= (1 << WGM11); //Waveform Generation Mode Bit

    //TCCR1B &= ~(1 << ICNC1); //Disable Input Capture Noise Canceler

    TCCR0B &= ~(1 << WGM02); //Waveform Generation Mode: Set Timer 0 to CTC mode
}

/**
 * @brief Initializes the timer 1 peripheral
 * @modifies TCCR1A, TCCR1B, OCR1A, OCR1B
 */
void timer1_init() {
    TCCR1A &= ~(1 << WGM10); //Waveform Generation Mode Bit
    TCCR1A &= ~(1 << WGM11); //Waveform Generation Mode Bit

    TCCR1B &= ~(1 << ICNC1); //Disable Input Capture Noise Canceler
    TCCR1B &= ~(1 << WGM13); //Waveform Generation Mode: Set Timer 1 to CTC mode (WGM12 = 1)
    TCCR1B |= (1 << WGM12); //Waveform Generation Mode

    //Set the same toggle value for both pins
    TIM16_WriteOCR1A(&timer1toggleTime); //Sets the Ouput compare value OCR1A
    //TIM16_WriteOCR1B(&timer1toggleTime); //Sets the Ouput compare value OCR1B

    //Timer starts when we set clock source - 0b000 No clock source (Timer/Counter stopped).
    TCCR1B &= ~(0x7 << CS10); //Clock Select: Reset bits
    TCCR1B |= (0x3 << CS10); //Clock Select: clkI/O/64 (From prescale
}

/**
 * @brief Initializes the interrupt connected to the toggle button
 * @modifies PRR, SMCR
 */
void PC0_interrupt_init() {
    DDRC &= ~(1 << DDC0); //PD5 as input
    PORTC |= (1 << PORTC0); //Enable internal pull-up

    //PC0 is PCINT8
    PCICR |= (1 << PCIE1); //PCINT[14:8] Pin Change Interrupt Control Register, Enable the PCIE2 group
    PCMSK1 |= (1 << PCINT8); //Pin Change Mask Register 2, mask for pin PD5 (PCINT21)

    //Global Interrupt Enable
    sei();
}

// ---- ISR Code ----
/**
 * @brief ISR Timer0 dedicated to the interaction with the ultrasonic range detector
 * @modifies TCNT0, DDRD, 
 */
ISR(TIMER0_COMPA_vect) {
    //Global interrupt disabled automatically for the duration of ISR
    sei(); //Must re-enable global interrupts to allow for the LED blinking nested ISR (high priority in the project)

    if(USF_process_step == 0x01){
        //The USF range finder finished the initial hold phase - waiting period between measurements
        //Timer starts when we set clock source - 0b000 No clock source (Timer/Counter stopped).
        TIMSK0 &= ~(1 << OCIE0A); //Disable ISR: Timer/Counter Interrupt Mask Register, OCIE0A: Timer/Counter0 Output Compare Match A Interrupt Enable
        TCCR0B &= ~(0x07 << CS00); //STOP timer0, clock Select: No clock source (Timer/Counter stopped)
        TCNT0 = (uint8_t)0x00; //Timer/Counter Register - reset the timer count
        //uart_debug_binary8("step == 0x01: TCNT0", TCNT0);
        //DDRD |= (1 << DDD6); //Set PD6 (OC0A) as output, connected to the detector
        PORTD|= (1 << PORTD6); //Set PD6 (OC0A) output to HIGH 

        //uart_print("finished the initial hold phase\r\n");
        //uart_debug_binary8("TCCR0B", TCCR0B);

        OCR0A = (USF_init_pulse_period); //Sets the Ouput compare value OCR0A, pulse length sent to the range finder

        TIMSK0 |= (1 << OCIE0A); //Enable ISR: Timer/Counter Interrupt Mask Register, OCIE0A: Timer/Counter0 Output Compare Match A Interrupt Enable

        USF_process_step = 0x02; //Setup for the next step

        //Start timer: Timer starts when we set clock source - 0b000 No clock source (Timer/Counter stopped).
        TCCR0B &= ~(0x7 << CS00); //Reset clock bits in register
        TCCR0B |= (1 << CS00); //Start Timer: Clock Select: clkI/O/(No prescaling)

        //The initial pulse sent to the USF is started
    } else if(USF_process_step == 0x02){
        //The USF range finder is at the end of the initial pulse phase - next it will be measuring time for the echo return 
        TIMSK0 &= ~(1 << OCIE0A); //Disable ISR: Timer/Counter Interrupt Mask Register, OCIE0A: Timer/Counter0 Output Compare Match A Interrupt Enable
        TCCR0B &= ~(0x07); //STOP timer0, clock Select: No clock source (Timer/Counter stopped)
        TCNT0 = (uint8_t)0x00; //Timer/Counter Register - reset the timer count

        //uart_print("end of the initial pulse phasee\r\n");
        USF_process_step = 0x03; //Setup for the next step
        
        PORTD &= ~(1 << PORTD6); //Set PD6 (OC0A) output to LOW
        DDRD &= ~(1 << DDD6); //Set PD6 (OC0A) as input, waiting for the response pulse
        
        //Pin Input Interrupt: (PCINT22/OC0A/AIN0) PD6, (PCINT21/OC0B/T1) PD5
        PCICR |= (1 << PCIE2); //PCINT[23:16] Pin Change Interrupt Control Register, Enable the PCIE1 group
        PCMSK2 |= (1 << PCINT22); //Pin Change Mask Register 2, mask for pin PD5 (PCINT22)

        OCR0A = (uint8_t)0xFF;

        TIFR0 |= (1 << TOV0); //TOV0: Timer/Counter0 Overflow Flag, Clear overflow flags
        //Start timer: Timer starts when we set clock source - 0b000 No clock source (Timer/Counter stopped).
        TCCR0B &= ~(0x7 << CS00); //Reset clock bits in register
        TCCR0B |= (0x5 << CS00); //Clock Select: clkI/O/1024 (From prescaler)
    }
}

//PC0 is PCINT8
/**
 * @brief ISR LED toggle button: dedicated to the interaction with the button that toggles the LEDs
 */
ISR(PCINT1_vect)
{
    //Global interrupt disabled automatically

    if (PINC & (1 << PINC0)) {
        //Toggle switch is LEFT HIGH
        TCCR1A |=  (1 << COM1A0); //Activate OC1A toggle
        TCCR1A &= ~(1 << COM1B0); //Deactivate OC1B toggle
    } else if(!(PIND & (1 << PIND5))){
        //Toggle switch is RIGHT LOW
        TCCR1A &= ~(1 << COM1A0); //Deactivate OC1A toggle
        TCCR1A |=  (1 << COM1B0); //Activate OC1B toggle
    }

    //Global interrupt enabled automatically
}

/**
 * @brief ISR PCINT2 dedicated to the interaction with the ultrasonic range detector
 *        Is triggered by the change on PD6 pin
 * @modifies PCMSK2, TCCR0B, USF_distance
 */
ISR(PCINT2_vect)
{   
    //Global interrupt disabled automatically for the duration of ISR
    sei(); //Must re-enable global interrupts to allow for the LED blinking nested ISR (high priority in the project)
    
    if(USF_process_step == 0x03){
        //Up tick on the sig pin is detected, coounter will start with ISR for tick down
        TCNT0 = (uint8_t)0x00; //Timer/Counter Register - reset the timer count
        USF_process_step = 0x04;
    } else if(USF_process_step == 0x04){
        //A down tick was detected on the SIG pin, stop timer, record timer count

        //Stop Timer: Timer starts when we set clock source - 0b000 No clock source (Timer/Counter stopped).
        TCCR0B &= ~(0x07); //Stop Timer: Clock Select: No clock source (Timer/Counter stopped)

        //The USF range finder is in the response measuring phase
        PCMSK2 &= ~(1 << PCINT22); //Disable the interrupt - If PCINT[23:16] is cleared, pin change interrupt on the corresponding I/O pin is disabled
        PCIFR |= (1 << PCIF2); //Cancels any potential ISR triggers thast occured before the ISR is disabled
        
        //uart_print("end of the measuring phase: PCINT2_vect\r\n");
        
        //Check for overflow flags
        if(0){//TIFR0 & (1 << TOV0)){
            //Overflow flag detected
            USF_distance = 0x0000; //No distance calculated
            //uart_debug_binary16("Timer0 overflow:", USF_distance); //UART transmit
        } else {
            //No Overflow flag detected
            //Convert Speed: 343 m/s=0.0343 cm/μs (centimeters per microsecond), 8Mhz 1024Prescalar, 8us per tick, 2.19 cm per tick
            USF_distance = (TCNT0 * 22)/10; //*USF_timer0_echo_measure_prescalar/F_CPU*340/2; //Calculates the measured range, range = high level time * velocity (340M/S) / 2
            tm1637_show_number((uint16_t)USF_distance); //Sends the distance to 4 digit display
            //After calculation a sound should be made
            //uart_debug_binary16("Range:", USF_distance); //UART transmit

        }
        TIFR0 |= (1 << TOV0); //TOV0: Timer/Counter0 Overflow Flag, Clear overflow flags
    }    
    //range_finder_get(); //Start the next distance measurement
}

/**
 * @brief Finds out which way the switch is innitally and starts to toggle
 *        the appropriate colors
 * @modifies TCCR1A Clock Prescale Register
 */
void color_toggle_init(){
    unsigned char sreg;
    /* Save global interrupt flag */
    sreg = SREG;
    cli(); // Disable interrupts

    //Find out which way the toggle swicth is initally and start LED toggle
    if (PINC & (1 << PINC0)) {
        //Toggle switch is LEFT HIGH, start RED LED toggle
        TCCR1A |=  (1 << COM1A0); //Activate OC1A toggle
        TCCR1A &= ~(1 << COM1B0); //Deactivate OC1B toggle
    } else {
        //Toggle switch is RIGHT LOW, start GREEN and BLUE LED toggle
        TCCR1A &= ~(1 << COM1A0); //Deactivate OC1A toggle
        TCCR1A |=  (1 << COM1B0); //Activate OC1B toggle
    }

    /* Restore global interrupt flag */
    SREG = sreg;
}

/**
 * @brief Enables Sleep mode and turns off all other peripherals
 * @modifies PRR, SMCR
 */
void enter_power_save_mode(){
    SMCR = (1 << SE); //Sleep Mode Control Register SE: Sleep Enable
    //Shut off all peripherals except TIMER1
    PRR = (1 << PRTWI) | (1 << PRTIM2) | (1 << PRTIM0) | (1 << PRADC) | (1 << PRUSART0) | (1 << PRSPI);
}

int8_t digits[4] = {0}; //Numbers to be displayed

int main(void) {
    //Initialize data
	digits[0] = 0;
	digits[1] = 1;
	digits[2] = 2;
	digits[3] = 4;

    //UART Port Init
    uart_init(9600); // Initialize UART at 9600 baud, DEBUG
    //uart_print("UART Initialized\r\n");

    //tm1637_pin_init();

    //Send data to display
	//tm1637_write_data_auto_increment(digits);

    tm1637_init();
    //tm1637_show_number((uint16_t)1234);

    //System Initialization 
    set_PB1_output(); //Initializes pins
    set_PB2_output(); //Initializes pins
    system_clock_init(); //Initialize systme clock 
    timer1_init(); //Initializes and starts the timers
    color_toggle_init(); //Sets up initial LED toggle
    PC0_interrupt_init(); //Initializze interrupt on PD5
    //range_finder_init(); //Sets up the ultrasonic range finder

    range_finder_get();
    
    while (1) {
        //......
    }
    return 0;
}