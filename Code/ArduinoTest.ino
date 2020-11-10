volatile int signalA = 9;                            // Set signal A for pin 9
volatile int signalB = 12;                           // Set signal B for pin 12
const int potPin = A1;                               // Set potentiometer for pin A1
const byte switchPin = 38;                           // Set switch for pin 38
volatile byte switchState = 0;                       // Current state of switchPin
byte changeState = 0;                                // Variable to check to see if switchPin state value has changed

boolean toggleA = 0;                                 // Toggle flag for signal A
boolean toggleB = 0;                                 // Toggle flag for signal B

int value = 0;                                       // Analog to digital value read in by potentiometer
int potval = 0;                                      // Variable to check to see if potentiometer value has changed
int buf[4];                                          // Buffer for potentiometer values
int cumulative = 0;                                  // Variable to hold all current buf values
int bufcount = 0;                                    // Counter for buf
uint16_t intcount = 0;                               // Counter for LEDs


void setup() {
  Serial.begin(9600);
  pinMode(signalA, OUTPUT);                          // Initialize signal pins to be outputs
  pinMode(signalB, OUTPUT);
  pinMode(potPin, INPUT);                            // Set potentiometer pin to be input
  pinMode(switchPin, INPUT_PULLUP);                  // The switch pin is an input pulled HIGH by an internal resistor

  DDRA = B11111111;                                  // Set PORTA to outputs
  DDRC = B11111111;                                  // Set PORTA to outputs

  cli();                                             // Disable all interrupts
  TCCR1A = 0;                                        // Set timer
  TCCR1B = 0;
  TCNT1 = 0;                                         // Set counter to 0. TCNT1 stores the timer 1 counter values

  ICR1 = 199;                                        // Set default ICR1 = [16Mhz / (desired frequency * prescaler)] - 1
  OCR1A = ICR1 - 1;                                  // Set Compare Match Register A to ICR1 at almost 100% duty cycle
  OCR1B = OCR1A / 2;                                 // set Compare Match Register B to OCR1A with 50% duty cycle
  TCCR1B |= (1 << CS11);                             // Set prescaler
  // CS10, CS11, CS12 are the clock select bits for Timer 1
  // Each bit corresponds to a prescaler
  // In this case, enabling CS11 will set the prescaler to 8.
  TIMSK1 |= (1 << OCIE1A) | (1 << OCIE1B);           // Enable Timer Compare Interrupt A & B

  TCCR1B |= (1 << WGM13) | (1 << WGM12);             // CTC mode with ICR1 = Clear timer when the counter reaches the compare match register
  // This will set the two bits of WGM to 14
  // Note: 1 << WGM12 is the same as TCCR1B = 32
  sei();                                             // Enable all interrupts
}


void loop() {
  switchState = PIND & 0b10000000;                   // Set switchState to only observe digital pin 38
  if (changeState != switchState) {                  // If changeState does not equal the current state of the switchPin
    switchPins();                                    // Call method to change the signal pins
    changeState = switchState;                       // Set changeState to equal the current state of the switchPin
  }
  delay(100);
  readPin();                                         // Read potentiometer pin
}

void readPin() {

  volatile int temp = analogRead(potPin);            // Analog read from potPin and assigned to value
  temp = temp >> 4;                                  // Shift value by 4 bits to reduce range (1023/16) for smoother transition
  if (bufcount < 4) {                                // If the bufcount is less than 4, have it equal the value of the potPin
    buf[bufcount] = temp;
    cumulative += buf[bufcount];                     // Add and store buf into cumulative
    bufcount++;                                      // Increment bufcount
  }
  else {                                             // If bufcount is now greater than the buf size
    value = cumulative >> 2;                         // value is equal to cumulative/4
    bufcount = 0;                                    // Reset both bufcount and cumulative
    cumulative = 0;
  }
  if (potval != value) {                             // If potval does not equal value, update the frequency
    updatePin();
  }
  potval = value;                                    // Set potval to equal value
}

void updatePin() {
  if (value <= 49 && value >= 0) {                   // Set a limit for when potentiometer ADC value to 48 (49 is for 20kHz)
    cli();                                           // Disable all interrupts
    resetICR(99);                                    // Set value
    sei();                                           // Enable all interrupts
  }
  else if (value == 63) {                            // Stop all interrupts if potentiometer value is 63
    cli();                                           // 63 is the "lowest" value the potentiometer can reach
  }
  else {
    cli();                                           // Disable all interrupts
    resetICR(value);                                 // Otherwise, update frequency to acceptable value from the potentiometer
    sei();                                           // Enable all interrupts
  }
}

void resetICR(int input) {
  intcount = 0;
  TCCR1A = 0;                                        // Set timers
  TCCR1B = 0;
  TCNT1 = 0;                                         // Set counter to 0. TCNT1 stores the timer 1 counter values
  ICR1 = input;                                      // Set ICR1 = [16Mhz / (desired frequency * prescaler)] - 1

  OCR1A = ICR1 - 1;                                  // Set Compare Match Register A to ICR1 at almost 100% duty cycle
  OCR1B = OCR1A / 2;                                 // set Compare Match Register B to OCR1A with 50% duty cycle, toggling signal half a period later
  TCCR1B |= (1 << CS11);                             // Set prescaler
  // CS10, CS11, CS12 are the clock select bits for Timer 1
  // Each bit corresponds to a prescaler
  // In this case, enabling CS11 will set the prescaler to 8.
  TIMSK1 |= (1 << OCIE1A) | (1 << OCIE1B);           // Enable Timer Compare Interrupt A & B
  TCCR1B |= (1 << WGM13) | (1 << WGM12);             // CTC mode with ICR1 = Clear timer when the counter reaches the compare match register

}

void switchPins() {                                  // Change direction of quadrature
  int temp = signalB;                                // Create temp variable to hold pin value of signal B
  signalB = signalA;                                 // Switch signal B to equal signal A
  signalA = temp;                                    // Have signal A equal temp
}

ISR(TIMER1_COMPA_vect) {                             // Interrupt handler. Toggle signalA
  if (toggleA) {
    digitalWrite(signalA, HIGH);
    PORTA = intcount;                                // This code is for the binary counter for the 12 LEDs
    PORTC = intcount >> 4;                           // Additional port registers are shifted by 4 bits
    intcount++;                                      // Increment counter

//    PORTA = 1 << intcount;                           // This code is for the sweeping LEDs
//    PORTC = 1 << intcount - 4;                       // Additional port registers are shifted by 4 bits
//    intcount++;                                      // Increment counter
//
//    if (intcount == 12) {                            // Clear counter once it reaches 12 since there are 12 LEDs
//      intcount = 0;
//    }
    toggleA = 0;
  }
  else {
    digitalWrite(signalA, LOW);
    toggleA = 1;
  }
}


ISR(TIMER1_COMPB_vect) {                             // Interrupt handler. Toggle signalB
  if (toggleB) {
    digitalWrite(signalB, HIGH);
    toggleB = 0;
  }
  else {
    digitalWrite(signalB, LOW);
    toggleB = 1;
  }
}
