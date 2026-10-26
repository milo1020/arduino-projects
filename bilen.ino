#include <Servo.h>

enum Direction {
  DIR_FORWARD = 8,
  DIR_RIGHT = 6,
  DIR_LEFT = 4,
  DIR_BACK = 2
};

const int pinLB = 6;   // Left back -> IN1
const int pinLF = 9;   // Left forward -> IN2
const int pinRB = 10;  // Right back -> IN3
const int pinRF = 11;  // Right forward -> IN4

const int inputPin = A0;   // Ultrasonic echo
const int outputPin = A1;  // Ultrasonic trigger

const int servoPin = 5;
const int servoCenter = 90;
const int servoLeft = 150;
const int servoRight = 20;

const int servoDelayMs = 250;

Servo myservo;

int Fspeedd = 0;
int Rspeedd = 0;
int Lspeedd = 0;
int directionn = DIR_FORWARD;

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

void advance(int ms) {
  digitalWrite(pinRB, LOW);
  digitalWrite(pinRF, HIGH);
  digitalWrite(pinLB, LOW);
  digitalWrite(pinLF, HIGH);
  delay(ms);
}

void turnR(int ms) {
  digitalWrite(pinRB, LOW);
  digitalWrite(pinRF, HIGH);
  digitalWrite(pinLB, HIGH);
  digitalWrite(pinLF, LOW);
  delay(ms);
}

void turnL(int ms) {
  digitalWrite(pinRB, HIGH);
  digitalWrite(pinRF, LOW);
  digitalWrite(pinLB, LOW);
  digitalWrite(pinLF, HIGH);
  delay(ms);
}

void stopp(int ms) {
  digitalWrite(pinRB, HIGH);
  digitalWrite(pinRF, HIGH);
  digitalWrite(pinLB, HIGH);
  digitalWrite(pinLF, HIGH);
  delay(ms);
}

void back(int ms) {
  digitalWrite(pinRB, HIGH);
  digitalWrite(pinRF, LOW);
  digitalWrite(pinLB, HIGH);
  digitalWrite(pinLF, LOW);
  delay(ms);
}

float readDistance() {
  digitalWrite(outputPin, LOW);
  delayMicroseconds(2);

  digitalWrite(outputPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(outputPin, LOW);

  unsigned long duration = pulseIn(inputPin, HIGH, 25000);

  if (duration == 0) {
    return 400.0;  // if no echo, treat as far away
  }

  // Convert time to distance in cm
  return (duration / 58.0);
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
  ask_pin_F();

  if (Fspeedd < 10) {
    stopp(150);
    back(300);
    directionn = DIR_BACK;
    return;
  }

  if (Fspeedd < 25) {
    stopp(150);

    ask_pin_L();
    delay(servoDelayMs);
    ask_pin_R();
    delay(servoDelayMs);

    // Choose the side with more free space
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

  detection();

  switch (directionn) {
    case DIR_BACK:
      back(250);
      turnL(350);   // reverse then turn left
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
