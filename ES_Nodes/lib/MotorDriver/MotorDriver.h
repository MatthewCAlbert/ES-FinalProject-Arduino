#ifndef MOTORDRIVER
#define MOTORDRIVER
#include <Arduino.h>

class MotorDriver
{
public:
  static void initPin();
  static bool sendCommand(String command, bool *isOpen);
  static void setMotorRunning(bool in1, bool in2, bool in3, bool in4);
};
#endif // !MOTORDRIVER