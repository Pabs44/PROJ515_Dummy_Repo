#include "BluetoothSerial.h"

#define AMOUNT_MOT 2

BluetoothSerial SerialBT;

// ADDED Values for now, will change once we have updated and tested the Bluetooth bash file on Linux.
float CIRCUMFERENCE_DISTANCE_DATA[AMOUNT_MOT] = {20.37, 10.20};
float PREV_VALUES[AMOUNT_MOT];
int CNT_CHANGE;

void SEND_BT(String Send_data) {
  SerialBT.print(Send_data);
}

void RECV_BT() {
  if (SerialBT.available()) {
    char Recv_data = SerialBT.read();
    Serial.write(Recv_data);
  }
}

void READ_FILE_BT() {
  uint8_t msg = 1;
  SerialBT.write(msg);
  SerialBT.println(1);
  while (!SerialBT.available());
  
  String Recv_data = SerialBT.readString();
  Serial.println(Recv_data);
}

void SEND_ARD() {
  // Makes sure that sending device is there.
  Serial.print('$');
  // Waits for response (Re-sends '$' if it isn't sorted within the time period).
  int init_t, now_t, cnt = 0;
  while ((Serial.available() == 0) && (Serial.read() != '$')) {
    if(cnt == 0) init_t = millis(); cnt++;
    
    now_t = millis();

    if ((now_t - init_t) >= 5000) {
      Serial.print('$');
      cnt = 0;
    }
  }
  // Start of first value.
  Serial.print('|');
  // Send both values of each motor to Arduino.
  for (int i = 0; i < AMOUNT_MOT; i++) {
    Serial.print(CIRCUMFERENCE_DISTANCE_DATA[i]);
    Serial.print('|');
  }

  // Checks if it has recieved both values. (Sends the integer, 1).
  while ((Serial.available() == 0) && (Serial.read() == '1')) {
    
  }
}

void RECV_ARD() {
  if (Serial.available()) {
    String Recv_data = Serial.readString();
    Serial.println(Recv_data);
  }
}

void setup() {
  Serial.begin(115200);
  SerialBT.begin("ESP32");
  Serial.println("Bluetooth device is ready to pair");

  for (int i = 0; i < AMOUNT_MOT; i++) {
    PREV_VALUES[i] = CIRCUMFERENCE_DISTANCE_DATA[i];
  }
  SEND_ARD();
}

void loop() {
  CNT_CHANGE = 0;
  for (int i = 0; i < AMOUNT_MOT; i++) {
    if (CIRCUMFERENCE_DISTANCE_DATA[i] != PREV_VALUES[i]) {
      CNT_CHANGE++;
    }
  }

  if (CNT_CHANGE > 0) SEND_ARD();
}
