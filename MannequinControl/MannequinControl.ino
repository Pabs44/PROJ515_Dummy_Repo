#define OUT LOW
#define IN HIGH
#define RX1_PIN 19
#define TX1_PIN 18

enum LEDStates{
  noProblemsFlag = 0,
  runningConcernFlag,
  showStopperFlag,
};

enum limitSwitchPins{
  NeckLimitSW = 2,
  LeftChestLimitSW, //3
  RightChestLimitSW, //4
  BackChestLimitSW, //5
  FrontChestLimitSW, //6
  LeftWaistLimitSW, //7
  RightWaistLimitSW, //8
  BackWaistLimitSW, //9
  FrontWaistLimitSW, //10
  LeftHipLimitSW, //11
  RightHipLimitSW, //12
  BackHipLimitSW //13
};

enum LEDPins{
  RedLED = 20,
  YellowLED, //21
  GreenLED = 50
};

enum stepperMotorPins{
  NeckStep = 22,
  NeckDir, //23
  LeftChestStep, //24
  LeftChestDir, //25
  RightChestStep, //26
  RightChestDir, //27
  BackChestStep, //28
  BackChestDir, //29
  FrontChestStep, //30
  FrontChestDir, //31
  LeftWaistStep, //32
  LeftWaistDir, //33
  RightWaistStep, //34
  RightWaistDir, //35
  BackWaistStep, //36
  BackWaistDir, //37
  FrontWaistStep, //38
  FrontWaistDir, //39
  LeftHipStep, //40
  LeftHipDir, //41
  RightHipStep, //42
  RightHipDir, //43
  BackHipStep, //44
  BackHipDir, //45

  NeckEn, //46
  LeftChestEn, //47
  RightChestEn, //48
  BackChestEn, //49
  
  LeftWaistEn = 51,
  RightWaistEn, //52
  BackWaistEn, //53

  FrontWaistEn = 14, //14
  LeftHipEn, //15
  RightHipEn, //16
  BackHipEn //17
};

enum ultrasonicSensorPins{
  TopFrontLeftTrig = 54,
  TopFrontLeftEcho,
  TopFrontRightTrig,
  TopFrontRightEcho,
  TopBackLeftTrig,
  TopBackLeftEcho,
  TopBackRightTrig,
  TopBackRightEcho,
  BottomFrontLeftTrig,
  BottomFrontLeftEcho,
  BottomFrontRightTrig,
  BottomFrontRightEcho,
  BottomBackLeftTrig,
  BottomBackLeftEcho,
  BottomBackRightTrig,
  BottomBackRightEcho
};

const int motSpd = 200;
const int circsCnt = 4;
const int motCnt = 12;
const int stpGoal = 1 * 16;

const int maxSteps[motCnt] = {
  100 * 16, //
  110 * 16, //
  80 * 16,  //
  80 * 16,  //
  40 * 16,  //
  20 * 16,  //
  30 * 16,  //
  20 * 16,  //
  40 * 16,  //
  50 * 16,  //
  30 * 16,  //
  20 * 16   //
};

const float percentageQualifier = 0.005;

int stepPin;
int dirPin;
int enPin;
int swPin;

//NECK, CHEST, WAIST, HIP
float minCircumferences[circsCnt] = {365, 921, 750, 925}; //minimum active Circumference values of the mannequin in mm
float maxCircumferences[circsCnt] = {374, 961, 799, 969}; //maximum active Circumference values of the mannequin in mm
float activeCirRange[circsCnt];     //difference between the min and max values of the mannequin
float desiredCirDiff[circsCnt];     //distance mannequin needs to move to reach desired measurments
float percentageMovement[circsCnt]; //array to save percentage mechanisms have to move

int desiredStepNum[motCnt];         //storing the number of steps needed for each motor to move the mechanism to create the desired Circumference

int stepPinNum = 22;
int dirPinNum = 23;
int limitSWPinNum = 2;
int enablePinNum = 46;

