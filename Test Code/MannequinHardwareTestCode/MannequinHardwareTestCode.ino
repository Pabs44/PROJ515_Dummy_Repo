#define OUT LOW
#define IN HIGH

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
  BackHipLimitSW, //13
};

enum LEDPins{
  RedLED = 18,
  YellowLED, //19
  GreenLED, //20
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
  FrontChestEn, //50
  LeftWaistEn, //51
  RightWaistEn, //52
  BackWaistEn, //53

  FrontWaistEn = 14, //14
  LeftHipEn, //15
  RightHipEn, //16
  BackHipEn, //17
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
  BottomBackRightEcho,
};

int stepDivision = 16;
int stpGoal = 25 * stepDivision;

int limitSwitchPinNum = 2;
int ultrasonicTrigPinNum = 54;
int ultrasonicEchoPinNum = 55;

int ultrasonicCheck[8];
int limitSwitchCheck[12];
int ultrasonicCheckSum;
int limitSwitchCheckSum;
int ultrasonicValue;

int reps = 0;

int stopCmdFlag = 0;
int incomingByte = 0;

const unsigned int MAX_MESSAGE_LENGTH = 12;
static char message[MAX_MESSAGE_LENGTH];
static unsigned int message_pos = 0;

static char smessage[MAX_MESSAGE_LENGTH];
static unsigned int smessage_pos = 0;

bool checkStopCmd(){
  while (Serial.available() > 0) {
    String smsg = Serial.readStringUntil('\n'); 

    if(smsg == "stop"){
      stopCmdFlag = 1;
      Serial.println("The test has stopped");
      return true;
    }
  }
}

void testMotors(int stepPin,int dirPin,int enPin){
  Serial.println("The requested motor should be moving back and forth");

  digitalWrite(enPin, LOW);
  
  while(!checkStopCmd()){ 
    digitalWrite(dirPin, OUT); //moves out
    for (int i=0;i<stpGoal;i++) {
      digitalWrite(stepPin, HIGH);
      delayMicroseconds(500);
      digitalWrite(stepPin, LOW);
      delayMicroseconds(500);
    }  
  
    delay(100);
  
    digitalWrite(dirPin, IN); //moves out
    for (int i=0;i<stpGoal;i++) {
      digitalWrite(stepPin, HIGH);
      delayMicroseconds(500);
      digitalWrite(stepPin, LOW);
      delayMicroseconds(500);
    }
  
    delay(100);
  }

  digitalWrite(enPin, HIGH);
}

void testLEDs(){
  Serial.println("The LEDs should be flashing at 2Hz");
  
  while(!checkStopCmd()){
    digitalWrite(RedLED, HIGH);
    digitalWrite(YellowLED, HIGH);
    digitalWrite(GreenLED, HIGH);
  
    delay(250);
  
    digitalWrite(RedLED, LOW);
    digitalWrite(YellowLED, LOW);
    digitalWrite(GreenLED, LOW);
  
    delay(250);
  }
}

