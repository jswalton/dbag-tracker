
#include <SoftwareSerial.h>
#include <Adafruit_GPS.h>
#include <Adafruit_FONA.h>

#include "Location.h"

#define FONA_PS 6
#define FONA_KEY A2
#define FONA_RI 12    // <-- must be connected to external interrupt pin
#define FONA_RX 9
#define FONA_TX 10
#define FONA_RST 4
#define FIX_LED 13

#define GPS_INTERVAL_SECONDS 10


Adafruit_GPS gps = Adafruit_GPS(&Serial1);

SoftwareSerial fonaSerial = SoftwareSerial(FONA_TX, FONA_RX);

Adafruit_FONA fona = Adafruit_FONA(FONA_RST);

Location current_location = Location();

unsigned long timer = 0;
volatile boolean ringing = false;


// Interrupt is called once a millisecond, looks for any new GPS data, and stores it
SIGNAL(TIMER0_COMPA_vect) {
  char c = gps.read();
  //if (c) UDR0 = c;
}

void setup () {
  delay(500);

  Serial.begin(115200);
  Serial.println(F("GPS Tracker."));

  pinMode(FONA_KEY, OUTPUT);
  digitalWrite(FONA_KEY, HIGH);

  // Power up the FONA if it needs it
  if (digitalRead(FONA_PS) == LOW) {
    Serial.print(F("Powering FONA on..."));
    while (digitalRead(FONA_PS) == LOW) {
      digitalWrite(FONA_KEY, LOW);
      delay(500);
    }
    digitalWrite(FONA_KEY, HIGH);
    Serial.println(F(" done."));
    delay(500);
  }

  // Start the FONA
  Serial.print(F("Initializing FONA..."));
  fonaSerial.begin(4800);
  while (1) {
    boolean fonaStarted = fona.begin(fonaSerial);
    if (fonaStarted){
      Serial.println(F(" fona started success."));
      break;
      }
    delay (1000);
  }
  Serial.println(F(" done."));

  // wait for a valid network, nothing works w/o that
  Serial.print(F("Waiting for GSM network..."));
  while (1) {
    uint8_t network_status = fona.getNetworkStatus();
    if (network_status == 1 || network_status == 5) break;
    delay(250);
  }

  // We need to start the GPS second... By default, the last intialized port is listening.
  Serial.print(F("Starting GPS..."));
  gps.begin(9600);
  gps.sendCommand(PMTK_SET_NMEA_OUTPUT_RMCGGA);
  gps.sendCommand(PMTK_SET_NMEA_UPDATE_1HZ);   // 1 Hz update rate
  pinMode(FIX_LED, OUTPUT);
  Serial.println(F(" done."));


  // Timer0 is already used for millis() - we'll just interrupt somewhere
  // in the middle and call the "Compare A" function above
  OCR0A = 0xAF;
  TIMSK0 |= _BV(OCIE0A);
}


void loop () {

  if (gps.newNMEAreceived() && gps.parse(gps.lastNMEA())) {
    boolean isValidFix = gps.fix && gps.HDOP < 5 && gps.HDOP != 0;  // HDOP == 0 is an error case that comes up on occasion.
    if (isValidFix) current_location.set(gps);
    if (millis() - timer > GPS_INTERVAL_SECONDS * 1000) {
      Serial.print(F("Satellites: ")); Serial.println((int)gps.satellites);

      if (isValidFix) {
        Serial.print(F("Location: "));
        Serial.print(current_location.latitude, 6);
        Serial.print(F(", "));
        Serial.print(current_location.longitude, 6);
        Serial.print(F(", "));
        Serial.println(current_location.altitude, 2);

        Serial.print(F("HDOP: ")); Serial.println(gps.HDOP);

        //if (digitalRead(FONA_PS) == HIGH && !current_location.isEqual(last_location)) {
        if (digitalRead(FONA_PS) == HIGH) {
          sendLocation();
        //  last_location.set(gps);
        }
      } else {
        Serial.println(F("No valid fix."));
      }
      Serial.println();

      timer = millis(); // reset the timer
    }
  }

  // if millis() wraps around, reset the timer

  if (timer > millis()){
    timer = millis();
  } 
}


void sendLocation () {
  char url[160];
  char data[80];
  uint16_t statuscode;
  int16_t length;

  sprintf (url, "http://dbag-tracker.herokuapp.com/locations");
  sprintf (data, "{\"latitude\": %s, \"longitude\": %s}",
    current_location.latitude_c, current_location.longitude_c);

  Serial.print(F("Sending: ")); Serial.println(url);

  // Make the FONA listen, we kinda need that...
  fonaSerial.listen();


    if (!fona.HTTP_POST_start(url, F("text/plain"), (uint8_t *) data, strlen(data), &statuscode, (uint16_t *)&length)) {
      while (length > 0) {
        while (fona.available()) {
          char c = fona.read();

          // // Serial.write is too slow, we'll write directly to Serial register!
          // loop_until_bit_is_set(UCSR0A, UDRE0); /* Wait until data register empty. */
          // UDR0 = c;


          length--;
          if (! length) break;
        }
      }
      fona.HTTP_POST_end();
    } else {
      Serial.println(F("Failed to send GPRS data!"));
    }
}



uint8_t barsFromRSSI (uint8_t rssi) {
  // https://en.wikipedia.org/wiki/Mobile_phone_signal#ASU
  //
  // In GSM networks, ASU maps to RSSI (received signal strength indicator, see TS 27.007[1] sub clause 8.5).
  //   dBm = 2 × ASU - 113, ASU in the range of 0..31 and 99 (for not known or not detectable).

  int8_t dbm = 2 * rssi - 113;

  if (rssi == 99 || rssi == 0) {
    return 0;
  } else if (dbm < -107) {
    return 1;
  } else if (dbm < -98) {
    return 2;
  } else if (dbm < -87) {
    return 3;
  } else if (dbm < -76) {
    return 4;
  }

  return 5;
}
