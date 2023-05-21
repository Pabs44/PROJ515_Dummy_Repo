String data;
char d1;
String x;
int mm;

//Test example pins, use the other pins for the actual test
const int dirPin = 3;
const int stepPin = 4;
const int switchPin = 2;
const int enPin = 5;
const int MS1 = 11;
const int MS2 = 10;
const int MS3 = 9;

//const int stepPin = 22;
//const int dirPin = 23;
//const int limitPin = 2;
#define OUT LOW
#define IN HIGH

typedef enum {
  noProblemsFlag = 0,
  runningConcernFlag,
  showStopperFlag,
} LEDStates;

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

  FrontWaistEn = 14, //14
  LeftHipEn, //15
  RightHipEn, //16
  BackHipEn, //17
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

int currentCompareCnt[8];
int zeroValueCnt[8];
int tempUltrasonicValue[8];
float percentageQualifier = 0.005;

int incomingByte = 0;

int systemStateFlag = noProblemsFlag;

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

void motorForwards(int numberOfCycles, int directionPin, int stepPin) {
  digitalWrite(directionPin, LOW);
  for (numberOfCycles; numberOfCycles > 0; numberOfCycles--) {
    for (int x = 0; x < 20; x++) {
      digitalWrite(stepPin, HIGH);
      delayMicroseconds(500);
      digitalWrite(stepPin, LOW);
      delayMicroseconds(500);
    }
    delay(10);
  }
}

void motorBackwards(int numberOfCycles, int directionPin, int stepPin) {
  digitalWrite(directionPin, HIGH);                                              //Select the direction 
  for (numberOfCycles; numberOfCycles > 0; numberOfCycles--) {
    for (int x = 0; x < 20; x++) {
      digitalWrite(stepPin, HIGH);
      delayMicroseconds(500);
      digitalWrite(stepPin, LOW);
      delayMicroseconds(500);
    }
    delay(10);
  }
}
//Move the mechanism 1mm
void motorForwards1mm(float distanceMil, int directionPin, int stepPin) {
  digitalWrite(directionPin, LOW);
  for (distanceMil; distanceMil > 0; distanceMil--) {
    for (int x = 0; x < 10; x++) {  //110 steps = 1mm in of the mechanism displacement
      digitalWrite(stepPin, HIGH);
      delayMicroseconds(500);
      digitalWrite(stepPin, LOW);
      delayMicroseconds(500);
    }
    delay(10);
  }
}

//Move the mechanism 1mm
void motorBackwards1mm(float distanceMil, int directionPin, int stepPin) {
  digitalWrite(directionPin, HIGH);
  for (distanceMil; distanceMil > 0; distanceMil--) {
    for (int x = 0; x < 10; x++) {  //110 steps = 1mm in of the mechanism displacement
      digitalWrite(stepPin, HIGH);
      delayMicroseconds(500);
      digitalWrite(stepPin, LOW);
      delayMicroseconds(500);
    }
    delay(10);
  }
}

void checkSwitch(int switchPin, int directionPin, int stepPin) {
  int check = 0;
  while (check == 0) {
    if (digitalRead(switchPin) == 1) {  //If the  switch is not triggered
      delay(50);
      if (digitalRead(switchPin) == 1) {  //When triggered, stop spinning
        check = 1;
        Serial.println(check);
      }
    } else {
      motorBackwards(1, directionPin, stepPin);
      Serial.println(check);
    }
  }
  check = 0;
}
void checkSwitch1(int switchPin, int directionPin, int stepPin) {
  int check = 0;
  while (check == 0) {
    if (digitalRead(switchPin) == 1) {  //If the  switch is not triggered
      delay(50);
      if (digitalRead(switchPin) == 1) {  //When triggered, stop spinning
        check = 1;
        Serial.println(check);
      }
    } else {
      motorForwards(1, directionPin, stepPin);
      Serial.println(check);
    }
  }
  check = 0;
}

