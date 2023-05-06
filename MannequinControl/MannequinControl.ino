//const int stepPin = 22;
//const int dirPin = 23;
//const int limitPin = 2;
#define OUT LOW
#define IN HIGH

const int motSpd = 200;

int steP;
int dIr;
int eN;
int liMit;
int stpGoal = 1 * 16;
int totalSteps = 0;

int maxSteps[12] = {100 * 16, 110 * 16, 80 * 16, 80 * 16, 40 * 16, 20 * 16, 30 * 16, 20 * 16, 40 * 16, 50 * 16, 30 * 16, 20 * 16};

//minimum active circumfrence values of the mannequin in mm
//NECK, CHEST, WAIST, HIP
float minCircumfrences[4] = {365, 921, 750, 925};

//maximum active circumfrence values of the mannequin in mm
float maxCircumfrences[4] = {374, 961, 799, 969};

//difference between the min and max values of the mannequin
float activeCirRange[4];

//desired circumfrence values for the mannequin in mm
float desiredCircumfrence[4] = {374, 961, 799, 969};

//distance mannequin needs to move to reach desired measurments
float desiredCirDiff[4];

//array to save percentage mechanisms have to move
float percentageMovement[4];

//storing the number of steps needed for each motor to move the mechanism to create the desired circumfrence
int desiredStepNum[12];

int stepPinNum = 22;
int dirPinNum = 23;
int limitSWPinNum = 2;
int enablePinNum = 46;
int ultrasonicTrigPinNum = 54;
int ultrasonicEchoPinNum = 55;
long minimumUltrasonicValue[8];
long ultrasonicValue[8];
int ultrasonicFlag;

int RedLEDState = 0;
int YellowLEDState = 0;
int GreenLEDState = 1;

int numOfSteps = 0;

typedef enum {
  noProblemsFlag = 0,
  runningConcernFlag,
  showStopperFlag,
} LEDStates;

int systemStateFlag = noProblemsFlag;

typedef enum {
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
  BackHipLimitSW, //13

  FrontWaistEn, //14
  LeftHipEn, //15
  RightHipEn, //16
  BackHipEn, //17
} limitSwitchPins;

typedef enum {
  RedLED = 18,
  YellowLED, //19
  GreenLED, //20
} LEDPins;

typedef enum {
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
  FrontChestEn, //50
  LeftWaistEn, //51
  RightWaistEn, //52
  BackWaistEn, //53
} stepperMotorPins;

typedef enum {
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
  BottomBackRightEcho,
} ultrasonicSensorPins;


