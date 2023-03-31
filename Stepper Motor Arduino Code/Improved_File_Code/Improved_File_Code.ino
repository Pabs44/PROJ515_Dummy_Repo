#define AMOUNT_MOT 2
#define AMOUNT_ULTRA 1
#define MOT_DELAY 1

struct RECVD_DISTANCES {
  float MOTOR_1;
  float MOTOR_2;
} MOV_DIST = {100, 100};

struct VARS_PINS {
  float CHEST_COG_RAD, NECK_COG_RAD, DEG_SIZE[5], MAX_DIST[AMOUNT_MOT];
  int STEP_PINS[AMOUNT_MOT], DIR_PINS[AMOUNT_MOT], LIMIT_PINS[AMOUNT_MOT], ULTRA_PINS[AMOUNT_ULTRA][2], BUTTON, TALLY[AMOUNT_MOT], CHECK[AMOUNT_MOT], DEBOUNCE[AMOUNT_MOT][2], init_sw[AMOUNT_MOT];
} V_P = {4.1, 2.5, {1.8, 0.9, 0.45, 0.225, 0.1125}, {25, 25}, {2, 4}, {3, 5}, {19, 20}, {6, 7}, 20, {}, {0, 0}, {}, {0, 0}};

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
  for (int i = 0; i < AMOUNT_MOT; i++) {
    if (V_P.TALLY[i] > 0) {
      for (int j = 0; j < V_P.TALLY[i]; j++) {
        digitalWrite(V_P.STEP_PINS[i], HIGH);
        wait_us(MOT_DELAY * 1000);
        digitalWrite(V_P.STEP_PINS[i], LOW);
      }
    }
  }
}

void MOVE_HOME_POS() {
  // SELECTING MOTOR TO MOVE.
  for (int i = 0; i < AMOUNT_MOT; i++) {
    // SETTING ROTATION.
    digitalWrite(V_P.DIR_PINS[i], LOW);

    // RUNS TILL HIT OF LIMIT SWITCH.
    while (V_P.CHECK[i] == 0) {
        digitalWrite(V_P.STEP_PINS[i], HIGH);
        wait_us(MOT_DELAY * 1000); //the delay() function is in ms so use that?
        digitalWrite(V_P.STEP_PINS[i], LOW);
    }

    // RESETTING CHECK.
    V_P.CHECK[i] = 0;
    Serial.println("E");  
    while (digitalRead(V_P.LIMIT_PINS[i]) == HIGH);
  }
}

// Interrupts for arduino.
// https://4donline.ihs.com/images/VipMasterIC/IC/MCHP/MCHP-S-A0010212075/MCHP-S-A0010410632-1.pdf?hkey=6D3A4C79FDBF58556ACFDE234799DDF0
// https://arduino.stackexchange.com/questions/30968/how-do-interrupts-work-on-the-arduino-uno-and-similar-boards 

void LIMIT_PUSH(int ID) {
  // Setting initial time of first rise of interrupt.
  if (V_P.init_sw[0] == 0) {
    V_P.DEBOUNCE[ID][0] = millis();
    V_P.init_sw[ID] = 1;
  }

  // Setting actual time.
  V_P.DEBOUNCE[ID][1] = millis();

  // Getting difference of both times, and if the bouncing of the button has gone.
  if ((V_P.DEBOUNCE[ID][1] - V_P.DEBOUNCE[ID][0]) > 200) {
    // Add one to check.
    V_P.CHECK[ID] = 1;
    V_P.init_sw[ID] = 0;
  }
}

void LIMIT_PUSH_1() {LIMIT_PUSH(0);}
void LIMIT_PUSH_2() {LIMIT_PUSH(1);}

// TAKEN FROM HERE: https://forum.arduino.cc/t/using-a-variable-as-a-function-name/168313/4
// GETTING ARRAY OF FUNCTIONS.
typedef void (*FuncPtr)(void);

FuncPtr LIMIT_PUSH_FUNC[] = {&LIMIT_PUSH_1, &LIMIT_PUSH_2};

void setup() {
  Serial.begin(9600);

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
  attachInterrupt(digitalPinToInterrupt(V_P.BUTTON), MOVE_HOME_POS, RISING);

  // STARTING BY MOVING HOME.
  // MOVE_HOME_POS();

  // CALCULATING AND MOVING MOTORS.
  CALC_MOV(MOV_DIST.MOTOR_1, 1, V_P.NECK_COG_RAD);
  CALC_MOV(MOV_DIST.MOTOR_2, 2, V_P.NECK_COG_RAD);
  MOV_MOTORS();
}

void loop() {
  // put your main code here, to run repeatedly:

}
