#include "BluetoothSerial.h"

#define AMOUNT_MOT 2

BluetoothSerial SerialBT;

// ADDED Values for now, will change once we have updated and tested the Bluetooth bash file on Linux.
float CIRCUMFERENCE_DISTANCE_DATA[AMOUNT_MOT] = {20.37, 10.20};

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
  // Waits for response. (Could put a timeout here if wanted).
  while (Serial.available() == 0) {}
  // Start of first value.
  Serial.print('|');
  // Send both values of each motor to Arduino.
  for (int i = 0; i < AMOUNT_MOT; i++) {
    Serial.print(CIRCUMFERENCE_DISTANCE_DATA[i]);
    Serial.print('|');
  }

  // Checks if it has recieved both values. (Sends the integer, 1).
  while (Serial.available() == 0) {}
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
}

void loop() {
  // Sends every second if it is recieved in the time.
  SEND_ARD();
  delay(1000);
}