int ultrasonicTrigPinNum = 54;
int ultrasonicEchoPinNum = 55;
long minimumUltrasonicValue[8];
long ultrasonicValue[8];
int ultrasonicFlag;

int numOfSteps = 0;

int currentCompareCnt[8];
int zeroValueCnt[8];
int tempUltrasonicValue[8];

int systemStateFlag = noProblemsFlag;

int tmr5Cnt = 0;
bool tmrESPState = false;

ISR(TIMER3_COMPA_vect) {
  if (systemStateFlag == runningConcernFlag) {
    digitalWrite(RedLED, LOW);
    digitalWrite(GreenLED, LOW);
    digitalWrite(YellowLED, !digitalRead(YellowLED));
  } else if (systemStateFlag == showStopperFlag) {
    digitalWrite(YellowLED, LOW);
    digitalWrite(GreenLED, LOW);
    digitalWrite(RedLED, !digitalRead(RedLED));
  }
}

ISR(TIMER5_COMPA_vect) {
  if(tmr5Cnt > 15){
    tmrESPState = true;
    tmr5Cnt = 0;
  }else tmr5Cnt++;
}

void showStopper() {
  for(int i = NeckEn; i < BackWaistEn; i++){
    if(i == GreenLED) continue;
    digitalWrite(stepperMotorPins(i), HIGH);
  }
  for(int i = FrontWaistEn; i < BackHipEn; i++) digitalWrite(stepperMotorPins(i), HIGH);

  systemStateFlag = showStopperFlag;
  for (int i=0;i<motCnt;i++) desiredStepNum[i] = 0;

  Serial.println("The system has encountered a Show Stopper level problem");
}

void runningConcern() {
  systemStateFlag = runningConcernFlag;

  Serial.println("The system has encountered a Running Concern level problem");
}

void moveOut() {

  digitalWrite(enPin, LOW);
  //Serial.println("Move Out");
  digitalWrite(dirPin, OUT); //moves out

  for (int i = 0; i < 50 * 16; i++) {
    //Serial.println("for");
    digitalWrite(stepPin, HIGH);
    delayMicroseconds(motSpd);
    digitalWrite(stepPin, LOW);
    delayMicroseconds(motSpd);

    //Serial.println(i);
  }
  //digitalWrite(enPin,HIGH);

  // numOfSteps += 10;
  //  Serial.println(numOfSteps);
}

void moveIn() {
  numOfSteps = 0;

  digitalWrite(enPin, LOW);
  digitalWrite(dirPin, IN); //moves in

  int check = 0;
  while (check == 0) {
    Serial.println(check);
    if (digitalRead(swPin) == HIGH) {
      delay(50);
      if (digitalRead(swPin) == HIGH) {
        check = 1;
        Serial.println(check);
      }
    }
    Serial.println(check);
    digitalWrite(stepPin, HIGH);
    delayMicroseconds(motSpd);
    digitalWrite(stepPin, LOW);
    delayMicroseconds(motSpd);
  }
  //digitalWrite(enPin,HIGH);
}

