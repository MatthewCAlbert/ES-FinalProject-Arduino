#include <ArduinoJson.h>

const size_t JSON_CAPACITY = 300;

class SensorStorage
{
public:
  void push(StaticJsonDocument<JSON_CAPACITY> data, String timestamp = "")
  {
    for (int i = 9; i > 0; i--)
    {
      storage[i] = storage[i - 1];
    }
    data["timemillis"] = millis();
    data["timestamp"] = timestamp;
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
  StaticJsonDocument<300 * 10> getConcatenatedData(String timestamp = "")
  {
    StaticJsonDocument<300 * 10> doc;
    doc["current_millis"] = millis();
    doc["current_timestamp"] = timestamp;
    JsonArray nested = doc.createNestedArray("data");

    for (int i = 0; i < 10; i++)
      nested.add(storage[i]);

    return doc;
  }
  String fetchAllSerializedJson(String timestamp = "")
  {
    String out = "";
    String outJson = "";
    StaticJsonDocument<300 * 10> doc = getConcatenatedData(timestamp);

    // for (int i = 0; i < 10; i++)
    // {
    //   out += fetchSerializedJson(i);
    //   if (i < 9)
    //     out += ",";
    // }
    // doc["data"] = "[" + out + "]";

    serializeJson(doc, outJson);

    return outJson;
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