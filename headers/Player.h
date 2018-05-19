#ifndef PLAYER_H
#define PLAYER_H

#include <string>
#include "Enums.h"

class Player {
public:
  std::string forename;
  std::string surname;
  unsigned number;
  Position position;
  unsigned goals;
  unsigned assists;
  unsigned cleanSheets;
};

#endif