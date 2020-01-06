const int buttonPin = 10;                     // the number of the pushbutton pin
const int redLED = 2;
const int blueLED = 3;
const int greenLED = 4;
const unsigned long debounceDuration = 50;    // to avoid button double pressing
const unsigned long blinkDuration = 250;

bool blinking = false;
int buttonState = 0;                          // variable for reading the pushbutton status
int buttonHeld = 0;                           // previous pushbutton state
enum states {waiting, counting, paused};      // possible state the timer can be in
enum states currentState = waiting;           // the state the timer is in
unsigned long lastBlink = 0;
unsigned long lastDebounceTime = 0;           // timestamp of last pushbutton press
unsigned long timerEnd = 0;                   // Time when the timer is up
unsigned long timeRemaining = 0;              // Time left on the counter
bool userInput = false;                       // The user  has put input on the button

class Pomodoro {
  private:
    int currentTimerStep;
    int shortRestTime = 2 * 1000;
    int longRestTime = 8 * 1000;
    int workTime = 5 * 1000;    
    const unsigned long times [8] = {workTime, shortRestTime, workTime, shortRestTime, workTime, shortRestTime, workTime, longRestTime};
    const String TimerStep [8] = {"work 1", "rest 1", "work 2", "rest 2", "work 3", "rest 3", "work 4", "long rest"};
  public:
    Pomodoro() {
      currentTimerStep = 0;
    }

    String GetCurrentTimerStep() {
      return TimerStep[currentTimerStep];
    }

    int GetCurrentTimerStepInt(){
      return currentTimerStep;
    }

    unsigned long GetCurrentTimerDuration() {
      return times[currentTimerStep];
    }

    void NextTimerStep() {
      currentTimerStep++;
      if (currentTimerStep > 7) {
        currentTimerStep = 0;
      }
    }
};

class Timer {
  private:
    unsigned long endTime;

  public :
    Timer() {
      endTime = 0;
    }

    bool TimerComplete() {
      if (millis() > endTime) {
        return true;
      }
      return false;
    }

    unsigned long GetRemaining() {
      return (endTime - millis());
    }

    void SetTimer(unsigned long newDuration) {
      endTime = millis() + newDuration;
    }
};

Pomodoro pomodoro = Pomodoro();
Timer timer = Timer();

bool blinkTime(){
  if (millis() > (lastBlink + blinkDuration)){
    lastBlink = millis();
    blinking = !blinking;
  }
  else{
    return blinking;
  }
}

void chooseColour(int timerStepInt){
  switch (timerStepInt){
        case 0:
        case 2:
        case 4:
        case 6:
          digitalWrite(greenLED, HIGH);
          break;
        case 1:
        case 3:
        case 5:
          digitalWrite(blueLED, HIGH);
          break;
        case 7:
          digitalWrite(redLED, HIGH);
          digitalWrite(blueLED, HIGH);
          break;
        }
  }

void onLED() {
  digitalWrite(blueLED, LOW);
  digitalWrite(redLED, LOW);
  digitalWrite(greenLED, LOW);
  switch(currentState){
    case waiting:
      if (blinking){
        chooseColour(pomodoro.GetCurrentTimerStepInt());
      }
      break;
    case counting:
      chooseColour(pomodoro.GetCurrentTimerStepInt());
      break;
    case paused:
      digitalWrite(redLED, HIGH);
      digitalWrite(greenLED, HIGH);
      break;
    }
}

bool CheckButton() {                          // returns true if valid button press
  if (lastDebounceTime + debounceDuration < millis()) {
    buttonState = digitalRead(buttonPin);
    if (buttonState != buttonHeld) {
      buttonHeld = buttonState;
      lastDebounceTime = millis();
      if (buttonState == 1) {
        return true;
      }
    }
  }
  return false;
}

void setup() {
  pinMode(buttonPin, INPUT);                  // initialize the pushbutton pin as an input:
  Serial.begin(9600);                         // send and receive at 9600 baud
  pinMode(redLED, OUTPUT);
  pinMode(blueLED, OUTPUT);
  pinMode(greenLED, OUTPUT);
  onLED();
}

void loop() {
  userInput = CheckButton();                 // has button been pressed this cycle
  switch (currentState) {                    // what state is it currently in?
    case waiting:                             // standby state, its just waiting for the user to start the timer
      if (userInput) {                        // if in standby mode and the user presses the button
        Serial.print("Starting timer for ");     // inform timer started
        Serial.println(pomodoro.GetCurrentTimerStep());
        currentState = counting;
        onLED();
        timer.SetTimer (pomodoro.GetCurrentTimerDuration());
      }
      else if(blinkTime()){
        onLED();
      }
      break;
    case counting:                            // the countdown timer is counting down
      if (timer.TimerComplete()) {                // check if the timer is done
        Serial.println("timer done");         // if done, inform user
        currentState = waiting;               // return to standby state
        onLED();
        pomodoro.NextTimerStep();
      }
      else if (userInput) {                   // if the button was pressed
        Serial.print("Pausing timer at ");    // inform that timer is paused
        timeRemaining = timer.GetRemaining();     // record how much time is left on the counter
        Serial.println(timeRemaining);        // inform how much time is left
        currentState = paused;                // go to the paused state
        onLED();
      }
      break;
    case paused:                              // paused state is similar to standby state
      if (userInput) {                        // if button pressed
        Serial.println("Restarting timer");   // inform user that timer restarted
        currentState = counting;              // next state will be counting state
        timer.SetTimer(timeRemaining);
        onLED();
      }
      break;
    default:                                  // should never get to default state
      Serial.println("default");              // spam the serial
      break;
  }
  userInput = false;                        // set button pressed to false as all actions are complete
}
