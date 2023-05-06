//const int stepPin = 22; 
//const int dirPin = 23; 
//const int limitPin = 2;
#define OUT LOW
#define IN HIGH

int steP;
int dIr;
int eN;
int liMit;
int stpGoal = 1 * 16;

int stepPinNum = 22;
int dirPinNum = 23;
int limitSWPinNum = 2;
int enablePinNum = 46;
int ultrasonicTrigPinNum = 54;
int ultrasonicEchoPinNum = 55;
int ultrasonicFlag;

int RedLEDState = 0;
int YellowLEDState = 0;
int GreenLEDState = 1;

long minimumUltrasonicValue[8];
long ultrasonicValue[8];

int numOfSteps = 0;

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
  YellowLED,
  GreenLED,
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
  
  pinMode(NeckLimitSW,INPUT);
  pinMode(LeftChestLimitSW,INPUT);
  pinMode(RightChestLimitSW,INPUT);
  pinMode(BackChestLimitSW,INPUT);
  pinMode(FrontChestLimitSW,INPUT);
  pinMode(LeftWaistLimitSW,INPUT);
  pinMode(RightWaistLimitSW,INPUT);
  pinMode(BackWaistLimitSW,INPUT);
  pinMode(FrontWaistLimitSW,INPUT);
  pinMode(LeftHipLimitSW,INPUT);
  pinMode(RightHipLimitSW,INPUT);
  pinMode(BackHipLimitSW,INPUT); 

  pinMode(NeckEn,OUTPUT);
  pinMode(LeftChestEn,OUTPUT);
  pinMode(RightChestEn,OUTPUT);
  pinMode(BackChestEn,OUTPUT);
  pinMode(FrontChestEn,OUTPUT);
  pinMode(LeftWaistEn,OUTPUT);
  pinMode(RightWaistEn,OUTPUT);
  pinMode(BackWaistEn,OUTPUT);
  pinMode(FrontWaistEn,OUTPUT);
  pinMode(LeftHipEn,OUTPUT);
  pinMode(RightHipEn,OUTPUT);
  pinMode(BackHipEn,OUTPUT);

  pinMode(RedLED,OUTPUT);
  pinMode(YellowLED,OUTPUT);
  pinMode(GreenLED,OUTPUT);

  pinMode(TopFrontLeftTrig,OUTPUT);
  pinMode(TopFrontLeftEcho,INPUT);
  pinMode(TopFrontRightTrig,OUTPUT);
  pinMode(TopFrontRightEcho,INPUT);
  pinMode(TopBackLeftTrig,OUTPUT);
  pinMode(TopBackLeftEcho,INPUT);
  pinMode(TopBackRightTrig,OUTPUT);
  pinMode(TopBackRightEcho,INPUT);
  pinMode(BottomFrontLeftTrig,OUTPUT);
  pinMode(BottomFrontLeftEcho,INPUT);
  pinMode(BottomFrontRightTrig,OUTPUT);
  pinMode(BottomFrontRightEcho,INPUT);
  pinMode(BottomBackLeftTrig,OUTPUT);
  pinMode(BottomBackLeftEcho,INPUT);
  pinMode(BottomBackRightTrig,OUTPUT);
  pinMode(BottomBackRightEcho,INPUT);

  digitalWrite(NeckEn,HIGH);
  digitalWrite(LeftChestEn,HIGH);
  digitalWrite(RightChestEn,HIGH);
  digitalWrite(BackChestEn,HIGH);
  digitalWrite(FrontChestEn,HIGH);
  digitalWrite(LeftWaistEn,HIGH);
  digitalWrite(RightWaistEn,HIGH);
  digitalWrite(BackWaistEn,HIGH);
  digitalWrite(FrontWaistEn,HIGH);
  digitalWrite(LeftHipEn,HIGH);
  digitalWrite(RightHipEn,HIGH);
  digitalWrite(BackHipEn,HIGH);

  digitalWrite(RedLED,LOW);
  digitalWrite(YellowLED,LOW);
  digitalWrite(GreenLED,HIGH);
  
  // Sets the two pins as Output
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

