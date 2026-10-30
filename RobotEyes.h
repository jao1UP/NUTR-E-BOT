#ifndef ROBOT_EYES_H
#define ROBOT_EYES_H

#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// Todas as 21 expressões originais da tabela do Cozmo
enum EyeExpression {
  EXPR_NEUTRAL,
  EXPR_BLINK_HIGH,
  EXPR_HAPPY,
  EXPR_GLEE,
  EXPR_BLINK_LOW,
  EXPR_SAD_DOWN,
  EXPR_SAD_UP,
  EXPR_WORRIED,
  EXPR_FOCUSED,
  EXPR_ANNOYED,
  EXPR_SURPRISED,
  EXPR_SKEPTIC,
  EXPR_FRUSTRATED,
  EXPR_UNIMPRESSED,
  EXPR_SLEEPY,
  EXPR_SUSPICIOUS,
  EXPR_SQUINT,
  EXPR_ANGRY,
  EXPR_FURIOUS,
  EXPR_SCARED,
  EXPR_AWE
};

struct EyeState {
  float w;     // Largura do olho
  float h;     // Altura do olho
  float r;     // Raio de arredondamento dos cantos
  float xOff;  // Ajuste fino horizontal
  float yOff;  // Ajuste fino vertical
  float topL;  // Corte superior esquerdo
  float topR;  // Corte superior direito
  float botL;  // Corte inferior esquerdo
  float botR;  // Corte inferior direito
  float arch;  // Curvatura côncava inferior (Glee)
};

class RobotEyes {
  private:
    Adafruit_SSD1306* disp;
    int screenWidth, screenHeight;

    int leftEyeX = 46;
    int rightEyeX = 82;
    int centerY = 32;

    bool useScanlines = false;
    bool connectBlinkLine = false;

    EyeState currentL, currentR;
    EyeState targetL, targetR;

    // Movimento de direção do olhar
    float gazeX = 0;
    float gazeY = 0;
    float targetGazeX = 0;
    float targetGazeY = 0;
    unsigned long manualGazeUntil = 0; // Mantém o olhar fixo quando uma cena mandar

    // Controle de piscada automática
    unsigned long lastBlink = 0;
    unsigned long nextBlinkInterval = 3200;
    bool blinking = false;

    // Controle de movimentos aleatórios do olhar
    unsigned long lastLook = 0;
    unsigned long nextLookInterval = 2000;

    EyeExpression currentExpr = EXPR_NEUTRAL;

    void setMirrored(float w, float h, float r, float yOff,
                     float tOut, float tIn, float bOut, float bIn,
                     float arch = 0, bool connect = false) {
      targetL = {w, h, r, 0, yOff, tOut, tIn, bOut, bIn, arch};
      targetR = {w, h, r, 0, yOff, tIn, tOut, bIn, bOut, arch};
      connectBlinkLine = connect;
    }

    void setCustom(EyeState leftEye, EyeState rightEye, bool connect = false) {
      targetL = leftEye;
      targetR = rightEye;
      connectBlinkLine = connect;
    }

    float lerpVal(float a, float b, float t) {
      return a + (b - a) * t;
    }

  public:
    RobotEyes(Adafruit_SSD1306* displayInstance) {
      disp = displayInstance;
      screenWidth = disp->width();
      screenHeight = disp->height();

      setExpression(EXPR_NEUTRAL);
      currentL = targetL;
      currentR = targetR;
    }

    void setScanlines(bool enable) {
      useScanlines = enable;
    }

    // Faz o robô olhar intencionalmente para uma direção (x: -10 a 10, y: -8 a 8)
    void lookAt(float x, float y, unsigned long durationMs = 3000) {
      targetGazeX = x;
      targetGazeY = y;
      manualGazeUntil = millis() + durationMs;
    }

    // Força uma piscada imediata
    void triggerBlink() {
      blinking = true;
      lastBlink = millis();
    }