void setup() {
  Serial.begin(115200);
  Serial.println("Start");

  pinMode(NeckLimitSW, INPUT);
  pinMode(LeftChestLimitSW, INPUT);
  pinMode(RightChestLimitSW, INPUT);
  pinMode(BackChestLimitSW, INPUT);
  pinMode(FrontChestLimitSW, INPUT);
  pinMode(LeftWaistLimitSW, INPUT);
  pinMode(RightWaistLimitSW, INPUT);
  pinMode(BackWaistLimitSW, INPUT);
  pinMode(FrontWaistLimitSW, INPUT);
  pinMode(LeftHipLimitSW, INPUT);
  pinMode(RightHipLimitSW, INPUT);
  pinMode(BackHipLimitSW, INPUT);

  pinMode(NeckEn, OUTPUT);
  pinMode(LeftChestEn, OUTPUT);
  pinMode(RightChestEn, OUTPUT);
  pinMode(BackChestEn, OUTPUT);
  pinMode(FrontChestEn, OUTPUT);
  pinMode(LeftWaistEn, OUTPUT);
  pinMode(RightWaistEn, OUTPUT);
  pinMode(BackWaistEn, OUTPUT);
  pinMode(FrontWaistEn, OUTPUT);
  pinMode(LeftHipEn, OUTPUT);
  pinMode(RightHipEn, OUTPUT);
  pinMode(BackHipEn, OUTPUT);

  pinMode(RedLED, OUTPUT);
  pinMode(YellowLED, OUTPUT);
  pinMode(GreenLED, OUTPUT);

  pinMode(TopFrontLeftTrig, OUTPUT);
  pinMode(TopFrontLeftEcho, INPUT);
  pinMode(TopFrontRightTrig, OUTPUT);
  pinMode(TopFrontRightEcho, INPUT);
  pinMode(TopBackLeftTrig, OUTPUT);
  pinMode(TopBackLeftEcho, INPUT);
  pinMode(TopBackRightTrig, OUTPUT);
  pinMode(TopBackRightEcho, INPUT);
  pinMode(BottomFrontLeftTrig, OUTPUT);
  pinMode(BottomFrontLeftEcho, INPUT);
  pinMode(BottomFrontRightTrig, OUTPUT);
  pinMode(BottomFrontRightEcho, INPUT);
  pinMode(BottomBackLeftTrig, OUTPUT);
  pinMode(BottomBackLeftEcho, INPUT);
  pinMode(BottomBackRightTrig, OUTPUT);
  pinMode(BottomBackRightEcho, INPUT);

  digitalWrite(NeckEn, HIGH);
  digitalWrite(LeftChestEn, HIGH);
  digitalWrite(RightChestEn, HIGH);
  digitalWrite(BackChestEn, HIGH);
  digitalWrite(FrontChestEn, HIGH);
  digitalWrite(LeftWaistEn, HIGH);
  digitalWrite(RightWaistEn, HIGH);
  digitalWrite(BackWaistEn, HIGH);
  digitalWrite(FrontWaistEn, HIGH);
  digitalWrite(LeftHipEn, HIGH);
  digitalWrite(RightHipEn, HIGH);
  digitalWrite(BackHipEn, HIGH);

  digitalWrite(RedLED, LOW);
  digitalWrite(YellowLED, LOW);
  digitalWrite(GreenLED, HIGH);

  // Sets the two pins as Output
  pinMode(NeckStep, OUTPUT);
  pinMode(NeckDir, OUTPUT);
  pinMode(LeftChestStep, OUTPUT);
  pinMode(LeftChestDir, OUTPUT);
  pinMode(RightChestStep, OUTPUT);
  pinMode(RightChestDir, OUTPUT);
  pinMode(BackChestStep, OUTPUT);
  pinMode(BackChestDir, OUTPUT);
  pinMode(FrontChestStep, OUTPUT);
  pinMode(FrontChestDir, OUTPUT);
  pinMode(LeftWaistStep, OUTPUT);
  pinMode(LeftWaistDir, OUTPUT);
  pinMode(RightWaistStep, OUTPUT);
  pinMode(RightWaistDir, OUTPUT);
  pinMode(BackWaistStep, OUTPUT);
  pinMode(BackWaistDir, OUTPUT);
  pinMode(FrontWaistStep, OUTPUT);
  pinMode(FrontWaistDir, OUTPUT);
  pinMode(LeftHipStep, OUTPUT);
  pinMode(LeftHipDir, OUTPUT);
  pinMode(RightHipStep, OUTPUT);
  pinMode(RightHipDir, OUTPUT);
  pinMode(BackHipStep, OUTPUT);
  pinMode(BackHipDir, OUTPUT);

  cli();//stop interrupts
  //set timer1 interrupt at 1Hz
  TCCR3A = 0;// set entire TCCR1A register to 0
  TCCR3B = 0;// same for TCCR1B
  TCNT3  = 0;//initialize counter value to 0
  // set compare match register for 1hz increments
  OCR3A = 3906;// = (16*10^6) / (1*1024) - 1 (must be <65536)
  // turn on CTC mode
  TCCR3B |= (1 << WGM12);
  // Set CS10 and CS12 bits for 1024 prescaler
  TCCR3B |= (1 << CS12) | (1 << CS10);
  // enable timer compare interrupt
  TIMSK3 |= (1 << OCIE3A);
  sei();//allow interrupts
}

ISR(TIMER3_COMPA_vect) {
  if (systemStateFlag == runningConcernFlag) {
    digitalWrite(RedLED, LOW);
    digitalWrite(GreenLED, LOW);
    digitalWrite(YellowLED, !digitalRead(YellowLED));
  }

  else if (systemStateFlag == showStopperFlag) {
    digitalWrite(YellowLED, LOW);
    digitalWrite(GreenLED, LOW);
    digitalWrite(RedLED, !digitalRead(RedLED));
  }
}

