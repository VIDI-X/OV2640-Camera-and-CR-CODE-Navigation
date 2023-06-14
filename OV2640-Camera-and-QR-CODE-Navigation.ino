#include <FS.h>
#include <SPIFFS.h>
#include <SPI.h>
#include "Adafruit_ILI9341.h"
#include "Adafruit_GFX.h"

#include <ESP32QRCodeReader.h>

#define CAMERA_MODEL_VIDI_X \
  {                         \
    .PWDN_GPIO_NUM = 14,    \
    .RESET_GPIO_NUM = -1,   \
    .XCLK_GPIO_NUM = 0,     \
    .SIOD_GPIO_NUM = 2,     \
    .SIOC_GPIO_NUM = 27,    \
    .Y9_GPIO_NUM = 35,      \
    .Y8_GPIO_NUM = 34,      \
    .Y7_GPIO_NUM = 39,      \
    .Y6_GPIO_NUM = 36,      \
    .Y5_GPIO_NUM = 13,      \
    .Y4_GPIO_NUM = 19,      \
    .Y3_GPIO_NUM = 12,      \
    .Y2_GPIO_NUM = 15,      \
    .VSYNC_GPIO_NUM = 33,   \
    .HREF_GPIO_NUM = 4,     \
    .PCLK_GPIO_NUM = 22,    \
  }

#define TFT_DC 21 // Data/Command VIDI X zaslona spojen je na PIN 21
#define TFT_CS 5  // Chip select VIDI X zaslona spojen je na PIN 5

// stvaranje TFT objekta (zaslon)
Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC);

ESP32QRCodeReader reader(CAMERA_MODEL_VIDI_X);
struct QRCodeData qrCodeData;

#define speed 25  // Less is Fasster

#define CELL 8    // Definiramo veličinu jedne čelije u pixelaima
#define RAW 33    // Broj redova naše igre
#define COL 33   // Definiramo broj stupaca naše igre
#define ssf 9     // 320 pixels / COL

#define Player 9  // Naš igrač
#define wall 1    // zid
#define empty 0   // prazan prolaz

int startX = 3;
int startY = 6;

// ovdije moramo definirati niz
// i popuniti ga s poćenim stanjem svijeta
uint8_t world[COL][RAW] = { 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
                            1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
                            1, 1, 1, 0, 0, 0, 1, 1, 1, 0, 0, 0, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 1, 1,
                            1, 1, 1, 0, 0, 0, 9, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 1, 1,
                            1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 1, 1,
                            1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 1, 1,
                            1, 1, 0, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 0, 1, 1, 0, 0, 0, 0, 1, 1,
                            1, 1, 0, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 0, 1, 1, 0, 0, 0, 0, 1, 1,
                            1, 1, 0, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 1, 1,
                            1, 1, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1,
                            1, 1, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1,
                            1, 1, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 1, 1,
                            1, 1, 0, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 1, 1,
                            1, 1, 0, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 1, 1,
                            1, 1, 0, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 1, 1, 
                            1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 0, 0, 0, 0, 0, 0, 1, 1, 1, 0, 0, 0, 0, 1, 1,
                            1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 0, 0, 0, 0, 0, 0, 1, 1, 1, 0, 0, 0, 0, 1, 1,
                            1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 0, 0, 0, 0, 0, 0, 1, 1, 1, 0, 0, 0, 0, 1, 1,
                            1, 1, 0, 1, 1, 1, 1, 1, 1, 0, 0, 0, 1, 1, 1, 1, 1, 1, 0, 0, 0, 1, 1, 1, 0, 1, 1, 0, 0, 0, 0, 1, 1, 
                            1, 1, 0, 1, 1, 1, 1, 1, 1, 0, 0, 0, 1, 1, 1, 1, 1, 1, 0, 0, 0, 1, 1, 1, 0, 1, 1, 0, 0, 0, 0, 1, 1,
                            1, 1, 0, 1, 1, 1, 1, 1, 1, 0, 0, 0, 1, 1, 1, 1, 1, 1, 0, 0, 0, 1, 1, 1, 0, 1, 1, 0, 0, 0, 0, 1, 1,
                            1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 1, 1,
                            1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 1, 1,
                            1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 1, 1,
                            1, 1, 0, 1, 1, 1, 1, 1, 1, 0, 0, 0, 1, 1, 1, 0, 0, 0, 1, 1, 1, 0, 0, 0, 1, 1, 1, 0, 0, 0, 0, 1, 1,
                            1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
                            1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
                          };

