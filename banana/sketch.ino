
// --- Código para Irrigação do Cultivo de Bananas --- 

#include <Arduino.h>
#include "DHTesp.h"


// ------------ CONFIGS RÁPIDAS ------------
#define RELAY_ACTIVE_HIGH true   // se seu relé ligar invertido, troque para false
#define PH_INVERT true           // se luz alta der pH baixo, troque para true


// Pinos (conforme seu diagrama)
const int PIN_LDR   = 34;   // LDR (pH simulado no ADC)
const int PIN_DHT   = 15;   // DHT22 (umidade)
const int PIN_RELAY = 25;   // Relé (bomba)

// Botões NPK como pushbutton (vamos tratá-los como toggle por software)
const int BTN_PINS[3] = {12, 13, 14}; // N, P, K (nessa ordem)

// Estados/toggle dos botões
bool latched[3]         = {false, false, false}; // N, P, K travados
int  prevRead[3]        = {HIGH, HIGH, HIGH};    // leitura anterior (pull-up = HIGH em repouso)
unsigned long lastT[3]  = {0, 0, 0};             // debounce ms

// Alvos da cultura - Banana
const float phMin = 5.5, phMax = 6.5;         // faixa ideal do solo
const float umidadeLiga = 60.0;               // liga bomba se < 60%
const float umidadeDesliga = 80.0;            // desliga bomba se > 80%

DHTesp dht;
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

void setPump(bool on) {
  digitalWrite(PIN_RELAY, (RELAY_ACTIVE_HIGH ? (on ? HIGH : LOW) : (on ? LOW : HIGH)));
  bombaLigada = on;
}

// Atualiza 1 botão (toggle por borda de descida + debounce)
void updateButton(int i) {
  int r = digitalRead(BTN_PINS[i]);
  unsigned long now = millis();
  if (r != prevRead[i] && (now - lastT[i]) > 25) {  // debounce ~25ms
    lastT[i] = now;
    // HIGH -> LOW significa "apertou": alterna o estado travado
    if (prevRead[i] == HIGH && r == LOW) latched[i] = !latched[i];
    prevRead[i] = r;
  }
}

void setup() {
  Serial.begin(115200);

  // Entradas com pull-up (pushbutton entre GPIO e GND)
  for (int i = 0; i < 3; i++) pinMode(BTN_PINS[i], INPUT_PULLUP);

  pinMode(PIN_RELAY, OUTPUT);
  setPump(false);

  dht.setup(PIN_DHT, DHTesp::DHT22);
  delay(1200); // estabilizar DHT

  analogReadResolution(12); // ESP32: leitura de 0..4095
  Serial.println("FarmTech - Irrigacao Inteligente para Banana (toggle em pushbuttons)");
}

void loop() {
  // Atualiza/toggle dos botoes (um clique alterna o estado travado)
  updateButton(0); // N
  updateButton(1); // P
  updateButton(2); // K

  float ph = readPH();
  TempAndHumidity th = dht.getTempAndHumidity();
  float hum = isnan(th.humidity) ? 57.0 : th.humidity; // fallback se erro

  bool nutrientesOK = latched[0] && latched[1] && latched[2]; // N, P, K ok?
  bool phOK = (ph >= phMin && ph <= phMax);

  // Controle da bomba com histerese para Banana
  if (!nutrientesOK || !phOK) {
    setPump(false);
  } else {
    if (!bombaLigada && hum < umidadeLiga)     setPump(true);
    if (bombaLigada  && hum > umidadeDesliga)  setPump(false);
  }

  // Imprime só quando algo muda ou a cada ~1,5s
  static bool prevN=false, prevP=false, prevK=false, prevPump=false;
  static unsigned long t0 = 0;
  bool changed = (prevN!=latched[0]) || (prevP!=latched[1]) || (prevK!=latched[2]) || (prevPump!=bombaLigada);
  if (changed || millis()-t0 > 1500) {
    t0 = millis();
    Serial.print("NPK: ");
    Serial.print(latched[0] ? "N " : "n ");
    Serial.print(latched[1] ? "P " : "p ");
    Serial.print(latched[2] ? "K " : "k ");

    Serial.print(" | Nutrientes: ");
    if (nutrientesOK) Serial.print("OK");
    else {
      Serial.print("Incompletos (faltam: ");
      bool first = true;
      if (!latched[0]) { Serial.print("N"); first = false; }
      if (!latched[1]) { if (!first) Serial.print(", "); Serial.print("P"); first = false; }
      if (!latched[2]) { if (!first) Serial.print(", "); Serial.print("K"); }
      Serial.print(")");
    }

    Serial.print(" | pH=");   Serial.print(ph, 2); Serial.print(" (");
    Serial.print(phMin); Serial.print("-"); Serial.print(phMax); Serial.print(")");
    Serial.print(" | Umid="); Serial.print(hum,1); Serial.print("%");
    Serial.print(" | Bomba="); Serial.println(bombaLigada ? "ON" : "OFF");

    prevN=latched[0]; prevP=latched[1]; prevK=latched[2]; prevPump=bombaLigada;
  }

  delay(3); // cooperativo
}
