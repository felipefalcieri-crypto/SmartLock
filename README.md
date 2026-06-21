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
