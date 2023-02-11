#include <Arduino.h>
#include "notes.h"
#include "arcade_panel.h"

#include <stdlib.h>

// Define game parameters
#define ROUNDS_TO_WIN      13 //Number of rounds to succesfully remember before you win. 13 is do-able.
#define DEFAULT_TIME_LIMIT   3000 //Amount of time to press a button before game times out. 3000ms = 3 sec
#define MAX_SCORE 64  // Setting this to a large number to avoid dealing with Vectors


#define MODE_MEMORY  0
#define MODE_BATTLE  1


// Game state variables
byte gameMode = MODE_MEMORY; //By default, let's play the memory game
byte gameBoard[MAX_SCORE]; //Contains the combination of buttons as we advance
byte gameRound = 0; //Counts the number of succesful rounds the player has made it through
int audioHighScore = 0;
int visualHighScore = 0;
int simonHighScore = 0;
int speedSetting = 3;
int timeLimit = DEFAULT_TIME_LIMIT;

Panel panel;

void(* resetFunc) (void) = 0; //declare reset function @ address 0

void setup()
{

  Serial.begin(115200);
  while (!Serial) delay(10);   // wait until serial port is opened
  Serial.println(F("Adafruit PID 5296 I2C QT 4x LED Arcade Buttons"));
  
  //Setup hardware inputs/outputs. These pins are defined in the hardware_versions header file
  //panel = Panel(150);
  panel.setup();

  //Mode checking
  gameMode = MODE_MEMORY; // By default, we're going to play the memory game

  // Hold down the yellow button when turning on power to play this song
  byte initButton = panel.pressed();

  switch (initButton) {
    case YELLOW:
      speedSetting = 5;
      panel.showMessage("SLOWEST");
      break;
    case GREEN:
      speedSetting = 4;
      panel.showMessage("SLOWER");
      break;
    case RED:
      speedSetting = 2;
      panel.showMessage("FASTER");
      break;
    case BLUE:
      speedSetting = 1;
      panel.showMessage("FASTEST");
      break;
  };

  panel.setSpeed(speedSetting);
  timeLimit = (0.33f * speedSetting) * DEFAULT_TIME_LIMIT;

  char message[100];
  char timeLimitMsg[20];
  itoa(timeLimit, timeLimitMsg, 10);
  strcpy(message, "TIME LIMIT = ");
  strcat(message, timeLimitMsg);
  Serial.println(message);

  play_winner(); // After setup is complete, say hello to the world

  panel.buttonLights(NONE);
}



void loop()
{
  attractMode(); // Blink lights while waiting for user to press a button

  // Indicate the start of game play
  panel.clearDisplay();
  int score = 0;
  panel.pulse(RED | GREEN | BLUE | YELLOW, 750); // Turn all LEDs on
  delay(250);

  bool audio = panel.soundsOn() && !(panel.lightsOn());
  bool visual = panel.lightsOn() && !(panel.soundsOn());
  bool simon = panel.lightsOn() && panel.soundsOn();
  bool song = !(panel.lightsOn() || panel.soundsOn());


  if (song){
    panel.showMessage("MUSIC");
    play_song();

  } else if (gameMode == MODE_BATTLE && simon) {
    play_battle(); // Play game until someone loses
    panel.playGameOver(); // Player lost, play loser tones

  } else if (simon) {
    panel.showMessage("Simon");
    int movesToWinSimon = 14;
    score = play_memory();

    // Display results
    char scoreMsg[2];
    char message[8];
    itoa(score, scoreMsg, 10);
    strcpy(message, "Score ");
    strcat(message, scoreMsg);
    panel.showMessage(message);

    if (score >= movesToWinSimon) {
      play_winner();
    } else {
      panel.playGameOver();
    }

  } else {
    score = 0;
    char scoreA[3];
    char scoreV[3];
    char message[9];
    itoa(audioHighScore, scoreA, 10);
    itoa(visualHighScore, scoreV, 10);

    strcpy(message, "V=");
    strcat(message, scoreV);
    if (strlen(scoreV) < 2) strcat(message, " ");
    strcat(message, "A=");
    strcat(message, scoreA);

    panel.showMessage(message);

    if (audio) {
      panel.buttonLights(YELLOW);
      panel.buttonTone(YELLOW);
      delay(300);
      panel.buttonLights(GREEN);
      panel.buttonTone(GREEN);
      delay(300);
      panel.buttonLights(RED);
      panel.buttonTone(RED);
      delay(300);
      panel.buttonLights(BLUE);
      panel.buttonTone(BLUE);
      delay(300);
      panel.buttonLights(NONE);
    }

    score = play_memory();

    if (audio && score > audioHighScore) {
      audioHighScore = score;
      play_winner();
    } else if (visual && score > visualHighScore) {
      visualHighScore = score;
      play_winner();
    } else {
      panel.playGameOver();
    }
  }

}



