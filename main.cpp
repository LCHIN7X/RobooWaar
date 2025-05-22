#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <sstream>
#include <random>
#include <stdexcept>
#include <cstdlib>
#include <queue>
#include <ctime>
#include <map>
#include <algorithm>
#include <unordered_map>

using namespace std;

// Forward declarations
class Robot;

class Logger
{
private:
    ofstream outputFile;
    string fileName = "outputFile.txt";

//**********************************************************
// Logger class that logs all activity (cout and output to file)
//**********************************************************
public:
    Logger()
    {
        cout << "Logger class starting up..." << endl;
        open();
    };

    bool open()
    {
        outputFile.open(fileName);
        if (outputFile.is_open())
        {
            return true;
        }
        return false;
    }

    void close()
    {
        if (outputFile.is_open())
        {
            outputFile.close();
        }
    }

    template <typename T>
    Logger &operator<<(const T &outputContent)
    {
        cout << outputContent;
        outputFile << outputContent;
        return *this;
    }

    Logger &operator<<(ostream &(*manip)(ostream &))
    {
        manip(cout);
        manip(outputFile);
        return *this;
    }

    ~Logger()
    {
        close();
    }
};

Logger logger;

//**********************************************************
// Battlefield class that keeps track of all activity
//**********************************************************
class Battlefield
{
private:
    int height; // this is the m value from m x n
    int width;  // this is the n value from m x n
    int steps;  // max number of simulation steps
    int numberOfRobots;
    vector<Robot *> listOfRobots;            // stores all robots
    vector<vector<Robot *>> battlefieldGrid; // battlefield grid
    vector<pair<string, int>> respawnQueue;  // Tracks robot name, robot lives in pairs
    map<string, int> respawnCounts;
    queue<pair<string, int>> reentryQueue;   // Queue for reentry

    // TODO for Battlefield:
    // - fix bugs if any

    // TODO others:
    // - Detailed comments
    // - More error handling

public:
    Battlefield() : height(0), width(0), steps(0), numberOfRobots(0) {};
    ~Battlefield();
    void setDimensions(int h, int w)
    {
        height = h;
        width = w;
        battlefieldGrid.assign(height, vector<Robot *>(width, nullptr));
    }

    void printDimensions() const
    {
        logger << "Height: " << height << endl;
        logger << "Width: " << width << endl;
    }

    void setSteps(int s)
    {
        steps = s;
    }

    void printSteps() const
    {
        logger << steps;
    }

    int getSteps() const
    {
        return steps;
    }

    void setNumberOfRobots(int n)
    {
        numberOfRobots = n;
    }

    void printNumberOfRobots() const
    {
        logger << numberOfRobots;
    }

    int getNumberOfRobots() const
    {
        return numberOfRobots;
    }

    const vector<Robot *> getListOfRobots() const
    {
        return listOfRobots;
    }

    int getWidth() const { return width; }
    int getHeight() const { return height; }

    // Function prototypes (definitions are after Robot class)
    void simulationStep();
    void addNewRobot(Robot *robot);
    int getNumberOfAliveRobots();
    void cleanupDestroyedRobots();
    void respawnRobots();
    void queueForReentry(Robot *robot);

    Robot *getRobotAt(int x, int y) const;
    void placeRobot(Robot *robot, int x, int y);
    void removeRobotFromGrid(Robot *robot);
    bool isPositionAvailable(int x, int y);
    bool isPositionWithinGrid(int x, int y) const;

    void displayBattlefield();
};

//******************************************
// Abstract base Robot class
//******************************************

class Robot
{
private:
    int positionX;
    int positionY;

protected:
    string name;
    int lives;
    bool hidden;
    bool isDie = false; // true if robot is out of the game (no lives or no ammo)
    bool isHurt = false; // true if robot is hit this turn and should be requeued
    virtual bool isHit() = 0;
    bool getEnemyDetectedNearby() const;

public:
    Robot(string name, int x, int y)
        : name(name), positionX(x), positionY(y), lives(3), hidden(false) {}

    virtual ~Robot() = default;

    virtual void think() = 0;
    virtual void act() = 0;
    virtual void move(Battlefield &battlefield) = 0;
    virtual void fire(Battlefield &battlefield) = 0;

    // Common robot functions
    string getName() const { return name; }
    int getX() const { return positionX; }
    int getY() const { return positionY; }
    int getLives() const { return lives; }
    void setLives(int numOfLives) { lives = numOfLives; }
    bool isHidden() const { return hidden; }
    bool getIsDie() const { return isDie; }
    void setIsDie(bool val) { isDie = val; }
    bool getIsHurt() const { return isHurt; }
    void setIsHurt(bool val) { isHurt = val; }

    void setPosition(int x, int y)
    {
        positionX = x;
        positionY = y;
    }

    void takeDamage()
    {
        if (!hidden)
        {
            lives--;
            if (lives <= 0)
            {
                logger << name << " has been destroyed!\n";
                isDie = true; // Mark as dead for this turn
            }
            isHurt = true; // Mark as hurt for requeue
        }
    }

    void setHidden(bool state) { hidden = state; }
};

//******************************************
// MovingRobot class
//*******************************************

class MovingRobot : public virtual Robot
{
protected:
    int moveCount;

public:
    MovingRobot(const string &name, int x, int y)
        : Robot(name, x, y), moveCount(0) {}

    virtual ~MovingRobot() = default;
    virtual void move(Battlefield &battlefield) = 0;

    bool isValidMove(int newX, int newY, const Battlefield &battlefield) const
    {
        // Only allow moving to adjacent cell (1 step in any direction)
        int dx = abs(newX - getX());
        int dy = abs(newY - getY());

        bool oneStep = (dx + dy == 1); // Manhattan distance of 1 (no diagonal)
        return oneStep && newX >= 0 && newX < battlefield.getWidth() &&
               newY >= 0 && newY < battlefield.getHeight();
    }

    void incrementMoveCount() { moveCount++; }
};

//******************************************
// Shooting Robot class
//******************************************
class ShootingRobot : public virtual Robot
{
protected:
    int ammo;

public:
    ShootingRobot(const string &name, int x, int y, int initialAmmo)
        : Robot(name, x, y), ammo(initialAmmo) {}

    virtual ~ShootingRobot() = default;
    virtual void fire(Battlefield &battlefield) = 0;
    bool hasAmmo() const { return ammo > 0; }

    void useAmmo()
    {
        if (ammo > 0)
            ammo--;
    }

    int getAmmo() const { return ammo; }

    // Stimulate and check hit probablity is 70 percent
    bool hitProbability() const
    {
        static random_device rd;                   // True random number generator
        static mt19937 gen(rd());                  // Mersenne Twister pseudo-random generator seeded with rd
        uniform_real_distribution<> dis(0.0, 1.0); // Distribution for numbers between 0.0 and 1.0
        return dis(gen) <= 0.7;                    // 70% hit chance
    }
};

//******************************************
// Seeing Robot class
//******************************************

class SeeingRobot : public virtual Robot
{
protected:
    int visionRange;
    std::vector<Robot*> detectedTargets;
    bool enemyDetectedNearby;  

public:
    SeeingRobot(const string &name, int x, int y, int range)
        : Robot(name, x, y), visionRange(range) {}

    virtual ~SeeingRobot() = default;
    virtual void look(Battlefield &battlefield) = 0;

    const std::vector<Robot*>& getDetectedTargets() const { return detectedTargets; }

