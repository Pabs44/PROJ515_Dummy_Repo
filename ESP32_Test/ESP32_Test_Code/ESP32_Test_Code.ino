#include "BluetoothSerial.h"

#define RX2_PIN 16
#define TX2_PIN 17

BluetoothSerial SerialBT;

enum section{
  neck,
  chest,
  waist,
  hips
};

const int circsCnt = 4;

//Default values in case no circumferences are given
float circs[circsCnt] = {10.00, 30.00, 40.00, 40.00};
float circsOld[circsCnt] = {10.00, 30.00, 40.00, 40.00};
bool recvBT = false;

void txBT(String data){
  SerialBT.print(data);
}

void rxBT(){
  if(SerialBT.available()) Serial.write(SerialBT.read());
}

void rxFileBT(){
  if(!recvBT){
    uint8_t msg = 1;
    SerialBT.write(msg);
    SerialBT.println(1);
    while (!SerialBT.available());
    
    Serial.println(SerialBT.readStringUntil('!'));
    SerialBT.readStringUntil('\n');
  
    String data;
    for(int i = 0; i < circsCnt; i++){
      switch(section(i)){
      case neck:
        data = SerialBT.readStringUntil('\n');
        break;
      case chest:
        data = SerialBT.readStringUntil('\n');
        break;
      case waist:
        data = SerialBT.readStringUntil('\n');
        break;
      case hips:
        data = SerialBT.readStringUntil('\n');
        break;
      }
      Serial.printf("Circumference %d: %s\n", i+1, data);
      circs[i] = data.toFloat();
      if(circs[i] <= 0 || circs[i] >= 50) recvBT = false;
      else recvBT = true;
    }
  }
}

void txMega(){
  Serial2.flush();
  int numAttempts = 0;
  bool handshake = false;
  unsigned long timeout = 5000, msBegin = 0, msCurr = 0;
  // Makes sure that sending device is there.
  Serial.println("Waiting for Mega");
  while(Serial2.available() < 0);
  Serial.println("Connected, sending handshake");
  Serial2.println('$');

  msBegin = millis();
  while(Serial2.read() != '$'){
    msCurr = millis();
    if(msCurr - msBegin >= timeout){
      Serial.println("Handshake timed out");
      return;
    }
  }
  handshake == true;
  Serial.println("Handshake confirmed");

  Serial2.print('!');
  for(int i = 0; i < circsCnt; i++){
    switch(section(i)){
    case neck:
      Serial.println("Outbound");
      Serial2.print('n');
      Serial2.println(circs[i]);
      break;
    case chest:
      Serial2.print('c');
      Serial2.println(circs[i]);
      break;
    case waist:
      Serial2.print('w');
      Serial2.println(circs[i]);
      break;
    case hips:
      Serial2.print('h');
      Serial2.println(circs[i]);
      break;
    }
  }
}

void rxMega() {
  if(Serial2.available() && Serial2.readString() == "esp") txMega();
  else if(Serial2.available()) Serial.println(Serial2.read());
}

void setup() {
  Serial.begin(115200);
  Serial2.begin(115200, SERIAL_8N1, RX2_PIN, TX2_PIN);
  SerialBT.begin();
  Serial.println("Bluetooth device is ready to pair");

  for(int i = 0; i < circsCnt; i++) circsOld[i] = circs[i];
}

void loop() {
  for(int i = 0; i < circsCnt; i++) circsOld[i] = circs[i];

  for(int i = 0; i < circsCnt; i++){
    switch(i){
    case 0:
      Serial.println("Neck: ");
      break;
    case 1:
      Serial.println("Chest: ");
      break;
    case 2:
      Serial.println("Waist: ");
      break;
    case 3:
      Serial.println("Hips: ");
      break;
    }
    Serial.flush();
    while(Serial.available() <= 0);
    String msg = Serial.readStringUntil('\n');
    Serial.println(msg);
    desiredCircumference[i] = msg.toFloat();
  }

  rxFileBT();
  delay(1000);
  txMega();
  delay(1000);
  
}
