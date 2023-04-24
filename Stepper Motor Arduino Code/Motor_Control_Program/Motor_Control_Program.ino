#define AMOUNT_MOT 3
enum MAN_LIM_STEP {NECK_MOT = 250, FRONT = 250, BACK = 250, SIDE = 185};

#define AMOUNT_ULTRA 1
#define MOT_DELAY 1

float MOTOR_CIR_DIST_ARRAY[4] = {400.5, 925.3, 735.8, 960.4};

struct VARS_PINS {
  float FRONT_COG_RAD, CHEST_WAIST_COG_RAD, NECK_COG_RAD, DEG_SIZE[5], MAX_MOT[AMOUNT_MOT], MIN_MOT[AMOUNT_MOT], MAX_CIRC_DIST[4], MIN_CIRC_DIST[4];
  int MOTOR_PINS[AMOUNT_MOT][2], LIMIT_PINS[AMOUNT_MOT], ULTRA_PINS[AMOUNT_ULTRA][2], BUTTON, TALLY[AMOUNT_MOT], CHECK_SW[AMOUNT_MOT], init_sw[AMOUNT_MOT];
  long DEBOUNCE_SW[AMOUNT_MOT][2];
} V_P = {4.1, 8.38, 5.17, {1.8, 0.9, 0.45, 0.225, 0.1125}, {}, {0, 0}, {440, 1020, 840, 1070}, {350, 840, 660, 890}, {}, {}, {}, 18, {}, {}, {}};

enum BODY_PART {NECK = 0, CHEST, WAIST, HIP};

void CONFIG_MOTORS_SW_ULTRA() {
  int MOTORS[12][2] = {{22, 23}, {24, 25}, {26, 27}, {28, 29}, {30, 31}, {32, 33}, {34, 35}, {36, 37}, {38, 39}, {40, 41}, {42, 43}, {44, 45}};
  int SW[12] = {2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13};
  int ULTRA[8][2] = {{54, 55}, {56, 57}, {58, 59}, {60, 61}, {62, 63}, {64, 65}, {66, 67}, {68, 69}}; 

  for (int i = 0; i < AMOUNT_MOT; i++) {
    V_P.LIMIT_PINS[i] = SW[i];
    pinMode(V_P.LIMIT_PINS[i], INPUT);
    for (int j = 0; j < 2; j++) {
      V_P.MOTOR_PINS[i][j] = MOTORS[i][j];
      pinMode(V_P.MOTOR_PINS[i][j], OUTPUT);
    }
  }

  for (int i = 0; i < AMOUNT_ULTRA; i++) {
    V_P.ULTRA_PINS[i][0] = ULTRA[i][0];
    V_P.ULTRA_PINS[i][1] = ULTRA[i][1];
    pinMode(V_P.ULTRA_PINS[i][0], OUTPUT); // TRIG
    pinMode(V_P.ULTRA_PINS[i][1], INPUT); // ECHO
  }
}

void wait_us(int DURATION) {
  int us_previousTime = micros();
  int us_currentTime = micros();
  while((us_currentTime - us_previousTime) <= DURATION) us_currentTime = micros();
}

float CHECK_DISTANCE_ULTRA(int ULTRA_ID) {
  // TRIG PIN. (RESET)
  digitalWrite(V_P.ULTRA_PINS[ULTRA_ID][0], LOW);
  wait_us(2);

  // TRIG PIN. (SEND PULSE)
  digitalWrite(V_P.ULTRA_PINS[ULTRA_ID][0], HIGH);
  wait_us(10);
  digitalWrite(V_P.ULTRA_PINS[ULTRA_ID][0], LOW);

  // ECHO PIN. (CALCULATE DISTANCE FROM RETURN PULSE)
  long DURATION_PULSE = pulseIn(V_P.ULTRA_PINS[ULTRA_ID][1], HIGH);
  float DIST = DURATION_PULSE * 0.034 / 2;

  // IF DISTANCE IS BIGGER THAN 40cm
  //                     TRUE   FALSE
  DIST = (DIST > 40.0) ? 40.0 : DIST;

  return DIST;
}

