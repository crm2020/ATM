#include <WiFi.h>
#include <WebServer.h>

// WiFi Credentials
const char* ssid = "Ché's S24 Ultra";
const char* password = "Niggers123456789";

// Create a WebServer object
WebServer server(80);

// A1 MOTOR
#define ENABLE_A1 22
#define A1_PIN_ONE 15
#define A1_PIN_TWO 2

// A2 MOTOR
#define ENABLE_A2 17
#define A2_PIN_ONE 16
#define A2_PIN_TWO 4

// DEFAULT MOTOR SPEED
#define DEFAULT_SPEED 255

// IR SENSOR LEFT
#define IR_SENSOR 35

// IR SENSOR RIGHT
#define IR_SENSOR2 34

// PING SENSOR GROUND
#define TRIG1 33
#define ECHO1 32

// PING SENSOR FRONT
#define TRIG2 26
#define ECHO2 27

// REED SENSOR
#define REED 14

// MOTOR CONTAINER MOVER UP-DOWN
#define ENABLE_CM 22
#define CM_PIN_ONE 23
#define CM_PIN_TWO 21

// MOTOR CONTAINER TIGHTEN-LOOSEN
#define ENABLE_CT 5
#define CT_PIN_ONE 18
#define CT_PIN_TWO 19

// varibles ping sensor ground
long Gduration;
int ground;

// caribles ping sensor front
long Fduration;
int front; 

String motorStatus = "Stopped";

// Function Prototypes
void driveForward();
void driveBackward();
void turnLeft();
void turnRight();
void turnMotorOff();
void setMotorSpeed(int motor_speed);
void groundSensor();
void frontSensor();

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

  // Initialize WiFi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to WiFi");
  Serial.print("IP Address: ");     //http://192.168.88.183/ esp32 ip
  Serial.println(WiFi.localIP());

  // Define WebServer routes
  server.on("/", HTTP_GET, []() {
    server.send(200, "text/html", R"rawliteral(
//     <!DOCTYPE html>
//     <html>
//       <head>
//         <title>Car Control</title>
//       </head>
//       <body>
//         <h1>Car Control</h1>
//         <button onclick="fetch('/control?action=stop')">Stop Car</button>
//         <button onclick="fetch('/control?action=start')">Resume Car</button>
//         <button onclick="fetch('/control?action=left')">Turn Car Left</button>
//         <button onclick="fetch('/control?action=right')">Turn Car Right</button>
//       </body>
//     </html>
//   )rawliteral");
  });


// http://<ESP32_IP>/control?action=
  // // Web Server Handlers
// void handleRoot() {
//   server.send(200, "text/html", R"rawliteral(
//     <!DOCTYPE html>
//     <html>
//       <head>
//         <title>Car Control</title>
//       </head>
//       <body>
//         <h1>Car Control</h1>
//         <button onclick="fetch('/stop')">Stop Car</button>
//         <button onclick="fetch('/resume')">Resume Car</button>
//       </body>
//     </html>
//   )rawliteral");
// }

  server.on("/control", HTTP_GET, []() {
    if (server.hasArg("action")) {
      String action = server.arg("action");
      if (action == "start") {
        driveForward();
        motorStatus = "Driving Forward";
      } else if (action == "stop") {
        turnMotorOff();
        motorStatus = "Stopped";
      } else if (action == "left") {
        turnLeft();
        motorStatus = "Turning Left";
      } else if (action == "right") {
        turnRight();
        motorStatus = "Turning Right";
      } else {
        motorStatus = "Unknown Command";
      }
      server.send(200, "text/plain", "Action: " + action + " | Status: " + motorStatus);
    } else {
      server.send(400, "text/plain", "No action specified.");
    }
  });

  // Start the server
  server.begin();

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
  pinMode(REED, INPUT);git
}

void loop() {
  // Handle incoming web requests
  server.handleClient();

  // int infraredState = digitalRead(IR_SENSOR);
  // int infraredState2 = digitalRead(IR_SENSOR2);
  // int reedState = digitalRead(REED);

  // // Check the state of the first sensor
  // switch (infraredState) {
  //   case HIGH:
  //     // Check the state of the second sensor only if the first sensor is HIGH
  //     switch (infraredState2) {
  //       case HIGH:
  //         switch (reedState) {
  //           case HIGH:
  //             Serial.println("no magnet detected");
  //           break;
  //           case LOW:
  //             turnMotorOff();
  //             Serial.println("magnet detected");
  //         }
  //         driveForward();  // Both sensors HIGH - drive forward
  //         Serial.println("Both sensors HIGH - Driving Forward");
  //         break;
  //       case LOW:
  //         turnLeft();  // First sensor HIGH, second sensor LOW - turn left
  //         Serial.println("First sensor HIGH, second sensor LOW - Turning Left");
  //         break;
  //     }
  //   break;
  //   case LOW:
  //     turnRight();  // First sensor LOW - turn right
  //     Serial.println("First sensor LOW - Turning Right");
  //     break;
  // }

  // groundSensor();
  // if(ground >= 5) {
  //   driveBackward();
  //   Serial.println(" - going back");
  // } else {
  //   Serial.println("Ground detected");
  // }

  // frontSensor();
  // if(front <= 10){
  //   turnLeft();
  //   Serial.println(" - Turning Left");
  // } else {
  //   Serial.println("No obect detected");
  // }

}

// Drives forward
void driveForward() {
  digitalWrite(A1_PIN_ONE, HIGH);
  digitalWrite(A1_PIN_TWO, LOW);
  digitalWrite(A2_PIN_ONE, LOW);
  digitalWrite(A2_PIN_TWO, HIGH);
}

// Drives backward
void driveBackward() {
  digitalWrite(A1_PIN_ONE, LOW);
  digitalWrite(A1_PIN_TWO, LOW);
  digitalWrite(A2_PIN_ONE, HIGH);
  digitalWrite(A2_PIN_TWO, LOW);
  Serial. println("backwards");
}


// Makes a left turn
void turnLeft() {
  digitalWrite(A1_PIN_ONE, HIGH);
  digitalWrite(A1_PIN_TWO, LOW);
  digitalWrite(A2_PIN_ONE, HIGH);
  digitalWrite(A2_PIN_TWO, LOW);
}

// Makes a right turn
void turnRight() {
  digitalWrite(A1_PIN_ONE, LOW);
  digitalWrite(A1_PIN_TWO, HIGH);
  digitalWrite(A2_PIN_ONE, LOW);
  digitalWrite(A2_PIN_TWO, HIGH);
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
  Gduration = pulseIn(ECHO1, HIGH);
  ground = Gduration * 0.0344 / 2;
}

void frontSensor() {
  digitalWrite(TRIG2, LOW);
  delayMicroseconds(2);  // Reduced delay
  digitalWrite(TRIG2, HIGH);
  delayMicroseconds(5);  // Increased delay for better triggering
  digitalWrite(TRIG2, LOW);
  Fduration = pulseIn(ECHO2, HIGH);
  front = Fduration * 0.0344 / 2;
}