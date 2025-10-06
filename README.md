# 🌱 Projeto FIAP – Fase 2, Capítulo 1: *Um Mapa do Tesouro*

Descrição do Projeto:

Considerando como base a Fase anterior do projeto — que envolveu o cálculo de área plantada, monitoramento climático, entre outros —, a Fase 2 vai avançar no sistema de gestão agrícola da empresa FarmTech Solutions usando um dispositivo construído por você e seu grupo.

Agora, vamos imaginar em como podemos conectar os sensores físicos para otimizar a irrigação agrícola e criar um sistema de irrigação inteligente. Toda cultura agrícola depende, em maior ou menor proporção, de três elementos químicos: Nitrogênio (N), Fósforo (P) e Potássio (K) — o famoso NPK. Isso vai influenciar o pH da terra e, obviamente, a produtividade daquela planta. Além disso, é preciso considerar a umidade do solo, que indica o quanto choveu em um determinado período observado. Infelizmente, no Wokwi.com — plataforma onde simulamos projetos ESP32 — não há sensores exclusivamente agrícolas. Por isso, faremos simulações e algumas substituições didáticas.

No lugar dos sensores de nutrientes N, P e K, utilizaremos um botão verde em cada. Portanto, seu projeto precisa ter três botões simulando os níveis de cada elemento.

No lugar do sensor de pH, utilizaremos um sensor de intensidade de luz chamado LDR (Light Dependent Resistor) que trará dados analógicos da intensidade da luz, mas, para fins de simulação, vamos assumir que ele representa o pH da terra. Como referência, podemos comparar os dados analógicos do pH que variam de 0 a 14, sendo próximo de 7, pH neutro. Você também pode adotar outras escalas maiores que 0 a 14 para melhorar sua mecânica ao manipular o sensor LDR.

Quanto ao sensor de umidade, este possui um similar no Wokwi que mede a umidade do ar. Portanto, vamos adotar o DHT22 como medidor de umidade do solo (embora seja do ar na prática).

O objetivo do projeto na Fase 2 será desenvolver um sistema de irrigação automatizado e inteligente que monitore a umidade do solo em tempo real, os níveis dos nutrientes N, P e K representados por botões (que vão “medir” os níveis como tudo ou nada, isto é, “true” ou “false”, ou em outras palavras, como botão pressionado ou não pressionado).  

Quando você mexer nos botões e alterar os níveis do NPK, você deve mexer no sensor pH representado pelo sensor LDR, pois, em tese, você estaria alterando o pH da terra.



Além disso, o sensor de umidade DHT22 vai decidir se precisa ligar ou não a irrigação conforme o necessário — isto é, ligando um relé azul representando uma bomba d’água de verdade.

Você e seu grupo podem pensar em irrigar uma lavoura (ligando o relé azul) de acordo com a combinação dos níveis de N, P, K, pH e umidade que desejarem, bastando para isso, escolher uma cultura agrícola e pesquisar quais suas reais necessidades de nutrientes em uma fazenda. A lógica de decisão de quando ligar ou desligar a bomba d’água é do grupo. Só precisa documentar isso na entrega.

 

Ir além – Integração Python com API Pública (opcional 1):

Além do controle básico de irrigação com base nos sensores já mencionados, você e seu grupo podem integrar dados meteorológicos obtidos de uma API pública, como a OpenWeather, para prever condições climáticas adversas e ajustar a irrigação automaticamente.

Por exemplo, se houver previsão de chuva, o sistema pode suspender a irrigação para economizar recursos. A integração entre Python e ESP32 do Wokwi.com não é trivial no plano gratuito. Caso não obtenha sucesso, transfira os dados manualmente entre os sistemas C/C++ e Python, copiando os dados dos resultados da API que conseguiu usando o Python para uma variável qualquer no código C/C++ do ESP32 no Wokwi, a qual indicará o nível de chuva na sua lógica de irrigação.

Caso encontre uma forma automática para resolver isso, melhor ainda! Uma opção é ler caracteres via Monitor Serial do simulador ESP32, onde você pode inserir dados via tela do seu computador com o seu teclado enquanto o código está rodando. Basta explorar as funções Serial.available() e Serial.read().

 

Ir além – Análise em R (opcional 2):

O grupo pode tentar implementar uma análise estatística em R qualquer — seja as apresentadas até o momento ou buscando nas referências da disciplina de R — para decidir se deve ligar ou não a bomba de irrigação (que é o relé azul). Isso lhe trará conhecimento de Data Science, um cargo que é bastante procurado no mercado de trabalho.


----------------------------------------------------------------------------------------------------------------------------------------------------------------


Este repositório reúne **três versões evolutivas** do mesmo projeto de irrigação inteligente com **ESP32**.  
Cada versão tem seu próprio `sketch.ino` e `diagram.json` (Wokwi), permitindo comparar decisões de projeto e resultados.

> **Resumo da evolução:** do controle básico com entradas binárias para uma simulação mais realista, com **entradas analógicas contínuas** para NPK e umidade, e regras de bomba com histerese e limiares configuráveis.

