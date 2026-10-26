#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <DHT.h>

// =====================================================
// OLED SETTINGS
// =====================================================

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

// =====================================================
// DHT11 SETTINGS
// =====================================================

#define DHTPIN 4
#define DHTTYPE DHT11

DHT dht(DHTPIN, DHTTYPE);

Adafruit_SSD1306 display(
  SCREEN_WIDTH,
  SCREEN_HEIGHT,
  &Wire,
  -1
);

// =====================================================
// LED AND BUZZER PINS
// =====================================================

#define RED_LED 18
#define GREEN_LED 19
#define BUZZER 23

// =====================================================
// MUSICAL NOTES
// =====================================================

// Lower octave
#define NOTE_C3  131
#define NOTE_D3  147
#define NOTE_E3  165
#define NOTE_F3  175
#define NOTE_FS3 185
#define NOTE_G3  196
#define NOTE_A3  220
#define NOTE_AS3 233
#define NOTE_B3  247

// Middle octave
#define NOTE_C4  262
#define NOTE_CS4 277
#define NOTE_D4  294
#define NOTE_DS4 311
#define NOTE_E4  330
#define NOTE_F4  349
#define NOTE_FS4 370
#define NOTE_G4  392
#define NOTE_GS4 415
#define NOTE_A4  440
#define NOTE_AS4 466
#define NOTE_B4  494

// Higher octave
#define NOTE_C5  523
#define NOTE_CS5 554
#define NOTE_D5  587
#define NOTE_DS5 622
#define NOTE_E5  659
#define NOTE_FS5 740
#define NOTE_G5  784

// =====================================================
// MULT0-STYLE CHORD PROGRESSION
// E -> B -> G#m -> F#
//
// This is an original arpeggiated arrangement using
// the chord progression provided.
// =====================================================

int melody[] = {

  // -------------------------
  // E MAJOR
  // E - G# - B
  // -------------------------
  NOTE_E4,
  NOTE_GS4,
  NOTE_B4,
  NOTE_GS4,

  // -------------------------
  // B MAJOR
  // B - D# - F#
  // -------------------------
  NOTE_B3,
  NOTE_DS4,
  NOTE_FS4,
  NOTE_DS4,

  // -------------------------
  // G# MINOR
  // G# - B - D#
  // -------------------------
  NOTE_GS4,
  NOTE_B4,
  NOTE_DS5,
  NOTE_B4,

  // -------------------------
  // F# MAJOR
  // F# - A# - C#
  // -------------------------
  NOTE_FS4,
  NOTE_AS4,
  NOTE_CS5,
  NOTE_AS4
};

// =====================================================
// NOTE DURATIONS
//
// Approximately 75 BPM.
// Each chord has four notes.
// =====================================================

int noteDuration[] = {

  // E
  200,
  200,
  200,
  200,

  // B
  200,
  200,
  200,
  200,

  // G#m
  200,
  200,
  200,
  200,

  // F#
  200,
  200,
  200,
  200
};

const int melodyLength =
  sizeof(melody) / sizeof(melody[0]);

// =====================================================
// MUSIC VARIABLES
// =====================================================

int currentNote = 0;

unsigned long previousNoteMillis = 0;

bool musicPlaying = false;
bool redLedState = false;

// =====================================================
// DHT11 TIMER
// =====================================================

unsigned long previousDHTMillis = 0;

const unsigned long dhtInterval = 2000;

float temperature = 0;
float humidity = 0;

// =====================================================
// GREEN LED SETTINGS
// =====================================================

unsigned long previousGreenMillis = 0;

bool greenLedState = false;

const unsigned long greenInterval = 1000;

// =====================================================
// SETUP
// =====================================================

void setup() {

  Serial.begin(115200);

  // Start DHT11
  dht.begin();

  // LED and buzzer
  pinMode(RED_LED, OUTPUT);
  pinMode(GREEN_LED, OUTPUT);
  pinMode(BUZZER, OUTPUT);

  // Everything OFF
  digitalWrite(RED_LED, LOW);
  digitalWrite(GREEN_LED, LOW);

  noTone(BUZZER);

  // ===================================================
  // START OLED
  // ===================================================

  if (!display.begin(
        SSD1306_SWITCHCAPVCC,
        0x3C
      )) {

    Serial.println("OLED not found!");

    while (true);
  }

  display.clearDisplay();

  display.setTextSize(1);
  display.setTextColor(WHITE);

  display.setCursor(25, 25);
  display.println("Starting...");

  display.display();

  delay(1000);
}

// =====================================================
// START MUSIC
// =====================================================

void startMusic() {

  currentNote = 0;

  previousNoteMillis = millis();

  musicPlaying = true;

  // Turn red LED ON with first beat
  redLedState = true;

  digitalWrite(
    RED_LED,
    HIGH
  );

  // Play first note
  tone(
    BUZZER,
    melody[currentNote]
  );
}

