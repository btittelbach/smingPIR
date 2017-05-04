#include <SmingCore/SmingCore.h>
#include <spiffsconfig.h>
#include "pinconfig.h"
#include "mqtt.h"
#include "application.h"

//////////////////////////////////
/////// MQTT Stuff ///////////////
//////////////////////////////////


Timer procMQTTTimer;
MqttClient *mqtt = 0;


// Check for MQTT Disconnection
void checkMQTTDisconnect(TcpClient& client, bool flag){

	// Called whenever MQTT connection is failed.
	if (flag == true)
	{
		Serial.println("MQTT Broker Disconnected!!");
	}
	else
	{
		Serial.println("MQTT Broker Unreachable!!");
	}

	// Restart connection attempt after few seconds
	// changes procMQTTTimer callback function
	procMQTTTimer.initializeMs(2 * 1000, startMqttClient).start(); // every 2 seconds
}

void onMessageDelivered(uint16_t msgId, int type) {
	//Serial.printf("Message with id %d and QoS %d was delivered successfully.", msgId, (type==MQTT_MSG_PUBREC? 2: 1));
}

// Publish our message
void publishMessage()
{
	if (0 == mqtt)
		return;
	if (mqtt->getConnectionState() != eTCS_Connected)
		startMqttClient(); // Auto reconnect	
	
	if (did_pir_trigger())
	{
		Serial.println("movement!!");
		mqtt->publish(NetConfig.getMQTTTopic(JSON_TOPIC3_MOVEMENT), "{}", false);
	}
}

// Callback for messages, arrived from MQTT server
void onMessageReceived(String topic, String message)
{
}

// Run MQTT client, connect to server, subscribe topics
void startMqttClient()
{
	procMQTTTimer.stop();
/*	if(!mqtt->setWill("last/will","The connection from this device is lost:(", 1, true)) {
		debugf("Unable to set the last will and testament. Most probably there is not enough memory on the device.");
	}
*/
	mqtt->connect(NetConfig.mqtt_clientid, NetConfig.mqtt_user, NetConfig.mqtt_pass, true);
	mqtt->setKeepAlive(42);
	mqtt->setPingRepeatTime(21);
#ifdef ENABLE_SSL
	mqtt->addSslOptions(SSL_SERVER_VERIFY_LATER);

	mqtt->setSslClientKeyCert(default_private_key, default_private_key_len,
							  default_certificate, default_certificate_len, NULL, true);

#endif
	// Assign a disconnect callback function
	mqtt->setCompleteDelegate(checkMQTTDisconnect);
	// mqtt->subscribe(NetConfig.getMQTTTopic(JSON_TOPIC3_LIGHT,true));

	procMQTTTimer.initializeMs(NetConfig.publish_interval, publishMessage).start(); // every 16 seconds
}

void stopMqttClient()
{
	// mqtt->unsubscribe(NetConfig.getMQTTTopic(JSON_TOPIC3_LIGHT,true));
	mqtt->setKeepAlive(0);
	mqtt->setPingRepeatTime(0);
	procMQTTTimer.stop();
}

void instantinateMQTT()
{
	mqtt = new MqttClient(NetConfig.mqtt_broker, NetConfig.mqtt_port, onMessageReceived);
}