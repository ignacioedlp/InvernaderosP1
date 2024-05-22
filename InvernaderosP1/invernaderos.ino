#include <WiFi.h>
#include <UniversalTelegramBot.h>
#include <WiFiClientSecure.h>

// Configuraciones WiFi y Telegram
const char* ssid = "Personal-5B3-2.4GHz"; // Con tu SSID
const char* password = "9114DEA5B3"; // Tu contraseña

const char* telegramToken = "7126983538:AAEo66DMT49mDPm-Aa1_IMp3G8E_1w5cdGA";
const char* chatId = "924578095";

// Intervalo de envío de datos
unsigned long lastSendTime = 0;
const unsigned long sendInterval = 300000; // 5 minutos en milisegundos

// Pines
const int ledPin = 2; // LED integrado para simular el motor

WiFiClientSecure client;
UniversalTelegramBot bot(telegramToken, client);

bool ventanasAbiertas = false;

void setup() {
  Serial.begin(115200);
  pinMode(ledPin, OUTPUT);
  digitalWrite(ledPin, LOW);

  // Conexión WiFi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Conectando a WiFi...");
  }
  Serial.println("Conectado a WiFi");

  // Configuración del cliente seguro
  client.setCACert(TELEGRAM_CERTIFICATE_ROOT); // Necesario para SSL

  // Notificación de inicio
  bot.sendMessage(chatId, "Sistema de ventilación del invernadero iniciado.", "");
}

void loop() {
  // Simular la lectura del sensor de temperatura
  float temperatura = random(15, 35);

  // Verificar y controlar las ventanas
  if (temperatura > 30 && !ventanasAbiertas) {
    abrirVentanas();
  } else if (temperatura < 20 && ventanasAbiertas) {
    cerrarVentanas();
  }

  // Enviar datos regularmente
  if (millis() - lastSendTime > sendInterval) {
    enviarDatos(temperatura);
    lastSendTime = millis();
  }

  // Verificar comandos de Telegram
  int numNewMessages = bot.getUpdates(bot.last_message_received + 1);
  while (numNewMessages) {
    handleNewMessages(numNewMessages);
    numNewMessages = bot.getUpdates(bot.last_message_received + 1);
  }

  delay(1000);
}

void abrirVentanas() {
  digitalWrite(ledPin, HIGH);
  delay(10000); // Simula el tiempo de activación del motor
  digitalWrite(ledPin, LOW);
  ventanasAbiertas = true;
  bot.sendMessage(chatId, "Ventanas abiertas.", "");
}

void cerrarVentanas() {
  digitalWrite(ledPin, HIGH);
  delay(10000); // Simula el tiempo de activación del motor
  digitalWrite(ledPin, LOW);
  ventanasAbiertas = false;
  bot.sendMessage(chatId, "Ventanas cerradas.", "");
}

void enviarDatos(float temperatura) {
  String estadoVentanas = ventanasAbiertas ? "abiertas" : "cerradas";
  String mensaje = "Temperatura: " + String(temperatura) + "°C\n";
  mensaje += "Ventanas: " + estadoVentanas;
  bot.sendMessage("your_telegram_chat_id", mensaje, "");
}

void handleNewMessages(int numNewMessages) {
  for (int i = 0; i < numNewMessages; i++) {
    String chat_id = String(bot.messages[i].chat_id);
    String text = bot.messages[i].text;

    if (text == "/abrir") {
      abrirVentanas();
    } else if (text == "/cerrar") {
      cerrarVentanas();
    }
  }
}
