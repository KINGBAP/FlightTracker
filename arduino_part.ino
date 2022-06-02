#include <SoftwareSerial.h>
#include <LiquidCrystal_I2C.h>
#include <Servo.h>
String input;
String output;
const byte numChars = 1000;
char receivedChars[numChars];
boolean newData = false;

const byte numChars2 = 100;
char receivedChars2[numChars2];
boolean newData2 = false;



Servo myservo;  // create servo object to control a servo
// twelve servo objects can be created on most boards

int pos = 0;    // variable to store the servo position

LiquidCrystal_I2C lcd(0x27,16,2);  // set the LCD address to 0x3F for a 16 chars and 2 line display

String payload;


SoftwareSerial ESP_Serial(6, 7); //Tx,Rx

void setup() {
  
  myservo.attach(9); 
   myservo.write(pos);

  lcd.init();
  lcd.clear();         
  lcd.backlight();      // Make sure backlight is on
  
  // Print a message on both lines of the LCD.
  lcd.setCursor(5,0);   //Set cursor to character 2 on line 0
  lcd.print("Flight");
  
  lcd.setCursor(0,1);   //Move cursor to character 2 on line 1
  lcd.print("Tracker by  Bap");
  
  Serial.begin(115200);

  ESP_Serial.begin(9600);
//  Serial.println("Waiting for you");
//  input=Serial.readString();
//  Serial.println(input);
//  ESP_Serial.println(input);

  
  


}


void loop() {
  //Serial.println(ESP_Serial.available());
  recvWithEndMarker();
  showNewData();
  userRcv();
  sendInput();
  
//    for (pos = 0; pos <= 180; pos += 1) { // goes from 0 degrees to 180 degrees
//    // in steps of 1 degree
//    myservo.write(pos);              // tell servo to go to position in variable 'pos'
//    delay(15);                       // waits 15ms for the servo to reach the position
//  }
//  for (pos = 180; pos >= 0; pos -= 1) { // goes from 180 degrees to 0 degrees
//    myservo.write(pos);              // tell servo to go to position in variable 'pos'
//    delay(15);                       // waits 15ms for the servo to reach the position
//  }
//  }
}

void recvWithEndMarker() {
 static byte ndx = 0;
 char endMarker = '\n';
 char rc;
 
 // if (Serial.available() > 0) {
           while (ESP_Serial.available() > 0 && newData == false) {
 rc = ESP_Serial.read();

 if (rc != endMarker) {
 receivedChars[ndx] = rc;
 ndx++;
 if (ndx >= numChars) {
 ndx = numChars - 1;
 }
 }
 else {
 receivedChars[ndx] = '\0'; // terminate the string
 ndx = 0;
 input=receivedChars;
 newData = true;
 }
 }
}

void showNewData() {
 if (newData == true) {
Serial.println(receivedChars);
String pourcent = getValue(input, ',', 1);
int pourcenta = pourcent.toInt();
String dep = getValue(input, ',', 2);
String arr = getValue(input, ',', 3);
String speedH =getValue(input, ',', 4);
String alt = getValue(input, ',', 5);
String fN = getValue(input, ',', 6);
String city = getValue(input,',',7);
pos = (pourcenta*180)/100;
  if (input[0]=='G') {
    if (pos >= 0 && pos <= 180) {
      Serial.println(pos);
       myservo.write(pos);
    }
    

    lcd.clear();
    lcd.setCursor(2,0);   //Set cursor to character 2 on line 0
    lcd.print("Flight Number");
    lcd.setCursor(4,1);
    lcd.print(fN);
    delay(1000);
    lcd.clear();
    lcd.setCursor(2,0);   //Set cursor to character 2 on line 0
    lcd.print("Speed");
    lcd.setCursor(2,1);
    lcd.print(speedH);
    lcd.clear();
    lcd.setCursor(3,0);   //Set cursor to character 2 on line 0
    lcd.print("From ");
    lcd.print(dep);
    lcd.setCursor(5,1);
    lcd.print("To ");
    lcd.print(arr);
    delay(1000);
    lcd.clear();
    lcd.setCursor(2,0);   //Set cursor to character 2 on line 0
    lcd.print("Speed");
    lcd.setCursor(2,1);
    lcd.print(speedH);
    delay(1000);
    lcd.clear();
    lcd.setCursor(0,0);   //Set cursor to character 2 on line 0
    lcd.print("Your city");
    lcd.setCursor(2,1);
    lcd.print(city);
    delay(1000);
    lcd.clear();
    lcd.setCursor(3,0);   //Set cursor to character 2 on line 0
    lcd.print(pourcent);
    lcd.print(" % ");
    lcd.setCursor(3,1);
    lcd.print("remaining");
    delay(1000);
    
    
 }
 if (input[0]=='1' || input[0]=='2') {
  lcd.clear();
    lcd.setCursor(2,0);   //Set cursor to character 2 on line 0
    lcd.print("HTML ERROR");
 }
 newData = false;
 }
}

void userRcv() {
 static byte ndx2 = 0;
 char endMarker2 = '\n';
 char rc2;
 
 // if (Serial.available() > 0) {
           while (Serial.available() > 0 && newData2 == false) {
 rc2 = Serial.read();

 if (rc2 != endMarker2) {
 receivedChars2[ndx2] = rc2;
 ndx2++;
 if (ndx2 >= numChars2) {
 ndx2 = numChars2 - 1;
 }
 }
 else {
 receivedChars2[ndx2] = '\0'; // terminate the string
 ndx2 = 0;
 newData2 = true;
 }
 }
}

void sendInput() {
 if (newData2 == true) {
  output = receivedChars2;
 Serial.print("Vous avez écrit ");
 Serial.println(output);
 ESP_Serial.println(output);
 newData2 = false;
 }
}

String getValue(String data, char separator, int index)
{
    int found = 0;
    int strIndex[] = { 0, -1 };
    int maxIndex = data.length() - 1;

    for (int i = 0; i <= maxIndex && found <= index; i++) {
        if (data.charAt(i) == separator || i == maxIndex) {
            found++;
            strIndex[0] = strIndex[1] + 1;
            strIndex[1] = (i == maxIndex) ? i+1 : i;
        }
    }
    return found > index ? data.substring(strIndex[0], strIndex[1]) : "";
}
