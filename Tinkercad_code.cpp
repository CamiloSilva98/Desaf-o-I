#include <Adafruit_LiquidCrystal.h>
Adafruit_LiquidCrystal lcd1(0);
int val = 0;
bool inicio1 = true;
bool inicio2 = false;
const int analogPin = 0;
const int botonInicioPin = 2;
const int botonMostrarPin = 4;
float voltage = 0;
float maxVoltage = 0;
float minVoltage = 5.0;
const int ledPin = 13;

void setup()
{
    pinMode(ledPin, OUTPUT);
    digitalWrite(ledPin, LOW);
    Serial.begin(9600);
    lcd1.begin(16, 2);
    lcd1.setCursor(0, 0);
    pinMode(botonInicioPin, INPUT_PULLUP);
    pinMode(botonMostrarPin, INPUT_PULLUP);
}

void loop()
{
    bool botonInicioEstado = digitalRead(botonInicioPin) == HIGH;
    bool botonMostrarEstado = digitalRead(botonMostrarPin) == HIGH;

    while (inicio1)
    {
        lcd1.print("Esperando...");
        delay(50);
        lcd1.clear();
        if(digitalRead(botonInicioPin) == HIGH)
        {
            inicio1 = false;
            lcd1.clear();
        }
    }
    while(!inicio2)
    {
        digitalWrite(ledPin, HIGH);
        lcd1.clear();
        lcd1.print("Leyendo onda...");
        val = analogRead(analogPin);
        Serial.println(val);
        if(digitalRead(botonMostrarPin) == HIGH)
        {
            inicio2 = true;
            lcd1.clear();
        }
    }
    if (inicio2)
    {
        digitalWrite(ledPin, LOW);
        lcd1.clear();
        lcd1.print("info");
        delay(15000);
        inicio2 = false;
    }
}
