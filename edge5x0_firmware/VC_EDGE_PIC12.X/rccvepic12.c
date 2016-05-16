/*
 * File:   rccvepic12.c
 * Author: MFC
 *
 * Created on May 14, 2014, 12:10 PM
 *  Revised on April 26, 2016 to add additional shutdown input, logic, and config changes (GPB)
 */
#include <htc.h>

// CONFIG1
#pragma config FOSC = INTOSC    /* Oscillator Selection Bits (INTOSC oscillator: I/O function on CLKIN pin) */
#pragma config WDTE = OFF       /* Watchdog Timer Enable (WDT disabled) */
#pragma config PWRTE = OFF      /* Power-up Timer Enable (PWRT disabled) */
#pragma config MCLRE = OFF      /* MCLR Pin Function Select (MCLR/Vpp pin function is digital input. MCLR internally disabled) */
#pragma config CP = OFF         /* Flash Program Memory Code Protection (Program memory code protection is disabled) */
#pragma config BOREN = ON       /* Brown-out Reset Enable (Brown-out Reset enabled) */
#pragma config CLKOUTEN = OFF   /* Clock Out Enable (CLKOUT function is disabled. I/O or oscillator function on the CLKOUT pin) */

// CONFIG2
#pragma config WRT = OFF        /* Flash Memory Self-Write Protection (Write protection off) */
#pragma config STVREN = ON      /* Stack Overflow/Underflow Reset Enable (Stack Overflow or Underflow will cause a Reset) */
#pragma config BORV = LO        /* Brown-out Reset Voltage Selection (Brown-out Reset Voltage (Vbor), low trip point selected.) */
// #pragma config LPBOR = OFF   /* Low-Power Brown Out Reset (Low-Power BOR is disabled) */
#pragma config LVP = OFF        /* Low-Voltage Programming Enable (High voltage on MCLR/Vpp must be used for programming) */

#define _XTAL_FREQ 500000       /* used by the HI-TECH delay_ms(x) macro */
#define PMU_SLP_S45_TIMEOUT 10  /* Timeout in seconds waiting for SoC to reassert. */
#define CAT_ERR_TIMEOUT     180 /* 3 min * 60 sec = 180 sec ticks */


#define _GPIO_CPU_SUS2 PORTAbits.RA0            /* Input: RA0 */
#define _GPIO_CPU_SUS2_DIR TRISAbits.TRISA0
#define _PWRGD_COREPWR PORTAbits.RA1            /* Input: RA1 */
#define _PWRGD_COREPWR_DIR TRISAbits.TRISA1
#define _PIC_SLP_N PORTAbits.RA2                /* Input: RA2 */
#define _PIC_SLP_N_DIR TRISAbits.TRISA2
#define _SYS_SHDN_N PORTAbits.RA3               /* Input: RA3 */
#define _SYS_SHDN_N_DIR TRISAbits.TRISA3
#define _ATX_PS_ON LATAbits.LATA4               /* Output: RA4 */
#define _ATX_PS_ON_DIR TRISAbits.TRISA4
#define _PIC_PWR_BTN PORTAbits.RA5              /* Input: RA5 */
#define _PIC_PWR_BTN_DIR TRISAbits.TRISA5
#define _PIC_PWR_BTN_OUT LATAbits.LATA5         /* Output: RA5 */