    void setDetectedTargets(const std::vector<Robot*>& targets) { detectedTargets = targets; }
    bool getEnemyDetectedNearby() const { return enemyDetectedNearby; }
    void setEnemyDetectedNearby(bool detected) { enemyDetectedNearby = detected; }
};

//******************************************
// Thinking Robot class
//******************************************

class ThinkingRobot : public virtual Robot
{
protected:
    int strategyLevel;

public:
    ThinkingRobot(const string &name, int x, int y, int strategy)
        : Robot(name, x, y), strategyLevel(strategy) {}

    virtual ~ThinkingRobot() = default;
    void think() override
    {
        logger << ">> " << name << " is thinking..." << endl;
    }

    int getStrategyLevel() const { return strategyLevel; }
};

//******************************************
// GenericRobot class
//******************************************

class GenericRobot : public MovingRobot, public ShootingRobot, public SeeingRobot, public ThinkingRobot
{
protected:
    bool hasUpgraded[3] = {false, false, false}; // Track upgrades for moving, shooting, seeing
    Battlefield* battlefield = nullptr; // Pointer to current battlefield
    bool pendingUpgrade = false;
    std::string upgradeType = "";
    bool enemyDetectedNearby = false; // Flag for detecting nearby enemies
    std::vector<Robot*> detectedTargets; // <-- Add this line
    std::vector<std::string> upgrades;
    std::vector<std::pair<int, int>> availableSpaces;

    // Action flags for per-round limitation
    bool hasLooked = false;
    bool hasMoved = false;
    bool hasThought = false;
    bool hasFired = false;

public:
    GenericRobot(const string &name, int x, int y);
    ~GenericRobot() override;
    void setBattlefield(Battlefield* bf);
    void think() override;
    void act() override;
    //std::string decideAction() const override;
    void move(Battlefield &battlefield) override;
    void fire(Battlefield &battlefield) override;
    void look(Battlefield &battlefield) override;
    bool canUpgrade(int area) const;
    void setUpgraded(int area);
    bool isHit() override;
    void setPendingUpgrade(const std::string& type);
    bool PendingUpgrade() const;
    std::string getUpgradeType() const;
    void clearPendingUpgrade();
    bool getEnemyDetectedNearby() const;

    // Reset all action flags (call at start/end of each round)
    void resetActionFlags() {
        hasLooked = false;
        hasMoved = false;
        hasThought = false;
        hasFired = false;
    }
};

// Definitions outside the class body
GenericRobot::GenericRobot(const string &name, int x, int y)
    : Robot(name, x, y),
      MovingRobot(name, x, y),
      ShootingRobot(name, x, y, 10),
      SeeingRobot(name, x, y, 1),
      ThinkingRobot(name, x, y, 1)
{
}

GenericRobot::~GenericRobot() = default;

void GenericRobot::setBattlefield(Battlefield* bf) { battlefield = bf; }

void GenericRobot::think()
{
    if (hasThought) return;
    hasThought = true;
    logger << ">> " << name << " is thinking...\n";
    if (getEnemyDetectedNearby()) {
        fire(*battlefield);
        move(*battlefield);
    } else {
        move(*battlefield);
        fire(*battlefield);
    }
}
void GenericRobot::act()
{
    if (isDie || isHurt || getLives() <= 0) return; // If robot is dead or hurt, skip action
    if (!battlefield) {
        logger << name << " has no battlefield context!" << endl;
        return;
    }
    look(*battlefield);
    think();
    detectedTargets.clear();
}

void GenericRobot::move(Battlefield &battlefield)
{
    if (hasMoved) return;
    hasMoved = true;
    logger << ">> " << name << " is moving...\n";
    // If availableSpaces (from look) is not empty, pick a random one
    if (!availableSpaces.empty()) {
        static std::random_device rd;
        static std::mt19937 g(rd());
        std::uniform_int_distribution<> dist(0, availableSpaces.size() - 1);
        auto [newX, newY] = availableSpaces[dist(g)];
        battlefield.removeRobotFromGrid(this);
        battlefield.placeRobot(this, newX, newY);
        incrementMoveCount();
        logger << name << " moved to (" << newX << ", " << newY << ")\n";
        return;
    }
    // Fallback: try the old adjacent logic if no availableSpaces (shouldn't happen)
    int dx[] = {0, 1, 0, -1};
    int dy[] = {1, 0, -1, 0};
    for (int i = 0; i < 4; ++i) {
        int newX = getX() + dx[i];
        int newY = getY() + dy[i];

        if (isValidMove(newX, newY, battlefield) && battlefield.isPositionAvailable(newX, newY)) {
            if (battlefield.getRobotAt(newX, newY) == nullptr) {
                battlefield.removeRobotFromGrid(this);
                battlefield.placeRobot(this, newX, newY);
                incrementMoveCount();
                logger << name << " moved to (" << newX << ", " << newY << ")\n";
                return;
            }
        }
    }
    logger << name << " could not move (no available adjacent cell).\n";
}

void GenericRobot::fire(Battlefield &battlefield)
{
    if (hasFired) return;
    hasFired = true;
    if (hasAmmo()) {
        if (!detectedTargets.empty()) {
            int idx = rand() % detectedTargets.size();
            Robot* target = detectedTargets[idx];
            int targetX = target->getX();
            int targetY = target->getY();
            logger << ">> " << name << " fires at (" << targetX << ", " << targetY << ")" << std::endl;
            useAmmo();
            if (target->isHidden()) {
                logger << target->getName() << " is hidden, attack miss." << std::endl;
            }
            else if (hitProbability()) {
                logger << "Hit! (" << target->getName() << ") be killed" << std::endl;
                target->takeDamage();
                static const std::vector<std::string> types = {"HideBot","JumpBot","LongShotBot","SemiAutoBot","ThirtyShotBot","ScoutBot","TrackBot","KnightBot","QueenBot"};
                int t = rand() % types.size();
                setPendingUpgrade(types[t]);
                logger << name << " will upgrade into " << types[t] << " next turn!" << std::endl;
            }
            else {
                logger << "Missed!" << std::endl;
            }
        }
        else {
            useAmmo();
            logger << ">> " << name << " fires." << endl;
            logger << "However no robots within shooting range ." << std::endl;
        }
        if (getAmmo() == 0) {
            isDie = true; // Out of ammo, robot is dead
        }
    }
    else {
        logger << name << " has no ammo left. It will self destroy!" << std::endl;
        lives = 0;
        isDie = true;
    }
    detectedTargets.clear();
}

void GenericRobot::look(Battlefield &battlefield)
{
    if (hasLooked) return;
    hasLooked = true;
    logger << ">> " << name << " is scanning surroundings...." << endl;
    int cx = getX();
    int cy = getY();

    enemyDetectedNearby = false;
    availableSpaces.clear(); // Clear previous available spaces
    //detectedTargets.clear();
    for (int dx = -1; dx <= 1; ++dx) {
        for (int dy = -1; ++dy <= 1; ++dy) {
            int nx = cx + dx;
            int ny = cy + dy;
            logger << "Checking (" << nx << ", " << ny << "): ";
            if (!battlefield.isPositionWithinGrid(nx, ny)) {
                logger << "Exceed Boundary" << endl;
            } else if (nx == cx && ny == cy) {
                logger << "Current Position" << endl;
            } else {
                Robot* r = battlefield.getRobotAt(nx, ny);
                if (r == nullptr) {
                    logger << "In Boundary but Empty" << endl;
                    availableSpaces.push_back({nx, ny}); // Add to available spaces
                } else if (r->getLives() > 0 && !r->getIsDie()){
                    logger << "Enemy detected: " << r->getName() << " at (" << nx << "," << ny << ")" << endl;
                    enemyDetectedNearby = true;
                    bool alreadyDetected = false;
                    for (Robot* existingRobot : detectedTargets) {
                        if (existingRobot == r) {
                            alreadyDetected = true;
                            break;
                        }
                    }
                    if (!alreadyDetected) {
                        detectedTargets.push_back(r);
                    }
                }
                else {
                    logger << "Dead Robot" << endl;
                }
            }
        }
    }
}

