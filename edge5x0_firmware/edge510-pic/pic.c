// main.c v1 Sandra Berndt
// velocloud dolphin pic code;

#define PIC_BOARD_V1
#define SIZE_MSG 400

// CONFIG1
#pragma config FEXTOSC = OFF    // FEXTOSC External Oscillator mode Selection bits (Oscillator not enabled)
#pragma config RSTOSC = HFINT1  // Power-up default value for COSC bits (HFINTOSC (1MHz))
#pragma config CLKOUTEN = OFF   // Clock Out Enable bit (CLKOUT function is disabled; I/O or oscillator function on OSC2)
#pragma config CSWEN = ON       // Clock Switch Enable bit (Writing to NOSC and NDIV is allowed)
#pragma config FCMEN = ON       // Fail-Safe Clock Monitor Enable (Fail-Safe Clock Monitor is enabled)

// CONFIG2
#pragma config PWRTE = OFF      // Power-up Timer Enable bit (PWRT disabled)
#pragma config WDTE = ON        // Watchdog Timer Enable bits (WDT disbled, SWDTEN is ignored)
#pragma config LPBOREN = OFF    // Low-power BOR enable bit (ULPBOR disabled)
#pragma config BOREN = ON       // Brown-out Reset Enable bits (Brown-out Reset enabled, SBOREN bit ignored)
#pragma config BORV = LOW       // Brown-out Reset Voltage selection bit (Brown-out voltage (Vbor) set to 2.45V)
#pragma config PPS1WAY = ON     // PPSLOCK bit One-Way Set Enable bit (The PPSLOCK bit can be cleared and set only once; PPS registers remain locked after one clear/set cycle)
#pragma config STVREN = ON      // Stack Overflow/Underflow Reset Enable bit (Stack Overflow or Underflow will cause a Reset)
#pragma config DEBUG = OFF      // Debugger enable bit (Background debugger disabled)

// CONFIG3
#pragma config WRT = OFF	// User NVM self-write protection bits (Write protection off)

// CONFIG4
#pragma config CP = OFF         // User NVM Program Memory Code Protection bit (User NVM code protection disabled)
#pragma config CPD = OFF        // Data NVM Memory Code Protection bit (Data NVM code protection disabled)

// board rev1 MCLR was broken, need to turn off;

#ifdef PIC_BOARD_V1
#pragma config MCLRE = OFF	// MCLRE-OFF to fix glitch bug on MCLR pin when host is powered on;
#pragma config LVP = OFF	// Low Voltage Programming disabled;
#else // PIC_BOARD_V1
#pragma config MCLRE = ON	// Master Clear Enable bit, enables weak pullup;
#pragma config LVP = ON		// Low Voltage Programming enabled;
#endif // PIC_BOARD_V1

// #pragma config statements should precede project file includes.
// Use project enums instead of #define for ON and OFF.

#include <stddef.h>
#include <xc.h>
#include <pic16f18344.h>
#include <stdlib.h>

// types;

typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned long u32;
typedef signed char s8;
typedef signed short s16;
typedef signed int s32;

#define XTAL_FREQ 16000000

// in-circuit LVP interface;
// are inputs by default;
// can be used for direct/fast protocols to host GPIO;

#define ICSDAT_PORT PORTAbits.RA0
#define ICSDAT_TRIS TRISAbits.TRISA0
#define ICSDAT_OUT LATAbits.LATA0

#define ICSCLK_PORT PORTAbits.RA1
#define ICSCLK_TRIS TRISAbits.TRISA1
#define ICSCLK_OUT LATAbits.LATA1

// LED interface;
// output 0 turns on LED;

#define LED_R_PORT PORTCbits.RC3
#define LED_R_TRIS TRISCbits.TRISC3
#define LED_R_OUT LATCbits.LATC3
#define LED_R_MAP RC3PPSbits.RC3PPS

#define LED_G_PORT PORTCbits.RC5
#define LED_G_TRIS TRISCbits.TRISC5
#define LED_G_OUT LATCbits.LATC5
#define LED_G_MAP RC5PPSbits.RC5PPS

#define LED_B_PORT PORTAbits.RA4
#define LED_B_TRIS TRISAbits.TRISA4
#define LED_B_OUT LATAbits.LATA4
#define LED_B_MAP RA4PPSbits.RA4PPS

// reset push button;
// input 0 means button pushed;

#define RST_BUTTON_PORT PORTAbits.RA5
#define RST_BUTTON_TRIS TRISAbits.TRISA5

