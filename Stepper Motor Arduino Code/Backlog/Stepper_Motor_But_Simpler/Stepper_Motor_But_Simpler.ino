// FOR SETTING MOTOR MODE, FOR EACH MOTOR, FOR EASE.
// Modes include:
// MS1    MS2    MS3    Microstep Res
// Low    Low    Low    Full Step
// High   Low    Low    Half Step
// Low    High   Low    Quarter Step
// High   High   Low    Eighth Step
// High   High   High   Sixteenth Step
// States this here: https://www.tme.eu/Document/25459777e672c305e474897eef284f74/POLOLU-2128.pdf

// CHEST COG: DIAMETER - 16.8 mm, 8.2 mm
// NECK COG: DIAMETER - 1:1 (5 mm)

struct RECVD_DISTANCES {
  float MOTOR_1;
  float MOTOR_2;
};

// RADIUS OF MOTOR COGS FOR NECK AND CHEST.
const float CHEST_COG_RAD = 4.1;
const float NECK_COG_RAD = 2.5;

// DEGREES OF TYPE OF STEP (TAKEN FROM MOTOR DATASHEET)
const float FULL_STEP_DEG = 1.8;
const float HALF_STEP_DEG = 0.9;
const float QUARTER_STEP_DEG = 0.45;
const float EIGHTH_STEP_DEG = 0.225;
const float SIXTEENTH_STEP_DEG = 0.1125;

// TALLY FOR AMOUNT OF TYPE OF STEP
int T_FULL_STEP, T_HALF_STEP, T_QUARTER_STEP, T_EIGHTH_STEP, T_SIXTEENTH_STEP;

// DISTANCE REQUIRED TO MOVE OF MOTOR VAR
RECVD_DISTANCES MOV_DIST = {-3.0, 5.0};

// BUTTON PIN.
const int BUTTON = 18;

// SWITCH PIN.
const int LIMIT_SWITCH = 19;

// PINS (MOTOR ONE) (NON READ-ONLY PINS SO WE CAN MODIFY GLOBAL FUNCTION).
int StepPin1 = 2;
int DirPin1 = 3;

// (MOTOR TWO) (NON READ-ONLY PINS SO WE CAN MODIFY GLOBAL FUNCTION).
int StepPin2 = 4;
int DirPin2 = 5;

// ULTRASONIC SENSOR.
const int TrigPin = 6;
const int EchoPin = 7;

// TMP_PIN_DEC
int STEP_PIN, DIR_PIN, MOTOR_ID_STORE_TMP;

// STORING distance values.
float DIST_STORE = 0;
float RETURN_DISTANCE;

// TMP_ARRAY_STORE_TALLY
int TMP_TALLY[2];

// IF ANTI_CLKWISE OR CLKWISE
int ROT;

// MAX DISTANCE.
float MAX_DIST = 30.0; 