bool GenericRobot::canUpgrade(int area) const
{
    if (area < 0 || area > 2)
        return false;
    return !hasUpgraded[area];
}

void GenericRobot::setUpgraded(int area)
{
    if (area >= 0 && area < 3)
    {
        hasUpgraded[area] = true;
    }
}

bool GenericRobot::isHit()
{
    return true;
}

void GenericRobot::setPendingUpgrade(const std::string& type) { pendingUpgrade = true; upgradeType = type; }
bool GenericRobot::PendingUpgrade() const { return pendingUpgrade; }
std::string GenericRobot::getUpgradeType() const { return upgradeType; }
void GenericRobot::clearPendingUpgrade() { pendingUpgrade = false; upgradeType = ""; }
bool GenericRobot::getEnemyDetectedNearby() const { return enemyDetectedNearby; }

//******************************************
// HideBot
//******************************************

class HideBot : public GenericRobot
{

private:
    int hide_count = 0;
    bool isHidden = false;

public:
    HideBot(const string &name, int x, int y)
        : Robot(name, x, y),
          GenericRobot(name, x, y) {}

    void move(Battlefield &battlefield) override
    {
        if (hide_count < 3 && rand() % 2 == 0)
        { 
            isHidden = true;
            hide_count++;
            setHidden(true);
            logger << getName() << " hide,(" << hide_count << "/3)" << endl;
        }
        else
        {
            isHidden = false;
            setHidden(false);

            if (hide_count >= 3)
                logger << getName() << " finish use hide , keep moving" << endl;
            else
                logger << getName() << " did not hide this turn, keep moving" << endl;
        }
    }

    bool getHiddenStatus() const
    {
        return isHidden;
    }

    void appear()
    {
        isHidden = false;
    }

    void act() override
    {
        cout << "JumpBot is thinking..." << endl;
        //TO DO : the logic will be implemented later
        look(*battlefield);
        fire(*battlefield);
        move(*battlefield);
    }

    bool isHit() override
    {
        return !isHidden;
    } 
};

//******************************************
// Jumpbot
//******************************************
class JumpBot : public GenericRobot
{
private:
    int jump_count = 0;

public:
    JumpBot(const string &name, int x, int y)
        : Robot(name, x, y),
          GenericRobot(name, x, y) {}

    void move(Battlefield &battlefield) override
    {

        if (jump_count < 3 && rand() % 2 == 0)
        {
            jump_count++;
            int jumpx = rand() % battlefield.getWidth();
            int jumpy = rand() % battlefield.getHeight();

            setPosition(jumpx, jumpy);
            logger << getName() << " jump to (" << jumpx << "," << jumpy << "), ("<< jump_count<<"/3)\n";
        }
        else
        {
            if (jump_count >= 3){
                logger<<getName()<< " cannot jump already \n";
            }
            else{
                logger<<getName()<<"did not jump this turn, keep moving\n";
            }
            
        }
    }

    void act() override
    {
        cout << "JumpBot is thinking..." << endl;
        //TO DO : the logic will be implemented later
        look(*battlefield);
        fire(*battlefield);
        move(*battlefield);
    }

    int getJumpCount() const
    {
        return jump_count;
    }
};

//******************************************
// LongShotBot
//******************************************

class LongShotBot : public GenericRobot
{
private:
    int fire_count = 0;
    const vector<string> upgradeTypes = {"HideBot", "JumpBot", "ScoutBot", "TrackBot"};

public:
    LongShotBot(const string &name, int x, int y)
        : Robot(name, x, y),
          GenericRobot(name, x, y) {}

    void fire(Battlefield &battlefield) override
    {

        bool fired = false;
        int x = getX();
        int y = getY();

        for (int dx = -3; dx <= 3; dx++)
        {
            for (int dy = -3; dy <= 3; dy++)
            {
                if (dx == 0 && dy == 0)
                    continue;
                if (abs(dx) + abs(dy) > 3)
                    continue;

                int targetX = x + dx;
                int targetY = y + dy;

                Robot *target = battlefield.getRobotAt(targetX, targetY);

                if (target && target != this)
                {

                    GenericRobot *gtarget = dynamic_cast<GenericRobot *>(target);
                    logger << getName() << " fire (" << targetX << "," << targetY << ")" << endl;
                    if (gtarget->isHit())
                    {
                        gtarget->takeDamage();
                        logger << getName() << " hit the target " << gtarget->getName() << endl;
                        fire_count++;
                        fired = true;
                        int t = rand() % upgradeTypes.size();
                        string newType = upgradeTypes[t];
                        setPendingUpgrade(newType);
                        logger << getName() << " will upgrade into " << newType << " next turn!" << endl;
                        break;
                    }
                }
            }
            if (fired)
                break;
        }
        if (!fired)
        {
            logger << getName() << " no robot there\n";
        }
    }
};

//******************************************
// SemiAutoBot
//******************************************

class SemiAutoBot : public GenericRobot
{
private:
    int fire_count = 0;
    const vector<string> upgradeTypes = {"HideBot", "JumpBot", "ScoutBot", "TrackBot"};

public:
    SemiAutoBot(const string &name, int x, int y)
        : Robot(name, x, y),
          GenericRobot(name, x, y) {}

    void fire(Battlefield &battlefield) override
    {
        int x = getX();
        int y = getY();

        Robot *target = nullptr;
        for (Robot *r : battlefield.getListOfRobots())
        {
            if (r != nullptr && r != this && r->getLives() > 0)
            {
                target = r;
                break;
            }
        }

        if (!target)
        {
            logger << getName() << " sadly didnt hit any robot\n";
            return;
        }

        GenericRobot *gtarget = dynamic_cast<GenericRobot *>(target);

        logger << getName() << "fire 3 consecutive shoot at (" << x << "," << y << ")\n";

        bool hitSuccessful = false;
        for (int i = 0; i < 3; i++)
        {
            double chance = (double)rand() / RAND_MAX;
            if (chance < 0.7)
            {
                logger << "shot " << (i + 1) << " successful hit the robot\n"
                       << gtarget->getName() << "!\n";
                if (gtarget->isHit())
                {
                    gtarget->takeDamage();
                    fire_count++;
                    hitSuccessful = true;
                }
            }
            else
            {
                logger << "shot " << (i + 1) << " is miss\n";
            }
        }
        if (hitSuccessful)
        {
            int t = rand() % upgradeTypes.size();
            string newType = upgradeTypes[t];
            setPendingUpgrade(newType);
            logger << getName() << " will upgrade into " << newType << " next turn!\n";
        }
    }
    int getFireCount() const
    {
        return fire_count;
    }
};