// reset signals to host cpu;

#define PICRST_PORT PORTCbits.RC7
#define PICRST_TRIS TRISCbits.TRISC7
#define PICRST_OUT LATCbits.LATC7

#define RSMRST_PORT PORTCbits.RC4
#define RSMRST_TRIS TRISCbits.TRISC4
#define RSMRST_OUT LATCbits.LATC4

// turn power supplies on;

#define PSON_PORT PORTBbits.RB5
#define PSON_TRIS TRISBbits.TRISB5
#define PSON_OUT LATBbits.LATB5

// sum of all power good signals from switchers;

#define PWRGD_PORT PORTAbits.RA2
#define PWRGD_TRIS TRISAbits.TRISA2

#define MCLR_PORT PORTAbits.RA3
#define MCLR_TRIS PORTAbits.TRISA3

// host cpu signaling out of sleep and/or running;
// 0 if sleeping (S3) or not in S0;

#define PMU_SLP_PORT PORTCbits.RC2
#define PMU_SLP_TRIS TRISCbits.TRISC2

// clock/power good;

#define CLKPWRGD_PORT PORTCbits.RC6
#define CLKPWRGD_TRIS TRISCbits.TRISC6
#define CLKPWRGD_OUT LATCbits.LATC6

// signal core/ddr power good;

#define COREPWR_PORT PORTCbits.RC1
#define COREPWR_TRIS TRISCbits.TRISC1
#define COREPWR_OUT LATCbits.LATC1

// i2c/smbus to host;

#define SCL_PORT PORTBbits.RB6
#define SCL_TRIS TRISBbits.TRISB6
#define SCL_MAP RB6PPS

#define SDA_PORT PORTBbits.RB4
#define SDA_TRIS TRISBbits.TRISB4
#define SDA_OUT LATBbits.LATB4
#define SDA_MAP RB4PPS

// misc io to host;
// as of board rev 2, this is thermtrip soignal from cpu;

#ifdef PIC_BOARD_V1
#define SUS2_PORT PORTCbits.RC0
#define SUS2_TRIS TRISCbits.TRISC0
#define SUS2_OUT LATCbits.LATC0
#else // PIC_BOARD_V1
#define THMTRIP_PORT PORTCbits.RC0
#define THMTRIP_TRIS TRISCbits.TRISC0
#define THMTRIP_OUT LATCbits.LATC0
#endif // PIC_BOARD_V1

// BLE chip reset;
// 0 keeps BLE/IOT chip in reset;

#define BLERST_PORT PORTBbits.RB7
#define BLERST_TRIS TRISBbits.TRISB7
#define BLERST_OUT LATBbits.LATB7

// reset cause;

typedef union reset reset_t;
union reset {
	unsigned char v;
	struct {
		unsigned RST_POR : 1;
		unsigned RST_BOR : 1;
		unsigned RST_WDT : 1;
		unsigned RST_MCLR : 1;
		unsigned RST_INS : 1;
		unsigned RST_STK : 1;
		unsigned : 2;
	} b;
};

reset_t reset;

// power states;

enum pstate {
	PST_OK = 0,
	PST_TURN_OFF,		// start power off;
	PST_MINOFF,		// minimal off time;
	PST_OFF,		// power is off;
	PST_TURN_ON,		// start power on sequence;
	PST_WAIT_PWRGD,		// wait for power good, has timeout;
	PST_PWRGD_STABLE,	// wait for PWRGD to stabilize;
	PST_RSMRST_DELAY,	// RSMRST delay;
	PST_WAIT_PMUSLP,	// wait for PMU_SLP deassertion;
	PST_CLKPWRGD_DELAY,	// CLKPWRGD delay;
	PST_COREPWR_DELAY,	// COREPWR delay;
	PST_RESET_ASSERT,	// assert reset, do reset cycle;
	PST_RESET_DELAY,	// reset delay;
	PST_RUNNING,		// running state;
	PST_COLD_RESET,		// cold reset;
	PST_SLP_CR_DELAY,	// wait for PMU SLP to reassert for cold reset;
};

