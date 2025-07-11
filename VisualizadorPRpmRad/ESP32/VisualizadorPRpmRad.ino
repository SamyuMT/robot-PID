#include <Arduino.h>

#define C1 4      // Señal A del encoder
#define C2 5      // Señal B del encoder
#define ENA 6     // Pin PWM para velocidad
#define IN1 7     // Dirección motor
#define IN2 8     // Dirección motor

// --- Encoder ---
volatile int n = 0;
volatile byte ant = 0;
volatile byte act = 0;
double P = 0.0;

// --- RPM ---
double N = 0.0;
const int R = 1980; //Depende del datasheet  R = MH * s * r
unsigned long lastTime = 0;
unsigned long sampleTime = 100;

// --- Rad/s ---
const double constValue = (2* 3.1416 * 1000) / R;
double W = 0.0;

// --- Serial Communication ---
String inputString = "";
bool stringComplete = false;
const char separator = ',';
const int dataLength = 1;
double data[dataLength]; // Valor regula ciclo de trabajo (PWM)

// --- Control PWM ---
int cv = 0; // Duty Cycle

void setup() {
  Serial.begin(115200);

  // Pines encoder
  pinMode(C1, INPUT);
  pinMode(C2, INPUT);

  // Pines motor
  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, LOW);

  // PWM (nueva API core 3.x)
  ledcAttach(ENA, 5000, 8);  // ENA: 5 kHz, 8 bits
  ledcWrite(ENA, 0);         // Motor detenido al inicio

  attachInterrupt(C1, encoder, CHANGE);
  attachInterrupt(C2, encoder, CHANGE);

  Serial.println("Posición en grados, RPM, rad/s:");
}

void loop() {
  // Leer entrada serial
  while (Serial.available()) {
    char inChar = (char)Serial.read();
    inputString += inChar;
    if (inChar == '\n') {
      stringComplete = true;
    }
  }

  // Procesar entrada
  if (stringComplete) {
    for (int i = 0; i < dataLength ; i++) {
      int index = inputString.indexOf(separator);
      if (index == -1) {
        data[i] = inputString.toInt();
        break;
      }
      data[i] = inputString.substring(0, index).toInt();
      inputString = inputString.substring(index + 1);
    }
    cv = data[0];
    if (cv > 0) anticlockwise(IN2, IN1, ENA, cv);
    else clockwise(IN2, IN1, ENA, abs(cv));
    inputString = "";
    stringComplete = false;
  }

  // Calcular posición y RPM cada sampleTime ms
  if (millis() - lastTime >= sampleTime) {
    computePos();
    computeRpm();
    computeW();
    Serial.print("Posición: "); Serial.print(P);
    Serial.print(" | RPM: "); Serial.print(N);
    Serial.print(" | rad/s :"); Serial.println(W);
    lastTime = millis();
    n = 0; // Reset contador de pulsos
  }
}

// --- Cálculos ---
void computePos() {
  P = (n * 360.0) / R;
}

void computeRpm() {
  N = (n * 60.0 * 1000.0) / (sampleTime * R);
}

void computeW(){
  W = (constValue * n)/ (sampleTime);

}

// --- Control de motor ---
void clockwise(int pin1, int pin2, int pwmPin, int pwm) {
  digitalWrite(pin1, LOW);
  digitalWrite(pin2, HIGH);
  ledcWrite(pwmPin, pwm);
}

void anticlockwise(int pin1, int pin2, int pwmPin, int pwm) {
  digitalWrite(pin1, HIGH);
  digitalWrite(pin2, LOW);
  ledcWrite(pwmPin, pwm);
}

// --- Interrupción del encoder ---
void encoder() {
  ant = act;
  act = (digitalRead(C1) << 2) | (digitalRead(C2) << 3);

  if (ant == 0 && act == 4) n++;
  if (ant == 4 && act == 12) n++;
  if (ant == 8 && act == 0) n++;
  if (ant == 12 && act == 8) n++;

  if (ant == 0 && act == 8) n--;
  if (ant == 4 && act == 0) n--;
  if (ant == 8 && act == 12) n--;
  if (ant == 12 && act == 4) n--;
}
