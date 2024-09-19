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
#include <Adafruit_LiquidCrystal.h>
Adafruit_LiquidCrystal lcd1(0);
int val = 0;
bool inicio1 = true;
bool inicio2 = false;
bool seno = false;
bool cuadrada = false;
bool triangulada = false;
const int analogPin = 0;
const int botonInicioPin = 2;
const int botonMostrarPin = 4;
float voltage = 0;
float maxVoltage = 0;
float minVoltage = 5.0;
float Amplitud = 0;

void setup()
{
    Serial.begin(9600);
    lcd1.begin(16, 2);
    lcd1.setCursor(0, 0);
    pinMode(botonInicioPin, INPUT_PULLUP);
    pinMode(botonMostrarPin, INPUT_PULLUP);
}

void loop()
{
    while (inicio1)
    {
        lcd1.print("Esperando...");
        delay(10);
        lcd1.clear();
        if(digitalRead(botonInicioPin) == HIGH)
        {
            delay(10);
            inicio1 = false;
            lcd1.clear();
        }
    }
    while(!inicio2)
    {
        lcd1.clear();
        lcd1.print("Leyendo onda...");
        val = analogRead(analogPin);
        voltage = val * (5.0 / 1023.0);
        Serial.println(voltage);
        if (val > maxVoltage)
        {
            maxVoltage = val;
        }
        if (val < minVoltage)
        {
            minVoltage = voltage;
        }
        Amplitud = (maxVoltage - minVoltage)/2.0;
        if(minVoltage*-1.0 == maxVoltage)
        {
            cuadrada = true;
        }
        if(digitalRead(botonMostrarPin) == HIGH)
        {
            inicio2 = true;
            lcd1.clear();
        }
    }
    if (inicio2)
    {
        lcd1.clear();
        if (cuadrada && !seno && !triangulada)
        {
            lcd1.print("Onda cuadrada. ");
            lcd1.setCursor(0, 1);
            lcd1.print(maxVoltage);
            lcd1.print(", ");
            lcd1.print(minVoltage);
        }
        if (!cuadrada && seno && !triangulada)
        {
            lcd1.print("Onda senoidal. ");
            lcd1.setCursor(0, 1);
            lcd1.print(maxVoltage);
        }
        if (!cuadrada && !seno && triangulada)
        {
            lcd1.print("Onda Triangulada. ");
            lcd1.setCursor(0, 1);
            lcd1.print(maxVoltage);
        }
        lcd1.print(Amplitud);
        delay(10000); //aumentar el tiempo para mostrar todos los datos y dar tiempo a leer
        inicio2 = false;
    }
}