void moveAllOut(float desiredCircumference[4]) {
  if((desiredCircumference[0] >= minCircumferences[0]) &&
     (desiredCircumference[1] >= minCircumferences[1]) &&
     (desiredCircumference[2] >= minCircumferences[2]) &&
     (desiredCircumference[3] >= minCircumferences[3]) &&
     (desiredCircumference[0] <= maxCircumferences[0]) &&
     (desiredCircumference[1] <= maxCircumferences[1]) &&
     (desiredCircumference[2] <= maxCircumferences[2]) &&
     (desiredCircumference[3] <= maxCircumferences[3]))
  {  
    systemStateFlag = noProblemsFlag;
    digitalWrite(RedLED, LOW);
    digitalWrite(YellowLED, HIGH);
    digitalWrite(GreenLED, LOW);

    ultrasonicFlag = 1;
    stepPinNum = 22;
    dirPinNum = 23;
    enablePinNum = 46;
  
    for (int i = 0; i < 4; i++) {
      activeCirRange[i] = maxCircumferences[i] - minCircumferences[i];
      desiredCirDiff[i] = desiredCircumference[i] - minCircumferences[i];
      percentageMovement[i] = desiredCirDiff[i] / activeCirRange[i];
    }
  
    int pos = 0;
    desiredStepNum[0] = maxSteps[0] * percentageMovement[0];
    for (pos = 1; pos < 5; pos++) desiredStepNum[pos] = maxSteps[pos] * percentageMovement[1];
    for (; pos < 9; pos++) desiredStepNum[pos] = maxSteps[pos] * percentageMovement[2];
    for (; pos < 12; pos++) desiredStepNum[pos] = maxSteps[pos] * percentageMovement[3];
  
    int totalSteps = 0;
    for (int i = 0; i < motCnt; i++) {
      //desiredStepNum[i] *= scalar;
      totalSteps += desiredStepNum[i];
  
      digitalWrite(dirPinNum, OUT);
      dirPinNum += 2;
    }

    int cnt = 0;
    while (totalSteps >= 1) {
      if (cnt >= motCnt){
        cnt = 0;
        ultrasonicRead();
      }
  
      if (stepPinNum >= 46) stepPinNum = 22;
  
      if (enablePinNum >= 54) {
        enablePinNum = 14;
      } else if (enablePinNum == 18) {
        enablePinNum = 46;
      }
  
      if (desiredStepNum[cnt] >= 1) {
        digitalWrite(enablePinNum, LOW);
  
        for (int i = 0; i < stpGoal; i++) {
          digitalWrite(stepPinNum, HIGH);
          delayMicroseconds(motSpd);
          digitalWrite(stepPinNum, LOW);
          delayMicroseconds(motSpd);
        }
  
        if (stepPinNum != FrontChestStep) digitalWrite(enablePinNum, HIGH);
  
        desiredStepNum[cnt] -= stpGoal;
        totalSteps -= stpGoal;
      }
  
      cnt++;
      stepPinNum += 2;
      enablePinNum++;
    }
    Serial.println("Manneqin has moved to your desired measurements");
  
    digitalWrite(RedLED, LOW);
    digitalWrite(YellowLED, LOW);
    digitalWrite(GreenLED, HIGH);
  }
  else{
    systemStateFlag = runningConcernFlag;
    Serial.println("The measurements provided cannot be achieved because they are not within the active range of the mannequin");
  }
}

void moveAllIn() {
  digitalWrite(RedLED, HIGH);
  digitalWrite(YellowLED, LOW);
  digitalWrite(GreenLED, LOW);

  stepPinNum = 22;
  dirPinNum = 23;
  limitSWPinNum = 2;
  enablePinNum = 46;

  for (int i = 0; i < motCnt; i++) {
    digitalWrite(dirPinNum, IN);
    dirPinNum += 2;
  }

  int cnt = 0, swChk = 0;
  do{
    swChk = 0;

    if (limitSWPinNum >= 14) limitSWPinNum = 2;

    if (stepPinNum >= 45) stepPinNum = 22;

    if (enablePinNum >= 54) {
      enablePinNum = 14;
    } else if (enablePinNum == 18) {
      enablePinNum = 46;
    }

    if (cnt > stpGoal / 16) {
      for(int i = NeckLimitSW; i < BackHipLimitSW; i++){
        Serial.print(digitalRead(i));
        if(digitalRead(i) == 1) swChk++;
      }
      Serial.println(digitalRead(BackHipLimitSW));
      if(digitalRead(BackHipLimitSW) == 1) swChk++;
      cnt = 0;
    }

    if (digitalRead(limitSWPinNum) == 1) {
      //delay(25);
      if (digitalRead(limitSWPinNum) != 1) {
        digitalWrite(enablePinNum, LOW);

        digitalWrite(stepPinNum, HIGH);
        delayMicroseconds(motSpd);
        digitalWrite(stepPinNum, LOW);
        delayMicroseconds(motSpd);
      }
    } else {
      digitalWrite(enablePinNum, LOW);

      for (int i = 0; i < stpGoal; i++) {
        digitalWrite(stepPinNum, HIGH);
        delayMicroseconds(motSpd);
        digitalWrite(stepPinNum, LOW);
        delayMicroseconds(motSpd);
      }
    }

    if (stepPinNum != FrontChestStep) digitalWrite(enablePinNum, HIGH);

    stepPinNum += 2;
    limitSWPinNum++;
    enablePinNum++;
    cnt++;
  }
  while (swChk < motCnt);
  
  Serial.println("The mannequin is at its minimum");

  digitalWrite(RedLED, LOW);
  digitalWrite(YellowLED, LOW);
  digitalWrite(GreenLED, HIGH);
}

