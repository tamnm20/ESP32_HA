#include <WiFi.h>
#include <PubSubClient.h>

uint32_t chipId = 0;
String mq_stt;
String mq_cmd;
String mq_sensors;
String device_on;
const char* ssid = "Free_Wifi";
const char* password = "bodeocho";

#define NUMBER_OF_WIFI 4
#define MAX_STRING_SIZE 16

char wifi_set[NUMBER_OF_WIFI][MAX_STRING_SIZE] =
{ "Xuong-2",
  "Wifi Free",
  "Free_Wifi",
  "Xuong_1"
};
char ip_pi[NUMBER_OF_WIFI][MAX_STRING_SIZE] =
{ "192.168.0.106",
  "192.168.0.113",
  "192.168.1.24",
  "192.168.0.106"
};

//#define MQTT_SERVER "192.168.0.106"
#define MQTT_PORT 1883
#define MQTT_USER "nmtam"
#define MQTT_PASSWORD "221220"
#define CONNECT_TOPIC "xuong/device/announce"

const char* MQTT_server;
const char* MQTT_TOPIC_stt;
const char* MQTT_TOPIC_cmd;
const char* MQTT_TOPIC_sensors;

WiFiClient wifiClient;
PubSubClient client(wifiClient);

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

void scanNetworks() {
  // scan for nearby networks:
  Serial.println("** Scan Networks **");
  byte numSsid = WiFi.scanNetworks();
  // print the list of networks seen:
  Serial.print("SSID List:");
  Serial.println(numSsid);
  // print the network number and name for each network found:
  for (int thisNet = 0; thisNet<numSsid; thisNet++) {
    Serial.print(thisNet);
    Serial.print(") Network: ");
    Serial.println(WiFi.SSID(thisNet));
  }
  Serial.print("Wifi dang ket noi: ");
  Serial.println(WiFi.SSID());
}

void autosetServer() {
  Serial.println("***Auto set Server***");
  int i = NUMBER_OF_WIFI;
  while (i--){
    if(WiFi.SSID() == String(wifi_set[i])){
       Serial.print("Wifi has saved: ");
       Serial.println(wifi_set[i]);
       MQTT_server = String(ip_pi[i]).c_str();
       Serial.print("Ip orangepi: ");
       Serial.println(ip_pi[i]);
       break;
    }
  }
  if (i<0) Serial.println("IP Not found");
  client.setServer(MQTT_server, MQTT_PORT);
}

void connect_to_broker() {
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    String clientId = "ESP32";
    clientId += String(random(0xffff), HEX);
    if (client.connect(clientId.c_str(), MQTT_USER, MQTT_PASSWORD)) {
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

void setup() {
  Serial.begin(115200);
  create_topic();
  setup_wifi();
  //setup_wifi_smart();
  scanNetworks();
  autosetServer();
  connect_to_broker();
}

void loop() {
  client.loop();
  if (!client.connected()) {
    connect_to_broker();
  }
}
