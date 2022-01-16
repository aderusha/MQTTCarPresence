# MQTTCarPresense

This project utilizes a [WeMos D1 mini Pro](https://wiki.wemos.cc/products:d1:d1_mini_pro), u.FL to SMA antenna whip, and an external antenna in a 3D-printed enclosure to indicate presense when powered on and attached to MQTT.

![MQTTCarPresense](https://github.com/aderusha/MQTTCarPresence/blob/master/Images/MQTTCarPresence.jpg?raw=true)

[The included Arduino code](/MQTTCarPresence) is setup to work with [Home Assistant MQTT Discovery](https://home-assistant.io/docs/mqtt/discovery/) to automatically create a `binary_sensor` device which will indicate the connection status of the device.  The device should be powered from your car's ignition so it's only on when the car is running.  When the device powers on, it will attempt to connect to the defined MQTT broker, publishes a discovery message, and configures a Last Will and Testament.  When the device is powered off (or you drive out of WiFi range), the Last Will and Testament is sent by your broker to update subscribed automation system(s).

[The included Home Assistant blueprint automation](MQTTCarPresence.yaml) example utilizes this information to open or close a connected garage door. Just copy the file url into the blueprint import menu and create an automation using the blueprint!

[![Open your Home Assistant instance and show the blueprint import dialog with a specific blueprint pre-filled.](https://my.home-assistant.io/badges/blueprint_import.svg)](https://my.home-assistant.io/redirect/blueprint_import/?blueprint_url=https%3A%2F%2Fraw.githubusercontent.com%2Faderusha%2FMQTTCarPresence%2Fmaster%2FMQTTCarPresence.yaml)

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
* Device eventually drives out of range, dropping connection with the broker
* MQTT broker sends disconnection message configured as the LWT
* Home automation platform sees the garage door is open and the device has disconnected, then sends command to close garage
* You go on about your day, then return home.
* Device connects to WiFi, then MQTT and publishes connection message
* Home automation platform sees that the garage door is closed and the device is connected, then sends command to open garage
* You park your car and turn it off, dropping connection with the broker
* MQTT broker sends disconnection message configured as the LWT
* Home automation platform sees the garage door is open and the device has disconnected, then sends command to close garage

## Bill of Materials

* [WeMos D1 mini Pro](https://wiki.wemos.cc/products:d1:d1_mini_pro)
* u.FL to SMA antenna whip
* External 2.4GHz WiFi antenna
* M3x8mm socket head cap screw (or similar)
* M3x4x6 threaded insert (or similar)

## Arduino programming

The first order of business will be to setup the Arduino IDE to flash the provided Arduino sketch to the ESP8266.  [Download the IDE for your platform](https://www.arduino.cc/en/Main/Software) and [follow these instructions to add support for the ESP8266 platform](https://github.com/esp8266/Arduino#installing-with-boards-manager).

Next you will need to add the PubSubClient library for MQTT.  [Follow this guide for the general process](https://www.arduino.cc/en/Guide/Libraries) and add the 'PubSubClient' from the Library Manager.

[At the top of the Arduino sketch are several fields you must modify to fit your environment](https://github.com/aderusha/MQTTCarPresence/blob/master/MQTTCarPresence/MQTTCarPresence.ino#L3-L10) (WiFi details, MQTT broker IP, node name, etc).  Once those fields have been set you can upload to your microcontroller and monitor sensor status in Home Assistant.

> ## WARNING:
> 
> If you will be deploying more than one of these devices you **must** change the node names to be unique.  Failure to do so will result in a cascading series of MQTT connections/disconnections as your devices compete for access to your broker.

## WeMos D1 Mini Pro Antenna

In order to utilize an external antenna on the WeMos D1 Mini Pro, you will need to move a 0 ohm 0603 SMT resistor.  See [this blog post](http://raspi.tv/2017/how-to-use-external-antenna-on-wemos-d1-mini-pro-surface-mount-rework-video) along with [an accompanying video](https://www.youtube.com/watch?v=3lHoG1mu7hY&t=72) on how to make that happen.  A hot-air station will make this easier, but I was able to do this with a normal pencil iron.

## Tasmota Firmware Variant

If you are more familiar to Tasmota flashing and usage rather than Arduino Programming and building, this is another variant that you could use.  It will replace the **Arduino Programming** section above, but follows the guidelines of just about everything else.  The blueprint that is attached to this project named MQTT_Tasmota_CarPresence.yaml can be used as an alternate along with flashing the Di-Mini with Tasmota to accomplish the same thing as the original.  

This Tasmota Variant is set-up to work with [Home Assistant Tasmota Integration](https://www.home-assistant.io/integrations/tasmota/) to automatically create a `switch` which will indicate the connection status of the Car Presence Device.  It should show up as a switch with a name that matches the FriendlyName set-up in the Tasmota Web Console.

####    Follow these steps to flash the D1 Mini if using Tasmota: ####

* Flash the D1 Mini to the latest Tasmota version.
* Set up your WIFI, MQTT, Hostname, Friendly name, and all the normal stuff.  Details on the Tasmota homepage.
* Set the Device module to type `generic(18)`.
* Under configure module, 
    * set GPIO2 as LEDi 1
    * set GPIO4 as Button 1
    * set GPIO5 as Relay 1
* On the console, paste this to set-up the mcu functions as required for proper operation (Check Tasmota website for details on what these are):  
    * ```Backlog  SO19 0; SO56 1; SO63 1; SO65 1; WifiConfig 5```
* Go to Configure Other and be sure to set a Web Admin Password in case it goes into AP mode when you are away from Home.
* Set-up the blueprint `MQTT_Tasmota_CarPresence.yaml` similar to the instructions for the other blueprint above.

[![Open your Home Assistant instance and show the blueprint import dialog with a specific blueprint pre-filled.](https://my.home-assistant.io/badges/blueprint_import.svg)](https://my.home-assistant.io/redirect/blueprint_import/?blueprint_url=https%3A%2F%2Fgithub.com%2Faderusha%2FMQTTCarPresence%2Fblob%2Fmaster%2FMQTT_Tasmota_CarPresence.yaml)

## Buy me a coffee

[![Buy me a coffee](https://www.buymeacoffee.com/assets/img/custom_images/black_img.png)](https://www.buymeacoffee.com/gW5rPpsKR)

This project is powered by coffee.  [I might get a little weird about it at times](https://github.com/aderusha/RoastLearner), but it's not much of a stretch to suggest that coffee both powers and consumes a fair portion of my mental energy.  [Hook me up if you think HASP is cool](https://www.buymeacoffee.com/gW5rPpsKR).  Thanks!
