#ifndef ARCADE_PANEL_H 
#define ARCADE_PANEL_H

#include <Arduino.h>
#include <seesaw_neopixel.h>
#include "Adafruit_seesaw.h"

#include <Wire.h>
#include <Adafruit_GFX.h>
#include "Adafruit_LEDBackpack.h"

#define DEFAULT_I2C_ADDR 0x3A

#define NUMBER_OF_BUTTONS 4
#define NONE 0
#define YELLOW (1 << 0)
#define GREEN (1 << 1)
#define RED (1 << 2)
#define BLUE (1 << 3)



// Button pin definitions
#define BUTTON_YELLOW 18
#define LED_YELLOW  12

#define BUTTON_GREEN  19
#define LED_GREEN   13

#define BUTTON_RED    20
#define LED_RED     0

#define BUTTON_BLUE   2
#define LED_BLUE    1

// Speaker/Buzzer definitions
#define BUZZER1  4
#define BUZZER2  7

#define TONE_DURATION_DEFAULT 150 

// Toggle Switch Pins
#define SOUNDS_TOGGLE 9
#define LIGHTS_TOGGLE 11

// Alphanumeric Display Settings
#define LEFT_DISPLAY_ADDR 0x70  // I2C default address for LED backpack
#define RIGHT_DISPLAY_ADDR 0x71 // I2C address with A0 shorted
#define DEFAULT_DISPLAY_BRIGHTNESS 1

struct Button {
  byte pin;
  byte led;
  byte color;
  int tone;
};

class Panel {
  
private:
  Button _yellow;
  Button _green;
  Button _red;
  Button _blue;
  bool _sounds;
  bool _lights;
  int _toneDuration;
  void _writeChar(int pos, char c);

public:
  Panel();
  void setup(void);
  void setSpeed(int setting);
  int duration();

  bool lightsOn();
  bool soundsOn();
  void buttonLights(byte colors);
  void buttonTone(byte colors);
  byte nextButton(byte colors);
  void pulse(byte colors, int duration_ms);
  void buzz(int frequency, int duation_ms);
  byte pressed(void);
  void alert(byte colors);
  void playGameOver();
  void playWinnerSound();
  void clearDisplay();
  void showMessage(char *msg);
};

#endif