void GROUP_MOTORS(int MEASUREMENT_GROUP, float PERCENTAGE) {
  int MAX_STEPS[4];
  // SETTING ALL MOTORS TO BE MOVING OUT.
  for (int i = 0; i < AMOUNT_MOT; i++) {
    if (i == 0) { // IF NECK.
      digitalWrite(V_P.MOTOR_PINS[i][1], HIGH);
    } else {
      digitalWrite(V_P.MOTOR_PINS[i][1], LOW);
    }
  }
  switch (MEASUREMENT_GROUP) {
    case NECK:
      MAX_STEPS[0] = NECK_MOT;
      V_P.TALLY[0] = (int)(MAX_STEPS[0] * PERCENTAGE);
      break;
    
    case CHEST:
      MAX_STEPS[0] = SIDE, MAX_STEPS[1] = SIDE, MAX_STEPS[2] = BACK, MAX_STEPS[3] = FRONT;
      for (int i = 1; i < 5; i++) V_P.TALLY[i] = (int)(MAX_STEPS[(i - 1)] * PERCENTAGE);
      break;

    case WAIST:
      MAX_STEPS[0] = SIDE, MAX_STEPS[1] = SIDE, MAX_STEPS[2] = BACK, MAX_STEPS[3] = FRONT;
      for (int i = 5; i < 9; i++) V_P.TALLY[i] = (int)(MAX_STEPS[(i - 5)] * PERCENTAGE);
      break;

    case HIP:
      MAX_STEPS[0] = SIDE, MAX_STEPS[1] = SIDE, MAX_STEPS[2] = BACK;
      for (int i = 9; i < 12; i++) V_P.TALLY[i] = (int)(MAX_STEPS[(i - 9)] * PERCENTAGE);
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
    
    if (MOTOR_CIR_DIST_ARRAY[i] > V_P.MAX_CIRC_DIST[i]) {
      Serial.println("Circumference measurement for " + TEXT_PART_BODY + " is over the maximum.");
    } else if (MOTOR_CIR_DIST_ARRAY[i] < V_P.MIN_CIRC_DIST[i]) {
      Serial.println("Circumference measurement for " + TEXT_PART_BODY + " is under the minimum.");
    } else {
      float DIFF_LIMITS = V_P.MAX_CIRC_DIST[i] - V_P.MIN_CIRC_DIST[i];
      float DIFF_DES_MIN = MOTOR_CIR_DIST_ARRAY[i] - V_P.MIN_CIRC_DIST[i];

      float PERCENTAGE = DIFF_DES_MIN / DIFF_LIMITS;
      GROUP_MOTORS(i, PERCENTAGE);
    }
  }
  interrupts();
}

// MOVING MOTORS.
void MOV_MOTORS() {
  int CNT_MOT_CHECK = 0, VERIFY_CNT_CHECK[AMOUNT_MOT] = {0, 0}, TMP_TALLY[AMOUNT_MOT] = {0, 0};

  while (CNT_MOT_CHECK != AMOUNT_MOT) {
    // RUNNING THROUGH THE MOTORS.
    for (int i = 0; i < AMOUNT_MOT; i++) {
        // IF TALLY OF MOVEMENTS OF MOTOR IS LESS THAN THE ACTUAL TALLY OF STEPS, DO:
      if (TMP_TALLY[i] < V_P.TALLY[i]) {
        // STEP AND INCREASE TALLY OF MOVEMENTS OF THE MOTOR.
        digitalWrite(V_P.MOTOR_PINS[i][0], HIGH);
        wait_us(MOT_DELAY * 1000);
        digitalWrite(V_P.MOTOR_PINS[i][0], LOW);
        TMP_TALLY[i]++;
      } 
      // IF TALLY OF MOVEMENTS OF MOTOR IS EQUAL TO ACTUAL TALLY OR THE LIMIT SWITCH HAS BEEN PRESSED, DO:
      if ((TMP_TALLY[i] == V_P.TALLY[i])) {
        // STOP THE MOTOR, AND ADD TO CNT, TO STATE IT HAS REACHED ITS DESTINATION.
        digitalWrite(V_P.MOTOR_PINS[i][0], LOW);
        if (VERIFY_CNT_CHECK[i] == 0) {
          VERIFY_CNT_CHECK[i] = 1;
          CNT_MOT_CHECK++;
        }
      }
    }
  }
}

