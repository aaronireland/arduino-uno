#include "arcade_panel.h"
#include "notes.h"


#include <Arduino.h>

Adafruit_seesaw seesaw;
Adafruit_AlphaNum4 displayL = Adafruit_AlphaNum4();
Adafruit_AlphaNum4 displayR = Adafruit_AlphaNum4();

Panel::Panel(){
  _yellow = {BUTTON_YELLOW, LED_YELLOW, YELLOW, AS7};
  _green = {BUTTON_GREEN, LED_GREEN, GREEN, CS7};
  _red = {BUTTON_RED, LED_RED, RED, F5};
  _blue = {BUTTON_BLUE, LED_BLUE, BLUE, AS4};
  _sounds = true;
  _lights = true;

  _toneDuration = TONE_DURATION_DEFAULT;
}

void Panel::setup(void) {
  Serial.println(F("~~ ARCADE PANEL SETUP ~~"));
  if (!seesaw.begin(DEFAULT_I2C_ADDR)) {
    Serial.println(F("seesaw not found!"));
    while(1) delay(10);
  } else {
    Serial.println(F("seesaw found!"));
  }

  Serial.println(_yellow.pin);
  seesaw.pinMode(_yellow.pin, INPUT_PULLUP);
  seesaw.pinMode(_yellow.led, OUTPUT);
  seesaw.digitalWrite(_yellow.led, LOW);

  seesaw.pinMode(_green.pin, INPUT_PULLUP);
  seesaw.pinMode(_green.led, OUTPUT);
  seesaw.digitalWrite(_green.led, LOW);

  seesaw.pinMode(_red.pin, INPUT_PULLUP);
  seesaw.pinMode(_red.led, OUTPUT);
  seesaw.digitalWrite(_red.led, LOW);

  seesaw.pinMode(_blue.pin, INPUT_PULLUP);
  seesaw.pinMode(_blue.led, OUTPUT);
  seesaw.digitalWrite(_blue.led, LOW);

  Serial.println(F("Buttons set..."));

  pinMode(BUZZER1, OUTPUT);
  pinMode(BUZZER2, OUTPUT);
  Serial.println(F("Buzzer set..."));

  pinMode(SOUNDS_TOGGLE, INPUT_PULLDOWN);
  pinMode(LIGHTS_TOGGLE, INPUT_PULLDOWN);
  Serial.println(F("Toggles set..."));

  displayL.begin(LEFT_DISPLAY_ADDR);
  displayR.begin(RIGHT_DISPLAY_ADDR);

  displayL.setBrightness(1);
  displayR.setBrightness(1);

  displayL.writeDigitAscii(0, '*');
  displayL.writeDigitAscii(1, '*');
  displayL.writeDigitAscii(2, '*');
  displayL.writeDigitAscii(3, '*');
  
  displayR.writeDigitAscii(0, '*');
  displayR.writeDigitAscii(1, '*');
  displayR.writeDigitAscii(2, '*');
  displayR.writeDigitAscii(3, '*');
  displayL.writeDisplay();
  displayR.writeDisplay();

  Serial.println(F("Alphanumeric displays set..."));
  Serial.println("~~ ARCADE PANEL SETUP COMPLETE ~~");
}

bool Panel::soundsOn(){
  byte soundsToggleState = digitalRead(SOUNDS_TOGGLE);
  _sounds = soundsToggleState == HIGH;
  return _sounds;
}

bool Panel::lightsOn(){
  byte lightsToggleState = digitalRead(LIGHTS_TOGGLE);
  _lights = lightsToggleState == HIGH;
  return _lights;
}

void Panel::setSpeed(int setting){
  float modifier = 0.33f * setting;
  _toneDuration = _toneDuration * modifier;
}

int Panel::duration(){
  return _toneDuration;
}

void Panel::buttonLights(byte colors) {
  if ((colors & _yellow.color) != 0)
    seesaw.digitalWrite(_yellow.led, HIGH);
  else
    seesaw.digitalWrite(_yellow.led, LOW);

  if ((colors & _green.color) != 0)
    seesaw.digitalWrite(_green.led, HIGH);
  else
    seesaw.digitalWrite(_green.led, LOW);

  if ((colors & _red.color) != 0)
    seesaw.digitalWrite(_red.led, HIGH);
  else
    seesaw.digitalWrite(_red.led, LOW);

  if ((colors & _blue.color) != 0)
    seesaw.digitalWrite(_blue.led, HIGH);
  else
    seesaw.digitalWrite(_blue.led, LOW);
}

byte Panel::nextButton(byte colors){
  byte next = NONE;
  switch(colors){
    case YELLOW:
      next = GREEN;
      break;
    case GREEN:
      next = RED;
      break;
    case RED:
      next = BLUE;
      break;
    case BLUE:
      next = YELLOW;
      break;
  }

  return next;
}

