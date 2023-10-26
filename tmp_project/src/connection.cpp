#include "secrets.h"
#include <WiFiClientSecure.h>
#include <MQTTClient.h>
#include <ArduinoJson.h>
#include "WiFi.h"
#include <Arduino.h>
#include <IRremoteESP8266.h>
#include <IRrecv.h>
#include <IRsend.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

// The MQTT topics that this device should publish/subscribe
#define AWS_IOT_PUBLISH_TOPIC_LIGHT    "sala7/light"
#define AWS_IOT_PUBLISH_TOPIC_PROJECTOR    "sala7/Projector"
#define AWS_IOT_PUBLISH_TOPIC_SOUNDBAR    "sala7/Soundbar"

#define AWS_IOT_SUBSCRIBE_TOPIC "esp32/sub"


const int IRemitterPin = 32;  // Pino D34 para o emissor IR
const int buttonPin18 = 5;  // Pino do botão D18
const int buttonPin19 = 18;  // Pino do botão D19
const int buttonPin21 = 19;  // Pino do botão D21
const int relayPin = 23;      // Pino do relé
const int receiverPin = 25;  // Pino D22 para o receptor IR
bool projectorOn = false;
bool soundbarOn = false;
int lightingMode = 0;  // 0: Desligado, 1: Modo 1, 2: Modo 2
int button21Clicks = 0;
unsigned long lastDebounceTime = 0;
unsigned long debounceDelay = 500;

IRsend irsend(IRemitterPin);
IRrecv irrecv(receiverPin);
decode_results results;

LiquidCrystal_I2C lcd(0x27, 16, 2); 

void messageHandler(String &topic, String &payload);

WiFiClientSecure net = WiFiClientSecure();
MQTTClient client = MQTTClient(256);

void connectAWS()
{
  WiFi.mode(WIFI_STA);
  WiFi.begin("RedmiNote", "Gportel@");

  Serial.println("Connecting to Wi-Fi");

  while (WiFi.status() != WL_CONNECTED){
    delay(500);
    Serial.print(".");
  }

  // Configure WiFiClientSecure to use the AWS IoT device credentials
  net.setCACert(AWS_CERT_CA);
  net.setCertificate(AWS_CERT_CRT);
  net.setPrivateKey(AWS_CERT_PRIVATE);

  // Connect to the MQTT broker on the AWS endpoint we defined earlier
  client.begin(AWS_IOT_ENDPOINT, 8883, net);

  // Create a message handler
  client.onMessage(messageHandler);

  Serial.print("Connecting to AWS IOT");

  while (!client.connect(THINGNAME)) {
    Serial.print("trying to connect");
    delay(100);
  }

  if(!client.connected()){
    Serial.println("AWS IoT Timeout!");
    return;
  }

  // Subscribe to a topic
  client.subscribe(AWS_IOT_SUBSCRIBE_TOPIC);

  Serial.println("AWS IoT Connected!");
}

void messageHandler(String &topic, String &payload) {
  Serial.println("incoming: " + topic + " - " + payload);

//  StaticJsonDocument<200> doc;
//  deserializeJson(doc, payload);
//  const char* message = doc["message"];
}

void Publish (const char* topic, const char* message){
  StaticJsonDocument<200> doc;
  doc["time"] = millis();
  doc["set"] = message;
  char jsonBuffer[512];
  serializeJson(doc, jsonBuffer); // print to client
  client.publish(topic, jsonBuffer);

}
void PrintSerialLcd(const char* message) {

  // Imprime na porta serial
  Serial.println(message);

  // Imprime no display LCD
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(message);
}

void setup() {
  Serial.begin(9600);
  connectAWS();
  pinMode(buttonPin18, INPUT);
  pinMode(buttonPin19, INPUT);
  pinMode(buttonPin21, INPUT);
  pinMode(relayPin, OUTPUT);
  digitalWrite(relayPin, LOW);  // Inicialmente, o relé está desligado
  irsend.begin();  // Inicializa o emissor IR
  irrecv.enableIRIn();  // Inicializa o receptor IR
  Wire.begin();
  lcd.begin(16, 2); // Inicialize o display com 16 co
  lcd.setBacklight(LOW); // Desligue o backlight
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Aguardando sinal IR...");
}

void loop() {
  
  // String minhaString = AWS_IOT_SUBSCRIBE_TOPIC;
  // String minhaString2 = "payload";
  // messageHandler(minhaString, minhaString2);
  client.loop();

  if (digitalRead(buttonPin18) == HIGH && (millis() - lastDebounceTime) > debounceDelay) {
    if (projectorOn) {
      PrintSerialLcd("Desligando Projetor");
      Publish(AWS_IOT_PUBLISH_TOPIC_PROJECTOR, "0");
      irsend.sendNEC(0xE4780854, 32, 2); // Substitua o código com o código IR correto
      projectorOn = false;
    } else {
      PrintSerialLcd("Ligando Projetor");
      Publish(AWS_IOT_PUBLISH_TOPIC_PROJECTOR, "1");
      irsend.sendNEC(0xE1BA5140, 32, 2); // Substitua o código com o código IR correto
      projectorOn = true;
    }
    lastDebounceTime = millis();
  }

  // Botão D19
  if (digitalRead(buttonPin19) == HIGH && (millis() - lastDebounceTime) > debounceDelay) {
    if (soundbarOn) {
      PrintSerialLcd("Desligando Soundbar");
      Publish(AWS_IOT_PUBLISH_TOPIC_SOUNDBAR, "0");
      irsend.sendNEC(0xCF000EF1, 32, 2);
      soundbarOn = false;
    } else {
      PrintSerialLcd("Ligando Soundbar");
      Publish(AWS_IOT_PUBLISH_TOPIC_SOUNDBAR, "1");
      irsend.sendNEC(0xCF000EF1, 32, 2);
      soundbarOn = true;
    }
    lastDebounceTime = millis();
  }

  // Botão D21
  if (digitalRead(buttonPin21) == HIGH && (millis() - lastDebounceTime) > debounceDelay) {
    button21Clicks++;
    if (button21Clicks == 1) {
      Publish(AWS_IOT_PUBLISH_TOPIC_LIGHT, "1");
      PrintSerialLcd("Ligando Iluminação (Modo 1)");
      lightingMode = 1;
      digitalWrite(relayPin, HIGH);  // Liga o relé
    } else if (button21Clicks == 2) {
      Publish(AWS_IOT_PUBLISH_TOPIC_LIGHT, "2");
      PrintSerialLcd("Ligando Iluminação (Modo 2)");
      lightingMode = 2;
      digitalWrite(relayPin, HIGH);
    } else if (button21Clicks == 3) {
      Publish(AWS_IOT_PUBLISH_TOPIC_LIGHT, "0");
      PrintSerialLcd("Desligando Iluminação");
      lightingMode = 0;
      digitalWrite(relayPin, LOW);  // Desliga o relé
      button21Clicks = 0;
    }
    lastDebounceTime = millis();
  }

  // Receptor IR
  if (irrecv.decode(&results)) {
    // Imprime o código IR capturado na porta serial
    Serial.print("Código IR: 0x");
    Serial.println(results.value, HEX);

    irrecv.resume();  // Continue a aguardar outros códigos IR
  }
}