//******************************************
// ThirtyShotBot
//******************************************
class ThirtyShotBot : public GenericRobot
{
private:
    int shell_count;
    const vector<string> upgradeTypes = {"HideBot", "JumpBot", "ScoutBot", "TrackBot"};

public:
    ThirtyShotBot(const string &name, int x, int y)
        : Robot(name, x, y),
          GenericRobot(name, x, y),
          shell_count(30)
    {
        logger << name << " got 30 shells replace current shells \n";
    }

    void fire(Battlefield &battlefield) override
    {
        if (shell_count <= 0)
        {
            logger << getName() << " shell is finish\n";
            return;
        }

        int x = getX();
        int y = getY();
        bool fired = false;
        bool hitSuccessful = false;

        for (int dx = -1; dx <= 1 && !fired; dx++)
        {
            for (int dy = -1; dy <= 1 && !fired; dy++)
            {
                int targetX = x + dx;
                int targetY = y + dy;

                Robot *target = battlefield.getRobotAt(targetX, targetY);
                if (target && target != this)
                {
                    GenericRobot *gtarget = dynamic_cast<GenericRobot *>(target);
                    if (gtarget && gtarget->isHit())
                    {
                        gtarget->takeDamage();
                        shell_count--;
                        logger << getName() << " fire at (" << targetX << ", " << targetY << "), shell left: " << shell_count << "\n";
                        logger << getName() << " fire at (" << targetX << ", " << targetY << "), shell left: " << shell_count << "\n";
                        fired = true;

                        // Display target that be hitted
                        if (gtarget->isHit())
                        {
                            gtarget->takeDamage();
                            hitSuccessful = true;
                            logger << "Successful hit on " << gtarget->getName() << "!\n";

                            int t = rand() % upgradeTypes.size();
                            string newType = upgradeTypes[t];
                            setPendingUpgrade(newType);
                            logger << getName() << " will upgrade into " << newType << " next turn!\n";
                        }
                    }
                }
            }
        }

        if (!fired)
        {
            logger << getName() << " no target to shoot\n";
        }
    }

    int getShellCount() const
    {
        return shell_count;
    }
};

//******************************************
// KnightBot
//******************************************

class KnightBot : public GenericRobot
{
private:
    int fire_count = 0;
    const vector<string> upgradeTypes = {"HideBot", "JumpBot", "ScoutBot", "TrackBot"};

public:
    KnightBot(const string &name, int x, int y)
        : Robot(name, x, y),
          GenericRobot(name, x, y) {}

    void fire(Battlefield &battlefield) override
    {
        int x = getX();
        int y = getY();
        bool anyFired = false;
        bool hitSuccessful = false;
        std::vector<std::string> hitRobots;

        for (Robot *target : battlefield.getListOfRobots())
        {
            if (target && target != this && target->getLives() > 0)
            {
                int dx = target->getX() - x;
                int dy = target->getY() - y;
                double distance = sqrt(dx * dx + dy * dy);
                if (distance <= 8.0)
                {
                    GenericRobot *gtarget = dynamic_cast<GenericRobot *>(target);
                    logger << getName() << " fires at (" << target->getX() << "," << target->getY() << ")" << endl;
                    anyFired = true;
                    if (gtarget && gtarget->isHit())
                    {
                        gtarget->takeDamage();
                        fire_count++;
                        hitSuccessful = true;
                        hitRobots.push_back(gtarget->getName());
                    }
                }
            }
        }
        if (!anyFired)
        {
            logger << getName() << " no robots in range to fire at\n";
        }
        else
        {
            logger << getName() << " hit the following robots: ";
            for (size_t i = 0; i < hitRobots.size(); ++i)
            {
                logger << hitRobots[i];
                if (i != hitRobots.size() - 1)
                    logger << ", ";
            }
            logger << endl;
            int t = rand() % upgradeTypes.size();
            string newType = upgradeTypes[t];
            setPendingUpgrade(newType);
            logger << getName() << " will upgrade into " << newType << " next turn!" << endl;
        }
    }
};

//******************************************
// ScoutBot
//******************************************
class ScoutBot : public GenericRobot
{
private:
    int scout_count = 0;

public:
    ScoutBot(const string &name, int x, int y)
        : Robot(name, x, y),
          GenericRobot(name, x, y) {}

    void look(Battlefield &battlefield)
    {
        if (scout_count >= 3)
        {
            logger << getName() << " reach the limit,cannot scan already\n";
            return;
        }

        if (rand() % 2 == 0)
        {
            logger << getName() << " scan the battlefield\n";

            for (int y = 0; y < battlefield.getHeight(); ++y)
            {
                for (int x = 0; x < battlefield.getWidth(); ++x)
                {
                    Robot *r = battlefield.getRobotAt(x, y);
                    if (r)
                    {
                        logger << "got robot: " << r->getName()
                             << " at (" << x << "," << y << ")\n";
                    }
                }
            }

            scout_count++;
        }
        else
        {
            logger << getName() << " try scan it next round \n";
        }
    }

    void act() override
    {
        think();
        look(*battlefield);
        fire(*battlefield);
        move(*battlefield);
    }

    int getScoutCount() const
    {
        return scout_count;
    }
};

//******************************************
// TrackBot
//******************************************

class TrackBot : public GenericRobot
{
private:
    int tracker = 3;
    vector<Robot *> track_target;

public:
    TrackBot(const string &name, int x, int y)
        : Robot(name, x, y),
          GenericRobot(name, x, y) {}

    void look(Battlefield &battlefield)
    {
        if (tracker == 0)
        {
            logger << getName() << " cannot track robot already\n";
            return;
        }

        int x = getX();
        int y = getY();
        bool plant = false;

        for (int dx = -1; dx <= 1 && !plant; dx++)
        {
            for (int dy = -1; dy <= 1 && !plant; dy++)
            {
                int targetX = x + dx;
                int targetY = y + dy;

                Robot *target = battlefield.getRobotAt(targetX, targetY);

                if (target && target != this)
                {
                    track_target.push_back(target);
                    tracker--;
                    logger << getName() << " track " << target->getName() << " at (" << targetX << "," << targetY << ")\n";
                    plant = true;
                }
            }
        }
        if (!plant)
        {
            logger << getName() << " no target can track\n";
        }
    }

    void act() override
    {
        think();
        look(*battlefield);
        fire(*battlefield);
        move(*battlefield);
    }

    void showTrackTarget()
    {
        if (track_target.empty())
        {
            logger << getName() << " didnt track any robot\n";
            return;
        }
        logger << getName() << " is tracking:\n";
        for (Robot *r : track_target)
        {
            logger << r->getName() << " at (" << r->getX() << "," << r->getY() << ")\n";
        }
    }
    int getTracker() const
    {
        return tracker;
    }
};


//******************************************
//QueenBot
//******************************************

class QueenBot : public GenericRobot{
    private:

    const std::vector <std::pair<int,int>>directions = {
        {0,1},
        {1,0},
        {0,-1},
        {-1,0},
        {1,1},
        {1,-1},
        {-1,1},
        {-1,-1}
    };

    public:
    QueenBot(const string& name, int x,int y)
    : Robot(name,x,y),
      GenericRobot(name,x,y) {}

