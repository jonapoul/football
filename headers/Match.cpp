#include <math.h>
#include <iostream>
#include <functional>
#include <vector>
#include "Date.h"
#include "Enums.h"
#include "Team.h"
#include "League.h"
#include "Cup.h"
#include "Player.h"
#include "Competition.h"

#include "Match.h"

Match::Match() {
  cup = league = nullptr;
  date = {0,0,0};
  result = DRAW;
  homeGoals = awayGoals = 0;
  homeOdds = awayOdds = 0.f;
  isNeutral = false;
}

Match::Match(const Date& d, 
             const Team* h, 
             const Team* a,
             const Competition* competition,
             const bool b_isNeutral) {
  date = d;
  home = h;
  away = a;
  homeGoals = awayGoals = 0;
  homeOdds = awayOdds = 0.f;
  isNeutral = b_isNeutral;

  League* l = static_cast<League*>(competition);
  Cup* c = static_cast<Cup*>(competition); 
  if (l) {
    league = l;
    cup = nullptr;
  } else if (c) {
    league = nullptr;
    cup = c;
  } else {
    league = cup = nullptr;
  }
}

Match::Match(const Match& m) {
  date = m.date;
  home = m.home;
  away = m.away;
  result = m.result;
  homeGoals = m.homeGoals;
  awayGoals = m.awayGoals;
  homeOdds = m.homeOdds;
  awayOdds = m.awayOdds;
  isNeutral = m.isNeutral;
}

inline bool Match::contains(const Team* t) const {
  return (home == t || away == t);
}

inline bool Match::contains(const std::string& t) const {
  return (home->name == t || away->name == t);
}

void Match::updateForm() {
  if (result == HOME_WIN) {
    home->form = "W" + home->form;
    away->form = "L" + away->form;
  } else if (result == DRAW) {
    home->form = "D" + home->form;
    away->form = "D" + away->form;
  } else if (result == AWAY_WIN) {
    home->form = "L" + home->form;
    away->form = "W" + away->form;
  }
  if (home->form.length() > 5) home->form = home->form.substr(0, 5);
  if (away->form.length() > 5) away->form = away->form.substr(0, 5);
}

bool Match::takeShot(const float odds,
                     const float oddsDiff,
                     int& shots,
                     int& goals) {
  float random = (rand() % 100) / 100.f;
  if (6*random < odds) {
    shots++;
    random = (rand() % 100) / 100.f;
    // decide if shot goes in
    if (random < pow(odds, 2.8) * pow(2.6, -oddsDiff)) {
      goals++;
      return true;
    }
  }
  return false;
}

void Match::calculateResult(const bool extraTime) {
///PREGAME
  // improve condition by linearly increasing it each day between games
  float recoveryRate = 0.06f;
  home->condition += (recoveryRate + (rand() % 10) / 1000.f) * Date::difference(home->lastGame, date);
  away->condition += (recoveryRate + (rand() % 10) / 1000.f) * Date::difference(away->lastGame, date);
  if (home->condition > 1.f || home->played == 0) home->condition = 1.f;
  if (away->condition > 1.f || away->played == 0) away->condition = 1.f;
  // calculate the probability of either team winning
  this->calculateOdds();
  float oddsDiff = homeOdds - awayOdds;
  homeShots = 0, awayShots = 0;
  goals = {};

///MATCH SIMULATION
  for (int minute = 0; minute < 91; minute++) {
    // home team takes a shot
    if ( takeShot(homeOdds, oddsDiff, homeShots, homeGoals) )
      goals.push_back({minute, 'H', nullptr});
    // away take a shot
    if ( takeShot(awayOdds, oddsDiff, awayShots, awayGoals) )
      goals.push_back({minute, 'A', nullptr});
  }
  if (extraTime && homeGoals == awayGoals) {
    for (int minute = 91; minute < 121; minute++) {
      // home team takes a shot
      if ( takeShot(homeOdds, oddsDiff, homeShots, homeGoals) )
        goals.push_back({minute, 'H', nullptr});
      // away take a shot
      if ( takeShot(awayOdds, oddsDiff, awayShots, awayGoals) )
        goals.push_back({minute, 'A', nullptr});
    }
  }

///POSTGAME
  // update team goal/conceded count
  home->scored += homeGoals;
  home->conceded += awayGoals;
  home->difference = home->scored - home->conceded;
  away->scored += awayGoals;
  away->conceded += homeGoals;
  away->difference = away->scored - away->conceded;
  // apply end result to the Match object
  if      (homeGoals > awayGoals) result = HOME_WIN;
  else if (homeGoals < awayGoals) result = AWAY_WIN;
  else                            result = DRAW;
  // update the two teams' conditions
  home->condition -= 0.6f;
  away->condition -= 0.6f;
  // the winning team loses a bit more
  if (result == HOME_WIN) home->condition -= 0.02f;
  if (result == AWAY_WIN) away->condition -= 0.02f;
  // if either condition is less than 0, bump it back to 0
  if (home->condition < 0.f) home->condition = 0.f;
  if (away->condition < 0.f) away->condition = 0.f;

  // update last game dates
  home->lastGame = away->lastGame = date;
  this->updateForm();
}

