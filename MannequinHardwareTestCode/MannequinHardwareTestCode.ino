#define OUT LOW
#define IN HIGH

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
  Serial.begin(9600);
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

  digitalWrite(RedLED, LOW);
  digitalWrite(YellowLED, LOW);
  digitalWrite(GreenLED, LOW);
}

void checkStopCmd(){
  while (Serial.available() > 0) {
    incomingByte = Serial.read();

    if ( incomingByte != '\n' && smessage_pos < MAX_MESSAGE_LENGTH - 1 ) {
      //Add the incoming byte to our message
      smessage[smessage_pos] = incomingByte;
      smessage_pos++;
    } else {
      //Add null character to string
      smessage[smessage_pos] = '\0';

      String smsg = smessage; 

      if(smsg == "stop"){
        stopCmdFlag = 1;
        Serial.println("The test has stopped");
      }
      smessage_pos = 0;
    }
  }
}

void testMotors(int steP,int dIr,int eN){
  Serial.println("The requested motor should be moving back and forth");

  digitalWrite(eN, LOW);
  
  while(stopCmdFlag == 0)
  { 
    digitalWrite(dIr, OUT); //moves out
    for (int i=0;i<stpGoal;i++) {
      digitalWrite(steP, HIGH);
      delayMicroseconds(500);
      digitalWrite(steP, LOW);
      delayMicroseconds(500);
    }  
  
    delay(100);
  
    digitalWrite(dIr, IN); //moves out
    for (int i=0;i<stpGoal;i++) {
      digitalWrite(steP, HIGH);
      delayMicroseconds(500);
      digitalWrite(steP, LOW);
      delayMicroseconds(500);
    }
  
    delay(100); 

    checkStopCmd();
  }

  digitalWrite(eN, HIGH);
}

void testLEDs(){
  Serial.println("The LEDs should be flashing at 2Hz");
  
  while(stopCmdFlag == 0){
    digitalWrite(RedLED, HIGH);
    digitalWrite(YellowLED, HIGH);
    digitalWrite(GreenLED, HIGH);
  
    delay(250);
  
    digitalWrite(RedLED, LOW);
    digitalWrite(YellowLED, LOW);
    digitalWrite(GreenLED, LOW);
  
    delay(250);

    checkStopCmd();
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
  
  while((limitSwitchCheckSum < 12) && (stopCmdFlag == 0)){
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

    checkStopCmd();
    
    for(int i=0;i<12;i++){
      limitSwitchCheckSum += limitSwitchCheck[i];
    }
    delay(250);
  }
}

void loop() {
  if (Serial.available() > 0) {
    incomingByte = Serial.read();
    
    if ( incomingByte != '\n' && message_pos < MAX_MESSAGE_LENGTH - 1 ) {
      //Add the incoming byte to our message
      message[message_pos] = incomingByte;
      message_pos++;
    } else {
      //Add null character to string
      message[message_pos] = '\0';
      
      String msg = message;
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
      message_pos = 0;
    }
  }
}
