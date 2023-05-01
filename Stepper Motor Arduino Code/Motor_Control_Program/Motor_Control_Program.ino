// https://www.digikey.co.uk/en/maker/blogs/2022/how-to-write-multi-threaded-arduino-programs (Maybe rewrite to include this.)
#define AMOUNT_MOT 2
enum MAN_LIM_STEP {NECK_MOT = 250, FRONT = 250, BACK = 250, SIDE = 185};

#define AMOUNT_ULTRA 2
#define MOT_DELAY 1

float MOTOR_CIR_DIST_ARRAY[4] = {400.5, 925.3, 735.8, 960.4};

struct VARS_PINS {
  float FRONT_COG_RAD, CHEST_WAIST_COG_RAD, NECK_COG_RAD, DEG_SIZE[5], MAX_MOT[AMOUNT_MOT], MIN_MOT[AMOUNT_MOT], MAX_CIRC_DIST[4], MIN_CIRC_DIST[4];
  int MOTOR_PINS[12][3], LIMIT_PINS[AMOUNT_MOT], ULTRA_PINS[AMOUNT_ULTRA][2], LED_PINS[3], BUTTON, TALLY[AMOUNT_MOT], CHECK_SW[AMOUNT_MOT], init_sw[AMOUNT_MOT];
  long DEBOUNCE_SW[AMOUNT_MOT][2];
} V_P = {4.1, 8.38, 5.17, {1.8, 0.9, 0.45, 0.225, 0.1125}, {}, {0, 0}, {440, 1020, 840, 1070}, {350, 840, 660, 890}, {}, {}, {}, {14, 15, 19}, 18, {}, {}, {}};

enum BODY_PART {NECK = 0, CHEST, WAIST, HIP};

