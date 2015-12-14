/*
 * Usage write: low_th  high_th
 *       read : distance
 */

#include <TinyWireS.h>              // Requires fork by Rambo with onRequest support
#include <TinyNewPing.h>            // NewPing library modified for ATtiny
#include <EEPROM.h>

#define LED_R_PIN 4
#define LED_G_PIN 1

#define EEPROM_THRES_ADDR 0

const byte SensorTrigPin = 3;        // Sensor 1 is connected to PB3
const byte SensorEchoPin = 3;        // Sensor 1 is connected to PB3
const int I2CSlaveAddress = 0x26;      // I2C Address.

byte distance;                              // Where the Distance is stored (8 bit unsigned)
uint8_t thres[2]; // 0-warn, 1-emerg

#define MAX_DISTANCE 200 // Maximum distance we want to ping for (in centimeters). Maximum sensor distance is rated at 400-500cm.

NewPing SensorOne (SensorTrigPin, SensorEchoPin, MAX_DISTANCE);       // Define the Sensor


void setup()
{
  delay(100);
  EEPROM.begin();

  pinMode(LED_R_PIN, OUTPUT);
  pinMode(LED_G_PIN, OUTPUT);
  digitalWrite(LED_R_PIN, LOW);
  digitalWrite(LED_G_PIN, LOW);

  thres[0] = EEPROM.read(EEPROM_THRES_ADDR + 0);
  thres[1] = EEPROM.read(EEPROM_THRES_ADDR + 1);
  if (thres[0] == 255) {
    thres[0] = 0;
  }
  if (thres[1] == 255) {
    thres[1] = 0;
  }

  TinyWireS.begin(I2CSlaveAddress);      // Begin I2C Communication
  TinyWireS.onRequest(transmit);         // When requested, call function transmit()
  TinyWireS.onReceive(set_thres);
}


void loop()
{
  //distance = SensorOne.ping_cm();        // Get distance in cm. Could be changed to
  distance = SensorOne.ping_median(10) / US_ROUNDTRIP_CM;     // Take the median of 5 readings

  delay(30);                             // Delay to avoid interference from last ping

  if (thres[0] > 0 && thres[1] > 0) {
    if (distance >= thres[0] && distance < thres[1]) {
      digitalWrite(LED_R_PIN,HIGH);
      digitalWrite(LED_G_PIN,HIGH);
    } else if (distance >= thres[0] && distance >= thres[1]) {
      digitalWrite(LED_R_PIN,HIGH);
      digitalWrite(LED_G_PIN,LOW);      
    } else {
      digitalWrite(LED_R_PIN,LOW);
      digitalWrite(LED_G_PIN,HIGH);            
    }
  }
}

void set_thres(uint8_t y) {
  uint8_t x[16];
  int i = 0;
  if (y < 1) {
    return;
  }
  while (y--) {
//  while (TinyWireS.available()) {
    x[i] = TinyWireS.receive(); // receive byte as a character
    i++;
  }
  if (i > 2) {
    thres[0] = x[0];    
    thres[1] = x[1];    
    EEPROM.update(EEPROM_THRES_ADDR + 0,thres[0]);
    EEPROM.update(EEPROM_THRES_ADDR + 1,thres[1]);
  }
}

void transmit()
{
  TinyWireS.send(distance);                 // Send last recorded distance for current sensor
}





