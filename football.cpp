#include <cstdlib> 
#include <ctime>
#include <chrono>
#include <fstream>
#include "headers/Nation.h"
#include "headers/League.h"
#include "headers/Enums.h"
using namespace std;
namespace chr = std::chrono;

int main() {
  srand( (unsigned)time(0) );
  std::ofstream titleFile("titles.txt");

  std::vector<Nation> nations { ENG };
  for (auto& n : nations) {
    n.readNationTeams();
    n.readFixtureDates();
  }

  chr::time_point<chr::system_clock> t0, t1, t2, t3, t4, t5;
  double tFixtures = 0;
  double tMatches = 0;
  double tSort = 0;
  double tPrint = 0;
  double tPlayoffs = 0;
  std::string buffer;
  while (nations[0].leagues[0].year <= 4000) {
    for (auto& n : nations) {
      for (auto& l : n.leagues) {
        t0 = chr::system_clock::now();
        l.generateFixtures();
        t1 = chr::system_clock::now();
        l.processMatches(false);
        t2 = chr::system_clock::now();
        l.sortTable();
        t3 = chr::system_clock::now();
        l.printTable(titleFile);
        t4 = chr::system_clock::now();
        if (l.contains()) getline(std::cin, buffer);
        l.playoffs();
        t5 = chr::system_clock::now();
        if (l.contains() && l.division != PL) getline(std::cin, buffer);

        tFixtures += std::chrono::duration<double>(t1-t0).count();
        tMatches += std::chrono::duration<double>(t2-t1).count();
        tSort += std::chrono::duration<double>(t3-t2).count();
        tPrint += std::chrono::duration<double>(t4-t3).count();
        tPlayoffs += std::chrono::duration<double>(t5-t4).count();
      }
      n.incrementSeason();
      if (buffer == "quit") break;
    }
  }
  titleFile.close();
  for (auto& n : nations) n.closeFiles();
  cout << "Fixtures = " << tFixtures << '\n';
  cout << "Matches  = " << tMatches << '\n';
  cout << "Sorting  = " << tSort << '\n';
  cout << "Printing = " << tPrint << '\n';
  cout << "Playoffs = " << tPlayoffs << '\n';
}
  // TO-DO
    // take it round by round, print each set of fixtures and the update table
      // green up arrows and red down arrows to show changes in position
      // maybe just for PL?
    // FA/League cups with their own draws, etc
      // extra time in playoffs/cups
    // organise seasons as a vector of fixtures for each league
      // then jumble them all together  with every other competition
      // add a pointer to each Match that links back to the respective competition and updates it
    // 11 players for each team with match ratings
    // at the end of every other season the best 11 players from each nation get called up for WC
    // include money spent?