    void setExpression(EyeExpression expr) {
      currentExpr = expr;

      switch (expr) {
        case EXPR_NEUTRAL:
          setCustom(
            {28, 28, 7, 0,  0, 0, 0, 0, 0, 0},
            {28, 30, 7, 0, -1, 0, 0, 0, 0, 0}
          );
          break;
        case EXPR_BLINK_HIGH:
          setMirrored(30, 2, 1, -10, 0, 0, 0, 0, 0, true);
          break;
        case EXPR_HAPPY:
          setMirrored(28, 24, 12, -2, 0, 0, 11, 11, 2, false);
          break;
        case EXPR_GLEE:
          setMirrored(28, 22, 12, -6, 0, 0, 8, 8, 10, false);
          break;
        case EXPR_BLINK_LOW:
          setMirrored(30, 2, 1, 10, 0, 0, 0, 0, 0, true);
          break;
        case EXPR_SAD_DOWN:
          setMirrored(27, 20, 6, 9, 9, 1, 0, 4, 0, false);
          break;
        case EXPR_SAD_UP:
          setMirrored(28, 26, 7, -7, 9, 1, 0, 3, 0, false);
          break;
        case EXPR_WORRIED:
          setCustom(
            {27, 24, 7, 0, 0, 6, 1, 0, 1, 0},
            {27, 27, 7, 0, -1, 0, 10, 1, 0, 0}
          );
          break;
        case EXPR_FOCUSED:
          setMirrored(27, 11, 3, 0, 1, 1, 1, 1, 0, false);
          break;
        case EXPR_ANNOYED:
          setCustom(
            {26, 16, 6, 0, 0, 7, 7, 0, 0, 0},
            {26, 16, 4, 0, 0, 7, 7, 5, 5, 0}
          );
          break;
        case EXPR_SURPRISED:
          setMirrored(31, 34, 10, -1, 0, 0, 0, 0, 0, false);
          break;
        case EXPR_SKEPTIC:
          setCustom(
            {27, 26, 7, 0, 0, 0, 0, 0, 0, 0},
            {27, 26, 7, 0, 0, 12, 8, 0, 0, 0}
          );
          break;
        case EXPR_FRUSTRATED:
          setMirrored(27, 20, 8, 2, 10, 10, 0, 0, 0, false);
          break;
        case EXPR_UNIMPRESSED:
          setCustom(
            {27, 20, 8, 0, 2, 11, 11, 0, 0, 0},
            {27, 20, 8, 0, 2, 9, 9, 0, 0, 0}
          );
          break;
        case EXPR_SLEEPY:
          setCustom(
            {26, 20, 5, 0, 5, 7, 4, 0, 2, 0},
            {26, 20, 3, 0, 5, 7, 12, 8, 3, 0}
          );
          break;
        case EXPR_SUSPICIOUS:
          setCustom(
            {27, 14, 4, 0, 1, 5, 3, 2, 2, 0},
            {27, 16, 4, 0, 1, 4, 2, 2, 2, 0}
          );
          break;
        case EXPR_SQUINT:
          setMirrored(27, 6, 2, 1, 0, 0, 0, 0, 0, false);
          break;
        case EXPR_ANGRY:
          setMirrored(27, 22, 7, 3, 3, 12, 0, 0, 0, false);
          break;
        case EXPR_FURIOUS:
          setMirrored(28, 24, 3, 2, 0, 15, 3, 0, 0, false);
          break;
        case EXPR_SCARED:
          setMirrored(29, 28, 9, -4, 5, 0, 4, 4, 0, false);
          break;
        case EXPR_AWE:
          setMirrored(31, 36, 14, 0, 0, 0, 0, 0, 0, false);
          break;
      }
    }