void ultrasonicRead() {
  for(int i=0; i<8; i++){
    zeroValueCnt[i] = 0;
    currentCompareCnt[i] = 0;
  }
  ultrasonicTrigPinNum = 54;
  ultrasonicEchoPinNum = 55;

  for (int i=0;i<8;i++) {
    digitalWrite(ultrasonicTrigPinNum, LOW);
    delayMicroseconds(2);
    digitalWrite(ultrasonicTrigPinNum, HIGH);
    delayMicroseconds(10);
    digitalWrite(ultrasonicTrigPinNum, LOW);
    delayMicroseconds(20);
    
    tempUltrasonicValue[i] = pulseIn(ultrasonicEchoPinNum, HIGH);
    if(tempUltrasonicValue[i] == 0){
      zeroValueCnt[i]++;
    }else{
      zeroValueCnt[i] = 0;
    }

    if(tempUltrasonicValue[i]-ultrasonicValue[i] <= ultrasonicValue[i]*percentageQualifier){
      currentCompareCnt[i]++;
    }else{
      currentCompareCnt[i] = 0;
    }

    if(zeroValueCnt[i] >= 10){
      systemStateFlag = runningConcernFlag;

      switch(i){
        case 0:
          Serial.println("The Top Front Left Ultrasonic sensor is repeatedly returning a 0 value.");
          Serial.println("The probable causes are that the ultrasonic sensor has been disconnected or it is inoperable.");
          break;
        case 1:
          Serial.println("The Top Front Right Ultrasonic sensor is repeatedly returning a 0 value.");
          Serial.println("The probable causes are that the ultrasonic sensor has been disconnected or it is inoperable.");
          break;
        case 2:
          Serial.println("The Top Back Left Ultrasonic sensor is repeatedly returning a 0 value.");
          Serial.println("The probable causes are that the ultrasonic sensor has been disconnected or it is inoperable.");
          break;
        case 3:
          Serial.println("The Top Back Right Ultrasonic sensor is repeatedly returning a 0 value.");
          Serial.println("The probable causes are that the ultrasonic sensor has been disconnected or it is inoperable.");
          break;
        case 4:
          Serial.println("The Bottom Front Left Ultrasonic sensor is repeatedly returning a 0 value.");
          Serial.println("The probable causes are that the ultrasonic sensor has been disconnected or it is inoperable.");
          break;
        case 5:
          Serial.println("The Bottom Front Right Ultrasonic sensor is repeatedly returning a 0 value.");
          Serial.println("The probable causes are that the ultrasonic sensor has been disconnected or it is inoperable.");
          break;
        case 6:
          Serial.println("The Bottom Back Left Ultrasonic sensor is repeatedly returning a 0 value.");
          Serial.println("The probable causes are that the ultrasonic sensor has been disconnected or it is inoperable.");
          break;
        case 7:
          Serial.println("The Bottom Back Right Ultrasonic sensor is repeatedly returning a 0 value.");
          Serial.println("The probable causes are that the ultrasonic sensor has been disconnected or it is inoperable.");
          break;
      }      
    }

    if(currentCompareCnt[i] >= 10){
      showStopper();

      switch(i){
        case 0:
          Serial.println("The Top Front Left Ultrasonic sensor is repeatedly returning similar values");
          Serial.println("The probable causes are that a corrisponding motor or mechanism has stopped functioning");
          break;
        case 1:
          Serial.println("The Top Front Right Ultrasonic sensor is repeatedly returning similar values");
          Serial.println("The probable causes are that a corrisponding motor or mechanism has stopped functioning");
          break;
        case 2:
          Serial.println("The Top Back Left Ultrasonic sensor is repeatedly returning similar values");
          Serial.println("The probable causes are that a corrisponding motor or mechanism has stopped functioning");
          break;
        case 3:
          Serial.println("The Top Back Right Ultrasonic sensor is repeatedly returning similar values");
          Serial.println("The probable causes are that a corrisponding motor or mechanism has stopped functioning");
          break;
        case 4:
          Serial.println("The Bottom Front Left Ultrasonic sensor is repeatedly returning similar values");
          Serial.println("The probable causes are that a corrisponding motor or mechanism has stopped functioning");
          break;
        case 5:
          Serial.println("The Bottom Front Right Ultrasonic sensor is repeatedly returning similar values");
          Serial.println("The probable causes are that a corrisponding motor or mechanism has stopped functioning");
          break;
        case 6:
          Serial.println("The Bottom Back Left Ultrasonic sensor is repeatedly returning similar values");
          Serial.println("The probable causes are that a corrisponding motor or mechanism has stopped functioning");
          break;
        case 7:
          Serial.println("The Top Front Right Ultrasonic sensor is repeatedly returning similar values");
          Serial.println("The probable causes are that a corrisponding motor or mechanism has stopped functioning");
          break;
      }
    }
    
    ultrasonicValue[i] = tempUltrasonicValue[i];
    ultrasonicTrigPinNum += 2;
    ultrasonicEchoPinNum += 2;

    //stops possible reflections between panels affecting results
    delay(5);
  }
}

