#ifndef LEAGUE_H
#define LEAGUE_H

#include <iostream>
#include <vector>
#include <fstream>
#include <sstream>
#include <algorithm>
#include "Team.h"
#include "Match.h"
#include "Colour.h"
#include "Date.h"
#include "Enums.h"
#include "Player.h"
using std::cout;

class League {
public:
  std::vector<Team*> teams;
  Division division;
  std::string season;
  int year;
  std::vector<Date> dates;
  std::vector<Match> matches;
  std::vector<unsigned> promotionPositions;
  std::vector<unsigned> playoffPositions;
  std::vector<Team*> playoffTeams;
  std::vector<unsigned> relegationPositions;
  Date spareDate1;
  Date spareDate2;
  bool fixturesHaveBeenGenerated;

  League(const Division& div) {
    division = div;
    season = "2016/17";
    year = 2017;
    fixturesHaveBeenGenerated = false;
    if (div == PL) {
      promotionPositions = {};
      playoffPositions = {};
      relegationPositions = {17,18,19};
    } else if (div == CH) {
      promotionPositions = {0,1};
      playoffPositions = {2,3,4,5};
      relegationPositions = {21,22,23};
    } else if (div == L1) {
      promotionPositions = {0,1};
      playoffPositions = {2,3,4,5};
      relegationPositions = {20,21,22,23};
    } else if (div == L2) {
      promotionPositions = {0,1,2};
      playoffPositions = {3,4,5,6};
      relegationPositions = {22,23};
    } else if (div == CN) {
      promotionPositions = {0};
      playoffPositions = {1,2,3,4};
      relegationPositions = {20,21,22,23};
    }
  }

  void sortTable() {
    int n = int(teams.size());
    for (int i = 0; i < n-1; i++) {
      for (int j = 0; j < n-i-1; j++){
        if (teams[j]->isHigherThan(teams[j+1])) {
          Team* temp = teams[j];
          teams[j] = teams[j+1];
          teams[j+1] = temp;
        }
      }
    }
    for (int i = 0; i < teams.size(); i++) {
      teams[i]->position = i+1;
      for (const auto p : promotionPositions)
        if (p == i) teams[i]->isPromoted = true;
      for (const auto pl : playoffPositions) {
        if (pl == i) {
          teams[i]->inPlayoffs = true;
          playoffTeams.push_back(teams[i]);
        }
      }
      for (const auto r : relegationPositions)
        if (r == i) teams[i]->isRelegated = true;
    }
  }

  void processMatches(const bool printFixtures = true) {
    for (auto& m : matches) {
      m.calculateResult();
      updateTable(m);
      if (printFixtures)
        printScore(m);
    }
  }

  void printScore(const Match& m) {
    int size = 0;
    for (auto t : teams) 
      if (t->name.length() > size) size = t->name.length();
    auto paddedA = [size](std::string s) { return s + std::string(size-s.length(), ' '); };
    auto paddedH = [size](std::string s) { return std::string(size-s.length(), ' ') + s; };
    Colour red(FG_LIGHT_RED);
    Colour grn(FG_LIGHT_GREEN);
    Colour ylw(FG_LIGHT_YELLOW);
    Colour def(FG_DEFAULT);
    auto printForm = [red,grn,ylw,def](const std::string& form) {
      for (auto c : form) {
        if (c == 'W') cout << grn << c;
        if (c == 'D') cout << ylw << c;
        if (c == 'L') cout << red << c;
        if (c == ' ') cout << c;
      }
      cout << def;
    };
    cout << m.date.to_string() << ' ';
    printForm(m.home->form);
    cout << ' ';
    if      (m.result == HOME_WIN) cout << grn;
    else if (m.result == DRAW)     cout << ylw;
    else                           cout << red;
    cout << paddedH(m.home->name) << ' '; 
    if (m.homeGoals < 10) cout << ' ';
    cout << m.homeGoals << def << " - ";
    if      (m.result == AWAY_WIN) cout << grn;
    else if (m.result == DRAW)     cout << ylw;
    else                           cout << red;
    cout << m.awayGoals;
    if (m.awayGoals < 10) cout << ' ';
    cout << ' ' << paddedA(m.away->name) << ' ' << def;
    printForm(m.away->form);
    cout << "      " << divisionNames[division];
    printf("\n                           Odds:   %.3f - %-.3f", m.homeOdds, m.awayOdds);
    printf("\n                          Shots:      %2d - %-2d", m.homeShots, m.awayShots);

    // printing all goals in order
    if (m.goals.size() > 0) {
      printf("\n                          Goals:");
      cout << std::string(6, ' ');
      if (m.goals[0].team == 'A') printf("     %-2d'\n", m.goals[0].minute);
      else                        printf("%2d'\n", m.goals[0].minute);
    }
    else {
      printf("\n\n");
      return;
    }
    for (int i = 1; i < m.goals.size(); i++) {
      Goal g = m.goals[i];
      cout << std::string(38, ' ');
      if (g.team == 'A') printf("     %-2d'\n", g.minute);
      else               printf("%2d'\n", g.minute);
    }
  }

