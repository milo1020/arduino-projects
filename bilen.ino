#include <Servo.h>

enum Direction {
  DIR_FORWARD = 8,
  DIR_RIGHT = 6,
  DIR_LEFT = 4,
  DIR_BACK = 2
};

// Motor pins
const int pinLB = 6;   // Left reverse / IN1
const int pinLF = 9;   // Left forward / IN2
const int pinRB = 10;  // Right reverse / IN3
const int pinRF = 11;  // Right forward / IN4

// Ultrasonic pins
const int inputPin = A0;   // Echo
const int outputPin = A1;  // Trigger

// Servo pins and angles
const int servoPin = 5;
const int servoCenter = 90;
const int servoLeft = 150;
const int servoRight = 20;

// Servo settling delay
const int servoDelayMs = 250;

// PWM tuning values
// These can be adjusted until the car drives straight and turns correctly.
const int leftForwardSpeed = 180;
const int rightForwardSpeed = 180;
const int leftReverseSpeed = 180;
const int rightReverseSpeed = 180;

// Small motor correction bias: positive = more power to that side
const int leftBias = 0;
const int rightBias = 10;

Servo myservo;

int Fspeedd = 0;
int Rspeedd = 0;
int Lspeedd = 0;
int directionn = DIR_FORWARD;

void setMotor(int forwardPin, int reversePin, int speed) {
  if (speed > 0) {
    analogWrite(forwardPin, speed);
    digitalWrite(reversePin, LOW);
  } else if (speed < 0) {
    analogWrite(reversePin, abs(speed));
    digitalWrite(forwardPin, LOW);
  } else {
    digitalWrite(forwardPin, LOW);
    digitalWrite(reversePin, LOW);
  }
}

void setLeftMotor(int speed) {
  setMotor(pinLF, pinLB, speed);
}

void setRightMotor(int speed) {
  setMotor(pinRF, pinRB, speed);
}

void stopMotors(int ms) {
  setLeftMotor(0);
  setRightMotor(0);
  delay(ms);
}

void advance(int ms) {
  setLeftMotor(leftForwardSpeed + leftBias);
  setRightMotor(rightForwardSpeed + rightBias);
  delay(ms);
}

void back(int ms) {
  setLeftMotor(-(leftReverseSpeed + leftBias));
  setRightMotor(-(rightReverseSpeed + rightBias));
  delay(ms);
}

void turnR(int ms) {
  setLeftMotor(leftForwardSpeed + leftBias);
  setRightMotor(0);
  delay(ms);
}

void turnL(int ms) {
  setLeftMotor(0);
  setRightMotor(rightForwardSpeed + rightBias);
  delay(ms);
}

void setup() {
  Serial.begin(9600);

  pinMode(pinLB, OUTPUT);
  pinMode(pinLF, OUTPUT);
  pinMode(pinRB, OUTPUT);
  pinMode(pinRF, OUTPUT);

  pinMode(inputPin, INPUT);
  pinMode(outputPin, OUTPUT);

  myservo.attach(servoPin);
  myservo.write(servoCenter);
  delay(500);
}

float readDistance() {
  digitalWrite(outputPin, LOW);
  delayMicroseconds(2);

  digitalWrite(outputPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(outputPin, LOW);

  unsigned long duration = pulseIn(inputPin, HIGH, 25000);
  if (duration == 0) {
    return 400.0;
  }

  float distance = duration / 58.0;
  if (distance <= 0.0 || distance > 400.0) {
    return 400.0;
  }

  // Distance in cm
  return distance;
}

void ask_pin_F() {
  myservo.write(servoCenter);
  delay(servoDelayMs);

  float distance = readDistance();
  Fspeedd = (int)distance;
  Serial.print("F distance: ");
  Serial.println(Fspeedd);
}

void ask_pin_L() {
  myservo.write(servoLeft);
  delay(servoDelayMs);

  float distance = readDistance();
  Lspeedd = (int)distance;
  Serial.print("L distance: ");
  Serial.println(Lspeedd);
}

void ask_pin_R() {
  myservo.write(servoRight);
  delay(servoDelayMs);

  float distance = readDistance();
  Rspeedd = (int)distance;
  Serial.print("R distance: ");
  Serial.println(Rspeedd);
}

void detection() {
  // Always center the sensor before evaluating forward distance.
  myservo.write(servoCenter);
  delay(200);
  ask_pin_F();

  if (Fspeedd < 10) {
    stopMotors(150);
    back(250);
    directionn = DIR_BACK;
    return;
  }

  if (Fspeedd < 25) {
    stopMotors(150);

    // Scan left and right while servo is moved to each side.
    myservo.write(servoLeft);
    delay(200);
    ask_pin_L();

    myservo.write(servoCenter);
    delay(200);

    myservo.write(servoRight);
    delay(200);
    ask_pin_R();

    myservo.write(servoCenter);
    delay(200);

    if (Lspeedd < 10 && Rspeedd < 10) {
      directionn = DIR_BACK;
    } else if (Lspeedd > Rspeedd) {
      directionn = DIR_LEFT;
    } else {
      directionn = DIR_RIGHT;
    }

    return;
  }

  directionn = DIR_FORWARD;
}

void loop() {
  myservo.write(servoCenter);
  delay(200);

  detection();

  switch (directionn) {
    case DIR_BACK:
      back(250);
      turnL(350);  // reverse then turn left
      Serial.println("Reverse");
      break;

    case DIR_RIGHT:
      back(150);
      turnR(450);
      Serial.println("Right");
      break;

    case DIR_LEFT:
      back(150);
      turnL(450);
      Serial.println("Left");
      break;

    case DIR_FORWARD:
    default:
      advance(180);
      Serial.println("Advance");
      break;
  }
}
