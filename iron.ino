#include <Encoder.h>

#include <Wire.h> 
#include <LiquidCrystal_I2C.h>
LiquidCrystal_I2C lcd(0x27,20,4);

#define tempSensor A0
#define knob A7
#define iron 10
#define GLED 3
#define RLED 2
#define BLED 4

int
minTemp = 27,       //Minimum aquired iron tip temp during testing (°C)
maxTemp = 525,      //Maximum aquired iron tip temp during testing (°C)
minADC  = 234,      //Minimum aquired ADC value during minTemp testing
maxADC  = 733,      //Maximum aquired ADC value during minTemp testing

maxPWM    = 255,    //Maximum PWM Power
avgCounts = 5,     //Number of avg samples
lcdInterval = 80,   //LCD refresh rate (miliseconds) 

pwm = 0,            //System Variable
tempRAW = 0,        //System Variable
knobRAW = 0,        //System Variable
counter = 0,        //System Variable
setTemp = 0,        //System Variable
setTempAVG = 0,     //System Variable
currentTempAVG = 0, //System Variable
previousMillis = 0; //System Variable

float 
currentTemp = 0.0,  //System Variable
store = 0.0,        //System Variable
knobStore = 0.0;    //System Variable

void setup(){
  pinMode(tempSensor,INPUT); //Set Temp Sensor pin as INPUT
  pinMode(knob,INPUT);       //Set Potentiometer Knob as INPUT
  pinMode(iron,OUTPUT);      //Set MOSFET PWM pin as OUTPUT
  pinMode(GLED,OUTPUT);       
  pinMode(RLED,OUTPUT);
  pinMode(BLED,OUTPUT);
  pinMode(A6,INPUT);       //Passthru Pin 
  lcd.backlight();
  lcd.init();
  lcd.clear();
  lcd.setCursor(0,1);lcd.print("PRESET T: ");  
  lcd.setCursor(0,0);lcd.print("ACTUAL T:"); 
}

void loop(){
  //--------Gather Sensor Data--------//
  knobRAW = analogRead(knob); //Get analog value of Potentiometer
  setTemp = map(knobRAW,0,1023,minTemp,maxTemp);  //Scale pot analog value into temp unit

  tempRAW = analogRead(tempSensor);  //Get analog value of temp sensor
  currentTemp = map(analogRead(tempSensor),minADC,maxADC,minTemp,maxTemp);  //Sacle raw analog temp values as actual temp units
  
  //--------Get Average of Temp Sensor and Knob--------//
  if(counter<avgCounts){  //Sum up temp and knob data samples
    store = store+currentTemp;
    knobStore = knobStore+setTemp;
    counter++;
  }
  else{
    currentTempAVG = (store/avgCounts)-1;  //Get temp mean (average)
    setTempAVG = (knobStore/avgCounts);  //Get knob - set temp mean (average)
    knobStore=0;  //Reset storage variable
    store=0;      //Reset storage variable
    counter=0;    //Reset storage variable
  }
  
  //--------PWM Soldering Iron Power Control--------//
  if(analogRead(knob)==0){  //Turn off iron when knob as at its lowest (iron shutdown)
    digitalWrite(RLED, LOW);
    digitalWrite(GLED, LOW);
    digitalWrite(BLED, HIGH);
    pwm=0;
  }
  else if(currentTemp<=setTemp){  //Turn on iron when iron temp is lower than preset temp
    digitalWrite(RLED,HIGH);
    digitalWrite(GLED, LOW);
    digitalWrite(BLED, LOW);
    pwm=maxPWM;
  }
  else{  //Turn off iron when iron temp is higher than preset temp
    digitalWrite(RLED,LOW);
    digitalWrite(GLED,HIGH);
    digitalWrite(BLED, LOW);
    pwm=0;
  }
  analogWrite(iron, pwm);  //Apply the aquired PWM value from the three cases above

  //--------Display Data--------//
  unsigned long currentMillis = millis(); //Use and aquire millis function instead of using delay
  if (currentMillis - previousMillis >= lcdInterval){ //LCD will only display new data ever n milisec intervals
    previousMillis = currentMillis;
 
    if(analogRead(knob)==0){
      lcd.setCursor(10,1);lcd.print("OFF  ");
    }
    else{
      lcd.setCursor(10,1);lcd.print(setTempAVG,1);lcd.print((char)223);lcd.print("C ");
    }
    
    if(currentTemp<minTemp){
      lcd.setCursor(10,0);lcd.print("COOL ");
    }
    else{
      lcd.setCursor(10,0);lcd.print(currentTempAVG,1);lcd.print((char)223);lcd.print("C ");
    }   
  } 
}