      void fire(Battlefield &battlefield)override{
        int x = getX();
        int y = getY();
        bool fired = false;

        for (const auto& dir : directions){
            int dx = dir.first;
            int dy = dir.second;

            for (int dist = 1; ; dist++){
                int targetX = x+dx*dist;
                int targetY = y+dy*dist;
            
                if (targetX < 0 || targetY < 0 || targetX >= battlefield.getWidth() || targetY >= battlefield.getHeight()) {
                    break;
                }

            Robot* target = battlefield.getRobotAt(targetX,targetY);

            if (target && target != this){
                GenericRobot* gtarget = dynamic_cast<GenericRobot*>(target);
                if (gtarget->isHit()){
                    cout << getName()<< " fire at ("<<targetX<<","<< targetY<<")\n";

                    gtarget->takeDamage();
                    cout<<getName()<< " hit "<<gtarget->getName()<< endl;
                    fired = true;

                    static const std::vector<std::string> types = {"HideBot","JumpBot","ScoutBot","TrackBot"};
                    int t = rand() % types.size();
                    setPendingUpgrade(types[t]);
                    cout << getName() << " will upgrade in to "<< types[t] << "next turn"<< endl;
                }
            }
        }
      }


      if (!fired){
        cout<<"sadly "<<getName()<< " found no target in straight line\n";
      }
    }

};

//******************************************
//upgradesConflict (know each robot category)
//******************************************

bool upgradesConflict(const std::string& type1, const std::string& type2) {
    static const std::unordered_map<std::string, std::string> categoryMap = {
        {"HideBot", "move"}, {"JumpBot", "move"},
        {"LongShotBot", "fire"}, {"SemiAutoBot", "fire"}, {"ThirtyShotBot", "fire"}, 
        {"KnightBot", "fire"}, {"QueenBot", "fire"},
        {"ScoutBot", "see"}, {"TrackBot", "see"}
    };

    auto cat1 = categoryMap.find(type1);
    auto cat2 = categoryMap.find(type2);

    if (cat1 == categoryMap.end() || cat2 == categoryMap.end()) {
        throw std::runtime_error("unknow upgrade type: " + (cat1 == categoryMap.end() ? type1 : type2));
    }

    return cat1->second == cat2->second;
}

//******************************************
// LevelThreeRobot
//******************************************
class LevelThreeRobot : public GenericRobot {
private:
    std::string upgradeType1;
    std::string upgradeType2;

public:
    LevelThreeRobot(const std::string &name, int x, int y, const std::string& type1, const std::string& type2)
    : Robot(name,x,y),
      GenericRobot(name, x, y),
      upgradeType1(type1),
      upgradeType2(type2) {
        if (upgradesConflict(type1, type2)) {
            throw std::runtime_error("cannot upgrade same category");
        }
        upgrades.push_back(type1);
        upgrades.push_back(type2);
    }

    void move(Battlefield &battlefield) override {
       
        for (const auto& upgrade : upgrades) {
            if (upgrade == "HideBot") {
                HideBot temp(name, getX(), getY());
                temp.move(battlefield);
            } else if (upgrade == "JumpBot") {
                JumpBot temp(name, getX(), getY());
                temp.move(battlefield);
            }
        }
        GenericRobot::move(battlefield);
    }

    void fire(Battlefield &battlefield) override {
      
        for (const auto& upgrade : upgrades) {
            if (upgrade == "LongShotBot") {
                LongShotBot temp(name, getX(), getY());
                temp.fire(battlefield);
            } 
            else if (upgrade == "SemiAutoBot") {
                SemiAutoBot temp(name, getX(), getY());
                temp.fire(battlefield);
            }
            else if (upgrade == "ThirtyShotBot") {
                ThirtyShotBot temp(name, getX(), getY());
                temp.fire(battlefield);
            }
            else if (upgrade == "KnightBot") {
                KnightBot temp(name, getX(), getY());
                temp.fire(battlefield);
            }
            else if (upgrade == "QueenBot") {
                QueenBot temp(name, getX(), getY());
                temp.fire(battlefield);
            }
        
            
        }
        GenericRobot::fire(battlefield);
    }

    void look(Battlefield &battlefield) override {
      
        for (const auto& upgrade : upgrades) {
            if (upgrade == "ScoutBot") {
                ScoutBot temp(name, getX(), getY());
                temp.look(battlefield);
            } else if (upgrade == "TrackBot") {
                TrackBot temp(name, getX(), getY());
                temp.look(battlefield);
            }
        }
        GenericRobot::look(battlefield);
    }
};

//******************************************
// LevelFourRobot
//******************************************
class LevelFourRobot : public GenericRobot {
private:
    std::vector<std::string> upgrades;

public:
    LevelFourRobot(const std::string &name, int x, int y, 
                  const std::string& type1, const std::string& type2, const std::string& type3)
    : Robot(name,x,y),
      GenericRobot(name, x, y) {
        upgrades = {type1, type2, type3};
        
        if (upgradesConflict(type1, type2) || upgradesConflict(type1, type3) || upgradesConflict(type2, type3)) {
            throw std::runtime_error("cannot upgrade same category");
        }
    }

    void move(Battlefield &battlefield) override {
        for (const auto& upgrade : upgrades) {
            if (upgrade == "HideBot") {
                HideBot temp(name, getX(), getY());
                temp.move(battlefield);
            } else if (upgrade == "JumpBot") {
                JumpBot temp(name, getX(), getY());
                temp.move(battlefield);
            }
        }
        GenericRobot::move(battlefield);
    }

    void fire(Battlefield &battlefield) override {
      
        for (const auto& upgrade : upgrades) {
            if (upgrade == "LongShotBot") {
                LongShotBot temp(name, getX(), getY());
                temp.fire(battlefield);
            } 
            else if (upgrade == "SemiAutoBot") {
                SemiAutoBot temp(name, getX(), getY());
                temp.fire(battlefield);
            }
            else if (upgrade == "ThirtyShotBot") {
                ThirtyShotBot temp(name, getX(), getY());
                temp.fire(battlefield);
            }
            else if (upgrade == "KnightBot") {
                KnightBot temp(name, getX(), getY());
                temp.fire(battlefield);
            }
            else if (upgrade == "QueenBot") {
                QueenBot temp(name, getX(), getY());
                temp.fire(battlefield);
            }
        
            
        }
        GenericRobot::fire(battlefield);
    }


    void look(Battlefield &battlefield) override {
        for (const auto& upgrade : upgrades) {
            if (upgrade == "ScoutBot") {
                ScoutBot temp(name, getX(), getY());
                temp.look(battlefield);
            } else if (upgrade == "TrackBot") {
                TrackBot temp(name, getX(), getY());
                temp.look(battlefield);
            }
        }
        GenericRobot::look(battlefield);
    }
};

//******************************************
// simulationStep member function of Battlefield class (declared later to avoid issues with code not seeing each other when they need to)
//******************************************