void CONFIG_MOTORS_SW_ULTRA() {
  int MOTORS[12][3] = {{22, 23, 46}, {24, 25, 47}, {26, 27, 48}, {28, 29, 49}, {30, 31, 50}, {32, 33, 51}, {34, 35, 52}, {36, 37, 53}, {38, 39, 14}, {40, 41, 15}, {42, 43, 16}, {44, 45, 17}};
  int SW[12] = {2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13};
  int ULTRA[8][2] = {{54, 55}, {56, 57}, {58, 59}, {60, 61}, {62, 63}, {64, 65}, {66, 67}, {68, 69}}; 

  for (int i = 0; i < AMOUNT_MOT; i++) {
    V_P.LIMIT_PINS[i] = SW[i];
    pinMode(V_P.LIMIT_PINS[i], INPUT);
    for (int j = 0; j < 2; j++) {
      V_P.MOTOR_PINS[i][j] = MOTORS[i][j];
      pinMode(V_P.MOTOR_PINS[i][j], OUTPUT);
    }
    V_P.MOTOR_PINS[i][2] = MOTORS[i][2];
    pinMode(V_P.MOTOR_PINS[i][2], OUTPUT);
    digitalWrite(V_P.MOTOR_PINS[i][2], HIGH);
  }

  for (int i = 0; i < (12 - AMOUNT_MOT); i++) {
    for (int j = 0; j < 2; j++) {
      V_P.MOTOR_PINS[i][j] = MOTORS[i][j];
      pinMode(V_P.MOTOR_PINS[i][j], OUTPUT);
      digitalWrite(V_P.MOTOR_PINS[i][j], LOW);
    }
    V_P.MOTOR_PINS[i][2] = MOTORS[i][2];
    pinMode(V_P.MOTOR_PINS[i][2], OUTPUT);
    digitalWrite(V_P.MOTOR_PINS[i][2], HIGH);
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
  int ULT_CONFIG[2] = {0, 1};

  // int* ULT_CONFIG = ULTRA_CONFIG(MOT_ID);

  // NUMBER of ultrasonic sensors for plate.
  for (int i = 0; i < 2; i++) {
    // TRIG PIN. (RESET)
    digitalWrite(V_P.ULTRA_PINS[ULT_CONFIG[i]][0], LOW);
    wait_us(2);

    // TRIG PIN. (SEND PULSE)
    digitalWrite(V_P.ULTRA_PINS[ULT_CONFIG[i]][0], HIGH);
    wait_us(10);
    digitalWrite(V_P.ULTRA_PINS[ULT_CONFIG[i]][0], LOW);

    // ECHO PIN. (CALCULATE DISTANCE FROM RETURN PULSE)
    unsigned long DURATION_PULSE = pulseIn(V_P.ULTRA_PINS[ULT_CONFIG[i]][1], HIGH, 6000);
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
  int CNT_MOT_CHECK = 0, VERIFY_CNT_CHECK[AMOUNT_MOT], TMP_TALLY[AMOUNT_MOT], CNT_NON_MOVE_ULTRA[AMOUNT_MOT], CNT_CHECK_ULT[AMOUNT_MOT], CNT_UNRESPONSIVE_ULT[AMOUNT_MOT][2];
  float PREV_DIST_ULTRAS[AMOUNT_MOT], PREV_DIST_ULTRA[AMOUNT_MOT][2];

  for (int i = 0; i < AMOUNT_MOT; i++) {
    VERIFY_CNT_CHECK[i] = 0;
    TMP_TALLY[i] = 0;
    CNT_NON_MOVE_ULTRA[i] = 0;
    PREV_DIST_ULTRAS[i] = 0;
    PREV_DIST_ULTRA[i][0] = 0;
    PREV_DIST_ULTRA[i][1] = 0;
    CNT_UNRESPONSIVE_ULT[i][0] = 0;
    CNT_UNRESPONSIVE_ULT[i][1] = 0;
    CNT_CHECK_ULT[i] = 0;
  }

  while (CNT_MOT_CHECK != AMOUNT_MOT) {
    digitalWrite(V_P.LED_PINS[1], HIGH);
    // RUNNING THROUGH THE MOTORS.
    for (int i = 0; i < AMOUNT_MOT; i++) {
      // Setting Motor driver enable to active low.
      digitalWrite(V_P.MOTOR_PINS[i][2], LOW);
      if (CNT_CHECK_ULT[i] == 15) {
        if (i != 0) {
          float* CHK_DIST = CHECK_DISTANCE_ULTRA(i);
          
          if ((int)PREV_DIST_ULTRAS[i] == (int)((CHK_DIST[0] + CHK_DIST[1]) / 2)) {
            if (CHK_DIST[0] != 0) CNT_UNRESPONSIVE_ULT[i][0] = 0;
            if (CHK_DIST[1] != 0) CNT_UNRESPONSIVE_ULT[i][1] = 0;

            CNT_NON_MOVE_ULTRA[i]++;
            if (CNT_NON_MOVE_ULTRA[i] == 15) {
              int* ULT_CONFIG = ULTRA_CONFIG(i);
              Serial.println("Motor between plates " + String(ULT_CONFIG[0]) + " and " + String(ULT_CONFIG[1]) + " is not showing movement.");
              CNT_NON_MOVE_ULTRA[i] = 0;
            }
            // Checking if the ultrasonic has not been pulled out.
          } else if (((PREV_DIST_ULTRA[i][0] && CHK_DIST[0]) == 0) || ((PREV_DIST_ULTRA[i][1] && CHK_DIST[1]) == 0)) {
            if (CHK_DIST[0] == 0) {
              CNT_UNRESPONSIVE_ULT[i][0]++;
            } else if (CHK_DIST[1] == 0) {
              CNT_UNRESPONSIVE_ULT[i][1]++;
            }

            // Checking if both ultrasonic counts are not over 15.
            for (int j = 0; j < 2; j++) {
              if (CNT_UNRESPONSIVE_ULT[i][j] == 15) {
                int* ULT_CONFIG = ULTRA_CONFIG(i);
                Serial.println("Ultrasonic " + String(ULT_CONFIG[j]) + " not responding.");
                CNT_UNRESPONSIVE_ULT[i][j] = 0;
              }
            }
          }

          PREV_DIST_ULTRA[i][0] = CHK_DIST[0];
          PREV_DIST_ULTRA[i][0] = CHK_DIST[1];
          PREV_DIST_ULTRAS[i] = (CHK_DIST[0] + CHK_DIST[1]) / 2;
        }
      } else {
        CNT_CHECK_ULT[i]++;
      }

      // IF TALLY OF MOVEMENTS OF MOTOR IS LESS THAN THE ACTUAL TALLY OF STEPS, DO:
      if (TMP_TALLY[i] < V_P.TALLY[i]) {
        // STEP AND INCREASE TALLY OF MOVEMENTS OF THE MOTOR.
        digitalWrite(V_P.MOTOR_PINS[i][0], HIGH);
        wait_us((MOT_DELAY * 1000) / (AMOUNT_MOT - CNT_MOT_CHECK));
        digitalWrite(V_P.MOTOR_PINS[i][0], LOW);
        TMP_TALLY[i]++;
      } 
      // IF TALLY OF MOVEMENTS OF MOTOR IS EQUAL TO ACTUAL TALLY, DO:
      if ((TMP_TALLY[i] == V_P.TALLY[i])) {
        // STOP THE MOTOR, AND ADD TO CNT, TO STATE IT HAS REACHED ITS DESTINATION.
        digitalWrite(V_P.MOTOR_PINS[i][0], LOW);
        if (VERIFY_CNT_CHECK[i] == 0) {
          VERIFY_CNT_CHECK[i] = 1;
          CNT_MOT_CHECK++;
        }
      }
      digitalWrite(V_P.LED_PINS[1], LOW);

      // Setting Motor driver enable to HIGH.
      digitalWrite(V_P.MOTOR_PINS[i][2], HIGH);      
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

    // MAKING sure that the initial switch and check switch variables are definitely 0, as variable sometimes floats.
    V_P.init_sw[i] = 0;
    V_P.CHECK_SW[i] = 0;
  }

  int CNT_MOT_CHECK = 0, VERIFY_CNT_CHECK[AMOUNT_MOT];

  for (int i = 0; i < AMOUNT_MOT; i++) {
    VERIFY_CNT_CHECK[i] = 0;
  }

  while (CNT_MOT_CHECK != AMOUNT_MOT) {
    for (int i = 0; i < AMOUNT_MOT; i++) {
      // Setting Motor driver enable to active low.
      digitalWrite(V_P.MOTOR_PINS[i][2], LOW);
      // READING SPECIFIC SWITCH.
      if (digitalRead(V_P.LIMIT_PINS[i]) == 1) DEBOUNCE_SW(i);
      // Checking if switch has been pressed.
      if (V_P.CHECK_SW[i] == 1) {
        // Stopping motors.
        digitalWrite(V_P.MOTOR_PINS[i][0], LOW);
        // Making sure that the count of each motor is only added once.
        if (VERIFY_CNT_CHECK[i] == 0) {
          VERIFY_CNT_CHECK[i] = 1;
          CNT_MOT_CHECK++;
        }
      } else {
        // Step the motors.
        digitalWrite(V_P.MOTOR_PINS[i][0], HIGH);
        wait_us(MOT_DELAY * 1000);
        digitalWrite(V_P.MOTOR_PINS[i][0], LOW);
      }
      // Setting Motor driver enable to HIGH.
      digitalWrite(V_P.MOTOR_PINS[i][2], HIGH);
    }
  }
}


void DEBOUNCE_SW(int ID) {
  // Debouncing using timers.
  // Setting initial time for debouncing of switch.
  if (V_P.init_sw[ID] == 0) {
    V_P.DEBOUNCE_SW[ID][0] = micros();
    V_P.init_sw[ID] = 1;
  }

  // Everytime the function is called, the micro time since startup is taken.
  V_P.DEBOUNCE_SW[ID][1] = micros();

  // IF there is an initial time, DO.
  if (V_P.init_sw[ID] == 1) {
    // IF initial time, and now time has got a difference of 10000, DO.
    if ((V_P.DEBOUNCE_SW[ID][1] - V_P.DEBOUNCE_SW[ID][0]) > 10000) {
      // CHECK SWITCH IS NOT ALREADY INITIATED. 
      // ALLOW switch press to be final.
      if (V_P.CHECK_SW[ID] == 0) V_P.CHECK_SW[ID] = 1;
      // Allow for initial time to be allocated again.
      V_P.init_sw[ID] = 0;
    }    
  }
}

void setup() {
  Serial.begin(115200);
  // For ESP32 UART Communication.
  Serial2.begin(115200);

  // Configuring motors and ultrasonics to their dedicated pins.
  CONFIG_MOTORS_SW_ULTRA();

  // Setting up user experience LEDs.
  for (int i = 0; i < 3; i++) {
    pinMode(V_P.LED_PINS[i], OUTPUT);
    digitalWrite(V_P.LED_PINS[i], LOW);
  }

  // BUTTON PIN AND ATTACHING INTERRUPT.
  pinMode(V_P.BUTTON, INPUT);

  // STARTING BY MOVING HOME.
  MOVE_HOME_POS();

  // CALCULATING AND MOVING MOTORS.
  CALC_MOVES();
  MOV_MOTORS();
}

void loop() {
  // CHECK if button has been pressed.
  if (digitalRead(V_P.BUTTON) == 1) MOVE_HOME_POS();
}
