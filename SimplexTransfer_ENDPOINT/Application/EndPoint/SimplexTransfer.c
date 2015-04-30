/**
 *  ----------------------------------------------------------------------------
 *  Copyright (c) 2012-13, Anaren Microwave, Inc.
 *
 *  For more information on licensing, please see Anaren Microwave, Inc's
 *  end user software licensing agreement: EULA.txt.
 *
 *  ----------------------------------------------------------------------------
 *
 *  SimplexTransfer.c - performs a simplex transfer using the SimpleTransfer()
 *  API call. For each message sent, a packet sequence number is incremented.
 *  
 *
 *  @version    1.0.00
 *  @date       04 Feb 2013
 *  @author     BPB, air@anaren.com
 *
 *  assumptions
 *  ===========
 *  - this is being compiled exclusively with End Point node(s).
 *  
 *  file dependency
 *  ===============
 *  API.h : defines the protocol API.
 *
 *  revision history
 *  ================
 *  ver 1.0.00 : 04 Feb 2013
 *  - initial release
 */
#ifndef bool
#define bool unsigned char
#endif

#ifndef true
#define true 1
#endif

#ifndef false
#define false 0
#endif

#include "API.h"

#include <stdio.h>
#include <math.h>

//#define Sensor 1


// -----------------------------------------------------------------------------
/**
 *  Defines, enumerations, and structure definitions
 */

#define ST(X) do { X } while (0)

/**
 *  Abstract hardware based on the supported example platform being used.
 * 
 *  Currently supported platforms:
 *    - TI MSP430G2553 + AIR A110x2500 Booster Pack (BPEXP430G2x53.c)
 */
#if defined( __MSP430G2553__ )
#define HardwareInit()\
  ST\
  (\
    WDTCTL = WDTPW | WDTHOLD;\
    BCSCTL1 = CALBC1_8MHZ;\
    DCOCTL = CALDCO_8MHZ;\
  )
#define McuSleep()    _BIS_SR(LPM4_bits | GIE)  // Go to low power mode 4
#define McuWakeup()   _BIC_SR(LPM4_EXIT);       // Wake up from low power mode 4
#define GDO0_VECTOR   PORT2_VECTOR
#define GDO0_EVENT    P2IFG
#endif

/**
 *  sPacket - an example packet. The sequence number is used to demonstrate
 *  communication by sending the same message (payload) and incrementing the
 *  sequence number on each transmission.
 */
struct sPacket
{
  unsigned char seqNum;      // Packet sequence number
  unsigned char payload[10]; // Packet payload
};

// -----------------------------------------------------------------------------
/**
 *  Global data
 */

/**
 *
 *  The following instance of sProtocolSetupInfo is used to initialize the
 *  protocol with required parameters. The End Point node role has the following
 *  parameters [optional parameters are marked with an asterix (*)].
 *        
 *  { channel_list, pan_id, local_address, Backup(*), TransferComplete(*) }
 *
 *  Note: Parameters marked with the asterix (*) may be assigned "NULL" if they
 *  are not needed.
 */
static const struct sProtocolSetupInfo gProtocolSetupInfo = {
  { PROTOCOL_CHANNEL_LIST },// Physical channel list
  { 0x01 },                 // Physical address PAN identifier
  { 0x03 },                 // Physical address
  NULL,                     // Protocol Backup callback (not used)
  NULL                      // Protocol Data Transfer Complete callback (not used)
};

static struct sPacket gPacket = {
  0x00,                     // Set the initial sequence number value to 0
  "Hello3"                   // Set the initial payload to a "Hello" string
};


////////////////////////////////////////////////////////////////////////////////

long Time[3];
unsigned int i = 0;
int wdtCounter = 0;
float LastB;
float ActualB;

//------------------------------------------------------------------------------
// Hardware-related definitions
//------------------------------------------------------------------------------
#define UART_TXD   0x02                     // TXD on P1.1 (Timer0_A.OUT0)
#define UART_RXD   0x04                     // RXD on P1.2 (Timer0_A.CCI1A)

