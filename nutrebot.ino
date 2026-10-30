#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Preferences.h>
#include <Wire.h> 
#include <time.h>
#include <sys/time.h>
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>
#include <qrcode.h>

#include "RobotEyes.h"

const char* ONBOARDING_URL = "https://jao1up.github.io/NUTR-E-BOT/";

#define SERVICE_UUID        "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
#define CHAR_UUID           "beb5483e-36e1-4688-b7f5-ea07361b26a8"

BLEServer* pServer = NULL;
BLECharacteristic* pCharacteristic = NULL;
bool deviceConnected = false;
unsigned long lastBleNotify = 0;

const int buzzerPin = 23;     
const int alarmLedPin = 19;   
const int greenLedPin = 18;   
const int buttonPin = 4;      

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

RobotEyes eyes(&display);
Preferences preferences;

bool isFirstTimeSetup = true;
bool showingQrScreen = false;

// Múltiplos horários de alimentação e controle de 1h para água
String foodTimesList = "12:00,19:00"; 
unsigned long lastWaterIntervalCheck = 0;
unsigned long waterIntervalMs = 3600000UL; // 1 Hora em milissegundos

bool alarmActive = false;
String currentAlarmType = "";
String currentAlarmBigWord = "";
int lastTriggeredMinute = -1;
unsigned long alarmStartTime = 0;
unsigned long lastAlarmToggle = 0;
bool alarmStep = false;
unsigned long greenLedUntil = 0;

bool dndMode = false; 
bool pomoActive = false;
bool pomoIsBreak = false;
unsigned long pomoStartTime = 0;
unsigned long pomoDurationMs = 25 * 60 * 1000UL;

String customNote = "";
bool noteFixed = false;

unsigned long lastOverlayToggle = 0;
bool showingOverlay = false;
unsigned long showClockCardUntil = 0;

String currentMoodText = "NUTR-E Calmo 👀";
unsigned long lastInteractionTime = 0;
unsigned long happyStateUntil = 0;
unsigned long lastExpressionChange = 0;
unsigned long nextExpressionInterval = 14000;

enum TransitionType {
  TRANS_NONE = 0,
  TRANS_SMOOTH_MOOD,
  TRANS_BLE_THINKING,
  TRANS_WAKE_UP
};

TransitionType activeTransition = TRANS_NONE;
int transitionStep = 0;
unsigned long transitionStepTime = 0;
EyeExpression targetFluidExpr = EXPR_NEUTRAL;
int targetLookX = 0;
int targetLookY = 0;
unsigned long targetLookDur = 3000;

unsigned long lastMicroSaccade = 0;
unsigned long nextMicroSaccadeInterval = 4000;
String pendingBleCmd = "";

void playTone(int freq, int duration) {
  if (!dndMode) tone(buzzerPin, freq, duration);
}

void playThinkingChirp() {
  if (!dndMode) tone(buzzerPin, 2600, 30);
}

void playHappySound() {
  if (dndMode) return;
  tone(buzzerPin, 1800, 80); delay(90);
  tone(buzzerPin, 2200, 80); delay(90);
  tone(buzzerPin, 2800, 140);
}

void playPettingSound() {
  if (dndMode) return;
  tone(buzzerPin, 1600, 60); delay(70);
  tone(buzzerPin, 2100, 70); delay(80);
  tone(buzzerPin, 2700, 110); delay(110);
  tone(buzzerPin, 3100, 130);
}

void playAngrySound() {
  if (dndMode) return;
  tone(buzzerPin, 320, 140); delay(160);
  tone(buzzerPin, 240, 220); delay(230);
  tone(buzzerPin, 180, 280);
}

void playWaterSound() {
  if (dndMode) return;
  tone(buzzerPin, 1400, 70); delay(80);
  tone(buzzerPin, 1900, 70); delay(80);
  tone(buzzerPin, 2500, 120);
}

void playVictorySound() {
  if (dndMode) return;
  tone(buzzerPin, 1568, 100); delay(120);
  tone(buzzerPin, 1975, 100); delay(120);
  tone(buzzerPin, 2349, 100); delay(120);
  tone(buzzerPin, 3136, 220);
}

