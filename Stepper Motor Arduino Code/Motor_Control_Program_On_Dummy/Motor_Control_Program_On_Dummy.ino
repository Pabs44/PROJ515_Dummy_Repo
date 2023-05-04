#define AMOUNT_MOT 2
enum MAN_LIM_STEP {NECK_S = 100, L_C = 110, R_C = 80, B_C = 80, F_C = 40, L_W = 20, R_W = 30, B_W = 20, F_W = 40, L_H = 50, R_H = 30, B_H = 20};

#define AMOUNT_ULTRA 8
#define MOT_DELAY 1

float MOTOR_CIR_DIST_ARRAY[4] = {400.5, 925.3, 735.8, 960.4};

float MAX_CIRC_DIST[4] = {440, 1020, 840, 1070}, MIN_CIRC_DIST[4] = {350, 840, 660, 890};
int LIMIT_PINS[AMOUNT_MOT], MOTOR_PINS[12][3], ULTRA_PINS[AMOUNT_ULTRA][2], LED_PINS[3], INIT_BUTTON, CHECK_SW[AMOUNT_MOT], init_sw[AMOUNT_MOT], TALLY[AMOUNT_MOT];
long DEBOUNCE_SW[AMOUNT_MOT][2];

enum BODY_PART {NECK = 0, CHEST, WAIST, HIP};

void CONFIG_P() {
  int MOTORS[12][3] = {{22, 23, 46}, {24, 25, 47}, {26, 27, 48}, {28, 29, 49}, {30, 31, 50}, {32, 33, 51}, {34, 35, 52}, {36, 37, 53}, {38, 39, 14}, {40, 41, 15}, {42, 43, 16}, {44, 45, 17}};
  int SW[12] = {2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13};
  int ULTRA[8][2] = {{54, 55}, {56, 57}, {58, 59}, {60, 61}, {62, 63}, {64, 65}, {66, 67}, {68, 69}}; 
  int LED[3] = {18, 19, 20};
  int BUTTON_PIN = 21;

  for (int i = 0; i < AMOUNT_MOT; i++) {
    LIMIT_PINS[i] = SW[i];
    pinMode(LIMIT_PINS[i], INPUT);
    for (int j = 0; j < 2; j++) {
      MOTOR_PINS[i][j] = MOTORS[i][j];
      pinMode(MOTOR_PINS[i][j], OUTPUT);
    }
    MOTOR_PINS[i][2] = MOTORS[i][2];
    pinMode(MOTOR_PINS[i][2], OUTPUT);
    digitalWrite(MOTOR_PINS[i][2], HIGH);
  }

  for (int i = 0; i < (12 - AMOUNT_MOT); i++) {
    for (int j = 0; j < 2; j++) {
      MOTOR_PINS[i][j] = MOTORS[i][j];
      pinMode(MOTOR_PINS[i][j], OUTPUT);
      digitalWrite(MOTOR_PINS[i][j], LOW);
    }
    MOTOR_PINS[i][2] = MOTORS[i][2];
    pinMode(MOTOR_PINS[i][2], OUTPUT);
    digitalWrite(MOTOR_PINS[i][2], HIGH);
  }

  for (int i = 0; i < AMOUNT_ULTRA; i++) {
    ULTRA_PINS[i][0] = ULTRA[i][0];
    ULTRA_PINS[i][1] = ULTRA[i][1];
    pinMode(ULTRA_PINS[i][0], OUTPUT); // TRIG
    pinMode(ULTRA_PINS[i][1], INPUT); // ECHO
  }

  // Setting up user experience LEDs.
  for (int i = 0; i < 3; i++) {
    LED_PINS[i] = LED[i];
    pinMode(LED_PINS[i], OUTPUT);
    digitalWrite(LED_PINS[i], LOW);
  }

  // BUTTON PIN.
  INIT_BUTTON = BUTTON_PIN;
  pinMode(INIT_BUTTON, INPUT);
}

void wait_us(int DURATION) {
  int us_previousTime = micros();
  int us_currentTime = micros();
  while((us_currentTime - us_previousTime) <= DURATION) us_currentTime = micros();
}

void DEBOUNCE_SWITCHES(int ID) {
  // Debouncing using timers.
  // Setting initial time for debouncing of switch.
  if (init_sw[ID] == 0) {
    DEBOUNCE_SW[ID][0] = micros();
    init_sw[ID] = 1;
  }

  // Everytime the function is called, the micro time since startup is taken.
  DEBOUNCE_SW[ID][1] = micros();

  // IF there is an initial time, DO.
  if (init_sw[ID] == 1) {
    // IF initial time, and now time has got a difference of 10000, DO.
    if ((DEBOUNCE_SW[ID][1] - DEBOUNCE_SW[ID][0]) > 10000) {
      // CHECK SWITCH IS NOT ALREADY INITIATED. 
      // ALLOW switch press to be final.
      if (CHECK_SW[ID] == 0) CHECK_SW[ID] = 1;
      // Allow for initial time to be allocated again.
      init_sw[ID] = 0;
    }    
  }
}

