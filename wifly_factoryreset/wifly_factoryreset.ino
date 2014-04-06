/*
 * WiFlyHQ Example udpclient.ino
 *
 * This sketch resets the WiFly to factory defaults.
 * You should upload a useful sketch after a power-cycle.
 *
 * This sketch is released to the public domain.
 *
 */


#include <SoftwareSerial.h>
SoftwareSerial wifiSerial(2,3);

//#include <AltSoftSerial.h>
//AltSoftSerial wifiSerial(8,9);

#include <WiFlyHQ.h>

void terminal();

WiFly wifly;

void setup()
{
    char buf[32];

    Serial.begin(115200);
    Serial.println("Starting");
    Serial.print("Free memory: ");
    Serial.println(wifly.getFreeMemory(),DEC);

    wifiSerial.begin(9600);

    if (!wifly.begin(&wifiSerial, &Serial)) {
        Serial.println("Failed to start wifly");
	terminal();
    }

    if (wifly.getFlushTimeout() != 10) {
        Serial.println("Restoring flush timeout to 10msecs");
        wifly.setFlushTimeout(10);
	wifly.save();
	wifly.reboot();
    }

   Serial.println("WiFly ready");
   
   /* Factory Reset */
   Serial.println("Factory restoring Wifly...");
   wifly.factoryRestore();
   Serial.println("Rebooting WiFly...");
   wifly.reboot();
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