float rxESP(float circs[4]){
  tmrESPState = false;
  Serial1.flush();
  byte recvCircs = 0b0000;
  bool handshake = false;
  unsigned long timeout = 5000, msBegin = 0, msCurr = 0;

  Serial.println("\nWaiting for ESP");
  while(Serial1.available() <= 0);
  Serial.println("Connected");

  msBegin = millis();
  while(Serial1.read() != '$'){
    msCurr = millis();
    if(msCurr - msBegin >= timeout){
      Serial.println("Handshake timed out");
      return;
    }
  }
  handshake = true;
  Serial.println("Handshake initiated, sending confirmation");
  Serial1.println('$');

  while (handshake && recvCircs != 0b1111) {
    int numAttempts = 0, badData = 0, goodData = 0;

    if(Serial1.available() > 0){
      Serial1.readStringUntil('!');
      for(int i = 0; i < circsCnt; i++){
        Serial.println("Next circ");
        String msg = Serial1.readStringUntil('\n');
        if(msg.charAt(0) == 'n'){
          msg.remove(0,1);
          Serial.println(msg);
          circs[i] = msg.toFloat();
          recvCircs |= 0b0001;
          goodData++;
          badData = 0;
        }else if(msg.charAt(0) == 'c'){
          msg.remove(0,1);
          Serial.println(msg);
          circs[i] = msg.toFloat();
          recvCircs |= 0b0010;
          goodData++;
          badData = 0;
        }else if(msg.charAt(0) == 'w'){
          msg.remove(0,1);
          Serial.println(msg);
          circs[i] = msg.toFloat();
          recvCircs |= 0b0100;
          goodData++;
          badData = 0;
        }else if(msg.charAt(0) == 'h'){
          msg.remove(0,1);
          Serial.println(msg);
          circs[i] = msg.toFloat();
          recvCircs |= 0b1000;
          goodData++;
          badData = 0;
        }else{
          Serial.println("Bad data coming in");
          badData++;
        }

        if(goodData >= 4){
          Serial.println("All circumferences recorded");
          Serial.println();
          for(int i = 0; i < circsCnt; i++) Serial.println(circs[i]);
          return;
        }else if(badData >= 12){
          Serial.println("Too much bad data, fix ESP output");
          return;
        }
      }
    }else{
      numAttempts++;
      Serial.println("\nUnable to connect, reattempting...");
    }

    if(numAttempts > 5){
      Serial.println("ESP disconnected, reattempt connection");
      return;
    }
  }
}

