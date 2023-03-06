//********************************************************************************************************************************************
//INFORMATION
//200 pulses for making one full cycle rotation.
//Rotating clockwise moves track towards the centre.
//Rotating anti-clockwise moves track away from the centre.
//Takes 855 full steps for translational motor to move track full distance.
//Takes 2000 full steps for rotational motor to cause full rotation of plate.
//Takes 10 rotations of roational motor for one full rotation of plate.
//One rotation of plate causes track to move one rotation of spindle += 200 full steps.
//When track has moved 855 full steps it needs to start rotating other way.
//Setting tDirPin LOW moves magnet away from centre.
//Setting tDirPin HIGH moves magnet towards the centre.
//Setting rDirPin LOW moves plate clockwise.
//Setting rDirPin HIGH moves plates anti-clockwise.
//1 Track step = 10 Rotational Steps
//********************************************************************************************************************************************

// defines pins numbers
#define tStepPin 3
#define tDirPin 4
#define tEnablePin 5
#define rStepPin 6
#define rDirPin 7
#define rEnablePin 8

enum serialInput{
  tDir = 1,
  tStepNum,
  rDir,
  rStepNum
};

bool TstepLevel = HIGH;
bool RstepLevel = HIGH;

int packetPeriod = 1;

float Tsteps = 0;
float Rsteps = 0;
int switchInput = 1;

void setup() {
  // Sets the two pins as Outputs
  pinMode(tStepPin, OUTPUT);
  pinMode(tDirPin, OUTPUT);
  pinMode(tEnablePin, OUTPUT);

  pinMode(rStepPin, OUTPUT);
  pinMode(rDirPin, OUTPUT);
  pinMode(rEnablePin, OUTPUT);

  digitalWrite(tDirPin, HIGH);
  digitalWrite(tEnablePin, LOW);
  digitalWrite(rDirPin, HIGH);
  digitalWrite(rEnablePin, LOW);

  Serial.begin(9600);

  noInterrupts();  //disable all interrupts
  TCCR3A = 0;      //set entire TCCR3A register to 0
  TCCR3B = 0;      //set entire TCCR3B register to 0
  TCNT3 = 0;       //initialize counter value to 0

  TCCR4A = 0;  //set entire TCCR4A register to 0
  TCCR4B = 0;  //set entire TCCR4B register to 0
  TCNT4 = 0;   //initialize counter value to 0

  //  TCCR5A = 0;               //set entire TCCR5A register to 0
  //  TCCR5B = 0;               //set entire TCCR5B register to 0
  //  TCNT5  = 0;               //initialize counter value to 0

  TCCR3B |= (1 << WGM12);   //CTC mode
  TCCR3B |= (1 << CS12);    //256 prescaler
  TIMSK3 |= (1 << OCIE3A);  //enable timer compare interrupt

  TCCR4B |= (1 << WGM12);   //CTC mode
  TCCR4B |= (1 << CS12);    //256 prescaler
  TIMSK4 |= (1 << OCIE4A);  //enable timer compare interrupt

  //  OCR5A = 10;             //Regular interrupt 10 times a second
  //  TCCR5B |= (1 << WGM12);   //CTC mode
  //  TCCR5B |= (1 << CS12);    //256 prescaler
  //  TIMSK5 |= (1 << OCIE5A);  //enable timer compare interrupt
  interrupts();  //enable all interrupts
}

ISR(TIMER3_COMPA_vect) {
  if (Tsteps >= 1) {
    TstepLevel = !TstepLevel;
    digitalWrite(tStepPin, TstepLevel);
    Tsteps--;
  } else digitalWrite(tEnablePin, HIGH);
}

ISR(TIMER4_COMPA_vect) {
  if (Rsteps >= 1) {
    RstepLevel = !RstepLevel;
    digitalWrite(rStepPin, RstepLevel);
    Rsteps--;
  } else digitalWrite(rEnablePin, HIGH);
}

ISR(TIMER5_COMPA_vect) {
}

void chosenDir(enum serialInput dirPin, char* serialRead) {
  if(serialRead == 'p') digitalWrite(dirPin, LOW);
  else if(serialRead == 'n') digitalWrite(dirPin, HIGH);
}

void loop() {
  if (Tsteps < 1 && Rsteps < 1 && Serial.available()) {
    while (Tsteps < 1 || Rsteps < 1) {
      switch (switchInput) {
      case tDir:
        switchInput = tStepNum;
        chosenDir(tDirPin, Serial.read());
      case tStepNum:
        digitalWrite(tEnablePin, LOW);
        Tsteps = Serial.read() * 2 * 16;
        OCR3A = packetPeriod * 62500 / Tsteps;
        switchInput = rDir;
      case rDir:
        switchInput = rStepNum;
        chosenDir(tDirPin, Serial.read());
      case rStepNum:
        digitalWrite(rEnablePin, LOW);
        Rsteps = Serial.read() * 2 * 16;
        OCR4A = packetPeriod * 62500 / Rsteps;
        switchInput = tDir;
      }
    }
  }
}
