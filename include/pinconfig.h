#ifndef USERCONFIG_H
#define USERCONFIG_H

//#define PIR_PIN 0 //D3 10k pullup on D1mini
//#define PIR_PIN 2 //D4 10k pullup on D1mini, BUILTIN LED
#define PIR_PIN 4 //D2 10k pulldown on D1mini

const uint32_t TELNET_PORT_ = 2323;

const String JSON_TOPIC1 = "realraum/";
const String JSON_TOPIC3_MOVEMENT = "/movement";


#endif // USERCONFIG_H
