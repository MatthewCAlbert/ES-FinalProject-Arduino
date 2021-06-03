#include <ArduinoJson.h>

const size_t JSON_CAPACITY = 300;

class SensorStorage
{
public:
  void push(StaticJsonDocument<JSON_CAPACITY> data)
  {
    for (int i = 9; i > 0; i--)
    {
      storage[i] = storage[i - 1];
    }
    storage[0] = data;
  }
  StaticJsonDocument<JSON_CAPACITY> *getStorage()
  {
    return storage;
  }
  String fetchSerializedJson(int index)
  {
    return serializeDefinedJson(storage[index < 0 || index > 9 ? 0 : index]);
  }
  String fetchAllSerializedJson()
  {
    String out = "";

    for (int i = 0; i < 10; i++)
    {
      if (i == 0)
      {
        out += "[";
      }
      out += fetchSerializedJson(i);
      if (i < 9)
        out += ",";
      else
        out += "]";
    }

    return out;
  }

private:
  String serializeDefinedJson(StaticJsonDocument<JSON_CAPACITY> doc)
  {
    String out = "";
    serializeJson(doc, out);
    return out;
  }
  StaticJsonDocument<JSON_CAPACITY> storage[10];
};