void MOVE_HOME_POS() {
  int CNT_MOT_CHECK = 0, VERIFY_CNT_CHECK[AMOUNT_MOT];
  
  for (int i = 0; i < AMOUNT_MOT; i++) {
    // SETTING ROTATION TO MOVE IN.
    digitalWrite(MOTOR_PINS[i][1], HIGH);

    // MAKING sure that the initial switch and check switch variables are definitely 0, as variable sometimes floats.
    init_sw[i] = 0;
    CHECK_SW[i] = 0;
    VERIFY_CNT_CHECK[i] = 0;
  }

  while (CNT_MOT_CHECK != AMOUNT_MOT) {
    for (int i = 0; i < AMOUNT_MOT; i++) {
      // Setting Motor driver enable to active low.
      digitalWrite(MOTOR_PINS[i][2], LOW);
      // READING SPECIFIC SWITCH.
      if (digitalRead(LIMIT_PINS[i]) == 1) DEBOUNCE_SWITCHES(i);
      // Checking if switch has been pressed.
      if (CHECK_SW[i] == 1) {
        // Stopping motors.
        digitalWrite(MOTOR_PINS[i][0], LOW);
        // Making sure that the count of each motor is only added once.
        if (VERIFY_CNT_CHECK[i] == 0) {
          VERIFY_CNT_CHECK[i] = 1;
          CNT_MOT_CHECK++;
        }
      } else {
        // Step the motors.
        digitalWrite(MOTOR_PINS[i][0], HIGH);
        wait_us(MOT_DELAY * 1000);
        digitalWrite(MOTOR_PINS[i][0], LOW);
      }
      // Setting Motor driver enable to HIGH.
      digitalWrite(MOTOR_PINS[i][2], HIGH);
    }
  }
}

// For plate config message, and ultrasonic distance function.
int* ULTRA_CONFIG(int MOT_ID) {
  static int ULTRA_ID[2];

  if (MOT_ID == 1) {ULTRA_ID[0] = 0; ULTRA_ID[1] = 2;}
  if (MOT_ID == 2) {ULTRA_ID[0] = 1; ULTRA_ID[1] = 3;}
  if (MOT_ID == 3) {ULTRA_ID[0] = 2; ULTRA_ID[1] = 3;}
  if (MOT_ID == 4) {ULTRA_ID[0] = 0; ULTRA_ID[1] = 1;}
  if ((MOT_ID == 5) || (MOT_ID == 9)) {ULTRA_ID[0] = 4; ULTRA_ID[1] = 6;}
  if ((MOT_ID == 6) || (MOT_ID == 10)) {ULTRA_ID[0] = 5; ULTRA_ID[1] = 7;}
  if ((MOT_ID == 7) || (MOT_ID == 11)) {ULTRA_ID[0] = 6; ULTRA_ID[1] = 7;}
  if (MOT_ID == 8) {ULTRA_ID[0] = 4; ULTRA_ID[1] = 5;}
  
  return ULTRA_ID;
}

// float* to accept array of the two ultrasonic's.
float* CHECK_DISTANCE_ULTRA(int MOT_ID) {
  static float DIST[2];

  int* ULT_CONFIG = ULTRA_CONFIG(MOT_ID);

  // NUMBER of ultrasonic sensors for plate.
  for (int i = 0; i < 2; i++) {
    // TRIG PIN. (RESET)
    digitalWrite(ULTRA_PINS[ULT_CONFIG[i]][0], LOW);
    wait_us(2);

    // TRIG PIN. (SEND PULSE)
    digitalWrite(ULTRA_PINS[ULT_CONFIG[i]][0], HIGH);
    wait_us(10);
    digitalWrite(ULTRA_PINS[ULT_CONFIG[i]][0], LOW);

    // ECHO PIN. (CALCULATE DISTANCE FROM RETURN PULSE)
    unsigned long DURATION_PULSE = pulseIn(ULTRA_PINS[ULT_CONFIG[i]][1], HIGH, 6000);
    DIST[i] = DURATION_PULSE * 0.034 / 2;
    
    // IF DISTANCE IS BIGGER THAN 40cm.
    //                           TRUE   FALSE
    DIST[i] = (DIST[i] > 40.0) ? 40.0 : DIST[i];
  }

  return DIST;
}