void drawWorld() {
  for (int y = 0; y < RAW; y++) {
    for (int x = 0; x < COL; x++) {
      if (world[y][x] == wall) {
        tft.fillRect(x * ssf, y * ssf, CELL, CELL, ILI9341_BLUE);
      } else if (world[y][x] == empty) {
        tft.fillRect(x * ssf, y * ssf, CELL, CELL, ILI9341_BLACK);
      } else if (world[y][x] == Player) {
        tft.fillRect(x * ssf, y * ssf, CELL, CELL, ILI9341_RED);
      }
    }
  }
}

void go_down() {
  for (int y = RAW - 1; y > 0; y--) {
    for (int x = 0; x < COL; x++) {
      if (world[y][x] == empty && world[y - 1][x] == Player) {
        world[y][x] = Player;
        world[y - 1][x] = empty;
      }
    }
  }
}

void go_up() {
  for (int y = 0; y < RAW - 1; y++) {
    for (int x = 0; x < COL; x++) {
      if (world[y][x] == empty && world[y + 1][x] == Player) {
        world[y][x] = Player;
        world[y + 1][x] = empty;
      }
    }
  }
}

void go_left() {
  for (int y = 0; y < RAW; y++) {
    for (int x = 0; x < COL - 1; x++) {
      if (world[y][x] == empty && world[y][x + 1] == Player) {
        world[y][x] = Player;
        world[y][x + 1] = empty;
      }
    }
  }
}

void go_right() {
  for (int y = 0; y < RAW; y++) {
    for (int x = COL - 1; x > 0; x--) {
      if (world[y][x] == empty && world[y][x - 1] == Player) {
        world[y][x] = Player;
        world[y][x - 1] = empty;
      }
    }
  }
}

void setup() {
  Serial.begin(115200);

  reader.setup();
  //reader.setDebug(true);
  Serial.println("Setup QRCode Reader");

  reader.begin();
  Serial.println("Begin QR Code reader");

  world[startX][startY] = Player;

  SPIFFS.begin(true);

  // inicijalizacija zaslona
  tft.begin();
  // postavljamo orijentaciju ekzaslonarana
  tft.setRotation(3);
  // definiramo boju pozadine (crna troši najmanje struje)
  tft.fillScreen(ILI9341_BLACK);
  // pozvamo funkciju za iscrtavanje mreže
  drawWorld();
}

void callCodeChecker(String code) {
  if (code == "GO-LEFT-VIDI-X") {
    Serial.println("GO-LEFT-VIDI-X");
    go_left();
  }
  else if (code == "GO-RIGHT-VIDI-X") {
    Serial.println("GO-RIGHT-VIDI-X");
    go_right();
  }
  else if (code == "GO-UP-VIDI-X") {
    Serial.println("GO-UP-VIDI-X");
    go_up();
  }
  else if (code == "GO-DOWN-VIDI-X") {
    Serial.println("GO-DOWN-VIDI-X");
    go_down();
  }
  else {
    Serial.println("###########################");
    Serial.println("## Unrecongized command! ##");
    Serial.println("###########################");
  }
}

void loop() {
  if (reader.receiveQrCode(&qrCodeData, 100)) {
    Serial.print("Found QRCode ");
    if (qrCodeData.valid) {
      Serial.print("Payload: ");
      Serial.println((const char *)qrCodeData.payload);
      callCodeChecker(String((const char *)qrCodeData.payload));
      drawWorld();
    }
    else {
      Serial.print("Invalid: ");
      Serial.println((const char *)qrCodeData.payload);
    }
  }
}
