#include <Arduino.h>
#include <ArduinoJson.h>

#ifndef DECISION_CLASS

#define DECISION_CLASS
#define FUZZ_STAY 1;
#define FUZZ_OPEN 2;
#define FUZZ_CLOSE 3;
#define FUZZ_OPEN_FORCE 4;
#define FUZZ_CLOSE_FORCE 5;

#endif // !DECISION_CLASS

class Decision
{
public:
  static int fuzzyLogic(StaticJsonDocument<300> *_storage);
};