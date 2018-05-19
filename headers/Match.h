#ifndef MATCH_H
#define MATCH_H

class Match {
public:
  Competition* competition;
	Date date;
	Team* home;
	Team* away;
	Result result;
	int homeGoals;
	int awayGoals;
	float homeOdds;
	float awayOdds;
  int homeShots;
  int awayShots;
  std::vector<Goal> goals;
  bool isNeutral;

  Match();
  Match(const Date& d, const Team* h, const Team* a, const bool b_isNeutral = false);
  Match(const Match& m);
  inline bool contains(const Team* t) const;
  inline bool contains(const std::string& t) const;
  void updateForm();
  bool takeShot(const float odds, const float oddsDiff, int& shots, int& goals);
  void calculateResult(const bool extraTime = false);
  void calculateOdds();
  void swap(Team* oldTeam, Team* newTeam);
};

#endif