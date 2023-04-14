#define AMOUNT_MOT 2
#define AMOUNT_ULTRA 1
#define MOT_DELAY 1

struct RECVD_DISTANCES {
  float MOTOR_1;
  float MOTOR_2;
} MOV_DIST = {100, 100};

struct VARS_PINS {
  float CHEST_COG_RAD, NECK_COG_RAD, DEG_SIZE[5], MAX_DIST[AMOUNT_MOT];
  int STEP_PINS[AMOUNT_MOT], DIR_PINS[AMOUNT_MOT], LIMIT_PINS[AMOUNT_MOT], ULTRA_PINS[AMOUNT_ULTRA][2], BUTTON, TALLY[AMOUNT_MOT];
  volatile int CHECK_SW[AMOUNT_MOT], init_sw[AMOUNT_MOT], RUNNING_MOT[AMOUNT_MOT], SWITCH_OFF_MOT[AMOUNT_MOT], RUN_TO_HOME;
  volatile long DEBOUNCE_SW[AMOUNT_MOT][2];
} V_P = {4.1, 2.5, {1.8, 0.9, 0.45, 0.225, 0.1125}, {25, 25}, {2, 4}, {3, 5}, {19, 20}, {6, 7}, 18, {}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, 0};

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

void CALC_MOV(float MOTOR_DIS, int MOT_ID, float COG_RAD) {
  noInterrupts();

  // CHECKING IF DISTANCE FROM BLUETOOTH CONTROLLER, IS POSSIBLE AND IS LIMITED IF NOT.
  MOTOR_DIS = (MOTOR_DIS > V_P.MAX_DIST[(MOT_ID - 1)]) ? V_P.MAX_DIST[(MOT_ID - 1)] : MOTOR_DIS;

  // MOVE PLATES ACCORDING TO MEASUREMENTS.
  if (CHECK_DISTANCE_ULTRA(1) > MOTOR_DIS) {
    MOTOR_DIS = CHECK_DISTANCE_ULTRA(1) - MOTOR_DIS;

    // ANTI-CLKWISE, MOVING IN.
    digitalWrite(V_P.DIR_PINS[(MOT_ID - 1)], LOW);
  } else if (CHECK_DISTANCE_ULTRA(1) < MOTOR_DIS) {
    MOTOR_DIS = MOTOR_DIS - CHECK_DISTANCE_ULTRA(1);

    // CLKWISE, MOVING OUT.
    digitalWrite(V_P.DIR_PINS[(MOT_ID - 1)], HIGH);
  }

  // CALCULATING AND STORING STEPS.
  V_P.TALLY[(MOT_ID - 1)] = (int)((MOTOR_DIS / (COG_RAD * (3.14 / 180))) / V_P.DEG_SIZE[0]);

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
    // SETTING ROTATION.
    digitalWrite(V_P.DIR_PINS[i], LOW);
    // ALLOWING THE INTERRUPT TO CHECK IF THE MOTOR IS RUNNING WITHIN THIS FUNCTION.
    V_P.RUNNING_MOT[i] = 1;
  }

  int CNT_MOT_CHECK = 0, VERIFY_CNT_CHECK[AMOUNT_MOT] = {0, 0};

  while (CNT_MOT_CHECK != AMOUNT_MOT) {
    for (int i = 0; i < AMOUNT_MOT; i++) {
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
void DEBOUNCE(int ID, bool SWITCH_OR_BUTTON) {
  switch (SWITCH_OR_BUTTON) {
  case true:
    if (V_P.init_sw[ID] == 0) {
      V_P.DEBOUNCE_SW[ID][0] = micros();
      V_P.init_sw[ID] = 1;
    }

    V_P.DEBOUNCE_SW[ID][1] = micros();

    if (V_P.init_sw[ID] == 1) {
      if ((V_P.DEBOUNCE_SW[ID][1] - V_P.DEBOUNCE_SW[ID][0]) > 100000) {
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
    break;
  
  case false:
    // CHECKS IF FUNCTION IS FINISHED, TO REDUCE INITIATING THE FUNCTION MORE THAN ONCE, FOR SPEED.
    if (V_P.RUN_TO_HOME == 0) V_P.RUN_TO_HOME = 1;
    break;
  }
}

void LIMIT_PUSH_1() {DEBOUNCE(0, true);}
void LIMIT_PUSH_2() {DEBOUNCE(1, true);}
void LIMIT_PUSH_3() {DEBOUNCE(2, true);}
void LIMIT_PUSH_4() {DEBOUNCE(3, true);}
void LIMIT_PUSH_5() {DEBOUNCE(4, true);}
void LIMIT_PUSH_6() {DEBOUNCE(5, true);}
void LIMIT_PUSH_7() {DEBOUNCE(6, true);}
void LIMIT_PUSH_8() {DEBOUNCE(7, true);}
void LIMIT_PUSH_9() {DEBOUNCE(8, true);}
void LIMIT_PUSH_10() {DEBOUNCE(9, true);}
void LIMIT_PUSH_11() {DEBOUNCE(10, true);}
void LIMIT_PUSH_12() {DEBOUNCE(11, true);}
void BUTTON_PUSH() {DEBOUNCE(0, false);}

// TAKEN FROM HERE: https://forum.arduino.cc/t/using-a-variable-as-a-function-name/168313/4
// GETTING ARRAY OF FUNCTIONS.
typedef void (*FuncPtr)(void);

FuncPtr LIMIT_PUSH_FUNC[] = {&LIMIT_PUSH_1, &LIMIT_PUSH_2, &LIMIT_PUSH_3, &LIMIT_PUSH_4, &LIMIT_PUSH_5, &LIMIT_PUSH_6, &LIMIT_PUSH_7, &LIMIT_PUSH_8, &LIMIT_PUSH_9, &LIMIT_PUSH_10, &LIMIT_PUSH_11, &LIMIT_PUSH_12};

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
    // ATTACHING INTERRUPTS.
    attachInterrupt(digitalPinToInterrupt(V_P.LIMIT_PINS[j]), LIMIT_PUSH_FUNC[j], RISING);
  }
  
  // BUTTON PIN AND ATTACHING INTERRUPT.
  pinMode(V_P.BUTTON, INPUT);
  attachInterrupt(digitalPinToInterrupt(V_P.BUTTON), BUTTON_PUSH, RISING);

  // STARTING BY MOVING HOME.
  MOVE_HOME_POS();

  // CALCULATING AND MOVING MOTORS.
  CALC_MOV(MOV_DIST.MOTOR_1, 1, V_P.NECK_COG_RAD);
  CALC_MOV(MOV_DIST.MOTOR_2, 2, V_P.NECK_COG_RAD);
  MOV_MOTORS();
}

void loop() {
  // Interrupt initiates this for button.
  if (V_P.RUN_TO_HOME == 1) MOVE_HOME_POS();
}