const int buttonPin = 10;                   // the number of the pushbutton pin
const unsigned long debounceDuration = 50;  // to avoid button double pressing

int buttonState = 0;                        // variable for reading the pushbutton status
int buttonHeld = 0;                         // previous pushbutton state
unsigned long lastDebounceTime = 0;         // timestamp of last pushbutton press

void setup() {
  pinMode(buttonPin, INPUT);                // initialize the pushbutton pin as an input:
  Serial.begin(9600);                       // send and receive at 9600 baud
}

void loop() {
  if(lastDebounceTime + debounceDuration < millis()){ //if outside the debounce window
    buttonState = digitalRead(buttonPin);     // read the state of the pushbutton value
    if (buttonState != buttonHeld) {          // buttonState edege, could be rising or falling
      buttonHeld = buttonState;               // past state gets updated
      lastDebounceTime = millis();            // debounce window resets
      if (buttonState == 1){                  // if rising edge
        Serial.println("Pressed");            // serial print Pressed
      }
      else {                                  // if it wasnt rising it was falling edge
        Serial.println("Unpressed");          // serial print Unpressed
      }
    }
  }
}
