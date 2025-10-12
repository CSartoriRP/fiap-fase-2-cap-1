# 🌱 FarmTech Solutions – Sistema de Irrigação Inteligente  
**FIAP – Fase 2, Capítulo 1 – Um Mapa do Tesouro**

---

## 🎯 Visão Geral

O projeto **FarmTech Solutions** é uma simulação completa de um **sistema de irrigação inteligente** para agricultura digital.  
Desenvolvido em etapas progressivas, ele combina **IoT (ESP32)**, **integração de dados meteorológicos (Python)** e **análise estatística (R)** — culminando em um pipeline completo de **Agricultura 4.0**.

A solução monitora variáveis como **umidade do solo**, **pH**, e **níveis de nutrientes (NPK)**, além de ajustar automaticamente a irrigação conforme a **probabilidade de chuva (POP)** e a **precipitação esperada (RAIN3H)**.

---

## 🧩 Estrutura do Projeto

| Etapa | Tecnologia | Descrição |
|:------|:------------|:-----------|
| 1️⃣ | **C++ / ESP32** | Simulação no [Wokwi](https://wokwi.com) com sensores (LDR, DHT22, potenciômetros) e relé da bomba d’água. |
| 2️⃣ | **Python** | Captura de dados meteorológicos em tempo real via API pública [OpenWeather](https://openweathermap.org/). |
| 3️⃣ | **R** | Análise estatística e visualização dos dados com regressão logística e correlação entre variáveis. |
| 4️⃣ | **HTML (RMarkdown)** | Geração de relatório visual consolidando os resultados da simulação e análise. |

---

## ⚙️ Arquitetura Completa

```
[ESP32 + Sensores (Wokwi)] 
         │
         ▼
 [Python - API OpenWeather]
         │
         ▼
 [R - Processamento e Modelagem]
         │
         ▼
 [HTML - Relatório Final]
```

---

## 🧱 Componentes e Arquivos

### 🔧 ESP32 (C++ / Arduino)
**Arquivos:**  
- `sketch.ino`  
- `diagram.json`

**Principais Funções:**
- Leitura de sensores simulados (LDR → pH, potenciômetros → NPK e umidade).  
- Controle da bomba com histerese (liga <55%, desliga >70%).  
- Comandos via Serial (`POP=`, `RAIN3H=`, `HOLD=`, `CLEAR`, `STATUS?`).  
- Registro contínuo dos dados (`logs.txt`).

---

### 🌦️ Python – Integração com OpenWeather

**Arquivos:**  
- `busca_clima.py` → coleta dados de previsão do tempo e gera `weather.csv`.  
- `gera_pop_rain.py` → lê `weather.csv`, extrai POP/RAIN3H e gera comandos para o Wokwi.  

**Exemplo de uso:**
```bash
python busca_clima.py --cities-file cities.csv --out .
python gera_pop_rain.py --city "Curitiba" --hours 3
```

**Saída:**  
`weather.csv` com previsões completas e POP/RAIN3H prontos para enviar ao ESP32.

---

### 💧 R – Análise Estatística e Visualização

**Arquivos:**  
- `data_analysis.R` → lê `logs.txt` e `weather.csv`, gera `irrigation_events.csv`.  
- `data_analysis.Rmd` → cria o relatório final `report.html`.  
- `report_final.html` → relatório visual autônomo com gráficos finais.  

**Gráficos gerados:**
- `bomba_vs_umidade.png` → histerese da irrigação.  
- `bomba_vs_pop_rain.png` → influência de POP e chuva.  
- `correlacao.png` → correlação entre variáveis.  

**Saídas:**  
- `irrigation_events.csv`  
- `weather_latest_by_city.csv`  
- `modelo_logistico.txt`  
- `report.html` ou `report_final.html`

---

### 📊 Relatório Final HTML

O relatório HTML integra os resultados de todas as etapas e apresenta os gráficos abaixo:

| Gráfico | Descrição |
|:--------|:-----------|
| ![Umidade](bomba_vs_umidade_ficticio.png) | Relação entre **umidade do solo** e acionamento da bomba. |
| ![POP/RAIN3H](bomba_vs_pop_rain_ficticio.png) | Influência da **probabilidade e volume de chuva** sobre a irrigação. |
| ![Correlação](correlacao_ficticio.png) | Correlação entre as principais variáveis do sistema. |

📄 **Acesse:** [Relatório Final – report_final.html](analise-integrada/report_final.html)

---

## 🧠 Lógica de Decisão (Resumo)

| Condição | Ação da Bomba | Observação |
|-----------|---------------|-------------|
| Umid < 55% e sem chuva prevista | 💧 **Liga** | Irrigação necessária |
| Umid > 70% | 📴 **Desliga** | Solo úmido o suficiente |
| POP ≥ 60% ou RAIN3H ≥ 1.0 | ☁️ **Desliga (HOLD ON)** | Chuva prevista, irrigação suspensa |
| HOLD=ON manual | ⏸️ **Pausa** | Bloqueio temporário via Serial |

---

## 🧪 Resultados de Simulação

- Bomba ligada em condições de baixa umidade (<55%).  
- Sistema suspende automaticamente quando há previsão de chuva (POP ≥ 60%).  
- HOLD automático evita reativações prematuras após detecção de chuva.  
- Regressão logística confirma correlação inversa entre **umidade** e acionamento.  

---

## 🧾 Estrutura de Pastas Recomendada

```
fiap-fase-2-cap-1/
│
├── sketch.ino
├── diagram.json
│
├── busca_clima.py
├── gera_pop_rain.py
│
├── data_analysis.R
├── data_analysis.Rmd
│
├── irrigation_events.csv
├── weather_latest_by_city.csv
├── logs.txt
│
├── report_final.html
│
├── bomba_vs_umidade_ficticio.png
├── bomba_vs_pop_rain_ficticio.png
├── correlacao_ficticio.png
│
└── README.md
```

---

## 🧬 Conclusão

O projeto **FarmTech Solutions** demonstra um **ciclo completo de IoT + Data Science**, aplicando:

- Sensores simulados no **ESP32 (C++)**  
- Aquisição de dados via **API pública (Python)**  
- Processamento e análise com **R**  
- Relatório interativo em **HTML**

🧭 Representa um exemplo prático de **Agricultura Inteligente** com integração entre hardware, software e ciência de dados aplicada.

---

## ✨ Créditos

**Desenvolvido por:**  
👤 *Cláudio Sartori*  
📚 FIAP – Pós Tech em Data Science & AI  

**Colaboração:**  
🧠 ChatGPT (OpenAI) como assistente técnico e de documentação.

---

## 🔗 Repositório GitHub

👉 [https://github.com/CSartoriRP/fiap-fase-2-cap-1](https://github.com/CSartoriRP/fiap-fase-2-cap-1)

---

© 2025 • FarmTech Solutions – FIAP • Todos os direitos reservados