  void updateTable(const Match& m) {
    if (m.result == HOME_WIN) {
      m.home->won++;
      m.home->points += 3;
      m.away->lost++;
    }
    if (m.result == DRAW) {
      m.home->drawn++;
      m.home->points++;
      m.away->drawn++;
      m.away->points++;
    }
    if (m.result == AWAY_WIN) {
      m.away->won++;
      m.away->points += 3;
      m.home->lost++;
    }
    m.home->played++;
    m.away->played++;
  }

  void printTable(std::ofstream& titleFile) {
    int size = 0;
    for (auto t : teams) 
      if (t->name.length() > size) size = t->name.length();
    auto strPad = [size](const std::string& s) { 
      int l = size - s.length() + 2; 
      return std::string(l, ' ') + s; 
    };
    auto intPad = [](const int i) {
      int length, size = 3;
      if (i < 0) {
        if      (i > -10)   length = 2;
        else if (i > -100)  length = 3;
        else                length = 4;
      } else {
        if      (i < 10)    length = 1;
        else if (i < 100)   length = 2;
        else                length = 3;
      }
      std::stringstream ss;
      ss << std::string(size-length+1, ' ') << i;
      return ss.str();
    };
    auto titlePad = [](const std::string& s) {
      std::stringstream ss;
      ss << "  " << s << ' ';
      return ss.str();
    };

    cout << "        " << divisionNames[division];
    cout << "\n" << season << strPad("TEAMS") << titlePad("G") << titlePad("W") << titlePad("D") << titlePad("L") << titlePad("S") << titlePad("C") << titlePad("GD") << titlePad("P") << titlePad("FORM") << "   PREV 5 SEASONS  " << titlePad("WGHT") << titlePad("TOP") << titlePad("LUC") << titlePad("UNL") << titlePad("UNB") << titlePad("RICH") << titlePad("TITLES") << '\n';
    cout << std::string(size+113, '=') << '\n';
    int i = 1;
    Colour red(FG_LIGHT_RED);
    Colour grn(FG_LIGHT_GREEN);
    Colour ylw(FG_LIGHT_YELLOW);
    Colour cyn(FG_LIGHT_CYAN);
    Colour mag(FG_LIGHT_MAGENTA);
    Colour def(FG_DEFAULT);
    Colour bgGrn(BG_GREEN);
    Colour bgDef(BG_DEFAULT);
    for (auto t : teams) {
      if (i == 1 && division == PL) {
        t->titles++;
        titleFile << season << ' ';
        if (t->titles < 10) 
          titleFile << "00";
        else if (t->titles < 100) 
          titleFile << "0";
        titleFile << t->titles << ' ' << t->name << '\n';
      }
      bool isMyTeam = (teams[i-1]->name == "Liverpool" || 
                       teams[i-1]->name == "Tamworth"  ||
                       teams[i-1]->name == "Burton Albion");
      if (isMyTeam)                     cout << bgGrn;
      if (i == 1)                       cout << grn << "   C";
      else if (teams[i-1]->isPromoted)  cout << cyn << "   P";
      else if (teams[i-1]->inPlayoffs)  cout << mag << intPad(i);
      else if (teams[i-1]->isRelegated) cout << red << "   R";
      else                              cout << def << intPad(i);
      if (isMyTeam)                     cout << bgDef;
      cout << "   " << strPad(t->name) << intPad(t->played) << intPad(t->won) << intPad(t->drawn) << intPad(t->lost) << intPad(t->scored) << intPad(t->conceded) << intPad(t->difference) << intPad(t->points) << "  ";
      for (auto c : t->form) {
        if (c == 'W') cout << grn << c;
        if (c == 'D') cout << ylw << c;
        if (c == 'L') cout << red << c;
        if (c == ' ') cout << c;
      }
      cout << def << "   ";
      for (auto s : t->last5Seasons) {
        if (s.wasLucky)   cout << grn;
        if (s.wasUnlucky) cout << red;
        printf("%3d", s.position);
        if (s.wasLucky || s.wasUnlucky) cout << def;
      }
      printf("  %6.2f  ", t->weighted);
      printf("%s ", (t->top)  ? " yes "  : "     ");
      printf("%s ", (t->lucky)   ? " yes "  : "     ");
      printf("%s ", (t->unlucky) ? " yes " : "     ");
      printf("%s ", (t->lost == 0) ? " yes"  : "     ");
      printf(" %s", (t->rich) ? "yes" : "   ");
      printf("      %s ", (t->titles > 0) ? std::to_string(t->titles).c_str() : " ");
      cout << '\n';
      bool putDashedLine = (division == PL && (i == 4 || i == 7)) || 
                           (division != PL && (i-1 == promotionPositions[promotionPositions.size()-1])) ||
                           (division != PL && (i-1 == playoffPositions[playoffPositions.size()-1])) ||
                           (i == relegationPositions[0]);
      if (putDashedLine) cout << def << std::string(size+113, '-') << '\n';
      i++;
      t->printToFile(division, season);
    }
    cout << def;
  }

