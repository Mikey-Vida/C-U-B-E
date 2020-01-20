//constants
const int buttonPin = 10;                     // the number of the pushbutton pin
const int redLED = 3;                         // the red led pin
const int blueLED = 5;                        // the green led pin
const int greenLED = 6;                       // the blue led pin
const unsigned long debounceDuration = 50;    // to avoid button double pressing
const unsigned long multiplePressDuration = 350; // how long to wait when finding a multiple press and not to separate single presses
const unsigned long longPressDuration = 600;  // how long to hold until a longpress is registered

//states
enum stepNames {work, shortRest, longRest};
enum statusNames {waiting, counting, paused, exited};
enum buttonStates {noneB, ignored, processing, singlePress, longPress, triplePress};
enum otherInputStates {noneO, held, something};

//input variables
buttonStates buttonState = noneB;
otherInputStates otherInputState = noneO;
unsigned long lastButtonPress = 0;
unsigned long lastDebounceTime = 0;
int buttonPresses = 0;
bool pastButtonPressed = false;
bool buttonPressed = false;
bool normalPress = false;
bool stillLong = false;

//other global variables
statusNames currentStatus = waiting;
unsigned long timeRemaining = 0;

buttonStates checkButton(){
  if (lastDebounceTime + debounceDuration > millis()){
    return ignored;
  }
  pastButtonPressed = buttonPressed;
  buttonPressed = digitalRead(buttonPin);
  if (buttonPressed && pastButtonPressed){
    if (stillLong){return ignored;}
    if (millis() > lastButtonPress + longPressDuration){
      buttonPresses = 0;
      normalPress = false;
      stillLong = true;
      return longPress;
    }
    return processing;
  }
  if (!buttonPressed && pastButtonPressed){
    lastDebounceTime = millis();
    if(stillLong){
      stillLong = false;
      return ignored;
    }
    if (millis() > lastButtonPress + longPressDuration){
      buttonPresses = 0;
      normalPress = false;
      return longPress;
    }
    if (buttonPresses >=3){
      buttonPresses = 0;
      normalPress = false;
      return triplePress;
    }
  }
  if (buttonPressed && !pastButtonPressed){
    buttonPresses ++;
    lastButtonPress = millis();
    lastDebounceTime = millis();
    normalPress = true;
    return processing;
  }
  if(millis() < lastButtonPress + multiplePressDuration){
    return processing;
  }
  if(normalPress){
    normalPress = false;
    return singlePress;
  }
  buttonPresses = 0;
  return noneB; 
}

buttonStates updateButton(){
  if (otherInputState == noneO){
    return checkButton();
  }
  else if ((otherInputState == held) && (checkButton() == noneB)){
    otherInputState = noneO;
    return noneB;
  }
  else{
    return ignored;
    }
}

class Blinker{
  private:
    unsigned long blinkDuration;
    bool lightOn;
    unsigned long lastBlink;
  public:
    Blinker(int blinkDurationIn){
      lightOn = false;
      lastBlink = 0;
      blinkDuration = blinkDurationIn;
    }

    bool check(){
      if (millis() > (lastBlink + blinkDuration)){
        lightOn = !lightOn;
        lastBlink = millis();
        return true;
      }
      return false;
    }

    bool getLightOn(){
      return lightOn;  
    }
};

class Pomodoro {
  private:
    class pomodoroStep{
      private:
        const int shortRestTime = 2 * 1000;
        const int longRestTime = 8 * 1000;
        const int workTime = 5 * 1000;
        stepNames stepName;
        int stepNum;
      public:
        pomodoroStep(stepNames n, int m){
          stepName = n;
          stepNum = m;
        }
        getStepName(){
          return stepName;
        }
        getStepNum(){
          return stepNum;
        }
        getStepDuration(){
          switch(stepName){
            case work:
              return workTime;
            case shortRest:
              return shortRestTime;
            case longRest:
              return longRestTime;
          }
        }
    };
    int currentStep;
    const pomodoroStep pomodoroSteps [8] = {pomodoroStep(work, 1), pomodoroStep(shortRest, 1),pomodoroStep(work, 2),pomodoroStep(shortRest, 2),pomodoroStep(work, 3), pomodoroStep(shortRest, 3), pomodoroStep(work, 4), pomodoroStep(longRest,1)};
  public:
    Pomodoro() {
      currentStep = 0;
    }

    int getCurrentStepInt(){
      return currentStep;
    }

