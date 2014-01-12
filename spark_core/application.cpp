#include "application.h"

// ------------- configuration ------------

// set the parametrs from= and to= (keep the fields[] as it is).
 const char* query = "/v1/connections?from=Bern&to=Bern,Hirschengraben&fields[]=connections/from/departure&limit=6";

const char* connName1 = "9  >Bern";

// possible status list:
enum Status {
	// increasing priority, if there is "walk" and "run", it shows "walk".
	off=1, missed=2, run=3, leave_now=4, walk=5
};

Status getStatus(int diffSeconds) {
	if (diffSeconds < -60) {
		// too late
		return off;
	} else if (diffSeconds < 90) {
		return missed;
	} else if (diffSeconds < 3 * 60) {
		return run;
	} else if (diffSeconds < 3.5 * 60 ) {
		return leave_now;
	} else if (diffSeconds < 7 * 60) {
		return walk;
	} else {
		// longer, then turn led off
		return off;
	}
}

void updateLED(Status status) {
	switch(status) {
		case off:      RGB.color(0,0,10); break;   // dimmed blue
		case missed:   RGB.color(255,0,0); break;  // red
		case run:      RGB.color(255,50,0); break;// orange
		case leave_now:RGB.color(255,150,0); break;// yellow
		case walk:     RGB.color(0,255,0); break; // green
	}
}

// is a display connected?
#define DISPLAY true

// ------------- enf of configuration ------------

#include <time.h>
#define DEBUG true

String http_get(const char* hostname, String path);
long currentTimeEpoche();
long parseDateWithTimezone(String str);

int led = D7;
Status currentStatus = off;

unsigned int nextTime = 0;    // next time to contact the server

#define CONNECTION_CACHE_SIZE 10
unsigned long connections[CONNECTION_CACHE_SIZE];

void setup() {
	Serial.begin(9600);
	pinMode(led, OUTPUT);

	// init connection array
	for (int i = 0; i < CONNECTION_CACHE_SIZE; i++) {
		connections[i] = 0;
	}

	RGB.control(true);
	RGB.color(0,0,10);   // dimmed blue
}


void printCache(unsigned long* cache) {
	Serial.println("conn:");
	Serial.println("-----------");

	for (int i = 0; i < CONNECTION_CACHE_SIZE; i++) {
			Serial.println(cache[i]);
	}
	Serial.println("-------");
}
/**
 * Adds the given timestamp to the connection array. Multiple arrays might be possible.
 */
void addConnection(unsigned long ts, unsigned long* cache) {
	if(DEBUG) Serial.print("fahrplan ts:");
	if(DEBUG) Serial.println(ts);

	for (int i = 0; i < CONNECTION_CACHE_SIZE; i++) {

		if (cache[i] == ts) {
			// already in table, ignore
			return;
		} else if (cache[i] == 0) {
			// found empty slot, add ts. empty slots are in the end.
			cache[i] = ts;
			return;
		}
	}
}

void cleanupCache(unsigned long* cache, unsigned long now) {
	for (int i = 0; i < CONNECTION_CACHE_SIZE; i++) {
		if(cache[i] == 0) {
			// empty
		} else if(cache[i]  < now - 120) {
			// delete old entries
			cache[i] = 0;
		}
		if(i > 0 && cache[i-1] == 0) {
			// if prev entry is empty: move current
			cache[i-1] = cache[i];
			cache[i] = 0;
		}
	}
}
int getCacheSize(unsigned long* cache) {
	int count = 0;
	for (int i = 0; i < CONNECTION_CACHE_SIZE; i++) {
		if(cache[i] != 0) {
			count++;
		}
	}
	return count;
}

void parseFahrplan(String jsonData, unsigned long* cache) {
	int offset = 0;
	do {
		offset = jsonData.indexOf("departure\":\"", offset);
		if(DEBUG) Serial.print("offset: ");
		if(DEBUG) Serial.println(offset);

		if (offset == -1) {
			break;
		}
		offset += 12; // move to timestamp
		String str = jsonData.substring(offset, offset + 24);
		if(DEBUG) Serial.print("date: ");
		if(DEBUG) Serial.println(str);
		if(str.length() == 0) {
			continue;
		}
		long ts = parseDateWithTimezone(str);
		addConnection(ts, cache);
	} while (offset >= 0);
}


Status calculateStatus(unsigned long* cache, long now) {
	// Connections might be overlapping. In case you have every 2 minutes a connection,
	// you want to see green all the time. the enum Status is ordered by increasing priority.
	Status bestStatus = off;

	for (int i = 0; i < CONNECTION_CACHE_SIZE; i++) {
		unsigned long ts = cache[i];
		if(ts == 0) {
			continue;
		}
		int diff = ts - now;
		Status newColor = getStatus(diff);
		if (newColor > bestStatus) {
			bestStatus = newColor;
		}
		if(bestStatus == walk) {
			// it is as good as it gets, stop evaluating
			break;
		}
	}
	return bestStatus;
}

