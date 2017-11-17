# MQTTCarPresense

This project utilizes a WeMos D1 mini Pro, u.FL to SMA antenna whip, and an external antenna in a 3D-printed enclosure to indicate presense when powered on and attached to MQTT.

![MQTTCarPresense](https://github.com/aderusha/MQTTCarPresense/blob/master/Images/MQTTCarPresense.jpg?raw=true)

[The included Arduino code](/MQTTCarPresence) is setup to work with [Home Assistant MQTT Discovery](https://home-assistant.io/docs/mqtt/discovery/) to automatically create a `binary_sensor` device which will indicate the connection status of the device.  The device should be powered from your car's ignition so it's only on when the car is running.  When the device powers on, it will attempt to connect to the defined MQTT broker, publishes a discovery message, and configures a Last Will and Testament.  When the device is powered off (or you drive out of WiFi range, the Last Will and Testament is sent by your broker.

[The included Home Assitant automation](MQTTCarPresence.yaml) example utilizes this information to open or close a connected garage door.

### Workflow
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