    stepNames getCurrentStepName(){
      return pomodoroSteps[currentStep].getStepName();
    }

    int getCurrentStepNum(){
      return pomodoroSteps[currentStep].getStepNum();
    }
    
    unsigned long getCurrentStepDuration() {
      return pomodoroSteps[currentStep].getStepDuration();
    }

    void nextPomodoroStep() {
      currentStep++;
      if (currentStep > 7) {
        currentStep = 0;
      }
    }

    void resetPomodoro(){
      currentStep = 0;
    }
};

class Timer {
  private:
    unsigned long endTime;

  public :
    Timer() {
      endTime = 0;
    }

    bool timerComplete() {
      if (millis() > endTime) {
        return true;
      }
      return false;
    }

    unsigned long getRemaining() {
      return (endTime - millis());
    }

    void setTimer(unsigned long newDuration) {
      endTime = millis() + newDuration;
    }
};

Timer timer = Timer();
Pomodoro pomodoro = Pomodoro();
Blinker blinker = Blinker(250);

void chooseColour(int timerStepInt){
  switch (timerStepInt){
        case 0:
        case 2:
        case 4:
        case 6:
          analogWrite(greenLED, 255);
          break;
        case 1:
        case 3:
        case 5:
          analogWrite(blueLED, 255);
          break;
        case 7:
          analogWrite(redLED, 255);
          analogWrite(blueLED, 255);
          break;
        }
  }

void onLED() {
  analogWrite(blueLED, 0);
  analogWrite(redLED, 0);
  analogWrite(greenLED, 0);
  switch(currentStatus){
    case waiting:
      if (blinker.getLightOn()){
        chooseColour(pomodoro.getCurrentStepInt());
      }
      break;
    case counting:
      chooseColour(pomodoro.getCurrentStepInt());
      break;
    case paused:
      analogWrite(redLED, 255);
      analogWrite(greenLED, 255);
      break;
    }
}

void setup() {
  pinMode(buttonPin, INPUT);                  // initialize the pushbutton pin as an input:
  Serial.begin(9600);                         // send and receive at 9600 baud
}

void loop() {
  buttonState = updateButton();
  switch (currentStatus){
    case waiting:
      switch (buttonState){
        case singlePress:
          Serial.print("starting timer for ");
          Serial.println(pomodoro.getCurrentStepInt());
          currentStatus = counting;
          timer.setTimer(pomodoro.getCurrentStepDuration());
          onLED();
          break;
        case triplePress:
          pomodoro.nextPomodoroStep();
          Serial.print("skipping to ");
          Serial.println(pomodoro.getCurrentStepInt());
          if(blinker.check()){onLED();}
          break;
        case longPress:
          Serial.println("exiting");
          currentStatus = exited;
          onLED();
          break;
        default:
          if(blinker.check()){onLED();}
      }     
      break;
    case counting:
      if (timer.timerComplete() && buttonState != processing){
        Serial.println("timer done");
        currentStatus = waiting;
        pomodoro.nextPomodoroStep();
        if(blinker.check()){onLED();}
      }
      else{
        switch (buttonState){
          case singlePress:
            Serial.print("Pausing timer at ");    // inform that timer is paused
            timeRemaining = timer.getRemaining();     // record how much time is left on the counter
            Serial.println(timeRemaining);        // inform how much time is left
            currentStatus = paused;
            onLED();
            break;
          case triplePress:
            currentStatus = waiting;
            Serial.println("skipping to next");
            pomodoro.nextPomodoroStep();
            if(blinker.check()){onLED();}
            break;
          case longPress:
            currentStatus = waiting;
            Serial.println("reseting");
            if(blinker.check()){onLED();}
            break;
        }
      }
      break;
    case paused:
      switch (buttonState){
        case singlePress:
          Serial.println("Restarting timer");   // inform user that timer restarted
          currentStatus = counting;              // next state will be counting state
          timer.setTimer(timeRemaining);
          onLED();
          break;
        case triplePress:
          currentStatus = waiting;
          Serial.println("skipping to next");
          pomodoro.nextPomodoroStep();
          onLED();
          break;
        case longPress:
          currentStatus = waiting;
          Serial.println("reseting");
          onLED();
          break;
      }
      break;
    case exited:
      if (buttonState == longPress){
        Serial.println("restarting Pomodoro");
        pomodoro.resetPomodoro();
        currentStatus = waiting;
        onLED();
      }
      break;
  }
  buttonState = noneB;
}
