// Header file
#include <WiFi.h>
#include <WebServer.h>

// A1 MOTOR
#define ENABLE_A1 17
#define A1_PIN_ONE 16
#define A1_PIN_TWO 4

// A2 MOTOR
#define ENABLE_A2 32
#define A2_PIN_ONE 33
#define A2_PIN_TWO 25

// DEFAULT MOTOR SPEED
#define DEFAULT_SPEED 255

// IR SENSOR LEFT
#define IR_SENSOR 13

// IR SENSOR RIGHT
#define IR_SENSOR2 14

// PING SENSOR GROUND
#define TRIG1 26
#define ECHO1 27

// PING SENSOR FRONT
#define TRIG2 22
#define ECHO2 23

// REED SENSOR
#define REED 21

/* Put your SSID & Password */
const char* ssid = "tesla iot";  // Enter SSID here
const char* password = "fsL6HgjN";  //Enter Password here

/* Put IP Address details */
IPAddress local_ip(192,168,1,1);
IPAddress gateway(192,168,1,1);
IPAddress subnet(255,255,255,0);

WebServer server(80);

/* 
MOTOR LAYOUT
LEFT <--> RIGHT

    A1 --- A2
        |
        |
    A3 --- A4    
*/
void setup() {
  // Serial monitor
  Serial.begin(9600);

  // Left DC motor
  pinMode(ENABLE_A1, OUTPUT);
  pinMode(A1_PIN_ONE, OUTPUT);
  pinMode(A1_PIN_TWO, OUTPUT);

  // Right DC motor
  pinMode(ENABLE_A2, OUTPUT);
  pinMode(A2_PIN_ONE, OUTPUT);
  pinMode(A2_PIN_TWO, OUTPUT);

  // IR Sensor
  pinMode(IR_SENSOR, INPUT);

  // IR Sensor
  pinMode(IR_SENSOR2, INPUT);

  // Set motor to LOW
  digitalWrite(A1_PIN_ONE, LOW);
  digitalWrite(A1_PIN_TWO, LOW);
  digitalWrite(A2_PIN_ONE, LOW);
  digitalWrite(A2_PIN_TWO, LOW);

  // Set motor speed
  setMotorSpeed(DEFAULT_SPEED);

  // ping sensor 1 ground
  pinMode(TRIG1, OUTPUT);
  pinMode(ECHO1, INPUT);

  // ping sensor 2 front 
  pinMode(TRIG2, OUTPUT);
  pinMode(ECHO2, INPUT);

  // Reed sensor
  pinMode(REED, INPUT);

  // wifi
  WiFi.softAP(ssid, password);
  WiFi.softAPConfig(local_ip, gateway, subnet);
  delay(100);
}

// varibles ping sensor ground
long duration;
int ground;

// caribles ping sensor front
long time;
int front; 


void loop() {
  // Check the ground condition
  groundSensor();

  int infraredState = digitalRead(IR_SENSOR);
  int infraredState2 = digitalRead(IR_SENSOR2);
  int reedState = digitalRead(REED);

  // Switch case for detecting the magnets and to shut down the ACM
  switch (reedState) {
    case LOW:
      Serial.println("No magnet detected");
      break;
    case HIGH:
      turnMotorOff();
      Serial.println("Magnet detected - Stopping the car");
      return;  // Exit the loop to prevent further actions when the ACM is stopped
  }

  // Check the state of the first sensor
  switch (infraredState) {
    case LOW:
      driveForward();
      // Check the state of the second sensor only if the first sensor is HIGH
      switch (infraredState2) {
        case LOW:
          driveForward();  // Both sensors HIGH - drive forward
          Serial.println("Both sensors HIGH - Driving Forward");
          break;
        case HIGH:
          turnLeft();  // First sensor HIGH, second sensor LOW - turn left
          Serial.println("First sensor HIGH, second sensor LOW - Turning Left");
          break;
      }
      break;
    case HIGH:
      turnRight();  // First sensor LOW - turn right
      Serial.println("First sensor LOW - Turning Right");
      Serial.print(infraredState);
      Serial.print(infraredState2);
      break;
  }

  // Ground senor
  while(ground >= 8) {
    groundSensor();
    driveBackward();
    Serial.println(" - Going back");
    Serial.println(ground);
  }
  
  // front sensor
  frontSensor();
  if(front <= 10){
    turnLeft();
    Serial.println(" - Turning Left");
    Serial.println(front);
  } else {
    Serial.println("No obect detected");
  }
  
}

// Drives forward
void driveForward() {
  digitalWrite(A1_PIN_ONE, LOW);
  digitalWrite(A1_PIN_TWO, HIGH);
  digitalWrite(A2_PIN_ONE, HIGH);
  digitalWrite(A2_PIN_TWO, LOW);
}

// Drives backward
void driveBackward() {
  digitalWrite(A1_PIN_ONE, HIGH);
  digitalWrite(A1_PIN_TWO, LOW);
  digitalWrite(A2_PIN_ONE, LOW);
  digitalWrite(A2_PIN_TWO, HIGH);
  Serial. println("backwards");
}


// Makes a left turn
void turnRight() {
  digitalWrite(A1_PIN_ONE, LOW);
  digitalWrite(A1_PIN_TWO, HIGH);
  digitalWrite(A2_PIN_ONE, LOW);
  digitalWrite(A2_PIN_TWO, HIGH);
}

// Makes a right turn
void turnLeft() {
  digitalWrite(A1_PIN_ONE, HIGH);
  digitalWrite(A1_PIN_TWO, LOW);
  digitalWrite(A2_PIN_ONE, HIGH);
  digitalWrite(A2_PIN_TWO, LOW);
}

// Sets motor speeds
void setMotorSpeed(int motor_speed) {
  analogWrite(ENABLE_A1, motor_speed);
  analogWrite(ENABLE_A2, motor_speed);
}

// Turns all motors off
void turnMotorOff() {
  digitalWrite(A1_PIN_ONE, LOW);
  digitalWrite(A1_PIN_TWO, LOW);
  digitalWrite(A2_PIN_ONE, LOW);
  digitalWrite(A1_PIN_TWO, LOW);
}

// Turns all motors on
void turnMotorOn() {
  digitalWrite(A1_PIN_ONE, HIGH);
  digitalWrite(A1_PIN_TWO, HIGH);
  digitalWrite(A2_PIN_ONE, HIGH);
  digitalWrite(A2_PIN_TWO, HIGH);
}

void groundSensor() {
  digitalWrite(TRIG1, LOW);
  delayMicroseconds(2);  // Reduced delay
  digitalWrite(TRIG1, HIGH);
  delayMicroseconds(5);  // Increased delay for better triggering
  digitalWrite(TRIG1, LOW);
  duration = pulseIn(ECHO1, HIGH);
  ground = duration * 0.0344 / 2;
}

void frontSensor() {
  digitalWrite(TRIG2, LOW);
  delayMicroseconds(2);  // Reduced delay
  digitalWrite(TRIG2, HIGH);
  delayMicroseconds(5);  // Increased delay for better triggering
  digitalWrite(TRIG2, LOW);
  time = pulseIn(ECHO2, HIGH);
  front = time * 0.0344 / 2;
}