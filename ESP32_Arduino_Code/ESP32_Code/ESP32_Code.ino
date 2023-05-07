#include "BluetoothSerial.h"

#define TX_PIN 1
#define RX_PIN 3

BluetoothSerial SerialBT;

enum section{
  neck,
  chest,
  waist,
  hips
};

const int circsCnt = 4;

// ADDED Values for now, will change once we have updated and tested the Bluetooth bash file on Linux.
float circs[circsCnt] = {20.37, 10.20};
float circsOld[circsCnt] = {20.37, 10.20};
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
      Serial.printf("Circumference %d: %x\n", i+1, data);
      circs[i] = data.toFloat();
      if(circs[i] <= 0 || circs[i] >= 50) recvBT = false;
      else recvBT = true;
    }
  }
}

void txMega(){
  int numAttempts = 0;
  // Makes sure that sending device is there.
  Serial2.println('$');

  for(int i = 0; i < circsCnt;){
    Serial.printf("\nAttempt number %d", numAttempts);
    if(Serial2.available() && Serial2.read() == '$'){
      switch(section(i)){
      case neck:
        Serial2.printf("n%d\n", circs[i]);
        break;
      case chest:
        Serial2.printf("c%d\n", circs[i]);
        break;
      case waist:
        Serial2.printf("w%d\n", circs[i]);
        break;
      case hips:
        Serial2.printf("h%d\n", circs[i]);
        break;
      }
      i++;
    }else{
      numAttempts++;
      Serial.println("\nUnable to connect, reattempting...");
      delay(100);
    }

    if(numAttempts > 5) break;
  }
}

void rxMega() {
  if(Serial2.available() && Serial2.readString() == "esp") txMega();
  else if(Serial2.available()) Serial.println(Serial2.read());
}

void setup() {
  Serial.begin(115200);
  Serial2.begin(115200, RX_PIN, TX_PIN);
  SerialBT.begin();
  Serial.println("Bluetooth device is ready to pair");

  for(int i = 0; i < circsCnt; i++) circsOld[i] = circs[i];
}

void loop() {
  for(int i = 0; i < circsCnt; i++) circsOld[i] = circs[i];

  rxFileBT();
  delay(1000);
  txMega();
  delay(1000);
  
}
