#include "Decision.h"

int Decision::fuzzyLogic(StaticJsonDocument<300> *_storage)
{

  float rainAvg15 = 0;
  float divider = 0;

  for (int i = 0; i < 5; i++)
  {
    if (_storage[i] != NULL)
    {
      rainAvg15 += _storage[i]["data"]["wetness"].as<float>();
      divider++;
    }
  }

  if (divider == 0 || rainAvg15 == 0)
    return FUZZ_STAY;

  float score = rainAvg15 / divider;

  if (score > 0.6)
  {
    // Big Rain
    return FUZZ_CLOSE_FORCE;
  }
  else if (score > 0.4)
  {
    // Small Rain
    return FUZZ_CLOSE;
  }
  else if (score > 0.1)
  {
    // Ignored
    return FUZZ_OPEN;
  }
  else if (score < 0.05)
  {
    return FUZZ_OPEN_FORCE;
  }

  return FUZZ_STAY;
}