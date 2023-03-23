const int Q3 = 2;
const int Q4 = 3;
const int Q5 = 4;
const int Q6 = 5;
const int Q7 = 6;
const int Q8 = 7;
const int Q9 = 8;
const int Q11 = 9;
const int Q12 = 10;
const int Q13 = 11;
const int RST = 12;
const int CLK = 13;

void setup() {
  pinMode(Q3, INPUT);
  pinMode(Q4, INPUT);
  pinMode(Q5, INPUT);
  pinMode(Q6, INPUT);
  pinMode(Q7, INPUT);
  pinMode(Q8, INPUT);
  pinMode(Q9, INPUT);
  pinMode(Q11, INPUT);
  pinMode(Q12, INPUT);
  pinMode(Q13, INPUT);
  pinMode(RST, OUTPUT);
  pinMode(CLK, OUTPUT);
  
  Serial.begin(9600);

  int MOTOR_ID[2] = {0, 1};
  int DIR_ID[2] = {0, 1};
  int MODE_ID[2][3] = {{1, 0, 1}, {0, 1, 0}};

  RUN_COUNTER_MODE(MOTOR_ID, DIR_ID, MODE_ID);
}

void RUN_COUNTER_MODE(int M[2], int DIR[2], int MODE[2][3]) {
  // RESET COUNTER.
  digitalWrite(RST, HIGH);
  delay(1);
  digitalWrite(RST, LOW);

  // IF ERROR, TO AVOID EXTRA WORK FOR THE COUNTER. (MOTOR DRIVER HAS ACTIVE-LOW ENABLE)
  if ((M[0] == 0) && (M[1] == 1)) {
    DIR[1] = 0;
    MODE[1][0] = 0;
    MODE[1][1] = 0;
    MODE[1][2] = 0;
  }

  if ((M[0] == 1) && (M[1] == 0)) {
    DIR[0] = 0;
    MODE[0][0] = 0;
    MODE[0][1] = 0;
    MODE[0][2] = 0;
  }
  
  // EN1, DIR1, M11, M21, M31, EN2, DIR2, M12, M22, M32
  int C_OUT = (M[0] * pow(2, 3)) + (DIR[0] * pow(2, 4)) + (MODE[0][0] * pow(2, 5)) + (MODE[0][1] * pow(2, 6)) + (MODE[0][2] * pow(2, 7)) + (M[1] * pow(2, 8)) + (DIR[1] * pow(2, 9)) + (MODE[1][0] * pow(2, 11)) + (MODE[1][1] * pow(2, 12)) + (MODE[1][2] * pow(2, 13));

  // RUNNING CLK TO SPECIFIED COUNT.
  for (int i = 0; i < C_OUT; i++) {
    digitalWrite(CLK, HIGH);
    delay(0.01); // SETTING FREQUENCY TO 100KHz.
    analogWrite(CLK, LOW);
  }
}

void loop() {
  Serial.print(digitalRead(Q3));
  Serial.print(digitalRead(Q4));
  Serial.print(digitalRead(Q5));
  Serial.print(digitalRead(Q6));
  Serial.print(digitalRead(Q7));
  Serial.print(digitalRead(Q8));
  Serial.print(digitalRead(Q9));
  Serial.print(digitalRead(Q11));
  Serial.print(digitalRead(Q12));
  Serial.println(digitalRead(Q13));
}
