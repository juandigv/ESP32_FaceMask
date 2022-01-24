#include "BluetoothSerial.h"
#include <ArduinoJson.h>

#if !defined(CONFIG_BT_ENABLED) || !defined(CONFIG_BLUEDROID_ENABLED)
#error Bluetooth is not enabled! Please run `make menuconfig` to and enable it
#endif

BluetoothSerial SerialBT;

DynamicJsonDocument doc(1024);
DynamicJsonDocument altDoc(1024);
DynamicJsonDocument recievedDoc(1024);
JsonObject obj;
JsonObject altObj;
JsonObject recievedObj;

unsigned long currentTime;
unsigned long lastMessageTime;
String input;
String output;
int timeInterval = 300000; //300 seconds = 5 mins

void setup() {

  String defaultJson =
    "{\"ftemp\":0,\"fmic\":0,\"fresp\":0,\"val\":0,\"tresp\":0,\"ratio\":0,\"co2\":0,\"tvoc\":0}";

  String altJson =
    "{\"co2\":0,\"tvoc\":0,\"temp\":0}";

  deserializeJson(doc, defaultJson);
  obj = doc.as<JsonObject>();
  deserializeJson(altDoc, altJson);
  altObj = altDoc.as<JsonObject>();
  Serial.begin(115200);
  SerialBT.begin("ESP32-FaceMask"); //Bluetooth device name
  Serial.println("The device started, now you can pair it with bluetooth!");
  lastMessageTime = millis();
}

void loop() {
  currentTime = millis();
  if ((currentTime - lastMessageTime) > timeInterval) {
    sendSensorData(1);
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
  obj[String("ftemp")] = random(0, 46); //Se asume que se imprime la maxima
  obj[String("fmic")] = random(0, 30);  //Se asume que se imprime la maxima
  obj[String("fresp")] = random(0, 30);  //Frecuencia respoiratoria de fusion de Sensores
  obj[String("val")] = random(0,2);
  obj[String("tresp")] = random(0,3);
  obj[String("ratio")] = random(5, 100)/100.0; //Al no saber los valores de FrIn y FrEx se inventa un ratio temporalmente
  obj[String("co2")] = random(300, 600);
  obj[String("tvoc")] = random(50);
}

void generateNewDataAlt() {
  altObj[String("temp")] = random(20, 30);
  altObj[String("co2")] = random(300, 600);
  altObj[String("tvoc")] = random(50);
}

void sendSensorData(boolean typeJSON) {
  output = "";
  if(typeJSON){
    generateNewDataAlt();
    serializeJson(altDoc, output);
    Serial.println(output);
    SerialBT.println(output);
  }else{
    generateNewData();
    serializeJson(doc, output);
    Serial.println(output);
    SerialBT.println(output);
  }

}

void readBTChanges() {
  input = SerialBT.readString();
  deserializeJson(recievedDoc, input);
  recievedObj = recievedDoc.as<JsonObject>();
  if (recievedObj["timeInterval"] !=  nullptr) {
    Serial.println("Resetting Time Interval");
    timeInterval = recievedDoc["timeInterval"];
    Serial.println(timeInterval);
  }
  if (recievedObj["check"] !=  nullptr) {
    Serial.println("Sending Total Sensor Information");
    sendSensorData(0);
  }
  if (recievedObj["temp"] !=  nullptr) {
    Serial.println("Sending Temperature Information");
    sendSensorData(1);
  }
}
