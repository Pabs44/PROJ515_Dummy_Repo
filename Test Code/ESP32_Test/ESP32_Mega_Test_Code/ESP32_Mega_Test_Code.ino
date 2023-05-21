#define RX1_PIN 19
#define TX1_PIN 18

const int circsCnt = 4;
float circs[circsCnt];

void setup() {
  Serial.begin(115200);
  Serial1.begin(115200);
}

void loop() {
  Serial1.flush();
  byte recvCircs = 0b0000;
  bool handshake = false;
  unsigned long timeout = 5000, msBegin = 0, msCurr = 0;

  Serial.println("\nWaiting for ESP");
  while(Serial1.available() < 0);
  Serial.println("Connected");

  msBegin = millis();
  while(Serial1.read() != '$'){
    msCurr = millis();
    if(msCurr - msBegin >= timeout){
      Serial.println("Handshake timed out");
      break;
    }
  }
  handshake = true;
  Serial.println("Handshake initiated, sending confirmation");
  Serial1.println('$');

  while (handshake && recvCircs != 0b1111) {
    int numAttempts = 0, badData = 0, goodData = 0;

    if(Serial1.available() > 0){
      Serial1.readStringUntil('!');
      for(int i = 0; i < circsCnt; i++){
        Serial.println("Next circ");
        String msg = Serial1.readStringUntil('\n');
        if(msg.charAt(0) == 'n'){
          msg.remove(0,1);
          Serial.println(msg);
          circs[i] = msg.toFloat();
          recvCircs |= 0b0001;
          goodData++;
          badData = 0;
        }else if(msg.charAt(0) == 'c'){
          msg.remove(0,1);
          Serial.println(msg);
          circs[i] = msg.toFloat();
          recvCircs |= 0b0010;
          goodData++;
          badData = 0;
        }else if(msg.charAt(0) == 'w'){
          msg.remove(0,1);
          Serial.println(msg);
          circs[i] = msg.toFloat();
          recvCircs |= 0b0100;
          goodData++;
          badData = 0;
        }else if(msg.charAt(0) == 'h'){
          msg.remove(0,1);
          Serial.println(msg);
          circs[i] = msg.toFloat();
          recvCircs |= 0b1000;
          goodData++;
          badData = 0;
        }else{
          Serial.println("Bad data coming in");
          badData++;
        }

        if(goodData >= 4){
          Serial.println("All circumferences recorded");
          break;
        }else if(badData >= 12){
          Serial.println("Too much bad data, fix ESP output");
          break;
        }
      }
    }else{
      numAttempts++;
      Serial.println("\nUnable to connect, reattempting...");
    }

    if(numAttempts > 5){
      Serial.println("ESP disconnected, reattempt connection");
      break;
    }
  }

  for(int i = 0; i < circsCnt; i++) Serial1.println(circs[i]);
}