enum pstate_times {
	PSTATE_MIN_OFF = 2000,		// min power off time;
	PSTATE_PWRGD_TO = 200,		// max PWRGD timout;
	PSTATE_PWRGD_STABLE = 100,	// wait for PWRGD to stabilize;
	PSTATE_RSMRST_DELAY = 10,	// delay PWRDGD -> RSMRST rising;
	PSTATE_PMUSLP_TO = 250,		// max PMU_SLP delay;
	PSTATE_CLKPWRGD_DELAY = 5,	// delay PMUSLP -> CLKPWRGD rising;
	PSTATE_COREPWR_DELAY = 150,	// delay CLKPWRGD -> COREPWR rising;
	PSTATE_RESET_DELAY = 100,	// delay COREPWR -> RESET deassertion;
	PSTATE_RSMRST_ASSERT = 100,	// RSMRST assertion duration for cold reset;
	PSTATE_SLP_CR_DELAY = 5100,	// cold reset reassert delay;
};

// ble reset state;
// eeprom tells if BLE should be kept in reset on pic startup;

enum ble_set {
	BLE_NONE = 0,
	BLE_RST_ASSERT,		// assert BLE reset;
	BLE_RST_DEASSERT,	// deassert BLE reset;
};

typedef union ble_get ble_get_t;
union ble_get {
	unsigned char v;
	struct {
		unsigned IN_RESET : 1;	// ble chip held in reset;
		unsigned EE_RESET : 1;	// eeprom forces ble held in reset;
		unsigned : 6;
	} b;
};

// eeprom data;

#define EE_SWDT 	0	// 0:1: default swdt;
#define EE_BLE_RESET 	2	// 2: BLE reset byte;

enum eests {
	EE_IDLE = 0,
	EE_ERROR,	// must be before others;
	EE_START,
	EE_BUSY,
};

// globals;

u8 pstate;	// power state;
u8 nstate;	// new power requested by host;
u8 rgb[3];	// LED pwm values per RGB;
u8 thmtrip;	// # of thermal shutdowns;
ble_get_t ble;	// ble reset state;
u8 eeaddr;	// write address;
u8 eedata;	// write data byte;
u8 eests;	// eeprom status;

// i2c commands;
// each address has specific command;

enum i2c_cmds {
	I2C_BASE = 0x40,	// i2c base address;
	I2C_MASK = 0xe0,	// accept all 010xxxxR addresses;

	I2C_LED_SET = I2C_BASE,	// set rgb LED;
	I2C_LED_GET,		// read rgb LED;
	I2C_PST,		// host requests reset/power state change;
	I2C_RESET,		// read reset cause;
	I2C_SWDT_SET,		// set system watchdog;
	I2C_SWDT_GET,		// get system watchdog;
	I2C_BLE_SET,		// set BLE reset state;
	I2C_BLE_GET,		// get BLE reset state;

	I2C_EE_WRITE,		// write eeprom byte;
	I2C_EE_STS,		// eeprom status;
	I2C_EE_ADDR,		// set eeprom read address;
	I2C_EE_READ,		// read eeprom byte(s);

	I2C_MSG_SET,		// set msg ptr;
	I2C_MSG_GET,		// get msg ptr;
	I2C_MSG_PUTC,		// write msg byte;
	I2C_MSG_GETC,		// read msg byte;
};

// i2c byte for I2C_PST command;

enum i2c_reset {
	RESET_WARM = 0,		// warm reset;
	RESET_COLD,		// cold reset;
	POWER_OFF,		// turn off power;
};

// i2c state;

typedef union {
	u8 v;
	struct {
		unsigned read : 1;
		unsigned addr : 7;
	};
} i2c_addr_t;

i2c_addr_t i2c_addr;	// address byte;
u8 i2c_idx;		// buffer index;
u8 i2c_nb;		// # of bytes;
u8 i2c_buf[8];		// cmd/data buffer;

// message buffer;

typedef union mptr mptr_t;
union mptr {
	u8 b[2];	// as bytes, little endian;
	u16 p;		// as offset;
};

mptr_t mptr;
u8 msg[SIZE_MSG];

// system watchdog;
// ticks at 25 Hz (40ms);

#define SWDT_TICK 25
#define SWDT_MIN (1*60*SWDT_TICK)	// min time to do anything;
#define SWDT_DEFAULT (5*60*SWDT_TICK)	// default if eeprom is not programmed;

typedef union swdt swdt_t;
union swdt {
	u8 b[2];	// as bytes, little endian;
	u16 v;		// value;
};
swdt_t swdt;		// system watchdog timer;

// interrupt enable/disable;

#define IntrOff() GIE = 0
#define IntrOn() GIE = 1

// set LED pwm;
// LEDs are powered from 5V, driving out H (off) can bleed, as seen on R;

