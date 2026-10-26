
#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ILI9341.h>
#include <XPT2046_Touchscreen.h>

// ==========================================
// BSIT PIXEL PET - CHARACTER & SKIN EDITION
// ==========================================

#define TFT_CS    5
#define TFT_DC    27
#define TFT_RST   4

#define TOUCH_CS  14
#define TOUCH_IRQ 22

#define SPI_SCK   18
#define SPI_MISO  19
#define SPI_MOSI  23

#define SCREEN_W 320
#define SCREEN_H 240

Adafruit_ILI9341 tft(TFT_CS, TFT_DC, TFT_RST);
XPT2046_Touchscreen touch(TOUCH_CS, TOUCH_IRQ);

#define BG       0x0841
#define WHITE    0xFFFF
#define BLACK    0x0000
#define CYAN     0x07FF
#define YELLOW   0xFFE0
#define GREEN    0x07E0
#define RED      0xF800
#define PINK     0xF81F
#define BLUE     0x051F
#define ORANGE   0xFD20
#define PURPLE   0x801F
#define GRAY     0x8410

// ==========================================
// GAME DATA
// ==========================================

// ===== PHYSICAL BUTTONS =====
#define BTN_ACTION 25
#define BTN_LEFT   26
#define BTN_RIGHT  32
#define BTN_MENU   33
#define BTN_BACK   13

const unsigned long BUTTON_DEBOUNCE_MS = 180;

const int buttonPins[] = {
  BTN_ACTION, BTN_LEFT, BTN_RIGHT, BTN_MENU, BTN_BACK
};

bool lastButtonReading[5] = {HIGH, HIGH, HIGH, HIGH, HIGH};
bool stableButtonState[5] = {HIGH, HIGH, HIGH, HIGH, HIGH};
unsigned long lastButtonChange[5] = {0, 0, 0, 0, 0};

void setupButtons() {
  for (int i = 0; i < 5; i++) {
    pinMode(buttonPins[i], INPUT_PULLUP);
  }
}

// Returns true once when a button is pressed
bool buttonPressed(int index) {
  bool reading = digitalRead(buttonPins[index]);

  if (reading != lastButtonReading[index]) {
    lastButtonChange[index] = millis();
    lastButtonReading[index] = reading;
  }

  if (millis() - lastButtonChange[index] >= BUTTON_DEBOUNCE_MS) {
    if (reading != stableButtonState[index]) {
      stableButtonState[index] = reading;

      if (stableButtonState[index] == LOW) {
        return true;
      }
    }
  }

  return false;
}
int hunger = 20;
int happiness = 80;
int energy = 80;

int xp = 0;
int level = 1;
int age = 0;

int currentCharacter = 0;
int currentSkin = 0;
int animationFrame = 0;
int taskPage = 0;

bool sleeping = false;
bool playing = false;

unsigned long lastNeedUpdate = 0;
unsigned long lastAnimation = 0;
unsigned long lastAgeUpdate = 0;
unsigned long noticeUntil = 0;

String noticeText = "";

enum ScreenMode {
  HOME,
  TASKS,
  CHARACTERS,
  SKINS,
  NOTICE
};

ScreenMode screenMode = HOME;

// ==========================================
// CHARACTERS
// ==========================================

const char* characterNames[] = {
  "CAT CODER",
  "CYBER BOT",
  "NINJA HACKER",
  "GOLD DRAGON",
  "GHOST PROGRAMMER"
};

const int characterXP[] = {
  0, 30, 70, 120, 180
};

const int CHARACTER_COUNT = 5;

// Each character has a different 16 x 16 sprite.

const char* catSprite[16] = {
  "....##....##....",
  "...###....###...",
  "..####....####..",
  "..############..",
  ".##############.",
  ".##..######..##.",
  "################",
  "################",
  "################",
  "################",
  "################",
  ".##############.",
  "..############..",
  "...##########...",
  "....########....",
  ".....######....."
};

const char* botSprite[16] = {
  ".....######.....",
  "....########....",
  "...##########...",
  "..############..",
  "..##..####..##..",
  "..##..####..##..",
  "################",
  "################",
  "##..########..##",
  "##..########..##",
  "################",
  "...##########...",
  "...##......##...",
  "...##########...",
  "....########....",
  ".....######....."
};

