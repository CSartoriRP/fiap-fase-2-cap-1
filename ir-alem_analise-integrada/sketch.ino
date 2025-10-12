#include <Arduino.h>

// ------------ CONFIGS ------------
#define RELAY_ACTIVE_HIGH true
#define PH_INVERT true

const int PIN_LDR   = 34;   // LDR (pH simulado)
const int PIN_RELAY = 25;   // Relé
const int PIN_N     = 33;   // Potenciômetro slide p/ Nitrogênio
const int PIN_P     = 32;   // Potenciômetro slide p/ Fósforo
const int PIN_K     = 35;   // Potenciômetro slide p/ Potássio
const int PIN_UMID  = 27;   // Potenciômetro comum (umidade simulada)

// Valores alvo (Banana)
const float alvoN   = 8.0;
const float alvoP   = 10.0;
const float alvoK   = 8.0;
const float phMin   = 5.5, phMax = 6.6;
const float umidadeLiga    = 55.0;
const float umidadeDesliga = 70.0;

// --- Sinais externos (via Serial) ---
float popChuvaPct = -1.0f;      // Prob. de chuva (%), -1 = não informado
float chuva3h_mm  = -1.0f;      // Chuva prevista (mm/3h), -1 = não informado
unsigned long holdAteMs = 0;    // Bloqueio até este instante (millis)
const unsigned long RAIN_AUTO_HOLD_MS = 10UL * 60UL * 1000UL; // 10 min

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

float readNPK(int pin) {
  int val = analogRead(pin);
  float npk = fmap(val, 0, 4095, 0.0, 10.0);
  if (npk < 0) npk = 0;
  if (npk > 10) npk = 10;
  return npk;
}

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

// ---- Serial: parser de comandos ----
// Aceita linhas com um ou vários tokens separados por espaço ou vírgula:
//   POP=75
//   RAIN3H=1.2
//   HOLD=ON   | HOLD=OFF | HOLD=15   (minutos)
//   CLEAR
//   STATUS?
String readSerialLineNonBlocking() {
  static String buffer;
  while (Serial.available() > 0) {
    char c = (char)Serial.read();
    if (c == '\r') continue; // Windows envia CRLF
    if (c == '\n') { String line = buffer; buffer = ""; return line; }
    buffer += c;
  }
  return String();
}

void processToken(const String& tokRaw) {
  String tok = tokRaw; tok.trim();
  String U = tok; U.toUpperCase();

  if (U.startsWith("POP=")) {
    String s = tok.substring(4); s.replace(',', '.');
    popChuvaPct = s.toFloat();
    if (popChuvaPct >= 60.0f) holdAteMs = millis() + RAIN_AUTO_HOLD_MS;
    Serial.print("[OK] POP atualizado: "); Serial.print(popChuvaPct, 1); Serial.println("%");

  } else if (U.startsWith("RAIN3H=")) {
    String s = tok.substring(7); s.replace(',', '.');
    chuva3h_mm = s.toFloat();
    if (chuva3h_mm >= 1.0f) holdAteMs = millis() + RAIN_AUTO_HOLD_MS;
    Serial.print("[OK] RAIN3H atualizado: "); Serial.print(chuva3h_mm, 2); Serial.println(" mm");

  } else if (U.startsWith("HOLD=")) {
    String arg = tok.substring(5); arg.trim(); String A = arg; A.toUpperCase();
    if (A == "ON") {
      holdAteMs = millis() + RAIN_AUTO_HOLD_MS;
    } else if (A == "OFF") {
      holdAteMs = 0;
    } else {
      unsigned long min = (unsigned long) arg.toInt();
      holdAteMs = millis() + (min * 60UL * 1000UL);
    }
    Serial.print("[OK] HOLD: "); Serial.println((holdAteMs > millis()) ? "ON" : "OFF");

  } else if (U == "CLEAR") {
    popChuvaPct = -1.0f; chuva3h_mm = -1.0f; holdAteMs = 0;
    Serial.println("[OK] Sinais externos limpos (POP/RAIN3H) e HOLD removido.");

  } else if (U == "STATUS?") {
    Serial.print("[STATUS] POP=");
    if (popChuvaPct < 0) Serial.print("NA"); else Serial.print(popChuvaPct,1);
    Serial.print("%  RAIN3H=");
    if (chuva3h_mm < 0) Serial.print("NA"); else { Serial.print(chuva3h_mm,2); Serial.print("mm"); }
    Serial.print("  HOLD_ATIVO=");
    Serial.println((holdAteMs > millis()) ? "SIM" : "NAO");

  } else {
    Serial.print("[WARN] Comando desconhecido: ");
    Serial.println(tok);
  }
}