// =====================================================
// STOP MUSIC
// =====================================================

void stopMusic() {

  musicPlaying = false;

  // Stop buzzer
  noTone(BUZZER);

  // Stop red LED
  digitalWrite(
    RED_LED,
    LOW
  );

  redLedState = false;

  currentNote = 0;
}

// =====================================================
// UPDATE MUSIC
// =====================================================

void updateMusic() {

  if (!musicPlaying) {
    return;
  }

  unsigned long currentMillis = millis();

  // Check note timing
  if (
    currentMillis - previousNoteMillis
    >= noteDuration[currentNote]
  ) {

    previousNoteMillis = currentMillis;

    // Move to next note
    currentNote++;

    // Restart progression
    if (
      currentNote >= melodyLength
    ) {

      currentNote = 0;
    }

    // Play next note
    tone(
      BUZZER,
      melody[currentNote]
    );

    // =================================================
    // RED LED SYNCHRONIZED WITH NOTES
    // =================================================

    redLedState = !redLedState;

    digitalWrite(
      RED_LED,
      redLedState
    );
  }
}

// =====================================================
// GREEN LED - NORMAL TEMPERATURE
// =====================================================

void updateGreenLED() {

  unsigned long currentMillis = millis();

  if (
    currentMillis - previousGreenMillis
    >= greenInterval
  ) {

    previousGreenMillis = currentMillis;

    greenLedState = !greenLedState;

    digitalWrite(
      GREEN_LED,
      greenLedState
    );
  }
}

// =====================================================
// UPDATE OLED
// =====================================================

void updateOLED() {

  display.clearDisplay();

  display.setTextSize(1);
  display.setTextColor(WHITE);

  // ===================================================
  // TITLE
  // ===================================================

  display.setCursor(18, 0);

  display.println(
    "ESP32 DASHBOARD"
  );

  display.drawLine(
    0,
    10,
    128,
    10,
    WHITE
  );

  // ===================================================
  // TEMPERATURE
  // ===================================================

  display.setCursor(0, 20);

  display.print("Temp : ");

  display.print(
    temperature,
    1
  );

  display.println(" C");

  // ===================================================
  // HUMIDITY
  // ===================================================

  display.setCursor(0, 40);

  display.print("Hum  : ");

  display.print(
    humidity,
    1
  );

  display.println("%");

  // ===================================================
  // STATUS
  // ===================================================

  display.setCursor(0, 55);

  if (
    temperature > 24.0
  ) {

    display.print(
      "STATUS: HOT!"
    );

  } else {

    display.print(
      "STATUS: NORMAL"
    );
  }

  display.display();
}

// =====================================================
// MAIN LOOP
// =====================================================

void loop() {

  // ===================================================
  // READ DHT11 EVERY 2 SECONDS
  // ===================================================

  if (
    millis() - previousDHTMillis >= dhtInterval
    ||
    previousDHTMillis == 0
  ) {

    previousDHTMillis = millis();

    // Read temperature
    temperature =
      dht.readTemperature();

    // Read humidity
    humidity =
      dht.readHumidity();

    // =================================================
    // CHECK DHT11
    // =================================================

    if (
      isnan(temperature)
      ||
      isnan(humidity)
    ) {

      Serial.println(
        "Failed to read from DHT11!"
      );

      return;
    }

    // =================================================
    // SERIAL MONITOR
    // =================================================

    Serial.print(
      "Temperature: "
    );

    Serial.print(
      temperature
    );

    Serial.println(
      " °C"
    );

    Serial.print(
      "Humidity: "
    );

    Serial.print(
      humidity
    );

    Serial.println(
      " %"
    );

    // =================================================
    // HOT TEMPERATURE
    // =================================================

    if (
      temperature > 24.0
    ) {

      Serial.println(
        "STATUS: HOT"
      );

      Serial.println(
        "E - B - G#m - F# rhythm ON!"
      );

      // Green LED OFF
      digitalWrite(
        GREEN_LED,
        LOW
      );

      greenLedState = false;

      // Start music
      if (!musicPlaying) {

        startMusic();
      }
    }

    // =================================================
    // NORMAL TEMPERATURE
    // =================================================

    else {

      Serial.println(
        "STATUS: NORMAL"
      );

      // Stop music
      if (musicPlaying) {

        stopMusic();
      }

      // Red LED OFF
      digitalWrite(
        RED_LED,
        LOW
      );

      redLedState = false;
    }

    // Update OLED
    updateOLED();
  }

  // ===================================================
  // HOT = PLAY E-B-G#m-F# PROGRESSION
  // ===================================================

  if (
    temperature > 24.0
  ) {

    updateMusic();
  }

  // ===================================================
  // NORMAL = GREEN LED
  // ===================================================

  else {

    updateGreenLED();
  }
}