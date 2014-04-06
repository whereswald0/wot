/*
 * Adpated fromWiFlyHQ Example httpserver.ino
 *
 * This sketch implements a simple Web server that waits for requests
 * and serves up a small form asking for a relay state. Then when the
 * client posts that form, the relay changes states, and the server 
 * sends a greeting page with the relay state and the first char in 
 * byte from the form.
 *
 * This sketch is released to the public domain.
 *
 */

 /* Notes:
  * Uses chunked message bodies to work around a problem where
  * the WiFly will not handle the close() of a client initiated
  * TCP connection. It fails to send the FIN to the client.
  * (WiFly RN-XV Firmware version 2.32).
  */

/* Work around a bug with PROGMEM and PSTR where the compiler always
 * generates warnings.
 */

#include <WiFlyHQ.h>
#include <SoftwareSerial.h>

#define rxPin 2
#define txPin 3
 
#undef PROGMEM 
#define PROGMEM __attribute__(( section(".progmem.data") )) 
#undef PSTR 
#define PSTR(s) (__extension__({static prog_char __c[] PROGMEM = (s); &__c[0];})) 

byte relayPin[2] = {4,5};
int curState = 0;

SoftwareSerial wifiSerial(rxPin, txPin);

WiFly wifly;

/* Change these to match your WiFi network */
const char mySSID[] = "myNetworkname";
const char myPassword[] = "superSECRETpassword";

void sendIndex();
void sendGreeting(char *name);
void send404();

char buf[80];

void setup()
{
    
    pinMode(rxPin, INPUT);
    pinMode(txPin, OUTPUT);
    Serial.begin(9600);
    for(int i = 0; i < 2; i++)  pinMode(relayPin[i],OUTPUT);
    Serial.println(F("Starting"));
    Serial.print(F("Free memory: "));
    Serial.println(wifly.getFreeMemory(),DEC);
    
    wifiSerial.begin(9600);
    if (!wifly.begin(&wifiSerial, &Serial)) {
        Serial.println(F("Failed to start wifly"));
	wifly.terminal();
    } 

    /* Join wifi network if not already associated */
    if (!wifly.isAssociated()) {
	/* Setup the WiFly to connect to a wifi network */
	Serial.println(F("Joining network"));
	wifly.setSSID(mySSID);
	wifly.setPassphrase(myPassword);
	wifly.enableDHCP();
	wifly.save();

	if (wifly.join()) {
	    Serial.println(F("Joined wifi network"));
	} else {
	    Serial.println(F("Failed to join wifi network"));
	    wifly.terminal();
	}
    } else {
        Serial.println(F("Already joined network"));
    }

    wifly.setBroadcastInterval(0);	// Turn off UPD broadcast

    //wifly.terminal();

    Serial.print(F("MAC: "));
    Serial.println(wifly.getMAC(buf, sizeof(buf)));
    Serial.print(F("IP: "));
    Serial.println(wifly.getIP(buf, sizeof(buf)));

    wifly.setDeviceID("GROton-WebServer");

    if (wifly.isConnected()) {
        Serial.println(F("Old connection active. Closing"));
	wifly.close();
    }

    wifly.setProtocol(WIFLY_PROTOCOL_TCP);
    if (wifly.getPort() != 80) {
        wifly.setPort(80);
	/* local port does not take effect until the WiFly has rebooted (2.32) */
	wifly.save();
	Serial.println(F("Set port to 80, rebooting to make it work"));
	wifly.reboot();
	delay(3000);
    }
    Serial.println(F("Ready"));
  }

void loop()
{
    if (wifly.available() > 0) {
    
        /* See if there is a request */
	if (wifly.gets(buf, sizeof(buf))) {
	    if (strncmp_P(buf, PSTR("GET / "), 6) == 0) {
		/* GET request */
		Serial.println(F("Got GET request"));
		while (wifly.gets(buf, sizeof(buf)) > 0) {
		    /* Skip rest of request */
		}
		sendIndex();
		Serial.println(F("Sent index page"));
	    } else if (strncmp_P(buf, PSTR("POST"), 4) == 0) {
	        /* Form POST */
	        char states[2];
	        Serial.println(F("Got POST"));

		/* Get posted field value */
		if (wifly.match(F("state="))) {
		    wifly.gets(states, sizeof(states));
		    wifly.flushRx();		// discard rest of input
		    sendGreeting(states);
		    Serial.println(F("Sent greeting page"));
		}
	    } else {
	        /* Unexpected request */
		Serial.print(F("Unexpected: "));
		Serial.println(buf);
		wifly.flushRx();		// discard rest of input
		Serial.println(F("Sending 404"));
		send404();
	    }
	}
    }
}

