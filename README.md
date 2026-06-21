# SmartLock
Projeto de Microcontroladores — SmartLock

# Descrição
Sistema de controle de acesso a um ambiente (sala,
laboratório, catraca simulada), com o ESP32. O usuário digita uma senha de 4 dígitos no
teclado matricial 4x4. O display LCD 16x2 exibe o status em tempo real. O ESP32 publica
eventos de acesso via MQTT e recebe comandos remotos. O FreeRTOS gerencia quatro
tarefas concorrentes com requisitos de tempo real.

# Componentes
1 - LCD  16X2 (I2C)  
1 - Keypad 4x4  
1 - LED Verde  
1 - Led Vermelho  
2 - Esp32 DevKit c  
1 - Protoboard 830 pontos  
2 - Resistores de 220 Ohms  

# Esquemático do Circuito
<img width="1339" height="678" alt="Captura de tela 2026-06-21 180326" src="https://github.com/user-attachments/assets/5014562d-c082-40d9-8331-360e78185249" />  

# Ligações do circuito

---

## ESP32 2 – LED Verde

- Anodo (perna maior) → saída do resistor de 220Ω  
- Resistor 220Ω → GPIO 02  
- Catodo (perna menor) → GND  

---

## ESP32 2 – LED Vermelho

- Anodo (perna maior) → saída do resistor de 220Ω  
- Resistor 220Ω → GPIO 04  
- Catodo (perna menor) → GND  

---

## ESP32 2 – LCD 16x2 (I2C)

- SDA → GPIO 21  
- SCL → GPIO 22  
- GND → GND  
- VCC → 3V3  

---

## ESP32 1 – Keypad 4x4

- R1 → GPIO 23  
- R2 → GPIO 22  
- R3 → GPIO 21  
- R4 → GPIO 19  

- C1 → GPIO 02  
- C2 → GPIO 04  
- C3 → GPIO 05  
- C4 → GPIO 18  

---

## Conexão UART entre os Microcontroladores

- ESP32 1 GPIO 17 → ESP32 2 GPIO 16  
- ESP32 1 GPIO 16 → ESP32 2 GPIO 17  
- GND ESP32 1 → GND ESP32 2  

---

# Tecnologias de Comunicação Utilizadas

O sistema SmartLock utiliza diferentes protocolos de comunicação para garantir integração entre os módulos e conectividade.

---

## Wi-Fi
Utilizado para conexão do sistema com a rede local.

- Permite comunicação com serviços externos
- Base para o funcionamento do MQTT
- Usado para conectividade do ESP32

---

## MQTT
Protocolo de mensageria leve baseado em publish/subscribe.

- Usado para envio de mensagens entre dispositivos
- Comunicação com broker
- Ideal para IoT devido ao baixo consumo

---

## UART
Comunicação serial entre os dois microcontroladores ESP32.

- ESP32 1 ↔ ESP32 2
- Transmissão de comandos e dados do sistema
- Alta velocidade e baixa latência

---

## I2C
Usado para comunicação com o display LCD 16x2.

- Protocolo de dois fios (SDA e SCL)
- Permite controle simplificado do display
- Utiliza endereço de dispositivo

---

## Resumo da Arquitetura

- Wi-Fi → conecta o sistema à rede  
- MQTT → comunicação em nuvem/local broker  
- UART → comunicação entre ESPs  
- I2C → controle do display LCD  

# Configuração MQTT

O SmartLock utiliza o protocolo MQTT para comunicação entre o ESP32 e o broker MQTT através da rede Wi-Fi.

Após conectar-se à rede, o ESP32 estabelece conexão com o broker e passa a publicar eventos do sistema e receber configurações remotamente.

---

## Tópicos MQTT Utilizados

### smartlock/eventos

Tópico utilizado para publicar eventos relacionados ao controle de acesso.

**Direção:** ESP32 → Broker

Exemplo:

```json
{
  "status": "OK",
  "ts": 1720000000
}
```

Campos:

- `status`: resultado da operação realizada
- `ts`: timestamp do evento

---

### smartlock/config

Tópico utilizado para receber configurações remotas.

**Direção:** Broker → ESP32

Exemplo:

```json
{
  "nova_senha": "5678"
}
```

Campos:

- `nova_senha`: nova senha cadastrada no sistema

---