const char* ninjaSprite[16] = {
  ".....######.....",
  "....########....",
  "...##########...",
  "..############..",
  ".##############.",
  "################",
  "##..########..##",
  "################",
  "################",
  ".##############.",
  "..############..",
  "...##########...",
  "....########....",
  "...##########...",
  "..############..",
  ".##############."
};

const char* dragonSprite[16] = {
  "##............##",
  "###..........###",
  ".###........###.",
  "..############..",
  "...##########...",
  "..############..",
  ".##############.",
  "################",
  "################",
  "####..####..####",
  "################",
  ".##############.",
  "..############..",
  "...##########...",
  "....########....",
  ".....######....."
};

const char* ghostSprite[16] = {
  ".....######.....",
  "...##########...",
  "..############..",
  ".##############.",
  "################",
  "################",
  "##..########..##",
  "##..########..##",
  "################",
  "################",
  "################",
  "################",
  "##..##..##..##..",
  "##..##..##..##..",
  "....##....##....",
  "....##....##...."
};

const char** characterSprites[] = {
  catSprite,
  
  botSprite,
  ninjaSprite,
  dragonSprite,
  ghostSprite
};

// ==========================================
// UNLOCKABLE COLOR SKINS
// ==========================================

const char* skinNames[] = {
  "CLASSIC",
  "OCEAN",
  "CYBER PURPLE",
  "GOLD",
  "GHOST ICE"
};

const uint16_t skinColors[] = {
  YELLOW,
  BLUE,
  PURPLE,
  ORANGE,
  CYAN
};

const int skinXP[] = {
  0, 20, 50, 90, 140
};

const int SKIN_COUNT = 5;

// ==========================================
// BSIT STUDY TASKS
// ==========================================

struct StudyTask {
  const char* name;
  int reward;
};

StudyTask studyTasks[] = {
  {"CODE PRACTICE", 10},
  {"DEBUG A BUG", 15},
  {"SQL PRACTICE", 10},
  {"NETWORKING", 10},
  {"READ DOCUMENTS", 5},
  {"BUILD A PROJECT", 20}
};

const int TASK_COUNT = 6;

// ==========================================
// UI HELPERS
// ==========================================

void drawHeader(const char* title) {
  tft.fillScreen(BG);
  tft.fillRect(0, 0, SCREEN_W, 28, BLUE);

  tft.setTextSize(2);
  tft.setTextColor(WHITE);
  tft.setCursor(7, 7);
  tft.print(title);
}

void drawButton(int x, int y, int w, int h,
                uint16_t color, const char* label) {
  tft.fillRoundRect(x, y, w, h, 5, color);
  tft.drawRoundRect(x, y, w, h, 5, WHITE);

  tft.setTextSize(1);
  tft.setTextColor(WHITE);

  int16_t bx, by;
  uint16_t bw, bh;

  tft.getTextBounds(label, 0, 0, &bx, &by, &bw, &bh);

  tft.setCursor(
    x + (w - bw) / 2,
    y + (h - bh) / 2
  );

  tft.print(label);
}

void drawBar(int x, int y, const char* label,
             int value, uint16_t color) {
  value = constrain(value, 0, 100);

  tft.setTextSize(1);
  tft.setTextColor(WHITE);
  tft.setCursor(x, y);
  tft.print(label);

  tft.drawRect(x, y + 12, 100, 10, WHITE);
  tft.fillRect(x + 2, y + 14, 96, 6, BG);

  tft.fillRect(
    x + 2, y + 14,
    value * 96 / 100, 6, color
  );
}

// ==========================================
// DRAW SELECTED CHARACTER
// ==========================================

