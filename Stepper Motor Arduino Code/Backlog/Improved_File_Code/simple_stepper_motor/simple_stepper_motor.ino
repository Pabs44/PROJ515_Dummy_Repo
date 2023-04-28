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
//Setting dirPin LOW moves magnet away from centre.
//Setting dirPin HIGH moves magnet towards the centre.
//
// FOR SETTING MOTOR MODE, FOR EACH MOTOR, FOR EASE.
// Modes include:
// MS1    MS2    MS3    Microstep Res
// Low    Low    Low    Full Step
// High   Low    Low    Half Step
// Low    High   Low    Quarter Step
// High   High   Low    Eighth Step
// High   High   High   Sixteenth Step
// States this here: https://www.tme.eu/Document/25459777e672c305e474897eef284f74/POLOLU-2128.pdf
//
// CHEST COG: DIAMETER - 16.8 mm, 8.2 mm
// NECK COG: DIAMETER - 1:1 (5 mm)
//********************************************************************************************************************************************

struct RECVD_DISTANCES{
  float MOTOR_1;
  float MOTOR_2;
};

// RADIUS OF MOTOR COGS FOR NECK AND CHEST.
const float CHEST_COG_RAD = 4.1;
const float NECK_COG_RAD = 2.5;

// MOTOR CHOICE
enum MTR_ID{
  MOTOR_1 = 1,
  MOTOR_2,
  BOTH
};

// STEP TYPES
enum STEP_TYPE{
  FULL_STEP = 0,
  HALF_STEP,
  QUARTER_STEP,
  EIGHTH_STEP,
  SIXTEENTH_STEP
};

// DEGREES OF TYPE OF STEP (TAKEN FROM MOTOR DATASHEET)
const float DEG_STEP[5] = {1.8, 0.9, 0.45, 0.225, 0.1125};

// TALLY FOR AMOUNT OF TYPE OF STEP
int T_STEP[5];

// DISTANCE REQUIRED TO MOVE OF MOTOR VAR
RECVD_DISTANCES MOV_DIST = {-3.0, 5.0};

// NON READ-ONLY PINS SO WE CAN MODIFY GLOBAL FUNCTION
int StepPin1 = 4; //MOTOR ONE
int StepPin2 = 5; //MOTOR TWO

// 4060 PINS FOR MOTORS 1/2.
const int RST1 = 2;
const int CLK1 = 3;

// TMP_STORE_EN_DIR_VALUES
int TMP_EN[2], TMP_DIR[2];

// TMP_ARRAY_STORE_TALLY
int TMP_TALLY_ARRAY[2][5];

void MODE_DEF(int **INPUT_ARRAY, int *INPUT_DATA, MTR_ID INPUT_MOTOR){
  switch(INPUT_MOTOR){
  case MOTOR_1:
    for(int i = 0; i < sizeof(INPUT_DATA); i++) INPUT_ARRAY[0][i] = INPUT_DATA[i];
    break;
  case MOTOR_2:
    for(int i = 0; i < sizeof(INPUT_DATA); i++) INPUT_ARRAY[1][i] = INPUT_DATA[i];
    break;
  case BOTH:
    for(int i = 0; i < sizeof(INPUT_DATA); i++){
      INPUT_ARRAY[0][i] = INPUT_DATA[i];
      INPUT_ARRAY[1][i] = INPUT_DATA[i];
    }
    break;
  }
  return INPUT_ARRAY;
}

void CALC_MOV(float MOTOR_DIS, int MOT_ID, float COG_RAD) {  // Needs something like this for specify what motor we want to calculate.
  // INITIATE CALCULATION CONDITIONS
  int ARRAY_POS = (MOT_ID - 1); //ARRAY INDEX
  float RAD = COG_RAD;          //RADIUS CAN BE MODIFIED
  TMP_EN[ARRAY_POS] = 0;        //ACTIVE LOW ENABLE PIN FOR MOTOR

  // SOMEWHERE HERE, WE NEED TO CHECK ULTRASONIC SENSOR AND LIMIT SWITCH, IF THIS MEASUREMENT IS POSSIBLE

  // SETTING DIRECTION OF MOVEMENT
  TMP_DIR[ARRAY_POS] = (MOTOR_DIS < 0.0) ? 0 : 1; //ANTI-CLKWISE : CLKWISE
  if (MOTOR_DIS < 0.0) MOTOR_DIS *= -1;          //SETTING DISTANCE REQUIRED TO POSITIVE VALUE IF NOT

  // AS DISTANCE = RADIUS * THETA, THIS CALCULATION IS PERFORMED
  float THETA = MOTOR_DIS / (RAD * (3.14 / 180));  // pi/180 as we are working in degrees not radians. Described here: https://www.cuemath.com/geometry/arc-length/

  // TALLYING STEP SIZES
  for (int i = FULL_STEP; i <= SIXTEENTH_STEP; i++){
    T_STEP[i] = THETA / DEG_STEP[i];            //Rounds float value down and sets it as amount of steps to take
    THETA -= T_STEP[i] * DEG_STEP[i];           //Changing theta to remove the amount of degrees taken
    TMP_TALLY_ARRAY[ARRAY_POS][i] = T_STEP[i];  //Stores tallied step amount
  }
}

