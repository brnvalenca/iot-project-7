// #include <Arduino.h>
// #include <IRremoteESP8266.h>
// #include <IRsend.h>
// #include <connection.h>

// const int analogicPin = 2;

// const uint16_t kIrLed = 4;  
// IRsend irsend(kIrLed); 

// // put function declarations here:
// void setup() {
//   connectAWS();
//   analogReadResolution(12);
//   Serial.begin(115200);
//   irsend.begin();
//   // put your setup code here, to run once:
// }

// void loop() {
//   Serial.println("starting");
//   uint16_t rawData[] = {0x30};
//   irsend.sendRaw(rawData, sizeof(rawData) / sizeof(rawData[0]), 38);
//   int analogicValue = analogRead(analogicPin);
//   if (analogicValue  <= 1000) {
//     Serial.println("Luz ligada");
//   } else {
//     Serial.println("Luz desligada");
//   }
//   client.loop();
//   publishMessage(analogicValue);
//   Serial.println(analogicValue);
//   delay(5000);
//   // put your main code here, to run repeatedly:
// }

// // put function definitions here