  bool isPromoted(const int pos) {
    bool result = false;
    if (division == CH || division == L1) {
      if (pos < 3) {
        result = teams[pos-1]->isPromoted = true;
      }
    }
    else if (division == L2) {
      if (pos < 4) {
        result = teams[pos-1]->isPromoted = true;
      }
    }
    return result;
  }

  bool isRelegated(const int pos) {
    size_t size = teams.size();
    int offset;
    switch(division) {
      case PL: 
      case CH: offset = 3; break;
      case L1: offset = 4; break;
      case L2: offset = 2; break;
      case CN: offset = 4; break;
      default: offset = 3; break;
    }
    if (pos > size-offset) {
      teams[pos-1]->isRelegated = true;
      return true;
    }
    return false;
  }

  void playoffs() {
    // only dealing with lower leagues
    if (division == PL) return;

    cout << Colour(FG_LIGHT_CYAN) << "\n|||||||||||||||||||||||||||||||||||    PLAYOFF SEMIS    |||||||||||||||||||||||||||||||||||\n" << Colour(FG_DEFAULT);

    Team* t1 = playoffTeams[0];
    Team* t2 = playoffTeams[1];
    Team* t3 = playoffTeams[2];
    Team* t4 = playoffTeams[3];

    Match semi1_leg1( Date(year, 5, 13), t4, t1, false );
    semi1_leg1.calculateResult();
    printScore(semi1_leg1);
    Match semi1_leg2( Date(year, 5, 16), t1, t4, false );
    semi1_leg2.calculateResult();
    printScore(semi1_leg2);
    int t4Score = semi1_leg1.homeGoals + semi1_leg2.awayGoals;
    int t1Score = semi1_leg1.awayGoals + semi1_leg2.homeGoals;
    Team* semi1;
    if (t4Score == t1Score) {
      t4Score += semi1_leg2.awayGoals;
      t1Score += semi1_leg1.awayGoals;
      if (t1Score > t4Score) {
        semi1 = t1;
        cout << t1->name << " wins on away goals\n";
      } else if (t1Score < t4Score) {
        semi1 = t4;
        cout << t4->name << " wins on away goals\n";
      } else {
        int homePens, awayPens;
        penaltyShootout(homePens, awayPens);
        if (homePens > awayPens) {
          t1->condition -= 0.1;
          t4->condition -= 0.08;
          semi1 = t1;
          cout << t1->name << " wins " << homePens << '-' << awayPens << " on penalties\n";
        } else {
          t1->condition -= 0.08;
          t4->condition -= 0.1;
          semi1 = t4;
          cout << t4->name << " wins " << awayPens << '-' << homePens << " on penalties\n";
        }
      }
    } else {
      semi1 = (t1Score > t4Score) ? t1 : t4;
      cout << ((t1Score > t4Score) ? t1 : t4)->name << " wins " << ((t1Score > t4Score) ? t1Score : t4Score) << '-' << ((t1Score > t4Score) ? t4Score : t1Score) << " on aggregate\n";
    }

    Match semi2_leg1( Date(year, 5, 14), t3, t2, false );
    semi2_leg1.calculateResult();
    printScore(semi2_leg1);
    Match semi2_leg2( Date(year, 5, 17), t2, t3, false );
    semi2_leg2.calculateResult();
    printScore(semi2_leg2);
    int t3Score = semi2_leg1.homeGoals + semi2_leg2.awayGoals;
    int t2Score = semi2_leg1.awayGoals + semi2_leg2.homeGoals;
    Team* semi2;
    if (t2Score == t3Score) {
      t3Score += semi2_leg2.awayGoals;
      t2Score += semi2_leg1.awayGoals;
      if (t2Score > t3Score) {
        semi2 = t2;
        cout << t2->name << " wins on away goals\n";
      } else if (t2Score < t3Score) {
        semi2 = t3;
        cout << t3->name << " wins on away goals\n";
      } else {
        int homePens, awayPens;
        penaltyShootout(homePens, awayPens);
        if (homePens > awayPens) {
          t2->condition -= 0.1;
          t3->condition -= 0.08;
          semi2 = t2;
          cout << t2->name << " wins " << homePens << '-' << awayPens << " on penalties\n";
        } else {
          t3->condition -= 0.1;
          t2->condition -= 0.08;
          semi2 = t3;
          cout << t3->name << " wins " << awayPens << '-' << homePens << " on penalties\n";
        }
      }
    } else {
      semi2 = (t2Score > t3Score) ? t2 : t3;
      cout << ((t2Score > t3Score) ? t2 : t3)->name << " wins " << ((t2Score > t3Score) ? t2Score : t3Score) << '-' << ((t2Score > t3Score) ? t3Score : t2Score) << " on aggregate\n";
    }

    cout << Colour(FG_LIGHT_CYAN) << "\n|||||||||||||||||||||||||||||||||||    PLAYOFF FINAL    |||||||||||||||||||||||||||||||||||\n" << Colour(FG_DEFAULT);
    Match final( Date(year, 5, 28), semi1, semi2, true );
    final.calculateResult(true);
    printScore(final);
    Team* promoted;
    if (final.homeGoals == final.awayGoals) {
      int homePens, awayPens;
      penaltyShootout(homePens, awayPens);
      if (homePens > awayPens) {
        promoted = semi1;
        cout << semi1->name << " wins " << homePens << '-' << awayPens << " on penalties\n";
      } else {
        promoted = semi2;
        cout << semi2->name << " wins " << awayPens << '-' << homePens << " on penalties\n";
      }
    } else if (final.homeGoals > final.awayGoals) {
      promoted = semi1;
    } else {
      promoted = semi2;
    }
    promoted->isPromoted = true;
    for (auto& t : playoffTeams) t->inPlayoffs = false;
    playoffTeams = {};
  }

