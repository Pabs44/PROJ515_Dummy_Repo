#define AMOUNT_MOT 2
#define AMOUNT_ULTRA 1
#define MOT_DELAY 1

struct RECVD_DISTANCES {
  // NECK, CHEST, WAIST, HIP.
  float MOTOR_CIR_DIST_ARRAY[4];
} MOVE_DIST = {400.5, 925.3, 735.8, 960.4};

struct VARS_PINS {
  float FRONT_COG_RAD, CHEST_WAIST_COG_RAD, NECK_COG_RAD, DEG_SIZE[5], MAX_MOT[AMOUNT_MOT], MIN_MOT[AMOUNT_MOT], MAX_CIRC_DIST[4], MIN_CIRC_DIST[4];
  int STEP_PINS[AMOUNT_MOT], DIR_PINS[AMOUNT_MOT], LIMIT_PINS[AMOUNT_MOT], ULTRA_PINS[AMOUNT_ULTRA][2], BUTTON, TALLY[AMOUNT_MOT];
  volatile int CHECK_SW[AMOUNT_MOT], init_sw[AMOUNT_MOT], RUNNING_MOT[AMOUNT_MOT], SWITCH_OFF_MOT[AMOUNT_MOT], RUN_TO_HOME;
  volatile long DEBOUNCE_SW[AMOUNT_MOT][2];
} V_P = {4.1, 8.38, 5.17, {1.8, 0.9, 0.45, 0.225, 0.1125}, {}, {0, 0}, {440, 1020, 840, 1070}, {350, 840, 660, 890}, {2, 4}, {3, 5}, {45, 20}, {6, 7}, 18, {}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, 0};

enum BODY_PART {NECK = 0, CHEST, WAIST, HIP};

void wait_us(int DURATION) {
  int us_previousTime = micros();
  int us_currentTime = micros();
  while((us_currentTime - us_previousTime) <= DURATION) us_currentTime = micros();
}

float CHECK_DISTANCE_ULTRA(int ULTRA_ID) {
  // TRIG PIN. (RESET)
  digitalWrite(V_P.ULTRA_PINS[(ULTRA_ID - 1)][0], LOW);
  wait_us(2);

  // TRIG PIN. (SEND PULSE)
  digitalWrite(V_P.ULTRA_PINS[(ULTRA_ID - 1)][0], HIGH);
  wait_us(10);
  digitalWrite(V_P.ULTRA_PINS[(ULTRA_ID - 1)][0], LOW);

  // ECHO PIN. (CALCULATE DISTANCE FROM RETURN PULSE)
  long DURATION_PULSE = pulseIn(V_P.ULTRA_PINS[(ULTRA_ID - 1)][1], HIGH);
  float DIST = DURATION_PULSE * 0.034 / 2;

  // IF DISTANCE IS BIGGER THAN 40cm
  //                     TRUE   FALSE
  DIST = (DIST > 40.0) ? 40.0 : DIST;

  return DIST;
}

void GROUP_MOTORS(int MEASUREMENT_GROUP, float PERCENTAGE) {
  int MAX_STEPS[4];
  // SETTING ALL MOTORS TO BE MOVING OUT.
  for (int i = 0; i < 12; i++) {
    if (i == 0) { // IF NECK.
      digitalWrite(V_P.DIR_PINS[i], HIGH);
    } else {
      digitalWrite(V_P.DIR_PINS[i], LOW);
    }
  }
  switch (MEASUREMENT_GROUP) {
    case NECK:
      MAX_STEPS[0] = 250;
      V_P.TALLY[0] = (int)(MAX_STEPS[0] * PERCENTAGE);
      break;
    
    case CHEST:
      MAX_STEPS[0] = 250, MAX_STEPS[1] = 185, MAX_STEPS[2] = 250, MAX_STEPS[3] = 185;
      for (int i = 1; i < 5; i++) V_P.TALLY[i] = (int)(MAX_STEPS[(i - 1)] * PERCENTAGE);
      break;

    case WAIST:
      MAX_STEPS[0] = 250, MAX_STEPS[1] = 185, MAX_STEPS[2] = 250, MAX_STEPS[3] = 185;
      for (int i = 5; i < 9; i++) V_P.TALLY[i] = (int)(MAX_STEPS[(i - 5)] * PERCENTAGE);
      break;

    case HIP:
      MAX_STEPS[0] = 185, MAX_STEPS[1] = 250, MAX_STEPS[2] = 185;
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
    
    if (MOVE_DIST.MOTOR_CIR_DIST_ARRAY[i] > V_P.MAX_CIRC_DIST[i]) {
      Serial.println("Circumference measurement for " + TEXT_PART_BODY + " is over the maximum.");
    } else if (MOVE_DIST.MOTOR_CIR_DIST_ARRAY[i] < V_P.MIN_CIRC_DIST[i]) {
      Serial.println("Circumference measurement for " + TEXT_PART_BODY + " is under the minimum.");
    } else {
      float DIFF_LIMITS = V_P.MAX_CIRC_DIST[i] - V_P.MIN_CIRC_DIST[i];
      float DIFF_DES_MIN = MOVE_DIST.MOTOR_CIR_DIST_ARRAY[i] - V_P.MIN_CIRC_DIST[i];

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
      // READING SPECIFIC SWITCH.
      if (digitalRead(V_P.LIMIT_PINS[i]) == 1) DEBOUNCE_SW(i);
        // IF TALLY OF MOVEMENTS OF MOTOR IS LESS THAN THE ACTUAL TALLY OF STEPS, DO:
      if (TMP_TALLY[i] < V_P.TALLY[i]) {
        // STEP AND INCREASE TALLY OF MOVEMENTS OF THE MOTOR.
        digitalWrite(V_P.STEP_PINS[i], HIGH);
        wait_us(MOT_DELAY * 1000);
        digitalWrite(V_P.STEP_PINS[i], LOW);
        TMP_TALLY[i]++;
      } 
      // IF TALLY OF MOVEMENTS OF MOTOR IS EQUAL TO ACTUAL TALLY OR THE LIMIT SWITCH HAS BEEN PRESSED, DO:
      if ((TMP_TALLY[i] == V_P.TALLY[i]) || (V_P.SWITCH_OFF_MOT[i] == 1)) {
        // STOP THE MOTOR, AND ADD TO CNT, TO STATE IT HAS REACHED ITS DESTINATION.
        digitalWrite(V_P.STEP_PINS[i], LOW);
        if (VERIFY_CNT_CHECK[i] == 0) {
          VERIFY_CNT_CHECK[i] = 1;
          CNT_MOT_CHECK++;
        }
      }
    }
  }

  // RESET THE LIMIT SWITCH VARIABLE IF IT BEEN PRESSED AGAIN.
  for (int i = 0; i < AMOUNT_MOT; i++) {
    V_P.SWITCH_OFF_MOT[i] = 0;
  }
}