void Match::calculateOdds() {
  homeOdds = awayOdds = 0.f;
  // recent form
  for (int i = 0; i < 5; i++) {
    if      (home->form[i] == 'W') homeOdds += 0.2f * (1.f - pow(i,1.5)/10.f);
    else if (home->form[i] == 'D') homeOdds += 0.1f * (1.f - pow(i,1.5)/10.f);
    if      (away->form[i] == 'W') awayOdds += 0.2f * (1.f - pow(i,1.5)/10.f);
    else if (away->form[i] == 'D') awayOdds += 0.1f * (1.f - pow(i,1.5)/10.f);
  }
  // overconfidence from good form
  if (homeOdds > 0.5) homeOdds -= 0.1;
  if (awayOdds > 0.5) awayOdds -= 0.1;
  // rebound from bad form
  if      (homeOdds < 0.2) homeOdds += 0.2;
  else if (homeOdds < 0.1) homeOdds += 0.8;
  if      (awayOdds < 0.2) awayOdds += 0.2;
  else if (awayOdds < 0.1) awayOdds += 0.8;

  // team condition
  homeOdds += home->condition * 2.f;
  awayOdds += away->condition * 2.f;
  // home advantage
  if (!isNeutral) homeOdds += 0.5f;
  // weighted average past league positions
  homeOdds += 1.2 * pow( 1/(exp((home->weighted - 4.7)/(3.3)) + 1), 0.2);
  awayOdds += 1.2 * pow( 1/(exp((away->weighted - 4.7)/(3.3)) + 1), 0.2);
  // traditional big team advantage
  if (home->top) homeOdds += 0.3;
  if (away->top) awayOdds += 0.3;
  // advantage for recently promoted teams
  if (home->weighted > 22) homeOdds += (home->weighted - 22) / 30.0;
  if (away->weighted > 22) awayOdds += (away->weighted - 22) / 30.0;
  // lucky season
  if (home->lucky) homeOdds += 0.5;
  if (away->lucky) awayOdds += 0.5;
  if (home->unlucky) homeOdds -= 0.5;
  if (away->unlucky) awayOdds -= 0.5;
  // boost from CL money
  if (home->division == PL && home->last5Seasons[0].position <= 4) homeOdds += 0.05;
  if (away->division == PL && away->last5Seasons[0].position <= 4) awayOdds += 0.05;
  // previous season un/lucky
  if (home->last5Seasons[0].wasLucky)   homeOdds += 0.2;
  if (home->last5Seasons[0].wasUnlucky) homeOdds -= 0.2;
  if (away->last5Seasons[0].wasLucky)   awayOdds += 0.2;
  if (away->last5Seasons[0].wasUnlucky) awayOdds -= 0.2;
  // boost from dat sweet, sweet oil money
  if (home->rich) homeOdds += 0.25;
  if (away->rich) awayOdds += 0.25;

  // normalise the odds
  float sum = homeOdds + awayOdds;
  homeOdds /= sum;
  awayOdds /= sum;
}

void Match::swap(Team* oldTeam,
                 Team* newTeam) {
  if      (home == oldTeam) home = newTeam;
  else if (away == oldTeam) away = newTeam;
}