//------------------------------------------------------------------------------
// Conditions for 9600 Baud SW UART, SMCLK = 1MHz
//------------------------------------------------------------------------------
#define UART_TBIT_DIV_2     (8000000 / (4800 * 2))
#define UART_TBIT           (8000000 / 4800)

//------------------------------------------------------------------------------
// Global variables used for full-duplex UART communication
//------------------------------------------------------------------------------
unsigned int txData;                        // UART internal variable for TX
unsigned char rxBuffer;                     // Received UART character

//------------------------------------------------------------------------------
// Function prototypes
//------------------------------------------------------------------------------
void TimerA_UART_init(void);
void TimerA_UART_tx(unsigned char byte);
void TimerA_UART_print(char *string);



void ChargingStep( int ChargingBit, int DischargingBits )
{
	//P2DIR |= ChargingBit; // Set P2.x to output direction
	//P2DIR &= (~DischargingBits);
	P2DIR = ( P2DIR | ChargingBit ) & ~DischargingBits;

	P2OUT |= ( ChargingBit | DischargingBits ) ; // Set P2.x at 1.0
	_delay_cycles(100 * 8);
}

void DischargingStep( int OutputBit, int InputBits, int ChargingBit )
{
	// Iniciar el temporizador y apagar poner el micro a estado de bajo consumo

	// Inicializamos la interrupción para finalizar la descarga y activar el micro
	P2IES |= ChargingBit;   // high -> low is selected with IES.x = 1.
	P2IFG &= ~ChargingBit;  // To prevent an immediate interrupt, clear the flag for
	                        // P2.x before enabling the interrupt.
	P2IE |= ChargingBit;    // Enable interrupts for P2.x
	__enable_interrupt();

	// Acomodamos las patitas
	P2DIR = ( P2DIR | OutputBit ) & ~InputBits;
	P2OUT &= ~OutputBit;// Set P2.x at 0.0

	// Iniciar el temporizador y poner a dormir el micro
	//TACCTL0 = CCIE;
	//TACTL = TASSEL_2 + MC_2; // Set the timer A to SMCLCK, Continuous
	TACTL = TASSEL_2 + MC_2; // Set the timer A to ACLK, Continuous
	// Clear the timer and enable timer interrupt
	TA0R = 0;
    __enable_interrupt();
	//__delay_cycles(1000);
	// Mandar a dormir el micro
	_BIS_SR(LPM1_bits + GIE); // Enter LPM3 w/interrupt
}

float ReadB()
{
	// Carga y descarga para P2.3
	i = 0;
	ChargingStep( BIT0, BIT3 | BIT2 | BIT1 );
	DischargingStep( BIT3, BIT0 | BIT2 | BIT1, BIT0 );

	// Carga y descarga para P2.2
	i = 1;
	ChargingStep( BIT0, BIT3 | BIT2 | BIT1 );
	DischargingStep( BIT2, BIT0 | BIT3 | BIT1, BIT0 );

	// Carga y descarga para P2.1
	i = 2;
	ChargingStep( BIT0, BIT3 | BIT2 | BIT1 );
	DischargingStep( BIT1, BIT0 | BIT2 | BIT3, BIT0 );

	//float x = 2.0f * (float)( Time[0] - Time[2] ) / (float)( Time[1] + Time[2] - Time[0] );

	//float B = x / ( ( x - 2.0f ) * 14.5f );

	long a = ( 1000000 * 2 * ( Time[0] - Time[2] ) );
    // / ( 29.0 * ( 2.0 * (Time[0] - Time[2]) - Time[1] ) );

	long b = ( 29 * ( 2 * (Time[0] - Time[2]) - Time[1] ) );

	return (float)a / (float)b;
}

////////////////////////////////////////////////////////////////////////////////


// -----------------------------------------------------------------------------

/**
 *  PlatformInit - sets up platform and protocol hardware. Also configures the
 *  protocol using the setup structure data.
 *
 *    @return   Success of the operation. If true, the protocol has been setup
 *              successfully. If false, an error has occurred during protocol
 *              setup.
 */