//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//The following functions are related to game play only
// Play the regular memory game
// Returns 0 if player loses, or 1 if player wins
int play_memory(void)
{
  panel.buttonLights(NONE);
  delay(1000);

  randomSeed(millis()); // Seed the random generator with random amount of millis()
  gameRound = 0; // Reset the game to the beginning

  while (1) 
  {

    // Add a random button to the sequence
    byte newButton = random(0, 4); //min (included), max (exluded)

    // We have to convert this number, 0 to 3, to CHOICEs
    if(newButton == 0) newButton = YELLOW;
    else if(newButton == 1) newButton = GREEN;
    else if(newButton == 2) newButton = RED;
    else if(newButton == 3) newButton = BLUE;

    gameBoard[gameRound] = newButton; // Add this new button to the game array
    playMoves(); // Play back the current game board

    // Then require the player to repeat the sequence.
    for (int currentMove = 0 ; currentMove < gameRound ; currentMove++)
    {

      byte choice = wait_for_button(); // See what button the user presses

      // If wait timed out, player loses
      if (choice == 0) return gameRound-1; 

      // If the choice is incorect, player loses
      if (choice != gameBoard[currentMove]) return gameRound-1; 

    }
    gameRound++;
    delay(1000); // Player was correct, delay before playing moves

  }



  return gameRound-1; // Player made it through all the rounds to win!

}



// Play the special 2 player battle mode

// A player begins by pressing a button then handing it to the other player

// That player repeats the button and adds one, then passes back.

// This function returns when someone loses

boolean play_battle(void) {

  gameRound = 0; // Reset the game frame back to one frame

  while (1) // Loop until someone fails 
  {

    byte newButton = wait_for_button(); // Wait for user to input next move
    gameBoard[gameRound++] = newButton; // Add this new button to the game array

    // Then require the player to repeat the sequence.
    for (byte currentMove = 0 ; currentMove < gameRound ; currentMove++)
    {

      byte choice = wait_for_button();

      if (choice == 0) return false; // If wait timed out, player loses.
      if (choice != gameBoard[currentMove]) return false; // If the choice is incorect, player loses.

    }
    delay(100); // Give the user an extra 100ms to hand the game to the other player
  }
  return true; // We should never get here
}


// Plays the current contents of the game moves
void playMoves(void)
{

  for (byte currentMove = 0 ; currentMove < gameRound ; currentMove++) 
  {

    panel.alert(gameBoard[currentMove]);

    // Wait some amount of time between button playback
    // Shorten this to make game harder
    delay(panel.duration()); // 150 works well. 75 gets fast.

  }

}

// Wait for a button to be pressed. 
// Returns one of LED colors (LED_RED, etc.) if successful, 0 if timed out
byte wait_for_button(void)
{

  long startTime = millis(); // Remember the time we started the this loop

  while ( (millis() - startTime) < timeLimit) // Loop until too much time has passed
  {

    byte button = panel.pressed();

    if (button != NONE) { 
      panel.alert(button);
      while(panel.pressed() != NONE) ;  // Now let's wait for user to release button
      delay(10); // This helps with debouncing and accidental double taps

      return button;

    }

  }
  return NONE; // If we get here, we've timed out!

}


// Play the winner sound and lights
void play_winner(void) {
  panel.buttonLights(GREEN | BLUE);
  panel.playWinnerSound();

  panel.buttonLights(RED | YELLOW);
  panel.playWinnerSound();

  panel.buttonLights(GREEN | BLUE);
  panel.playWinnerSound();

  panel.buttonLights(RED | YELLOW);
  panel.playWinnerSound();
}


// Show an "attract mode" display while waiting for user to press button.

void attractMode(void) {
  panel.clearDisplay();
  while(1) {
    panel.pulse(RED, 100);
    if (panel.pressed() != NONE) return;

    panel.pulse(BLUE, 100);
    if (panel.pressed() != NONE) return;

    panel.pulse(GREEN, 100);
    if (panel.pressed() != NONE) return;

    panel.pulse(YELLOW, 100);
    if (panel.pressed() != NONE) return;
  }

}



//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// The following functions are related to Beegees Easter Egg only
// Notes in the melody. Each note is about an 1/8th note, "0"s are rests.

int melody[] = {
  G4, A4, 0, C5, 0, 0, G4, 0, 0, 0,
  E4, 0, D4, E4, G4, 0,
  D4, E4, 0, G4, 0, 0,
  D4, 0, E4, 0, G4, 0, A4, 0, C5, 0};

int noteDuration = 115; // This essentially sets the tempo, 115 is just about right for a disco groove :)
int LEDnumber = 0; // Keeps track of which LED we are on during the beegees loop

// Do nothing but play bad beegees music
// This function is activated when user holds bottom right button during power up
void play_beegees() {

  //Turn on the bottom right (yellow) LED
  panel.alert(YELLOW);
  panel.buttonLights(RED | GREEN | BLUE); // Turn on the other LEDs until you release button

  while(panel.pressed() != NONE) ; // Wait for user to stop pressing button

  panel.buttonLights(NONE); // Turn off LEDs
  delay(1000); // Wait a second before playing song

  digitalWrite(BUZZER1, LOW); // setup the "BUZZER1" side of the buzzer to stay low, while we play the tone on the other pin.

  byte thisButton = YELLOW;
  while(panel.pressed() == NONE) //Play song until you press a button
  {

    // iterate over the notes of the melody:
    for (int thisNote = 0; thisNote < 32; thisNote++) {
      thisButton = panel.nextButton(thisButton);
      panel.buttonLights(thisButton);

      tone(BUZZER2, melody[thisNote],noteDuration);

      // to distinguish the notes, set a minimum time between them.
      // the note's duration + 30% seems to work well:
      int pauseBetweenNotes = noteDuration * 1.30;

      delay(pauseBetweenNotes);

      // stop the tone playing:
      noTone(BUZZER2);

    }
  }
}

void play_song() {
  // TODO - play a song!!
  play_beegees();
  resetFunc();
}
