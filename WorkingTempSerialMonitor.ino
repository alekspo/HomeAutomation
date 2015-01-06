#include <SoftwareSerial.h>

// Shield Information
// digital 9 = fan2
// digital 2 = RX LCD
// digital 3 = fan1
// digital 4 = button 4
// digital 5 = button 3
// digital 6 = button 2
// digital 7 = button 1
// analog  2 = Temp sensor
// file = read_arduino.sh
// dbfile = tempDatabase.db3
// table = tempData
#define fan1 3  // the one with PWM
#define fan2 9 
#define rxPin 2
#define nsamp 20
#define sensorPin 2
#define resetPin 4
#define fanup 6
#define fandown 7
//#define manual 8
String cmd;
bool cmdRec = false;
double coldtemp = 25;
double warmtemp = 26;
int manual = 0;
int fanspeed = 0;
boolean onoff = HIGH;
float mxt = 0;
float mnt = 100;
float thermValue = 0;
float temp = 0;
float maxT = 0;
float minT = 100;
long previousMillis = 0;       // will store last time temperature was updated  
const long interval = 2000;    // interval for updating the system

SoftwareSerial LCD = SoftwareSerial(3, rxPin);
// since the LCD does not send data back to the Arduino, we should only define the txPin
const int LCDdelay=10;  // conservative, 2 actually works

// wbp: goto with row & column
void lcdPosition(int row, int col) {
  LCD.write(0xFE);   //command flag
  LCD.write((col + row*64 + 128));    //position 
  delay(LCDdelay);
}
void clearLCD(){
  LCD.write(0xFE);   //command flag
  LCD.write(0x01);   //clear command.
  delay(LCDdelay);
}
void backlightOn() {  //turns on the backlight
  LCD.write(0x7C);   //command flag for backlight stuff
  LCD.write(157);    //light level.
  delay(LCDdelay);
}
void backlightOff(){  //turns off the backlight
  LCD.write(0x7C);   //command flag for backlight stuff
  LCD.write(128);     //light level for off.
   delay(LCDdelay);
}
void serCommand(){   //a general function to call the command flag for issuing all other commands   
  LCD.write(0xFE);
}
float avgTemp() {
  
  thermValue = 0;
  float temp = 0;
  float mVout = 0;
  float thermValueAvg = 0;
  float TempC = 0;
  
  for (byte j=0;j<nsamp;j++) {
    thermValue += analogRead(sensorPin);
    delay(10);
  }
  
  thermValueAvg = thermValue/nsamp;

  TempC = ((100.1*thermValueAvg)/1300);
  return TempC;
}

float maxTemp(float temp, float mxt) { 
  if(temp > mxt) {
    mxt = temp; 
  }
  return mxt;
}
float minTemp(float temp, float mnt) {
  if(temp < mnt) {
    mnt = temp; 
  }
  return mnt;
}

void sendTempToDB(float temp) {
  Serial.print(1);
  Serial.print(",");
  Serial.print(temp);
  Serial.println("");
}

void setup()
{
  pinMode(fan1, OUTPUT);
  pinMode(fan2, OUTPUT);
  pinMode(resetPin, INPUT);
  pinMode(fanup, INPUT);
  pinMode(fandown, INPUT);
  pinMode(manual, INPUT);
  //pinMode(fan1, OUTPUT);
  analogReference(INTERNAL);
  pinMode(rxPin, OUTPUT);
  LCD.begin(9600);
  clearLCD();
  lcdPosition(0,0);
  LCD.print("Welcome Aleks");
  Serial.print("Welcome Aleks");
  /*
  lcdPosition(1,0);
  LCD.print("Mx");
  lcdPosition(1, 8);
  LCD.print("Mn");
  */
  digitalWrite(fan2, LOW);
  Serial.begin(9600);
}

void reset(float maxT, float minT) {
    mxt = 0;
    mnt = 100;
    maxT = 0;
    minT = 100; 
}

void printout(float maxT, float minT, float temp, int fan) {
  clearLCD();
  lcdPosition(0,0);
  LCD.print("Temp:"); 
  lcdPosition(0,6);
  LCD.print(temp);
  lcdPosition(1,0);
  /*
  LCD.print("Fan:");
  lcdPosition(1, 4);
  if(fan < 10)
    LCD.print("OFF");
  else
    LCD.print("ON");
  lcdPosition(1, 9);
  if(manual == 1)
    LCD.print("manual");
  else
    LCD.print("auto");
    */
  //Serial.print("temp: ");
  //Serial.print(temp);
  //Serial.print("  fanSpeed: ");
  //Serial.println(fanspeed);
  
  /*
  lcdPosition(1,3);
  LCD.print(maxT);
  lcdPosition(1,11);
  LCD.print(minT);
 */ 
}