void main(void) {

    int cat_err = 0;        /* var to check time for slp_n, pwr_gd seq. */
    int first_on = 1;       /* var to check first power up sequence */
    int cpu_pc = 0;         /* var to check cpu requested power cycle */
    int time_up;            /* var to check time */
    int cold_reset;         /* var to test cold reset/power off. */
 
    /* initialize Port A */
    TRISA = 0xFF;           /* initially set port as all inputs */
 
    /* set direction of any output I/O's */
    _ATX_PS_ON = 0; /* turn off  power supplies */
    _ATX_PS_ON_DIR = 0;
    ANSELA = 0;     /* turn off all analog inputs */

    /* enumerate the state machine and define all states */
    typedef enum {
        PWR_ON,
        G3_IDLE,
        PWR_UP1,
        PWR_UP2,
        PWR_BTN1, /* deprecated */
        PWR_BTN2, /* deprecated */
        S0_IDLE,
        FAULT,
        PWR_DN1
    } pic_state_t;

    /* define state variable and set it to initial state */
    pic_state_t state = PWR_ON;

    // state machine
    while (1) {
        switch(state) {
            case PWR_ON:

                /* Delay for 2 seconds */
                __delay_ms(2000);
                state = G3_IDLE;
                break;
 
            case G3_IDLE:

                //* two conditions for proceeding w/ power up... */
                if (first_on == 1 || cpu_pc == 1) {

                    first_on = 0;   /* clear first time power up flag */
                    cpu_pc = 0;     /* clear auto restart flag */

                    state = PWR_UP1;
                }
                break;

            case PWR_UP1:
 
                _ATX_PS_ON = 1; /* turn on other power supplies */
                __delay_ms(6000);
                state = PWR_UP2;

                break;

            case PWR_UP2:

                if (_SYS_SHDN_N == 1)
                {
                    state = FAULT;
                }

                /* check for valid PIC_SLP_N and COREPWR signal */
                else if (_PIC_SLP_N == 1 && (_PWRGD_COREPWR == 1))
                {
                    cat_err = 0;
                    state = S0_IDLE;    /* valid signal - go to S0 idle */
                }
                /* If SLP_N signal is postponed beyond normal operation,
                 * it could be we're signaling an upgrade -- or if the SoC
                 * is dead, continue to monitor for 3 minutes and then
                 * give up.
                 */
                else if ((_PIC_SLP_N == 0) || (_PWRGD_COREPWR == 0)) {
                    __delay_ms(1000);
                    cat_err++;

                    if (cat_err > CAT_ERR_TIMEOUT) {
                        state = FAULT;
                    }
                }
                break;

            case S0_IDLE:

                /* check for error */
                if (_PWRGD_COREPWR == 0) {
                    state = FAULT;

                /* If CPU THERMTRIP/SYS_SHDN, simply power off. */
                } else if (_SYS_SHDN_N == 1) {
                    state = FAULT;

                /*
                 * If _PIC_SLP_N, find if this is a power off request
                 * or a cold reset by timing how long the line stays
                 * low (forever for pwr off, 4 - 5 seconds if cold reset).
                 * If cold reset, keep a timer.  If our signal comes back
                 * in that time, keep in the 'IDLE' state. Otherwise, shake
                 * things loose with a poweroff/power on action.
                 */

                } else if (_PIC_SLP_N == 0 && _GPIO_CPU_SUS2 == 1) {

                    cold_reset = 0;

                    do {
                        __delay_ms(1000);
                        cold_reset++;
                    } while ( _PIC_SLP_N == 0 &&
                            cold_reset < PMU_SLP_S45_TIMEOUT);

                    /* If we timeout and _PIC_SLP_N and PWRGD are both 0,
                     * the SoC has parked itself in S5 and is ready to
                     * pwr off.
                     */
                    if ((_PIC_SLP_N == 0 && _PWRGD_COREPWR == 0) &&
                            cold_reset >= PMU_SLP_S45_TIMEOUT) {

                        state = FAULT;

                    /* Clocks may not have stabilized yet, so delay looking for
                     * a good PWRGD signal
                     */
                    } else if (_PIC_SLP_N == 1 && _PWRGD_COREPWR == 0) {

                        cold_reset = 0;
                        do {
                            __delay_ms(1000);
                            cold_reset++;
                        } while ((_PWRGD_COREPWR == 0 && _PIC_SLP_N == 1) &&
                                cold_reset < PMU_SLP_S45_TIMEOUT);

                        if (cold_reset < PMU_SLP_S45_TIMEOUT &&
                                (_PWRGD_COREPWR == 1 && _PIC_SLP_N == 1))
                            state = S0_IDLE;

                        /* As a last resort, knock hardware loose by issuing
                         * a power off/on action if PWRGD never asserts.
                         * Will never happen in practice since power
                         * will be maintained despite low input power condition.
                         * In other words, no rolling power cycles.  Tested with
                         * AC variac with low input AC voltages.  We want this
                         * in the extreme case.
                         */
                        else {
                            cpu_pc = 1;
                            state = PWR_DN1;
                        }
                    }
                    else
                        state = S0_IDLE;

                /* Hard reset beyond SoC cold reset, i.e. power off
                 * board for 6s before powering on. A VeloCloud want
                 * for a a 3rd tier policy(warm reset, cold reset,
                 *  power cycle, power off).
                 */
                } else if (_PIC_SLP_N == 0 && _GPIO_CPU_SUS2 == 0) {
                    cpu_pc = 1;
                    state = PWR_DN1;
                }
                break;

            case FAULT:
                _ATX_PS_ON = 0;
                while(1);           /* Turn off power supply and loop
                                     * infinite until external pwr removed.
                break;               */

            case PWR_DN1:
                _ATX_PS_ON = 0;

                __delay_ms(6000);
                state = G3_IDLE;
                break;

            default:
                /* default action should be to do nothing and continue
                 * checking only for catastrophic errors.
                 */
                if (_SYS_SHDN_N == 1)
                    state = FAULT;

                break;
        }
    }

}