void MOVE_HOME_POS() {
  for (int i = 0; i < AMOUNT_MOT; i++) {
    // SETTING ROTATION TO MOVE IN.
    if (i == 0) { // IF NECK.
      digitalWrite(V_P.MOTOR_PINS[i][1], LOW);
    } else {
      digitalWrite(V_P.MOTOR_PINS[i][1], HIGH);
    }

    if (V_P.init_sw[i] >= 1) {
      V_P.init_sw[i] = 0;
    }
  }

  int CNT_MOT_CHECK = 0, VERIFY_CNT_CHECK[AMOUNT_MOT] = {0, 0};

  while (CNT_MOT_CHECK != AMOUNT_MOT) {
    for (int i = 0; i < AMOUNT_MOT; i++) {
      // READING SPECIFIC SWITCH.
      if (digitalRead(V_P.LIMIT_PINS[i]) == 1) DEBOUNCE_SW(i);
      if (V_P.CHECK_SW[i] == 1) {
        digitalWrite(V_P.MOTOR_PINS[i][0], LOW);
        if (VERIFY_CNT_CHECK[i] == 0) {
          VERIFY_CNT_CHECK[i] = 1;
          CNT_MOT_CHECK++;
        }
      } else {
        digitalWrite(V_P.MOTOR_PINS[i][0], HIGH);
        wait_us(MOT_DELAY * 1000);
        digitalWrite(V_P.MOTOR_PINS[i][0], LOW);
      }
    }
  }

  for (int i = 0; i < AMOUNT_MOT; i++) V_P.CHECK_SW[i] = 0;
}


void DEBOUNCE_SW(int ID) {
  if (V_P.init_sw[ID] == 0) {
    V_P.DEBOUNCE_SW[ID][0] = micros();
    Serial.print("ID = ");
    Serial.println(ID);
    V_P.init_sw[ID] = 1;
  }

  Serial.println(V_P.init_sw[ID]);
  Serial.println(V_P.CHECK_SW[ID]);

  V_P.DEBOUNCE_SW[ID][1] = micros();

  Serial.print("Debounce microsecond difference = ");
  Serial.println((V_P.DEBOUNCE_SW[ID][1] - V_P.DEBOUNCE_SW[ID][0]));
  if (V_P.init_sw[ID] == 1) {
    if ((V_P.DEBOUNCE_SW[ID][1] - V_P.DEBOUNCE_SW[ID][0]) > 10000) {
      // CHECK SWITCH IS NOT ALREADY INITIATED. 
      Serial.println("IN DEBOUNCE.");
      if (V_P.CHECK_SW[ID] == 0) V_P.CHECK_SW[ID] = 1;
      V_P.init_sw[ID] = 0;
    }    
  }
}

void setup() {
  Serial.begin(115200);
  // For ESP32 UART Communication.
  Serial2.begin(115200);

  CONFIG_MOTORS_SW_ULTRA();

  // BUTTON PIN AND ATTACHING INTERRUPT.
  pinMode(V_P.BUTTON, INPUT);

  // STARTING BY MOVING HOME.
  MOVE_HOME_POS();

  // CALCULATING AND MOVING MOTORS.
  CALC_MOVES();
  delay(1000);
  MOV_MOTORS();
}

void loop() {
  // CHECK if button has been pressed.
  if (digitalRead(V_P.BUTTON) == 1) MOVE_HOME_POS();
}