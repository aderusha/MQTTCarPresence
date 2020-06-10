////////////////////////////////////////////////////////////////////////////////////////////////////
// Modify these values for your environment
const char* wifiSSID = "4254GAR";  // Your WiFi network name
const char* wifiPassword = "6129206467";  // Your WiFi network password
const char* otaPassword = "dfDF34#$";  // OTA update password
const char* mqttServer = "192.168.1.187";  // Your MQTT server IP address
const char* mqttUser = ""; // mqtt username, set to "" for no user
const char* mqttPassword = ""; // mqtt password, set to "" for no password
const String mqttNode = "AudiQ7"; // Your unique hostname for this device
const String mqttDiscoveryPrefix = "homeassistant"; // Home Assistant MQTT Discovery, see https://home-assistant.io/docs/mqtt/discovery/
////////////////////////////////////////////////////////////////////////////////////////////////////

// Home Assistant MQTT Discovery, see https://home-assistant.io/docs/mqtt/discovery/
// We'll create one binary_sensor device to track MQTT connectivity
const String mqttDiscoBinaryStateTopic = mqttDiscoveryPrefix + "/binary_sensor/" + mqttNode + "/state";
const String mqttDiscoBinaryConfigTopic = mqttDiscoveryPrefix + "/binary_sensor/" + mqttNode + "/config";
// And a sensor for WiFi signal strength
const String mqttDiscoSignalStateTopic = mqttDiscoveryPrefix + "/sensor/" + mqttNode + "-signal/state";
const String mqttDiscoSignalConfigTopic = mqttDiscoveryPrefix + "/sensor/" + mqttNode + "-signal/config";
// And a sensor for device uptime
const String mqttDiscoUptimeStateTopic = mqttDiscoveryPrefix + "/sensor/" + mqttNode + "-uptime/state";
const String mqttDiscoUptimeConfigTopic = mqttDiscoveryPrefix + "/sensor/" + mqttNode + "-uptime/config";

// The strings below will spill over the PubSubClient_MAX_PACKET_SIZE 128
// You'll need to manually set MQTT_MAX_PACKET_SIZE in PubSubClient.h to 512
const String mqttDiscoBinaryConfigPayload = "{\"name\": \"" + mqttNode + "\", \"device_class\": \"connectivity\", \"state_topic\": \"" + mqttDiscoBinaryStateTopic + "\"}";
const String mqttDiscoSignalConfigPayload = "{\"name\": \"" + mqttNode + "-signal\", \"state_topic\": \"" + mqttDiscoSignalStateTopic + "\", \"unit_of_measurement\": \"dBm\", \"value_template\": \"{{ value }}\"}";
const String mqttDiscoUptimeConfigPayload = "{\"name\": \"" + mqttNode + "-uptime\", \"state_topic\": \"" + mqttDiscoUptimeStateTopic + "\", \"unit_of_measurement\": \"msec\", \"value_template\": \"{{ value }}\"}";

// Set the signal strength and uptime reporting interval in milliseconds
const unsigned long reportInterval = 5000;
unsigned long reportTimer = millis();

// Set LED "twinkle" time for maximum daylight visibility
const unsigned long twinkleInterval = 50;
unsigned long twinkleTimer = millis();

#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
#include <PubSubClient.h>

WiFiClient wifiClient;
PubSubClient mqttClient(wifiClient);