void GROUP_MOTORS(int MEASUREMENT_GROUP, float PERCENTAGE) {
  int MAX_STEPS[4];
  // SETTING ALL MOTORS TO BE MOVING OUT.
  for (int i = 0; i < AMOUNT_MOT; i++) {
    if (i == 0) { // IF NECK.
      digitalWrite(MOTOR_PINS[i][1], HIGH);
    } else {
      digitalWrite(MOTOR_PINS[i][1], LOW);
    }
  }
  switch (MEASUREMENT_GROUP) {
    case NECK:
      MAX_STEPS[0] = NECK_S;
      TALLY[0] = (int)(MAX_STEPS[0] * PERCENTAGE);
      break;
    
    case CHEST:
      MAX_STEPS[0] = L_C, MAX_STEPS[1] = R_C, MAX_STEPS[2] = B_C, MAX_STEPS[3] = F_C;
      for (int i = 1; i < 5; i++) TALLY[i] = (int)(MAX_STEPS[(i - 1)] * PERCENTAGE);
      break;

    case WAIST:
      MAX_STEPS[0] = L_W, MAX_STEPS[1] = R_W, MAX_STEPS[2] = B_W, MAX_STEPS[3] = F_W;
      for (int i = 5; i < 9; i++) TALLY[i] = (int)(MAX_STEPS[(i - 5)] * PERCENTAGE);
      break;

    case HIP:
      MAX_STEPS[0] = L_H, MAX_STEPS[1] = R_H, MAX_STEPS[2] = B_H;
      for (int i = 9; i < 12; i++) TALLY[i] = (int)(MAX_STEPS[(i - 9)] * PERCENTAGE);
      break;
  }
}

void CALC_MOVES() {
  noInterrupts();
  String TEXT_PART_BODY = "";
  for (int i = 0; i < 4; i++) {
    switch (i) {
      case NECK:
        TEXT_PART_BODY = "NECK";
        break;

      case CHEST:
        TEXT_PART_BODY = "CHEST";
        break;

      case WAIST:
        TEXT_PART_BODY = "WAIST";
        break;

      case HIP:
        TEXT_PART_BODY = "HIP";
        break;
    }
    
    if (MOTOR_CIR_DIST_ARRAY[i] > MAX_CIRC_DIST[i]) {
      Serial.println("Circumference measurement for " + TEXT_PART_BODY + " is over the maximum.");
    } else if (MOTOR_CIR_DIST_ARRAY[i] < MIN_CIRC_DIST[i]) {
      Serial.println("Circumference measurement for " + TEXT_PART_BODY + " is under the minimum.");
    } else {
      float DIFF_LIMITS = MAX_CIRC_DIST[i] - MIN_CIRC_DIST[i];
      float DIFF_DES_MIN = MOTOR_CIR_DIST_ARRAY[i] - MIN_CIRC_DIST[i];

      float PERCENTAGE = DIFF_DES_MIN / DIFF_LIMITS;
      GROUP_MOTORS(i, PERCENTAGE);
    }
  }
  interrupts();
}

// MOVING MOTORS.
void MOV_MOTORS() {
  int CNT_MOT_CHECK = 0, VERIFY_CNT_CHECK[AMOUNT_MOT], TMP_TALLY[AMOUNT_MOT];

  for (int i = 0; i < AMOUNT_MOT; i++) {
    VERIFY_CNT_CHECK[i] = 0;
    digitalWrite(MOTOR_PINS[i][1], LOW);
    TMP_TALLY[i] = 0;
  }

  while (CNT_MOT_CHECK != AMOUNT_MOT) {
    // RUNNING THROUGH THE MOTORS.
    for (int i = 0; i < AMOUNT_MOT; i++) {
      // Setting Motor driver enable to active low.
      digitalWrite(MOTOR_PINS[i][2], LOW);

      // IF TALLY OF MOVEMENTS OF MOTOR IS LESS THAN THE ACTUAL TALLY OF STEPS, DO:
      if (TMP_TALLY[i] < TALLY[i]) {
        // STEP AND INCREASE TALLY OF MOVEMENTS OF THE MOTOR.
        digitalWrite(MOTOR_PINS[i][0], HIGH);
        wait_us(MOT_DELAY * 1000);
        digitalWrite(MOTOR_PINS[i][0], LOW);
        TMP_TALLY[i]++;
      } 
      // IF TALLY OF MOVEMENTS OF MOTOR IS EQUAL TO ACTUAL TALLY, DO:
      if ((TMP_TALLY[i] == TALLY[i])) {
        // STOP THE MOTOR, AND ADD TO CNT, TO STATE IT HAS REACHED ITS DESTINATION.
        digitalWrite(MOTOR_PINS[i][0], LOW);

        if (VERIFY_CNT_CHECK[i] == 0) {
          VERIFY_CNT_CHECK[i] = 1;
          CNT_MOT_CHECK++;
        }
      }

      // Setting Motor driver enable to HIGH.
      digitalWrite(MOTOR_PINS[i][2], HIGH);      
    }
  }
}

void setup() {
  CONFIG_P();

  MOVE_HOME_POS();
  CALC_MOVES();
  delay(1000);
  MOV_MOTORS();
}

void loop() {

}
