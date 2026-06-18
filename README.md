# SmartLock
Projeto de Microcontroladores — SmartLock

# Descrição
istema de controle de acesso a um ambiente (sala,
laboratório, catraca simulada), com o ESP32. O usuário digita uma senha de 4 dígitos no
teclado matricial 4x4. O display LCD 16x2 exibe o status em tempo real. O ESP32 publica
eventos de acesso via MQTT e recebe comandos remotos. O FreeRTOS gerencia quatro
tarefas concorrentes com requisitos de tempo real.