static inline void
led_set(void)
{
	CCPR1H = ~rgb[1];	// green;
	CCPR2H = ~rgb[0];	// red;
	CCPR4H = ~rgb[2];	// blue;

	LED_R_TRIS = (rgb[0] == 0);
	LED_G_TRIS = (rgb[1] == 0);
	LED_B_TRIS = (rgb[2] == 0);
}

// turn off LED;

static inline void
led_off(void)
{
	rgb[0] = 0;
	rgb[1] = 0;
	rgb[2] = 0;
	led_set();
}

// turn LED red;

static inline void
led_red(void)
{
	rgb[0] = 0xff;
	rgb[1] = 0;
	rgb[2] = 0;
	led_set();
}

// eeprom setup;

static inline void
ee_setup(void)
{
        NVMCON1bits.NVMREGS = 1;
        NVMCON1bits.LWLO = 0;
	NVMADRH = 0x70;
}

// read eeprom byte;

static inline u8
ee_read(u8 addr)
{
	NVMCON1bits.WREN = 0;
	NVMADRL = addr;
	NVMCON1bits.RD = 1;
	return(NVMDATL);
}

// write eeprom byte;

static inline void
ee_write(void)
{
	ee_setup();
	NVMCON1bits.WREN = 1;
	NVMADRL = eeaddr;
	NVMDATL = eedata;
	NVMCON2 = 0x55;
	NVMCON2 = 0xaa;
	NVMCON1bits.WR = 1;
}

// check eeprom done;
// returns new eests;

static inline u8
ee_done(void)
{
	if(NVMCON1bits.WR)
		return(EE_BUSY);
	NVMCON1bits.WREN = 0;
	if(NVMCON1bits.WRERR)
		return(EE_ERROR);
	return(EE_IDLE);
}

// read swdt from eeprom;
// set to default if not programmed;

static inline void
swdt_init(void)
{
	ee_setup();
	swdt.b[0] = ee_read(EE_SWDT + 0);
	swdt.b[1] = ee_read(EE_SWDT + 1);
	if(swdt.v == 0xffff)
		swdt.v = SWDT_DEFAULT;
	if(swdt.v < SWDT_MIN)
		swdt.v = SWDT_DEFAULT;
}

// handle system watchdog;

static inline void
swdt_fsm(void)
{
	if(swdt.v) {
		if(TMR4IF) {
			TMR4IF = 0;
			swdt.v--;
		}
		if(swdt.v == 0)
			pstate = PST_COLD_RESET;
	}
}

// start new i2c transaction;

u8
i2c_start(void)
{
	switch(i2c_addr.v) {

	// set LED;
	case I2C_LED_SET:
		i2c_nb = sizeof(rgb);
		break;

	// get LED;
	case I2C_LED_GET:
		i2c_nb = sizeof(rgb);
		i2c_buf[0] = rgb[0];
		i2c_buf[1] = rgb[1];
		i2c_buf[2] = rgb[2];
		break;

	// power/reset something;
	// only byte is reset/power type;
	case I2C_RESET:
		i2c_buf[0] = reset.v;
	case I2C_PST:
		i2c_nb = 1;
		break;

	// system watchdog timer;
	// write data is actual next timeout;
	// 0 effectively stops it;
	case I2C_SWDT_GET:
		i2c_buf[0] = swdt.b[0];
		i2c_buf[1] = swdt.b[1];
	case I2C_SWDT_SET:
		i2c_nb = 2;
		break;

	// BLE reset handling;
	case I2C_BLE_GET:
		i2c_buf[0] = ble.v;
	case I2C_BLE_SET:
		i2c_nb = 1;
		break;

	// eeprom access;
	case I2C_EE_STS:
		i2c_buf[0] = eests;
		i2c_nb = 1;
		break;
	case I2C_EE_WRITE:
		if(eests >= EE_START)
			return(0);
		i2c_nb = 2;
		break;
	case I2C_EE_READ:
		if(eests >= EE_START)
			return(0);
		ee_setup();
		i2c_buf[0] = ee_read(eeaddr);
		eeaddr++;
		i2c_nb = 1;
		break;
	case I2C_EE_ADDR:
		if(eests >= EE_START)
			return(0);
		i2c_nb = 1;
		break;

	// message buffer;
	case I2C_MSG_GET:
		i2c_buf[0] = mptr.b[0];
		i2c_buf[1] = mptr.b[1];
	case I2C_MSG_SET:
		i2c_nb = 2;
		break;
	case I2C_MSG_GETC:
	case I2C_MSG_PUTC:
		if(mptr.p >= SIZE_MSG)
			return(0);
		if(i2c_addr.read) {
			i2c_buf[0] = msg[mptr.p];
			mptr.p++;
		}
		i2c_nb = 1;
		break;

	// NACK invalid addresses;
	default:
		i2c_nb = 0;
		return(0);
	}
	return(1);
}