    void update() {
      unsigned long now = millis();

      // 1. Piscada automática natural
      bool isStaticBlink = (currentExpr == EXPR_BLINK_HIGH || currentExpr == EXPR_BLINK_LOW);
      if (!isStaticBlink) {
        if (!blinking && now - lastBlink > nextBlinkInterval) {
          blinking = true;
          lastBlink = now;
          nextBlinkInterval = random(2200, 5800);
        }
      } else {
        blinking = false;
      }

      float targetH_L = targetL.h;
      float targetH_R = targetR.h;
      if (blinking) {
        targetH_L = 2;
        targetH_R = 2;
        if (currentL.h <= 3.2 && currentR.h <= 3.2) {
          blinking = false;
        }
      }

      // 2. Movimento do olhar (respeita comandos manuais das cenas ou move sozinho)
      if (now >= manualGazeUntil) {
        if (currentExpr == EXPR_SCARED || currentExpr == EXPR_FURIOUS) {
          targetGazeX = random(-1, 2);
          targetGazeY = random(-1, 2);
        } else if (now - lastLook > nextLookInterval) {
          if (random(0, 10) < 5) {
            targetGazeX = 0;
            targetGazeY = 0;
          } else {
            targetGazeX = random(-7, 8);
            targetGazeY = random(-4, 5);
          }
          lastLook = now;
          nextLookInterval = random(1000, 3000);
        }
      }

      // 3. Interpolação suave (Lerp)
      float sp = 0.40;
      currentL.w    = lerpVal(currentL.w,    targetL.w,    sp);
      currentL.h    = lerpVal(currentL.h,    targetH_L,    0.5);
      currentL.r    = lerpVal(currentL.r,    targetL.r,    sp);
      currentL.xOff = lerpVal(currentL.xOff, targetL.xOff, sp);
      currentL.yOff = lerpVal(currentL.yOff, targetL.yOff, sp);
      currentL.topL = lerpVal(currentL.topL, targetL.topL, sp);
      currentL.topR = lerpVal(currentL.topR, targetL.topR, sp);
      currentL.botL = lerpVal(currentL.botL, targetL.botL, sp);
      currentL.botR = lerpVal(currentL.botR, targetL.botR, sp);
      currentL.arch = lerpVal(currentL.arch, targetL.arch, sp);

      currentR.w    = lerpVal(currentR.w,    targetR.w,    sp);
      currentR.h    = lerpVal(currentR.h,    targetH_R,    0.5);
      currentR.r    = lerpVal(currentR.r,    targetR.r,    sp);
      currentR.xOff = lerpVal(currentR.xOff, targetR.xOff, sp);
      currentR.yOff = lerpVal(currentR.yOff, targetR.yOff, sp);
      currentR.topL = lerpVal(currentR.topL, targetR.topL, sp);
      currentR.topR = lerpVal(currentR.topR, targetR.topR, sp);
      currentR.botL = lerpVal(currentR.botL, targetR.botL, sp);
      currentR.botR = lerpVal(currentR.botR, targetR.botR, sp);
      currentR.arch = lerpVal(currentR.arch, targetR.arch, sp);

      gazeX = lerpVal(gazeX, targetGazeX, 0.30);
      gazeY = lerpVal(gazeY, targetGazeY, 0.30);

      // 4. Desenho no display
      disp->clearDisplay();

      if (connectBlinkLine || (blinking && currentL.h <= 4.0)) {
        int yLine = centerY + (int)gazeY + (int)currentL.yOff;
        int xStart = leftEyeX + (int)gazeX - (int)(currentL.w / 2);
        int xEnd   = rightEyeX + (int)gazeX + (int)(currentR.w / 2);
        int hLine  = (int)currentL.h;
        if (hLine < 2) hLine = 2;
        disp->fillRoundRect(xStart, yLine - hLine / 2, xEnd - xStart, hLine, 1, SSD1306_WHITE);
      } else {
        drawSingleEye(leftEyeX, centerY, currentL);
        drawSingleEye(rightEyeX, centerY, currentR);
      }

      if (useScanlines) {
        for (int y = 0; y < screenHeight; y += 3) {
          disp->drawFastHLine(0, y, screenWidth, SSD1306_BLACK);
        }
      }

      disp->display();
    }

  private:
    void drawSingleEye(int baseX, int baseY, const EyeState& st) {
      int w = (int)(st.w + 0.5f);
      int h = (int)(st.h + 0.5f);
      if (w < 4) w = 4;
      if (h < 2) h = 2;

      int r = (int)(st.r + 0.5f);
      if (r > w / 2) r = w / 2;
      if (r > h / 2) r = h / 2;

      int cx = baseX + (int)gazeX + (int)st.xOff;
      int cy = baseY + (int)gazeY + (int)st.yOff;

      int left   = cx - w / 2;
      int right  = left + w;
      int top    = cy - h / 2;
      int bottom = top + h;

      disp->fillRoundRect(left, top, w, h, r, SSD1306_WHITE);

      int tL = (int)(st.topL + 0.5f);
      int tR = (int)(st.topR + 0.5f);
      if (tL > 0 || tR > 0) {
        int x0 = left - 2;
        int x1 = right + 2;
        int yTop = top - 3;
        int yL = top + tL;
        int yR = top + tR;
        disp->fillTriangle(x0, yTop, x1, yTop, x1, yR, SSD1306_BLACK);
        disp->fillTriangle(x0, yTop, x0, yL,   x1, yR, SSD1306_BLACK);
      }

      int bL = (int)(st.botL + 0.5f);
      int bR = (int)(st.botR + 0.5f);
      if (bL > 0 || bR > 0) {
        int x0 = left - 2;
        int x1 = right + 2;
        int yBot = bottom + 3;
        int yL = bottom - bL;
        int yR = bottom - bR;
        disp->fillTriangle(x0, yBot, x1, yBot, x1, yR, SSD1306_BLACK);
        disp->fillTriangle(x0, yBot, x0, yL,   x1, yR, SSD1306_BLACK);
      }

      int arch = (int)(st.arch + 0.5f);
      if (arch > 0) {
        int activeBot = bottom - ((bL + bR) / 2);
        int archRadius = w + 6;
        int circleCenterY = activeBot + archRadius - arch;
        disp->fillCircle(cx, circleCenterY, archRadius, SSD1306_BLACK);
      }
    }
};

#endif