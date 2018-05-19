#ifndef TEAM_H
#define TEAM_H

#include <numeric>
#include <string>
#include <vector>
#include <cmath>
#include <fstream>
#include <iostream>
#include <sstream>
#include <boost/format.hpp>
#include "Date.h"
#include "Enums.h"
#include "Colour.h"
using std::cout;

class Match;

class Team {
public:
  void* league;
	std::string name;
  int position;
	int played;
	int won;
	int drawn;
	int lost;
	int points;
	int scored;
	int conceded;
	int difference;
	std::string form;
	Division division;
	float condition;
	std::vector<Season> last5Seasons;
	float weighted;
	Date lastGame;
	bool isPromoted;
	bool isRelegated;
  bool inPlayoffs;
  bool top;
  bool lucky;
  bool unlucky;
  bool rich;
  unsigned titles;
  std::ofstream file;

	Team() {
    name = form = "     ";
    won = drawn = lost = points = played = 0;
    scored = conceded = difference = position = 0;
    division = PL;
    condition = 1.f;
    last5Seasons = {{},{},{},{},{}};
    weighted = 0.f;
    lastGame = Date();
    isPromoted = inPlayoffs = isRelegated = rich = false;
  }

  Team(const std::string& teamName, 
       const Division teamDivision, 
       const std::vector<Season>& last5) {
    name = teamName;
    form = "     ";
    won = drawn = lost = points = played = 0;
    scored = conceded = difference = position = 0;
    division = teamDivision;
    condition = 1.f;
    last5Seasons = last5;
    weighted = weightedLast5Seasons();
    lastGame = Date();
    isPromoted = inPlayoffs = isRelegated = false;
  }

  Team(const Team& t2) {
    played = t2.played;
    position = t2.position;
    name = t2.name;
    won = t2.won;
    drawn = t2.drawn;
    lost = t2.lost;
    points = t2.points;
    scored = t2.scored;
    conceded = t2.conceded;
    difference = t2.difference;
    form = t2.form;
    division = t2.division;
    condition = t2.condition;
    last5Seasons = t2.last5Seasons;
    weighted = t2.weighted;
    lastGame = t2.lastGame;
    isPromoted = false;
    isRelegated = false;
    inPlayoffs = false;
  }

  void printToFile(const Division& div,
                   const std::string& season) {
    file << boost::format{"%s %2s %2d %2d %2d %2d %3d %3d %4d %3d %c%c%c%c%c %3d\n"} 
            % season % divisionCodes[div] % position % won % drawn % lost 
            % scored % conceded % difference % points % (lost==0?"i":" ") % (top?"t":" ") 
            % (lucky?"l":" ") % (unlucky?"u":" ") % (rich?"r":" ") % titles;
  }

  void reset() {
    form = "     ";
    won = drawn = lost = points = played = 0;
    scored = conceded = difference = 0;
  }

  void updateLast5() {
    last5Seasons = { 
      {position, lucky, unlucky}, 
      last5Seasons[0], 
      last5Seasons[1],
      last5Seasons[2], 
      last5Seasons[3] 
    };
    lucky = unlucky = false;
    weighted = weightedLast5Seasons();
    if (division == PL) {
      if (!top) {
        bool movingIntoTop6 = true;
        for (auto l : last5Seasons) if (l.position > 7) movingIntoTop6 = false;
        if (movingIntoTop6) top = true;
      } else {
        bool leavingTop6 = true;
        for (auto l : last5Seasons) if (l.position <= 7) leavingTop6 = false;
        if (leavingTop6) top = false;
      }
    }
    float random = (rand() % 100)/100.f;
    if      (!rich && random > 0.97) rich = true;
    else if (rich  && random < 0.1)  rich = false;
  }

  bool isHigherThan(const Team* t2) {
    if (this->points     != t2->points)     return (this->points     < t2->points);
    if (this->difference != t2->difference) return (this->difference < t2->difference);
    if (this->scored     != t2->scored)     return (this->scored     < t2->scored);
    if (this->name       != t2->name)       return (this->name       > t2->name);
    return true;
  }

  float weightedLast5Seasons() {
    float output = 0.f;
    float w[5];
    for (int i = 0; i < 5; i++)
      w[i] = exp(-i / 2.f);
    float denominatorSum = std::accumulate(w, w+5, 0.f);
    float numeratorSum = 0.f;
    for (int i = 0; i < 5; i++)
      numeratorSum += this->last5Seasons[i].position * w[i];
    return numeratorSum / denominatorSum;
  }

  void promote() {
    switch(division) {
      case PL: 
        division = PL;
        return;
      case CH: 
        division = PL;
        return;
      case L1: 
        division = CH;
        return;
      case L2: 
        division = L1;
        return;
      case CN:
        division = L2;
        return;
    }
  }
  void relegate() {
    switch(division) {
      case PL: 
        division = CH;
        return;
      case CH: 
        division = L1;
        return;
      case L1: 
        division = L2;
        return;
      case L2: 
        division = CN;
        return;
      case CN:
        division = CN;
        return;
    }
  }
};

#endif