ISR(TIMER3_COMPA_vect){
  digitalWrite(RedLED,!digitalRead(RedLED));
  digitalWrite(YellowLED,!digitalRead(YellowLED));
}

void oscillate(){
  digitalWrite(eN,LOW);
  
  digitalWrite(dIr,LOW);
  for(int i=0;i<75;i++){
    //Serial.println("for");
    digitalWrite(steP,HIGH);
    delayMicroseconds(500);
    digitalWrite(steP,LOW);
    delayMicroseconds(500);

    //Serial.println(i);
  }

  digitalWrite(eN,HIGH);
  delay(1000);

  digitalWrite(eN,LOW);
  
  digitalWrite(dIr,HIGH);
  for(int i=0;i<75;i++){
    //Serial.println("for");
    digitalWrite(steP,HIGH);
    delayMicroseconds(500);
    digitalWrite(steP,LOW);
    delayMicroseconds(500);
  
    //Serial.println(i);
  }
  digitalWrite(eN,HIGH);
  delay(1000);
}

void moveOut(){

  digitalWrite(eN,LOW);
  //Serial.println("Move Out");
  digitalWrite(dIr,OUT); //moves out
  
  for(int i=0;i<50*16;i++){
    //Serial.println("for");
    digitalWrite(steP,HIGH);
    delayMicroseconds(500);
    digitalWrite(steP,LOW);
    delayMicroseconds(500);

    //Serial.println(i);
  }
  //digitalWrite(eN,HIGH);

 // numOfSteps += 10;
//  Serial.println(numOfSteps); 
}

void moveIn(){
  numOfSteps = 0;
  
  digitalWrite(eN,LOW);
  digitalWrite(dIr,IN); //moves in
  
  int check = 0;
  while(check == 0){
    Serial.println(check);
    if(digitalRead(liMit) == HIGH){
      delay(50);
      if(digitalRead(liMit) == HIGH){
        check = 1;
        Serial.println(check);
      }
    }
    Serial.println(check);
    digitalWrite(steP,HIGH);
    delayMicroseconds(500);
    digitalWrite(steP,LOW);
    delayMicroseconds(500);
  }
  //digitalWrite(eN,HIGH);  
}

int incomingByte = 0;

void moveAllOut(){
  
  stepPinNum = 22;
  dirPinNum = 23;
  enablePinNum = 46;
  //Neck Steps: 100
  //LeftChest: 110
  //RightChest: 80
  //BackChest: 80
  //FrontChest:40
  //Left Waist: 20
  //Right Waist: 30
  //BackWaist:20
  //FrontWaist: 40
  //LeftHip: 50
  //RightHip: 30
  //BackHip: 20
  float scalar = 1.1;
  int desiredStepNum[12] = {100*16,110*16,80*16,80*16,40*16,20*16,30*16,20*16,40*16,50*16,30*16,20*16};

  
  int totalSteps = 0;
  for(int i=0;i<12;i++){
    //desiredStepNum[i] *= scalar;
    totalSteps += desiredStepNum[i];

    digitalWrite(dirPinNum,OUT);
    dirPinNum += 2;
  }
   
  int cnt = 0;
  while(totalSteps >= 1){
    if(cnt >= 12){
      cnt = 0;
    }

    if(stepPinNum >= 46){
      stepPinNum = 22;
    }

    if(enablePinNum >= 54){
      enablePinNum = 14;
    }
    else if(enablePinNum == 18){
      enablePinNum = 46;
    }

    if(desiredStepNum[cnt] >= 1){
      digitalWrite(enablePinNum,LOW);

      for(int i=0;i<stpGoal;i++){
        digitalWrite(stepPinNum,HIGH); 
        delayMicroseconds(1000); 
        digitalWrite(stepPinNum,LOW); 
        delayMicroseconds(1000); 
      }
      
      if(stepPinNum != FrontChestStep) digitalWrite(enablePinNum,HIGH);

      desiredStepNum[cnt] -= stpGoal;
      totalSteps -= stpGoal;   
    }

    cnt++;
    stepPinNum += 2;
    enablePinNum++;
  }
  Serial.println("Manneqin has moved to your desired measurements");
}