// execute i2c write commands;
// returns !0 on command error;

u8
i2c_exec(void)
{
	switch(i2c_addr.v) {

	// set LED;
	case I2C_LED_SET:
		rgb[0] = i2c_buf[0];
		rgb[1] = i2c_buf[1];
		rgb[2] = i2c_buf[2];
		led_set();
		break;

	// reset something;
	// single byte is reset type;
	case I2C_PST:
		if(i2c_buf[0] == RESET_WARM)
			nstate = PST_RESET_ASSERT;
		else if(i2c_buf[0] == RESET_COLD)
			nstate = PST_COLD_RESET;
		else if(i2c_buf[0] == POWER_OFF)
			nstate = PST_TURN_OFF;
		break;

	// system watchdog;
	case I2C_SWDT_SET:
		swdt.b[0] = i2c_buf[0];
		swdt.b[1] = i2c_buf[1];
		break;

	// BLE reset handling;
	case I2C_BLE_SET:
		if(i2c_buf[0] == BLE_RST_ASSERT)
			ble.b.IN_RESET = 1;
		else if(i2c_buf[0] == BLE_RST_DEASSERT)
			ble.b.IN_RESET = 0;
		break;

	// eeprom access;
	case I2C_EE_WRITE:
		eedata = i2c_buf[1];
		eests = EE_START;
	case I2C_EE_ADDR:
		eeaddr = i2c_buf[0];
		break;

	// msg putc;
	case I2C_MSG_SET:
		mptr.b[0] = i2c_buf[0];
		mptr.b[1] = i2c_buf[1];
		break;
	case I2C_MSG_PUTC:
		msg[mptr.p] = i2c_buf[0];
		mptr.p++;
		break;

	// default, NACK;
	default:
		return(1);
	}

	return(0);
}

// commit read transaction;
// in case we need to do something at end of read;

void
i2c_commit(void)
{
}

// interrupt handler;

void interrupt
Interrupt_ISR(void)
{
	// handle ssp2 i2c bus collision;

	if(BCL1IF) {
		BCL1IF = 0;
		//XXX
	}

	// handle ssp2 interrupt;
	// ACK handling needs to be fast path;
	// master sending transactions with less bytes than required are dropped;

	if(SSP1IF) {
		SSP1IF = 0;

		// handle byte phase;
		// ACKTIM=1 & BF=1 only for i2c address and write data;
		// D/A=0 indicates new address phase;

		if(SSP1CON3bits.ACKTIM) {
			SSP1CON2bits.ACKDT = 1;				// nack by default;
			if(SSPSTATbits.BF) {
				if( !SSPSTATbits.D_nA) {		// new address phase;
					i2c_addr.v = SSP1BUF;
					i2c_idx = 0;
					if(i2c_start())
						SSP1CON2bits.ACKDT = 0;	// ack good addr;
				} else if( !i2c_addr.read) {		// write data byte;
					if(i2c_idx < i2c_nb) {
						i2c_buf[i2c_idx] = SSP1BUF;	// put byte into buffer;
						SSP1CON2bits.ACKDT = 0;		// ack good byte;
					}
					if(++i2c_idx == i2c_nb) {
						i2c_idx = 0;			// accept more bytes;
						if(i2c_exec())			// execute write command;
							SSP1CON2bits.ACKDT = 1;	// nack cmd errors;
					}
				}
			}

		// ACKTIM=0 ack/nack phase;
		// if i2c read, load next byte to send to master;
		// commit read transaction when ACKed;

		} else if( !SSP1CON1bits.CKP) {
			if(i2c_addr.read) {
				if(i2c_idx == i2c_nb) {
					if(i2c_nb && !SSP1CON2bits.ACKSTAT)
						i2c_commit();		// ack'ed, commit transaction;
					i2c_nb = 0;
					SSP1BUF = 0;
				} else if(i2c_idx < i2c_nb) {
					SSP1BUF = i2c_buf[i2c_idx];	// send next byte;
					i2c_idx++;
				} else {
					SSP1BUF = 0;			// reading more than expected;
				}
			}
		}

		// SCK is being held low, release it;
		// CKP will be held low only when ACKed, not when NACKed;

		if( !SSP1CON1bits.CKP)
			SSP1CON1bits.CKP = 1;
	}
}

