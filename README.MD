# TEAM 7

Welcome to Team 7 of the Women Who Code Hackathon in Portland Oregon, 2019!

## Project Name

### Human Comfort System

Dynamically respond to atmospheric gas composition changes by issuing "Open" and "Close" servo commands to air vents both locally on a host ESP8266 as well as securely over the internet utilizing the Azure Sphere IoT infrastructure tool-chain.

## Team Members

- Amelia Wietting (Aask)
- Kate LaFrance
- Xuehong Liu
- Allison Walker
- Claire Cassidy

## Summary

We built a hybrid IoT device utilizing the flexibilty of the ESP8266 (and the ESP12-S) for its abundant support for many different sensors and acuators, as well as the security of the Azure Sphere hardware and software stack

This infrastructure stack is set up to demonstrate the detection of and response to a change in atmospheric gas composition in the vents which deliver the Oxygen humans need to survive

## Things we think are important to know

- We are using a 2-way communication channel over UART to read and repsond to explicit instructions across the Arduino and Azure sphere devices dynamically
- Captive Portal is used for accessing ESP8266 locally and serves interface automatically upon connect by a local device
- The Azure tool chain is fully configured to the point where we can dynamically pull data from the device in the field locally for review and other data driven tasks
- Dynamic web-app that can respond to and display data from the AzSphere IoT
- Half of our team left by mid-day on Saturday
- The solder I provided was of the Radio Shack Brand
