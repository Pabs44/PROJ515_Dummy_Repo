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
  SerialBT.println("1");
}

void setup() {
  Serial.begin(115200);
  SerialBT.begin("ESP32");
  Serial.println("Bluetooth device is ready to pair");
  READ_FILE();
  Serial.println("Opened File.");
}

void loop() {
  READ_FILE();
  delay(1000);
}
