/*
 * \file main.c
 *
 * \brief Main source file.
 *
 *  (c) 2020 Microchip Technology Inc. and its subsidiaries.
 *
 *  Subject to your compliance with these terms,you may use this software and
 *  any derivatives exclusively with Microchip products.It is your responsibility
 *  to comply with third party license terms applicable to your use of third party
 *  software (including open source software) that may accompany Microchip software.
 *
 *  THIS SOFTWARE IS SUPPLIED BY MICROCHIP "AS IS". NO WARRANTIES, WHETHER
 *  EXPRESS, IMPLIED OR STATUTORY, APPLY TO THIS SOFTWARE, INCLUDING ANY IMPLIED
 *  WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY, AND FITNESS FOR A
 *  PARTICULAR PURPOSE.
 *
 *  IN NO EVENT WILL MICROCHIP BE LIABLE FOR ANY INDIRECT, SPECIAL, PUNITIVE,
 *  INCIDENTAL OR CONSEQUENTIAL LOSS, DAMAGE, COST OR EXPENSE OF ANY KIND
 *  WHATSOEVER RELATED TO THE SOFTWARE, HOWEVER CAUSED, EVEN IF MICROCHIP HAS
 *  BEEN ADVISED OF THE POSSIBILITY OR THE DAMAGES ARE FORESEEABLE. TO THE
 *  FULLEST EXTENT ALLOWED BY LAW, MICROCHIP'S TOTAL LIABILITY ON ALL CLAIMS IN
 *  ANY WAY RELATED TO THIS SOFTWARE WILL NOT EXCEED THE AMOUNT OF FEES, IF ANY,
 *  THAT YOU HAVE PAID DIRECTLY TO MICROCHIP FOR THIS SOFTWARE.
 */

#include <avr/io.h>
#include <avr/cpufunc.h>
#include <util/delay.h>
#include "adc.h"
#include "dac.h"
#include "rtc.h"
#include "uart.h"

/*Configure system fuses ensuring known start condition for MCU*/

FUSES = {
    .WDTCFG = FUSE_WDTCFG_DEFAULT,
    .BODCFG = FUSE_BODCFG_DEFAULT,
    .OSCCFG = FUSE_OSCCFG_DEFAULT,
    .SYSCFG0 = FUSE_SYSCFG0_DEFAULT,
    .SYSCFG1 = (1 << FUSE_MVSYSCFG0_bp) | SUT_0MS_gc,
    .CODESIZE = FUSE_CODESIZE_DEFAULT,
    .BOOTSIZE = FUSE_BOOTSIZE_DEFAULT
};

// To compile the lockbits configuration, the LOCKBITS macro is used. The lockbits are set to unlocked.
LOCKBITS = {
    LOCKBITS_DEFAULT,
};

void GPIO_init(void);
void OPAMP_init(void);

int main(void)
{
    /* Set Main Clock to 24 MHz */
    ccp_write_io((void*)&CLKCTRL.OSCHFCTRLA, CLKCTRL_FREQSEL_24M_gc);

    GPIO_init();

    LED0_init();

    RTC_init();     /*1s interrupt timer*/

    DAC0_init();    /*Creates and outputs the sine waveform*/

    USART_init();   /*Streams data into Data Visualizer*/

    ADC0_init();    /*Acquire Signal from OPAMP0 output*/

    /*Start the timer controlling the sine waveform generator 
    and the timer controlling the acquisition */
    DAC0_SineEaveTimer_enable();
    ADC0_SampleTimer_enable();

    OPAMP_init();

    RTC_enable();   /* Start blinking LED, indicating that */
                    /* initialization is complete and successful */

    sei();

    while (1) 
    {
    }
}

#define OPAMP_TIMEBASE_US     (ceil(F_CPU /1e6)-1)

void OPAMP_init ()
{
    /* Configure the Timebase */
    OPAMP.TIMEBASE = OPAMP_TIMEBASE_US;

    /* Configure the OP0 */
    OPAMP.OP0CTRLA = OPAMP_OP0CTRLA_OUTMODE_NORMAL_gc | OPAMP_ALWAYSON_bm;
    OPAMP.OP0INMUX = OPAMP_OP0INMUX_MUXNEG_OUT_gc | OPAMP_OP0INMUX_MUXPOS_VDDDIV2_gc;
    OPAMP.OP0RESMUX =  OPAMP_OP0RESMUX_MUXBOT_OFF_gc | OPAMP_OP0RESMUX_MUXWIP_WIP0_gc | OPAMP_OP0RESMUX_MUXTOP_OFF_gc;
    OPAMP.OP0SETTLE = 0x7F;

    /* Configure the OP1*/
    OPAMP.OP1CTRLA = OPAMP_OP1CTRLA_OUTMODE_NORMAL_gc | OPAMP_ALWAYSON_bm;
    OPAMP.OP1INMUX = OPAMP_OP1INMUX_MUXNEG_WIP_gc | OPAMP_OP1INMUX_MUXPOS_DAC_gc;
    OPAMP.OP1RESMUX = OPAMP_OP1RESMUX_MUXBOT_LINKOUT_gc | OPAMP_OP1RESMUX_MUXWIP_WIP5_gc | OPAMP_OP1RESMUX_MUXTOP_OUT_gc;
    OPAMP.OP1SETTLE = 0x7F;

    /* Enable OPAMP peripheral */
    OPAMP.CTRLA = OPAMP_ENABLE_bm;

    /* Wait for the operational amplifiers to settle */
    while (!(OPAMP.OP0STATUS & OPAMP_SETTLED_bm) & !(OPAMP.OP1STATUS & OPAMP_SETTLED_bm))
    {
        ;
    }
}

void GPIO_init(void){
    /* Disable digital input buffer*/
    /*OP0*/
    PORTD.PIN2CTRL = PORT_ISC_INPUT_DISABLE_gc;
    /*OP1*/
    PORTD.PIN5CTRL = PORT_ISC_INPUT_DISABLE_gc;
    /*DACOUT*/
    PORTD.PIN6CTRL = PORT_ISC_INPUT_DISABLE_gc; 
}