void Battlefield::simulationStep()
{
    // Handle upgrades first
    for (size_t i = 0; i < listOfRobots.size(); ++i)
    {
        GenericRobot *gen = dynamic_cast<GenericRobot *>(listOfRobots[i]);
        if (gen && gen->PendingUpgrade())
        {
            string type = gen->getUpgradeType();
            Robot *upgraded = nullptr;
            if (type == "HideBot")
                upgraded = new HideBot(gen->getName(), gen->getX(), gen->getY());
            else if (type == "JumpBot")
                upgraded = new JumpBot(gen->getName(), gen->getX(), gen->getY());
            else if (type == "LongShotBot")
                upgraded = new LongShotBot(gen->getName(), gen->getX(), gen->getY());
            else if (type == "KnightBot")
                upgraded = new KnightBot(gen->getName(), gen->getX(), gen->getY());
            else if (type == "SemiAutoBot")
                upgraded = new SemiAutoBot(gen->getName(), gen->getX(), gen->getY());
            else if (type == "ThirtyShotBot")
                upgraded = new ThirtyShotBot(gen->getName(), gen->getX(), gen->getY());
            else if (type == "ScoutBot")
                upgraded = new ScoutBot(gen->getName(), gen->getX(), gen->getY());
            else if (type == "TrackBot")
                upgraded = new TrackBot(gen->getName(), gen->getX(), gen->getY());
            else if (type == "QueenBot")
                upgraded = new QueenBot(gen->getName(), gen->getX(), gen->getY());
            if (upgraded)
            {
                upgraded->setLives(gen->getLives());
                // Fix: clear the upgrade flag so the new bot doesn't try to upgrade again
                GenericRobot *upGen = dynamic_cast<GenericRobot *>(upgraded);
                if (upGen)
                    upGen->clearPendingUpgrade();
                removeRobotFromGrid(gen);
                placeRobot(upgraded, gen->getX(), gen->getY());
                delete gen;
                listOfRobots[i] = upgraded;
                logger << upgraded->getName() << " has upgraded to " << type << "!" << endl;
            }
        }
    }

    vector<Robot *> currentlyAliveRobots;

    for (Robot *robot : listOfRobots)
    {
        if (robot->getLives() > 0 && !robot->getIsDie())
        {
            currentlyAliveRobots.push_back(robot);
        }
    }

    for (Robot *robot : currentlyAliveRobots)
    {
        // Set battlefield context if robot is a GenericRobot
        if (auto gen = dynamic_cast<GenericRobot *>(robot))
        {
            gen->setBattlefield(this);
            gen->resetActionFlags(); // Reset action flags for the new round
        }
        logger << "----------------------------------------" << endl;
        logger << robot->getName() << "'s turn: " << endl;
        robot->act();
        logger << robot->getName() << " is done." << endl;
    }

    cleanupDestroyedRobots();
    logger << "----------------------------------------" << endl;
    respawnRobots();
    for (Robot *robot : listOfRobots)
    {
        robot->setIsDie(false);
    }
}

// COMPLETED: To get the number of alive robots
// **TO BE USED IN MAIN LOOP**
int Battlefield::getNumberOfAliveRobots()
{
    int num = 0;
    for (const Robot *robot : listOfRobots)
    {
        if (robot->getLives() > 0)
        {
            num++;
        }
    }
    return num;
}

// COMPLETED: To add new robot to battlefield during initialization
void Battlefield::addNewRobot(Robot *robot)
{
    listOfRobots.push_back(robot);
    if (respawnCounts.find(robot->getName()) == respawnCounts.end())
    {
        respawnCounts[robot->getName()] = 3;
    }
}

// COMPLETED: to check if the given coordinates are within the grid
bool Battlefield::isPositionWithinGrid(int x, int y) const
{
    return (x >= 0 && x < width) && (y >= 0 && y < height);
}

// COMPLETED: to get the Robot at given coordinates, if not found then return nullptr
Robot *Battlefield::getRobotAt(int x, int y) const
{
    if (!isPositionWithinGrid(x, y))
    {
        return nullptr;
    }
    return battlefieldGrid[y][x];
}

// COMPLETED: to check if the given coordinates is available
bool Battlefield::isPositionAvailable(int x, int y)
{
    if (getRobotAt(x, y) == nullptr)
    {
        return true;
    }
    else
    {
        return false;
    }
}

// COMPLETED: to put robot at specified coordinates
void Battlefield::placeRobot(Robot *robot, int x, int y)
{
    if (isPositionAvailable(x, y) && isPositionWithinGrid(x, y))
    {
        battlefieldGrid[y][x] = robot;
        robot->setPosition(x, y);
    }
}

// COMPLETED: to remove robot from grid position
void Battlefield::removeRobotFromGrid(Robot *robot)
{
    int x = robot->getX();
    int y = robot->getY();

    if (isPositionWithinGrid(x, y) && battlefieldGrid[y][x] == robot)
    {
        battlefieldGrid[y][x] = nullptr;
    }
}

// COMPLETED: to queue robot for reentry
void Battlefield::queueForReentry(Robot *robot)
{
    reentryQueue.push({robot->getName(), robot->getLives()});
}

// PARTIALLY COMPLETED: respawnRobots member function definition
void Battlefield::respawnRobots()
{
    if (!reentryQueue.empty())
    {
        auto respawnInfo = reentryQueue.front();
        std::string nameOfRobotToRespawn = respawnInfo.first;
        int livesLeft = respawnInfo.second;
        reentryQueue.pop();
        logger << "Reentering " << nameOfRobotToRespawn << std::endl;
        int randomX;
        int randomY;
        int attempts = 50;
        bool spotFound = false;
        while (attempts > 0)
        {
            randomX = rand() % width;
            randomY = rand() % height;
            if (isPositionWithinGrid(randomX, randomY) && isPositionAvailable(randomX, randomY))
            {
                spotFound = true;
                break;
            }
            attempts--;
        }
        if (spotFound)
        {
            Robot *newRobot = new GenericRobot(nameOfRobotToRespawn, randomX, randomY);
            newRobot->setLives(livesLeft); // Restore the lives before being hit
            placeRobot(newRobot, randomX, randomY);
            listOfRobots.push_back(newRobot);
            logger << nameOfRobotToRespawn << " reentered as GenericRobot at (" << randomX << ", " << randomY << ") with ";
            logger << livesLeft << " lives" <<" next turn."<< std::endl;
        }
        else
        {
            logger << "No available spot for reentry for " << nameOfRobotToRespawn << ". Will try again next turn." << std::endl;
            // If no spot, requeue for next turn
            reentryQueue.push({nameOfRobotToRespawn, livesLeft});
        }
    }
}

//  PARTIALLY COMPLETED: cleanupDestroyedRobots member function
void Battlefield::cleanupDestroyedRobots()
{
    logger << "----------------------------------------" << endl;
    logger << "Battlefield::cleanupDestroyedRobots()" << endl;
    auto iterator = listOfRobots.begin();
    while (iterator != listOfRobots.end())
    {
        Robot *robot = *iterator;
        if (robot != nullptr && (robot->getLives() <= 0 || robot->getIsDie() || robot->getIsHurt()))
        {
            removeRobotFromGrid(robot);
            logger << robot->getName() << " has been removed from the battlefield." << endl;
            if (robot->getIsHurt())
                queueForReentry(robot);
            delete robot;
            iterator = listOfRobots.erase(iterator);
            continue;
        }
        ++iterator;
    }
}