// check if reset button has been pressed;

static inline u8
rst_button(void)
{
	return( !RST_BUTTON_PORT);
}

// start power state timer;
// time in msec, upto 32secs;

inline void
pst_start(u16 msec)
{
	u8 val;

	TMR1ON = 0;
	TMR1IF = 0;
	msec <<= 1;
	val = msec;
	TMR1L = ~val;
	val = msec >> 8;
	TMR1H = ~val;
	TMR1ON = 1;
}

// check if power state timer expired;
// turn off pst timer;

static inline u8
pst_expired(void)
{
	if( !TMR1IF)
		return(0);
	TMR1IF = 0;
	TMR1ON = 0;
	return(1);
}
 
// advance the power on/off state machine;
// returns 0 if ok, else state that failed;
// on failure, goes back to PST_TURN_OFF;

u8
power_fsm(void)
{
	u8 failed = pstate;

	switch(pstate) {

	// turn power off;
	// assert resets first, then take away power goods;
	case PST_TURN_OFF:
		RSMRST_OUT = 0;
		COREPWR_OUT = 0;
		CLKPWRGD_OUT = 0;
		PSON_OUT = 0;
		led_off();
		pst_start(PSTATE_MIN_OFF);
		pstate++;
		break;

	// wait for minimal power off time;
	case PST_MINOFF:
		if(pst_expired())
			pstate++;
		break;

	// min power off time has expired;
	// monitor reset button, if pushed then power on;
	case PST_OFF:
		if(rst_button())
			pstate++;
		break;

	// start power-on sequence;
	// turn on supplies, start power timer;
	case PST_TURN_ON:
		RSMRST_OUT = 0;
		COREPWR_OUT = 0;
		CLKPWRGD_OUT = 0;
		PSON_OUT = 1;
		led_red();
		pst_start(PSTATE_PWRGD_TO);
		pstate++;
		break;

	// wait for power good;
	// turn power off if timed out;
	case PST_WAIT_PWRGD:
		if(PWRGD_PORT) {
			pst_start(PSTATE_PWRGD_STABLE);
			pstate++;
		} else if(pst_expired()) {
			goto fail_pst;
		}
		break;

	// PWRGD appears to be unstable after first assertion;
	// delay for spurious PWRGD settling down;
	case PST_PWRGD_STABLE:
		if(pst_expired()) {
			pst_start(PSTATE_RSMRST_DELAY);
			pstate++;
		} else if( !PWRGD_PORT) {
			pstate = PST_WAIT_PWRGD;
		}
		break;

	// deassert rsm reset;
	// from now on, if PWRGD ever goes 0, then we have a power issue;
	case PST_RSMRST_DELAY:
		if(pst_expired()) {
			RSMRST_OUT = 1;
			pst_start(PSTATE_PMUSLP_TO);
			pstate++;
		}
		break;

	// wait for PMU_SLP deassertion;
	// turn power off if timed out;
	case PST_WAIT_PMUSLP:
		if(pst_expired()) {
			goto fail_pst;
		} else if(PMU_SLP_PORT) {
			pst_start(PSTATE_CLKPWRGD_DELAY);
			pstate++;
		}
		break;

	// delay and assert CLKPWRGD;
	// from now on, if PMU_SLP ever goes 0, then cpu tells us to power off;
	case PST_CLKPWRGD_DELAY:
		if(pst_expired()) {
			CLKPWRGD_OUT = 1;
			pst_start(PSTATE_COREPWR_DELAY);
			pstate++;
		}
		break;

	// delay and assert CLKPWRGD;
	case PST_COREPWR_DELAY:
		if(pst_expired()) {
			COREPWR_OUT = 1;
			swdt_init();
			pstate = PST_RUNNING;
		}
		break;

	// assert reset;
	// we can enter into this state for just a reset cycle;
	case PST_RESET_ASSERT:
		PICRST_TRIS = 0;
		led_red();
		pst_start(PSTATE_RESET_DELAY);
		pstate++;
		break;

	// delay and deassert reset;
	case PST_RESET_DELAY:
		if(pst_expired()) {
			PICRST_TRIS = 1;
			swdt_init();
			pstate++;
		}
		break;

	// running state;
	// PWRGD and PMU_SLP are monitored below;
	// cold reset system is SWDT triggers;
	case PST_RUNNING:
		if(rst_button())
			pstate = PST_RESET_ASSERT;
		swdt_fsm();
		break;

	// cold reset;
	// going through RSMRST;
	case PST_COLD_RESET:
		RSMRST_OUT = 0;
		led_red();
		pst_start(PSTATE_RSMRST_ASSERT);
		pstate = PST_RSMRST_DELAY;
		break;

	// PMU_SLP cold reset / power off delay;
	case PST_SLP_CR_DELAY:
		if(pst_expired()) {
			if(PMU_SLP_PORT)
				pstate = PST_COLD_RESET;
			else
				pstate = PST_TURN_OFF;
		}
		break;

	// this should never happen;
	default:
		goto fail_pst;
	}

	// if thermtrip ever goes active, power off hard;

#ifndef PIC_BOARD_V1
	if((pstate > PST_TURN_ON) && THMTRIP_PORT) {
		if(thmtrip != 0xff)
			thmtrip++;
		goto fail_pst;
	}
#endif // PIC_BOARD_V1

	// if >= PST_RSMRST_DELAY, PWRGD must never go 0;
	// if it does we have a power issue, so shut down;

	if((pstate >= PST_RSMRST_DELAY) && !PWRGD_PORT)
		goto fail_pst;

	// cpu signals cold reset or power off via PMU_SLP;
	// if it reasserts within 5secs, then cold reset, else power off;
	// if >= PST_CLKPWRGD_DELAY and PMU_SLP goes 0, then power off;

	if((pstate >= PST_CLKPWRGD_DELAY) && (pstate != PST_SLP_CR_DELAY) && !PMU_SLP_PORT) {
		pst_start(PSTATE_SLP_CR_DELAY);
		pstate = PST_SLP_CR_DELAY;
	}

	return(0);

	// something failed;
	// turn power off, return failed state;
fail_pst:
	pstate = PST_TURN_OFF;
	return(failed);
}

