// defines pins numbers
const int stepPin = 4; 
const int dirPin = 5; 
 
void setup() {
  Serial.begin(9600);
  // Sets the two pins as Outputs
  pinMode(stepPin,OUTPUT); 
  pinMode(dirPin,OUTPUT);

  // http://cmra.rec.ri.cmu.edu/previews/rcx_products/robotics_educator_workbook/content/mech/pages/Diameter_Distance_TraveledTEACH.pdf 
//  float MOTOR_DIS = ((250.0 * 1.8) / 360) * (2 * 3.14 * 4.1);
//  Serial.println(MOTOR_DIS);
}
void loop() {
  digitalWrite(dirPin, LOW); // Enables the motor to move in a particular direction
  // Makes 200 pulses for making one full cycle rotation
  for(int x = 0; x < 100; x++) {
    digitalWrite(stepPin,HIGH); 
    delayMicroseconds(500); 
    digitalWrite(stepPin,LOW); 
    delayMicroseconds(500); 
  }

  delay(1000);

  digitalWrite(dirPin, HIGH); // Enables the motor to move in a particular direction
  // Makes 200 pulses for making one full cycle rotation
  for(int x = 0; x < 100; x++) {
    digitalWrite(stepPin,HIGH); 
    delayMicroseconds(500); 
    digitalWrite(stepPin,LOW); 
    delayMicroseconds(500); 
  }

  delay(1000);
}
