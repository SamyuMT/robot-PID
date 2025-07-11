/*
  ENGLISH:
  This code counts quadrature encoder pulses using interrupts and prints the count every 100 ms.
  It is designed for microcontrollers and uses two digital inputs for the encoder signals.

  ESPAÑOL:
  Este código cuenta los pulsos de un encoder de cuadratura usando interrupciones y muestra el conteo cada 100 ms.
  Está diseñado para microcontroladores y utiliza dos entradas digitales para las señales del encoder.
*/

// Encoder signal A input pin
const int    C1 = 3; 
// Encoder signal B input pin
const int    C2 = 2; 

// Pulse count variable
volatile int  n    = 0;
// Previous encoder state
volatile byte ant  = 0;
// Current encoder state
volatile byte act  = 0;

// Previous sample time
unsigned long lastTime = 0;  
// Sampling interval in milliseconds
unsigned long sampleTime = 100;  

void setup()
{
  Serial.begin(9600); // Initialize serial communication

  pinMode(C1, INPUT); // Set encoder A pin as input
  pinMode(C2, INPUT); // Set encoder B pin as input

  // Attach interrupts for encoder signals
  attachInterrupt(digitalPinToInterrupt(C1), encoder, CHANGE);
  attachInterrupt(digitalPinToInterrupt(C2), encoder, CHANGE);
  
  Serial.println("Numero de conteos"); // Print header
}

void loop() {
  // Check if sampling interval has passed
  if (millis() - lastTime >= sampleTime)
  {  
      lastTime = millis(); // Update last sample time
      Serial.print("Cuentas: "); // Print label
      Serial.println(n); // Print pulse count
   }
}

// Quadrature encoder ISR
void encoder(void)
{
    ant=act; // Save previous state
    act = (digitalRead(C1) << 2) | (digitalRead(C2) << 3);

    // Detect forward transitions and increment count
    if(ant==0  && act== 4)  n++;
    if(ant==4  && act==12)  n++;
    if(ant==8  && act== 0)  n++;
    if(ant==12 && act== 8)  n++;
    
    // Detect backward transitions and decrement count
    if(ant==0  && act==8)  n--; 
    if(ant==4  && act==0)  n--;
    if(ant==8  && act==12) n--;
    if(ant==12 && act==4) n--;
}
