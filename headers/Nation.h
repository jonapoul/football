#ifndef NATION_H
#define NATION_H

#include <fstream>
#include <iostream>
#include <string>
#include <vector>
#include <sstream>
#include "Enums.h"
#include "Team.h"
#include "League.h"
using std::cout;

class Nation {
public:
	CountryCode country;
	std::vector<League> leagues;

	Nation() {
    country = ENG;
  }

  Nation(const CountryCode& c) {
    country = c;
  }

  void readNationTeams() {
    std::ifstream inStream("teams/ENG.csv");
    if (inStream.is_open()) {
      // finding how many leagues we're working with
      int numLeagues = 0;
      for (std::string buffer; getline(inStream, buffer); ) {
        if (buffer.length() > 0) {
          int intBuf = stoi( buffer.substr( 0, buffer.find(",") ) );
          if (intBuf > numLeagues) numLeagues = intBuf;
        }
      }
      // add the Leagues to the Nation
      for (int i = 0; i <= numLeagues; i++) 
        leagues.push_back( League( static_cast<Division>(i) ) );
      // tell the bottommost league to disallow relegation
      leagues[leagues.size()-1].relegationPositions = {};
      // clear stream so we can start reading from the top again
      inStream.clear();
      inStream.seekg(0, std::ios::beg);

      // start actually parsing the buffers
      for (std::string buffer; getline(inStream, buffer); ) {
        if (buffer.length() > 0 && buffer[0] != '#') {
          // finding all comma locations in the buffer
          int leagueComma = buffer.find(",");
          int teamComma = buffer.find(",", leagueComma+1);
          int formComma[] = {
            (int)buffer.find(",", teamComma+1),
            (int)buffer.find(",", formComma[0]+1),
            (int)buffer.find(",", formComma[1]+1),
            (int)buffer.find(",", formComma[2]+1),
          };
          // building a Team object to store the parsed info in
          Team* t = new Team();
          t->division = (Division)std::stoi(buffer.substr(0,leagueComma));
          if (leagues.size() < (unsigned)t->division+1) {
            League l( (Division)(leagues.size()) );
            leagues.push_back(l);
          }
          t->name = buffer.substr(leagueComma+1, teamComma-leagueComma-1);
          t->last5Seasons = {
            {stoi(buffer.substr(teamComma+1,    formComma[0]-teamComma-1)), false, false},
            {stoi(buffer.substr(formComma[0]+1, formComma[1]-formComma[0]-1)), false, false},
            {stoi(buffer.substr(formComma[1]+1, formComma[2]-formComma[1]-1)), false, false},
            {stoi(buffer.substr(formComma[2]+1, formComma[3]-formComma[2]-1)), false, false},
            {stoi(buffer.substr(formComma[3]+1)), false, false}
          };
          t->weighted = t->weightedLast5Seasons();
          t->top = (buffer.find(",top") != std::string::npos);
          t->lucky = t->unlucky = false;
          t->isPromoted = t->isRelegated = false;
          t->titles = 0;
          t->league = &(leagues[t->division]);
          t->file.open("teamhistories/" + t->name + ".txt");
          leagues[t->division].teams.push_back(t);
        }
      }
      inStream.close();
    }
  }

  void incrementSeason() {
    for (auto& l : leagues)
      for (auto& t : l.teams)
        t->updateLast5();

    for (int l = 0; l < leagues.size(); l++) {
      int leagueSizes[] = {20,24,24,24};
      cout << divisionCodes[l] << " Promoted:  ";
      for (int t = 0; t < leagues[l].teams.size(); ) {
        if (leagues[l].teams[t]->isPromoted) {
          Team* promoted = leagues[l].teams[t];
          printf("%-24s ", promoted->name.c_str());
          for (auto& p : promoted->last5Seasons) p.position += leagueSizes[l-1];
          promoted->weighted = promoted->weightedLast5Seasons();
          promoted->promote();
          promoted->isPromoted = false;
          leagues[l-1].teams.push_back(promoted);
          leagues[l].teams.erase(leagues[l].teams.begin() + t);
        } else t++;
      }
      cout << '\n' << divisionCodes[l] << " Relegated: ";
      for (int t = 0; t < leagues[l].teams.size(); ) {
        if (leagues[l].teams[t]->isRelegated) {
          Team* relegated = leagues[l].teams[t];
          printf("%-24s ", relegated->name.c_str());
          for (auto& p : relegated->last5Seasons) p.position -= leagueSizes[l];
          relegated->weighted = relegated->weightedLast5Seasons();
          relegated->relegate();
          relegated->isRelegated = false;
          leagues[l+1].teams.push_back(relegated);
          leagues[l].teams.erase(leagues[l].teams.begin() + t);
        } else t++;
      }
      cout << '\n';
    }

    // update season number
    int s1 = stoi(leagues[0].season.substr(0, 4)) + 1;
    int s2 = stoi(leagues[0].season.substr(5)) + 1;
    if (s2 > 99) s2 = 0;
    for (auto& l : leagues) {
      std::stringstream ss;
      ss << s1 << '/';
      if (s2 < 10) ss << '0';
      ss << s2;
      l.season = ss.str();
      l.year = s1 + 1;
      for (auto& t : l.teams) t->reset();
      for (auto& d : l.dates) d.year++;
      l.spareDate1.year++;
      l.spareDate2.year++;
    }
  }

  void readFixtureDates() {
    for (int i = 0; i < leagues.size(); i++) {
      std::string filename;
      if (i == 0) filename = "fixtures/20.csv";
      else        filename = "fixtures/46.csv";

      std::ifstream file(filename);
      if (file.is_open()) {
        for (std::string buffer; getline(file, buffer); ) {
          Date d(buffer);
          leagues[i].dates.push_back(d);
        }
        file.close();
      }
      // just in case no other fixtures are available for whatever reason
      leagues[i].spareDate1 = {2016, 10,  8};
      leagues[i].spareDate2 = {2017, 05, 16};
    }
  }

  void closeFiles() {
    for (auto& l : leagues) {
      for (auto t : l.teams) {
        t->file.close();
      }
    }
  }

};

#endif