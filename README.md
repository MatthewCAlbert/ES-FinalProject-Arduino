# Embedded System Final Project

## To Do
- [x] Create code for motor node + auto reconnect
- [x] Send command to motor node
- [x] Master Node auto reconnect
- [x] Master Node Time Sync
- [x] Create MQTT Server using VPS
- [x] MQTT Connection between Master Node and MQTT Broker
- [x] Create decision if raining or not at Master Node
- [x] Local Remote Web Control
- [ ] Get BMKG Weather from API
- [ ] Tune Motor Driver for Demo

## Optional
- [ ] Enable bluetooth control capability using MIT AppInventor 2
- [ ] Implement RF Remote using NRF24L01
- [ ] Enable SSL for WebSocket and WebServer HTTP

## Done
- [x] Sensor Node
- [x] Motor Node
- [x] Master/Fog Node (main function)

## Usage
- Use Platform.io with VSCode to built.
- **ES_Nodes** comprises of two projects (motor node and sensor node) so edit its platformio.ini and disable other corresponding **main**.