void triggerGreenLed(unsigned long durationMs = 4000) {
  digitalWrite(alarmLedPin, LOW);
  digitalWrite(greenLedPin, HIGH);
  greenLedUntil = millis() + durationMs;
}

void setExpressionFluid(EyeExpression newExpr, int lookX = 0, int lookY = 0, unsigned long lookDur = 3000) {
  targetFluidExpr = newExpr;
  targetLookX = lookX;
  targetLookY = lookY;
  targetLookDur = lookDur;
  activeTransition = TRANS_SMOOTH_MOOD;
  transitionStep = 0;
  transitionStepTime = millis();
  eyes.triggerBlink();
  eyes.lookAt(0, 2, 180);
}

void triggerBleCommandTransition(EyeExpression finalExpr, int lookX = 0, int lookY = 0) {
  targetFluidExpr = finalExpr;
  targetLookX = lookX;
  targetLookY = lookY;
  targetLookDur = 3500;
  activeTransition = TRANS_BLE_THINKING;
  transitionStep = 0;
  transitionStepTime = millis();
  playThinkingChirp();
  eyes.setExpression(EXPR_SKEPTIC);
  eyes.lookAt(-7, -3, 180);
}

void triggerWakeUpFluid(EyeExpression finalExpr = EXPR_HAPPY) {
  targetFluidExpr = finalExpr;
  activeTransition = TRANS_WAKE_UP;
  transitionStep = 0;
  transitionStepTime = millis();
  eyes.setExpression(EXPR_BLINK_LOW);
  eyes.triggerBlink();
  eyes.lookAt(-4, 1, 200);
}

void updateFluidTransitions() {
  unsigned long now = millis();
  if (activeTransition == TRANS_SMOOTH_MOOD) {
    if (transitionStep == 0 && (now - transitionStepTime >= 110)) {
      eyes.setExpression(targetFluidExpr);
      eyes.lookAt(targetLookX, targetLookY, targetLookDur);
      transitionStep = 1;
      transitionStepTime = now;
    } else if (transitionStep == 1 && (now - transitionStepTime >= 240)) {
      if (random(0, 100) < 35) eyes.triggerBlink();
      activeTransition = TRANS_NONE;
    }
  } else if (activeTransition == TRANS_BLE_THINKING) {
    if (transitionStep == 0 && (now - transitionStepTime >= 150)) {
      eyes.lookAt(7, -3, 180);
      transitionStep = 1;
      transitionStepTime = now;
    } else if (transitionStep == 1 && (now - transitionStepTime >= 150)) {
      eyes.triggerBlink();
      transitionStep = 2;
      transitionStepTime = now;
    } else if (transitionStep == 2 && (now - transitionStepTime >= 100)) {
      eyes.setExpression(targetFluidExpr);
      eyes.lookAt(targetLookX, targetLookY, targetLookDur);
      activeTransition = TRANS_NONE;
    }
  } else if (activeTransition == TRANS_WAKE_UP) {
    if (transitionStep == 0 && (now - transitionStepTime >= 180)) {
      eyes.lookAt(5, 0, 200);
      transitionStep = 1;
      transitionStepTime = now;
    } else if (transitionStep == 1 && (now - transitionStepTime >= 180)) {
      eyes.triggerBlink();
      eyes.setExpression(targetFluidExpr);
      eyes.lookAt(0, 0, 3000);
      activeTransition = TRANS_NONE;
    }
  } else {
    if (now - lastMicroSaccade > nextMicroSaccadeInterval) {
      lastMicroSaccade = now;
      nextMicroSaccadeInterval = random(3200, 6500);
      int r = random(0, 100);
      if (r < 55) eyes.lookAt(random(-5, 6), random(-2, 3), random(1200, 2800));
      else if (r < 80) eyes.lookAt(0, 0, 2000);
      else eyes.triggerBlink();
    }
  }
}

