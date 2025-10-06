#include <Arduino.h>

// ------------ CONFIGS ------------
#define RELAY_ACTIVE_HIGH true
#define PH_INVERT true

const int PIN_LDR   = 34;   // LDR (pH simulado)
const int PIN_RELAY = 25;   // Relé
const int PIN_N     = 33;   // Potenciômetro slide para Nitrogênio
const int PIN_P     = 32;   // Potenciômetro slide para Fósforo
const int PIN_K     = 35;   // Potenciômetro slide para Potássio
const int PIN_UMID  = 27;   // Potenciômetro comum (pode trocar GPIO se usar slide separado)

// Valores alvo para simulação de cultura Banana
const float alvoN   = 8.0;
const float alvoP   = 10.0;
const float alvoK   = 8.0;
const float phMin   = 5.5, phMax = 6.6;
const float umidadeLiga    = 55.0;
const float umidadeDesliga = 70.0;

bool bombaLigada = false;

// --- Utilidades ---
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

// Slide: lê e escala 0-10
float readNPK(int pin) {
  int val = analogRead(pin);
  float npk = fmap(val, 0, 4095, 0.0, 10.0);
  if (npk < 0) npk = 0;
  if (npk > 10) npk = 10;
  return npk;
}

// Potenciômetro comum para umidade (0-100%)
float readSimulatedHumidity() {
  int val = analogRead(PIN_UMID);
  float hum = fmap(val, 0, 4095, 0.0, 100.0);
  if (hum < 0) hum = 0;
  if (hum > 100) hum = 100;
  return hum;
}

void setPump(bool on) {
  digitalWrite(PIN_RELAY, (RELAY_ACTIVE_HIGH ? (on ? HIGH : LOW) : (on ? LOW : HIGH)));
  bombaLigada = on;
}

void setup() {
  Serial.begin(115200);

  pinMode(PIN_RELAY, OUTPUT);
  setPump(false);

  pinMode(PIN_LDR, INPUT);
  pinMode(PIN_N, INPUT);
  pinMode(PIN_P, INPUT);
  pinMode(PIN_K, INPUT);
  pinMode(PIN_UMID, INPUT);

  analogReadResolution(12);
  Serial.println("FarmTech - Irrigacao Inteligente Dinamica! NPK via potenciometros (slide).");
}

void loop() {
  float ph      = readPH();
  float hum     = readSimulatedHumidity();
  float valN    = readNPK(PIN_N);
  float valP    = readNPK(PIN_P);
  float valK    = readNPK(PIN_K);

  bool nutrientesOK = fabs(valN - alvoN) <= 1.0 && fabs(valP - alvoP) <= 1.0 && fabs(valK - alvoK) <= 1.0; // tolerância 1 unidade
  bool phOK        = (ph >= phMin && ph <= phMax);

  // Lógica bomba: apenas liga se tudo OK e humidade abaixo do alvo
  if (!nutrientesOK || !phOK) {
    setPump(false);
  } else {
    if (!bombaLigada && hum < umidadeLiga) setPump(true);
    if (bombaLigada && hum > umidadeDesliga) setPump(false);
  }

  static unsigned long t0 = 0;
  if (millis() - t0 > 1500) {
    t0 = millis();
    Serial.print("N=");
    Serial.print(valN, 1);
    Serial.print(" P=");
    Serial.print(valP, 1);
    Serial.print(" K=");
    Serial.print(valK, 1);

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

    Serial.print(" | Nutrientes: ");
    Serial.print(nutrientesOK ? "OK" : "Fora da faixa");
    Serial.print(" | Bomba=");
    Serial.println(bombaLigada ? "ON" : "OFF");
  }
  delay(2);
}