  void penaltyShootout(int& homePens,
                       int& awayPens) {
    homePens = awayPens = 0;
    for (int i = 1; ; i++) {
      bool homeScored = (rand() % 100)/100.f < 0.73;
      if (homeScored) homePens++;
      bool awayScored = (rand() % 100)/100.f < 0.73;
      if (awayScored) awayPens++;
      if (i >= 5 && (homePens != awayPens)) 
        return;
    }
  }

  void generateFixtures() {

    // randomly shuffle team order
    std::random_shuffle(teams.begin(), teams.end());
    // decide if any teams will have a good/bad season
    for (auto& t : teams) {
      float random = (rand() % 100) / 100.f;
      t->lucky   = random > 0.94;
      t->unlucky = random < 0.06;
    }
    
    // if we don't need to go through and regenerate everything
    if (fixturesHaveBeenGenerated) {
      int vectorsize = promotionPositions.size() + 
                       playoffPositions.size()/4 + 
                       relegationPositions.size();
      std::vector<Team*> oldTeams(0), newTeams(0);
      auto wasAlreadyFound = [&oldTeams](Team* t1) {
        for (auto& t : oldTeams) if (t == t1) return true;
        return false;
      };
      for (auto& m : matches) {
        if (m.home->division != division && !wasAlreadyFound(m.home)) 
          oldTeams.push_back(m.home);
        else if (m.away->division != division && !wasAlreadyFound(m.away)) 
          oldTeams.push_back(m.away);
        if (oldTeams.size() == vectorsize)                            
          break;
      }
      for (auto& t : teams) {
        if (t->last5Seasons[0].position < 1 || t->last5Seasons[0].position > teams.size())
          newTeams.push_back(t);
      }
      for (auto& m : matches) {
        for (int i = 0; i < oldTeams.size(); i++) 
          if (m.contains(oldTeams[i]))
            m.swap(oldTeams[i], newTeams[i]);
      }

      int half = teams.size()/2 - 1;
      struct SwapBuddies { Team* t1; Team* t2; };
      std::vector<SwapBuddies> mates;
      for (int i = 0; i < teams.size(); i += 2) mates.push_back({teams[i], teams[i+1]});
      for (auto& m : matches) {
        for (auto& mate : mates) {
          if      (m.home == mate.t1) m.home = mate.t2;
          else if (m.home == mate.t2) m.home = mate.t1;
          if      (m.away == mate.t1) m.away = mate.t2;
          else if (m.away == mate.t2) m.away = mate.t1;
        }
      }
      // resetting all other stats about the match
      for (auto& m : matches) {
        m.date.year++;
        m.result = DRAW;
        m.homeGoals = m.awayGoals = 0;
        m.homeShots = m.awayShots = 0;
        m.goals = {};
        m.homeOdds = m.awayOdds = 0;
      }
      // back out because we've repurposed the old fixture dates
      return;
    }
    // reset all fixtures from last season
    matches = std::vector<Match>(0);
    int half = teams.size()-1;

    std::vector<std::vector<Team*>> valid(teams.size());
    for (int d = 0; d <= dates.size()/2; d++) {
      for (int i = 0; i < teams.size()/2; i++) {
        updateValid(valid, dates[d], teams);
        if (valid[0].size() > 1) {
          // add the home/away fixtures to the schedule
          if (rand() % 2 == 1) {
            matches.push_back({dates[d],      valid[0][0], valid[0][1]});
            matches.push_back({dates[d+half], valid[0][1], valid[0][0]});
          } else {
            matches.push_back({dates[d],      valid[0][1], valid[0][0]});
            matches.push_back({dates[d+half], valid[0][0], valid[0][1]});
          }
        }
      }
    }
    // counting how many fixtures each team has
    std::vector<unsigned> fixtureCounts(teams.size(), 0); // 20 elements
    for (const auto& m : matches) {
      for (int t = 0; t < teams.size(); t++)
        if (m.contains(teams[t])) 
          fixtureCounts[t]++;
    }
    // making an array of the teams without a full fixture set
    std::vector<Team*> missing;
    for (int i = 0; i < teams.size(); i++) {
      if (fixtureCounts[i] < 2*teams.size()-2)
        missing.push_back(teams[i]);
    }
  
    valid = std::vector<std::vector<Team*>>(missing.size());
    for (const auto& v : valid) {
      updateValid(valid, spareDate1, missing);
      if (valid[0].size() > 1) {
        if (rand() % 2 == 1) {
          matches.push_back({spareDate1, valid[0][0], valid[0][1]});
          matches.push_back({spareDate2, valid[0][1], valid[0][0]});
        } else {
          matches.push_back({spareDate1, valid[0][1], valid[0][0]});
          matches.push_back({spareDate2, valid[0][0], valid[0][1]});
        }
      }
    }

    auto matchSort = [](const Match& m1, const Match& m2) { return (m1.date < m2.date); };
    std::sort(matches.begin(), matches.end(), matchSort);
    fixturesHaveBeenGenerated = true;
  }

