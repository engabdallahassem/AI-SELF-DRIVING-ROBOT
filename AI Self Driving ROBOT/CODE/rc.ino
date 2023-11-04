#include "HUSKYLENS.h"


#include <Servo.h>

Servo myservo;  // create servo object to control a servo

#define IN2 6
#define IN1 7

#define ENA 5

#define Trig_Front 4
#define Echo_Front 3

#define Trig_Right 10
#define Echo_Right 9

#define Trig_Left 13
#define Echo_Left 12

long duration, distance, RightSensor, BackSensor, FrontSensor, LeftSensor;

HUSKYLENS huskylens;

const int ID0 = 0;  //not learned results. Grey result on HUSKYLENS screen
const int ID1 = 1;  //first learned results. colored result on HUSKYLENS screen
const int ID2 = 2;  //second learned results. colored result on HUSKYLENS screen

#define Red_Color_ID 2
#define Green_Color_ID 3
#define Red_Block_Width_Upper_Threshold 65
#define Red_Block_Width_Lower_Threshold 50
#define Green_Block_Width_Upper_Threshold 70
#define Green_Block_Width_Lower_Threshold 45

int currentColorID = 0;
int currentBlockWidth = 0;

#define Trig_Right 10


bool blueExist = false;
int blueCount = 0;
int lastBlueTime = 0;
int stopp = 0;

void printResult(HUSKYLENSResult result);

void setup() {
  Serial.begin(115200);

  myservo.attach(11);  // attaches the servo on pin 9 to the servo object

  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);

  pinMode(ENA, OUTPUT);

  pinMode(Trig_Front, OUTPUT);
  pinMode(Trig_Right, OUTPUT);
  pinMode(Trig_Left, OUTPUT);

  pinMode(Echo_Front, INPUT);
  pinMode(Echo_Right, INPUT);
  pinMode(Echo_Left, INPUT);

  Wire.begin();
  while (!huskylens.begin(Wire)) {
    Serial.println(F("Begin failed!"));
    Serial.println(F("1.Please recheck the \"Protocol Type\" in HUSKYLENS (General Settings>>Protocol Type>>I2C)"));
    Serial.println(F("2.Please recheck the connection."));
    delay(100);
  }
}

void loop() {

  delay(10);
  avoidance();

  if (!huskylens.request()) Serial.println(F("Fail to request data from HUSKYLENS, recheck the connection!"));
  else if (!huskylens.isLearned()) Serial.println(F("Nothing learned, press learn button on HUSKYLENS to learn one!"));
  else if (!huskylens.available()) {
    if (blueExist == true) {
      Serial.println(F("No Thing on screen"));
      blueExist = false;
    }

  } else {

    while (huskylens.available()) {
      HUSKYLENSResult result = huskylens.read();
      printResult(result);

      if (huskylens.count(ID1) == 0) {
        blueExist = false;
      } else if (blueExist == false && millis() - lastBlueTime > 1000) {
        lastBlueTime = millis();
        blueCount++;
        blueExist = true;
        if (blueCount == 12) stopp = 1;
        Serial.print("Blue Found, count is :");
        Serial.println(blueCount);
      }
    }
  }
}

void printResult(HUSKYLENSResult result) {
  if (result.command == COMMAND_RETURN_BLOCK) {  //result is a block
    currentColorID = result.ID;
    currentBlockWidth = result.width;
    Serial.println(String() + F("Block:xCenter=") + result.xCenter + F(",yCenter=") + result.yCenter + F(",width=") + result.width + F(",height=") + result.height + F(",ID=") + result.ID);
  } else if (result.command == COMMAND_RETURN_ARROW) {  //result is an arrow
    Serial.println(String() + F("Arrow:xOrigin=") + result.xOrigin + F(",yOrigin=") + result.yOrigin + F(",xTarget=") + result.xTarget + F(",yTarget=") + result.yTarget + F(",ID=") + result.ID);
  } else {  //result is unknown.
    Serial.println("Object unknown!");
  }
}
void resetBlockTurning() {

  currentBlockWidth = 0;
  currentColorID = 0;
}

void avoidance() {

  if (stopp == 1) {
    Stop();
    return;
  }
  SonarSensor(Trig_Front, Echo_Front);
  FrontSensor = distance;

  SonarSensor(Trig_Left, Echo_Left);
  LeftSensor = distance;

  SonarSensor(Trig_Right, Echo_Right);
  RightSensor = distance;

  Serial.print("Front Sensor: ");
  Serial.println(FrontSensor);
  Serial.print("Right Sensor: ");
  Serial.println(RightSensor);
  Serial.print("Left Sensor: ");
  Serial.println(LeftSensor);
  if (FrontSensor <= 45 ) {

    Backward();
    delay(450);
    if (RightSensor < LeftSensor) {
      Right();
      delay(300);

    } else {
      Left();
      delay(300);
    }
  } else if (RightSensor < = 50 || (currentColorID == Red_Color_ID && (currentBlockWidth != 0 && currentBlockWidth < Red_Block_Width_Upper_Threshold && currentBlockWidth > Red_Block_Width_Lower_Threshold))) {
    if ((currentColorID == Red_Color_ID && (currentBlockWidth != 0 && currentBlockWidth < Red_Block_Width_Upper_Threshold && currentBlockWidth > Red_Block_Width_Lower_Threshold))) {
      Serial.print("Gomaa Hack Red-Left: ");
      Serial.print("currentColorID: ");
      Serial.print(currentColorID);
      Serial.print(", currentBlockWidth : ");
      Serial.println(currentBlockWidth);
      resetBlockTurning();
    }
    Left();
    delay(300);

  }
  else if (LeftSensor <= 40 || (currentColorID == Green_Color_ID && (currentBlockWidth != 0 && currentBlockWidth < Green_Block_Width_Upper_Threshold && currentBlockWidth > Green_Block_Width_Lower_Threshold))) {
    if ((currentColorID == Green_Color_ID && (currentBlockWidth != 0 && currentBlockWidth < Green_Block_Width_Upper_Threshold && currentBlockWidth > Green_Block_Width_Lower_Threshold))) {
      Serial.print("Gomaa Hack Green-Right: ");
      Serial.print("currentColorID: ");
      Serial.print(currentColorID);
      Serial.print(", currentBlockWidth : ");
      Serial.println(currentBlockWidth);
      resetBlockTurning();
    }
    Right();
    delay(300);

  } else if (RightSensor >= 50) {
    Right();
    delay(300);
  } else {
    Forward();
    delay(300);
  } 

  delay(5);
}

void SonarSensor(int trigPin, int echoPin) {
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  duration = pulseIn(echoPin, HIGH);
  distance = duration / 58.2;
}

void Forward() {

  myservo.write(90);

  analogWrite(ENA, 200);
  digitalWrite(IN1, HIGH);
  digitalWrite(IN2, LOW);
}

void Backward() {

  myservo.write(90);

  analogWrite(ENA, 255);
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, HIGH);
}

void Right() {

  myservo.write(20);

  analogWrite(ENA, 255);
  digitalWrite(IN1, HIGH);
  digitalWrite(IN2, LOW);
}

void Left() {

  myservo.write(160);

  analogWrite(ENA, 255);
  digitalWrite(IN1, HIGH);
  digitalWrite(IN2, LOW);
}

void Stop() {

  analogWrite(ENA, 0);
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, LOW);
}
