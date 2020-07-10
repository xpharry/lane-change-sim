#include "model.h"

//************************************************************************
// class Model: method implementations
//************************************************************************

Model::Model(Layout& lay) : layout(lay) {
  initLines();
  initBlocks();
  initGraphs();
  initIntersections();
  // initOtherCars();

  int startX = layout.getStartX();
  int startY = layout.getStartY();
  string startDir = layout.getHostDir();
  vector<int> finishdata = layout.getFinish();
  finish = new Block(finishdata);
  host = new Host(Vector2f(startX, startY), startDir, Vector2f(0.0, 0.0));
  cars.push_back(host);

  for (vector<int> other : layout.getOtherData()) {
    Car* othercar =
        new Agent(Vector2f(other[0], other[1]), "east", Vector2f(0.0, 0.0));
    otherCars.push_back(othercar);
    cars.push_back(othercar);
  }

  cartoindex = UMAP<size_t, int>();

  for (int i = 0; i < otherCars.size(); i++) {
    cartoindex.insert({(size_t)otherCars[i], i});
  }
}

Model::Model(const Model& mo) : layout(mo.layout) {
  finish = mo.finish;
  blocks = mo.blocks;
  lines = mo.lines;
  interSections = mo.interSections;
  agentGraph = mo.agentGraph;
  hostGraph = mo.hostGraph;
  allGraph = mo.allGraph;
  host = new Host(*mo.getHost());
  host->setup();
  cars.push_back(host);

  for (Car* car : mo.getOtherCars()) {
    Car* othercar = new Agent(*car);
    othercar->setup();
    otherCars.push_back(othercar);
    cars.push_back(othercar);
  }
}

Model::~Model() {
  if (cars.size() != 0) {
    while (cars.size() != 0) {
      delete cars.back();
      cars.pop_back();
    }
  }
}

void Model::setHost(Car* car) {
  vector<Car*> cars;
  cars.push_back(car);
  for (auto c : cars) {
    if (typeid(*c) != typeid(*host)) cars.push_back(c);
  }
  this->cars = cars;
}

void Model::clearBlocks(vector<Block*>& bloc) {
  if (bloc.size() != 0) {
    while (bloc.size() != 0) {
      delete bloc.back();
      bloc.pop_back();
    }
  }
}

void Model::initBlocks() {
  for (vector<int> blockData : layout.getBlockData()) {
    blocks.push_back(new Block(blockData));
  }
}

void Model::initLines() {
  for (vector<int> lineData : layout.getLineData())
    lines.push_back(new Line(lineData));
}

void Model::initGraphs() {
  for (vector<int> data : layout.getHostGraph()) {
    Block* hostgraph = new Block(data);
    hostGraph.push_back(hostgraph);
    allGraph.push_back(hostgraph);
  }

  for (vector<int> data : layout.getAgentGraph()) {
    Block* agentgraph = new Block(data);
    agentGraph.push_back(agentgraph);
    allGraph.push_back(agentgraph);
  }
}

void Model::initIntersections() {
  for (vector<int> blockData : layout.getIntersectionData()) {
    Block* inter = new Block(blockData);
    interSections.push_back(inter);
    allGraph.push_back(inter);
  }
}
bool Model::checkVictory() const {

  vector<Vector2f> bounds = host->getBounds();
  for (Vector2f point : bounds) {
    if (finish->containsPoint(point[0], point[1])) return true;
  }

  return false;
}

bool Model::checkCollision(Car* car) const {
  vector<Vector2f> bounds = car->getBounds();
  for (Vector2f point : bounds) {
    if (!inBounds(point.x, point.y)) return true;
  }

  for (Car* othercar : cars) {
    if (othercar == car) continue;
    if (othercar->collides(car->getPos(), bounds)) return true;
  }

  return false;
}

bool Model::inBounds(float x, float y) const {
  if (x < 0 || x >= getWidth()) return false;
  if (y < 0 || y >= getHeight()) return false;
  for (const auto& it : blocks) {
    if (it->containsPoint(x, y)) return false;
  }
  return true;
}

bool Model::inBoundsLarger(float x, float y) const {
  if (x < 0 || x >= getWidth()) return false;
  if (y < 0 || y >= getHeight()) return false;
  for (const auto& it : blocks)
    if (it->containsPointLarger(x, y)) return false;
  return true;
}