  void updateValid(std::vector<std::vector<Team*>>& valid,
                   const Date& d,
                   const std::vector<Team*>& teamVec) {
    valid = std::vector<std::vector<Team*>>(teamVec.size());
    for (int v = 0; v < valid.size(); v++) {
      valid[v].push_back(teamVec[v]);
      for (int t = 0; t < teamVec.size(); t++) {
        if (isValid(v, t, d, teamVec))
          valid[v].push_back(teamVec[t]);
      }
    }
    std::sort(valid.begin(), valid.end(), [](const std::vector<Team*>& v1, const std::vector<Team*>& v2) {
      if (v1.size() == 1 || v2.size() == 1) return (v1.size() > v2.size());
      else                                  return (v1.size() < v2.size());
    }); 
  }

  bool isValid(const int v, 
               const int t,
               const Date& d,
               const std::vector<Team*>& teamVec) {
    Team* t1 = teamVec[v];
    Team* t2 = teamVec[t];
    if (t1 == t2)
      return false;
    for (const auto& m : matches) {
      // if both teams are already booked to play a game at some point
      if (m.contains(t1) && m.contains(t2))
        return false; 
      // if either team already has a game on that date
      if (d == m.date && (m.contains(t1) || m.contains(t2)))
        return false;
    }
    return true;
  }

  bool contains() {
    std::string targets[] = {"Liverpool", "Burton Albion", "Tamworth"};
    for (auto& t : teams) {
      for (int i = 0; i < 3; i++) {
        if (targets[i] == t->name)
          return true;
      }
    }
    return false;
  }
};

#endif