void setInternalTime(int h, int m, int s = 0) {
  struct tm tm_new = {0};
  tm_new.tm_year = 2026 - 1900;
  tm_new.tm_mon = 9;
  tm_new.tm_mday = 29;
  tm_new.tm_hour = h;
  tm_new.tm_min = m;
  tm_new.tm_sec = s;
  time_t t = mktime(&tm_new);
  struct timeval now = { .tv_sec = t, .tv_usec = 0 };
  settimeofday(&now, NULL);
}

String getCurrentTimeString() {
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo, 10)) return "--:--";
  char timeStr[6];
  strftime(timeStr, sizeof(timeStr), "%H:%M", &timeinfo);
  return String(timeStr);
}

String getPomodoroStatusString() {
  if (!pomoActive) return "Inativo";
  unsigned long elapsed = millis() - pomoStartTime;
  if (elapsed >= pomoDurationMs) return "00:00";
  unsigned long remSec = (pomoDurationMs - elapsed) / 1000;
  char buf[32];
  snprintf(buf, sizeof(buf), "%s (%02lu:%02lu)", pomoIsBreak ? "Pausa" : "Foco", (remSec / 60) % 100, remSec % 60);
  return String(buf);
}

void sendBleStatus() {
  if (!deviceConnected || pCharacteristic == NULL) return;
  String json = "{\"time\":\"" + getCurrentTimeString() + 
                "\",\"mood\":\"" + currentMoodText + 
                "\",\"pomo\":\"" + getPomodoroStatusString() + 
                "\",\"dnd\":" + String(dndMode ? "true" : "false") + 
                ",\"food\":\"" + foodTimesList + "\"}";
  pCharacteristic->setValue(json.c_str());
  pCharacteristic->notify();
}

void renderQrOnOled(esp_qrcode_handle_t qrcode) {
  int qrSize = esp_qrcode_get_size(qrcode);
  int scale = (qrSize <= 31) ? 2 : 1;
  int offset = (64 - (qrSize * scale)) / 2;

  for (int y = 0; y < qrSize; y++) {
    for (int x = 0; x < qrSize; x++) {
      if (esp_qrcode_get_module(qrcode, x, y)) {
        display.fillRect(offset + (x * scale), offset + (y * scale), scale, scale, SSD1306_BLACK);
      }
    }
  }
}

void drawOnboardingQrScreen() {
  display.clearDisplay();
  display.fillRect(0, 0, 64, 64, SSD1306_WHITE);
  esp_qrcode_config_t cfg = {};
  cfg.display_func = renderQrOnOled;
  cfg.max_qrcode_version = 4;
  cfg.qrcode_ecc_level = ESP_QRCODE_ECC_LOW;
  esp_qrcode_generate(&cfg, ONBOARDING_URL);
  display.setTextColor(SSD1306_WHITE);
  display.setTextSize(1);
  display.setCursor(68, 2);
  display.print("NUTR-E");
  display.drawFastHLine(68, 12, 56, SSD1306_WHITE);
  display.setCursor(68, 18);
  display.print("1.Escaneie");
  display.setCursor(68, 29);
  display.print("o QR Code");
  display.setCursor(68, 42);
  display.print("2.Abra no");
  display.setCursor(68, 53);
  display.print("CHROME!");
  display.display();
}

class MyServerCallbacks: public BLEServerCallbacks {
  void onConnect(BLEServer* pServer) { deviceConnected = true; };
  void onDisconnect(BLEServer* pServer) {
    deviceConnected = false;
    BLEDevice::startAdvertising();
  }
};

class MyCharCallbacks: public BLECharacteristicCallbacks {
  void onWrite(BLECharacteristic *pChar) {
    String val = pChar->getValue().c_str();
    val.trim();
    if (val.length() > 0) pendingBleCmd = val;
  }
};

