#ifndef MUSIC_H
#define MUSIC_H

#include <Arduino.h>
#include "notes.h"

int starWarsMelodyCantinaBand[] = {
  // Cantina BAnd - Star wars 
  // Score available at https://musescore.com/user/6795541/scores/1606876
  B4,-4, E5,-4, B4,-4, E5,-4, 
  B4,8,  E5,-4, B4,8, REST,8,  AS4,8, B4,8, 
  B4,8,  AS4,8, B4,8, A4,8, REST,8, GS4,8, A4,8, G4,8,
  G4,4,  E4,-2, 
  B4,-4, E5,-4, B4,-4, E5,-4, 
  B4,8,  E5,-4, B4,8, REST,8,  AS4,8, B4,8,

  A4,-4, A4,-4, GS4,8, A4,-4,
  D5,8,  C5,-4, B4,-4, A4,-4,
  B4,-4, E5,-4, B4,-4, E5,-4, 
  B4,8,  E5,-4, B4,8, REST,8,  AS4,8, B4,8,
  D5,4, D5,-4, B4,8, A4,-4,
  G4,-4, E4,-2,
  E4, 2, G4,2,
  B4, 2, D5,2,

  F5, -4, E5,-4, AS4,8, AS4,8, B4,4, G4,4, 
};

#endif
