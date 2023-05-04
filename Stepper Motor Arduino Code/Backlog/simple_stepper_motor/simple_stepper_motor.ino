//********************************************************************************************************************************************
//INFORMATION
//200 pulses for making one full cycle rotation.
//Rotating clockwise moves track towards the centre.
//Rotating anti-clockwise moves track away from the centre.
//Takes 855 full steps for translational motor to move track full distance.
//Takes 2000 full steps for rotational motor to cause full rotation of plate.
//Takes 10 rotations of roational motor for one full rotation of plate.
//One rotation of plate causes track to move one rotation of spindle += 200 full steps.
//When track has moved 855 full steps it needs to start rotating other way.
//Setting TdirPin LOW moves magnet away from centre.
//Setting TdirPin HIGH moves magnet towards the centre.
//Setting RdirPin LOW moves plate clockwise.
//Setting RdirPin HIGH moves plates anti-clockwise.
//1 Track step = 10 Rotational Steps
//********************************************************************************************************************************************

// defines pins numbers
const int TstepPin = 3; 
const int TdirPin = 4; 
const int TenablePin = 5;
const int RstepPin = 6; 
const int RdirPin = 7; 
const int RenablePin = 8;

const int Tdir = 1;
const int TstepNum = 2;
const int Rdir = 3;
const int RstepNum = 4;

boolean TstepLevel = HIGH;
boolean RstepLevel = HIGH;

int packetPeriod = 1;

char TchosenDir;
char RchosenDir;
float Tsteps = 0;
float Rsteps = 0;
int switchInput = 1;
 
void setup() {
  // Sets the two pins as Outputs
  pinMode(TstepPin,OUTPUT); 
  pinMode(TdirPin,OUTPUT);
  pinMode(TenablePin,OUTPUT);
  
  pinMode(RstepPin,OUTPUT); 
  pinMode(RdirPin,OUTPUT);
  pinMode(RenablePin,OUTPUT);
  
  digitalWrite(TdirPin,HIGH);
  digitalWrite(TenablePin,LOW);
  digitalWrite(RdirPin,HIGH);
  digitalWrite(RenablePin,LOW);

  Serial.begin(9600);

  noInterrupts();           //disable all interrupts
  TCCR3A = 0;               //set entire TCCR3A register to 0
  TCCR3B = 0;               //set entire TCCR3B register to 0
  TCNT3  = 0;               //initialize counter value to 0

  TCCR4A = 0;               //set entire TCCR4A register to 0
  TCCR4B = 0;               //set entire TCCR4B register to 0
  TCNT4  = 0;               //initialize counter value to 0

//  TCCR5A = 0;               //set entire TCCR5A register to 0
//  TCCR5B = 0;               //set entire TCCR5B register to 0
//  TCNT5  = 0;               //initialize counter value to 0

  TCCR3B |= (1 << WGM12);   //CTC mode
  TCCR3B |= (1 << CS12);    //256 prescaler
  TIMSK3 |= (1 << OCIE3A);  //enable timer compare interrupt

  TCCR4B |= (1 << WGM12);   //CTC mode
  TCCR4B |= (1 << CS12);    //256 prescaler
  TIMSK4 |= (1 << OCIE4A);  //enable timer compare interrupt

//  OCR5A = 10;             //Regular interrupt 10 times a second
//  TCCR5B |= (1 << WGM12);   //CTC mode
//  TCCR5B |= (1 << CS12);    //256 prescaler
//  TIMSK5 |= (1 << OCIE5A);  //enable timer compare interrupt
  interrupts();             //enable all interrupts
}

ISR(TIMER3_COMPA_vect)
{
  if(Tsteps >= 1)
  {
    TstepLevel = !TstepLevel;
    digitalWrite(TstepPin,TstepLevel); 
    Tsteps--;
  }
  else{digitalWrite(TenablePin,HIGH);}
}

ISR(TIMER4_COMPA_vect)
{
  if(Rsteps >= 1)
  {
    RstepLevel = !RstepLevel;
    digitalWrite(RstepPin,RstepLevel); 
    Rsteps--;
  }
  else{digitalWrite(RenablePin,HIGH);}
}

ISR(TIMER5_COMPA_vect)
{

}

void loop()
{
  if ((Tsteps < 1) && (Rsteps < 1))
  {
    while ((Tsteps < 1) || (Rsteps < 1))
    {
      if (Serial.available() > 0) 
      {
        if (switchInput == Tdir)
        {
          TchosenDir = Serial.read();
          switchInput = TstepNum;
          if (TchosenDir == 'p')
          {
            digitalWrite(TdirPin,LOW);
          }
          else if (TchosenDir == 'n')
          {
            digitalWrite(TdirPin,HIGH);
          }
        }
        else if (switchInput == TstepNum)
        {
          digitalWrite(TenablePin,LOW);
          Tsteps = Serial.read();
          Tsteps = Tsteps*2*16;
          OCR3A = packetPeriod*62500/Tsteps;
          switchInput = Rdir;
        }
  
        else if (switchInput == Rdir)
        {
          RchosenDir = Serial.read();
          switchInput = RstepNum;
          if (RchosenDir == 'p')
          {
            digitalWrite(RdirPin,LOW);
          }
          else if (RchosenDir == 'n')
          {
            digitalWrite(RdirPin,HIGH);
          }
        }
        else if (switchInput == RstepNum)
        {
          digitalWrite(RenablePin,LOW);
          Rsteps = Serial.read();
          Rsteps = Rsteps*2*16;
          OCR4A = packetPeriod*62500/Rsteps;
          switchInput = Tdir;
        }
      }
    }
  }
}