/** Send an index HTML page with an input box for a username */
void sendIndex()
{
    /* Send the header direclty with print */
    wifly.println(F("HTTP/1.1 200 OK"));
    wifly.println(F("Content-Type: text/html"));
    wifly.println(F("Transfer-Encoding: chunked"));
    wifly.println();

    /* Send the body using the chunked protocol so the client knows when
     * the message is finished.
     * Note: we're not simply doing a close() because in version 2.32
     * firmware the close() does not work for client TCP streams.
     */
    wifly.sendChunkln(F("<html>"));
    wifly.sendChunkln(F("<title>Groton WiFly HTTP Server Example</title>"));
    wifly.sendChunkln(F("<h1>"));
    wifly.sendChunk(F("<p>Current status of Relay1: "));
    if (curState == 0){
      wifly.sendChunk(F("OFF"));
    } else{
      wifly.sendChunk(F("ON"));
    }
    wifly.sendChunkln(F("</p></h1>"));
    wifly.sendChunkln(F("<form name=\"input\" action=\"/\" method=\"post\">"));
    wifly.sendChunkln(F("Relay1 (0 for OFF/1 for ON):"));
    wifly.sendChunkln(F("<input type=\"text\" name=\"state\" />"));
    wifly.sendChunkln(F("<input type=\"submit\" value=\"Submit\" />"));
    wifly.sendChunkln(F("</form>")); 
    wifly.sendChunkln(F("</html>"));
    wifly.sendChunkln();
}

/** Send a greeting HTML page with the user's name and an analog reading */
void sendGreeting(char *tripped)
{
    if(tripped[0] == '0'){
      digitalWrite(4,LOW);
      curState = 0;
    }else if(tripped[0] == '1'){
      digitalWrite(4,HIGH);
      curState = 1;
    } else{
        digitalWrite(4,LOW);
        curState = 0;
      }
   
 
      /* Send the header directly with print */
    wifly.println(F("HTTP/1.1 200 OK"));
    wifly.println(F("Content-Type: text/html"));
    wifly.println(F("Transfer-Encoding: chunked"));
    wifly.println();
  
    /* Send the body using the chunked protocol so the client knows when
     * the message is finished.
     */
    wifly.sendChunkln(F("<html>"));
    wifly.sendChunkln(F("<title>WiFly HTTP Server Example</title>"));
    /* No newlines on the next parts */
    wifly.sendChunk(F("<h1><p>Current state of Relay1: "));
    if(tripped[0] == '0'){
      wifly.sendChunk(F("OFF"));
    } else{
      wifly.sendChunk(F("ON"));
    }
    //wifly.sendChunk(tripped);
    /* Finish the paragraph and heading */
    wifly.sendChunkln(F("</p></h1>"));
    
    /* Include a reading from Analog pin 0 */
    snprintf_P(buf, sizeof(buf), PSTR("<p>First char of tripped=%d</p>"), tripped[0]);
    wifly.sendChunkln(buf);

    wifly.sendChunkln(F("</html>"));
    wifly.sendChunkln();
}

/** Send a 404 error */
void send404()
{
    wifly.println(F("HTTP/1.1 404 Not Found"));
    wifly.println(F("Content-Type: text/html"));
    wifly.println(F("Transfer-Encoding: chunked"));
    wifly.println();
    wifly.sendChunkln(F("<html><head>"));
    wifly.sendChunkln(F("<title>404 Not Found</title>"));
    wifly.sendChunkln(F("</head><body>"));
    wifly.sendChunkln(F("<h1>Not Found</h1>"));
    wifly.sendChunkln(F("<hr>"));
    wifly.sendChunkln(F("</body></html>"));
    wifly.sendChunkln();
}