void fanControl(float temp) {
  int up = digitalRead(fanup);
  int down = digitalRead(fandown);
//  int man = digitalRead(manual);
  
  if(up == LOW && fanspeed < 255 && manual == 1) {
    fanspeed += 2;
    delay(60);
  }
  else if(down == LOW && fanspeed > 0 && manual == 1) {
    fanspeed -= 2;
    delay(60);
  }
  if(manual == 0) {
    
    // hvis det er kaldt
    if(temp <= coldtemp){
      //Serial.println("For kaldt");
      fanspeed = 0;
      digitalWrite(fan1, LOW);
      digitalWrite(fan2, LOW);
    // ellers når det er for varmt
    }else if(temp > warmtemp){
      //Serial.println("For varmt");
      digitalWrite(fan1, HIGH);
      digitalWrite(fan2, HIGH);
      fanspeed = 225;
    }
  } else if(manual == 1) {
      
  }
 //analogWrite(fan1, fanspeed);
}

void serialEvent() {//THIS SERIAL EVENT DONT WORK FOR SOFT SERIAL
    while(Serial.available() > 0 ) {
        char inByte = (char)Serial.read();
        //Serial.print("Recived: "); //Debug
        //Serial.println(inByte);    //Debug
        if(inByte == ':') {
            cmdRec = true;
            return;
        } else if(inByte == 'm') {
            cmd = "";
            cmdRec = false;
            return;
        } else {
            cmd += inByte;
            return;
        }
    }
}

void handleCmd() {
    if(!cmdRec) return;

    int data[4];
    int numArgs = 0;

    int beginIdx = 0;
    int idx = cmd.indexOf(",");

    String arg;
    char charBuffer[16];

    while (idx != -1) {
        arg = cmd.substring(beginIdx, idx);
        arg.toCharArray(charBuffer, 16);

        // add error handling for atoi:
        data[numArgs++] = atoi(charBuffer);
        beginIdx = idx + 1;
        idx = cmd.indexOf(",", beginIdx);
    }
    // And also fetch the last command
    arg = cmd.substring(beginIdx);
    arg.toCharArray(charBuffer, 16);
    data[numArgs++] = atoi(charBuffer);

    // We just want to switch a port so lets change the values
    if(data[0] < 100) {
        execCmd(data);
    } else {
        execCmds(data);
    }
    cmdRec = false;
}
//Serie komunikasjon funker på denne måten
// Startbit  :
// Stopbit   m
// deler av info ,
// Setter coldtemp   m1,"temp": 
// Setter warmtemp   m2,"temp":
// Setter auto       m101:
// Setter manuel     m102:
// Viser informasjon m103:
// Skrur på vifter   m104:
// Skrur av vifter   m105:
void execCmd(int* data) {
    //Serial.print("exeCmd: ");
    //Serial.println(data[0], data[1]);
    switch(data[0]) {
    case 1:
        coldtemp = data[1];
        //Serial.print("coldtemp: ");
        //Serial.println(coldtemp);
        break;
    case 2:
        warmtemp = data[1];
        //Serial.print("warmtemp: ");
        //Serial.println(warmtemp);
        break;
    }
}

void execCmds(int* data) {
  //Serial.print("exeCmds: ");
  //Serial.println(data[0]);
    switch(data[0]) {
    case 101:
        Serial.print("Going auto");
        manual = 0;
        break;

    case 102:
        Serial.print("Going manual");
        manual = 1;
        break;
    case 103:
        Serial.print("Temperature: ");
        Serial.println(temp);
        Serial.print("Cold temperature: ");
        Serial.println(coldtemp);
        Serial.print("Warm temperature: ");
        Serial.println(warmtemp);        
        break;
    case 104:
        Serial.println("Turning on fans");
        digitalWrite(fan1, HIGH);
        digitalWrite(fan2, HIGH);
        fanspeed = 255;
        break;
    case 105:
        Serial.println("Turning off fans");
        digitalWrite(fan1, LOW);
        digitalWrite(fan2, LOW);
        fanspeed = 0;
        break;
    }
}

//############## LOOP ####################
void loop() {
  
 unsigned long currentMillis = millis();
 //float temp;
 
 handleCmd();// Sjekker COM port for komandoer
  
 if(currentMillis - previousMillis > interval) {  
    temp = avgTemp();
    float maxT = maxTemp(temp, mxt);
    float minT = minTemp(temp, mnt); 
    printout(maxT, minT, temp, fanspeed);// printer ut resultatet
    fanControl(temp);
    sendTempToDB(temp);
    previousMillis = currentMillis;
    
 }

  if(digitalRead(resetPin) == 1) {
    reset(maxT, minT);
  }
  
  

    
  //delay(800);
  
}