bool Model::inIntersection(float x, float y) const {
  Block* result = getIntersection(x, y);
  return result != NULL;
}

Block* Model::getIntersection(float x, float y) const {
  for (int i = 0; i < interSections.size(); i++)
    if (interSections[i]->containsPoint(x, y)) return interSections[i];
  return NULL;
}

vector<Vector2f> Model::getIntersectionCenter() {
  vector<Vector2f> IntersectionCenter;
  for (const auto& it : interSections)
    IntersectionCenter.push_back(it->getCenter());
  return IntersectionCenter;
}

//************************************************************************
// class Car: method implementations
//************************************************************************

const float Car::RADIUS = sqrt(pow(Car::LENGTH, 2) + pow(Car::WIDTH, 2));

void Car::init(const string& dir) {
  pii p = direction[dir];
  this->dir = Vector2f(p.first, p.second);
  wheelAngle = 0;
  setup();
}

void Car::init() {
  wheelAngle = 0;
  setup();
}

void Car::setup() {
  maxSpeed = 5.0;
  friction = 0.5;
  maxWheelAngle = 130.0;
  maxaccler = 2.0;
  minSpeed = 1;
}

void Car::turnCarTowardsWheels() {
  if (velocity.Length() > 0.0) {
    velocity.rotate(wheelAngle);
    dir = Vector2f(velocity[0], velocity[1]);
    dir.normalized();
  }
}

void Car::update() {
  turnCarTowardsWheels();
  pos += velocity;
  turnWheelsTowardsStraight();
  applyFriction();
}

void Car::decellerate(float amount) {
  float speed = velocity.Length();

  if (speed < minSpeed) {
    speed = minSpeed;
    return;
  }

  Vector2f frictionVec = velocity.get_reflection();
  frictionVec.normalized();
  frictionVec *= amount;
  velocity += frictionVec;
  float angle = velocity.get_angle_between(frictionVec);

  if (abs(angle) < 180) velocity = Vector2f(0, 0);
}

void Car::setWheelAngle(float angle) {
  wheelAngle = angle;

  if (wheelAngle <= -maxWheelAngle) wheelAngle = -maxWheelAngle;
  if (wheelAngle >= maxWheelAngle) wheelAngle = maxWheelAngle;
}

void Car::accelerate(float amount) {
  amount = std::min(amount, maxaccler);

  if (amount < 0) decellerate(amount);
  if (amount == 0) return;

  Vector2f acceleration = Vector2f(dir[0], dir[1]);
  acceleration.normalized();
  acceleration *= amount;
  velocity += acceleration;

  if (velocity.Length() >= maxSpeed) {
    velocity.normalized();
    velocity *= maxSpeed;
  }
}

vector<Vector2f> Car::getBounds() {
  dir.normalized();
  Vector2f perpDir = dir.perpendicular();

  vector<Vector2f> bounds;
  bounds.push_back(pos + dir * float(LENGTH / 2) + perpDir * float(WIDTH / 2));
  bounds.push_back(pos + dir * float(LENGTH / 2) - perpDir * float(WIDTH / 2));
  bounds.push_back(pos - dir * float(LENGTH / 2) + perpDir * float(WIDTH / 2));
  bounds.push_back(pos - dir * float(LENGTH / 2) - perpDir * float(WIDTH / 2));

  return bounds;
}

vector<Vector2f> Car::getBounds(Car& car, float LEN, float WID) {
  Vector2f normalDir = normalized(car.getDir());
  Vector2f perpDir = normalDir.perpendicular();

  vector<Vector2f> bounds;
  bounds.push_back(pos + dir * float(LEN / 2) + perpDir * float(WID / 2));
  bounds.push_back(pos + dir * float(LEN / 2) - perpDir * float(WID / 2));
  bounds.push_back(pos - dir * float(LEN / 2) + perpDir * float(WID / 2));
  bounds.push_back(pos - dir * float(LEN / 2) - perpDir * float(WID / 2));

  return bounds;
}

