/**
 *  ----------------------------------------------------------------------------
 *  Copyright (c) 2012-13, Anaren Microwave, Inc.
 *
 *  For more information on licensing, please see Anaren Microwave, Inc's
 *  end user software licensing agreement: EULA.txt
 *
 *  ----------------------------------------------------------------------------
 *
 *  SimplexTransfer.c - acts as the Gateway node for the SimplexTransfer
 *  example. Receives packets from the Simplex End Point node(s) and stores it
 *  in a local packet.
 *
 *  @version    1.0.00
 *  @date       04 Feb 2013
 *  @author     BPB, air@anaren.com
 *
 *  assumptions
 *  ===========
 *  - this is being compiled exclusively with Gateway node(s).
 *  
 *  file dependency
 *  ===============
 *  string.h : defines memcpy which is used to copy one buffer to another
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

#include <string.h>       // memcpy
#include "API.h"

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
#define McuSleep()    _BIS_SR(LPM4_bits | GIE)  // Low power mode 4
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
 *  Callback function prototypes
 */

/**
 *  TransferComplete - acts as the callback function for the Protocol Data
 *  Transfer Complete event. When a frame is received that meets addressing
 *  requirements, the Protocol Data Transfer Complete event is triggered and a
 *  callback must be implemented to receive the frame payload (packet).
 *
 *  Note: Please refer to API.h for more information on the TransferComplete
 *  callback for a Gateway node.
 */
unsigned char TransferComplete(bool dataRequest,
                               unsigned char *data,
                               unsigned char length);

// -----------------------------------------------------------------------------
/**
 *  Global data
 */

/**
 *  The following instance of sProtocolSetupInfo is used to initialize the
 *  protocol with required parameters. The Gateway node role has the following
 *  parameters [optional parameters are marked with an asterix (*)].
 *        
 *  { channel_list, pan_id, local_address, LinkRequest(*), TransferComplete(*) }
 *
 *  Note: Parameters marked with the asterix (*) may be assigned "NULL" if they
 *  are not needed.
 */
static const struct sProtocolSetupInfo gProtocolSetupInfo = {
  { PROTOCOL_CHANNEL_LIST },// Physical channel list
  { 0x01 },                 // Physical address PAN identifier
  { 0x01 },                 // Physical address
  NULL,                     // Protocol Link Request callback (not used)
  TransferComplete          // Protocol Data Transfer Complete callback
};

static struct sPacket gPacket = {
  0x00,                     // Set the initial sequence number value to 0
  ""                        // Set the initial payload to an empty string
};


////////////////////////////////////////////////////////////////////////////////
/*
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
*/

//------------------------------------------------------------------------------
// Hardware-related definitions
//------------------------------------------------------------------------------
#define UART_TXD   BIT1                     // TXD on P1.1 (Timer0_A.OUT0)
#define UART_RXD   BIT2                     // RXD on P1.2 (Timer0_A.CCI1A)

//------------------------------------------------------------------------------
// Conditions for 9600 Baud SW UART, SMCLK = 1MHz
//------------------------------------------------------------------------------
#define UART_TBIT           (8000000 / 9600)


//------------------------------------------------------------------------------
// Global variables used for full-duplex UART communication
//------------------------------------------------------------------------------
unsigned int txData;                        // UART internal variable for TX
unsigned char rxBuffer;                     // Received UART character
unsigned int i = 0;

//------------------------------------------------------------------------------
// Function prototypes
//------------------------------------------------------------------------------
/*void TimerA_UART_init(void);
void TimerA_UART_tx(unsigned char byte);
void TimerA_UART_print(char *string);
*/
void UARTInit(void);

////////////////////////////////////////////////////////////////////////////////

// -----------------------------------------------------------------------------

int a, b, c;

unsigned char TransferComplete(bool dataRequest,
                               unsigned char *data,
                               unsigned char length)
{
  // Cast the received data pointer to a packet structure pointer so that it may
  // be accessed using the structure member notation.
  struct sPacket *p = (struct sPacket*)data;
  
  // Retrieve the sequence number and copy the payload from the protocol into
  // the local application packet (gPacket).
  gPacket.seqNum = p->seqNum;
  memcpy(gPacket.payload, p->payload, length-1);
  
  /*TimerA_UART_init();                     // Start Timer_A UART
  TimerA_UART_print((char*)p->payload);
*/

  i = 5;
  IE2 |= UCA0TXIE;                        // Enable USCI_A0 TX interrupt

  if ( gPacket.payload[5] == '2' )
  {
	  a++;
  }
  else if ( gPacket.payload[5] == '3' )
  {
	  b++;
  }
  else if ( gPacket.payload[5] == '4' )
  {
	  c++;
  }


  __enable_interrupt();

  return 0;
}

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
 *  main - main application loop. Sets up platform and then goes to sleep for
 *  the lifetime of execution.
 *
 *    @return   Exit code of the main application, however, this application
 *              should never exit.
 */
int main(void)
{
  // Setup hardware and protocol.
  PlatformInit();

  UARTInit();

  /**
   *  For this example, all operations are performed inside the protocol ISR
   *  (ProtocolEngine) and callback function (TransferComplete). The protocol 
   *  puts the Gateway node into a receive state by default. The application 
   *  main loop can simply sleep.
   */
  while (true)
  {
    // Put the microcontroller into a low power state (sleep).
    McuSleep();
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
}

// Note: No hardware timer interrupt required for this example because the 
// Gateway node does not use it for anything at this time.




/*
//------------------------------------------------------------------------------
// Function configures Timer_A for full-duplex UART operation
//------------------------------------------------------------------------------
void TimerA_UART_init(void)
{
    P1SEL |= UART_TXD + UART_RXD;            // Timer function for TXD/RXD pins
    P1DIR &= ~UART_RXD;               // Set all pins but RXD to output


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
*/

void UARTInit(void)
{
	P1DIR &= ~UART_RXD;                 // Set all pins but RXD to output

	P1SEL  |= UART_TXD + UART_RXD;            // P1.1 = RXD, P1.2 = TXD
	P1SEL2 |= UART_TXD + UART_RXD;            // P1.1 = RXD, P1.2 = TXD

	UCA0CTL1 |= UCSSEL_2;                     // SMCLK
	UCA0BR0 = UART_TBIT & 0xFF;               // 1MHz 9600
	UCA0BR1 = ( UART_TBIT & 0xFF00 ) >> 8;    // 1MHz 9600

	UCA0MCTL = UCBRS1 + UCBRS0;               // Modulation UCBRSx = 1
	UCA0CTL1 &= ~UCSWRST;                     // **Initialize USCI state machine**

	IE2 |= UCA0RXIE;                          // Enable USCI_A0 RX interrupt

	__enable_interrupt();
}

// USCI A0/B0 Transmit ISR
#pragma vector=USCIAB0TX_VECTOR
__interrupt void USCI0TX_ISR(void)
{
  UCA0TXBUF = gPacket.payload[i++];              // TX next character

//  if (i == sizeof gPacket.payload -1)                 // TX over?
  if (i >= 6)                 // TX over?
    IE2 &= ~UCA0TXIE;                     // Disable USCI_A0 TX interrupt

/*
	UCA0TXBUF = buf[i++];                 // TX next character

	if (i == sizeof buf - 1)              // TX over?
		IE2 &= ~UCA0TXIE;                 // Disable USCI_A0 TX interrupt
*/
}

// USCI A0/B0 Receive ISR
#pragma vector=USCIAB0RX_VECTOR
__interrupt void USCI0RX_ISR(void)
{
}
