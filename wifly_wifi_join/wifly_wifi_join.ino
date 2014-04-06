/*
 * wifly_wifi_join.ino
 *
 * This sketch can be used as a test. It connects the WiFly to a wireless network
 * with specified SSID and passphrase.
 *
 * This sketch is released to the public domain.
 *
 */

#include <WiFlyHQ.h>
#include <SoftwareSerial.h>
SoftwareSerial wifiSerial(2,3);

//#include <AltSoftSerial.h>
//AltSoftSerial wifiSerial(8,9);

/* Change these to match your WiFi network */
const char mySSID[] = "myNetworkSSID";
const char myPassword[] = "superSECRETpassword";

WiFly wifly;

void setup()
{
    char buf[32];

    Serial.begin(115200);
    Serial.println("! Starting...");
    Serial.print("Free memory: ");
    Serial.println(wifly.getFreeMemory(),DEC);

    wifiSerial.begin(9600);
    if (!wifly.begin(&wifiSerial, &Serial)) {
        Serial.println("- Failed to start wifly  =(");
	terminal();
    }

    if (wifly.getFlushTimeout() != 10) {
        Serial.println("! Restoring flush timeout to 10msecs");
        wifly.setFlushTimeout(10);
	wifly.save();
	wifly.reboot();
    }

    /* Join wifi network if not already associated */
    if (!wifly.isAssociated()) {
	/* Setup the WiFly to connect to a wifi network */
	Serial.println("! Joining network...");
	wifly.setSSID(mySSID);
	wifly.setPassphrase(myPassword);
	wifly.enableDHCP();

	if (wifly.join()) {
	    Serial.println("+ Joined wifi network! ");
	} else {
	    Serial.println("- Failed to join wifi network");
	    terminal();
	}
    } else {
        Serial.println("! Already joined network");
    }

    Serial.print("MAC: ");
    Serial.println(wifly.getMAC(buf, sizeof(buf)));
    Serial.print("IP: ");
    Serial.println(wifly.getIP(buf, sizeof(buf)));
    Serial.print("Netmask: ");
    Serial.println(wifly.getNetmask(buf, sizeof(buf)));
    Serial.print("Gateway: ");
    Serial.println(wifly.getGateway(buf, sizeof(buf)));

    wifly.setDeviceID("Wifly-test");
    Serial.print("DeviceID: ");
    Serial.println(wifly.getDeviceID(buf, sizeof(buf)));

    Serial.println("! WiFly ready");
}

void loop()
{
    if (Serial.available()) {
        /* if the user hits 't', switch to the terminal for debugging */
        if (Serial.read() == 't') {
	    terminal();
	}
    }
}

void terminal()
{
    Serial.println("Terminal ready");
    while (1) {
	if (wifly.available() > 0) {
	    Serial.write(wifly.read());
	}

	if (Serial.available()) {
	    wifly.write(Serial.read());
	}
    }
}