// main entry;
// called for every reset, wdt, brown-out, etc;
// power may have been on or off;

void
main(void)
{
	// set internal oscillator;
	// 16Mhz, COSC=000 CDIV=0001 (/2);
	// HFINTOSC can be +/-2%, so upto 1/2hr/day;

	OSCCON1 = 0x01;

	// tristate all ports;
	// making them inputs;

	TRISA = 0xff;
	TRISB = 0xff;
	TRISC = 0xff;

	// turn off all analog function selects;

	ANSELA = 0;
	ANSELB = 0;
	ANSELC = 0;

	// determine reset cause;

	reset.v = 0;
	if(STATUSbits.nPD && STATUSbits.nTO) {
		if(PCON0bits.nPOR == 0)
			reset.b.RST_POR = 1;
		if(PCON0bits.nBOR == 0)
			reset.b.RST_BOR = 1;
	} else if(STATUSbits.nTO == 0) {
		reset.b.RST_WDT = 1;
		asm("clrwdt");
	} else {
		if(PCON0bits.nRMCLR == 0)
			reset.b.RST_MCLR = 1;
		if(PCON0bits.nRI == 0)
			reset.b.RST_INS = 1;
		if(PCON0bits.STKUNF || PCON0bits.STKOVF)
			reset.b.RST_STK = 1;
	}

	// configure important power controls;
	// if anything was on before, apply resets first;
	// then remove power good signals and power off;

	PICRST_OUT = 0;
	PICRST_TRIS = 1;
	RSMRST_OUT = 0;
	RSMRST_TRIS = 0;

	// configure LED pins;
	// turn on red LED, to indicate error in case we fail;

	LED_R_OUT = 0;
	LED_G_OUT = 1;
	LED_B_OUT = 1;
	LED_R_TRIS = 0;
	LED_G_TRIS = 0;
	LED_B_TRIS = 0;

	// turn off power good signals to host cpu;
	// turn off powers;

	COREPWR_OUT = 0;
	COREPWR_TRIS = 0;
	CLKPWRGD_OUT = 0;
	CLKPWRGD_TRIS = 0;
	PSON_OUT = 0;
	PSON_TRIS = 0;

	// do not disturb BLE/IOT reset;
	// however, later, eeprom may tell us otherwise;

	BLERST_OUT = BLERST_PORT;
	BLERST_TRIS = 0;

	// init TMR2 for pwm operation;
	// this is an 8-bit timer, counting 0..ff;
	// prescaler=16, timer on, postscaler=1;
	// pwm freq is fosc/4/pre/256/post = ~1000 Hz;

	T2CON = 0x06;
	PR2 = 0xfe;
	TMR2 = 0;

	// init TMR4 for SWDT;
	// this is an 8-bit timer, counting 0..ff;
	// prescaler=64, timer on, postscaler=10;
	// tick req is fosc/4/pre/250/post = 25 Hz;

	T4CON = 0x4f;
	PR4 = 250;
	TMR4 = 0;

	// init CCP mapping for LED pwm control;
	// pwm mode, enabled, left-aligned mode;
	// left-aligned only needs to write CCPxH, leave CCPRxL fixed;

	CCPTMRS = 0;		// TMR2 for all CCPs;

	CCP1CON = 0x9f;
	CCP2CON = 0x9f;
	CCP4CON = 0x9f;

	CCPR1L = 0xc0;
	CCPR2L = 0xc0;
	CCPR4L = 0xc0;

	LED_R_MAP = 0x0d;
	LED_G_MAP = 0x0c;
	LED_B_MAP = 0x0f;

	// init LED to full red;

	led_red();

	// read eeprom variables;

	eeaddr = 0;
	eests = EE_IDLE;

	// setup ble reset behaviour;
	// eeprom tells if BLE should be kept in reset;
	// main loop updates BLERST_OUT;

	ble.v = 0;
	if( !BLERST_PORT)
		ble.b.IN_RESET = 1;
	ee_setup();
	eedata = ee_read(EE_BLE_RESET);
	if(eedata == 0xff) {
		ble.b.EE_RESET = 1;
		ble.b.IN_RESET = 1;
		BLERST_OUT = 0;
	}

	// set msg ptr beyon buffer to force pointer setting;

	mptr.p = SIZE_MSG;

	// 1ms timer;
	// TMR0 divides clock to 1000Hz (1ms);
	// fosc/4, prescaler=16, period=250, postscaler=1;

	T0CON0 = 0;		// disable timer, postscaler=2;
	TMR0L = 0;		// clear timer;
	TMR0H = 250;
	T0CON1 = 0x44;		// fosc/4, sync, prescaler=16;
	T0EN = 1;

	// TMR1 is power fsm timer;
	// TMR1 uses TMR0 output to count total time;
	// TMR1 is set to 0xffff-time, then triggers on overflow;
	// based on the prescalers TMR0=16/TMR1=8 gate allows two clocks through;

	T1CON = 0x30;		// prescaler=8, sync;
	T1GCON = 0xc1;		// use TMR0 gating;

	// init SSP for I2C slave;
	// use block of addresses for writes/reads;

	SSP1ADD = I2C_BASE;
	SSP1MSK = I2C_MASK;
	SSP1IF = 0;		// clear old intr;

	SSPSTATbits.SMP = 1;
	SSP1CON2 = 0x01;	// gcen=0, enable clock stretching;
	SSP1CON3 = 0x07;	// 100ns hold, AHEN=1 DHEN=1, enable collision detect;
	SSP1CON1 = 0x36;	// i2c slave, 7-bit addr, enabled;

	SDA_MAP = 0x19;		// map i2c outputs, inputs have correct defaults;
	SCL_MAP = 0x18;

	SSP1IE = 1;		// enable ssp interrupts;
	BCL1IE = 1;		// enable ssp bus collision intr;

	// first time boot;
	// turn on system;

	pstate = PST_TURN_ON;
	nstate = PST_OK;
	thmtrip = 0;

	// reduce watchdog to 256ms;

	WDTCON = 8;

	// enable peripheral interrupts;
	// enable global interrupts;

	PEIE = 1;
	IntrOn();

	// main loop;
	// the power state machine is polling;
	// i2c transactions are mostly handled in intr;
	// no function in here should block;

	while(1) {
		asm("clrwdt");

		// handle power/reset state machine;
		// remember states that had errors;

		power_fsm();
		//XXX fail log

		// apply power state changes requested by host;

		if(nstate) {
			pstate = nstate;
			nstate = PST_OK;
		}

		// update BLE reset;

		BLERST_OUT = !ble.b.IN_RESET;

		// eeprom byte write state machine;

		if(eests == EE_START) {
			ee_write();
			eests = EE_BUSY;
		} else if(eests == EE_BUSY) {
			eests = ee_done();
		}
	}

}