void drawCharacter(int x, int y, int scale,
                   int character, int skin) {
  const char** sprite = characterSprites[character];
  uint16_t body = skinColors[skin];

  for (int row = 0; row < 16; row++) {
    for (int col = 0; col < 16; col++) {
      if (sprite[row][col] == '#') {
        tft.fillRect(
          x + col * scale,
          y + row * scale,
          scale, scale, body
        );
      }
    }
  }

  // Draw eyes.
  int eyeY = y + 7 * scale;

  if (sleeping) {
    tft.fillRect(x + 4 * scale, eyeY,
                 2 * scale, scale, BG);
    tft.fillRect(x + 10 * scale, eyeY,
                 2 * scale, scale, BG);
  } else {
    tft.fillRect(x + 4 * scale, eyeY,
                 scale, scale, BLACK);
    tft.fillRect(x + 11 * scale, eyeY,
                 scale, scale, BLACK);

    tft.fillRect(x + 7 * scale, y + 10 * scale,
                 3 * scale, scale, BLACK);
  }

  tft.fillRect(x + 2 * scale, y + 9 * scale,
               2 * scale, scale, PINK);
  tft.fillRect(x + 12 * scale, y + 9 * scale,
               2 * scale, scale, PINK);

  if (sleeping) {
    tft.setTextSize(2);
    tft.setTextColor(CYAN);
    tft.setCursor(x + 35, y - 10);
    tft.print("Z");
  }
}

// ==========================================
// XP AND UNLOCK SYSTEM
// ==========================================

void addXP(int amount) {
  int oldLevel = level;

  xp += amount;
  level = 1 + xp / 50;

  Serial.print("XP TOTAL: ");
  Serial.println(xp);

  Serial.print("LEVEL: ");
  Serial.println(level);

  // Unlock the next character if a threshold is reached.
  for (int i = 1; i < CHARACTER_COUNT; i++) {
    if (xp >= characterXP[i] &&
        xp - amount < characterXP[i]) {
      noticeText = String("CHARACTER UNLOCKED!");
      screenMode = NOTICE;
      noticeUntil = millis() + 2000;

      Serial.print("UNLOCKED: ");
      Serial.println(characterNames[i]);
    }
  }

  // Unlockable skins use separate XP milestones.
  for (int i = 1; i < SKIN_COUNT; i++) {
    if (xp >= skinXP[i] &&
        xp - amount < skinXP[i]) {
      noticeText = String("SKIN UNLOCKED!");
      screenMode = NOTICE;
      noticeUntil = millis() + 2000;

      Serial.print("UNLOCKED SKIN: ");
      Serial.println(skinNames[i]);
    }
  }

  if (level > oldLevel) {
    noticeText = String("LEVEL UP! LEVEL ") + level;
    screenMode = NOTICE;
    noticeUntil = millis() + 2000;
  }
}

// ==========================================
// HOME SCREEN
// ==========================================

void drawHome() {
  drawHeader("BSIT PIXEL PET");

  drawBar(8, 36, "HUNGER", 100 - hunger, GREEN);
  drawBar(8, 67, "HAPPY", happiness, PINK);
  drawBar(8, 98, "ENERGY", energy, CYAN);

  tft.setTextSize(1);
  tft.setTextColor(WHITE);

  tft.setCursor(8, 132);
  tft.print("LEVEL: ");
  tft.print(level);

  tft.setCursor(8, 145);
  tft.print("XP: ");
  tft.print(xp);

  tft.setCursor(8, 158);
  tft.print("AGE: ");
  tft.print(age);

  tft.setCursor(8, 171);
  tft.print("CHARACTER:");

  tft.setCursor(8, 181);
  tft.print(characterNames[currentCharacter]);

  int bounce = sleeping ? 0 : (animationFrame % 2) * 2;

  drawCharacter(
    230, 65 + bounce, 3,
    currentCharacter, currentSkin
  );

  drawButton(3, 193, 76, 39, GREEN, "FEED");
  drawButton(83, 193, 76, 39, PINK, "TASKS");
  drawButton(163, 193, 76, 39, PURPLE, "CHARS");
  drawButton(243, 193, 74, 39, BLUE, "SKINS");
}

// ==========================================
// TASK MENU
// ==========================================

void drawTasks() {
  drawHeader("BSIT TASKS");

  int start = taskPage * 3;

  for (int i = 0; i < 3; i++) {
    int idx = start + i;
    if (idx >= TASK_COUNT) break;

    int y = 37 + i * 48;

    drawButton(5, y, 310, 40, BLUE,
               studyTasks[idx].name);

    tft.setTextSize(1);
    tft.setTextColor(YELLOW);
    tft.setCursor(12, y + 29);
    tft.print("+");
    tft.print(studyTasks[idx].reward);
    tft.print(" XP");
  }

  drawButton(5, 190, 98, 43, GRAY, "PAGE 1");
  drawButton(111, 190, 98, 43, GREEN, "PAGE 2");
  drawButton(217, 190, 98, 43, RED, "BACK");
}