//******************************************
// To display the battlefield state
//******************************************
void Battlefield::displayBattlefield()
{
    // Print column numbers header
    logger << "   ";
    for (int x = 0; x < width; x++)
    {
        logger << x % 10 << " "; // Single digit for each column
    }
    logger << endl;

    // Print top border
    logger << "  +";
    for (int x = 0; x < width; x++)
    {
        logger << "--";
    }
    logger << "+" << endl;

    // Print each row
    for (int y = 0; y < height; y++)
    {
        // Print row number
        logger << y % 10 << " |"; // Single digit for each row

        // Print cells
        for (int x = 0; x < width; x++)
        {
            Robot *robot = battlefieldGrid[y][x];
            if (robot != nullptr)
            {
                if (robot->getLives() <= 0)
                {
                    logger << "X "; // Dead robot
                }
                else if (robot->isHidden())
                {
                    logger << "H "; // Hidden robot
                }
                else
                {
                    // Show first letter of robot's name
                    logger << robot->getName()[0] << " ";
                }
            }
            else
            {
                logger << ". "; // Empty space
            }
        }
        logger << "| " << y % 10 << endl; // Row number on right side
    }

    // Print bottom border
    logger << "  +";
    for (int x = 0; x < width; x++)
    {
        logger << "--";
    }
    logger << "+" << endl;

    // Print column numbers footer
    logger << "   ";
    for (int x = 0; x < width; x++)
    {
        logger << x % 10 << " ";
    }
    logger << endl
         << endl;

    // Print legend
    logger << "LEGEND:" << endl;
    logger << "  . - Empty space" << endl;
    logger << "  X - Destroyed robot" << endl;
    logger << "  H - Hidden robot" << endl;
    logger << "  [Letter] - First letter of robot's name" << endl
         << endl;
}

//******************************************
// Destructor
//******************************************

Battlefield::~Battlefield()
{
    for (Robot *robot : listOfRobots)
    {
        delete robot;
    }
    listOfRobots.clear();
    logger << "Battlefield: Destructor" << endl;
}

//******************************************
// Function implementations
//******************************************

void parseInputFile(const string &line, Battlefield &battlefield)
{
    vector<string> tokens;
    istringstream iss(line);
    string token;

    while (iss >> token)
    {
        tokens.push_back(token);
    }

    if (tokens.empty())
    {
        return; // Skip empty lines
    }

    if (tokens[0] == "M" && tokens.size() >= 6)
    {
        int height = stoi(tokens[4]);
        int width = stoi(tokens[5]);
        battlefield.setDimensions(height, width);
    }
    else if (tokens[0] == "steps:" && tokens.size() >= 2)
    {
        int steps = stoi(tokens[1]);
        battlefield.setSteps(steps);
    }
    else if (tokens[0] == "robots:" && tokens.size() >= 2)
    {
        int numRobots = stoi(tokens[1]);
        battlefield.setNumberOfRobots(numRobots);
    }
    else if (tokens[0] == "GenericRobot" && tokens.size() >= 4)
    {

        string robotName = tokens[1];
        int robotXCoordinates;
        int robotYCoordinates;

        if (tokens[2] == "random" && tokens[3] == "random")
        {
            robotXCoordinates = rand() % battlefield.getWidth();
            robotYCoordinates = rand() % battlefield.getHeight();
        }
        else
        {
            robotXCoordinates = stoi(tokens[2]);
            robotYCoordinates = stoi(tokens[3]);
        }

        Robot *newRobot = new GenericRobot(robotName, robotXCoordinates, robotYCoordinates);
        battlefield.addNewRobot(newRobot);
        battlefield.placeRobot(newRobot, robotXCoordinates, robotYCoordinates);
    }
    else if (tokens[0] == "HideBot" && tokens.size() >= 4)
    {
        string robotName = tokens[1];
        int robotXCoordinates;
        int robotYCoordinates;

        if (tokens[2] == "random" && tokens[3] == "random")
        {
            robotXCoordinates = rand() % battlefield.getWidth();
            robotYCoordinates = rand() % battlefield.getHeight();
        }
        else
        {
            robotXCoordinates = stoi(tokens[2]);
            robotYCoordinates = stoi(tokens[3]);
        }

        Robot *newRobot = new HideBot(robotName, robotXCoordinates, robotYCoordinates);
        battlefield.addNewRobot(newRobot);
        battlefield.placeRobot(newRobot, robotXCoordinates, robotYCoordinates);
    }
    else if (tokens[0] == "JumpBot" && tokens.size() >= 4)
    {
        string robotName = tokens[1];
        int robotXCoordinates;
        int robotYCoordinates;

        if (tokens[2] == "random" && tokens[3] == "random")
        {
            robotXCoordinates = rand() % battlefield.getWidth();
            robotYCoordinates = rand() % battlefield.getHeight();
        }
        else
        {
            robotXCoordinates = stoi(tokens[2]);
            robotYCoordinates = stoi(tokens[3]);
        }
        Robot *newRobot = new JumpBot(robotName, robotXCoordinates, robotYCoordinates);
        battlefield.addNewRobot(newRobot);
        battlefield.placeRobot(newRobot, robotXCoordinates, robotYCoordinates);
    }
    else if (tokens[0] == "LongShotBot" && tokens.size() >= 4)
    {
        string robotName = tokens[1];
        int robotXCoordinates;
        int robotYCoordinates;
        if (tokens[2] == "random" && tokens[3] == "random")
        {
            robotXCoordinates = rand() % battlefield.getWidth();
            robotYCoordinates = rand() % battlefield.getHeight();
        }
        else
        {
            robotXCoordinates = stoi(tokens[2]);
            robotYCoordinates = stoi(tokens[3]);
        }

        Robot *newRobot = new LongShotBot(robotName, robotXCoordinates, robotYCoordinates);
        battlefield.addNewRobot(newRobot);
        battlefield.placeRobot(newRobot, robotXCoordinates, robotYCoordinates);
    }
    else if (tokens[0] == "SemiAutoBot" && tokens.size() >= 4)
    {
        string robotName = tokens[1];
        int robotXCoordinates;
        int robotYCoordinates;

        if (tokens[2] == "random" && tokens[3] == "random")
        {
            robotXCoordinates = rand() % battlefield.getWidth();
            robotYCoordinates = rand() % battlefield.getHeight();
        }
        else
        {
            robotXCoordinates = stoi(tokens[2]);
            robotYCoordinates = stoi(tokens[3]);
        }

        Robot *newRobot = new SemiAutoBot(robotName, robotXCoordinates, robotYCoordinates);
        battlefield.addNewRobot(newRobot);
        battlefield.placeRobot(newRobot, robotXCoordinates, robotYCoordinates);
    }
    else if (tokens[0] == "ThirtyShotBot" && tokens.size() >= 4)
    {
        string robotName = tokens[1];
        int robotXCoordinates;
        int robotYCoordinates;

        if (tokens[2] == "random" && tokens[3] == "random")
        {
            robotXCoordinates = rand() % battlefield.getWidth();
            robotYCoordinates = rand() % battlefield.getHeight();
        }
        else
        {
            robotXCoordinates = stoi(tokens[2]);
            robotYCoordinates = stoi(tokens[3]);
        }

        Robot *newRobot = new ThirtyShotBot(robotName, robotXCoordinates, robotYCoordinates);
        battlefield.addNewRobot(newRobot);
        battlefield.placeRobot(newRobot, robotXCoordinates, robotYCoordinates);
    }
    else if (tokens[0] == "ScoutBot" && tokens.size() >= 4)
    {
        string robotName = tokens[1];
        int robotXCoordinates;
        int robotYCoordinates;

        if (tokens[2] == "random" && tokens[3] == "random")
        {
            robotXCoordinates = rand() % battlefield.getWidth();
            robotYCoordinates = rand() % battlefield.getHeight();
        }
        else
        {
            robotXCoordinates = stoi(tokens[2]);
            robotYCoordinates = stoi(tokens[3]);
        }

        Robot *newRobot = new ScoutBot(robotName, robotXCoordinates, robotYCoordinates);
        battlefield.addNewRobot(newRobot);
        battlefield.placeRobot(newRobot, robotXCoordinates, robotYCoordinates);
    }
    else if (tokens[0] == "TrackBot" && tokens.size() >= 4)
    {
        string robotName = tokens[1];
        int robotXCoordinates;
        int robotYCoordinates;

        if (tokens[2] == "random" && tokens[3] == "random")
        {
            robotXCoordinates = rand() % battlefield.getWidth();
            robotYCoordinates = rand() % battlefield.getHeight();
        }
        else
        {
            robotXCoordinates = stoi(tokens[2]);
            robotYCoordinates = stoi(tokens[3]);
        }

        Robot *newRobot = new TrackBot(robotName, robotXCoordinates, robotYCoordinates);
        battlefield.addNewRobot(newRobot);
        battlefield.placeRobot(newRobot, robotXCoordinates, robotYCoordinates);
    }
    else if (tokens[0] == "KnightBot" && tokens.size() >= 4)
    {
        string robotName = tokens[1];
        int robotXCoordinates;
        int robotYCoordinates;

        if (tokens[2] == "random" && tokens[3] == "random")
        {
            robotXCoordinates = rand() % battlefield.getWidth();
            robotYCoordinates = rand() % battlefield.getHeight();
        }
        else
        {
            robotXCoordinates = stoi(tokens[2]);
            robotYCoordinates = stoi(tokens[3]);
        }

        Robot *newRobot = new KnightBot(robotName, robotXCoordinates, robotYCoordinates);
        battlefield.addNewRobot(newRobot);
        battlefield.placeRobot(newRobot, robotXCoordinates, robotYCoordinates);
    }
    else if (tokens[0] == "QueenBot" && tokens.size() >= 4)
    {
        string robotName = tokens[1];
        int robotXCoordinates;
        int robotYCoordinates;

        if (tokens[2] == "random" && tokens[3] == "random")
        {
            robotXCoordinates = rand() % battlefield.getWidth();
            robotYCoordinates = rand() % battlefield.getHeight();
        }
        else
        {
            robotXCoordinates = stoi(tokens[2]);
            robotYCoordinates = stoi(tokens[3]);
        }

        Robot *newRobot = new QueenBot(robotName, robotXCoordinates, robotYCoordinates);
        battlefield.addNewRobot(newRobot);
        battlefield.placeRobot(newRobot, robotXCoordinates, robotYCoordinates);
    }
}