### smartlock/heartbeat

Tópico utilizado para monitoramento do dispositivo.

**Direção:** ESP32 → Broker

Exemplo:

```json
{
  "uptime_s": 3600
}
```

Campos:

- `uptime_s`: tempo de funcionamento do ESP32 em segundos

---

## Fluxo de Comunicação

1. O ESP32 conecta-se à rede Wi-Fi.
2. O ESP32 conecta-se ao broker MQTT.
3. Eventos de acesso são publicados em `smartlock/eventos`.
4. Configurações remotas são recebidas por `smartlock/config`.
5. Periodicamente o dispositivo envia mensagens de monitoramento para `smartlock/heartbeat`.

---

## Benefícios do MQTT no Projeto

- Comunicação leve e eficiente
- Baixo consumo de banda
- Monitoramento remoto do sistema
- Atualização remota de configurações
- Fácil integração com aplicações IoT

# FreeRTOS

O sistema SmartLock foi desenvolvido utilizando o FreeRTOS, permitindo a execução concorrente de quatro tarefas independentes. Cada tarefa possui uma responsabilidade única, garantindo organização, modularidade e melhor gerenciamento dos recursos do sistema.

---

## Tarefas Implementadas

### 1. KeypadTask

Responsável pela leitura dos caracteres recebidos do teclado matricial através da comunicação UART.

**Funções:**
- Monitorar a porta Serial2.
- Receber os caracteres digitados.
- Enviar os caracteres para a fila de processamento.

**Prioridade:** 3

---

### 2. AuthTask

Responsável pela autenticação da senha digitada.

**Funções:**
- Receber os caracteres da fila.
- Montar a senha digitada.
- Comparar a senha informada com a senha cadastrada.
- Determinar se o acesso será permitido ou negado.
- Notificar as demais tarefas após a autenticação.

**Prioridade:** 2

---

### 3. DisplayTask

Responsável pela interface visual do sistema.

**Funções:**
- Atualizar o display LCD.
- Exibir os dígitos digitados de forma mascarada.
- Informar sucesso ou falha na autenticação.
- Acionar os LEDs de indicação visual.

**Prioridade:** 1

---

### 4. MQTTTask

Responsável pela comunicação em rede.

**Funções:**
- Gerenciar a conexão Wi-Fi.
- Conectar ao broker MQTT.
- Publicar eventos de autenticação.
- Receber configurações remotas.
- Publicar mensagens de heartbeat.

**Prioridade:** 1

---

# Recursos FreeRTOS Utilizados

## Queue (Fila)

Foi utilizada uma fila para transferir os caracteres digitados da tarefa de leitura para a tarefa de autenticação.

```text
KeypadTask → filaDigitos → AuthTask
```

**Objeto utilizado:**

```cpp
QueueHandle_t filaDigitos;
```

**Finalidade:**
- Comunicação segura entre tarefas.
- Armazenamento temporário dos caracteres digitados.
- Evita perda de dados durante o processamento.

---

## Semaphore (Semáforo)

Foi utilizado um semáforo binário para sincronizar a atualização do display.

```text
AuthTask → semaforoDisplay → DisplayTask
```

**Objeto utilizado:**

```cpp
SemaphoreHandle_t semaforoDisplay;
```

**Finalidade:**
- Sinalizar quando o LCD deve ser atualizado.
- Evitar atualizações desnecessárias.
- Melhorar a sincronização entre tarefas.

---

## Event Group

Foi utilizado um grupo de eventos para informar à MQTTTask quando uma autenticação foi concluída.

```text
AuthTask → BIT_AUTH_PROCESSADA → MQTTTask
```

**Objeto utilizado:**

```cpp
EventGroupHandle_t grupoEventosMQTT;
```

**Evento utilizado:**

```cpp
#define BIT_AUTH_PROCESSADA (1 << 0)
```

**Finalidade:**
- Notificar a conclusão do processo de autenticação.
- Permitir a publicação de eventos MQTT apenas quando necessário.

---

# Configuração MQTT

O SmartLock utiliza o protocolo MQTT para comunicação remota através da rede Wi-Fi.

Após conectar-se à rede, o ESP32 estabelece conexão com o broker MQTT e passa a publicar eventos de autenticação, receber configurações remotas e enviar mensagens periódicas de monitoramento.