// ==========================================
// CHARACTER SELECT MENU
// ==========================================

void drawCharacters() {
  drawHeader("CHARACTERS");

  for (int i = 0; i < CHARACTER_COUNT; i++) {
    int y = 34 + i * 30;

    uint16_t color = (i == currentCharacter)
                     ? GREEN : BLUE;

    drawButton(5, y, 310, 26, color,
               characterNames[i]);

    tft.setTextSize(1);
    tft.setTextColor(WHITE);
    tft.setCursor(10, y + 9);

    if (xp >= characterXP[i]) {
      tft.print(i == currentCharacter
                ? "[USING]" : "[UNLOCKED]");
    } else {
      tft.print("LOCKED ");
      tft.print(characterXP[i]);
      tft.print(" XP");
    }
  }

  drawButton(5, 190, 310, 43, RED, "BACK");
}

// ==========================================
// SKIN SELECT MENU
// ==========================================

void drawSkins() {
  drawHeader("CHANGE SKIN");

  for (int i = 0; i < SKIN_COUNT; i++) {
    int y = 34 + i * 30;

    drawButton(5, y, 310, 26,
               skinColors[i], skinNames[i]);

    tft.setTextSize(1);
    tft.setTextColor(WHITE);
    tft.setCursor(10, y + 9);

    if (xp >= skinXP[i]) {
      tft.print(i == currentSkin
                ? "[EQUIPPED]" : "[UNLOCKED]");
    } else {
      tft.print("LOCKED ");
      tft.print(skinXP[i]);
      tft.print(" XP");
    }
  }

  drawButton(5, 190, 310, 43, RED, "BACK");
}

// ==========================================
// SCREEN RENDERING
// ==========================================

void showScreen() {
  switch (screenMode) {
    case HOME:
      drawHome();
      break;

    case TASKS:
      drawTasks();
      break;

    case CHARACTERS:
      drawCharacters();
      break;

    case SKINS:
      drawSkins();
      break;

    case NOTICE:
      drawHeader("REWARD!");

      tft.setTextSize(2);
      tft.setTextColor(YELLOW);
      tft.setCursor(10, 80);
      tft.print(noticeText);

      tft.setTextSize(1);
      tft.setTextColor(WHITE);
      tft.setCursor(10, 120);
      tft.print("Keep completing BSIT tasks!");

      drawButton(35, 175, 250, 45,
                 GREEN, "CONTINUE");
      break;
  }
}

// ==========================================
// TASK COMPLETION
// ==========================================

void completeTask(int idx) {
  if (idx < 0 || idx >= TASK_COUNT) return;

  addXP(studyTasks[idx].reward);

  happiness = min(100, happiness + 8);
  energy = max(0, energy - 5);
  hunger = min(100, hunger + 2);

  Serial.print("TASK COMPLETED: ");
  Serial.println(studyTasks[idx].name);

  if (screenMode != NOTICE) {
    noticeText = String("+") +
                 studyTasks[idx].reward + " XP!";
    screenMode = NOTICE;
    noticeUntil = millis() + 1800;
  }

  showScreen();
}

// ==========================================
// PET CARE
// ==========================================

void feedPet() {
  hunger = max(0, hunger - 25);
  happiness = min(100, happiness + 5);
  sleeping = false;
  playing = false;

  showScreen();
}

// ==========================================
// TOUCH HANDLER
// ==========================================