void showStopper() {
  digitalWrite(NeckEn, HIGH);
  digitalWrite(LeftChestEn, HIGH);
  digitalWrite(RightChestEn, HIGH);
  digitalWrite(BackChestEn, HIGH);
  digitalWrite(FrontChestEn, HIGH);
  digitalWrite(LeftWaistEn, HIGH);
  digitalWrite(RightWaistEn, HIGH);
  digitalWrite(BackWaistEn, HIGH);
  digitalWrite(FrontWaistEn, HIGH);
  digitalWrite(LeftHipEn, HIGH);
  digitalWrite(RightHipEn, HIGH);
  digitalWrite(BackHipEn, HIGH);

  systemStateFlag = showStopperFlag;
  totalSteps = 0;
  for (int i=0;i<12;i++) desiredStepNum[i] = 0;

  Serial.println("The system has encountered a Show Stopper level problem");
}

void runningConcern() {
  systemStateFlag = runningConcernFlag;

  Serial.println("The system has encountered a Running Concern level problem");
}

void moveOut() {

  digitalWrite(eN, LOW);
  //Serial.println("Move Out");
  digitalWrite(dIr, OUT); //moves out

  for (int i = 0; i < 50 * 16; i++) {
    //Serial.println("for");
    digitalWrite(steP, HIGH);
    delayMicroseconds(motSpd);
    digitalWrite(steP, LOW);
    delayMicroseconds(motSpd);

    //Serial.println(i);
  }
  //digitalWrite(eN,HIGH);

  // numOfSteps += 10;
  //  Serial.println(numOfSteps);
}

void moveIn() {
  numOfSteps = 0;

  digitalWrite(eN, LOW);
  digitalWrite(dIr, IN); //moves in

  int check = 0;
  while (check == 0) {
    Serial.println(check);
    if (digitalRead(liMit) == HIGH) {
      delay(50);
      if (digitalRead(liMit) == HIGH) {
        check = 1;
        Serial.println(check);
      }
    }
    Serial.println(check);
    digitalWrite(steP, HIGH);
    delayMicroseconds(motSpd);
    digitalWrite(steP, LOW);
    delayMicroseconds(motSpd);
  }
  //digitalWrite(eN,HIGH);
}

int incomingByte = 0;