void CONFIGURE_4060(int EN[2], int DIR[2], int MODE[2][3]) {
  // RESET COUNTER.
  digitalWrite(RST1, HIGH);
  delay(1);
  digitalWrite(RST1, LOW);

  // IF ERROR, TO AVOID EXTRA WORK FOR THE COUNTER. (MOTOR DRIVER HAS ACTIVE-LOW ENABLE)
  int MODE_ERROR[3] = {0,0,0};
  if (EN[0] == 1 && EN[1] == 0) {
    DIR[0] = 0;
    MODE_DEF((int **)MODE, (int *)MODE_ERROR, MOTOR_1);
  }
  if (TMP_EN[0] == 0 && TMP_EN[1] == 1) {
    DIR[1] = 0;
    MODE_DEF((int **)MODE, (int *)MODE_ERROR, MOTOR_2);
  }

  // EN1, DIR1, M11, M21, M31, EN2, DIR2, M12, M22, M32
  int C_OUT = (EN[0] * pow(2, 3)) + (DIR[0] * pow(2, 4)) + (MODE[0][0] * pow(2, 5)) + (MODE[0][1] * pow(2, 6)) + (MODE[0][2] * pow(2, 7)) + (EN[1] * pow(2, 8)) + (DIR[1] * pow(2, 9)) + (MODE[1][0] * pow(2, 11)) + (MODE[1][1] * pow(2, 12)) + (MODE[1][2] * pow(2, 13));

  // RUNNING CLK TO SPECIFIED COUNT.
  for (int i = 0; i < C_OUT; i++) {
    digitalWrite(CLK1, HIGH);
    delay(0.01);  // SETTING FREQUENCY TO 100KHz.
    analogWrite(CLK1, LOW);
  }
}

void MOV_MOTORS(int TALLY_ARRAY[2][5]) {
  // CONFIGURING 4060.
  digitalWrite(StepPin1, LOW);
  digitalWrite(StepPin2, LOW);
  int MODE_SEL[2][3];
  for (int i = FULL_STEP; i <= SIXTEENTH_STEP; i++) {
    switch (i) {
    case FULL_STEP:
      int MODE_FULL_STEP[3] = {0,0,0};
      MODE_DEF((int **)MODE_SEL, (int *)MODE_FULL_STEP, BOTH);
      break;
    case HALF_STEP:
      int MODE_HALF_STEP[3] = {1,0,0};
      MODE_DEF((int **)MODE_SEL, (int *)MODE_HALF_STEP, BOTH);
      break;
    case QUARTER_STEP:
      int MODE_QTR_STEP[3] = {0,1,0};
      MODE_DEF((int **)MODE_SEL, (int *)MODE_QTR_STEP, BOTH);
      break;
    case EIGHTH_STEP:
      int MODE_EIG_STEP[3] = {1,1,0};
      MODE_DEF((int **)MODE_SEL, (int *)MODE_EIG_STEP, BOTH);
      break;
    case SIXTEENTH_STEP:
      int MODE_SXTH_STEP[3] = {1,1,1};
      MODE_DEF((int **)MODE_SEL, (int *)MODE_SXTH_STEP, BOTH);
      break;
    }

    // CONFIGURING BIT COUNTER.
    CONFIGURE_4060(TMP_EN, TMP_DIR, MODE_SEL);

    // USING TALLY OF STEPS, MOVE MOTORS.
    for (int k = 0; k < TALLY_ARRAY[0][i-1]; k++) {
      digitalWrite(StepPin1, HIGH);
      delay(1);
      digitalWrite(StepPin1, LOW);
    }

    for (int k = 0; k < TALLY_ARRAY[1][i-1]; k++) {
      digitalWrite(StepPin2, HIGH);
      delay(1);
      digitalWrite(StepPin2, LOW);
    }
  }
}

void setup() {
  Serial.begin(9600);
  pinMode(RST1, OUTPUT);
  pinMode(CLK1, OUTPUT);
  CALC_MOV(MOV_DIST.MOTOR_1, MOTOR_1, NECK_COG_RAD);
  CALC_MOV(MOV_DIST.MOTOR_2, MOTOR_2, CHEST_COG_RAD);
  MOV_MOTORS(TMP_TALLY_ARRAY);
}

void loop() {
}