float CHECK_DISTANCE_ULTRA() {
  // Code from https://howtomechatronics.com/tutorials/arduino/ultrasonic-sensor-hc-sr04/?utm_content=cmp-true
  // RESET TRIG PIN
  digitalWrite(TrigPin, LOW);
  delayMicroseconds(2);

  // SEND PULSE.
  digitalWrite(TrigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(TrigPin, LOW);

  // CALCULATE DISTANCE FROM RETURN PULSE.
  long duration = pulseIn(EchoPin, HIGH);
  float distance = duration * 0.034 / 2;

  // As the distance gets less accurate through testing after 40cm, this is added.
  if (distance > 40) {
    distance = 40;
  }
  
  return distance;
}

void CALC_MOV(float MOTOR_DIS, int MOT_ID, float COG_RAD) { // Needs something like this for specify what motor we want to calculate.
  noInterrupts();
  // SPECIFYING MOTOR PINS AND POSITION IN ARRAY.
  switch (MOT_ID) {
    case 1:
      DIR_PIN = DirPin1;

      pinMode(DIR_PIN, OUTPUT);
      
      break;
    case 2:
      DIR_PIN = DirPin2;
      
      pinMode(DIR_PIN, OUTPUT);

      break;
  }
  
  int ARRAY_POS =(MOT_ID - 1);
  
  float RAD = COG_RAD; // RADIUS CAN BE MODIFIED.

  MOVE_HOME_POS();

  // If distance is not possible, move to home and try again.
  if (((CHECK_DISTANCE_ULTRA() + MOTOR_DIS) < 0) || ((CHECK_DISTANCE_ULTRA() + MOTOR_DIS) > MAX_DIST)) {
    DIST_STORE = 0;
  } else {
    DIST_STORE += MOTOR_DIS;
  }

  digitalWrite(DIR_PIN, HIGH);
    
  // SETTING DIRECTION OF MOVEMENT.
//  if (MOTOR_DIS < 0.0) {
//    // ANTI-CLKWISE
//    digitalWrite(DIR_PIN, LOW);
//
//    // SETTING DISTANCE REQUIRED TO POSITIVE VALUE. (TWO MINUSES MAKE POSITIVE).
//    MOTOR_DIS = -MOTOR_DIS;
//
//    // SET ROT TO -1, TO STATE ANTI-CLKWISE.
//    ROT = -1;
//  } else {
//    // CLKWISE
//    digitalWrite(DIR_PIN, HIGH);
//
//    // SET ROT TO 1, TO STATE CLKWISE.
//    ROT = 1;
//  }
  
  // AS DISTANCE = RADIUS * THETA, THIS CALCULATION IS PERFORMED.
  float THETA = MOTOR_DIS / (RAD * (3.14 / 180)); // pi/180 as we are working in degrees not radians. Described here: https://www.cuemath.com/geometry/arc-length/
  
  // TALLYING FULL STEP SIZE. (WILL NEED TO BE MODIFIED IF USE OF OTHER MODES).
  // HOLDS a float of how many full microsteps can be achieved within the theta angle specified.
  float HOLD_FULL = THETA / FULL_STEP_DEG;
  // Rounds down the float value, and sets it as amount of steps to take.
  T_FULL_STEP = (int)HOLD_FULL;
  
  // STORING the tallied step amount for specific motor.
  TMP_TALLY[ARRAY_POS] = T_FULL_STEP;
  
  interrupts();
}

void MOV_MOTORS(int TALLY_ARRAY[2]) {
  // VERIFIES and selects pin.
  for (int i = 0; i < sizeof(TALLY_ARRAY); i++) {
    if (TALLY_ARRAY[i] > 0) {
      switch(i) {
        case 0:
          STEP_PIN = StepPin1;
        
          pinMode(STEP_PIN, OUTPUT);
          digitalWrite(STEP_PIN, LOW);

          // Store MOTOR ID SETTING FOR INTERRUPT IF BUTTON IS PRESSED.
          MOTOR_ID_STORE_TMP = 0;
          break;

        case 1:
          STEP_PIN = StepPin2;
        
          pinMode(STEP_PIN, OUTPUT);
          digitalWrite(STEP_PIN, LOW);

          MOTOR_ID_STORE_TMP = 1;
          break;
      }

      // MOVE dedicated by step pin selected.
      for (int k = 0; k < TALLY_ARRAY[i]; k++) {
        digitalWrite(STEP_PIN, HIGH);
        delay(10);
        digitalWrite(STEP_PIN, LOW);
      }
    }
  }
}

void MOVE_HOME_POS() {
    digitalWrite(DIR_PIN, HIGH);
    Serial.println("BUTTON");
    
    while (digitalRead(LIMIT_SWITCH) == LOW) {
      digitalWrite(StepPin1, HIGH);
      digitalWrite(StepPin2, HIGH);
      delay(100);
      digitalWrite(StepPin1, LOW);
      digitalWrite(StepPin2, LOW);
    }
}

// Switching off particular motor, if limit switch is applied.
void SWITCH_OFF_MOTOR() {
  digitalWrite(StepPin1, LOW);
  digitalWrite(StepPin2, LOW);
  TMP_TALLY[0] = 0;
  TMP_TALLY[1] = 0;
  
//  switch (MOTOR_ID_STORE_TMP) {
//    case 0:
//      digitalWrite(StepPin1, LOW);
//
//      // RESETTING MOTOR STEPS.
//      TMP_TALLY[0] = 0;
//      break;
//      
//    case 1:
//      digitalWrite(StepPin2, LOW);
//
//      TMP_TALLY[1] = 0;
//      break;
//  }
}

void setup() {
  Serial.begin(9600);
  pinMode(StepPin1, OUTPUT);
  pinMode(StepPin2, OUTPUT);
  pinMode(BUTTON, INPUT);
  // MOTOR KEEPS MOVING EVEN THOUGH THAT STEP IS 0!
  attachInterrupt(digitalPinToInterrupt(BUTTON), MOVE_HOME_POS, RISING);

  pinMode(LIMIT_SWITCH, INPUT);
  // LIMIT SWITCH.
  attachInterrupt(digitalPinToInterrupt(LIMIT_SWITCH), SWITCH_OFF_MOTOR, RISING); 

  pinMode(TrigPin, OUTPUT);
  pinMode(EchoPin, INPUT);
  
  // CALC_MOV(MOV_DIST.MOTOR_1, 1, NECK_COG_RAD);
  // CALC_MOV(MOV_DIST.MOTOR_2, 2, CHEST_COG_RAD);
  MOV_MOTORS(TMP_TALLY);
}

void loop() {
  
}
