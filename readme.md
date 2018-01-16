# MQTTCarPresense

This project utilizes a [WeMos D1 mini Pro](https://wiki.wemos.cc/products:d1:d1_mini_pro), u.FL to SMA antenna whip, and an external antenna in a 3D-printed enclosure to indicate presense when powered on and attached to MQTT.

![MQTTCarPresense](https://github.com/aderusha/MQTTCarPresence/blob/master/Images/MQTTCarPresence.jpg?raw=true)

[The included Arduino code](/MQTTCarPresence) is setup to work with [Home Assistant MQTT Discovery](https://home-assistant.io/docs/mqtt/discovery/) to automatically create a `binary_sensor` device which will indicate the connection status of the device.  The device should be powered from your car's ignition so it's only on when the car is running.  When the device powers on, it will attempt to connect to the defined MQTT broker, publishes a discovery message, and configures a Last Will and Testament.  When the device is powered off (or you drive out of WiFi range), the Last Will and Testament is sent by your broker to update subscribed automation system(s).

[The included Home Assistant automation](MQTTCarPresence.yaml) example utilizes this information to open or close a connected garage door.

## Workflow - User perspective
* Car is in your garage and the garage door is closed
* You get in the car and turn it on, the garage door opens
* You drive away, the garage door closes a minute later
* When you arrive back home, the garage door opens as you approach
* When you turn off your car, the garage door closes about a minute later

## Detailed workflow
* Car is powered off, garage door is closed
* Car turns on, device publishes connection message to MQTT
* Home automation platform sees that the garage door is closed and the device is connected, then sends command to open garage
* You drive away when ready
* Device eventually drives out of range, MQTT broker sends disconnection message configured as the LWT
* Home automation platform sees the garage door is open and the device has disconnected, then sends command to close garage
* You go on about your day, then return home.
* Device connects to WiFi, then MQTT and publishes connection message
* Home automation platform sees that the garage door is closed and the device is connected, then sends command to open garage
* You park your car and turn it off
* Home automation platform sees the garage door is open and the device has disconnected, then sends command to close garage

## Bill of Materials
* [WeMos D1 mini Pro](https://wiki.wemos.cc/products:d1:d1_mini_pro)
* u.FL to SMA antenna whip
* External 2.4GHz WiFi antenna
* M3x8mm socket head cap screw (or similar)
* M3x4x6 threaded insert (or similar)

## Arduino programming notes
The first order of business will be to setup the Arduino IDE to flash the provided Arduino sketch to the ESP8266.  [Download the IDE for your platform](https://www.arduino.cc/en/Main/Software) and [follow these instructions to add support for the ESP8266 platform](https://github.com/esp8266/Arduino#installing-with-boards-manager).

Next you will need to add the PubSubClient library for MQTT.  [Follow this guide for the general process](https://www.arduino.cc/en/Guide/Libraries) and add the 'PubSubClient' from the Library Manager.  Once that is installed you will need to edit the `PubSubClient.h` file and change the line `#define MQTT_MAX_PACKET_SIZE 128` to `#define MQTT_MAX_PACKET_SIZE 512`.  You can find the installed library under the path shown in `File > Preferences > Sketchbook location`.

[At the top of the Arduino sketch are several fields you must modify to fit your environment (WiFi details, MQTT broker IP, node name, etc)](https://github.com/aderusha/MQTTCarPresence/blob/master/MQTTCarPresence/MQTTCarPresence.ino#L3-L10).  Once those fields have been set you can upload to your microcontroller and monitor sensor status in Home Assistant.

> ## WARNING:
> If you will be deploying more than one of these devices you **must** change the node names to be unique.  Failure to do so will result in a cascading series of MQTT connections/disconnections as your devices compete for access to your broker.
