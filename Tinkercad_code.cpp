#include <Adafruit_LiquidCrystal.h>
#include <avr/io.h> // Para manipular registros internos y aumentar la frecuencia de muestreo

Adafruit_LiquidCrystal lcd1(0);

// Pines y variables de estado
const int analogPin = A0;
const int botonInicioPin = 2;
const int botonMostrarPin = 4;

float *onda = NULL;  // Buffer dinámico para almacenar las muestras
int totalMuestras = 100; // Capacidad del buffer circular
int indice = 0;  // Índice para el buffer circular

// Variables para el procesamiento
float maxVoltage = 0;
float minVoltage = 5.0;
float Amplitud = 0;
float Hertz = 0;

// Variables para detección de cruce por cero
unsigned long tiempoAnterior = 0;
bool cruceDetectado = false;

// Banderas para identificar la forma de onda
bool seno = false;
bool cuadrada = false;
bool triangulada = false;
bool desconocida = false;

// Variables de control
bool inicio1 = true;
bool inicio2 = false;  // Controla si el programa debe mostrar o capturar datos

void setup() {
    Serial.begin(9600);
    lcd1.begin(16, 2);
    pinMode(botonInicioPin, INPUT_PULLUP);
    pinMode(botonMostrarPin, INPUT_PULLUP);

    // Asignar memoria inicial para el buffer circular
    onda = (float*)malloc(totalMuestras * sizeof(float));
    if (onda == NULL) {
        lcd1.print("Error memoria!");
        while (1);
    }

    // Aumentar la frecuencia de muestreo ajustando el prescaler del ADC
    // El prescaler del ADC está por defecto en 128, lo que da una velocidad de muestreo de 9600 Hz
    // Vamos a reducirlo para aumentar la velocidad
    ADCSRA &= ~((1 << ADPS2) | (1 << ADPS1)); // Ajustamos el prescaler a 16
    ADCSRA |= (1 << ADPS0); // Frecuencia de muestreo más rápida (~ 76.9 kHz)
}

void loop() {
    while (inicio1) {
        lcd1.print("Esperando...");
        delay(10);
        lcd1.clear();
        if (digitalRead(botonInicioPin) == HIGH) {
            delay(10);
            inicio1 = false;
            lcd1.clear();
            lcd1.print("Leyendo onda...");
        }
    }

    while (!inicio2) {
        // Leer el valor de la señal analógica (ajustada para señales hasta 10V con divisor de voltaje)
        float voltage = analogRead(analogPin) * (10.0 / 1023.0); // Multiplicamos por 10 debido al divisor de voltaje

        // Guardar el valor en el buffer circular
        onda[indice] = voltage;
        indice = (indice + 1) % totalMuestras; // Mantener el índice dentro de los límites del buffer

        // Actualizar voltajes máximo y mínimo
        if (voltage > maxVoltage) maxVoltage = voltage;
        if (voltage < minVoltage) minVoltage = voltage;

        // Detección de cruce por cero para identificar el ciclo
        float nivelMedio = (maxVoltage + minVoltage) / 2.0;
        if (voltage >= nivelMedio && !cruceDetectado) {
            cruceDetectado = true;
            if (tiempoAnterior > 0) {
                unsigned long periodo = micros() - tiempoAnterior;
                Hertz = 1000000.0 / periodo; // Calcular frecuencia en Hz
            }
            tiempoAnterior = micros();
        } else if (voltage < nivelMedio) {
            cruceDetectado = false;
        }

        // Verificar si se presiona el botón para mostrar resultados
        if (digitalRead(botonMostrarPin) == HIGH) {
            inicio2 = true;
        }
    }

    // Mostrar resultados y reiniciar captura
    if (inicio2) {
        lcd1.clear();
        lcd1.print("Mostrando...");

        // Procesar los datos almacenados en el buffer circular
        Amplitud = (maxVoltage - minVoltage) / 2.0;

        // Detectar el tipo de onda
        detectarFormaOnda();

        // Mostrar los resultados en la pantalla
        lcd1.clear();
        if (cuadrada) {
            lcd1.print("Onda cuadrada");
        } else if (seno) {
            lcd1.print("Onda senoidal");
        } else if (triangulada) {
            lcd1.print("Onda triangular");
        } else {
            lcd1.print("Señal desconocida");
        }
        lcd1.setCursor(0, 1);
        lcd1.print(Amplitud, 2);
        lcd1.print("V ");
        lcd1.print(Hertz, 2);
        lcd1.print(" Hz");

        delay(3000);  // Mostrar la información durante 3 segundos

        // Reiniciar para capturar nuevamente
        lcd1.clear();
        lcd1.print("Leyendo onda...");
        inicio2 = false;
        seno = false;
        cuadrada = false;
        triangulada = false;
        maxVoltage = 0;
        minVoltage = 5.0;
        Amplitud = 0;
        Hertz = 0;
        tiempoAnterior = 0;
    }
}

// Función para detectar la forma de la onda
void detectarFormaOnda() {
    int numCambiosLineales = 0;
    int numCambiosSuaves = 0;
    
    // Definir variables para contar los valores únicos
    float valorMaximo = -1000.0;
    float valorMinimo = 1000.0;
    int valoresDiferentes = 0;

    // Ajuste de umbrales
    float umbralRuido = 0.05 * (maxVoltage - minVoltage); // 5% del rango de la señal
    float cambioPromedio = (maxVoltage - minVoltage) / totalMuestras; // Cambio ideal para onda triangular
    float umbralCambioTriangular = cambioPromedio * 0.4; // Mayor margen para detectar cambios lineales
    float umbralCambioSenoidal = cambioPromedio * 2.0;   // Permitir cambios más suaves para onda senoidal

    // Recorrer el buffer de muestras para detectar cuántos valores diferentes hay
    for (int i = 1; i < totalMuestras; i++) {
        float voltage = onda[i];

        // Identificar valores diferentes: actualizar max y min si se detectan valores fuera del umbral
        if (abs(voltage - valorMaximo) > umbralRuido && abs(voltage - valorMinimo) > umbralRuido) {
            valoresDiferentes++;
            if (voltage > valorMaximo) {
                valorMaximo = voltage;
            } else if (voltage < valorMinimo) {
                valorMinimo = voltage;
            }
        }

        // Detectar si el cambio es lineal (para onda triangular)
        float delta = onda[i] - onda[i - 1];
        if (abs(delta - cambioPromedio) < umbralCambioTriangular) {
            numCambiosLineales++;
        }
        // Detectar si el cambio es suave (para onda senoidal)
        else if (abs(delta) < umbralCambioSenoidal && abs(delta) > umbralRuido) {
            numCambiosSuaves++;
        }
    }

    // Detectar la onda cuadrada: solo debería haber 2 valores dominantes
    if (valoresDiferentes <= 2) {
        cuadrada = true;
        seno = false;
        triangulada = false;
        desconocida = false;
    }
    // Detectar la onda triangular: la mayoría de los cambios son casi constantes
    else if (numCambiosLineales > totalMuestras * 0.7) {
        triangulada = true;
        seno = false;
        cuadrada = false;
        desconocida = false;
    }
    // Detectar la onda senoidal: mayoría de los cambios suaves y continuos
    else if (numCambiosSuaves > totalMuestras * 0.7) {
        seno = true;
        cuadrada = false;
        triangulada = false;
        desconocida = false;
    }
    // Si no cumple ninguna de las condiciones anteriores, es señal desconocida
    else {
        desconocida = true;
        seno = false;
        cuadrada = false;
        triangulada = false;
    }
}
    Amplitud = 0;
    prevVal = 0;
  }
}
