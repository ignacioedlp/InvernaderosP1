#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>
#include <ArduinoJson.h>
#include <DHT.h>

// Configuración de red WiFi
const char* ssid = "Wokwi-GUEST";
const char* password = "";
const char* chatId = "tu_chat_id";

// Configuración del bot de Telegram
#define BOT_TOKEN "7211031772:AAEeW7VskINFGisF3GMdapLfyyOi3YjbWis"
WiFiClientSecure client;
UniversalTelegramBot bot(BOT_TOKEN, client);

// Pin del LED integrado
const int ledPin = 2;

// Configuración del sensor de temperatura
#define DHTPIN 4     // Pin donde está conectado el DHT11/22
#define DHTTYPE DHT22   // Cambia a DHT11 si estás usando ese modelo
DHT dht(DHTPIN, DHTTYPE);

// Variables de control
float temperature;
float humidity;
bool windowsOpen = false;
unsigned long lastTempCheck = 0;
unsigned long tempCheckInterval = 30000; // 5 minutos en milisegundos

// Función para leer temperatura y humedad del sensor DHT
void readTemperatureAndHumidity() {
  temperature = dht.readTemperature();
  humidity = dht.readHumidity();

  if (isnan(temperature) || isnan(humidity)) {
    Serial.println("Error leyendo del sensor DHT");
  }
}

// Función para controlar el LED (motor)
void controlMotor(bool open) {
  digitalWrite(ledPin, HIGH);
  delay(10000); // Simula 10 segundos de operación del motor
  digitalWrite(ledPin, LOW);
  windowsOpen = open;
}

// Función para enviar notificaciones al bot
void sendNotification(String message) {
  bot.sendMessage(chatId, message, "");
}

// Setup
void setup() {
  Serial.begin(115200);
  WiFi.begin(ssid, password, 6); // Wokwi-GUEST es una red de prueba, cámbiala por la tuya
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }
  Serial.println(" Conectado!");

  pinMode(ledPin, OUTPUT);
  digitalWrite(ledPin, LOW);
  dht.begin();

  // Configuración del cliente seguro
  client.setCACert(TELEGRAM_CERTIFICATE_ROOT); // Necesario para SSL

  // Notificación de inicio
  sendNotification("Sistema de ventilación del invernadero iniciado.");
}

// Loop
void loop() {

  readTemperatureAndHumidity();

  if (temperature > 30 && !windowsOpen) {
    sendNotification("Temperatura: " + String(temperature) + "°. Iniciando apertura!");
    controlMotor(true);
  } else if (temperature < 20 && windowsOpen) {
    sendNotification("Temperatura: " + String(temperature) + "°. Iniciando cierre!");
    controlMotor(false);
  }
    
  // Lectura de temperatura cada 30 minutos
  if (millis() - lastTempCheck > tempCheckInterval) {
    lastTempCheck = millis();
    sendNotification("Temperatura: " + String(temperature) + "°. Ventanas " + (windowsOpen ? "abiertas" : "cerradas") + ".");
  }
  
  // Manejo de comandos de Telegram
  int numNewMessages = bot.getUpdates(bot.last_message_received + 1);
  for (int i = 0; i < numNewMessages; i++) {
    String text = bot.messages[i].text;
    if (text == "/abrir" && !windowsOpen) {
      sendNotification("Comando recibido: /abrir. Iniciando apertura!");
      controlMotor(true);
    } else if (text == "/cerrar" && windowsOpen) {
      sendNotification("Comando recibido: /cerrar. Iniciando cierre!");
      controlMotor(false);
    }
  }
}