bool PlatformInit(void)
{
  // Disable global interrupts during hardware initialization to prevent any
  // unwanted interrupts from occurring.
  MCU_DISABLE_INTERRUPT();
  
  // Setup basic platform hardware (e.g. watchdog, clocks).
  HardwareInit();
  
  // Attempt to initialize protocol hardware and information using the provided
  // setup structure data.
  if (!ProtocolInit(&gProtocolSetupInfo))
  {
    return false;
  }
  
  // Re-enable global interrupts for normal operation.
  MCU_ENABLE_INTERRUPT();
  
  return true;
}

/**
 *  main - main application loop. Sets up platform and then performs simple
 *  transfers (simplex) while incrementing the sequence number for the lifetime 
 *  of execution.
 *
 *    @return   Exit code of the main application, however, this application
 *              should never exit.
 */
int main(void)
{
    // Setup hardware and protocol.

	#ifdef Sensor
	WDTCTL = WDTPW + WDTHOLD;                // Stop WDT
	//IE1 |= WDTIE;  // Enable WDT interrupt

	// Para trabajar a 8 MHz
	BCSCTL1 = CALBC1_8MHZ;
	DCOCTL = CALDCO_8MHZ;


	// TODO: Es para el debug el serial
    P1OUT = 0x00;                           // Initialize all GPIO
    P1SEL = UART_TXD + UART_RXD;            // Timer function for TXD/RXD pins
    P1DIR = 0xFF & ~UART_RXD;               // Set all pins but RXD to output

    //P1DIR |= 0x01;                            // Set P1.0 to output direction

    __enable_interrupt();

    TimerA_UART_init();                     // Start Timer_A UART
    TimerA_UART_print("Probando123");

	// Hacer entradas al puerto tres
	P3DIR = 0xFF;
	P3OUT = 0x00;
	#endif


	PlatformInit();


	#ifdef Sensor
	LastB = ReadB();
	__delay_cycles(70000 * 8 );
	LastB = ReadB();
	__delay_cycles(70000 * 8 );
	LastB = ReadB();
	__delay_cycles(70000 * 8 );
	#endif

	while (true)
	{
		__delay_cycles(700000 * 8 );

		#ifdef Sensor
		//LastB = ActualB;
		ActualB = ReadB();

		if( abs( ActualB - LastB ) >= 100 )
			P1OUT = BIT0;
		else P1OUT = 0;

		if( abs( ActualB - LastB ) >= 200 )
			P1OUT |= BIT6;
		else P1OUT = 0;

		int a = (int)ActualB;

		//char *intStr = itoa(a);

		//char buf[12];
		sprintf((char*)gPacket.payload, "%d\n\r", a);

		//string str = string(intStr);

		//unsigned char b = (a - 0) * (255 - 0) / (5000 - 0) + 0;

		//if( b == 0 ) continue;

	    TimerA_UART_init();                     // Start Timer_A UART
	    TimerA_UART_print((char*)gPacket.payload);

		WDTCTL = WDT_ADLY_16;
		IE1 |= WDTIE;             // Enable WDT interrupt
		__enable_interrupt();
		_BIS_SR(LPM3_bits + GIE); // Enter LPM3 w/interrupt
		#endif

		// Perform a simple transfer of the packet.
		if (!ProtocolSimpleTransfer((unsigned char*)&gPacket, sizeof(struct sPacket)))
		{
		  // Put the microcontroller into a low power state (sleep). Remain here
		  // until the ISR wakes up the processor.
		  McuSleep();
		}

		/**
		 *  Check if the protocol is busy. If it is, a new transfer cannot occur
		 *  until it becomes ready for the next instruction. Do not increment the
		 *  sequence number until the protocol is ready. This prevents incrementing
		 *  the sequence number more than once between transmissions.
		 */
		if (!ProtocolBusy())
		{
		  // Increment the sequence number for the next transmission.
		  gPacket.seqNum++;
		}
	}
}

/**
 *  GDO0Isr - GDO0 interrupt service routine. This service routine will always
 *  be a I/O interrupt service routine. Therefore, it is important to pass the
 *  port flag to the ProtocolEngine so that the protocol can determine if a GDO0
 *  event has occurred.
 */
