/*
   RadioLib SX126x Receive with Interrupts Example

   This example listens for LoRa transmissions and tries to
   receive them. Once a packet is received, an interrupt is
   triggered. To successfully receive data, the following
   settings have to be the same on both transmitter
   and receiver:
    - carrier frequency
    - bandwidth
    - spreading factor
    - coding rate
    - sync word

   Other modules from SX126x family can also be used.

   For default module settings, see the wiki page
   https://github.com/jgromes/RadioLib/wiki/Default-configuration#sx126x---lora-modem

   For full API reference, see the GitHub Pages
   https://jgromes.github.io/RadioLib/
*/

// include the library
#include <SPI.h>
#include <RadioLib.h>

#define ACTION

#define SS   10
#define MOSI 11
#define MISO 12
#define SCK  13

void somethingAvailableLORA();
void sendMessage(String&);

// SX1262 has the following connections:
// NSS pin:   10
// DIO1 pin:  2
// NRST pin:  3
// BUSY pin:  9
SX1262 radio = new Module(10, 2, 3, 9);

bool stringComplete = false;  // whether the string is complete
String inputString = "";         // a String to hold incoming data

// save transmission states between loops
int transmissionState = RADIOLIB_ERR_NONE;

// flag to indicate transmission or reception state
bool transmitFlag = false;

// flag to indicate that a packet was sent or received
volatile bool operationDone = false;

// this function is called when a complete packet
// is transmitted or received by the module
// IMPORTANT: this function MUST be 'void' type
//            and MUST NOT have any arguments!
#if defined(ESP8266) || defined(ESP32)
  ICACHE_RAM_ATTR
#endif
void setFlag(void) {
  // we sent or received a packet, set the flag
  operationDone = true;
}

void setup() {
  Serial.begin(115200);
  SPI.begin(SCK, MISO, MOSI, SS);

  // initialize SX1262 with default settings
#ifndef ACTION  
  Serial.print(F("[SX1262] Initializing ... "));
#endif
  int state = radio.begin();
  if (state == RADIOLIB_ERR_NONE) {
#ifndef ACTION    
    Serial.println(F("success!"));
#endif
  } else {
    Serial.print(F("failed, code "));
    Serial.println(state);
    while (true) { delay(10); }
  }

  // set carrier frequency to 433.5 MHz
  if (radio.setFrequency(868) == RADIOLIB_ERR_INVALID_FREQUENCY) {
    Serial.println(F("Selected frequency is invalid for this module!"));
    while (true) { delay(10); }
  }

  // set the function that will be called
  // when new packet is received
  radio.setDio1Action(setFlag);

  #if defined(INITIATING_NODE)
    // send the first packet on this node
    Serial.print(F("[SX1262] Sending first packet ... "));
    transmissionState = radio.startTransmit("Hello World!");
    transmitFlag = true;
  #else
    // start listening for LoRa packets on this node
  #ifndef ACTION
    Serial.print(F("[SX1262] Starting to listen ... "));
  #endif
    state = radio.startReceive();
    if (state == RADIOLIB_ERR_NONE) {
  #ifndef ACTION
      Serial.println(F("success!"));
  #endif  
    } else {
      Serial.print(F("failed, code "));
      Serial.println(state);
      while (true) { delay(10); }
    }
  #endif
}

void loop() {
  
  somethingAvailableLORA();

  while(Serial.available()>0){
    char inChar = (char)Serial.read();
    inputString += inChar;
    stringComplete = true;
  }

  if(stringComplete){
    stringComplete = false;
    // SerialPort.print(inputString);
    // ResponseStatus rs = e22ttl.sendFixedMessage(0, DESTINATION_ADDL, 23, inputString); // 20
    sendMessage(inputString);
    inputString = "";
  }

  // check if the previous operation finished
  if(operationDone) {
    // reset flag
    operationDone = false;

    if(transmitFlag) {
      // the previous operation was transmission, listen for response
      // print the result
      if (transmissionState == RADIOLIB_ERR_NONE) {
        // packet was successfully sent
    #ifndef ACTION
        Serial.println(F("transmission finished!"));
    #endif
      } else {
        Serial.print(F("failed, code "));
        Serial.println(transmissionState);
      }

      // listen for response
      radio.startReceive();
      transmitFlag = false;

    } else {
      // the previous operation was reception
      // print data and send another packet
      String str;
      int state = radio.readData(str);

      if (state == RADIOLIB_ERR_NONE) {
        // packet was successfully received
    #ifndef ACTION  
        Serial.println(F("[SX1262] Received packet!"));
    // #endif
        // print data of the packet
        Serial.print(F("[SX1262] Data:\t\t"));
    #endif
    #ifndef ACTION  
        Serial.println(str);
    #else
        Serial.print(str);
    #endif
        // print RSSI (Received Signal Strength Indicator)
      #ifndef ACTION  
        Serial.print(F("[SX1262] RSSI:\t\t"));
        Serial.print(radio.getRSSI());
        Serial.println(F(" dBm"));

        // print SNR (Signal-to-Noise Ratio)
        Serial.print(F("[SX1262] SNR:\t\t"));
        Serial.print(radio.getSNR());
        Serial.println(F(" dB"));
      #endif
      }

      // wait a second before transmitting again
      // delay(1000);

    }
  
  }

}

void somethingAvailableLORA(){
  
}

void sendMessage(String &str){
  // send another one
#ifndef ACTION
  Serial.print(F("[SX1262] Sending another packet ... "));
#endif
  // transmissionState = radio.startTransmit("Hello World!");
  transmissionState = radio.startTransmit(str);
  transmitFlag = true;
}