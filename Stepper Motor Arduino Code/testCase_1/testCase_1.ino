String data;
char d1;
String x;
int mm;

/*     Simple Stepper Motor Control Exaple Code
 *      
 *  by Dejan Nedelkovski, www.HowToMechatronics.com
 *  
 */

// defines pins numbers
const int stepPin = 3;
const int dirPin = 4;

void setup() {
  Serial.begin(9600);

  // Sets the two pins as Outputs
  pinMode(stepPin, OUTPUT);
  pinMode(dirPin, OUTPUT);
}
void loop() {
  if (Serial.available()) {
    data = Serial.readString();
    d1 = data.charAt(0);

    switch(d1) {  //Select the action based on the first character
    
    //Spin motor forwards
    case 'A':
    motorForwards(1, dirPin, stepPin);
    break;

    //Spin motor backwards
    case 'a':
    motorBackwards(1, dirPin, stepPin);
    break;

    //Enter the mm 
    case 'S':
    x = data.substring(1);
    mm = x.toInt();
    motorForwards (mm, dirPin, stepPin);
    break;


    }
  }
}

void motorForwards (int distanceMil, int directionPin, int stepPin){
  digitalWrite(directionPin, HIGH);
  for (distanceMil; distanceMil > 0; distanceMil--){
    for (int x = 0; x < 200; x++)
    {
      digitalWrite(stepPin,HIGH);
      delayMicroseconds(500);
      digitalWrite(stepPin,LOW);
      delayMicroseconds(500);
    }
    delay(10);

  }
}

void motorBackwards (int distanceMil, int directionPin, int stepPin){
  digitalWrite(directionPin, LOW);
  for (distanceMil; distanceMil > 0; distanceMil--){
    for (int x = 0; x < 200; x++)
    {
      digitalWrite(stepPin,HIGH);
      delayMicroseconds(500);
      digitalWrite(stepPin,LOW);
      delayMicroseconds(500);
    }
    delay(10);

  }
}