const int buttonPin = 10;                     // the number of the pushbutton pin
const unsigned long debounceDuration = 50;    // to avoid button double pressing

int buttonState = 0;                          // variable for reading the pushbutton status
int buttonHeld = 0;                           // previous pushbutton state
enum states{waiting, counting, paused};       // possible state the timer can be in
enum states currentState = waiting;           // the state the timer is in
unsigned long lastDebounceTime = 0;           // timestamp of last pushbutton press
unsigned long timerEnd = 0;                   // Time when the timer is up
unsigned long timeRemaining = 0;              // Time left on the counter
bool userInput = false;                       // The user  has put input on the button

class Pomodoro {
  private:
    int currentTimerStep;
    const unsigned long times [8] = {5*1000,2*1000,5*1000,2*1000,5*1000,2*1000,5*1000,8*1000};
    const String TimerStep [8] = {"work 1", "rest 1", "work 2", "rest 2","work 3","rest 3","work 4","long rest"};

  public:
    Pomodoro() {
      currentTimerStep = 0;
    }
    
    String GetCurrentTimerStep(){
      return TimerStep[currentTimerStep];
    }

    unsigned long GetCurrentTimerDuration(){
      return times[currentTimerStep];
    }

    void NextTimerStep(){
      currentTimerStep++;
      if (currentTimerStep>7){
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

    bool TimerComplete(){
      if (millis() > endTime){
        return true;
      }
      return false;
    }
  
    unsigned long GetRemaining(){
      return (endTime - millis());
    }
    
    void SetTimer(unsigned long newDuration){
        endTime = millis() + newDuration;
    }
};
Pomodoro pomodoro = Pomodoro();
Timer timer = Timer();

bool CheckButton(){                           // returns true if valid button press
  if(lastDebounceTime + debounceDuration < millis()){
    buttonState = digitalRead(buttonPin);
    if (buttonState != buttonHeld) {
      buttonHeld = buttonState;
      lastDebounceTime = millis();
      if (buttonState == 1){
        return true;
      }
    }
  }  
  return false;
}

void setup() {
  pinMode(buttonPin, INPUT);                  // initialize the pushbutton pin as an input:
  Serial.begin(9600);                         // send and receive at 9600 baud
}

void loop() {
   userInput = CheckButton();                 // has button been pressed this cycle
   switch (currentState){                     // what state is it currently in?
    case waiting:                             // standby state, its just waiting for the user to start the timer
      if (userInput){                         // if in standby mode and the user presses the button
        Serial.print("Starting timer for ");     // inform timer started
        Serial.println(pomodoro.GetCurrentTimerStep());
        currentState = counting;
        timer.SetTimer (pomodoro.GetCurrentTimerDuration());
      }
      break;
    case counting:                            // the countdown timer is counting down
      if (timer.TimerComplete()){                 // check if the timer is done
        Serial.println("timer done");         // if done, inform user
        currentState = waiting;               // return to standby state
        pomodoro.NextTimerStep(); 
      } 
      else if (userInput){                    // if the button was pressed
        Serial.print("Pausing timer at ");    // inform that timer is paused
        timeRemaining = timer.GetRemaining();     // record how much time is left on the counter
        Serial.println(timeRemaining);        // inform how much time is left
        currentState = paused;                // go to the paused state
        }
      break;
    case paused:                              // paused state is similar to standby state
      if (userInput){                         // if button pressed
        Serial.println("Restarting timer");   // inform user that timer restarted
        currentState = counting;              // next state will be counting state
        timer.SetTimer(timeRemaining);
      }
      break;
    default:                                  // should never get to default state
      Serial.println("default");              // spam the serial
      break;
    }
    userInput = false;                        // set button pressed to false as all actions are complete
}