void testUltrasonics(){
  reps = 0;
  
  while(ultrasonicCheckSum < 8){
    ultrasonicCheckSum = 0;
    ultrasonicTrigPinNum = 54;
    ultrasonicEchoPinNum = 55;

    for(int i=0; i<8; i++){
      ultrasonicCheck[i] = 0;
    }
    
    for(int i=0;i<8;i++){
      digitalWrite(ultrasonicTrigPinNum, LOW);
      delayMicroseconds(2);
      digitalWrite(ultrasonicTrigPinNum, HIGH);
      delayMicroseconds(10);
      digitalWrite(ultrasonicTrigPinNum, LOW);
      delayMicroseconds(20);
      
      ultrasonicValue = pulseIn(ultrasonicEchoPinNum, HIGH);
      delay(25);
      
      if(ultrasonicValue > 0){
        ultrasonicCheck[i] = 1;
      }

      ultrasonicTrigPinNum += 2;
      ultrasonicEchoPinNum += 2;
    }

    for(int i=0;i<8;i++){
      ultrasonicCheckSum += ultrasonicCheck[i];
    }
    
    if(reps >= 10) break;
    else reps++;
  }

  if(ultrasonicCheckSum >= 8) Serial.println("All ultrasonics are functioning");
  else{
    if(ultrasonicCheck[0] == 0){
      Serial.println("The Top Front Left Ultrasonic sensor is repeatedly returning a 0 value.");
      Serial.println("The probable causes are that the ultrasonic sensor has been disconnected or it is inoperable.");  
    }
    if(ultrasonicCheck[1] == 0){
      Serial.println("The Top Front Right Ultrasonic sensor is repeatedly returning a 0 value.");
      Serial.println("The probable causes are that the ultrasonic sensor has been disconnected or it is inoperable.");
    }
    if(ultrasonicCheck[2] == 0){
      Serial.println("The Top Back Left Ultrasonic sensor is repeatedly returning a 0 value.");
      Serial.println("The probable causes are that the ultrasonic sensor has been disconnected or it is inoperable.");
    }
    if(ultrasonicCheck[3] == 0){
      Serial.println("The Top Back Right Ultrasonic sensor is repeatedly returning a 0 value.");
      Serial.println("The probable causes are that the ultrasonic sensor has been disconnected or it is inoperable.");
    }
    if(ultrasonicCheck[4] == 0){
      Serial.println("The Bottom Front Left Ultrasonic sensor is repeatedly returning a 0 value.");
      Serial.println("The probable causes are that the ultrasonic sensor has been disconnected or it is inoperable.");
    }
    if(ultrasonicCheck[5] == 0){
      Serial.println("The Bottom Front Right Ultrasonic sensor is repeatedly returning a 0 value.");
      Serial.println("The probable causes are that the ultrasonic sensor has been disconnected or it is inoperable.");
    }
    if(ultrasonicCheck[6] == 0){
      Serial.println("The Bottom Back Left Ultrasonic sensor is repeatedly returning a 0 value.");
      Serial.println("The probable causes are that the ultrasonic sensor has been disconnected or it is inoperable.");
    }
    if(ultrasonicCheck[7] == 0){
      Serial.println("The Bottom Back Right Ultrasonic sensor is repeatedly returning a 0 value.");
      Serial.println("The probable causes are that the ultrasonic sensor has been disconnected or it is inoperable.");
    }
  }
}

void testLimitSwitch(){
  for(int i=0;i<12;i++){
    limitSwitchCheck[i] = 0;
  }
  limitSwitchCheckSum = 0;
  
  while((limitSwitchCheckSum < 12) && !checkStopCmd()){
    limitSwitchPinNum = 2;
    limitSwitchCheckSum = 0;
    
    for(int i=0;i<12;i++){
      if((digitalRead(limitSwitchPinNum) == 1) && (limitSwitchCheck[i] == 0)){
        limitSwitchCheck[i] = 1;
        
        switch(i){
          case 0:
            Serial.println("The Neck Limit Switch has been pressed");
            break;
          case 1:
            Serial.println("The Left Chest Limit Switch has been pressed");
            break;
          case 2:
            Serial.println("The Right Chest Limit Switch has been pressed");
            break;
          case 3:
            Serial.println("The Back Chest Limit Switch has been pressed");
            break;
          case 4:
            Serial.println("The Front Chest Limit Switch has been pressed");
            break;
          case 5:
            Serial.println("The Left Waist Limit Switch has been pressed");
            break;
          case 6:
            Serial.println("The Right Waist Limit Switch has been pressed");
            break;
          case 7:
            Serial.println("The Back Waist Limit Switch has been pressed");
            break;
          case 8:
            Serial.println("The Front Waist Limit Switch has been pressed");
            break;
          case 9:
            Serial.println("The Left Hip Limit Switch has been pressed");
            break;
          case 10:
            Serial.println("The Right Hip Limit Switch has been pressed");
            break;
          case 11:
            Serial.println("The Back Hip Limit Switch has been pressed");
            break;
        }
      }
      limitSwitchPinNum++;
    }
    
    for(int i=0;i<12;i++){
      limitSwitchCheckSum += limitSwitchCheck[i];
    }
    delay(250);
  }
}

