#include <WiFi.h>
#include <PubSubClient.h>
#include "DHT.h"
#include "ArduinoJson.h"

#define LED1 13
#define LED2 12
uint32_t chipId = 0;
unsigned long lastMsg = 0;
String mq_stt;
String mq_cmd;
String mq_sensors;
String device_on;

const char* ssid = "Xuong-2";
const char* password = "68686868";
//const char* ssid = "ELE Club PTIT";
//const char* password = "emyeuanhTrai";
//const char* ssid = "Free_Wifi";
//const char* password = "bodeocho";

#define MQTT_SERVER "192.168.0.128"

#define MQTT_PORT 1883
#define MQTT_USER "nmtam"
#define MQTT_PASSWORD "221220"
#define CONNECT_TOPIC "xuong/device/announce"
const char* MQTT_TOPIC_stt;
const char* MQTT_TOPIC_cmd;
const char* MQTT_TOPIC_sensors;

#define DHTPIN 4
#define DHTTYPE DHT22

DHT dht(DHTPIN, DHTTYPE);  
WiFiClient wifiClient;
PubSubClient client(wifiClient);

void publishData(float temperature, float humidity) {
  /*
     {
        "temperature": "23.20" ,
        "humidity": "43.70"
     }
  */
  
  char buffer[100] = {0};
  sprintf(buffer, "{\"temperature\": %0.2f, \"humidity\": %0.2f}", temperature, humidity);
  client.publish(MQTT_TOPIC_sensors, buffer, true);
  Serial.println(buffer);
}

void create_topic(){
  for(int i=0; i<17; i=i+8) {
    chipId |= ((ESP.getEfuseMac() >> (40 - i)) & 0xff) << i;
  }
  mq_stt = "xuong/device/esp" + String(chipId) + "/status";
  mq_cmd = "xuong/device/esp" + String(chipId) + "/cmd";
  mq_sensors = "xuong/device/esp" + String(chipId) + "/sensors";
  MQTT_TOPIC_stt = mq_stt.c_str();
  MQTT_TOPIC_cmd = mq_cmd.c_str();
  MQTT_TOPIC_sensors = mq_sensors.c_str();
  Serial.println(MQTT_TOPIC_stt);
  Serial.println(MQTT_TOPIC_cmd);
  Serial.println(MQTT_TOPIC_sensors);
}

void setup_wifi() {
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  device_on = "esp"+ String(chipId) +" is online";
  Serial.println(device_on);
}

void setup_wifi_smart() {
  //Init WiFi as Station, start SmartConfig
  WiFi.mode(WIFI_AP_STA);
  WiFi.beginSmartConfig();
  //Wait for SmartConfig packet from mobile
  Serial.println("Waiting for SmartConfig.");
  while (!WiFi.smartConfigDone()) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("SmartConfig received.");
  //Wait for WiFi to connect to AP
  Serial.println("Waiting for WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("WiFi Connected.");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());
  device_on = "esp"+ String(chipId) +" is online";
  Serial.println(device_on);
}
 
void connect_to_broker() {
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    String clientId = "ESP32";
    clientId += String(random(0xffff), HEX);
    if (client.connect(clientId.c_str(), MQTT_USER, MQTT_PASSWORD)) {
    //if (client.connect(clientId.c_str())) {
      Serial.println("connected");
      client.subscribe(MQTT_TOPIC_cmd);
      client.publish(CONNECT_TOPIC, device_on.c_str());
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 2 seconds");
      delay(2000);
    }
  }
}
 
void callback(char* topic, byte *payload, unsigned int length) {  
  char status_[length+1];
  Serial.println("-------new message from broker-----");
  Serial.print("topic: ");
  Serial.println(topic);
  Serial.print("message: ");
  memcpy(status_, payload, length);
  status_[length] = NULL;
  String message(status_);
  Serial.println();
  Serial.println(message);
  if(String(topic) == MQTT_TOPIC_cmd)
  {
    if(message == "OFF1")
    {
      digitalWrite(LED1, LOW);
      Serial.println("LED1 OFF");
      client.publish(MQTT_TOPIC_stt, "OFF1");
    }
    if(message == "ON1")
    {
      digitalWrite(LED1, HIGH);
      Serial.println("LED1 ON");
      client.publish(MQTT_TOPIC_stt, "ON1");
    }
    if(message == "OFF2")
    {
      digitalWrite(LED2, LOW);
      Serial.println("LED2 OFF");
      client.publish(MQTT_TOPIC_stt, "OFF2");
    }
    if(message == "ON2")
    {
      digitalWrite(LED2, HIGH);
      Serial.println("LED2 ON");
      client.publish(MQTT_TOPIC_stt, "ON2");
    }
  }  
}
void setup() {
  Serial.begin(115200);
  dht.begin();
  create_topic();
  Serial.setTimeout(500);
  setup_wifi();
//  setup_wifi_smart();
  client.setServer(MQTT_SERVER, MQTT_PORT);
  client.setCallback(callback);
  connect_to_broker();
  Serial.println("Start transfer");
  pinMode(LED1, OUTPUT);
  pinMode(LED2, OUTPUT);
}

void loop() {
  client.loop();
  if (!client.connected()) {
    connect_to_broker();
  }
  unsigned long now = millis();
  if (now - lastMsg > 3000)
  {
    lastMsg = now;
    float h = dht.readHumidity();
    float t = dht.readTemperature();
    if (isnan(h) || isnan(t)) {
      Serial.println("ERROR: Failed to read from DHT sensor!");
      return;
    } else {
      Serial.print(F("Humidity: "));
      Serial.print(h);
      Serial.print(F("%  Temperature: "));
      Serial.print(t);
      Serial.println(F("°C"));
      publishData(t, h);
    }
  }
}