void setup() {
  Serial.begin(115200);
  Serial.println("Start");
  Serial1.begin(115200);

  for(int i = NeckLimitSW; i < BackHipLimitSW; i++) pinMode(limitSwitchPins(i), INPUT);

  for(int i = NeckStep; i < BackHipDir; i++) pinMode(stepperMotorPins(i), OUTPUT);
  for(int i = NeckEn; i < BackWaistEn; i++){
    if(i == GreenLED) continue;
    pinMode(stepperMotorPins(i), OUTPUT);
    digitalWrite(stepperMotorPins(i), HIGH);
  }
  for(int i = FrontWaistEn; i < BackHipEn; i++){
    pinMode(stepperMotorPins(i), OUTPUT);
    digitalWrite(stepperMotorPins(i), HIGH);
  }

  for(int i = TopFrontLeftTrig; i < BottomBackRightTrig; i+=2) pinMode(ultrasonicSensorPins(i), OUTPUT);
  for(int i = TopFrontLeftEcho; i < BottomBackRightEcho; i+=2) pinMode(ultrasonicSensorPins(i), INPUT);

  pinMode(GreenLED, OUTPUT);
  pinMode(YellowLED, OUTPUT);
  pinMode(RedLED, OUTPUT);
  digitalWrite(GreenLED, HIGH);
  digitalWrite(YellowLED, LOW);
  digitalWrite(RedLED, LOW);

  cli();  //stop interrupts
  //set timer3 interrupt at 1Hz
  TCCR3A = 0; // set entire TCCR3A register to 0
  TCCR3B = 0; // same for TCCR1B
  TCNT3  = 0; //initialize counter value to 0
  //set compare match register for 1hz increments
  OCR3A = 3906; // = (16*10^6) / (1*1024) - 1 (must be <65536)
  TIMSK3 |= (1 << OCIE3A);  // enable timer compare interrupt
  TCCR3B |= (1 << WGM12);   // turn on CTC mode
  TCCR3B |= (1 << CS12) | (1 << CS10);  // Set CS10 and CS12 bits for 1024 prescaler

  //set timer5 interrupt at 1Hz
  TCCR5A = 0; // set entire TCCR5A register to 0
  TCCR5B = 0; // same for TCCR1B
  TCNT5  = 0; //initialize counter value to 0
  //set compare match register for 1hz increments
  OCR5A = 62500;  // = (16*10^6) / (1*1024) - 1 (must be < 65536)
  TIMSK5 |= (1 << OCIE5A); // enable timer compare interrupt
  TCCR5B |= (1 << WGM12);  // turn on CTC mode
  TCCR5B |= (1 << CS12) | (1 << CS10);  // Set CS10 and CS12 bits for 1024 prescaler
  sei();  //allow interrupts
}

