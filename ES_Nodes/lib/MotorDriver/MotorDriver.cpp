#include "MotorDriver.h"

static int motorTiming = 5000;
bool motorInProgress = false;
bool motorOpen = true;

bool MotorDriver::isOpen()
{
  return motorOpen;
}
void MotorDriver::initPin()
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

  analogWrite(en_a, 50);
  analogWrite(en_b, 50);
}
bool MotorDriver::sendCommand(String command)
{
  if (motorInProgress)
  {
    Serial.println("Motor busy!");
    return false;
  }
  motorInProgress = true;
  if (command == "close" && isOpen())
  {
    setMotorRunning(1, 0, 1, 0);
    delay(motorTiming);
    setMotorRunning(0, 0, 0, 0);
    motorOpen = true;
  }
  else if (command == "open" && !isOpen())
  {
    setMotorRunning(0, 1, 0, 1);
    delay(motorTiming);
    setMotorRunning(0, 0, 0, 0);
    motorOpen = false;
  }
  motorInProgress = false;
}

void MotorDriver::toggle()
{
  if (isOpen())
    sendCommand("close");
  else
    sendCommand("open");
}

void MotorDriver::directionControl()
{
  // Set motors to maximum speed
  // For PWM maximum possible values are 0 to 255
  analogWrite(en_a, 255);
  analogWrite(en_b, 255);

  // Turn on motor A & B
  setMotorRunning(1, 0, 1, 0);
  delay(2000);

  // Now change motor directions
  setMotorRunning(0, 1, 0, 1);
  delay(2000);

  // Turn off motors
  setMotorRunning(0, 0, 0, 0);
}

void MotorDriver::setMotorRunning(bool in1, bool in2, bool in3, bool in4)
{
  digitalWrite(in_1, in1 ? HIGH : LOW);
  digitalWrite(in_2, in2 ? HIGH : LOW);
  digitalWrite(in_3, in3 ? HIGH : LOW);
  digitalWrite(in_4, in4 ? HIGH : LOW);
}

void MotorDriver::speedControl()
{
  // Accelerate from zero to maximum speed
  for (int i = 0; i < 256; i++)
  {
    analogWrite(en_a, i);
    analogWrite(en_b, i);
    delay(20);
  }

  // Decelerate from maximum speed to zero
  for (int i = 255; i >= 0; --i)
  {
    analogWrite(en_a, i);
    analogWrite(en_b, i);
    delay(20);
  }
}