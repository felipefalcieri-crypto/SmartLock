#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h> 

// --- Configurações de Hardware ---
#define LED_VERDE 4
#define LED_VERMELHO 2   

LiquidCrystal_I2C lcd(0x27, 16, 2);

// Configurações de Rede 
const char* ssid = "iPhone";
const char* password = "12345678";
const char* mqtt_server = "broker.hivemq.com"; 

WiFiClient espClient;
PubSubClient client(espClient);

// Variáveis Globais
char senhaCorreta[5] = "1234"; 
bool ultimoResultadoAuth = false; 
char digitosMascarados[5] = "    "; 
uint8_t digitosDigitados = 0;       

// Identificadores
QueueHandle_t filaDigitos;
SemaphoreHandle_t semaforoDisplay;
EventGroupHandle_t grupoEventosMQTT;

bool atualizarApenasDigitos = false; 

#define BIT_AUTH_PROCESSADA (1 << 0)

void KeypadTask(void *pvParameters);
void AuthTask(void *pvParameters);
void DisplayTask(void *pvParameters);
void MQTTTask(void *pvParameters);


// Callback do MQTT: Recebe a nova senha em formato JSON

void mqttCallback(char* topic, byte* payload, unsigned int length) {
  if (strcmp(topic, "smartlock/config") == 0) {

    // Criamos um documento JSON dinâmico para analisar a mensagem recebida
    DynamicJsonDocument doc(256);
    DeserializationError error = deserializeJson(doc, payload, length);

    if (!error) {

      // Verifica se a chave "nova_senha" existe no JSON recebido
      if (doc.containsKey("nova_senha")) {
        const char* novaSenhaMqtt = doc["nova_senha"];
        
        if (strlen(novaSenhaMqtt) == 4) { // Valida se tem 4 dígitos
          strncpy(senhaCorreta, novaSenhaMqtt, 4);
          senhaCorreta[4] = '\0';
          Serial.print("Nova senha configurada via JSON: ");
          Serial.println(senhaCorreta);
        }
      }
    } else {
      Serial.print("Erro ao decodificar JSON recebido: ");
      Serial.println(error.c_str());
    }
  }
}

void setup() {
  Serial.begin(9600);
  delay(500); // Pequena pausa para estabilizar o Serial Monitor
  
  Serial.println("\n--- TESTE DE CONEXAO WI-FI ---");
  
  Serial2.begin(9600, SERIAL_8N1, 16, 17); // RX=16, TX=17

  pinMode(LED_VERDE, OUTPUT);
  pinMode(LED_VERMELHO, OUTPUT);
  digitalWrite(LED_VERDE, LOW);
  digitalWrite(LED_VERMELHO, LOW);

  lcd.init();
  lcd.backlight();
  lcd.clear();
  lcd.print("Conectando WiFi...");

  // Teste WIFI
  Serial.print("Tentando conectar a rede: ");
  Serial.println(ssid);
  
  WiFi.begin(ssid, password);

  // Fica em loop enquanto não conectar
  int tentativas = 0;
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
    tentativas++;
    
    // Se passar de 20 tentativas (10 segundos), avisa que falhou
    if (tentativas > 20) {
      Serial.println("\n[ERRO] Nao foi possivel conectar. Verifique o SSID/Senha.");
      lcd.clear();
      lcd.print("Erro no Wi-Fi!");
      break; 
    }
  }

  // Se conectou com sucesso
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\n[SUCESSO] Wi-Fi Conectado!");
    Serial.print("IP obtido: ");
    Serial.println(WiFi.localIP()); // Mostra o IP
    
    lcd.clear();
    lcd.print("WiFi: OK!");
    delay(2000); // Deixa a mensagem na tela por 2 segundos
  }

  lcd.clear();
  lcd.print("Digite:");

  // Inicialização dos Primitivos do FreeRTOS
  filaDigitos = xQueueCreate(10, sizeof(char));
  semaforoDisplay = xSemaphoreCreateBinary();
  grupoEventosMQTT = xEventGroupCreate();

  // Criação das Tarefas
  xTaskCreate(KeypadTask, "KeypadTask", 2048, NULL, 3, NULL);
  xTaskCreate(AuthTask, "AuthTask", 2048, NULL, 2, NULL);
  xTaskCreate(DisplayTask, "DisplayTask", 2048, NULL, 1, NULL);
  xTaskCreate(MQTTTask, "MQTTTask", 4096, NULL, 1, NULL);
}

void loop() {
  vTaskDelay(pdMS_TO_TICKS(1000));
}

void KeypadTask(void *pvParameters) {
  TickType_t xLastWakeTime = xTaskGetTickCount();
  const TickType_t xFrequency = pdMS_TO_TICKS(20); 

  for (;;) {
    vTaskDelayUntil(&xLastWakeTime, xFrequency);
    while (Serial2.available() > 0) {
      char c = Serial2.read();
      if ((c >= '0' && c <= '9') || c == 'A' || c == 'B' || c == 'C' || c == 'D' || c == '*' || c == '#') {
        xQueueSend(filaDigitos, &c, 0);
      }
    }
  }
}

