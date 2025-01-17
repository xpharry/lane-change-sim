#ifndef DECISION_MAKING_H
#define DECISION_MAKING_H

#include "search.h"

inline static int xToCol(float x) {
  return int(x / (Globals::constant.BELIEF_TILE_SIZE));
}

inline static int yToRow(float y) {
  return int((y / Globals::constant.BELIEF_TILE_SIZE));
}

inline static float rowToY(int row) {
  return (row + 0.5) * Globals::constant.BELIEF_TILE_SIZE;
}

inline static float colToX(int col) {
  return (col + 0.5) * Globals::constant.BELIEF_TILE_SIZE;
}

inline float manhattanDistance(const Vector2f& v1, const Vector2f& v2) {
  float distance = abs(v1[0] - v2[0]) + abs(v1[1] - v2[1]);
  return distance;
}

/*
 * This class provides some common elements to all of your
 * multi-agent searchers.  Any methods defined here will be available
 * Note: this is an abstract class: one that should not be instantiated.  It's
 * only partially specified, and designed to be extended.  Car (game.py)
 * is another abstract class.
 */
class DecisionMaker {
public:
  static vector<std::string> m_host_actions;
  static vector<std::string> m_other_actions;
  static unordered_map<std::string, float> m_action_rewards;
  // static const unorder_mapd<std::string, float> command;

  DecisionMaker(int dep = 2, int ind = 0) : depth(dep), index(ind) {}

  vector<string> generateLegalActions(const Simulation&);

  vector<vector<Vec2f>>& generatePaths(const Simulation&, vector<string>&);

  void applyAction(const Simulation&, int, const std::string&);

  float evaluatePath(const Simulation&, const vector<Vec2f>& path, vector<int>& car_intentions);

  bool getPath(const Simulation& simulation, vector<Vec2f>& final_path, vector<int>& carIntentions);

  vector<vector<Vec2f>> getPaths() { return paths; }

  bool isCloseToOtherCar(Actor* car, const Simulation& simulation) const;

  bool isChangeRequired(const Simulation& simulation);

private:
  int depth;
  unsigned int index;
  vector<vector<Vec2f>> paths;
};

#endif /* DECISION_MAKING_H */