void MOVE_HOME_POS() {
  for (int i = 0; i < AMOUNT_MOT; i++) {
    // SETTING ROTATION TO MOVE IN.
    if (i == 0) { // IF NECK.
      digitalWrite(V_P.DIR_PINS[i], LOW);
    } else {
      digitalWrite(V_P.DIR_PINS[i], HIGH);
    }
    // ALLOWING THE INTERRUPT TO CHECK IF THE MOTOR IS RUNNING WITHIN THIS FUNCTION.
    V_P.RUNNING_MOT[i] = 1;
  }

  int CNT_MOT_CHECK = 0, VERIFY_CNT_CHECK[AMOUNT_MOT] = {0, 0};

  while (CNT_MOT_CHECK != AMOUNT_MOT) {
    for (int i = 0; i < AMOUNT_MOT; i++) {
      // READING SPECIFIC SWITCH.
      if (digitalRead(V_P.LIMIT_PINS[i]) == 1) DEBOUNCE_SW(i);
      if (V_P.CHECK_SW[i] == 1) {
        digitalWrite(V_P.STEP_PINS[i], LOW);
        V_P.RUNNING_MOT[i] = 0;
        if (VERIFY_CNT_CHECK[i] == 0) {
          VERIFY_CNT_CHECK[i] = 1;
          CNT_MOT_CHECK++;
        }
      } else {
        digitalWrite(V_P.STEP_PINS[i], HIGH);
        wait_us(MOT_DELAY * 1000);
        digitalWrite(V_P.STEP_PINS[i], LOW);
      }
    }
  }

  for (int i = 0; i < AMOUNT_MOT; i++) V_P.CHECK_SW[i] = 0;

  // FUNCTION FINISHED, CAN BE INITIATED AGAIN.
  V_P.RUN_TO_HOME = 0;
}

// Taken advice from here: https://arduino.stackexchange.com/questions/22212/using-millis-and-micros-inside-an-interrupt-routine
void DEBOUNCE_SW(int ID) {
  if (V_P.init_sw[ID] == 0) {
    V_P.DEBOUNCE_SW[ID][0] = micros();
    V_P.init_sw[ID] = 1;
  }

  V_P.DEBOUNCE_SW[ID][1] = micros();

  if (V_P.init_sw[ID] == 1) {
    if ((V_P.DEBOUNCE_SW[ID][1] - V_P.DEBOUNCE_SW[ID][0]) > 50000) {
      // CHECK SWITCH IS NOT ALREADY INITIATED. 
      if (V_P.CHECK_SW[ID] == 0) {
        // IF IN HOME POS, SET CHECK TO 1 TO STOP MOTORS WHEN MOVING HOME.
        if (V_P.RUNNING_MOT[ID] == 1) {
          V_P.CHECK_SW[ID] = 1;
        } else {
          // SWITCH OFF MOTOR IF APPLYING TO MOVING OF THE PLATES.
          V_P.SWITCH_OFF_MOT[ID] = 1;
        }
      }
      V_P.init_sw[ID] = 0;
    }
  }
}

void BUTTON_PUSH() {if (V_P.RUN_TO_HOME == 0) V_P.RUN_TO_HOME = 1;}

void setup() {
  Serial.begin(115200);
  // For ESP32 UART Communication.
  Serial2.begin(115200);

  // PINS OF TRIG AND ECHO, OF EACH SENSOR.
  for (int i = 0; i < AMOUNT_ULTRA; i++) {
    pinMode(V_P.ULTRA_PINS[i][0], OUTPUT);
    pinMode(V_P.ULTRA_PINS[i][1], INPUT);
  }

  // PINS OF MOTORS AND SWITCHES.
  for (int j = 0; j < AMOUNT_MOT; j++) {
    pinMode(V_P.DIR_PINS[j], OUTPUT);
    pinMode(V_P.STEP_PINS[j], OUTPUT);
    pinMode(V_P.LIMIT_PINS[j], INPUT);
  }
  
  // BUTTON PIN AND ATTACHING INTERRUPT.
  pinMode(V_P.BUTTON, INPUT);
  attachInterrupt(digitalPinToInterrupt(V_P.BUTTON), BUTTON_PUSH, RISING);

  // STARTING BY MOVING HOME.
  MOVE_HOME_POS();

  // CALCULATING AND MOVING MOTORS.
  CALC_MOVES();
  delay(1000);
  MOV_MOTORS();
}

void loop() {
  // Interrupt initiates this for button.
  if (V_P.RUN_TO_HOME == 1) MOVE_HOME_POS();
}
