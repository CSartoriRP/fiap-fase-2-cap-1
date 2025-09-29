🌱 Projeto FIAP – Irrigação Inteligente com ESP32

Este projeto foi desenvolvido no curso da FIAP como parte das atividades práticas de IoT.
O objetivo é simular um sistema de irrigação inteligente controlado por um ESP32, sensores e atuadores, permitindo ligar e desligar automaticamente a bomba d’água de acordo com:

Disponibilidade de nutrientes (N, P, K)

Faixa de pH da solução nutritiva

Nível de umidade do solo

⚙️ Funcionalidades

Botões N, P, K: simulam a presença de nutrientes.

LDR: usado como entrada analógica para simular valores de pH (0–14).

DHT22: mede temperatura e umidade relativa do ar (simulando a umidade do solo).

Relé: aciona a bomba d’água (simulada com LED no Wokwi).

Controle com histerese:

Liga a bomba se a umidade < 55%

Desliga a bomba se a umidade > 70%

🔌 Hardware utilizado

ESP32 DevKit v1

Sensor DHT22 (umidade/temperatura)

LDR (simulação de pH)

3 botões (pushbutton) – nutrientes N, P, K

Módulo Relé 1 canal (LED vermelho no Wokwi)

Potenciômetro (opcional, para simular variação da umidade em tempo real)

🖥️ Simulação no Wokwi

O projeto pode ser testado integralmente no simulador online Wokwi.
No diagrama (diagram.json):

GPIO 12 → Botão N

GPIO 13 → Botão P

GPIO 14 → Botão K

GPIO 15 → DHT22 (pino DAT)

GPIO 25 → Relé (bomba)

GPIO 34 → LDR (simulação pH)

GPIO 35 → Potenciômetro (opcional para simular umidade)

📂 Estrutura do projeto
/ (repositório)
├── sketch.ino        # Código principal do projeto (Arduino/ESP32)
├── diagram.json      # Diagrama do circuito para simulação no Wokwi
└── README.md         # Documentação do projeto

▶️ Como executar

Abra o projeto no Wokwi
.

Cole o conteúdo de sketch.ino.

Substitua o diagram.json pelo diagrama fornecido.

Clique em Play ▶️ para iniciar a simulação.

Abra o Serial Monitor para acompanhar as leituras.

👨‍💻 Autor

Projeto desenvolvido por Claudio Sartori durante o curso FIAP.
Objetivo: aprendizado em IoT, sensores e atuadores com ESP32.