void drawAlarmScreen(const char* bigWord, const char* timeStr) {
  display.clearDisplay();
  display.drawRoundRect(0, 0, 128, 64, 5, SSD1306_WHITE);
  display.drawRoundRect(2, 2, 124, 60, 4, SSD1306_WHITE);
  display.fillRect(2, 2, 124, 14, SSD1306_WHITE);
  display.setTextSize(1);
  display.setTextColor(SSD1306_BLACK);
  String header = "NUTR-E - " + String(timeStr);
  int headerX = (128 - (header.length() * 6)) / 2;
  if (headerX < 4) headerX = 4;
  display.setCursor(headerX, 5);
  display.print(header);

  display.setTextSize(3);
  display.setTextColor(SSD1306_WHITE);
  int wordLen = strlen(bigWord);
  int bigX = (128 - (wordLen * 18)) / 2;
  if (bigX < 4) bigX = 4;
  display.setCursor(bigX, 21);
  display.print(bigWord);
  display.display();
}

void drawClockStatusCard() {
  display.clearDisplay();
  display.drawRoundRect(0, 0, 128, 64, 5, SSD1306_WHITE);
  display.fillRect(2, 2, 124, 13, SSD1306_WHITE);
  display.setTextSize(1);
  display.setTextColor(SSD1306_BLACK);
  const char* topTxt = deviceConnected ? "NUTR-E: ONLINE" : "BLE: NUTRE";
  int ipX = (128 - (strlen(topTxt) * 6)) / 2;
  display.setCursor(ipX, 5);
  display.print(topTxt);

  String nowStr = getCurrentTimeString();
  display.setTextSize(3);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(19, 20);
  display.print(nowStr);
  display.display();
}

void startAlarm(String type) {
  alarmActive = true;
  showingQrScreen = false;
  currentAlarmType = type;
  alarmStartTime = millis();
  lastAlarmToggle = 0;
  alarmStep = false;
  digitalWrite(greenLedPin, LOW);
  digitalWrite(alarmLedPin, HIGH);

  if (type == "FOOD") {
    currentAlarmBigWord = "COMER!";
    currentMoodText = "NUTR-E: Hora de Comer! 🍽️";
    setExpressionFluid(EXPR_SAD_UP, 0, -2);
  } else {
    currentAlarmBigWord = "AGUA!";
    currentMoodText = "NUTR-E: Hora de Água (1h)! 💧";
    setExpressionFluid(EXPR_WORRIED, 0, 0);
  }
  sendBleStatus();
}

void triggerCareBoost(String type) {
  alarmActive = false;
  showingQrScreen = false;
  noTone(buzzerPin);
  triggerGreenLed(4000);
  lastInteractionTime = millis();
  happyStateUntil = millis() + 180000;
  triggerBleCommandTransition(EXPR_HAPPY, 0, 0);

  if (type == "FOOD") {
    currentMoodText = "NUTR-E Satisfeito! 😊";
    playHappySound();
  } else {
    currentMoodText = "Água Registrada! 💧";
    playWaterSound();
  }
  sendBleStatus();
}

void processBleCommands() {
  if (pendingBleCmd == "") return;
  String cmd = pendingBleCmd;
  pendingBleCmd = "";

  if (cmd == "FOOD") triggerCareBoost("FOOD");
  else if (cmd == "WATER") triggerCareBoost("WATER");
  else if (cmd == "STOP") triggerCareBoost(currentAlarmType != "" ? currentAlarmType : "FOOD");
  else if (cmd.startsWith("FOODS:")) {
    foodTimesList = cmd.substring(6);
    preferences.putString("food_list", foodTimesList);
    triggerGreenLed(1500);
    playTone(2000, 100);
  }
  else if (cmd == "SHOW_QR") { showingQrScreen = true; drawOnboardingQrScreen(); }
  else if (cmd == "HIDE_QR") { showingQrScreen = false; triggerWakeUpFluid(EXPR_HAPPY); }
  else if (cmd == "DND") {
    dndMode = !dndMode;
    currentMoodText = dndMode ? "NUTR-E Mutado 🔇" : "Som Ativo 🔊";
  }
  else if (cmd.startsWith("TIME:")) {
    String tStr = cmd.substring(5);
    if (tStr.length() >= 5) {
      setInternalTime(tStr.substring(0, 2).toInt(), tStr.substring(3, 5).toInt());
    }
  }
  sendBleStatus();
}

