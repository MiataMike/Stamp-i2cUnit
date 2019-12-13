#pragma config FCKSM = CSDCMD // Clock Switching and Monitor disabled
#pragma config FNOSC = 1    // Internal fast oscillator with PLL
#pragma config POSCMD = 3
#pragma config PLLDIV = NODIV   // Prescaler divide not sure how this works
#pragma config IESO = ON      // Int Ext Switch Over Mode enabled
#pragma config I2C2SEL = 0  // I2C unit 2 on F4 and F5 pins
// CONFIG1
#pragma config ICS = PGx3      // Emulator/debugger uses EMUC2/EMUD2
#pragma config GWRP = OFF      // Writes to program memory allowed
#pragma config GCP = OFF       // Code protection is disabled
#pragma config JTAGEN = ON    // JTAG port is enabled
#pragma config FWDTEN = 0   // Watchdog Timer disabled


// #pragma config statements should precede project file includes.
// Use project enums instead of #define for ON and OFF.

#include <xc.h>
#include <math.h>

void initUART( void); // init the serial port (UART2, 9600@32MHz, 8, N, 1, CTS/RTS )
void putcharUART( char c); // send a character to the serial port
void putstringUART( char *); // send a null terminated string to the serial port
void initI2C(void);
void pnUnlk(void);
void pnLk(void);
// useful macros
//#define clrscr() putstringUART( "\x1b[2J") 
//#define home()   putstringUART( "\x1b[H") 
//#define pcr()    putcharUART( '\r')

int cnt = 0;
int main(void) 
{ 
    initI2C();
    //TRISEbits.TRISE7 = 0; // set all LED1 bits to be output
    //TRISBbits.TRISB6 = 0; // set all LED2 bits to be output
    //ANSBbits.ANSB6 = 0; // Make the pin Digital
    //ANSEbits.ANSE7 = 0; // Make the pin Digital
    PR1 = 16374; // Timer 1 terminal value
    T1CON = 0x8030; // Timer 1 on - pre-scale divide by 256
   
    
    int i=0;
    while (1)
    {
        if (TMR1 < 8139) LATEbits.LATE7 = 1; // LED2 On for first half
        else LATEbits.LATE7 = 0; //LED2 off for 2nd half.
        if (IFS0bits.T1IF) // Test the terminal count interrupt
        {
            LATBbits.LATB6 = !PORTBbits.RB6; // toggle the LED
            IFS0bits.T1IF = 0; // Clear the interrupt flag bit

            I2C2STATbits.ACKSTAT = 0;
            I2C2CONbits.SEN = 1;
            for(i=0; i<50; i++)
            {
                Nop();
            }
            I2C2TRN = 0x75;
            for(i=0; i<400; i++)
            {
                Nop();
            }
            I2C2CONbits.PEN = 1;
            Nop();
        }
        
        if ((TMR1 % 20) == 0) {cnt++; cnt%=360;} // set the dac frequency
        DAC1DAT = (int)(512.0*(1+sin(cnt*6.28/360))); // sinwave to DAC output
    

    }
    return 0;
}
void initUART( void) // init the serial port (UART2, 9600@32MHz, 8, N, 1, CTS/RTS )
{
__builtin_write_OSCCONL(OSCCON & 0xbf);
RPOR0bits.RP0R = 3;  // TX pin U1TX
//RPINR19bits.U2RXR = 9;  // RX pin U2RX
__builtin_write_OSCCONL(OSCCON | 0x40);
    ANSBbits.ANSB0 = 0; // RB0 is a digital pin
    TRISBbits.TRISB0 = 0; // RB0 is an output pin
    U1MODEbits.STSEL = 0; // one stop bit only 
    U1MODEbits.ABAUD = 0; // Auto-Baud disabled - requires header
    U1MODEbits.UEN = 0; //RTS and CTS controlled by pins   
    U1MODEbits.UARTEN = 1; // Enable UART
    U1BRG = 103; // for a baud rate of 9600
    U1STAbits.URXISEL = 0; // receive buffer
    U1STAbits.UTXEN = 1; // Enable UART TX    
}
/*I2C initialization.
 */
void initI2C(void)
{
    ANSFbits.ANSF4 = 0;
    TRISFbits.TRISF4 = 0;
    //ODCFbits.ODF4 = 1;
    ANSFbits.ANSF5 = 0;
    TRISFbits.TRISF5 = 0;
    //ODCFbits.ODF5 = 1;
    I2C2BRG = 0x9d; // set the baud rate at 100kHz for 16Mhz FCY.
    ///I2C2CON |= 0x0003;
    I2C2CONbits.I2CEN = 1;

}
/* this program will put a single out the UART transmit pin.
 It first checks the U1STAbit.UTXBF (Uart 1 status Buffer Full
 When this is 0 indicating there is room for at least one more
 character, the requested character is put into U1TXREG.*/
void putcharUART( char c) // send a character to the serial port
{
	while ( U1STAbits.UTXBF == 1);   // wait while Tx buffer full
	U1TXREG = c; // add character to buffer.
}
/* this routine will call putcharUART to send a string of characters
 one at a time.  It is called with s, a pointer to a string. It
 sends the characters 1 at a time until *s is null.  Then it sends
 a final cr character.*/
void putstringUART( char *s) // send a null terminated string to the serial port
{
    while( *s)			// loop until *s == '\0' end of the string
 	putcharUART( *s++);	// send the character and point to the next one
    putcharUART( '\r'); // terminate with a cr / line feed
}
void pnUnlk()
{
    asm volatile ("MOV #OSCCON, w1 \n"
                  "MOV #0x46, w2 \n"
                  "MOV #0x57, w3 \n"
                  "MOV.b w2, [w1] \n"
                  "MOV.b w3, [w1] \n"
                  "BCLR OSCCON, #6") ;
}
void pnLk()
{
    asm volatile ("MOV #OSCCON, w1 \n"
                  "MOV #0x46, w2 \n"
                  "MOV #0x57, w3 \n"
                  "MOV.b w2, [w1] \n"
                  "MOV.b w3, [w1] \n"
                  "BSET OSCCON, #6") ;
}