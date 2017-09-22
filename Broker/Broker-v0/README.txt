1. This folder has the code for broker, which can be run using Broker_Main.py
2. The other python scripts are invoked as subprocesses from Broker_Main.py:

Avahi Publish - ServiceAdvertise.py
Mqtt Broker -MqttBroker.py
HTTP Server- HttpServer.py
LWM2M Server,observer, client finder, HTTP PUT reader - serv.py
Visualisation - gui_new.py
User App- gui_1.py

3. The json files found in this folder were generated from the devices connected during plug fest.