#pragma vector=GDO0_VECTOR
__interrupt void GDO0Isr(void)
{
	if( P2IFG & BIT0 )
	{
		P2IFG &= ~BIT0;    // clear the interrupt flag
		// TA0R es el registro que tiene el conteo
		Time[i] = TA0R;
		TACTL = 0;
		TA0R  = 0;
		TACCTL0 = 0;
		_BIC_SR(LPM1_EXIT); // wake up from low power mode

		return;
	}


  /**
   *  Store the port interrupt flag register so that it may be used to determine
   *  what caused the interrupt (e.g. did a GDO0 interrupt occur? The protocol 
   *  needs this information to determine what to do next...).
   */
  register volatile unsigned char event = GDO0_EVENT;
  
  /**
   *  Notify the protocol of a port event. The protocol will determine if it is
   *  associated with the GDO0 pin and act accordingly (e.g. if GDO0 has not
   *  triggered an interrupt then the protocol will not run, otherwise it will
   *  continue where it left off).
   *
   *  Note: Clearing of the GDO0 event (interrupt flag bit) is handled 
   *  internally. It is important that the application does not clear the GDO0
   *  event in this ISR.
   */
  ProtocolEngine(event);

  // Wake up the microcontroller to continue normal operation upon exiting the
  // ISR.
  McuWakeup();
}

// Note: No hardware timer interrupt required for this example because the 
// End Point node does not perform half duplex transfers.



// Watchdog Timer interrupt service routine
#pragma vector=WDT_VECTOR
__interrupt void watchdog_timer(void)
{
	// 16ms * 125 = 4 seg
	if(wdtCounter == 124 / 10 )
	{
		//P1OUT ^= 0x01;                            // Toggle P1.0 using exclusive-OR
		//P1OUT = 0x00; // P1.0 turn off
		wdtCounter = 0;
		//_BIS_SR(LPM4_bits + GIE); // Enter LPM3 w/interrupt
		_BIC_SR(LPM3_EXIT); // wake up from low power mode
	}
	else ++wdtCounter;
}

/*#pragma vector=TIMER0_A0_VECTOR
//#pragma vector=(9 * 1u)
__interrupt void Timer_A(void)
{
	P1OUT ^= 0x01;// Toggle P1.0
	// no flag clearing necessary; CCR0 has only one source, so it's automatic.
} // CCR0_ISR
*/


// Port 1 interrupt service routine
#pragma vector=PORT1_VECTOR
__interrupt void Port_1(void)
{
/*	wdtCounter = 0; // reset watchdog timer counter
	P1OUT |= 0x01; // turn LED on
	P1IFG = 0x0;
	_BIC_SR(LPM3_EXIT); // wake up from low power mode

	WDTCTL = WDTPW + WDTHOLD;*/

	  /**
	   *  Store the port interrupt flag register so that it may be used to determine
	   *  what caused the interrupt (e.g. did a GDO0 interrupt occur? The protocol
	   *  needs this information to determine what to do next...).
	   */
	  register volatile unsigned char event = P1IFG;

	  /**
	   *  Notify the protocol of a port event. The protocol will determine if it is
	   *  associated with the GDO0 pin and act accordingly (e.g. if GDO0 has not
	   *  triggered an interrupt then the protocol will not run, otherwise it will
	   *  continue where it left off).
	   *
	   *  Note: Clearing of the GDO0 event (interrupt flag bit) is handled
	   *  internally. It is important that the application does not clear the GDO0
	   *  event in this ISR.
	   */
	  ProtocolEngine(event);

	  // Wake up the microcontroller to continue normal operation upon exiting the
	  // ISR.
	  McuWakeup();


	//P1IFG = 0x0;
}

// Port 2 interrupt service routine
/*#pragma vector=PORT2_VECTOR
__interrupt void P2_ISR(void)
{
	if( P2IFG & BIT0 )
	{
		P2IFG &= ~BIT0;    // clear the interrupt flag
		// TA0R es el registro que tiene el conteo
		Time[i] = TA0R;
		TACTL = 0;
		TA0R  = 0;
		TACCTL0 = 0;
		_BIC_SR(LPM1_EXIT); // wake up from low power mode
	}
} // P1_ISR

*/




