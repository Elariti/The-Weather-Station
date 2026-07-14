#include <SoftwareSerial.h>
#include <LiquidCrystal.h>
#include "dht.h"
#include <avr/io.h>
#include <avr/interrupt.h>


// ---------------- Pin Definitions ----------------

#define DHT_PIN A1
#define TILT_PIN 2        // PD2 / INT0


// LCD pins
// RS, EN, D4, D5, D6, D7
// D7 moved from pin 2 to pin 6
LiquidCrystal lcd(12, 11, 5, 4, 3, 6);


// Bluetooth HC-05
SoftwareSerial bt(9,10);


// DHT object
dht DHT;


// ---------------- Global Variables ----------------

volatile bool earthquakeDetected = false;

unsigned long lastInterrupt = 0;

int temperature = 0;
int humidity = 0;

float dewPoint = 0;



// =================================================
//                    SETUP
// =================================================

void setup()
{

    Serial.begin(9600);

    bt.begin(9600);


    lcd.begin(16,2);

    lcd.setCursor(0,0);
    lcd.print("Earthquake");

    lcd.setCursor(0,1);
    lcd.print("Monitoring");

    delay(2000);

    lcd.clear();



    /*
       Register Level INT0 Configuration

       INT0 = PD2 = Arduino Pin 2

       Falling edge:
       ISC01 = 1
       ISC00 = 0
    */


    // PD2 as input
    DDRD &= ~(1 << DDD2);


    // Enable internal pull-up
    PORTD |= (1 << PORTD2);



    // Falling edge trigger
    EICRA |= (1 << ISC01);
    EICRA &= ~(1 << ISC00);



    // Enable INT0 interrupt
    EIMSK |= (1 << INT0);



    // Enable global interrupts
    sei();


    Serial.println("System Ready");

}



// =================================================
//                    LOOP
// =================================================

void loop()
{


    int result = DHT.read11(DHT_PIN);


    if(result == DHTLIB_OK)
    {

        temperature = DHT.temperature;

        humidity = DHT.humidity;


        // Simple dew point approximation
        dewPoint = temperature - ((100 - humidity) / 5.0);



        // -------- LCD Display --------


        lcd.setCursor(0,0);

        lcd.print("Hum:");

        lcd.print(humidity);

        lcd.print("%   ");



        lcd.setCursor(0,1);

        lcd.print("Tmp:");

        lcd.print(temperature);

        lcd.write(223);

        lcd.print("C   ");





        // -------- Bluetooth --------


        bt.print("Temperature:");

        bt.print(temperature);

        bt.print("C,");


        bt.print("Humidity:");

        bt.print(humidity);

        bt.print("%,");


        bt.print("Dew:");

        bt.println(dewPoint);





        // -------- Serial Monitor --------


        Serial.print("Temperature: ");

        Serial.print(temperature);

        Serial.print(" C   ");


        Serial.print("Humidity: ");

        Serial.print(humidity);

        Serial.print(" %   ");


        Serial.print("Dew Point: ");

        Serial.println(dewPoint);


    }

    else
    {

        Serial.println("DHT11 Reading Failed");

    }





    // Check interrupt flag

    if(earthquakeDetected)
    {

        earthquakeDetected = false;


        showEarthquakeWarning();

    }



    delay(1000);

}



// =================================================
//              INT0 INTERRUPT SERVICE ROUTINE
// =================================================

ISR(INT0_vect)
{

    unsigned long now = millis();



    // Software debounce
    if(now - lastInterrupt > 200)
    {

        earthquakeDetected = true;

        lastInterrupt = now;

    }

}



// =================================================
//              EARTHQUAKE WARNING
// =================================================

void showEarthquakeWarning()
{

    lcd.clear();


    lcd.setCursor(0,0);

    lcd.print("EARTHQUAKE");


    lcd.setCursor(0,1);

    lcd.print("WARNING!");



    bt.println("Earthquake Warning!");

    Serial.println("Earthquake Detected!");



    delay(3000);



    lcd.clear();

}