void updateDisplay(const char* connectionName, unsigned long* cache, unsigned long now) {
	if(DEBUG) Serial.println("updating display...");

	// assuming it is a 16 column, 2 row display
	String time1;
	String time2;
	int line = 1;
	for (int i = 0; i < CONNECTION_CACHE_SIZE; i++) {
		long diff = cache[i] - now;
		if(diff < 0) {
			continue;
		} else {
			String time(diff);
			if(line == 1){
				time1 = time;
				line++;
				continue;
			} else {
				time2 = time;
				break;
			}
		}
	}

	// Line 1
	String line1 (connectionName);
	unsigned int spacing1 = 16 - line1.length() - time1.length();
	for(unsigned int i=0; i < spacing1; i++) {
		line1 += " "; // fill with spaces
	}
	line1 += time1;

	// Line 2
	String line2 (connectionName);
	unsigned int spacing2 = 16 - line2.length() - time2.length();
	for(unsigned int j=0; j < spacing2; j++) {
		line2 += " "; // fill with spaces
	}
	line2 += time2;

	// print lines to display or serial
	// TODO: use Serial connection to display when ready
	Serial.println(line1);
	Serial.println(line2);
}



unsigned long lastTimestamp =0;
long lastTsMilis  = 0;

void loop() {
	if (nextTime > millis()) {
		// keep the same color while waiting
		return;
	}

	digitalWrite(led, HIGH);
	delay(200);
	digitalWrite(led, LOW);


	// the timestamp is checked online every 5 min
	if(lastTimestamp == 0 || lastTsMilis == 0 || millis() - lastTsMilis > 5 * 60 * 1000 ) {
		Serial.println("local time is old, refreshing");
		// refresh time online
		lastTimestamp = currentTimeEpoche();
		lastTsMilis = millis();
	}
	// estimate current time (last ts + difference)
	unsigned long now = lastTimestamp + ((millis() - lastTsMilis) / 1000);
	if(DEBUG) Serial.print("now: ");
	if(DEBUG) Serial.println(now);


	// organize connection cache
	cleanupCache(connections, now);
	if ( getCacheSize(connections) <=1 )  {
		Serial.println("loading connections...");
		// refresh connections
		String resp = http_get("transport.opendata.ch", query);
		parseFahrplan(resp, connections);
	}

	if(DEBUG)printCache(connections);

	Status status  = calculateStatus(connections, now);
	updateLED(status);

	if(DISPLAY) updateDisplay(connName1, connections,now);

	// check the color again in 5 seconds:
	nextTime = millis() + 5000;
}

// ------------- HTTP functions --------------

/**
 * make http request and return body
 */
TCPClient client;
char buffer[512];
String http_get(char const* hostname, String path) {

	if (client.connect(hostname, 80)) {
		client.print("GET ");
		client.print(path);
		client.print(" HTTP/1.0\n");
		client.print("HOST: ");
		client.println(hostname);
		client.print("\n");
		//	client.print("Connection: close\n\n");
		client.flush();
	} else {
		Serial.println("connection failed");
		client.stop();
		return NULL;
	}

	// Block until first byte is read.
	client.read();
	for (unsigned int i = 0; i < sizeof(buffer) && client.available(); i++) {
		char c = client.read();
		if (c == -1) {
			break;
		}
		buffer[i] = c;
	}
	client.stop();

	String response(buffer);
	int bodyPos = response.indexOf("\r\n\r\n");
	if (bodyPos == -1) {
		Serial.println("can not find http reponse body");
		return NULL;
	}
	return response.substring(bodyPos + 4);
}

// ------------- DATE / TIME functions --------------
/**
 * parse a string of the form "2014-01-11T17:17:59+0200"
 */
long parseDate(String str) {
	// TODO: it assumes it is running in UTC. mktime() uses the local time (time zone) for creating timestamp.
	// parse date. timegm() would be better, but is not available.
	struct tm time;
	strptime(str.c_str(), "%Y-%m-%dT%H:%M:%S", &time);
	return (long) mktime(&time);
}


/**
 * can parse the timezone offset in the string "2014-01-11T17:17:59+0100"
 */
long parseTzOffset(String str) {
	// strptime currently does not parse the timezone with %z, so we do it ourself:
	// parse 3 digits the "+0100" which result in 1 hour.
	int offsetHours = atoi(str.substring(19,22).c_str());
	return offsetHours * 3600;
}


/**
 * parse a string of the form "2014-01-11T17:17:59+0200" and fixes the timezone offset
 */
long parseDateWithTimezone(String str) {
	long ts = parseDate(str);
	long offset = parseTzOffset(str);
	ts -= offset;
	if(DEBUG) Serial.print("convert ");
	if(DEBUG) Serial.print(str);
	if(DEBUG) Serial.print(" to ");
	if(DEBUG) Serial.println(ts);
	return ts;
}

/**
 * returns current time since epoche, from a http server.
 */
long currentTimeEpoche() {
	Serial.println("getting current time");

	String response = http_get("www.timeapi.org", "/utc/now?\\s");
	if (response != NULL) {
		Serial.print("timeapi time=");
		Serial.println(response);
		return atoi(response.c_str());
	} else {
		return 0;
	}
}

/*timeapi example :
 *
 GET /utc/now?\s HTTP/1.0
 Host: www.timeapi.org
 Connection: close

 HTTP/1.1 200 OK
 Server: nginx
 Date: Sat, 11 Jan 2014 11:59:37 GMT
 Content-Type: text/html;charset=utf-8
 Content-Length: 25
 Connection: keep-alive
 X-Frame-Options: sameorigin
 X-Xss-Protection: 1; mode=block

1389485095
 */