//------------------------------------------------------------------------------
// Function configures Timer_A for full-duplex UART operation
//------------------------------------------------------------------------------
void TimerA_UART_init(void)
{
    TACCTL0 = OUT;                          // Set TXD Idle as Mark = '1'
    TACCTL1 = SCS + CM1 + CAP + CCIE;       // Sync, Neg Edge, Capture, Int
    TACTL = TASSEL_2 + MC_2;                // SMCLK, start in continuous mode
}
//------------------------------------------------------------------------------
// Outputs one byte using the Timer_A UART
//------------------------------------------------------------------------------
void TimerA_UART_tx(unsigned char byte)
{
    while (TACCTL0 & CCIE);                 // Ensure last char got TX'd
    TACCR0 = TAR;                           // Current state of TA counter
    TACCR0 += UART_TBIT;                    // One bit time till first bit
    TACCTL0 = OUTMOD0 + CCIE;               // Set TXD on EQU0, Int
    txData = byte;                          // Load global variable
    txData |= 0x100;                        // Add mark stop bit to TXData
    txData <<= 1;                           // Add space start bit
}

//------------------------------------------------------------------------------
// Prints a string over using the Timer_A UART
//------------------------------------------------------------------------------
void TimerA_UART_print(char *string)
{
    while (*string) {
        TimerA_UART_tx(*string++);
    }
}

//------------------------------------------------------------------------------
// Timer_A UART - Transmit Interrupt Handler
//------------------------------------------------------------------------------
#if defined(__TI_COMPILER_VERSION__) || defined(__IAR_SYSTEMS_ICC__)
#pragma vector = TIMER0_A0_VECTOR
__interrupt void Timer_A0_ISR(void)
#elif defined(__GNUC__)
void __attribute__ ((interrupt(TIMER0_A0_VECTOR))) Timer_A0_ISR (void)
#else
#error Compiler not supported!
#endif
{
    static unsigned char txBitCnt = 10;

    TACCR0 += UART_TBIT;                    // Add Offset to CCRx
    if (txBitCnt == 0) {                    // All bits TXed?
        TACCTL0 &= ~CCIE;                   // All bits TXed, disable interrupt
        txBitCnt = 10;                      // Re-load bit counter
    }
    else {
        if (txData & 0x01) {
          TACCTL0 &= ~OUTMOD2;              // TX Mark '1'
        }
        else {
          TACCTL0 |= OUTMOD2;               // TX Space '0'
        }
        txData >>= 1;
        txBitCnt--;
    }
}
//------------------------------------------------------------------------------
// Timer_A UART - Receive Interrupt Handler
//------------------------------------------------------------------------------
#if defined(__TI_COMPILER_VERSION__) || defined(__IAR_SYSTEMS_ICC__)
#pragma vector = TIMER0_A1_VECTOR
__interrupt void Timer_A1_ISR(void)
#elif defined(__GNUC__)
void __attribute__ ((interrupt(TIMER0_A1_VECTOR))) Timer_A1_ISR (void)
#else
#error Compiler not supported!
#endif
{
    static unsigned char rxBitCnt = 8;
    static unsigned char rxData = 0;

    switch (__even_in_range(TA0IV, TA0IV_TAIFG)) { // Use calculated branching
        case TA0IV_TACCR1:                        // TACCR1 CCIFG - UART RX
            TACCR1 += UART_TBIT;                 // Add Offset to CCRx
            if (TACCTL1 & CAP) {                 // Capture mode = start bit edge
                TACCTL1 &= ~CAP;                 // Switch capture to compare mode
                TACCR1 += UART_TBIT_DIV_2;       // Point CCRx to middle of D0
            }
            else {
                rxData >>= 1;
                if (TACCTL1 & SCCI) {            // Get bit waiting in receive latch
                    rxData |= 0x80;
                }
                rxBitCnt--;
                if (rxBitCnt == 0) {             // All bits RXed?
                    rxBuffer = rxData;           // Store in global variable
                    rxBitCnt = 8;                // Re-load bit counter
                    TACCTL1 |= CAP;              // Switch compare to capture mode
                    __bic_SR_register_on_exit(LPM0_bits);  // Clear LPM0 bits from 0(SR)
                }
            }
            break;
    }
}
//------------------------------------------------------------------------------