void moveAllIn() {
  digitalWrite(RedLED, HIGH);
  digitalWrite(YellowLED, LOW);
  digitalWrite(GreenLED, LOW);

  int stepPinNum = 22;
  int dirPinNum = 23;
  int limitSWPinNum = 2;
  int enablePinNum = 46;

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

void setup() {
  Serial.begin(9600);
  
  //Set all the pin modes to OUTPUT
  pinMode(NeckStep,OUTPUT);
  pinMode(NeckDir,OUTPUT);
  pinMode(LeftChestStep,OUTPUT);
  pinMode(LeftChestDir,OUTPUT);
  pinMode(RightChestStep,OUTPUT);
  pinMode(RightChestDir,OUTPUT);
  pinMode(BackChestStep,OUTPUT);
  pinMode(BackChestDir,OUTPUT);
  pinMode(FrontChestStep,OUTPUT);
  pinMode(FrontChestDir,OUTPUT);
  pinMode(LeftWaistStep,OUTPUT);
  pinMode(LeftWaistDir,OUTPUT);
  pinMode(RightWaistStep,OUTPUT);
  pinMode(RightWaistDir,OUTPUT);
  pinMode(BackWaistStep,OUTPUT);
  pinMode(BackWaistDir,OUTPUT);
  pinMode(FrontWaistStep,OUTPUT);
  pinMode(FrontWaistDir,OUTPUT);
  pinMode(LeftHipStep,OUTPUT);
  pinMode(LeftHipDir,OUTPUT);
  pinMode(RightHipStep,OUTPUT);
  pinMode(RightHipDir,OUTPUT);
  pinMode(BackHipStep,OUTPUT);
  pinMode(BackHipDir,OUTPUT);
  pinMode(LeftWaistDir,OUTPUT);
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
  pinMode(NeckLimitSW, OUTPUT);
  pinMode(LeftChestLimitSW, OUTPUT);
  pinMode(RightChestLimitSW, OUTPUT);
  pinMode(BackChestLimitSW, OUTPUT);
  pinMode(FrontChestLimitSW, OUTPUT);
  pinMode(LeftWaistLimitSW, OUTPUT);
  pinMode(RightWaistLimitSW, OUTPUT);
  pinMode(BackWaistLimitSW, OUTPUT);
  pinMode(FrontWaistLimitSW, OUTPUT);
  pinMode(LeftHipLimitSW, OUTPUT);
  pinMode(RightHipLimitSW, OUTPUT);
  pinMode(BackHipLimitSW, OUTPUT);

  //Set the direction and step pin LOW
  digitalWrite(NeckStep,LOW);
  digitalWrite(NeckDir,LOW);
  digitalWrite(LeftChestStep,LOW);
  digitalWrite(LeftChestDir,LOW);
  digitalWrite(RightChestStep,LOW);
  digitalWrite(RightChestDir,LOW);
  digitalWrite(BackChestStep,LOW);
  digitalWrite(BackChestDir,LOW);
  digitalWrite(FrontChestStep,LOW);
  digitalWrite(FrontChestDir,LOW);
  digitalWrite(LeftWaistStep,LOW);
  digitalWrite(LeftWaistDir,LOW);
  digitalWrite(RightWaistStep,LOW);
  digitalWrite(RightWaistDir,LOW);
  digitalWrite(BackWaistStep,LOW);
  digitalWrite(BackWaistDir,LOW);
  digitalWrite(FrontWaistStep,LOW);
  digitalWrite(FrontWaistDir,LOW);
  digitalWrite(LeftHipStep,LOW);
  digitalWrite(LeftHipDir,LOW);
  digitalWrite(RightHipStep,LOW);
  digitalWrite(RightHipDir,LOW);
  digitalWrite(BackHipStep,LOW);
  digitalWrite(BackHipDir,LOW);

  //Set all the enable pins high to disable the motors
  digitalWrite(NeckEn, HIGH);
  digitalWrite(LeftChestEn, HIGH);
  digitalWrite(RightChestEn, HIGH);
  digitalWrite(BackChestEn, HIGH);
  digitalWrite(FrontChestEn, LOW);
  digitalWrite(LeftWaistEn, HIGH);
  digitalWrite(RightWaistEn, HIGH);
  digitalWrite(BackWaistEn, HIGH);
  digitalWrite(FrontWaistEn, HIGH);
  digitalWrite(LeftHipEn, HIGH);
  digitalWrite(RightHipEn, HIGH);
  digitalWrite(BackHipEn, HIGH);
}
void loop() {

  if (Serial.available()) {
    data = Serial.readString();
    d1 = data.charAt(0);

    switch (d1) {  //Select the action based on the first character

      case 'M':
        moveAllIn();
        break;
      
      case 'A'://Just testing
        x = data.substring(1);
        mm = x.toInt();
        digitalWrite(enPin, LOW);
        motorForwards1mm(mm, dirPin, stepPin);
        digitalWrite(enPin, HIGH);
        break;
      case 'a': //Just testing
        checkSwitch(switchPin, dirPin, stepPin);
        break;
      
      case 'B'://Enter the mm front chest
        x = data.substring(1);
        mm = x.toInt();
        //digitalWrite(FrontChestEn, LOW);
        motorForwards1mm(mm, FrontChestDir, FrontChestStep);
        //digitalWrite(FrontChestEn, HIGH);
        break;
      case 'b'://In a number of steps
        x = data.substring(1);
        mm = x.toInt();
        digitalWrite(FrontChestEn, LOW);
        motorBackwards1mm(mm, FrontChestDir, FrontChestStep);
        digitalWrite(FrontChestEn, HIGH);
        break;
      case 'C'://Front Waist out
        x = data.substring(1);
        mm = x.toInt();
        digitalWrite(FrontWaistEn, LOW);
        motorForwards1mm(mm, FrontWaistDir, FrontWaistStep);
        digitalWrite(FrontWaistEn, HIGH);
        break;
      case 'c'://Front Waist in
        x = data.substring(1);
        mm = x.toInt();
        digitalWrite(FrontWaistEn, LOW);
        motorBackwards1mm(mm, FrontWaistDir, FrontWaistStep);
        digitalWrite(FrontWaistEn, HIGH);
        break;
      case 'D'://Left Chest out
        x = data.substring(1);
        mm = x.toInt();
        digitalWrite(LeftChestEn, LOW);
        motorForwards1mm(mm, LeftChestDir, LeftChestStep);
        digitalWrite(LeftChestEn, HIGH);
        break;
      case 'd'://Left Chest in
        x = data.substring(1);
        mm = x.toInt();
        digitalWrite(LeftChestEn, LOW);
        motorBackwards1mm(mm, LeftChestDir, LeftChestStep);
        digitalWrite(LeftChestEn, HIGH);
        break;
      case 'E'://Left Waist out
        x = data.substring(1);
        mm = x.toInt();
        digitalWrite(LeftWaistEn, LOW);
        motorForwards1mm(mm, LeftWaistDir, LeftWaistStep);
        digitalWrite(LeftWaistEn, HIGH);
        break;
      case 'e'://Left Waist in
        x = data.substring(1);
        mm = x.toInt();
        digitalWrite(LeftWaistEn, LOW);
        motorBackwards1mm(mm, LeftWaistDir, LeftWaistStep);
        digitalWrite(LeftWaistEn, HIGH);
        break;
      case 'F'://Left Hips out
        x = data.substring(1);
        mm = x.toInt();
        digitalWrite(LeftHipEn, LOW);
        motorBackwards1mm(mm, LeftHipDir, LeftHipStep);
        digitalWrite(LeftHipEn, HIGH);
        break;
      case 'f'://Left Hips in
        x = data.substring(1);
        mm = x.toInt();
        digitalWrite(LeftHipEn, LOW);
        motorForwards1mm(mm, LeftHipDir, LeftHipStep);
        digitalWrite(LeftHipEn, HIGH);
        break;
      case 'G'://Right Chest out
        x = data.substring(1);
        mm = x.toInt();
        digitalWrite(RightChestEn, LOW);
        motorForwards1mm(mm, RightChestDir, RightChestStep);
        digitalWrite(RightChestEn, HIGH);
        break;
      case 'g'://Right Chest in
        x = data.substring(1);
        mm = x.toInt();
        digitalWrite(RightChestEn, LOW);
        motorBackwards1mm(mm, RightChestDir, RightChestStep);
        digitalWrite(RightChestEn, HIGH);
        break;
      case 'H'://Right Waist out
        x = data.substring(1);
        mm = x.toInt();
        digitalWrite(RightWaistEn, LOW);
        motorForwards1mm(mm, RightWaistDir, RightWaistStep);
        digitalWrite(RightWaistEn, HIGH);
        break;
      case 'h'://Right Waist in
        x = data.substring(1);
        mm = x.toInt();
        digitalWrite(RightWaistEn, LOW);
        motorBackwards1mm(mm, RightWaistDir, RightWaistStep);
        digitalWrite(RightWaistEn, HIGH);
        break;
      case 'I'://Right Hip out
        x = data.substring(1);
        mm = x.toInt();
        digitalWrite(RightHipEn, LOW);
        motorForwards1mm(mm, RightHipDir, RightHipStep);
        digitalWrite(RightHipEn, HIGH);
        break;
      case 'i'://Right Hip in
        x = data.substring(1);
        mm = x.toInt();
        digitalWrite(RightHipEn, LOW);
        motorBackwards1mm(mm, RightHipDir, RightHipStep);
        //digitalWrite(RightHipEn, HIGH);
        break;
      case 'J'://Back Chest out
        x = data.substring(1);
        mm = x.toInt();
        digitalWrite(BackChestEn, LOW);
        motorForwards1mm(mm, BackChestDir, BackChestStep);
        digitalWrite(BackChestEn, HIGH);
        break;
      case 'j'://Back Chest in
        x = data.substring(1);
        mm = x.toInt();
        digitalWrite(BackChestEn, LOW);
        motorBackwards1mm(mm, BackChestDir, BackChestStep);
        digitalWrite(BackChestEn, HIGH);
        break;
      case 'K'://Back Waist out
        x = data.substring(1);
        mm = x.toInt();
        digitalWrite(BackWaistEn, LOW);
        motorForwards1mm(mm, BackWaistDir, BackWaistStep);
        digitalWrite(BackWaistEn, HIGH);
        break;
      case 'k'://Back Waist in
        x = data.substring(1);
        mm = x.toInt();
        digitalWrite(BackWaistEn, LOW);
        motorBackwards1mm(mm, BackWaistDir, BackWaistStep);
        digitalWrite(BackWaistEn, HIGH);
        break;
      case 'L'://Back Hip out
        x = data.substring(1);
        mm = x.toInt();
        digitalWrite(BackHipEn, LOW);
        motorForwards1mm(mm, BackHipDir, BackHipStep);
        digitalWrite(BackHipEn, HIGH);
        break;
      case 'l'://Back Hip in
        x = data.substring(1);
        mm = x.toInt();
        digitalWrite(BackHipEn, LOW);
        motorBackwards1mm(mm, BackHipDir, BackHipStep);
        digitalWrite(BackHipEn, HIGH);
        break;
      case 'Z'://Enter the mm hips back

        x = data.substring(1);
        mm = x.toInt();
        digitalWrite(NeckEn, LOW);
        motorForwards1mm(mm, NeckDir, NeckStep);
        digitalWrite(NeckEn, HIGH);
        break;
      case 'z'://Enter the mm hips back

        x = data.substring(1);
        mm = x.toInt();
        digitalWrite(NeckEn, LOW);
        motorBackwards1mm(mm, NeckDir, NeckStep);
        digitalWrite(NeckEn, HIGH);
        break;
      case 'm': //Set front chest to minimum
        //digitalWrite(FrontChestEn, LOW);
        checkSwitch(FrontChestLimitSW, FrontChestDir, FrontChestStep);
        //digitalWrite(FrontChestEn, HIGH);
        break;
      case 'n': //Set waist chest to minimum
        digitalWrite(FrontWaistEn, LOW);
        checkSwitch(FrontWaistLimitSW, FrontWaistDir, FrontWaistStep);
        digitalWrite(FrontWaistEn, HIGH);
        break;

      case 'o': //Set left chest to minimum
        digitalWrite(LeftChestEn, LOW);
        checkSwitch(LeftChestLimitSW, LeftChestDir, LeftChestStep);
        digitalWrite(LeftChestEn, HIGH);
        break;
      case 'p': //Set left waist to minimum
        digitalWrite(LeftWaistEn, LOW);
        checkSwitch(LeftWaistLimitSW, LeftWaistDir, LeftWaistStep);
        digitalWrite(LeftWaistEn, HIGH);
        break;
      case 'q': //Set left hips to minimum
        digitalWrite(LeftHipEn, LOW);
        checkSwitch1(LeftHipLimitSW, LeftHipDir, LeftHipStep);
        digitalWrite(LeftHipEn, HIGH);
        break;
      case 'r': //Set right chest to minimum
        digitalWrite(RightChestEn, LOW);
        checkSwitch(RightChestLimitSW, RightChestDir, RightChestStep);
        digitalWrite(RightChestEn, HIGH);
        break;
      case 's': //Set right waist to minimum
        digitalWrite(RightWaistEn, LOW);
        checkSwitch(RightWaistLimitSW, RightWaistDir, RightWaistStep);
        digitalWrite(RightWaistEn, HIGH);
        break;
      case 't': //Set right hips to minimum
        digitalWrite(RightHipEn, LOW);
        checkSwitch(RightHipLimitSW, RightHipDir, RightHipStep);
        digitalWrite(RightHipEn, HIGH);
        break;
      case 'u': //Set back chest to minimum
        digitalWrite(BackChestEn, LOW);
        checkSwitch(BackChestLimitSW, BackChestDir, BackChestStep);
        digitalWrite(BackChestEn, HIGH);
        break;
      case 'v': //Set back waist to minimum
        digitalWrite(BackWaistEn, LOW);
        checkSwitch(BackWaistLimitSW, BackWaistDir, BackWaistStep);
        digitalWrite(BackWaistEn, HIGH);
        break;
      case 'w': //Set back hips to minimum
        digitalWrite(BackHipEn, LOW);
        checkSwitch(BackHipLimitSW, BackHipDir, BackHipStep);
        digitalWrite(BackHipEn, HIGH);
        break;
    }
  }
}
