#include "BluetoothSerial.h"
#include <ArduinoJson.h>

#if !defined(CONFIG_BT_ENABLED) || !defined(CONFIG_BLUEDROID_ENABLED)
#error Bluetooth is not enabled! Please run `make menuconfig` to and enable it
#endif

BluetoothSerial SerialBT;

DynamicJsonDocument doc(1024);
DynamicJsonDocument recievedDoc(1024);
JsonObject obj;
JsonObject recievedObj;

unsigned long currentTime;
unsigned long lastMessageTime;
String input;
String output;
int timeInterval = 5000; //5 second

void setup() {

  String defaultJson =
    "{\"temperature\":0,\"co2\":0,\"tvoc\":0}";
  //Temperature in C
  //CO2 in PPM
  //TVOC in PPB

  deserializeJson(doc, defaultJson);
  obj = doc.as<JsonObject>();
  Serial.begin(115200);
  SerialBT.begin("ESP32-FaceMask"); //Bluetooth device name
  Serial.println("The device started, now you can pair it with bluetooth!");
  lastMessageTime = millis();
}

void loop() {
  currentTime = millis();
  if ((currentTime - lastMessageTime) > timeInterval) {
    sendSensorData();
    lastMessageTime = millis();
  }
  bluetoothDataSerial();
  delay(20);
}

void bluetoothDataSerial() {
  if (Serial.available()) {
    SerialBT.write(Serial.read());
  }
  if (SerialBT.available()) {
    //Serial.write(SerialBT.read());
    readBTChanges();
  }
}

void generateNewData() {
  obj[String("temperature")] = random(20, 30);
  obj[String("co2")] = random(300, 600);
  obj[String("tvoc")] = random(50);
}

void sendSensorData() {
  generateNewData();
  output = "";
  serializeJson(doc, output);
  Serial.println(output);
  SerialBT.println(output);
}

void readBTChanges() {
  input = SerialBT.readString();
  deserializeJson(recievedDoc, input);
  recievedObj = recievedDoc.as<JsonObject>();
  if (recievedObj["timeInterval"] !=  nullptr) {
    Serial.println("Resetting Time Interval");
    timeInterval = int(recievedObj["timeInterval"]);
  }
}