//#
//http://www.gamedev.net/page/resources/_/technical/game-programming/2d-rotated-rectangle-collision-r2604
bool Car::collides(const Vector2f& otherPos,
                   const vector<Vector2f>& otherBounds) {
  Vector2f diff = otherPos - pos;
  float dist = diff.Length();
  if (dist > RADIUS * 2) return false;

  vector<Vector2f> bounds = getBounds();
  Vector2f vec1 = bounds[0] - bounds[1];
  Vector2f vec2 = otherBounds[0] - otherBounds[1];
  vector<Vector2f> axis = {vec1,
                           vec1.perpendicular(),
                           vec2,
                           vec2.perpendicular()
                           };

  for (const auto& vec : axis) {
    pff result = projectPoints(bounds, vec);
    float minA = result.first;
    float maxA = result.second;
    result = projectPoints(otherBounds, vec);
    float minB = result.first;
    float maxB = result.second;
    bool leftmostA = (minA <= minB) ? true : false;
    bool overlap = false;

    if (leftmostA && maxA >= minB) overlap = true;
    if (!leftmostA && maxB >= minA) overlap = true;
    if (!overlap) return false;
  }
  return true;
}

// carfufl not to too use the function, this is used for planning ahead
void Car::setVelocity(float amount) {
  Vector2f ve = Vector2f(dir[0], dir[1]);
  ve.normalized();
  ve *= amount;
  velocity = ve;
}

// check car is in instersection
bool Car::carInintersection(const Model& model) {
  vector<Vector2f> bounds = getBounds();
  for (const auto& point : bounds) {
    if (model.inIntersection(point[0], point[1])) return true;
  }
  return false;
}

bool Car::isCloseToOtherCar(const Model& model) const {
  //### check the master car is close to others
  vector<Car*> cars = model.getCars();
  if (cars.size() == 0) return false;
  const Car* obstaclecar = nullptr;
  float distance = 9999999;
  for (const Car* car : cars) {
    if (car == this) continue;
    float cardis = abs(car->getPos()[0] - getPos()[0]) +
                   abs(car->getPos()[1] - getPos()[1]);
    if (cardis < distance) {
      distance = cardis;
      obstaclecar = car;
    }
  }

  if (!obstaclecar) return false;

  Vector2f diff = obstaclecar->getPos() - this->getPos();
  float angdiff = -diff.get_angle_between(this->getDir());
  if (abs(angdiff) > 90) return false;
  // std::cout<<angdiff <<std::endl;

  if ((abs(obstaclecar->getPos()[0] - getPos()[0]) <
       Globals::constant.BELIEF_TILE_SIZE * 1.5) &&
      (abs(obstaclecar->getPos()[1] - getPos()[1]) < Car::WIDTH / 2))
    return true;
  return false;
}

//************************************************************************
// class Host: method implementations
//************************************************************************

void Host::setup() {
  maxSpeed = 3.0;
  friction = 1;
  maxWheelAngle = 45;
  maxaccler = 1.5;
  minSpeed = 1;
}

void Host::autonomousAction(const vector<Vector2f>& path, const Model& model,
                            kdtree::kdtree<point<float>>* tree = NULL) {
  if (path.size() == 0) return;

  Vector2f oldPos = getPos();
  Vector2f oldDir = getDir();
  // Vector2f oldVel = getVelocity();
  UMAP<string, float> actions = getAutonomousActions(path, model, tree);
  assert(getPos() == oldPos);
  assert(getDir() == oldDir);

  // assert (getVelocity() == oldVel);
  if (actions.count("DRIVE_FORWARD") != 0) {
    float percent = actions["DRIVE_FORWARD"];
    int sign = 1;
    if (percent < 0) sign = -1;
    percent = abs(percent);
    percent = percent > 0.0 ? percent : 0.0;
    percent = percent < 1.0 ? percent : 1.0;
    percent *= sign;
    accelerate(maxWheelAngle * percent);
    if (actions.count("TURN_WHEEL") != 0) {
      float turnAngle = actions["TURN_WHEEL"];
      setWheelAngle(turnAngle);
    }
  }
}

void Host::autonomousAction2(const vector<Vector2f>& path, const Model& model,
                             int i) {
  if (path.size() == 0) return;

  Vector2f oldPos = getPos();
  Vector2f oldDir = getDir();
  // Vector2f oldVel = getVelocity();

  UMAP<string, float> actions = getAutonomousActions2(path, model);

  assert(getPos() == oldPos);
  assert(getDir() == oldDir);
  // assert (getVelocity() == oldVel);

  if (actions.count("DRIVE_FORWARD") != 0) {
    float percent = actions["DRIVE_FORWARD"];
    int sign = 1;
    if (percent < 0) sign = -1;
    percent = abs(percent);
    percent = percent > 0.0 ? percent : 0.0;
    percent = percent < 1.0 ? percent : 1.0;
    percent *= sign;
    accelerate(maxWheelAngle * percent);
    if (actions.count("TURN_WHEEL") != 0) {
      float turnAngle = actions["TURN_WHEEL"];
      setWheelAngle(turnAngle);
    }
  }
}

