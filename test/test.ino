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

// IR SENSORS
#define IR_SENSOR_LEFT 13
#define IR_SENSOR_RIGHT 14

// ULTRASONIC SENSORS
#define TRIG_GROUND 26
#define ECHO_GROUND 27
#define TRIG_FRONT 22
#define ECHO_FRONT 23

// REED SENSOR
#define REED_SENSOR 21

/* Wi-Fi Credentials */
const char* ssid = "Internet!!";  // Enter SSID here
const char* password = "123456789";  // Enter Password here

/* Static IP Configuration */
IPAddress local_ip(192, 168, 1, 1);
IPAddress gateway(192, 168, 1, 1);
IPAddress subnet(255, 255, 255, 0);

WebServer server(80);
bool stopCar = false;

// Variables for sensor distances
int groundDistance = 0;
int frontDistance = 0;

/* MOTOR LAYOUT
LEFT <--> RIGHT

    A1 --- A2
        |
        |
        W
*/

void setup() {
  // Serial Monitor
  Serial.begin(9600);

  // Initialize motor pins
  pinMode(ENABLE_A1, OUTPUT);
  pinMode(A1_PIN_ONE, OUTPUT);
  pinMode(A1_PIN_TWO, OUTPUT);

  pinMode(ENABLE_A2, OUTPUT);
  pinMode(A2_PIN_ONE, OUTPUT);
  pinMode(A2_PIN_TWO, OUTPUT);

  // Initialize sensor pins
  pinMode(IR_SENSOR_LEFT, INPUT);
  pinMode(IR_SENSOR_RIGHT, INPUT);

  pinMode(TRIG_GROUND, OUTPUT);
  pinMode(ECHO_GROUND, INPUT);
  pinMode(TRIG_FRONT, OUTPUT);
  pinMode(ECHO_FRONT, INPUT);

  pinMode(REED_SENSOR, INPUT);

  // Initialize motor speed (ensure PWM setup is configured correctly)
  ledcSetup(0, 5000, 8); // Channel 0, 5kHz frequency, 8-bit resolution
  ledcAttachPin(ENABLE_A1, 0);
  ledcAttachPin(ENABLE_A2, 0);
  setMotorSpeed(DEFAULT_SPEED);

  // Connect to Wi-Fi
  int retryCount = 0;
  while (!WiFi.softAP(ssid, password) && retryCount < 5) {
    Serial.println("Retrying Wi-Fi setup...");
    delay(1000);
    retryCount++;
  }
  if (retryCount == 5) {
    Serial.println("Wi-Fi setup failed. Restarting...");
    ESP.restart();
  }

  // Web Server Setup
  server.on("/", handleRoot);
  server.on("/stop", handleStopCar);
  server.on("/resume", handleResumeCar);
  server.begin();
  Serial.println("HTTP server started");
}

void loop() {
  server.handleClient();

  if (stopCar) {
    turnMotorOff();
    Serial.println("Car stopped via Web Server");
    return;
  }

  // Handle sensors and motor movement
  groundDistance = measureDistance(TRIG_GROUND, ECHO_GROUND);
  frontDistance = measureDistance(TRIG_FRONT, ECHO_FRONT);

  int leftIR = digitalRead(IR_SENSOR_LEFT);
  int rightIR = digitalRead(IR_SENSOR_RIGHT);
  int reedState = digitalRead(REED_SENSOR);

  // Reed Sensor Handling
  if (reedState == HIGH) {
    turnMotorOff();
    Serial.println("Magnet detected - Stopping the car");
    return;
  }

  // Infrared Sensors Handling
  if (leftIR == LOW && rightIR == LOW) {
    driveForward();
    Serial.println("Driving Forward");
  } else if (leftIR == LOW && rightIR == HIGH) {
    turnLeft();
    Serial.println("Turning Left");
  } else if (leftIR == HIGH && rightIR == LOW) {
    turnRight();
    Serial.println("Turning Right");
  }

  // Ground Sensor Handling
  if (groundDistance >= 8) {
    driveBackward();
    Serial.println("Obstacle below - Driving Backward");
  }

  // Front Sensor Handling
  if (frontDistance <= 10 && frontDistance > 0) {
    turnLeft();
    Serial.println("Obstacle ahead - Turning Left");
  }
}

// Measure distance using ultrasonic sensors
int measureDistance(int trigPin, int echoPin) {
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  long duration = pulseIn(echoPin, HIGH, 30000); // 30ms timeout
  if (duration == 0) {
    Serial.println("Sensor timeout");
    return -1;
  }

  return duration * 0.0344 / 2; // Convert to cm
}

// Motor Control Functions
void driveForward() {
  digitalWrite(A1_PIN_ONE, HIGH);
  digitalWrite(A1_PIN_TWO, LOW);
  digitalWrite(A2_PIN_ONE, LOW);
  digitalWrite(A2_PIN_TWO, HIGH);
}

void driveBackward() {
  digitalWrite(A1_PIN_ONE, LOW);
  digitalWrite(A1_PIN_TWO, HIGH);
  digitalWrite(A2_PIN_ONE, HIGH);
  digitalWrite(A2_PIN_TWO, LOW);
}

void turnLeft() {
  digitalWrite(A1_PIN_ONE, HIGH);
  digitalWrite(A1_PIN_TWO, LOW);
  digitalWrite(A2_PIN_ONE, HIGH);
  digitalWrite(A2_PIN_TWO, LOW);
}

void turnRight() {
  digitalWrite(A1_PIN_ONE, LOW);
  digitalWrite(A1_PIN_TWO, HIGH);
  digitalWrite(A2_PIN_ONE, LOW);
  digitalWrite(A2_PIN_TWO, HIGH);
}

void turnMotorOff() {
  digitalWrite(A1_PIN_ONE, LOW);
  digitalWrite(A1_PIN_TWO, LOW);
  digitalWrite(A2_PIN_ONE, LOW);
  digitalWrite(A2_PIN_TWO, LOW);
}

void setMotorSpeed(int motorSpeed) {
  ledcWrite(0, motorSpeed);
}

// Web Server Handlers
void handleRoot() {
  server.send(200, "text/html", R"rawliteral(
    <!DOCTYPE html>
    <html>
      <head>
        <title>Car Control</title>
      </head>
      <body>
        <h1>Car Control</h1>
        <button onclick="fetch('/stop')">Stop Car</button>
        <button onclick="fetch('/resume')">Resume Car</button>
      </body>
    </html>
  )rawliteral");
}

void handleStopCar() {
  stopCar = true;
  server.send(200, "text/plain", "Car Stopped");
}

void handleResumeCar() {
  stopCar = false;
  server.send(200, "text/plain", "Car Resumed");
}