---

## 📁 Estrutura do repositório

| Pasta | Foco | O que mudou |
|---|---|---|
| `projeto_base/` | Fundamentos | Primeira versão funcional: leitura de sensores, NPK por **botões** (on/off), relé da bomba e histerese de umidade. |
| `humidade-potenciometro/` | Umidade ajustável | Adiciona **potenciômetro** para simular umidade via ADC (`0–4095 → 0–100%`). Mantém DHT22 para ambiente. |
| `botoes-npk-dinamicos/` | NPK dinâmico | Substitui os botões por **3 potenciômetros lineares (sliders)** para N, P e K (valores contínuos). Regras passam a usar limiares de NPK. |

> Obs.: os nomes das pastas refletem o histórico da disciplina; a pasta `botoes-npk-dinamicos` usa **sliders analógicos** (não mais botões).

---

## 🔧 Versões em detalhe

### 1) `projeto_base` — Fundamentos
- **Sensores/atuadores:** DHT22 (umidade/temperatura), LDR (pH simulado), 3 botões (N, P, K), 1 relé (bomba).  
- **Lógica:** liga/desliga da bomba por **histerese de umidade**; validações de pH e de NPK em nível **binário**.  
- **O que mudou nesta etapa:** criação da base de leitura estável + acionamento do relé e logs via Serial.

---

### 2) `humidade-potenciometro` — Umidade ajustável em tempo real
- **Mudança principal:** adiciona **potenciômetro** ligado a um canal **ADC** do ESP32 para simular a umidade do solo.  
- **Mapeamento típico:** `0–4095` → `0–100%`.  
- **Benefício:** permite testar a histerese girando o knob e observar quando a bomba liga/desliga, sem depender das variações ambientais do DHT22.  
- **O que mudou em relação ao `projeto_base`:** a decisão de irrigar passa a responder a uma entrada analógica contínua de “umidade do solo” (mantendo pH/NPK como estavam).

---

### 3) `botoes-npk-dinamicos` — NPK com sliders analógicos
- **Mudança:** N, P e K passam a ser **valores contínuos**, lidos por **três potenciômetros lineares** (sliders) conectados a entradas analógicas.  
- **Leitura típica:** `porcentagem = map(analogRead(pin), 0, 4095, 0, 100)`.  
- **Regra de irrigação (exemplo):** irrigar somente se **(umidade < LIMIAR)** **e** `N ≥ LIMIAR_N` **e** `P ≥ LIMIAR_P` **e** `K ≥ LIMIAR_K` **e** `pH` dentro da faixa.  
- **O que mudou em relação a `humidade-potenciometro`:** além da umidade analógica, os nutrientes viram **entradas analógicas** com limiares individuais.

> Consulte cada `diagram.json` para o mapeamento exato de pinos no Wokwi/ESP32, pois a pinagem pode variar entre as versões.

---

## 🧷 Pinagem (referência de uso)

> **Importante:** os pinos abaixo são uma **referência**. Confirme sempre no `diagram.json` de cada pasta, que é a “fonte da verdade” do hardware no Wokwi.

| Componente | Sinal | Sugestão de pino ESP32 | Observações |
|---|---|---|---|
| DHT22 | DADOS | `GPIO15` | Requer biblioteca DHTesp; definir taxa de leitura (>2s). |
| Relé (IN) | Controle | `GPIO25` | Ajuste `RELAY_ACTIVE_HIGH` conforme seu módulo (active high/low). |
| LDR / Sensor pH simulado | ADC | `GPIO34` (ADC1_CH6) | Entrada **somente leitura**; mapear para faixa de pH. |
| Pot. Umidade (knob) | ADC | `GPIO33` (ADC1_CH5) | `0–4095 → 0–100%`. |
| Slider N | ADC | `GPIO32` (ADC1_CH4) | `0–4095 → 0–100%`. |
| Slider P | ADC | `GPIO35` (ADC1_CH7) | `0–4095 → 0–100%`. |
| Slider K | ADC | `GPIO36`/`VP` (ADC1_CH0) | `0–4095 → 0–100%`. |

> **Dica:** prefira os canais **ADC1** (GPIOs 32–39) para evitar conflitos com o Wi-Fi (que usa ADC2).

---

## 🎚️ Limiares e configuração (sugestão)

Ajuste no topo do `sketch.ino` de cada versão conforme sua necessidade:

```cpp
// Histerese de umidade do solo
#define HUM_ON   55    // (%) liga a bomba quando abaixo deste valor
#define HUM_OFF  70    // (%) desliga quando acima deste valor

// Faixa de pH aceitável
#define PH_MIN   5.8
#define PH_MAX   6.5

// Limiar de nutrientes (versão NPK dinâmico)
#define N_MIN    60    // (%)
#define P_MIN    60    // (%)
#define K_MIN    60    // (%)

// Ajustes elétricos / hardware
#define RELAY_ACTIVE_HIGH true   // Se seu relé for ativo em LOW, mudar para false