// bool Host::isCloseToOtherCar(const Model& model) {
//
//    vector<Car*> cars = model.getOtherCars();
//    if (cars.size() == 0) return false;
//    Car* obstaclecar = nullptr;
//    float distance = inf;
//    for (Car*car: cars) {
//        float cardis = manhattanDistance(car->getPos(), getPos());
//        if (cardis < distance) {
//            distance = cardis;
//            obstaclecar = car;
//        }
//        }
//    if (abs(obstaclecar->getPos()[0] - getPos()[0]) <
//    Globals::constant.BELIEF_TILE_SIZE*1.5 &&
//        abs(obstaclecar->getPos()[1] - getPos()[1]) < Car::WIDTH/2)
//        return true;
//    return false;
//}

// void decisionMaking(const DecisionAgent& decision, const Model& model) {
//
//   if (isCloseToOtherCar(model))
//    action = agent.getAction(model);
//    return action;
//}

UMAP<string, float> Host::getAutonomousActions2(const vector<Vector2f>& path,
                                                const Model& model) {
  UMAP<string, float> output;
  if (nodeId >= path.size()) nodeId = 0;

  // set the timer to control time
  if (path.size() == 0) return output;
  int nextId;

  Vector2f vectogoal;
  nextId = nodeId + 1;
  if (nodeId >= path.size()) nodeId = pre;
  if (nextId > path.size()) nextId = nodeId;

  Vector2f nextpos = path[nextId];

  if (nextpos.get_distance(getPos()) <
      Globals::constant.BELIEF_TILE_SIZE * 0.3) {
    pre = nodeId;
    nodeId = nextId;
    nextId = nodeId + 1;
  }

  if (nextId >= path.size()) nextId = nodeId;

  //        goalPos = path[nextId];
  // we finish the checking of end point
  vectogoal = path[nextId] - getPos();
  float wheelAngle = -vectogoal.get_angle_between(getDir());
  int sign = (wheelAngle < 0) ? -1 : 1;
  wheelAngle = std::min(abs(wheelAngle), maxWheelAngle);

  output["TURN_WHEEL"] = wheelAngle * sign;
  output["DRIVE_FORWARD"] = 1.0;
  //    if (abs(wheelAngle) < 20) output["DRIVE_FORWARD"] = 1.0;
  //    else if(abs(wheelAngle) < 45) output["DRIVE_FORWARD"] = 0.8;
  //    else output["DRIVE_FORWARD"] = 0.5;

  return output;
}

void Host::makeObse(const Model& state) {
  vector<Car*> cars = state.getOtherCars();
  for (const auto& car : cars) {
    Vector2f obsv = dynamic_cast<Agent*>(car)->getObserv();
    float obs = obsv.Length();
    obs = obs > 0 ? obs : 0;
    if (dynamic_cast<Agent*>(car)->history.size() == 11)
      dynamic_cast<Agent*>(car)->history.pop();
    dynamic_cast<Agent*>(car)->history.push(obs);
  }
}