void moveAllOut(float desiredCircumfrence[4]) {
  if((desiredCircumfrence[0] >= minCircumfrences[0]) &&
     (desiredCircumfrence[1] >= minCircumfrences[1]) &&
     (desiredCircumfrence[2] >= minCircumfrences[2]) &&
     (desiredCircumfrence[3] >= minCircumfrences[3]) &&
     (desiredCircumfrence[0] <= maxCircumfrences[0]) &&
     (desiredCircumfrence[1] <= maxCircumfrences[1]) &&
     (desiredCircumfrence[2] <= maxCircumfrences[2]) &&
     (desiredCircumfrence[3] <= maxCircumfrences[3]))
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
      activeCirRange[i] = maxCircumfrences[i] - minCircumfrences[i];
      desiredCirDiff[i] = desiredCircumfrence[i] - minCircumfrences[i];
      percentageMovement[i] = desiredCirDiff[i] / activeCirRange[i];
    }
  
    int pos = 0;
    desiredStepNum[0] = maxSteps[0] * percentageMovement[0];
    for (pos = 1; pos < 5; pos++) desiredStepNum[pos] = maxSteps[pos] * percentageMovement[1];
    for (; pos < 9; pos++) desiredStepNum[pos] = maxSteps[pos] * percentageMovement[2];
    for (; pos < 12; pos++) desiredStepNum[pos] = maxSteps[pos] * percentageMovement[3];
  
    totalSteps = 0;
    for (int i = 0; i < 12; i++) {
      //desiredStepNum[i] *= scalar;
      totalSteps += desiredStepNum[i];
  
      digitalWrite(dirPinNum, OUT);
      dirPinNum += 2;
    }
  
    int cnt = 0;
    while (totalSteps >= 1) {
      if (cnt >= 12){
        cnt = 0;
        ultrasonicRead();
      }
  
      if (stepPinNum >= 46) stepPinNum = 22;
  
      if (enablePinNum >= 54) {
        enablePinNum = 14;
      }
      else if (enablePinNum == 18) {
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
    Serial.println("The measurements provided cannot be achived because they are not within the active range of the mannequin");
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

  for (int i = 0; i < 12; i++) {
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
    }
    else if (enablePinNum == 18) {
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
    }
    else {
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
  while (swChk < 12);
  
  Serial.println("The mannequin is at its minimum");

  digitalWrite(RedLED, LOW);
  digitalWrite(YellowLED, LOW);
  digitalWrite(GreenLED, HIGH);
}

int currentCompareCnt[8];
int zeroValueCnt[8];
int tempUltrasonicValue[8];
float percentageQualifier = 0.005;

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
    }
    else{
      zeroValueCnt[i] = 0;
    }

    if(tempUltrasonicValue[i]-ultrasonicValue[i] <= ultrasonicValue[i]*percentageQualifier){
      currentCompareCnt[i]++;
    }
    else{
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

      Serial.println("yo papa");

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

void loop() {
  float desiredCircumfrence[4] = {374, 961, 799, 969};
  
  if (Serial.available() > 0) {
    incomingByte = Serial.read();
    const unsigned int MAX_MESSAGE_LENGTH = 12;
    static char message[MAX_MESSAGE_LENGTH];
    static unsigned int message_pos = 0;

    if ( incomingByte != '\n' && message_pos < MAX_MESSAGE_LENGTH - 1 ) {
      //Add the incoming byte to our message
      message[message_pos] = incomingByte;
      message_pos++;
    } else {
      //Add null character to string
      message[message_pos] = '\0';

      String msg = message;

      if (msg == "i") {
        Serial.println("Received i");
        moveIn();
        incomingByte = 0;
      } else if (msg == "o") {
        Serial.println("Received o");
        moveOut();
        incomingByte = 0;
      } else if (msg == "n") {
        Serial.println("Neck");
        steP = NeckStep;
        dIr = NeckDir;
        eN = NeckEn;
        liMit = NeckLimitSW;
      } else if (msg == "fc") {
        Serial.println("Front Chest");
        steP = FrontChestStep;
        dIr = FrontChestDir;
        eN = FrontChestEn;
        liMit = FrontChestLimitSW;
      } else if (msg == "lc") {
        Serial.println("Left Chest");
        steP = LeftChestStep;
        dIr = LeftChestDir;
        eN = LeftChestEn;
        liMit = LeftChestLimitSW;
      } else if (msg == "rc") {
        Serial.println("Right Chest");
        steP = RightChestStep;
        dIr = RightChestDir;
        eN = RightChestEn;
        liMit = RightChestLimitSW;
      } else if (msg == "bc") {
        Serial.println("Back Chest");
        steP = BackChestStep;
        dIr = BackChestDir;
        eN = BackChestEn;
        liMit = BackChestLimitSW;
      } else if (msg == "fw") {
        Serial.println("Front Waist");
        steP = FrontWaistStep;
        dIr = FrontWaistDir;
        eN = FrontWaistEn;
        liMit = FrontWaistLimitSW;
      } else if (msg == "lw") {
        Serial.println("Left Waist");
        steP = LeftWaistStep;
        dIr = LeftWaistDir;
        eN = LeftWaistEn;
        liMit = LeftWaistLimitSW;
      } else if (msg == "rw") {
        Serial.println("Right Waist");
        steP = RightWaistStep;
        dIr = RightWaistDir;
        eN = RightWaistEn;
        liMit = RightWaistLimitSW;
      } else if (msg == "bw") {
        Serial.println("Back Waist");
        steP = BackWaistStep;
        dIr = BackWaistDir;
        eN = BackWaistEn;
        liMit = BackWaistLimitSW;
      } else if (msg == "lh") {
        Serial.println("Left Hip");
        steP = LeftHipStep;
        dIr = LeftHipDir;
        eN = LeftHipEn;
        liMit = LeftHipLimitSW;
      } else if (msg == "rh") {
        Serial.println("Right Hip");
        steP = RightHipStep;
        dIr = RightHipDir;
        eN = RightHipEn;
        liMit = RightHipLimitSW;
      } else if (msg == "bh") {
        Serial.println("Back Hip");
        steP = BackHipStep;
        dIr = BackHipDir;
        eN = BackHipEn;
        liMit = BackHipLimitSW;
      }
      else if (msg == "ao") moveAllOut(desiredCircumfrence);
      else if (msg == "ai") moveAllIn();
      else if (msg == "reset"){
        Serial.println("Reset");
        systemStateFlag = noProblemsFlag;
      }

      //Reset for the next message
      message_pos = 0;
    }
  }
}
