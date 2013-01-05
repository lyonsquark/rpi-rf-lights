// Home monitoring and control

#include <OneWire.h>

#define RF_DATA_PIN 3
#define TEMP_PIN 2
#define LED_PIN 13

// The transmitters
class FanTransmitter {
public:

  FanTransmitter() :
  gap_(14375),
  sLow_(875),
  sHigh_(375),
  lLow_(500),
  lHigh_(792),
  quiet_(102900)
  {
  }

  void transmitCodeString(const String& code) {
    // Run through the code string and sent out bits
    for ( int i=0; i < code.length(); i++) {
      switch ( code[i] ) {
      case 'S':
        transmit(sLow_, sHigh_);
        break;

      case 'L':
        transmit(lLow_, lHigh_);
        break;

      case 'G':
        transmit(gap_, 0);
        break;

      case 'Q':
        transmit(quiet_,0);
        break;
      }
    }
  }

  void transmit(int usecsOff, int usecsOn) {

    digitalWrite(LED_PIN, HIGH);
    // first turn off
    if ( usecsOff > 0 ) {
      digitalWrite(RF_DATA_PIN, LOW);
      delayMicroseconds(usecsOff);
    }

    // Turn on
    if (usecsOn > 0 ) {
      digitalWrite(RF_DATA_PIN, HIGH);
      delayMicroseconds(usecsOn);
    }

    // Turn pin off again
    digitalWrite(RF_DATA_PIN, LOW); 

    digitalWrite(LED_PIN, LOW);

  }

private:
  float gap_, sLow_, sHigh_, lLow_, lHigh_, quiet_;

};


class RFOutletTransmitter {
public:

  RFOutletTransmitter() :
  longMarker_(10166),
  longTime_(1750),
  shortTime_(582)
  {
  }

  void transmitCodeString(const String& code) {
    // Run through the code and send out bits
    for (int i=0; i< code.length(); i++) {
      switch ( code[i] ) {
      case 'M':
        // Transmit long high marker
        transmit(longMarker_, 0);
        break;

      case 'L':
        // Transmit long low marker
        transmit(0, longMarker_);  
        break;

      case 'A':
        // Transmit A (long on, short off)
        transmit(longTime_, shortTime_);
        break;

      case 'B': 
        // Transmit B (short on, long off)
        transmit(shortTime_, longTime_);
      }
    }
  }

  void transmit(int usecsOn, int usecsOff) {

    digitalWrite(LED_PIN, HIGH);
    // first turn on
    if ( usecsOn > 0 ) {
      digitalWrite(RF_DATA_PIN, HIGH);
      delayMicroseconds(usecsOn);
    }

    // Turn back off
    if (usecsOff == 0 ) usecsOff=1;
    digitalWrite(RF_DATA_PIN, LOW);
    delayMicroseconds(usecsOff); 

    digitalWrite(LED_PIN, LOW);

  }

private:
  const int longMarker_;
  const int longTime_;
  const int shortTime_;

};

// Class for fan code
class FanCode {
public:

  FanCode( String stuff, int mult ) :
  stuff_(stuff),
  mult_(mult)
  {
  }

  String constructCodeString() {
    String code = "";
    for ( int i = 0; i < mult_; ++i ) {
      code += stuff_;
    }     

    code += "Q";

    return code;
  }

private:
  String stuff_;
  int mult_;
};

// Class for code
class OutletCode {
public:

  // C'tor
  OutletCode(String S, String s, int mult) :
  S_(S),
  s_(s),
  mult_(mult)
  {
  }

  // Construct the code
  String constructCodeString() {
    String code = "M";

    for ( int i=0; i < mult_; ++i ) {
      code += S_;
    }

    code += s_;

    return code;
  }

private:
  String S_;
  String s_;
  int mult_;
};

// The RF outlets codes
OutletCode s1on( "BAABABBBABBBBBBAL", "BAABABB", 6);
OutletCode s2on( "BAABABBBBBABBBBAL", "BAABABBBBB", 5);
OutletCode s3on( "BAABABBABBBBBBBAL", "BAABABBABBBBBBB", 5);
OutletCode s1off("BAABABBBBABBBBBAL", "BAABABB", 4);
OutletCode s2off("BAABABBBBBBABBBAL", "BAABABBBBBBABBB", 4);
OutletCode s3off("BAABABABBBBBBBBAL", "", 4);

// The fan code
FanCode fanToggle("SSSSLLSSLLLLLG", 12);

// Temperature stuff
OneWire ds(TEMP_PIN);

float getTemp() {
  //returns the temperature from one DS18S20 in DEG F
  digitalWrite(LED_PIN, HIGH);

  byte data[12];
  byte addr[8];

  if ( !ds.search(addr)) {
    //no more sensors on chain, reset search
    ds.reset_search();
    return -1000;
  }

  if ( OneWire::crc8( addr, 7) != addr[7]) {
    return -2000;
  }

  if ( addr[0] != 0x10 && addr[0] != 0x28) {
    return -3000;
  }

  ds.reset();
  ds.select(addr);
  ds.write(0x44,1); // start conversion, with parasite power on at the end

  delay(1000);

  byte present = ds.reset();
  ds.select(addr);  
  ds.write(0xBE); // Read Scratchpad


  for (int i = 0; i < 9; i++) { // we need 9 bytes
    data[i] = ds.read();
  }

  ds.reset_search();

  unsigned int raw = (data[1] << 8) | data[0];
  byte cfg = (data[4] & 0x60);
  if (cfg == 0x00) raw = raw << 3;  // 9 bit resolution, 93.75 ms
  else if (cfg == 0x20) raw = raw << 2; // 10 bit res, 187.5 ms
  else if (cfg == 0x40) raw = raw << 1; // 11 bit res, 375 ms
  // default is 12 bit resolution, 750 ms conversion time
  float celsius = (float)raw / 16.0;
  float fahrenheit = celsius * 1.8 + 32.0;

  digitalWrite(LED_PIN, LOW);

  return fahrenheit;
}

// Transmitters
RFOutletTransmitter t;
FanTransmitter f;

// the setup routine runs once when you press reset:
void setup() {
  // initialize serial communication at 9600 bits per second:
  Serial.begin(9600);

  // Set up the pins
  pinMode(LED_PIN, OUTPUT);
  pinMode(RF_DATA_PIN, OUTPUT);

  Serial.println("Ready");
}

// the loop routine runs over and over again forever:
void loop() {

  // Get a command from the serial port
  // A = turn on S1
  // B = turn on S2
  // C = turn on S3
  // a = turn off S1
  // b = turn off S2
  // c = turn off S3
  // t = get temp
  // toggle fan

    if ( Serial.available() > 0 ) {
    char command = Serial.read();

    switch (command) {
    case 'A':
      t.transmitCodeString( s1on.constructCodeString() );
      break;

    case 'B':
      t.transmitCodeString( s2on.constructCodeString() );
      break;

    case 'C':
      t.transmitCodeString( s3on.constructCodeString() );
      break;

    case 'a':
      t.transmitCodeString( s1off.constructCodeString() );
      break;

    case 'b':
      t.transmitCodeString( s2off.constructCodeString() );
      break;

    case 'c':
      t.transmitCodeString( s3off.constructCodeString() );
      break;

    case 't':
      Serial.println( getTemp() );
      break;

    case 'f':
      f.transmitCodeString( fanToggle.constructCodeString() );
      break;

    default:
      Serial.println("??");
      break;
    }

    delay(500);

    Serial.print(command);
    Serial.println("Ack");

  }
}