void handleTouch() {
  if (!touch.touched()) return;

  TS_Point p = touch.getPoint();

  // Starter calibration. Adjust if needed.
  int x = map(p.x, 200, 3900, 0, SCREEN_W - 1);
  int y = map(p.y, 200, 3900, 0, SCREEN_H - 1);

  x = constrain(x, 0, SCREEN_W - 1);
  y = constrain(y, 0, SCREEN_H - 1);

  Serial.print("Touch X=");
  Serial.print(x);
  Serial.print(" Y=");
  Serial.println(y);

  if (screenMode == HOME) {
    if (y >= 190) {
      if (x < 81) {
        feedPet();
      } else if (x < 161) {
        taskPage = 0;
        screenMode = TASKS;
        showScreen();
      } else if (x < 241) {
        screenMode = CHARACTERS;
        showScreen();
      } else {
        screenMode = SKINS;
        showScreen();
      }
    }
  }

  else if (screenMode == TASKS) {
    if (y >= 190) {
      if (x < 107) {
        taskPage = 0;
        showScreen();
      } else if (x < 213) {
        taskPage = 1;
        showScreen();
      } else {
        screenMode = HOME;
        showScreen();
      }
    } else if (y >= 37 && y < 181) {
      int row = (y - 37) / 48;
      int idx = taskPage * 3 + row;
      completeTask(idx);
    }
  }

  else if (screenMode == CHARACTERS) {
    if (y >= 190) {
      screenMode = HOME;
      showScreen();
    } else if (y >= 34 && y < 184) {
      int idx = (y - 34) / 30;

      if (idx >= 0 && idx < CHARACTER_COUNT) {
        if (xp >= characterXP[idx]) {
          currentCharacter = idx;

          noticeText = String("CHARACTER SELECTED!");
          screenMode = NOTICE;
          noticeUntil = millis() + 1500;
          showScreen();
        } else {
          noticeText = String("NEED ") +
                       (characterXP[idx] - xp) + " MORE XP";
          screenMode = NOTICE;
          noticeUntil = millis() + 1800;
          showScreen();
        }
      }
    }
  }

  else if (screenMode == SKINS) {
    if (y >= 190) {
      screenMode = HOME;
      showScreen();
    } else if (y >= 34 && y < 184) {
      int idx = (y - 34) / 30;

      if (idx >= 0 && idx < SKIN_COUNT) {
        if (xp >= skinXP[idx]) {
          currentSkin = idx;

          noticeText = String("SKIN EQUIPPED!");
          screenMode = NOTICE;
          noticeUntil = millis() + 1500;
        } else {
          noticeText = String("NEED ") +
                       (skinXP[idx] - xp) + " MORE XP";
          screenMode = NOTICE;
          noticeUntil = millis() + 1800;
        }

        showScreen();
      }
    }
  }

  else if (screenMode == NOTICE) {
    screenMode = HOME;
    showScreen();
  }

  // Wait for touch release.
  while (touch.touched()) {
    delay(10);
  }

  delay(80);
}

// ==========================================
// PET NEEDS
// ==========================================

void updateNeeds() {
  unsigned long now = millis();

  if (now - lastNeedUpdate >= 10000UL) {
    lastNeedUpdate = now;

    hunger = min(100, hunger + 2);

    if (sleeping) {
      energy = min(100, energy + 8);
      hunger = min(100, hunger + 1);
    } else {
      energy = max(0, energy - 1);
    }

    if (hunger >= 80 || energy <= 10) {
      happiness = max(0, happiness - 2);
    }

    if (screenMode == HOME) showScreen();
  }

  if (now - lastAgeUpdate >= 60000UL) {
    lastAgeUpdate = now;
    age++;

    if (screenMode == HOME) showScreen();
  }
}

// ==========================================
// SETUP
// ==========================================

void setup() {
  Serial.begin(115200);

  SPI.begin(SPI_SCK, SPI_MISO, SPI_MOSI);

  tft.begin();
  tft.setRotation(1);
  tft.fillScreen(BG);

  touch.begin();
  touch.setRotation(1);

  lastNeedUpdate = millis();
  lastAgeUpdate = millis();
  lastAnimation = millis();

  Serial.println("BSIT PIXEL PET READY!");
  Serial.println("Complete tasks to unlock characters and skins.");

  showScreen();
}

// ==========================================
// MAIN LOOP
// ==========================================

void loop() {
  handleTouch();
  updateNeeds();

  if (screenMode == NOTICE &&
      (long)(millis() - noticeUntil) >= 0) {
    screenMode = HOME;
    showScreen();
  }

  if (screenMode == HOME &&
      !sleeping &&
      millis() - lastAnimation >= 400UL) {
    lastAnimation = millis();
    animationFrame++;
    showScreen();
  }
}
