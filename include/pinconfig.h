#ifndef USERCONFIG_H
#define USERCONFIG_H

#define PIR_PIN 0 //D3 10k pullup on D1mini
//#define PIR_PIN 2 //D4 10k pullup on D1mini, BUILTIN LED
//#define PIR_PIN 4 //D2 10k pulldown on D1mini
//#define PIR_PIN 12 //D6

const uint32_t TELNET_PORT_ = 2323;

const String JSON_TOPIC1 = "realraum/";
const String JSON_TOPIC3_MOVEMENT = "/movement";
const String JSON_TOPIC3_TEMP = "/temperature";
const String JSON_TOPIC3_PRESSURE = "/barometer";


#endif // USERCONFIG_H