void moveAllIn(){
  stepPinNum = 22;
  dirPinNum = 23;
  limitSWPinNum = 2;
  enablePinNum = 46;
  
  for(int i=0;i<12;i++){
    digitalWrite(dirPinNum,IN); 
    dirPinNum += 2;
  }

  int cnt = 0;
  while((digitalRead(NeckLimitSW) == 0)
  || (digitalRead(LeftChestLimitSW) == 0)
  || (digitalRead(RightChestLimitSW) == 0)
  || (digitalRead(BackChestLimitSW) == 0)
  || (digitalRead(FrontChestLimitSW) == 0)
  || (digitalRead(LeftWaistLimitSW) == 0)
  || (digitalRead(RightWaistLimitSW) == 0)
  || (digitalRead(BackWaistLimitSW) == 0)
  || (digitalRead(FrontWaistLimitSW) == 0)
  || (digitalRead(LeftHipLimitSW) == 0)
  || (digitalRead(RightHipLimitSW) == 0)
  || (digitalRead(BackHipLimitSW) == 0)
  ){
    
    if(limitSWPinNum >= 14){
      limitSWPinNum = 2;
    }
    
    if(stepPinNum >= 45){
      stepPinNum = 22;
    }

    if(enablePinNum >= 54){
      enablePinNum = 14;
    }
    else if(enablePinNum == 18){
      enablePinNum = 46;
    }

    if(cnt > stpGoal/16){
      Serial.print(digitalRead(NeckLimitSW));
      Serial.print(digitalRead(LeftChestLimitSW));
      Serial.print(digitalRead(RightChestLimitSW));
      Serial.print(digitalRead(BackChestLimitSW));
      Serial.print(digitalRead(FrontChestLimitSW));
      Serial.print(digitalRead(LeftWaistLimitSW));
      Serial.print(digitalRead(RightWaistLimitSW));
      Serial.print(digitalRead(BackWaistLimitSW));
      Serial.print(digitalRead(FrontWaistLimitSW));
      Serial.print(digitalRead(LeftHipLimitSW));
      Serial.print(digitalRead(RightHipLimitSW));
      Serial.println(digitalRead(BackHipLimitSW));
      cnt = 0;
    }
    
    if(digitalRead(limitSWPinNum) == 1){
      //delay(25);
      if(digitalRead(limitSWPinNum) != 1){
        digitalWrite(enablePinNum,LOW);
        
        digitalWrite(stepPinNum,HIGH); 
        delayMicroseconds(1000); 
        digitalWrite(stepPinNum,LOW); 
        delayMicroseconds(1000);
      }
    }
    else{
      digitalWrite(enablePinNum,LOW);

      for(int i=0;i<stpGoal;i++){
        digitalWrite(stepPinNum,HIGH); 
        delayMicroseconds(1000); 
        digitalWrite(stepPinNum,LOW); 
        delayMicroseconds(1000); 
      }
    }
    
    if(stepPinNum != FrontChestStep){
      digitalWrite(enablePinNum,HIGH);
    }
    
    stepPinNum += 2;
    limitSWPinNum++;
    enablePinNum++;
    cnt++;
  } 

  Serial.println("The mannequin is at its minimum");
}

void ultrasonicRead() {
  ultrasonicTrigPinNum = 54;
  ultrasonicEchoPinNum = 55;
  
  for(int i=0;i<8;i++){
    digitalWrite(ultrasonicTrigPinNum, LOW);
    delayMicroseconds(2);
    digitalWrite(ultrasonicTrigPinNum, HIGH);
    delayMicroseconds(10);
    digitalWrite(ultrasonicTrigPinNum, LOW);
    
    if(ultrasonicFlag == 0){
      delayMicroseconds(20);
      minimumUltrasonicValue[i] = pulseIn(ultrasonicEchoPinNum, HIGH);
      Serial.print("Ultrasonic: ");
      Serial.print(i);
      Serial.print(" Distance: ");
      Serial.println(minimumUltrasonicValue[i]);
    }
    else if(ultrasonicFlag == 1){
      ultrasonicValue[i] = pulseIn(ultrasonicEchoPinNum, HIGH);
    }

    ultrasonicTrigPinNum += 2;
    ultrasonicEchoPinNum += 2;

    //stops possible reflections between panels affecting results
    delay(5);
  }
  delay(1000);
}