void setup() {
  Serial.begin(9600);
  Serial.println("Start");

  for(int i = NeckLimitSW; i < BackHipLimitSW; i++) pinMode(limitSwitchPins(i), INPUT);

  for(int i = NeckStep; i < BackHipDir; i++) pinMode(stepperMotorPins(i), OUTPUT);
  for(int i = NeckEn; i < BackWaistEn; i++){
    pinMode(stepperMotorPins(i), OUTPUT);
    digitalWrite(stepperMotorPins(i), HIGH);
  }
  for(int i = FrontWaistEn; i < BackHipEn; i++){
    pinMode(stepperMotorPins(i), OUTPUT);
    digitalWrite(stepperMotorPins(i), HIGH);
  }

  for(int i = RedLED; i < GreenLED; i++){
    pinMode(LEDPins(i), OUTPUT);
    digitalWrite(LEDPins(i), LOW);
  }
  digitalWrite(GreenLED, HIGH);

  for(int i = TopFrontLeftTrig; i < BottomBackRightTrig; i+=2) pinMode(ultrasonicSensorPins(i), OUTPUT);
  for(int i = TopFrontLeftEcho; i < BottomBackRightEcho; i+=2) pinMode(ultrasonicSensorPins(i), INPUT);
}

void loop() {
  if (Serial.available() > 0) {
    String msg = Serial.readStringUntil('\n');
    stopCmdFlag = 0;
    
    if (msg == "n"){
      Serial.println("The Neck motor has been selected");
      testMotors(NeckStep,NeckDir,NeckEn);
    } 
    else if (msg == "fc"){
      Serial.println("The Front Chest motor has been selected");
      testMotors(FrontChestStep,FrontChestDir,FrontChestEn);
    } 
    else if (msg == "lc"){
      Serial.println("The Left Chest motor has been selected");
      testMotors(LeftChestStep,LeftChestDir,LeftChestEn);
    } 
    else if (msg == "rc"){
      Serial.println("The Right Chest motor has been selected");
      testMotors(RightChestStep,RightChestDir,RightChestEn);
    } 
    else if (msg == "bc"){
      Serial.println("The Back Chest motor has been selected");
      testMotors(BackChestStep,BackChestDir,BackChestEn);
    } 
    else if (msg == "fw"){
      Serial.println("The Front Waist motor has been selected");
      testMotors(FrontWaistStep,FrontWaistDir,FrontWaistEn);
    } 
    else if (msg == "lw"){
      Serial.println("The Left Waist motor has been selected");
      testMotors(LeftWaistStep,LeftWaistDir,LeftWaistEn);
    } 
    else if (msg == "rw"){
      Serial.println("The Right Waist motor has been selected");
      testMotors(RightWaistStep,RightWaistDir,RightWaistEn);
    } 
    else if (msg == "bw"){
      Serial.println("The Back Waist motor has been selected");
      testMotors(BackWaistStep,BackWaistDir,BackWaistEn);
    } 
    else if (msg == "lh"){
      Serial.println("The Left Hip motor has been selected");
      testMotors(LeftHipStep,LeftHipDir,LeftHipEn);
    } 
    else if (msg == "rh"){
      Serial.println("The Right Hip motor has been selected");
      testMotors(RightHipStep,RightHipDir,RightHipEn);
    } 
    else if (msg == "bh"){
      Serial.println("The Back Hip motor has been selected");
      testMotors(BackHipStep,BackHipDir,BackHipEn);
    }
    else if(msg == "leds"){
      Serial.println("The LEDs have been selected to be tested");
      testLEDs();
    }
    else if(msg == "us"){
      Serial.println("The Ultrasonic Sensors have been selected to be tested");
      testUltrasonics();
    }
    else if(msg == "ls"){
      Serial.println("The Limit Switches have been selected to be tested");
      testLimitSwitch();
    }
  }
}