////////////////////////////////////////////////////////////////////////////////////////////////////
// System setup
void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, HIGH);
  Serial.begin(115200);

  Serial.println("\nHardware initialized, starting program load");

  // Start up networking
  setupWifi();

  // Create server and assign callbacks for MQTT
  mqttClient.setServer(mqttServer, 1883);
  mqttClient.setCallback(mqtt_callback);

  // Start up OTA
  if (otaPassword[0]) {
    setupOTA();
  }

  Serial.println("Initialization complete\n");
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// Main execution loop
void loop() {
  // check WiFi connection
  if (WiFi.status() != WL_CONNECTED) {
    setupWifi();
  }

  // check MQTT connection
  if (!mqttClient.connected()) {
    mqttConnect();
  }

  // MQTT client loop
  if (mqttClient.connected()) {
    mqttClient.loop();
  }

  // LED twinkle
  if (mqttClient.connected() && ((millis() - twinkleTimer) >= twinkleInterval)) {
    digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
    twinkleTimer = millis();
  }

  // Report signal strength and uptime
  if (mqttClient.connected() && ((millis() - reportTimer) >= reportInterval)) {
    String signalStrength = String(WiFi.RSSI());
    String uptimeTimer = String(millis());
    mqttClient.publish(mqttDiscoSignalStateTopic.c_str(), signalStrength.c_str());
    mqttClient.publish(mqttDiscoUptimeStateTopic.c_str(), uptimeTimer.c_str());
    reportTimer = millis();
  }

  // OTA loop
  if (otaPassword[0]) {
    ArduinoOTA.handle();
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// Functions

////////////////////////////////////////////////////////////////////////////////////////////////////
// Handle incoming commands from MQTT
void mqtt_callback(char* topic, byte* payload, unsigned int payloadLength) {
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// Connect to WiFi
void setupWifi() {
  Serial.print("Connecting to WiFi network: " + String(wifiSSID));
  WiFi.hostname(mqttNode.c_str());
  WiFi.mode(WIFI_STA);
  WiFi.begin(wifiSSID, wifiPassword);

  while (WiFi.status() != WL_CONNECTED) {
    // Wait 500msec seconds before retrying
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi connected successfully and assigned IP: " + WiFi.localIP().toString());
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// MQTT connection and subscriptions
void mqttConnect() {
  digitalWrite(LED_BUILTIN, HIGH);
  Serial.println("Attempting MQTT connection to broker: " + String(mqttServer));
  // Attempt to connect to broker, setting last will and testament
  if (mqttClient.connect(mqttNode.c_str(), mqttUser, mqttPassword, mqttDiscoBinaryStateTopic.c_str(), 1, 1, "OFF")) {
    // when connected, record signal strength and reset reporting timer
    String signalStrength = String(WiFi.RSSI());
    reportTimer = millis();
    String uptimeTimer = String(millis());
    // publish MQTT discovery topics and device state
    Serial.println("MQTT discovery connectivity config: [" + mqttDiscoBinaryConfigTopic + "] : [" + mqttDiscoBinaryConfigPayload + "]");
    Serial.println("MQTT discovery connectivity state: [" + mqttDiscoBinaryStateTopic + "] : [ON]");
    Serial.println("MQTT discovery signal config: [" + mqttDiscoSignalConfigTopic + "] : [" + mqttDiscoSignalConfigPayload + "]");
    Serial.println("MQTT discovery signal state: [" + mqttDiscoSignalStateTopic + "] : " + WiFi.RSSI());
    Serial.println("MQTT discovery uptime config: [" + mqttDiscoUptimeConfigTopic + "] : [" + mqttDiscoUptimeConfigPayload + "]");
    Serial.println("MQTT discovery uptime state: [" + mqttDiscoUptimeStateTopic + "] : " + uptimeTimer);
    mqttClient.publish(mqttDiscoUptimeConfigTopic.c_str(), mqttDiscoUptimeConfigPayload.c_str(), true);
    mqttClient.publish(mqttDiscoUptimeStateTopic.c_str(), uptimeTimer.c_str());
    mqttClient.publish(mqttDiscoBinaryConfigTopic.c_str(), mqttDiscoBinaryConfigPayload.c_str(), true);
    mqttClient.publish(mqttDiscoBinaryStateTopic.c_str(), "ON");
    mqttClient.publish(mqttDiscoSignalConfigTopic.c_str(), mqttDiscoSignalConfigPayload.c_str(), true);
    mqttClient.publish(mqttDiscoSignalStateTopic.c_str(), signalStrength.c_str());
    Serial.println("MQTT connected");
    digitalWrite(LED_BUILTIN, LOW);
  }
  else {
    Serial.println("MQTT connection failed, rc=" + String(mqttClient.state()));
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// (mostly) boilerplate OTA setup from library examples
void setupOTA() {
  // Start up OTA
  // ArduinoOTA.setPort(8266); // Port defaults to 8266
  ArduinoOTA.setHostname(mqttNode.c_str());
  ArduinoOTA.setPassword(otaPassword);

  ArduinoOTA.onStart([]() {
    Serial.println("ESP OTA:  update start");
  });
  ArduinoOTA.onEnd([]() {
    Serial.println("ESP OTA:  update complete");
  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    //Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
  });
  ArduinoOTA.onError([](ota_error_t error) {
    Serial.println("ESP OTA:  ERROR code " + String(error));
    if (error == OTA_AUTH_ERROR) Serial.println("ESP OTA:  ERROR - Auth Failed");
    else if (error == OTA_BEGIN_ERROR) Serial.println("ESP OTA:  ERROR - Begin Failed");
    else if (error == OTA_CONNECT_ERROR) Serial.println("ESP OTA:  ERROR - Connect Failed");
    else if (error == OTA_RECEIVE_ERROR) Serial.println("ESP OTA:  ERROR - Receive Failed");
    else if (error == OTA_END_ERROR) Serial.println("ESP OTA:  ERROR - End Failed");
  });
  ArduinoOTA.begin();
  Serial.println("ESP OTA:  Over the Air firmware update ready");
}