UMAP<string, float> Host::getAutonomousActions(
    const vector<Vector2f>& path, const Model& model,
    kdtree::kdtree<point<float>>* tree) {
  UMAP<string, float> output;
  if (nodeId > path.size()) nodeId = 0;

  static unsigned int timer = 0;
  static bool stopflag = false;

  // set the timer to control time
  if (timer < 30 && stopflag) {
    setVelocity(0.0);
    output["TURN_WHEEL"] = 0;
    output["DRIVE_FORWARD"] = 0;
    timer++;
    return output;
  }

  if (carInintersection(model) && !stopflag) {
    stopflag = true;
    // setVelocity(0.0);
    output["TURN_WHEEL"] = 0;
    output["DRIVE_FORWARD"] = 0;
    timer = 0;
  }
  // finished checking the

  if (isCloseToOtherCar(model)) {
    output["TURN_WHEEL"] = 0;
    output["DRIVE_FORWARD"] = 0;
    return output;
  }

  if (path.size() == 0) return output;

  int nextId;
  //= nodeId + 1;
  // if (nodeId>=path.size()) nodeId = pre;
  // if (nextId > path.size()) nextId = nodeId;

  Vector2f vectogoal;

  // chek the kd tree
  if ((tree != NULL) && (path.size() == tree->size())) {
    Vector2f mypos = getPos();
    vector<kdtree::node<point<float>>*> neighbors =
        tree->k_nearest(point<float>(mypos[0], mypos[1]), 2);
    point<float> p1 = neighbors[0]->point;
    point<float> p2 = neighbors[1]->point;
    //        Vector2f v1 = Vector2f(p1.x, p1.y);
    Vector2f v2 = Vector2f(p2.x, p2.y);
    //        Vector2f vectogoal1 = v1 - getPos();
    Vector2f vectogoal2 = v2 - getPos();
    float angle2 = abs(vectogoal2.get_angle_between(getDir()));
    if (angle2 < 90)
      nextId = p2.id;
    else
      nextId = p1.id;
  } else {
    nextId = nodeId + 1;
    if (nodeId >= path.size()) nodeId = pre;
    if (nextId > path.size()) nextId = nodeId;
  }

  Vector2f nextpos = path[nextId];

  if (nextpos.get_distance(getPos()) <
      Globals::constant.BELIEF_TILE_SIZE * 0.5) {
    pre = nodeId;
    nodeId = nextId;
    nextId = nodeId + 1;
  }

  if (nextId >= path.size()) nextId = nodeId;

  // we finish the checking of end point
  vectogoal = path[nextId] - getPos();
  float wheelAngle = -vectogoal.get_angle_between(getDir());
  int sign = (wheelAngle < 0) ? -1 : 1;
  wheelAngle = std::min(abs(wheelAngle), maxWheelAngle);

  output["TURN_WHEEL"] = wheelAngle * sign;
  output["DRIVE_FORWARD"] = 1.0;
  // if (abs(wheelAngle) < 20) output["DRIVE_FORWARD"] = 1.0;
  // else if(abs(wheelAngle) < 45) output["DRIVE_FORWARD"] = 0.8;
  // else output["DRIVE_FORWARD"] = 0.5;

  return output;
}

//************************************************************************
// class Agent: method implementations
//************************************************************************

void Agent::setup() {
  maxSpeed = 3.0;
  friction = 1;
  maxWheelAngle = 45;
  maxaccler = 1.4;
  minSpeed = 1;
  history = std::queue<float>();
  hasinference = false;
  inference = NULL;
}

void Agent::autonomousAction(const vector<Vector2f>& vec2, const Model& model,
                             kdtree::kdtree<point<float>>* tree) {
  /*
   * here we have three choices to choose: normal, acc, dec
   */
  // set the timer to control time
  if (timer < 30 && stopflag) {
    // setVelocity(0.0);
    timer++;
    return;
  }
  //
  bool check = carInintersection(model);
  if (check && !stopflag) {
    stopflag = true;
    accelerate(0);
    setWheelAngle(0);
    timer = 0;
    return;
  }

  if (isCloseToOtherCar(model)) {
    accelerate(0);
    setWheelAngle(0);
    return;
  }

  // unsigned int i = rand()%1;
  // assume it is not conservative for all drivers
  unsigned int i = 1;

  Car* host = model.getHost();

  // conservative driver will yield
  if ((host->getPos().x < this->getPos().x + Car::LENGTH * 4) &&
      (host->getPos().x > this->getPos().x))
    i = 0;
  switch (i) {
    case 0:
      accelerate(friction);
      setWheelAngle(0);
      break;
    case 1:
      accelerate(maxaccler);
      setWheelAngle(0);
      break;
    case 2:
      accelerate(maxaccler * 0.25);
      setWheelAngle(0);
      break;
    default:
      break;
  }
}

void Agent::autonomousAction2(const vector<Vector2f>& vec2, const Model& model,
                              int i) {
  // unsigned int i = rand()%1;
  // assume it is not conservative for all drivers
  switch (i) {
    case 0:
      accelerate(friction);
      setWheelAngle(0);
      break;
    case 1:
      accelerate(maxaccler);
      setWheelAngle(0);
      break;
    default:
      break;
  }
}

//************************************************************************
// class Inference: method implementations
//************************************************************************

Inference::MarginalInference* Agent::getInference(int index,
                                                  const Model& state) {
  if (!hasinference) {
    inference = new Inference::MarginalInference(index, state);
    hasinference = true;
    return inference;
  }

  return inference;
}