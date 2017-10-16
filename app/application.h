#ifndef APPLICATION_H
#define APPLICATION_H

bool isBmp280Available();
void setupBmp280();
bool did_pir_trigger();
void wifiConnectOk();
void wifiConnectFail();
void connectToWifi();
void ready();
void init();
#endif