#include <Arduino.h>

const int en_a = D4;
const int in_1 = D1;
const int in_2 = D2;

const int en_b = D7;
const int in_3 = D5;
const int in_4 = D6;

class MotorDriver
{
public:
  static void initPin();
  static bool isOpen();
  static bool sendCommand(String command);
  static void setMotorRunning(bool in1, bool in2, bool in3, bool in4);
  static void toggle();
  static void directionControl();
  static void speedControl();
};