void loop() {
  ultrasonicRead();
  
  if (Serial.available() > 0) {
    incomingByte = Serial.read();
    const unsigned int MAX_MESSAGE_LENGTH = 12;
    static char message[MAX_MESSAGE_LENGTH];
    static unsigned int message_pos = 0;

    if ( incomingByte != '\n' && (message_pos < MAX_MESSAGE_LENGTH - 1) ){
      //Add the incoming byte to our message
      message[message_pos] = incomingByte;
      message_pos++;
   }else{
      //Add null character to string
      message[message_pos] = '\0';

      String msg = message;
      if(msg == "hi") Serial.println("yo mama");
      
      if(msg == "i"){
        Serial.println("Received i");
        moveIn();
        incomingByte = 0;
      }else if(msg == "o"){
        Serial.println("Received o");
        moveOut();
        incomingByte = 0;
      }else if(msg == "s"){
        Serial.println("Received oscillate");
        oscillate();
        incomingByte = 0;
      }else if(msg == "n"){
        Serial.println("Neck");
        steP = NeckStep;
        dIr = NeckDir;
        eN = NeckEn;
        liMit = NeckLimitSW;
      }else if(msg == "fc"){
        Serial.println("Front Chest");
        steP = FrontChestStep;
        dIr = FrontChestDir;
        eN = FrontChestEn;
        liMit = FrontChestLimitSW;
      }else if(msg == "lc"){
        Serial.println("Left Chest");
        steP = LeftChestStep;
        dIr = LeftChestDir;
        eN = LeftChestEn;
        liMit = LeftChestLimitSW;
      }else if(msg == "rc"){
        Serial.println("Right Chest");
        steP = RightChestStep;
        dIr = RightChestDir;
        eN = RightChestEn;
        liMit = RightChestLimitSW;
      }else if(msg == "bc"){
        Serial.println("Back Chest");
        steP = BackChestStep;
        dIr = BackChestDir;
        eN = BackChestEn;
        liMit = BackChestLimitSW;
      }else if(msg == "fw"){
        Serial.println("Front Waist");
        steP = FrontWaistStep;
        dIr = FrontWaistDir;
        eN = FrontWaistEn;
        liMit = FrontWaistLimitSW;
      }else if(msg == "lw"){
        Serial.println("Left Waist");
        steP = LeftWaistStep;
        dIr = LeftWaistDir;
        eN = LeftWaistEn;
        liMit = LeftWaistLimitSW;
      }else if(msg == "rw"){
        Serial.println("Right Waist");
        steP = RightWaistStep;
        dIr = RightWaistDir;
        eN = RightWaistEn;
        liMit = RightWaistLimitSW;
      }else if(msg == "bw"){
        Serial.println("Back Waist");
        steP = BackWaistStep;
        dIr = BackWaistDir;
        eN = BackWaistEn;
        liMit = BackWaistLimitSW;
      }else if(msg == "lh"){
        Serial.println("Left Hip");
        steP = LeftHipStep;
        dIr = LeftHipDir;
        eN = LeftHipEn;
        liMit = LeftHipLimitSW;
      }else if(msg == "rh"){
        Serial.println("Right Hip");
        steP = RightHipStep;
        dIr = RightHipDir;
        eN = RightHipEn;
        liMit = RightHipLimitSW;
      }else if(msg == "bh"){
        Serial.println("Back Hip");
        steP = BackHipStep;
        dIr = BackHipDir;
        eN = BackHipEn;
        liMit = BackHipLimitSW;
      }
      else if(msg == "ao") moveAllOut();
      else if(msg == "ai") moveAllIn();

     //Reset for the next message
     message_pos = 0;
   }
 }
}