void handleCommand(String line) {
  if (!line.length()) return;
  line.replace(',', ' ');  // permite separação por vírgula
  line.trim();
  int start = 0;
  while (start < line.length()) {
    int sp = line.indexOf(' ', start);
    String tok = (sp == -1) ? line.substring(start) : line.substring(start, sp);
    tok.trim();
    if (tok.length()) processToken(tok);
    if (sp == -1) break;
    start = sp + 1;
  }
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

  Serial.println("FarmTech - Irrigacao Inteligente + Serial-in (POP/RAIN3H/HOLD).");
  Serial.println("Exemplos: POP=75 | RAIN3H=1.2 | HOLD=15 | HOLD=ON | CLEAR | STATUS?");
}

void loop() {
  // 1) Comandos via Serial
  String line = readSerialLineNonBlocking();
  if (line.length() > 0) handleCommand(line);

  // 2) Leituras locais
  float ph   = readPH();
  float hum  = readSimulatedHumidity();
  float valN = readNPK(PIN_N);
  float valP = readNPK(PIN_P);
  float valK = readNPK(PIN_K);

  bool nutrientesOK = fabs(valN - alvoN) <= 1.0 && fabs(valP - alvoP) <= 1.0 && fabs(valK - alvoK) <= 1.0;
  bool phOK         = (ph >= phMin && ph <= phMax);

  // 3) Regras externas (chuva/hold)
  bool holdAtivo = (holdAteMs > millis());
  bool chuvaPrevAlta = (popChuvaPct >= 60.0f) || (chuva3h_mm >= 1.0f);

  // 4) Lógica da bomba (com histerese de umidade)
  if (holdAtivo || chuvaPrevAlta) {
    setPump(false);
  } else if (!nutrientesOK || !phOK) {
    setPump(false);
  } else {
    if (!bombaLigada && hum < umidadeLiga) setPump(true);
    if (bombaLigada && hum > umidadeDesliga) setPump(false);
  }

  // 5) Log periódico
  static unsigned long t0 = 0;
  if (millis() - t0 > 1500) {
    t0 = millis();
    Serial.print("N=");  Serial.print(valN, 1);
    Serial.print(" P="); Serial.print(valP, 1);
    Serial.print(" K="); Serial.print(valK, 1);

    Serial.print(" | pH="); Serial.print(ph, 2);
    Serial.print(" ("); Serial.print(phMin); Serial.print("-"); Serial.print(phMax); Serial.print(")");

    Serial.print(" | Umid="); Serial.print(hum, 1); Serial.print("%");

    Serial.print(" | Nutrientes: "); Serial.print(nutrientesOK ? "OK" : "Fora da faixa");

    Serial.print(" | Ext[POP=");
    if (popChuvaPct < 0) Serial.print("NA"); else Serial.print(popChuvaPct,1);
    Serial.print("% RAIN3H=");
    if (chuva3h_mm < 0) Serial.print("NA"); else Serial.print(chuva3h_mm,2);
    Serial.print("mm HOLD=");
    Serial.print(holdAtivo ? "ON" : "OFF");
    Serial.print("]");

    Serial.print(" | Bomba="); Serial.println(bombaLigada ? "ON" : "OFF");
  }
  delay(2);
}
