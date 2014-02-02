#include "application.h"

// ------------- configuration ------------

// set the parametrs from= and to= (keep the fields[] as it is).
const char* query = "/v1/connections?from=Bern&to=Bern,Hirschengraben&fields[]=connections/from/departure&limit=6";

const char* connName1 = "9 Bern";

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
	} else if (diffSeconds < 5 * 60) {
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
#include "application.h"


// ---------------- begin  of OLED code -------------------

#include <stdio.h>
#include <string.h>
#include <inttypes.h>

// OLED hardware versions
#define OLED_V1 0x01
#define OLED_V2 0x02

// commands
#define LCD_CLEARDISPLAY 0x01
#define LCD_RETURNHOME 0x02
#define LCD_ENTRYMODESET 0x04
#define LCD_DISPLAYCONTROL 0x08
#define LCD_CURSORSHIFT 0x10
#define LCD_FUNCTIONSET 0x28
#define LCD_SETCGRAMADDR 0x40
#define LCD_SETDDRAMADDR 0x80

// flags for display entry mode
#define LCD_ENTRYRIGHT 0x00
#define LCD_ENTRYLEFT 0x02
#define LCD_ENTRYSHIFTINCREMENT 0x01
#define LCD_ENTRYSHIFTDECREMENT 0x00

// flags for display on/off control
#define LCD_DISPLAYON 0x04
#define LCD_DISPLAYOFF 0x00
#define LCD_CURSORON 0x02
#define LCD_CURSOROFF 0x00
#define LCD_BLINKON 0x01
#define LCD_BLINKOFF 0x00

// flags for display/cursor shift
// Adafruit_CharacterOLED.h

#define LCD_DISPLAYMOVE 0x08
#define LCD_CURSORMOVE 0x00
#define LCD_MOVERIGHT 0x04
#define LCD_MOVELEFT 0x00

// flags for function set
#define LCD_8BITMODE    0x10
#define LCD_4BITMODE    0x00
#define LCD_JAPANESE    0x00
#define LCD_EUROPEAN_I  0x01
#define LCD_RUSSIAN     0x02
#define LCD_EUROPEAN_II 0x03


class Adafruit_CharacterOLED : public Print {
public:
  Adafruit_CharacterOLED(uint8_t ver, uint8_t rs, uint8_t rw, uint8_t enable,
		uint8_t d4, uint8_t d5, uint8_t d6, uint8_t d7);

  void init(uint8_t ver, uint8_t rs, uint8_t rw, uint8_t enable,
	    uint8_t d4, uint8_t d5, uint8_t d6, uint8_t d7);

  void begin(uint8_t cols, uint8_t rows);

  void clear();
  void home();

  void noDisplay();
  void display();
  void noBlink();
  void blink();
  void noCursor();
  void cursor();
  void scrollDisplayLeft();
  void scrollDisplayRight();
  void leftToRight();
  void rightToLeft();
  void autoscroll();
  void noAutoscroll();

  void createChar(uint8_t, uint8_t[]);
  void setCursor(uint8_t, uint8_t);
  virtual size_t write(uint8_t);
  void command(uint8_t);

private:
  void send(uint8_t, uint8_t);
  void write4bits(uint8_t);
  void pulseEnable();
  void waitForReady();

  uint8_t _oled_ver; // OLED_V1 = older, OLED_V2 = newer hardware version.
  uint8_t _rs_pin; // LOW: command.  HIGH: character.
  uint8_t _rw_pin; // LOW: write to LCD.  HIGH: read from LCD.
  uint8_t _enable_pin; // activated by a HIGH pulse.
  uint8_t _busy_pin;   // HIGH means not ready for next command
  uint8_t _data_pins[4];

  uint8_t _displayfunction;
  uint8_t _displaycontrol;
  uint8_t _displaymode;
  uint8_t _initialized;
  uint8_t _currline;
  uint8_t _numlines;
};

// Adafruit_CharacterOLED.cpp

// Derived from LiquidCrystal by David Mellis
// With portions adapted from Elco Jacobs OLEDFourBit
// Modified for 4-bit operation of the Winstar 16x2 Character OLED
// By W. Earl for Adafruit - 6/30/12
// Initialization sequence fixed by Technobly - 9/22/2013

// On power up, the display is initilaized as:
// 1. Display clear
// 2. Function set:
//    DL="1": 8-bit interface data
//    N="0": 1-line display
//    F="0": 5 x 8 dot character font
// 3. Power turn off
//    PWR="0"
// 4. Display on/off control: D="0": Display off C="0": Cursor off B="0": Blinking off
// 5. Entry mode set
//    I/D="1": Increment by 1
//    S="0": No shift
// 6. Cursor/Display shift/Mode / Pwr
//    S/C="0", R/L="1": Shifts cursor position to the right
//    G/C="0": Character mode
//    Pwr="1": Internal DCDC power on
//
// Note, however, that resetting the Arduino doesn't reset the LCD, so we
// can't assume that its in that state when a sketch starts (and the
// LiquidCrystal constructor is called).

Adafruit_CharacterOLED::Adafruit_CharacterOLED(uint8_t ver, uint8_t rs, uint8_t rw, uint8_t enable,
			     uint8_t d4, uint8_t d5, uint8_t d6, uint8_t d7)
{
  init(ver, rs, rw, enable, d4, d5, d6, d7);
}

void Adafruit_CharacterOLED::init(uint8_t ver, uint8_t rs, uint8_t rw, uint8_t enable,
			 uint8_t d4, uint8_t d5, uint8_t d6, uint8_t d7)
{
  _oled_ver = ver;
  if(_oled_ver != OLED_V1 && _oled_ver != OLED_V2) {
    _oled_ver = OLED_V2; // if error, default to newer version
  }
  _rs_pin = rs;
  _rw_pin = rw;
  _enable_pin = enable;

  _data_pins[0] = d4;
  _data_pins[1] = d5;
  _data_pins[2] = d6;
  _data_pins[3] = _busy_pin = d7;

  pinMode(_rs_pin, OUTPUT);
  pinMode(_rw_pin, OUTPUT);
  pinMode(_enable_pin, OUTPUT);

  _displayfunction = LCD_FUNCTIONSET | LCD_4BITMODE;

  begin(16, 2);
}

void Adafruit_CharacterOLED::begin(uint8_t cols, uint8_t lines)
{
  _numlines = lines;
  _currline = 0;

  pinMode(_rs_pin, OUTPUT);
  pinMode(_rw_pin, OUTPUT);
  pinMode(_enable_pin, OUTPUT);

  digitalWrite(_rs_pin, LOW);
  digitalWrite(_enable_pin, LOW);
  digitalWrite(_rw_pin, LOW);

  delayMicroseconds(50000); // give it some time to power up

  // Now we pull both RS and R/W low to begin commands

  for (int i = 0; i < 4; i++) {
    pinMode(_data_pins[i], OUTPUT);
    digitalWrite(_data_pins[i], LOW);
  }

  // Initialization sequence is not quite as documented by Winstar.
  // Documented sequence only works on initial power-up.
  // An additional step of putting back into 8-bit mode first is
  // required to handle a warm-restart.
  //
  // In the data sheet, the timing specs are all zeros(!).  These have been tested to
  // reliably handle both warm & cold starts.

  // 4-Bit initialization sequence from Technobly
  write4bits(0x03); // Put back into 8-bit mode
  delayMicroseconds(5000);
  if(_oled_ver == OLED_V2) {  // only run extra command for newer displays
    write4bits(0x08);
    delayMicroseconds(5000);
  }
  write4bits(0x02); // Put into 4-bit mode
  delayMicroseconds(5000);
  write4bits(0x02);
  delayMicroseconds(5000);
  write4bits(0x08);
  delayMicroseconds(5000);

  command(0x08);	// Turn Off
  delayMicroseconds(5000);
  command(0x01);	// Clear Display
  delayMicroseconds(5000);
  command(0x06);	// Set Entry Mode
  delayMicroseconds(5000);
  command(0x02);	// Home Cursor
  delayMicroseconds(5000);
  command(0x0C);	// Turn On - enable cursor & blink
  delayMicroseconds(5000);
}

/********** high level commands, for the user! */
void Adafruit_CharacterOLED::clear()
{
  command(LCD_CLEARDISPLAY);  // clear display, set cursor position to zero
  //  delayMicroseconds(2000);  // this command takes a long time!
}

void Adafruit_CharacterOLED::home()
{
  command(LCD_RETURNHOME);  // set cursor position to zero
  //  delayMicroseconds(2000);  // this command takes a long time!
}

void Adafruit_CharacterOLED::setCursor(uint8_t col, uint8_t row)
{
  uint8_t row_offsets[] = { 0x00, 0x40, 0x14, 0x54 };
  if ( row >= _numlines )
  {
    row = 0;  //write to first line if out off bounds
  }

  command(LCD_SETDDRAMADDR | (col + row_offsets[row]));
}

// Turn the display on/off (quickly)
void Adafruit_CharacterOLED::noDisplay()
{
  _displaycontrol &= ~LCD_DISPLAYON;
  command(LCD_DISPLAYCONTROL | _displaycontrol);
}
void Adafruit_CharacterOLED::display()
{
  _displaycontrol |= LCD_DISPLAYON;
  command(LCD_DISPLAYCONTROL | _displaycontrol);
}

// Turns the underline cursor on/off
void Adafruit_CharacterOLED::noCursor()
{
  _displaycontrol &= ~LCD_CURSORON;
  command(LCD_DISPLAYCONTROL | _displaycontrol);
}
void Adafruit_CharacterOLED::cursor()
{
  _displaycontrol |= LCD_CURSORON;
  command(LCD_DISPLAYCONTROL | _displaycontrol);
}

// Turn on and off the blinking cursor
void Adafruit_CharacterOLED::noBlink()
{
  _displaycontrol &= ~LCD_BLINKON;
  command(LCD_DISPLAYCONTROL | _displaycontrol);
}
void Adafruit_CharacterOLED::blink()
{
  _displaycontrol |= LCD_BLINKON;
  command(LCD_DISPLAYCONTROL | _displaycontrol);
}

// These commands scroll the display without changing the RAM
void Adafruit_CharacterOLED::scrollDisplayLeft(void)
{
  command(LCD_CURSORSHIFT | LCD_DISPLAYMOVE | LCD_MOVELEFT);
}
void Adafruit_CharacterOLED::scrollDisplayRight(void)
{
  command(LCD_CURSORSHIFT | LCD_DISPLAYMOVE | LCD_MOVERIGHT);
}

// This is for text that flows Left to Right
void Adafruit_CharacterOLED::leftToRight(void)
{
  _displaymode |= LCD_ENTRYLEFT;
  command(LCD_ENTRYMODESET | _displaymode);
}

// This is for text that flows Right to Left
void Adafruit_CharacterOLED::rightToLeft(void)
{
  _displaymode &= ~LCD_ENTRYLEFT;
  command(LCD_ENTRYMODESET | _displaymode);
}

// This will 'right justify' text from the cursor
void Adafruit_CharacterOLED::autoscroll(void)
{
  _displaymode |= LCD_ENTRYSHIFTINCREMENT;
  command(LCD_ENTRYMODESET | _displaymode);
}

// This will 'left justify' text from the cursor
void Adafruit_CharacterOLED::noAutoscroll(void)
{
  _displaymode &= ~LCD_ENTRYSHIFTINCREMENT;
  command(LCD_ENTRYMODESET | _displaymode);
}

// Allows us to fill the first 8 CGRAM locations
// with custom characters
void Adafruit_CharacterOLED::createChar(uint8_t location, uint8_t charmap[])
{
  location &= 0x7; // we only have 8 locations 0-7
  command(LCD_SETCGRAMADDR | (location << 3));
  for (int i=0; i<8; i++)
  {
    write(charmap[i]);
  }
}

/*********** mid level commands, for sending data/cmds */

inline void Adafruit_CharacterOLED::command(uint8_t value)
{
  send(value, LOW);
  waitForReady();
}

inline size_t Adafruit_CharacterOLED::write(uint8_t value)
{
  send(value, HIGH);
  waitForReady();
  return 0;
}

/************ low level data pushing commands **********/

// write either command or data
void Adafruit_CharacterOLED::send(uint8_t value, uint8_t mode)
{
  digitalWrite(_rs_pin, mode);
  pinMode(_rw_pin, OUTPUT);
  digitalWrite(_rw_pin, LOW);

  write4bits(value>>4);
  write4bits(value);
}

void Adafruit_CharacterOLED::pulseEnable(void)
{
  digitalWrite(_enable_pin, HIGH);
  delayMicroseconds(50);    // Timing Spec?
  digitalWrite(_enable_pin, LOW);
}

void Adafruit_CharacterOLED::write4bits(uint8_t value)
{
  for (int i = 0; i < 4; i++)
  {
    pinMode(_data_pins[i], OUTPUT);
    digitalWrite(_data_pins[i], (value >> i) & 0x01);
  }
  delayMicroseconds(50); // Timing spec?
  pulseEnable();
}

// Poll the busy bit until it goes LOW
void Adafruit_CharacterOLED::waitForReady(void)
{
  unsigned char busy = 1;
  pinMode(_busy_pin, INPUT);
  digitalWrite(_rs_pin, LOW);
  digitalWrite(_rw_pin, HIGH);
  do
  {
  	digitalWrite(_enable_pin, LOW);
  	digitalWrite(_enable_pin, HIGH);

  	delayMicroseconds(10);
  	busy = digitalRead(_busy_pin);
  	digitalWrite(_enable_pin, LOW);

  	pulseEnable();		// get remaining 4 bits, which are not used.
  }
  while(busy);

  pinMode(_busy_pin, OUTPUT);
  digitalWrite(_rw_pin, LOW);
}

Adafruit_CharacterOLED *lcd;


// ---------------- end of OLED code -------------------



#include <time.h>
#define DEBUG false

String http_get(const char* hostname, String path);
long currentTimeEpoche();
long parseDateWithTimezone(String str);

int led = D7;
Status currentStatus = off;

unsigned int nextTime = 0;    // next time to contact the server

#define CONNECTION_CACHE_SIZE 10
unsigned long connections[CONNECTION_CACHE_SIZE];

unsigned long lastTimestamp =0;
long lastTsMilis  = 0;

void setup() {
	Serial.begin(9600);
	pinMode(led, OUTPUT);

    lcd = new Adafruit_CharacterOLED(OLED_V1, D0, D1, D2, D3, D4, D5, D6);
    lcd->clear();

	 // Spark.variable("ms", &lastTsMilis, INT);
	 // Spark.variable("ts", &lastTimestamp, INT);


	// init connection array
	for (int i = 0; i < CONNECTION_CACHE_SIZE; i++) {
		connections[i] = 0;
	}

	RGB.control(true);
	RGB.color(0,0,10);   // dimmed blue
	RGB.brightness(255);
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
	int row = 0;
	for (int i = 0; i < CONNECTION_CACHE_SIZE && row < 2; i++) {
		long diff = cache[i] - now;
		if(diff < 0) {
			continue;
		} else {
			String time(diff / 60 ); // minutes

			// Connection name
			lcd->setCursor(0,row);
			lcd->print(connectionName);

			// fill spaces
			int spaces = 16 - strlen(connectionName) - time.length();
			for(int f = 0; f < spaces;f++) {
				lcd->print(" ");
			}

			// Print duration
			lcd->print(time);
			row++;
		}
	}
}



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
		lcd->setCursor(0,1);
		lcd->print("data loading...");

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


	delay(2000); // TODO: work arround the delay
	unsigned int bytes = client.available();
	for ( unsigned int i = 0; i < sizeof(buffer) && i < bytes; i++) {
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
	lcd->setCursor(0,1);
	lcd->print("updating time...");

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