void AuthTask(void *pvParameters) {
  char senhaDigitada[5] = {0};
  char digitoRecebido;

  for (;;) {
    if (xQueueReceive(filaDigitos, &digitoRecebido, portMAX_DELAY) == pdTRUE) {
      
      if (digitoRecebido == '*') {
        digitosDigitados = 0;
        strcpy(digitosMascarados, "    ");
        atualizarApenasDigitos = true;
        xSemaphoreGive(semaforoDisplay);
        memset(senhaDigitada, 0, sizeof(senhaDigitada));
        continue;
      }

      if (digitosDigitados < 4) {
        senhaDigitada[digitosDigitados] = digitoRecebido;
        digitosMascarados[digitosDigitados] = '*'; 
        digitosDigitados++;
        digitosMascarados[digitosDigitados] = '\0';
        atualizarApenasDigitos = true;
        xSemaphoreGive(semaforoDisplay);
      }

      if (digitosDigitados == 4) {
        senhaDigitada[4] = '\0'; 

        if (strcmp(senhaDigitada, senhaCorreta) == 0) {
          ultimoResultadoAuth = true;  
        } else {
          ultimoResultadoAuth = false; 
        }

        atualizarApenasDigitos = false;
        xSemaphoreGive(semaforoDisplay);
        xEventGroupSetBits(grupoEventosMQTT, BIT_AUTH_PROCESSADA);

        digitosDigitados = 0;
        strcpy(digitosMascarados, "    ");
        memset(senhaDigitada, 0, sizeof(senhaDigitada));
      }
    }
  }
}

void DisplayTask(void *pvParameters) {
  for (;;) {
    if (xSemaphoreTake(semaforoDisplay, portMAX_DELAY) == pdTRUE) {
      if (atualizarApenasDigitos) {
        lcd.setCursor(0, 1);
        lcd.print("    "); 
        lcd.setCursor(0, 1);
        lcd.print(digitosMascarados); 
      } 
      else {
        lcd.clear();
        if (ultimoResultadoAuth) {
          lcd.setCursor(0, 0);
          lcd.print("Acesso permitido");
          digitalWrite(LED_VERDE, HIGH);    
        } else {
          lcd.setCursor(0, 0);
          lcd.print("Senha incorreta");
          digitalWrite(LED_VERMELHO, HIGH); 
        }

        for (int i = 3; i > 0; i--) {
          lcd.setCursor(0, 1);
          lcd.print("Limpando em: ");
          lcd.print(i);
          vTaskDelay(pdMS_TO_TICKS(1000)); 
        }

        digitalWrite(LED_VERDE, LOW);
        digitalWrite(LED_VERMELHO, LOW);
        
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Digite:");
      }
    }
  }
}

// MQTTTask Atualizada: Envia JSON para smartlock/eventos e smartlock/heartbeat
void MQTTTask(void *pvParameters) {
  WiFi.begin(ssid, password);
  client.setServer(mqtt_server, 1883);
  client.setCallback(mqttCallback);

  // Inicializa o temporizador para o Heartbeat
  unsigned long ultimoHeartbeat = 0;

  for (;;) {
    if (WiFi.status() != WL_CONNECTED) {
      vTaskDelay(pdMS_TO_TICKS(2000));
      continue;
    }
    if (!client.connected()) {
      if (client.connect("ESP32_SmartLock_Client")) {
        client.subscribe("smartlock/config"); 
      } else {
        vTaskDelay(pdMS_TO_TICKS(5000));
        continue;
      }
    }
    client.loop();

    //(smartlock/eventos)
    EventBits_t bits = xEventGroupWaitBits(grupoEventosMQTT, BIT_AUTH_PROCESSADA, pdTRUE, pdFALSE, pdMS_TO_TICKS(10)); 
    if ((bits & BIT_AUTH_PROCESSADA) != 0) {
      
      StaticJsonDocument<128> docEvento;
      
      if (ultimoResultadoAuth) {
        docEvento["status"] = "OK";
      } else {
        docEvento["status"] = "ERRO"; // Adapte se o seu backend exigir outra palavra
      }
      
      docEvento["ts"] = 1720000000; 

      char bufferJson[128];
      serializeJson(docEvento, bufferJson);
      
      // Publica no tópico correto
      client.publish("smartlock/eventos", bufferJson);
    }

    //(smartlock/heartbeat) - A cada 30 segundos
    if (millis() - ultimoHeartbeat >= 30000) {
      ultimoHeartbeat = millis();

      StaticJsonDocument<128> docHeartbeat;
      // uptime_s calcula há quantos segundos o ESP32 está ligado
      docHeartbeat["uptime_s"] = millis() / 1000; 

      char bufferHeartbeat[128];
      serializeJson(docHeartbeat, bufferHeartbeat);

      client.publish("smartlock/heartbeat", bufferHeartbeat);
    }

    vTaskDelay(pdMS_TO_TICKS(50)); 
  }
}