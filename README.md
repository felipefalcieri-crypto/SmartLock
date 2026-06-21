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
<img width="1339" height="678" alt="Captura de tela 2026-06-21 180326" src="https://github.com/user-attachments/assets/5014562d-c082-40d9-8331-360e78185249" />
