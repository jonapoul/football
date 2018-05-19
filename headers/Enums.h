#ifndef ENUMS_H
#define ENUMS_H

#include <string>

enum CountryCode { ENG, SPA, GER, ITA, FRA, POR, SCO, RUS, TUR };

enum Result { HOME_WIN, DRAW, AWAY_WIN };

enum Position { GK, RB, CB, LB, DM, CM, AM, RW, LW, ST };

enum Division { PL, CH, L1, L2, CN };
std::string divisionCodes[] = {"PL", "CH", "L1", "L2", "CN"};
std::string divisionNames[] = {"PREMIER LEAGUE", "CHAMPIONSHIP", "LEAGUE ONE", "LEAGUE TWO", "CONFERENCE"};

struct Season {
  int position;
  bool wasLucky;
  bool wasUnlucky;
};

class Player;
struct Goal { 
  int minute; 
  char team;
  Player* goalscorer; 
};

#endif 