void handlePhysicalButton() {
  bool currentState = digitalRead(buttonPin);
  unsigned long now = millis();
  static unsigned long pressStart = 0;

  if (lastBtnState == HIGH && currentState == LOW) {
    pressStart = now;
  } else if (lastBtnState == LOW && currentState == HIGH) {
    if (now - pressStart > 1500) {
      triggerCareBoost("WATER"); // Segurar confirma água
    } else {
      if (alarmActive) triggerCareBoost(currentAlarmType);
      else {
        showClockCardUntil = now + 5000;
        triggerGreenLed(1000);
        playTone(2000, 60);
      }
    }
  }
  lastBtnState = currentState;
}

void checkTimersAndAlarms() {
  unsigned long now = millis();
  if (greenLedUntil > 0 && now >= greenLedUntil) {
    digitalWrite(greenLedPin, LOW);
    greenLedUntil = 0;
  }

  // Verificação de 1 em 1 hora para Água
  if (now - lastWaterIntervalCheck >= waterIntervalMs) {
    lastWaterIntervalCheck = now;
    if (!alarmActive) startAlarm("WATER");
  }

  struct tm timeinfo;
  if (getLocalTime(&timeinfo, 10)) {
    char currentHM[6];
    strftime(currentHM, sizeof(currentHM), "%H:%M", &timeinfo);
    String nowStr = String(currentHM);

    if (!alarmActive && timeinfo.tm_min != lastTriggeredMinute) {
      // Verifica se o minuto atual bate com algum dos horários da lista de alimentação
      if (foodTimesList.indexOf(nowStr) != -1) {
        lastTriggeredMinute = timeinfo.tm_min;
        startAlarm("FOOD");
      }
    }
  }

  if (alarmActive) {
    if (now - lastAlarmToggle > 650) {
      lastAlarmToggle = now;
      alarmStep = !alarmStep;
      if (alarmStep) {
        digitalWrite(alarmLedPin, HIGH);
        playTone(2400, 250);
        drawAlarmScreen(currentAlarmBigWord.c_str(), getCurrentTimeString().c_str());
      } else {
        digitalWrite(alarmLedPin, LOW);
        playTone(1800, 250);
      }
    }
    if (!alarmStep) eyes.update();
  } else {
    digitalWrite(alarmLedPin, LOW);
  }
}

void updatePersonality() {
  if (alarmActive || showingQrScreen) return;
  if (now < showClockCardUntil) {
    drawClockStatusCard();
    return;
  }
  updateFluidTransitions();
  eyes.update();
}

void setup() {
  Serial.begin(115200);
  pinMode(buzzerPin, OUTPUT);
  pinMode(alarmLedPin, OUTPUT);
  digitalWrite(alarmLedPin, LOW);
  pinMode(greenLedPin, OUTPUT);
  digitalWrite(greenLedPin, LOW);
  pinMode(buttonPin, INPUT_PULLUP);

  Wire.begin(21, 22);
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);

  preferences.begin("wifi_config", false);
  foodTimesList = preferences.getString("food_list", "12:00,19:00");

  setInternalTime(12, 0, 0);
  lastWaterIntervalCheck = millis(); // Inicia contagem de 1h

  BLEDevice::init("NUTRE");
  pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());
  BLEService *pService = pServer->createService(SERVICE_UUID);
  pCharacteristic = pService->createCharacteristic(
                      CHAR_UUID,
                      BLECharacteristic::PROPERTY_READ   |
                      BLECharacteristic::PROPERTY_WRITE  |
                      BLECharacteristic::PROPERTY_NOTIFY
                    );
  pCharacteristic->addDescriptor(new BLE2902());
  pCharacteristic->setCallbacks(new MyCharCallbacks());
  pService->start();

  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(SERVICE_UUID);
  pAdvertising->setScanResponse(true);
  BLEDevice::startAdvertising();

  eyes.setExpression(EXPR_HAPPY);
}

void loop() {
  processBleCommands();
  handlePhysicalButton();
  checkTimersAndAlarms();
  updatePersonality();

  if (deviceConnected && (millis() - lastBleNotify > 2000)) {
    lastBleNotify = millis();
    sendBleStatus();
  }
}