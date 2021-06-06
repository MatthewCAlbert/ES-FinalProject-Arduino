#include "MotorDriver.h"

static const int ledIndicatorPin = D8;

bool motorInProgress = false;
static int motorTiming = 3000;
static int en_a = D4;
static int in_1 = D1;
static int in_2 = D2;

static int en_b = D7;
static int in_3 = D5;
static int in_4 = D6;

void MotorDriver::initPin(int PWM)
{
  pinMode(en_a, OUTPUT);
  pinMode(in_1, OUTPUT);
  pinMode(in_2, OUTPUT);
  pinMode(en_b, OUTPUT);
  pinMode(in_3, OUTPUT);
  pinMode(in_4, OUTPUT);

  digitalWrite(in_1, LOW);
  digitalWrite(in_2, LOW);
  digitalWrite(in_3, LOW);
  digitalWrite(in_4, LOW);

  analogWrite(en_a, PWM);
  analogWrite(en_b, PWM);
}
bool MotorDriver::sendCommand(String command, bool *isOpen)
{
  if (motorInProgress)
  {
    Serial.println("Motor busy!");
    return false;
  }
  motorInProgress = true;
  Serial.println(*isOpen ? "OPEN" : "CLOSE");
  Serial.printf("Trying to %s\n", command.c_str());
  if (command == "close" && *isOpen)
  {
    Serial.println("\n[Motor CMD] Sending close command.");
    MotorDriver::setMotorRunning(1, 0, 1, 0);
    delay(motorTiming);
    MotorDriver::setMotorRunning(0, 0, 0, 0);
    Serial.println("\n[Motor CMD] Motor stopped.");
    *isOpen = false;
    digitalWrite(ledIndicatorPin, LOW);
  }
  else if (command == "open" && !*isOpen)
  {
    Serial.println("\n[Motor CMD] Sending open command.");
    MotorDriver::setMotorRunning(0, 1, 0, 1);
    delay(motorTiming);
    MotorDriver::setMotorRunning(0, 0, 0, 0);
    Serial.println("\n[Motor CMD] Motor stopped.");
    *isOpen = true;
    digitalWrite(ledIndicatorPin, HIGH);
  }
  else
  {
    Serial.println("\n[Motor CMD] Cannot do this action.");
  }
  motorInProgress = false;
}

void MotorDriver::setMotorRunning(bool in1, bool in2, bool in3, bool in4)
{
  digitalWrite(in_1, in1 ? HIGH : LOW);
  digitalWrite(in_2, in2 ? HIGH : LOW);
  digitalWrite(in_3, in3 ? HIGH : LOW);
  digitalWrite(in_4, in4 ? HIGH : LOW);
}