void loop() {
  float desiredCircumference[circsCnt];
  for(int i = 0; i < circsCnt; i++) desiredCircumference[i] = maxCircumferences[i];

  rxESP(desiredCircumference);
  Serial.flush();

  while(true){
    if (Serial.available() > 0) {
      String msg = Serial.readStringUntil('\n');
      Serial.println(msg);

      if (msg.equals( "i")) {
        Serial.println("Received i");
        moveIn();
      } else if (msg.equals("o")) {
        Serial.println("Received o");
        moveOut();
      } else if (msg.equals("n")) {
        Serial.println("Neck");
        stepPin = NeckStep;
        dirPin = NeckDir;
        enPin = NeckEn;
        swPin = NeckLimitSW;
      } else if (msg.equals("fc")) {
        Serial.println("Front Chest");
        stepPin = FrontChestStep;
        dirPin = FrontChestDir;
        swPin = FrontChestLimitSW;
      } else if (msg.equals("lc")) {
        Serial.println("Left Chest");
        stepPin = LeftChestStep;
        dirPin = LeftChestDir;
        enPin = LeftChestEn;
        swPin = LeftChestLimitSW;
      } else if (msg.equals("rc")) {
        Serial.println("Right Chest");
        stepPin = RightChestStep;
        dirPin = RightChestDir;
        enPin = RightChestEn;
        swPin = RightChestLimitSW;
      } else if (msg.equals("bc")) {
        Serial.println("Back Chest");
        stepPin = BackChestStep;
        dirPin = BackChestDir;
        enPin = BackChestEn;
        swPin = BackChestLimitSW;
      } else if (msg.equals("fw")) {
        Serial.println("Front Waist");
        stepPin = FrontWaistStep;
        dirPin = FrontWaistDir;
        enPin = FrontWaistEn;
        swPin = FrontWaistLimitSW;
      } else if (msg.equals("lw")) {
        Serial.println("Left Waist");
        stepPin = LeftWaistStep;
        dirPin = LeftWaistDir;
        enPin = LeftWaistEn;
        swPin = LeftWaistLimitSW;
      } else if (msg.equals("rw")) {
        Serial.println("Right Waist");
        stepPin = RightWaistStep;
        dirPin = RightWaistDir;
        enPin = RightWaistEn;
        swPin = RightWaistLimitSW;
      } else if (msg.equals("bw")) {
        Serial.println("Back Waist");
        stepPin = BackWaistStep;
        dirPin = BackWaistDir;
        enPin = BackWaistEn;
        swPin = BackWaistLimitSW;
      } else if (msg.equals("lh")) {
        Serial.println("Left Hip");
        stepPin = LeftHipStep;
        dirPin = LeftHipDir;
        enPin = LeftHipEn;
        swPin = LeftHipLimitSW;
      } else if (msg.equals("rh")) {
        Serial.println("Right Hip");
        stepPin = RightHipStep;
        dirPin = RightHipDir;
        enPin = RightHipEn;
        swPin = RightHipLimitSW;
      } else if (msg.equals("bh")) {
        Serial.println("Back Hip");
        stepPin = BackHipStep;
        dirPin = BackHipDir;
        enPin = BackHipEn;
        swPin = BackHipLimitSW;
      } else if (msg.equals("ao")) {
        Serial.println("Moving out to desired circumferences");
        moveAllOut(desiredCircumference);
      } else if (msg.equals("ai")) {
        Serial.println("Moving to minimum position");
        moveAllIn();
      } else if (msg.equals("esp")) {
        Serial.println("Retrieving data from ESP32");
        rxESP(desiredCircumference);
      } else if (msg.equals("reset")) {
        Serial.println("Reset");
        systemStateFlag = noProblemsFlag;
      }
    }

    if(tmrESPState){
      float tmpCircs[circsCnt];
      rxESP(tmpCircs);
      for(int i = 0; i < circsCnt; i++) Serial.println(tmpCircs[i]);
      Serial.println("New data input from ESP32, is it acceptable? [y/n]:");
      while(Serial.available() <= 0);
      if(Serial.readStringUntil('\n') == "y"){
        for(int i = 0; i < circsCnt; i++) desiredCircumference[i] = tmpCircs[i];
        Serial.println("New data accepted");
        moveAllIn();
        moveAllOut(desiredCircumference);
      }else Serial.println("New data rejected");
    }
  }
}
