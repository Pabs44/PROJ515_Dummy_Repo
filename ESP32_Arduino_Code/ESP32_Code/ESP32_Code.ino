#include "BluetoothSerial.h"

BluetoothSerial SerialBT;

void SEND(String Send_data) {
  SerialBT.print(Send_data);
}

void RECV() {
  if (SerialBT.available()) {
    char Recv_data = SerialBT.read();
    Serial.write(Recv_data);
  }
}

void READ_FILE() {
  uint8_t msg = 1;
  SerialBT.write(msg);
  SerialBT.println(1);
  while (!SerialBT.available());

  if (SerialBT.available()) {
    String Recv_data = SerialBT.readString();
    Serial.println(Recv_data);
  }
}

void setup() {
  Serial.begin(115200);
  SerialBT.begin("ESP32");
  Serial.println("Bluetooth device is ready to pair");
}

void loop() {
  READ_FILE();
  delay(1000);
}
