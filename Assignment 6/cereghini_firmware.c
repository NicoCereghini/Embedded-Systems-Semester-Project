#include <avr/io.h>      /* Defines pins, ports, etc */
#include <util/delay.h>  /* Functions to waste time */
#include <util/setbaud.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <USART.h>

//Globals
int toggle = 0; //Used for changing start and stop
int ultraCount = 0; //Used for the ultrasonic sensor

//Prototypes
void stop();
void go();
void goSlow();
void goRegular();
void turnLeft();
void turnRight();
void goBackwards();
void goForward();
void toggleStartAndStop();
void ultraPing();

int main(void)
{
    
    char mystring[2] = ""; //Input string to store user input
    DDRB |= 0b11001011; //Enable motor and motor controller pins
    DDRC |= 0b00100000; //Enable trigger on ultrasonic sensor
    DDRC &= ~(1<<PC4); //Set echo pin to input
    PORTB &= ~(1 << PB0); //Clear is for clockwise motor turn, Set is CCW, each pin is setting the direction
    PORTB &= ~(1 << PB6); //Other motor direction
    PORTB |= (1 << PB7); //Set enable output
    
    //---------PWM Setup--------//
    //STEP 1
    //Set the correct mode
    TCCR1A |= (1 << WGM10); 
    TCCR1A &= ~(1 << WGM11);
    TCCR1B |= (1 << WGM12);
    TCCR1B &= ~(1 << WGM13);

    //STEP 2
    //Set frequency of PWM
    TCCR1B |= (1 << CS11);
    TCCR1B &= ~((1 << CS10) | (1 << CS12));

    //STEP 3 
    //Setting which pins are used and use clear on match + set at BOTTOM
    TCCR1A |= (1 << COM1A1);
    TCCR1A |= ~(1 << COM1A0);
    TCCR1A |= (1 << COM1B1);
    TCCR1A |= ~(1 << COM1B0);

    //-------- USART SET UP --------//
    UBRR0H = UBRRH_VALUE;
    UBRR0L = UBRRL_VALUE;

    #if USE_2X
        UCSR0A |= (1 << U2X0);
    #else
        UCSR0A &= ~(1 << U2X0);
    #endif
    
    //Enable USART transmitter / receiver
    UCSR0B = (1 << TXEN0) | (1 << RXEN0);
    //Set packet size (8 data bits):
    UCSR0C = (1 << UCSZ01) | (1 << UCSZ00);
    //Set stop bit amount (1 stop bit):
    UCSR0C &= ~(1 << USBS0);
    //-------- END USART SET UP --------//
    
    OCR1A = 255; //This is input to the motor clocks
    stop(); //Start the vehicle parked
    
    

    while(1)
    {
        //Ultrasonic sensor sample
        ultraPing();

        readString(mystring, 2); //Get UART input

        //handle keyboard inputs
        //Side Note: I would have used a switch case but that method was not working properly
        if(strcmp(mystring, "w") == 0)
            goForward();
        else
            if(strcmp(mystring, "a") == 0)
                turnLeft();
            else
                if(strcmp(mystring, "s") == 0)
                    goBackwards();
                else
                    if(strcmp(mystring, "d") == 0)
                        turnRight();
                    else
                        if(strcmp(mystring, " ") == 0)
                            //Toggle start/stop
                            {
                                toggleStartAndStop();
                                if(toggle == 1)
                                    printString("Driving");
                                else
                                    printString("Stopped");
                            }
                        else //Change the speeds if the user presses c or z
                            if(strcmp(mystring, "z") == 0)
                                goFast();
                            else
                                if(strcmp(mystring, "c") == 0)
                                    goSlow();
        //Show the user what they pressed and format the print
        printString("\n\r");
        printString(mystring);
        printString("\r");
    }
}

void stop()
{
    PORTB &= ~(1 << PB7); //clear enable output
}

void go()
{
    //Operates wheels to last set speed
    PORTB |= (1 << PB7); //Set enable output
}

void goSlow()
{
    //Set frequency of PWM to a lower clock prescaler
    TCCR1B &= ~(1 << CS12);
    TCCR1B |= ((1 << CS10) | (1 << CS11));
}

void goFast()
{
    //Set frequency of PWM to a larger clock prescaler
    TCCR1B |= (1 << CS11);
    TCCR1B &= ~((1 << CS10) | (1 << CS12));
}

void turnRight()
{
    PORTB &= ~(1 << PB0); //Right motor direciton CW
    PORTB |= (1 << PB6); //Left motor direction CCW
}

void turnLeft()
{
    PORTB |= (1 << PB0); //Right motor direciton CCW
    PORTB &= ~(1 << PB6); //Left motor direction CW
}

void goBackwards()
{
    
    PORTB |= (1 << PB0); //Reverse Motors
    PORTB |= (1 << PB6);
}

void goForward()
{
    //Set motors to the Clockwise direction
    PORTB &= ~(1 << PB0); 
    PORTB &= ~(1 << PB6); 
}

void toggleStartAndStop()
{
    //Simple logic for handling the spacebar press for stop and start
    if(toggle == 1)
    {
        stop();
        toggle = 0;
    }
    else
        {
            go();
            toggle = 1;
        }
}

void ultraPing()
{   
    ultraCount = 0;
    //Send a pulse from the ultrasonic sensor sensor
    PORTC &= ~(1 << PC5);
    _delay_us(3);
    PORTC |= (1 << PC5);
    _delay_us(15);
    PORTC &= ~(1 << PC5);
    _delay_us(3);

    while((PINC & (0b00010000))!=0) //busy loop until pulse returns
        ultraCount++;
        
        if(ultraCount < 5000)
        {
            printWord(ultraCount);
            //Next Step: if you are too close to something, the bot will turn 180 before continuing
        }
}