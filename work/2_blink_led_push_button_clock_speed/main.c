/*
 * main.c
 *
 *      Created on: Sep 5, 2025
 *      Author: Everton Marcelino Junior
 *      Instituto Federal de Santa Catarina
 *
 *      This application uses the MSP430FR2355 Launchpad.
 *      Its purpose is to apply concepts related to GPIO,
 *      using an LED and a push button, as well as how
 *      clock speed affects program execution.
 *
 *      The flow of the program is as follows:
 *
 *      1. Wait for the push button 1 to be pressed.
 *      2. Blink the red LED 3 times.
 *      3. Wait for the push button 1 to be pressed.
 *      4. Blink the red LED 7 times.
 *      5. Go back to step 1.
 *
 *      Also, the clock speed should be changed to see how
 *      it affects the delays between blinks of the LED.
 *
 *      To change the clock speed, I implemented two options:
 *      - 8MHz clock
 *      - 24MHz clock - default
 *      To cycle between the two, you should press the push button 2.
 *      To confirm the clock change, the green LED should blink once.
 *
 *      This is the pinout used:
 *
 *      P1.0| --> Red LED
 *      P6.6| --> Green LED
 *      P2.3| <-- Push Button 1
 *      P4.1| <-- Push Button 2
 *
 */
#include <msp430.h>

#include "lib/bits.h"

#define LED         BIT0
#define LED1        BIT6
#define BUTTON      BIT3
#define BUTTON1     BIT1
#define DELAY       5000000

/**
  * @brief  Configura sistema de clock para usar o Digitally Controlled Oscillator (DCO)
  *         Essa configuração utiliza pinos para cristal externo.
  * @param
  *         int mode - set to 0 for 8MHz clock; set to 1 for 24MHz clock
  *
  * @retval none
  */
void set_clock_system(int mode) {

    // Configure two FRAM wait state as required by the device data sheet for MCLK
    // operation at 24MHz(beyond 8MHz) _before_ configuring the clock system.
    FRCTL0 = FRCTLPW | NWAITS_2;

    P2SEL1 |= BIT6 | BIT7;                       // P2.6~P2.7: crystal pins
    do
    {
        CSCTL7 &= ~(XT1OFFG | DCOFFG);           // Clear XT1 and DCO fault flag
        SFRIFG1 &= ~OFIFG;
    } while (SFRIFG1 & OFIFG);                   // Test oscillator fault flag

    __bis_SR_register(SCG0);                     // disable FLL
    CSCTL3 |= SELREF__XT1CLK;                    // Set XT1 as FLL reference source
    CSCTL0 = 0;                                  // clear DCO and MOD registers

    /* Selecionar DCORSEL_xyz de acordo com a máxima frequência da CPU */
    CSCTL1 = mode ? DCORSEL_7 : DCORSEL_3;       // Set DCO = 8MHz if mode = 0 else DCO = 24MHz

    /* Ajustar o multiplicador (ex. 731) para a frequência desejada *
     *
     * F = 32768*731 / 1  ---->   (32768 * n) / FLLD_x
     */
    CSCTL2 = FLLD_0 + (mode ? 731 : 244);                       // DCOCLKDIV = 32768*731 / 1
    __delay_cycles(3);
    __bic_SR_register(SCG0);                     // enable FLL
    while(CSCTL7 & (FLLUNLOCK0 | FLLUNLOCK1));   // FLL locked

    /* CSCTL4 = SELMS__DCOCLKDIV | SELA__REFOCLK;
     * set XT1 (~32768Hz) as ACLK source, ACLK = 32768Hz
     * default DCOCLKDIV as MCLK and SMCLK source
     - Selects the ACLK source.
     * 00b = XT1CLK with divider (must be no more than 40 kHz)
     * 01b = REFO (internal 32-kHz clock source)
     * 10b = VLO (internal 10-kHz clock source) (1)   */
    CSCTL4 = SELMS__DCOCLKDIV | SELA__REFOCLK;
}

void configure_pins(void) {
    P1DIR |= LED;
    CLR_BIT(P1OUT, LED); // Turns LED off

    P6DIR |= LED1;
    CLR_BIT(P6OUT, LED1); // Turns LED off

    P2REN |= BUTTON; // Enables internal resistor on button pin
    P2OUT |= BUTTON; // Sets button pin internal resistor in pull-up config

    P4REN |= BUTTON1; // Enables internal resistor on button1 pin
    P4OUT |= BUTTON1; // Sets button1 pin internal resistor in pull-up config
}

void blink_led(int times_to_blink) {
    int i;
    for(i = times_to_blink; i > 0; i--){
        P1OUT = P1OUT ^ LED; // Toggles the LED
        __delay_cycles(DELAY);
        P1OUT = P1OUT ^ LED; // Toggles the LED
        __delay_cycles(DELAY);
    }
}

void signal_clock_change_success(void){
    P6OUT = P6OUT ^ LED1; // Toggles the LED
    __delay_cycles(DELAY);
    P6OUT = P6OUT ^ LED1; // Toggles the LED
}

void main(void) {
    WDTCTL = WDTPW | WDTHOLD; // Disables watchdog

    PM5CTL0 &= ~LOCKLPM5; // Disables the GPIO power-on default high-impedance mode

    int clock_mode = 1;
    int led_blinks = 3;

    set_clock_system(clock_mode);
    configure_pins();

    while(1){
        if(!TST_BIT(P4IN, BUTTON1)){ // checks if BUTTON1 was pressed
            clock_mode = !clock_mode; // changes clock mode variable
            set_clock_system(clock_mode);
            signal_clock_change_success();
            __delay_cycles(DELAY/2); // debounces button
        }

        if(!TST_BIT(P2IN, BUTTON)){ // checks if BUTTON was pressed
            blink_led(led_blinks);
            led_blinks = led_blinks == 3 ? 7 : 3;
        }
    }
}