void Panel::pulse(byte colors, int duration_ms) {

  buttonLights(colors);
  delay(duration_ms);
  buttonLights(NONE);

}

void Panel::buzz(int frequency, int duration_ms) {
  long duration_micros = duration_ms * (long)1000;

  while (duration_micros >(frequency * 2))
  {
      duration_micros -= frequency * 2;

      digitalWrite(BUZZER1, LOW);
      digitalWrite(BUZZER2, HIGH);
      delayMicroseconds(frequency);

      digitalWrite(BUZZER1, HIGH);
      digitalWrite(BUZZER2, LOW);
      delayMicroseconds(frequency);
    }

  digitalWrite(BUZZER1, LOW);
  digitalWrite(BUZZER2, LOW);
}

void Panel::alert(byte colors) {
  if (_lights && colors != NONE) {
    buttonLights(colors);
  }

  if (_sounds) {
    buttonTone(colors);
    buttonLights(NONE);
    delay(_toneDuration / 2);
  } else {
    delay(_toneDuration);
    buttonLights(NONE);
    delay(_toneDuration);
  }

}

void Panel::buttonTone(byte colors) {
  switch(colors){
    case YELLOW:
      buzz(_yellow.tone, _toneDuration);
      break;
    case GREEN:
      buzz(_green.tone, _toneDuration);
      break;
    case RED:
      buzz(_red.tone, _toneDuration);
      break;
    case BLUE:
      buzz(_blue.tone, _toneDuration);
      break;
  }
}

byte Panel::pressed(void) {
  if (seesaw.digitalRead(_yellow.pin) == 0)
    return _yellow.color;

  if (seesaw.digitalRead(_green.pin) == 0)
    return _green.color;

  if (seesaw.digitalRead(_red.pin) == 0)
    return _red.color;

  if (seesaw.digitalRead(_blue.pin) == 0)
    return _blue.color;

  // No buttons have been pressed
  return NONE;
}

void Panel::playGameOver(){
  // sizeof gives the number of bytes, each int value is composed of two bytes (16 bits)
  // there are two values per note (pitch and duration), so for each note there are four bytes

  buttonLights(YELLOW | GREEN | RED | BLUE);
  buzz(D8, 550);
  delay(255);

  int tempo = 140;
  int melody[] = {
    C5,-4, G4,-4, E4,4, //45
    A4,-8, B4,-8, A4,-8, GS4,-8, AS4,-8, GS4,-8,
    G4,1,
  };

  int notes = sizeof(melody) / sizeof(melody[0]) / 2;
  // this calculates the duration of a whole note in ms
  int wholenote = (60000 * 2) / tempo;
  int divider = 0, noteDuration = 0;

  digitalWrite(BUZZER1, LOW);
  byte thisButton = YELLOW;

  // iterate over the notes of the melody. 
  // Remember, the array is twice the number of notes (notes + durations)
  for (int thisNote = 0; thisNote < notes * 2; thisNote = thisNote + 2) {

    thisButton = nextButton(thisButton);
    buttonLights(thisButton);
    // calculates the duration of each note
    divider = melody[thisNote + 1];
    if (divider > 0) {
      // regular note, just proceed
      noteDuration = (wholenote) / divider;
    } else if (divider < 0) {
      // dotted notes are represented with negative durations!!
      noteDuration = (wholenote) / abs(divider);
      noteDuration *= 1.5; // increases the duration in half for dotted notes
    }

    // we only play the note for 90% of the duration, leaving 10% as a pause
    tone(BUZZER1, melody[thisNote], noteDuration*0.9);

    // Wait for the specief duration before playing the next note.
    delay(noteDuration);
    // stop the waveform generation before the next note.
    noTone(BUZZER1);

    if (pressed() != NONE) return;
  }
}

void Panel::_writeChar(int pos, char c) {
  if (pos < 4) {
    displayL.writeDigitAscii(pos, c);
  } else {
    displayR.writeDigitAscii(pos-4, c);
  }
}

void Panel::playWinnerSound(){
  // Toggle the buzzer at various speeds

  for (byte x = 250 ; x > 70 ; x--)
  {
    for (byte y = 0 ; y < 3 ; y++)
    {
      digitalWrite(BUZZER2, HIGH);
      digitalWrite(BUZZER1, LOW);
      delayMicroseconds(x);

      digitalWrite(BUZZER2, LOW);
      digitalWrite(BUZZER1, HIGH);
      delayMicroseconds(x);

    }
  }
}

void Panel::clearDisplay() {
  displayL.clear();
  displayR.clear();
  displayL.writeDisplay();
  displayR.writeDisplay();
}

void Panel::showMessage(char *msg){
  if (strlen(msg) > 8) {
    // TODO: scrolling message
  }

  for (int i = 0; i < strlen(msg); i++){
    _writeChar(i, msg[i]);
  }
  displayL.writeDisplay();
  displayR.writeDisplay();
} 
