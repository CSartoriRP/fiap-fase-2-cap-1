// --- Código para Irrigação do Cultivo de Bananas --- 

#include <Arduino.h>

// ------------ CONFIGS RÁPIDAS ------------
#define RELAY_ACTIVE_HIGH true   // se seu relé ligar invertido, troque para false
#define PH_INVERT true           // se luz alta der pH baixo, troque para true

// Pinos (conforme seu diagrama)
const int PIN_LDR   = 34;   // LDR (pH simulado no ADC)
const int PIN_RELAY = 25;   // Relé (bomba)
const int PIN_POT = 32;     // Potenciômetro para simular umidade manualmente

// Botões NPK como pushbutton (vamos tratá-los como toggle por software)
const int BTN_PINS[3] = {12, 13, 14}; // N, P, K (nessa ordem)

// Estados/toggle dos botões
bool latched[3]         = {false, false, false}; // N, P, K travados
int  prevRead[3]        = {HIGH, HIGH, HIGH};    // leitura anterior (pull-up = HIGH em repouso)
unsigned long lastT[3]  = {0, 0, 0};             // debounce ms

// Alvos da cultura (Banana)
const float phMin = 5.5, phMax = 6.6;
const float umidadeLiga = 55.0;     // liga se umidade < 55%
const float umidadeDesliga = 70.0;  // desliga se umidade > 70%

bool bombaLigada = false;

// ---------- Utilidades ----------
inline float fmap(float x, float in_min, float in_max, float out_min, float out_max) {
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

float readPH() {
  const uint8_t S = 6;
  uint32_t acc = 0;
  for (uint8_t i = 0; i < S; i++) { acc += analogRead(PIN_LDR); delay(2); }
  float ph = fmap(acc / (float)S, 0.0, 4095.0, 0.0, 14.0);
  if (PH_INVERT) ph = 14.0 - ph;
  if (ph < 0) ph = 0; if (ph > 14) ph = 14;
  return ph;
}

// Leitura do potenciômetro suavizada e calibrada para umidade
float readSimulatedHumidity() {
  const int N = 10; // número de amostras para média
  float soma = 0.0;
  for (int i = 0; i < N; ++i) {
    soma += analogRead(PIN_POT);
    delay(1);
  }
  float media = soma / N;
  // Calibração da faixa do potenciômetro; ajuste se precisar baseado em medidas reais
  const float minADC = 200.0;   // mínimo obtido na prática
  const float maxADC = 3900.0;  // máximo obtido na prática
  float valorCalibrado = (media - minADC) * 100.0 / (maxADC - minADC);
  if (valorCalibrado < 0) valorCalibrado = 0;
  if (valorCalibrado > 100) valorCalibrado = 100;
  return valorCalibrado;
}

void setPump(bool on) {
  digitalWrite(PIN_RELAY, (RELAY_ACTIVE_HIGH ? (on ? HIGH : LOW) : (on ? LOW : HIGH)));
  bombaLigada = on;
}

// Atualiza 1 botão (toggle por borda de descida + debounce)
void updateButton(int i) {
  int r = digitalRead(BTN_PINS[i]);
  unsigned long now = millis();
  if (r != prevRead[i] && (now - lastT[i]) > 25) {
    lastT[i] = now;
    if (prevRead[i] == HIGH && r == LOW) latched[i] = !latched[i];
    prevRead[i] = r;
  }
}

void setup() {
  Serial.begin(115200);

  for (int i = 0; i < 3; i++) pinMode(BTN_PINS[i], INPUT_PULLUP);

  pinMode(PIN_RELAY, OUTPUT);
  setPump(false);

  pinMode(PIN_POT, INPUT);

  analogReadResolution(12);
  Serial.println("FarmTech - Irrigacao Inteligente para Banana (simulacao de umidade via potenciometro)");
}

void loop() {
  updateButton(0);
  updateButton(1);
  updateButton(2);

  float ph = readPH();
  float hum = readSimulatedHumidity();

  bool nutrientesOK = latched[0] && latched[1] && latched[2];
  bool phOK = (ph >= phMin && ph <= phMax);

  if (!nutrientesOK || !phOK) {
    setPump(false);
  } else {
    if (!bombaLigada && hum < umidadeLiga) setPump(true);
    if (bombaLigada && hum > umidadeDesliga) setPump(false);
  }

  static bool prevN = false, prevP = false, prevK = false, prevPump = false;
  static unsigned long t0 = 0;
  bool changed = (prevN != latched[0]) || (prevP != latched[1]) || (prevK != latched[2]) || (prevPump != bombaLigada);
  if (changed || millis() - t0 > 1500) {
    t0 = millis();
    Serial.print("NPK: ");
    Serial.print(latched[0] ? "N " : "n ");
    Serial.print(latched[1] ? "P " : "p ");
    Serial.print(latched[2] ? "K " : "k ");

    Serial.print(" | Nutrientes: ");
    if (nutrientesOK)
      Serial.print("OK");
    else {
      Serial.print("Incompletos (faltam: ");
      bool first = true;
      if (!latched[0]) {
        Serial.print("N");
        first = false;
      }
      if (!latched[1]) {
        if (!first) Serial.print(", ");
        Serial.print("P");
        first = false;
      }
      if (!latched[2]) {
        if (!first) Serial.print(", ");
        Serial.print("K");
      }
      Serial.print(")");
    }

    Serial.print(" | pH=");
    Serial.print(ph, 2);
    Serial.print(" (");
    Serial.print(phMin);
    Serial.print("-");
    Serial.print(phMax);
    Serial.print(")");
    Serial.print(" | Umid=");
    Serial.print(hum, 1);
    Serial.print("%");
    Serial.print(" | Bomba=");
    Serial.println(bombaLigada ? "ON" : "OFF");

    prevN = latched[0];
    prevP = latched[1];
    prevK = latched[2];
    prevPump = bombaLigada;
  }

  delay(3);
}
