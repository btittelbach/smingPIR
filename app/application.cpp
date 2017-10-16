#include <user_config.h>
#include <SmingCore/SmingCore.h>
#include <Adafruit_BMP280/Adafruit_BMP280.h>
#include <spiffsconfig.h>
#include <pinconfig.h>
#include <SmingCore/Debug.h>
#include "application.h"
#include "telnet.h"
#include "mqtt.h"
#ifdef ENABLE_SSL
	#include <ssl/private_key.h>
	#include <ssl/cert.h>
#endif

NetConfigStorage NetConfig;

uint32_t pid_event_ctr_ = 0;
uint32_t last_read_pid_event_ctr_ = 0;
uint32_t last_pid_event_time_ = 0;

Adafruit_BMP280 bmp = Adafruit_BMP280(); // I2C
bool bmp280_found_ = false;

///////////////////////////////////////
///// BMP280
///////////////////////////////////////

bool isBmp280Available()
{
	return bmp280_found_;
}

void setupBmp280() {
	// if (bmp.begin(0x77)) //Adafruit Sensors
	if (bmp.begin(0x76)) //AliExpress BMP280 Sensors
	{
		bmp280_found_ = true;  
		debugf("BMP280 found");
	} else {
		bmp280_found_ = false;
		debugf("BMP280 not found");
	}
}

///////////////////////////////////////
///// Interrupt
///////////////////////////////////////

void IRAM_ATTR interruptHandler()
{
	if (digitalRead(PIR_PIN))
		last_pid_event_time_ = millis();
	else {
		//debugf("pulsedur: %dms",millis() - last_pid_event_time_);
		if (millis() - last_pid_event_time_ > NetConfig.debounce_interval) {
			pid_event_ctr_++;
		}
	}
}

bool did_pir_trigger()
{
	if (pid_event_ctr_ == last_read_pid_event_ctr_)
		return false;
	debugf("movem %d",pid_event_ctr_ - last_read_pid_event_ctr_);
	last_read_pid_event_ctr_ = pid_event_ctr_;
	return true;
}

void setupInterrupt()
{
	// pinMode(PIR_PIN, INPUT);
	pinMode(PIR_PIN, INPUT_PULLUP);
	attachInterrupt(PIR_PIN, interruptHandler, CHANGE);
	// attachInterrupt(PIR_PIN, interruptHandler, FALLING);
}

///////////////////////////////////////
///// WIFI Stuff
///////////////////////////////////////

// Will be called when WiFi station was connected to AP
void wifiConnectOk()
{
	debugf("WiFi CONNECTED");
	startTelnetServer();
	startMqttClient();
}

// Will be called when WiFi station timeout was reached
void wifiConnectFail()
{
	debugf("WiFi NOT CONNECTED!");

	//FLASH ERROR TODO

	WifiStation.waitConnection(wifiConnectOk, 10, wifiConnectFail); // Repeat and check again
}

void configureWifi()
{
	WifiAccessPoint.enable(false);
	WifiStation.enable(true);
	WifiStation.setHostname(NetConfig.mqtt_clientid+".realraum.at");
	WifiStation.config(NetConfig.wifi_ssid, NetConfig.wifi_pass); // Put you SSID and Password here
	WifiStation.enableDHCP(NetConfig.enabledhcp);
	if (!NetConfig.enabledhcp)
		WifiStation.setIP(NetConfig.ip,NetConfig.netmask,NetConfig.gw);
}

void connectToWifi()
{	
	// init BMP280
	setupBmp280();
	// Run our method when station was connected to AP (or not connected)
	WifiStation.waitConnection(wifiConnectOk, 30, wifiConnectFail); // We recommend 20+ seconds at start
}

//////////////////////////////////////
////// Base System Stuff  ////////////
//////////////////////////////////////

void setupPINs() {
	pinMode(15, OUTPUT); //GND for BM280
	digitalWrite(15, LOW);
	pinMode(14, OUTPUT); //GND for BM280
	digitalWrite(14, HIGH);
	pinMode(16, OUTPUT); //GND for BM280
	digitalWrite(16, HIGH);

}

void init()
{
	Serial.begin(SERIAL_BAUD_RATE);
	Serial.systemDebugOutput(true); // Allow debug print to serial
	// Serial.commandProcessing(true);
	// Mount file system, in order to work with files
/*	int slot = rboot_get_current_rom();
#ifndef DISABLE_SPIFFS
	if (slot == 0) {
#ifdef RBOOT_SPIFFS_0
		debugf("trying to mount spiffs at %x, length %d", RBOOT_SPIFFS_0 + 0x40200000, SPIFF_SIZE);
		spiffs_mount_manual(RBOOT_SPIFFS_0 + 0x40200000, SPIFF_SIZE);
#else
		debugf("trying to mount spiffs at %x, length %d", 0x40300000, SPIFF_SIZE);
		spiffs_mount_manual(0x40300000, SPIFF_SIZE);
#endif
	} else {
#ifdef RBOOT_SPIFFS_1
		debugf("trying to mount spiffs at %x, length %d", RBOOT_SPIFFS_1 + 0x40200000, SPIFF_SIZE);
		spiffs_mount_manual(RBOOT_SPIFFS_1 + 0x40200000, SPIFF_SIZE);
#else
		debugf("trying to mount spiffs at %x, length %d", 0x40500000, SPIFF_SIZE);
		spiffs_mount_manual(0x40500000, SPIFF_SIZE);
#endif
	}
#else
	debugf("spiffs disabled");
#endif
*/	//spiffs_mount(); // default auto spiffs mount
	spiffs_mount_manual(0x100000,SPIFF_SIZE);
	setupPINs(); //Init HW
	setupInterrupt();
	telnetRegisterCmdsWithCommandHandler();
	commandHandler.registerSystemCommands();
	// configure stuff that needs to be done before system is ready
	NetConfig.load(); //loads netsettings from fs
	configureWifi();
	instantinateMQTT();
	// Set system ready callback method
	System.onReady(connectToWifi);
}