//*****************************************************************************************
// Modified to take Battlefield by reference to avoid copying/double-free memory error
//*****************************************************************************************

void readInputFile(Battlefield &battlefield, const string &filename = "inputFile.txt")
{
    ifstream inputFile(filename);

    if (!inputFile.is_open())
    {
        cerr << "Error opening input file." << endl;
        return;
    }

    string line;
    while (getline(inputFile, line))
    {
        if (!line.empty())
        {
            parseInputFile(line, battlefield);
        }
    }

    inputFile.close();
}

int main()
{
    // Seed the random number generator once
    srand(static_cast<unsigned>(time(0)));

    Battlefield battlefield;
    logger << "READING INPUT FILE" << endl;
    readInputFile(battlefield); // This sets dimensions and initializes grid

    logger << "TESTING BATTLEFIELD CLASS" << endl;
    logger << "Battlefield Dimensions: ";
    battlefield.printDimensions();
    logger << endl;

    logger << "Battlefield steps: ";
    battlefield.printSteps();
    logger << endl;

    logger << "Battlefield number of robots: ";
    battlefield.printNumberOfRobots();
    logger << endl;

    logger << "Initial Robots on battlefield: " << endl;

    for (Robot *robot : battlefield.getListOfRobots())
    {
        string name = robot->getName();
        int x = robot->getX();
        int y = robot->getY();
        int lives = robot->getLives();
        logger << "Robot Name: " << name << ", Coords: (" << x << "," << y << "), Lives: " << lives << endl;
    }
    logger << endl;

    logger << "Initial Battlefield State:" << endl;
    battlefield.displayBattlefield(); // Display the grid
    logger << endl;

    // --- Simulation Loop ---
    int currentStep = 0;
    int maxSteps = battlefield.getSteps();

    logger << "--- Starting Simulation ---" << endl;

    // Loop while max steps not reached AND there's more than one robot alive
    while (currentStep < maxSteps && battlefield.getNumberOfAliveRobots() > 1)
    {
        logger << "\n--- Simulation Step " << currentStep + 1 << " ---" << endl;

        logger << "Robot Status before Step" << currentStep + 1 << ":" << endl;
        for (Robot *robot : battlefield.getListOfRobots())
        {
            string type;
            GenericRobot* gen = dynamic_cast<GenericRobot*>(robot);
        
        if (gen) {
            if (gen->PendingUpgrade()) {
                type = gen->getUpgradeType();
            } else {
                if (dynamic_cast<HideBot*>(robot)) type = "HideBot";
                else if (dynamic_cast<JumpBot*>(robot)) type = "JumpBot";
                else if (dynamic_cast<LongShotBot*>(robot)) type = "LongShotBot";
                else if (dynamic_cast<KnightBot*>(robot)) type = "KnightBot";
                else if (dynamic_cast<SemiAutoBot*>(robot)) type = "SemiAutoBot";
                else if (dynamic_cast<ThirtyShotBot*>(robot)) type = "ThirtyShotBot";
                else if (dynamic_cast<ScoutBot*>(robot)) type = "ScoutBot";
                else if (dynamic_cast<TrackBot*>(robot)) type = "TrackBot";
                else if (dynamic_cast<QueenBot*>(robot)) type = "QueenBot";
                else type = "GenericRobot";
            }
        } else {
            type = "Robot";
        }
            //TO DO :able to display current robot type
            logger << "  Type: " << type
                 << ", Name: " << robot->getName()
                 << ", Coords: (" << robot->getX() << "," << robot->getY() << ")"
                 << ", Life: " << robot->getLives() << endl;
        }

        battlefield.simulationStep(); // Executes turns, cleans up, respawns

        logger << "\nBattlefield State after Step " << currentStep + 1 << ":" << endl;
        battlefield.displayBattlefield(); // Display the updated grid

        currentStep++;
    }

    // --- End of Simulation ---
    logger << "\n--- Simulation Ended ---" << endl;

    size_t remainingRobots = battlefield.getNumberOfAliveRobots();

    if (remainingRobots <= 1)
    {
        if (remainingRobots == 0)
        {
            logger << "Result: All robots destroyed!" << endl;
        }
        else
        {
            Robot *lastRobot = nullptr;
            const vector<Robot *> &finalRobotList = battlefield.getListOfRobots();
            for (Robot *robot : finalRobotList)
            {
                if (robot->getLives() > 0)
                {
                    lastRobot = robot;
                    break;
                }
            }
            if (lastRobot)
            {
                logger << "Result: " << lastRobot->getName() << " is the last one standing!" << endl;
            }
            else
            {
                logger << "Result: Simulation ended with one robot expected, but couldn't find the last one." << endl;
            }
        }
    }
    else
    {
        logger << "Result: Maximum steps reached (" << maxSteps << " steps)." << endl;
        logger << "Remaining robots: " << remainingRobots << endl;
    }

    return 0; // Battlefield object goes out of scope here, destructor is called.
}