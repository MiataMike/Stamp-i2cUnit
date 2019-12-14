#pragma config FCKSM = CSDCMD // Clock Switching and Monitor disabled
#pragma config FNOSC = 1    // Internal fast oscillator with PLL
#pragma config POSCMD = 3
#pragma config PLLDIV = NODIV   // Prescaler divide not sure how this works
#pragma config IESO = ON      // Int Ext Switch Over Mode enabled
//#pragma config I2C1SEL = 0  // I2C unit 2 on F4 and F5 pins
// CONFIG1
#pragma config ICS = PGx1      // Emulator/debugger uses EMUC2/EMUD2
#pragma config GWRP = OFF      // Writes to program memory allowed
#pragma config GCP = OFF       // Code protection is disabled
#pragma config JTAGEN = ON    // JTAG port is enabled
#pragma config FWDTEN = 0   // Watchdog Timer disabled


// here are some defines that make writing messages on i2c easier
// use them like sendbuffer = (s_address | read)
//address byte
#define s_address 0b01011110
#define write 0
#define read 1
//command byte
#define channel1 0b00000000
#define channel2 0b10000000
#define shutdown 0b01000000

// #pragma config statements should precede project file includes.
// Use project enums instead of #define for ON and OFF.

#include <xc.h>

void initI2C(void);


int main(void) 
{ 
    initI2C();
    PR1 = 16374; // Timer 1 terminal value
    //T1CON = 0x8030; // Timer 1 on - pre-scale divide by 256
    T1CONbits.TCKPS = 0b10; //prescale to 64
    T1CONbits.TON = 1; //timer on
   
    char i=0;
    while (1)
    {
        if (IFS0bits.T1IF) // Test the terminal count interrupt
        {
            IFS0bits.T1IF = 0; // Clear the interrupt flag bit
            I2C1STATbits.ACKSTAT = 0;
            I2C1CONbits.SEN = 1; // send a start bit
            while (I2C1CONbits.SEN); // wait until the start bit is complete.
            I2C1TRN = (s_address | write); // load the transmit register with 
                                           //address write.
            while (I2C1STATbits.TRSTAT); // wait for transmit complete
            I2C1TRN = (channel1); //command byte, select channel 1
            while (I2C1STATbits.TRSTAT); // wait for transmit complete
            I2C1TRN = (i);
            I2C1CONbits.PEN = 1;    // send stop bit
            while (I2C1CONbits.PEN);
            
            i++; //increment counter for setting resistance 
            i %= 255; // 8 bit register in the chip, reset when it gets too high
        }

    }
    return 0;
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
    I2C1BRG = 0x9d; // set the baud rate at 100kHz for 16Mhz FCY.
    ///I2C1CON |= 0x0003;
    I2C1CONbits.I2CEN = 1;

}


