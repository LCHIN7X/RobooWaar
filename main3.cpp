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

// TO DO: SemiAutoBot, LongShotBot, ThirtyShotBot, KnightBot, QueenBot, VampireBot have fire isHurt problem, will fix soon
//  Forward declarations
class Robot;

//**********************************************************
// Logger class that logs all activity (cout and output to file)
//**********************************************************
class Logger
{
private:
    ofstream outputFile;
    string fileName = "outputFile.txt";

public:
    // Constructor
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

// Global Logger instance
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
    queue<pair<string, int>> reentryQueue; // Queue for reentry

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
    bool isDie = false;  // true if robot is out of the game (no lives or no ammo)
    bool isHurt = false; // true if robot is hit this turn and should be requeued
    virtual bool isHit() = 0;
    bool getEnemyDetectedNearby() const;

public:
    Robot(string name, int x, int y)
        : name(name), positionX(x), positionY(y), lives(3), hidden(false) {}

    virtual ~Robot() = default;

    virtual void think() = 0;
    virtual void act() = 0;
    virtual void move() = 0;
    virtual void fire() = 0;
    virtual void look(int X, int Y) = 0;

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
    virtual void move() = 0;

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
    virtual void fire() = 0;
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
    vector<Robot *> detectedTargets;
    bool enemyDetectedNearby;

public:
    SeeingRobot(const string &name, int x, int y, int range)
        : Robot(name, x, y), visionRange(range) {}

    virtual ~SeeingRobot() = default;
    virtual void look(int X, int Y) = 0;

    const vector<Robot *> &getDetectedTargets() const { return detectedTargets; }

    void setDetectedTargets(const vector<Robot *> &targets) { detectedTargets = targets; }
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
    Battlefield *battlefield = nullptr;          // Pointer to current battlefield
    bool pendingUpgrade = false;
    string upgradeType = "";
    bool enemyDetectedNearby = false; // Flag for detecting nearby enemies
    vector<Robot *> detectedTargets;  // <-- Add this line
    vector<string> upgrades;
    vector<pair<int, int>> availableSpaces;

    // Action flags for per-round limitation
    bool hasLooked = false;
    bool hasMoved = false;
    bool hasThought = false;
    bool hasFired = false;

public:
    GenericRobot(const string &name, int x, int y);
    ~GenericRobot() override;
    void setBattlefield(Battlefield *bf);
    void think() override;
    void act() override;
    void move() override;
    void fire() override;
    void look(int X, int Y) override;
    bool canUpgrade(int area) const;
    void setUpgraded(int area);
    bool isHit() override;
    void setPendingUpgrade(const string &type);
    bool PendingUpgrade() const;
    string getUpgradeType() const;
    void clearPendingUpgrade();
    bool getEnemyDetectedNearby() const;

    // Reset all action flags (call at start/end of each round)
    void resetActionFlags()
    {
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

void GenericRobot::setBattlefield(Battlefield *bf) { battlefield = bf; }

void GenericRobot::think()
{
    if (hasThought)
        return;
    hasThought = true;
    logger << ">> " << name << " is thinking...\n";
    if (getEnemyDetectedNearby())
    {
        fire();
        move();
    }
    else
    {
        move();
        fire();
    }
}
void GenericRobot::act()
{
    if (isDie || isHurt || getLives() <= 0)
        return; // If robot is dead or hurt, skip action
    if (!battlefield)
    {
        logger << name << " has no battlefield context!" << endl;
        return;
    }
    look(0, 0);
    think();
    detectedTargets.clear();
}

void GenericRobot::move()
{
    if (hasMoved)
        return;
    hasMoved = true;
    logger << ">> " << name << " is moving...\n";
    if (!battlefield)
    {
        logger << name << " has no battlefield context!" << endl;
        return;
    }
    // If availableSpaces (from look) is not empty, pick a random one
    if (!availableSpaces.empty())
    {
        static random_device rd;
        static mt19937 g(rd());
        uniform_int_distribution<> dist(0, availableSpaces.size() - 1);
        auto [newX, newY] = availableSpaces[dist(g)];
        battlefield->removeRobotFromGrid(this);
        battlefield->placeRobot(this, newX, newY);
        incrementMoveCount();
        logger << name << " moved to (" << newX << ", " << newY << ")\n";
        return;
    }

    logger << name << " could not move (no available adjacent cell).\n";
}

void GenericRobot::fire()
{
    if (hasFired)
        return;
    hasFired = true;
    if (hasAmmo())
    {
        // Filter detectedTargets to only include robots that are hittable (isHit() == true)
        vector<Robot *> validTargets;
        for (Robot *r : detectedTargets)
        {
            if (r && r != this)
            {
                GenericRobot *gtarget = dynamic_cast<GenericRobot *>(r);
                if (gtarget && gtarget->isHit())
                {
                    validTargets.push_back(r);
                }
            }
        }
        if (!validTargets.empty())
        {
            int idx = rand() % validTargets.size();
            Robot *target = validTargets[idx];
            int targetX = target->getX();
            int targetY = target->getY();
            logger << ">> " << name << " fires at (" << targetX << ", " << targetY << ")" << endl;
            useAmmo();
            if (target->isHidden())
            {
                logger << target->getName() << " is hidden, attack miss." << endl;
            }
            else if (hitProbability())
            {
                logger << "Hit! (" << target->getName() << ") be killed" << endl;
                target->takeDamage();
                // Alternate upgrade types
                //  If the robot's class name starts with "Hide" (for HideBot and all Hide* hybrids)
                // if (typeid(*this).name() && std::string(typeid(*this).name()).find("Hide") != std::string::npos) {
                // types = {"HideLongShotBot","HideSemiAutoBot","HideThirtyShotBot","HideKnightBot","HideQueenBot","HideVampireBot","HideScoutBot","HideTrackBot"};
                //} else {
                // types = {"HideBot", "JumpBot", "LongShotBot", "SemiAutoBot", "ThirtyShotBot", "ScoutBot", "TrackBot", "KnightBot", "QueenBot", "VampireBot"};
                //}
                static const vector<string> types = {"HideBot", "JumpBot", "LongShotBot", "SemiAutoBot", "ThirtyShotBot", "ScoutBot", "TrackBot", "KnightBot", "QueenBot", "VampireBot"};
                int t = rand() % types.size();
                setPendingUpgrade(types[t]);
                logger << name << " will upgrade into " << types[t] << " next turn!" << endl;
            }
            else
            {
                logger << "Missed!" << endl;
            }
        }
        else
        {
            useAmmo();
            logger << ">> " << name << " fires." << endl;
            logger << "However no robots within shooting range ." << endl;
        }
    }
    else
    {
        logger << name << " has no ammo left. It will self destroy!" << endl;
        lives = 0;
        isDie = true;
    }
    detectedTargets.clear();
}

void GenericRobot::look(int X, int Y)
{
    if (!battlefield)
    {
        logger << name << " has no battlefield context!" << endl;
        return;
    }
    enemyDetectedNearby = false;
    availableSpaces.clear();
    detectedTargets.clear();

    int centerX = getX() + X;
    int centerY = getY() + Y;

    // Scan 3x3 area centered on robot's current position
    for (int dx = -1; dx <= 1; ++dx)
    {
        for (int dy = -1; dy <= 1; ++dy)
        {
            int lookX = centerX + dx;
            int lookY = centerY + dy;
            logger << "Revealing (" << lookX << ", " << lookY << "): ";
            if (!battlefield->isPositionWithinGrid(lookX, lookY))
            {
                logger << "Out of bounds" << endl;
                continue;
            }
            Robot *occupant = battlefield->getRobotAt(lookX, lookY);
            if (lookX == getX() && lookY == getY())
            {
                logger << "Current position" << endl;
                continue;
            }
            if (occupant == nullptr)
            {
                logger << "Empty space" << endl;
                availableSpaces.emplace_back(lookX, lookY);
            }
            else if (occupant != this)
            {
                logger << "Enemy " << occupant->getName() << endl;
                enemyDetectedNearby = true;
                if (std::find(detectedTargets.begin(), detectedTargets.end(), occupant) == detectedTargets.end())
                {
                    detectedTargets.push_back(occupant);
                }
            }
            else
            {
                logger << "Dead robot" << endl;
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

void GenericRobot::setPendingUpgrade(const string &type)
{
    pendingUpgrade = true;
    upgradeType = type;
}
bool GenericRobot::PendingUpgrade() const { return pendingUpgrade; }
string GenericRobot::getUpgradeType() const { return upgradeType; }
void GenericRobot::clearPendingUpgrade()
{
    pendingUpgrade = false;
    upgradeType = "";
}
bool GenericRobot::getEnemyDetectedNearby() const { return enemyDetectedNearby; }

//******************************************
// HideBot
//******************************************

class HideBot : public virtual GenericRobot
{

private:
    int hide_count = 0;
    bool isHidden = false;

public:
    HideBot(const string &name, int x, int y)
        : Robot(name, x, y),
          GenericRobot(name, x, y) {}

    void move() override
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
        logger << "HideBot is thinking..." << endl;
        // TO DO : the logic will be implemented later
        look(0, 0);
        fire();
        move();
    }

    void fire() override
    {
        if (hasFired)
            return;
        hasFired = true;
        if (hasAmmo())
        {
            // Filter detectedTargets to only include robots that are hittable (isHit() == true)
            vector<Robot *> validTargets;
            for (Robot *r : detectedTargets)
            {
                if (r && r != this)
                {
                    GenericRobot *gtarget = dynamic_cast<GenericRobot *>(r);
                    if (gtarget && gtarget->isHit())
                    {
                        validTargets.push_back(r);
                    }
                }
            }
            if (!validTargets.empty())
            {
                int idx = rand() % validTargets.size();
                Robot *target = validTargets[idx];
                int targetX = target->getX();
                int targetY = target->getY();
                logger << ">> " << name << " fires at (" << targetX << ", " << targetY << ")" << endl;
                useAmmo();
                if (target->isHidden())
                {
                    logger << target->getName() << " is hidden, attack miss." << endl;
                }
                else if (hitProbability())
                {
                    logger << "Hit! (" << target->getName() << ") be killed" << endl;
                    target->takeDamage();
                    static const vector<string> types = {"HideLongShotBot", "HideSemiAutoBot", "HideThirtyShotBot", "HideKnightBot", "HideQueenBot", "HideVampireBot", "HideScoutBot", "HideTrackBot"};
                    int t = rand() % types.size();
                    setPendingUpgrade(types[t]);
                    logger << name << " will upgrade into " << types[t] << " next turn!" << endl;
                }
                else
                {
                    logger << "Missed!" << endl;
                }
            }
            else
            {
                useAmmo();
                logger << ">> " << name << " fires." << endl;
                logger << "However no robots within shooting range ." << endl;
            }
        }
        else
        {
            logger << name << " has no ammo left. It will self destroy!" << endl;
            lives = 0;
            isDie = true;
        }
        detectedTargets.clear();
    }

    bool isHit() override
    {
        return !isHidden;
    }
};

//******************************************
// Jumpbot
//******************************************
class JumpBot : public virtual GenericRobot
{
private:
    int jump_count = 0;

public:
    JumpBot(const string &name, int x, int y)
        : Robot(name, x, y),
          GenericRobot(name, x, y) {}

    void move() override
    {
        if (jump_count < 3 && rand() % 2 == 0)
        {
            int jumpx, jumpy;
            bool positionFound = false;
            int attempts = 0;
            const int maxAttempt = 10;

            while (!positionFound && attempts < maxAttempt)
            {
                attempts++;
                jumpx = rand() % battlefield->getWidth();
                jumpy = rand() % battlefield->getHeight();

                if (!battlefield->getRobotAt(jumpx, jumpy))
                {
                    positionFound = true;
                }
            }

            if (positionFound)
            {
                jump_count++;
                battlefield->removeRobotFromGrid(this);
                battlefield->placeRobot(this, jumpx, jumpy);
                setPosition(jumpx, jumpy);
                logger << getName() << " jump to (" << jumpx << "," << jumpy << "), (" << jump_count << "/3)\n";
            }
            else
            {
                logger << getName() << " could not find empty position to jump\n";
            }
        }
        else
        {
            if (jump_count >= 3)
            {
                logger << getName() << " cannot jump already \n";
            }
            else
            {
                logger << getName() << " did not jump this turn, keep moving\n";
            }
        }
    }

    void act() override
    {
        logger << "JumpBot is thinking..." << endl;
        // TO DO : the logic will be implemented later
        look(0, 0);
        fire();
        move();
    }

    void fire() override
    {
        if (hasFired)
            return;
        hasFired = true;
        if (hasAmmo())
        {
            // Filter detectedTargets to only include robots that are hittable (isHit() == true)
            vector<Robot *> validTargets;
            for (Robot *r : detectedTargets)
            {
                if (r && r != this)
                {
                    GenericRobot *gtarget = dynamic_cast<GenericRobot *>(r);
                    if (gtarget && gtarget->isHit())
                    {
                        validTargets.push_back(r);
                    }
                }
            }
            if (!validTargets.empty())
            {
                int idx = rand() % validTargets.size();
                Robot *target = validTargets[idx];
                int targetX = target->getX();
                int targetY = target->getY();
                logger << ">> " << name << " fires at (" << targetX << ", " << targetY << ")" << endl;
                useAmmo();
                if (target->isHidden())
                {
                    logger << target->getName() << " is hidden, attack miss." << endl;
                }
                else if (hitProbability())
                {
                    logger << "Hit! (" << target->getName() << ") be killed" << endl;
                    target->takeDamage();
                    static const vector<string> types = {"JumpLongShotBot", "JumpSemiAutoBot", "JumpThirtyShotBot", "JumpKnightBot", "JumpQueenBot", "JumpVampireBot", "JumpScoutBot", "JumpTrackBot"};
                    int t = rand() % types.size();
                    setPendingUpgrade(types[t]);
                    logger << name << " will upgrade into " << types[t] << " next turn!" << endl;
                }
                else
                {
                    logger << "Missed!" << endl;
                }
            }
            else
            {
                useAmmo();
                logger << ">> " << name << " fires." << endl;
                logger << "However no robots within shooting range ." << endl;
            }
        }
        else
        {
            logger << name << " has no ammo left. It will self destroy!" << endl;
            lives = 0;
            isDie = true;
        }
        detectedTargets.clear();
    }

    int getJumpCount() const
    {
        return jump_count;
    }
};

//******************************************
// LongShotBot
//******************************************

class LongShotBot : public virtual GenericRobot
{
private:
    int fire_count = 0;
    const vector<string> upgradeTypes = {"HideLongShotBot", "JumpLongShotBot", "LongShotScoutBot", "LongShotTrackBot"};

public:
    LongShotBot(const string &name, int x, int y)
        : Robot(name, x, y),
          GenericRobot(name, x, y) {}

    void fire() override
    {
        if (hasFired)
            return;
        hasFired = true;

        if (!hasAmmo())
        {
            logger << name << " has no ammo left. It will self destroy!" << endl;
            lives = 0;
            isDie = true;
            return;
        }

        bool fired = false;
        int x = getX();
        int y = getY();

        useAmmo();

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

                Robot *target = battlefield->getRobotAt(targetX, targetY);

                if (target && target != this)
                {
                    GenericRobot *gtarget = dynamic_cast<GenericRobot *>(target);
                    logger << ">> " << getName() << "LongShot firesssss at (" << targetX << "," << targetY << ")" << endl;

                    if (gtarget->isHidden())
                    {
                        logger << gtarget->getName() << " is hidden, attack miss." << endl;
                        fired = true;
                    }
                    else if (hitProbability())
                    {
                        gtarget->takeDamage();
                        logger << "Hit! (" << gtarget->getName() << ") be killed" << endl;
                        fire_count++;
                        fired = true;
                        int t = rand() % upgradeTypes.size();
                        string newType = upgradeTypes[t];
                        setPendingUpgrade(newType);
                        logger << getName() << " will upgrade into " << newType << " next turn" << endl;
                    }
                    else
                    {
                        logger << "Missed!" << endl;
                        fired = true;
                    }
                    break;
                }
            }
            if (fired)
                break;
        }

        if (!fired)
        {
            logger << ">> " << getName() << " fires." << endl;
            logger << "no robot there " << endl;
        }
    }
};

//******************************************
// SemiAutoBot
//******************************************

class SemiAutoBot : public virtual GenericRobot
{
private:
    int fire_count = 0;
    const vector<string> upgradeTypes = {"HideSemiAutoBot", "JumpSemiAutoBot", "SemiAutoScoutBot", "SemiAutoTrackBot"};

public:
    SemiAutoBot(const string &name, int x, int y)
        : Robot(name, x, y),
          GenericRobot(name, x, y) {}

    void fire() override
    {
        if (hasFired)
            return;
        hasFired = true;

        if (!hasAmmo())
        {
            logger << name << " has no ammo left. It will self destroy!" << endl;
            lives = 0;
            isDie = true;
            return;
        }

        int x = getX();
        int y = getY();

        Robot *target = nullptr;
        for (Robot *r : battlefield->getListOfRobots())
        {
            if (r != nullptr && r != this && r->getLives() > 0)
            {
                target = r;
                break;
            }
        }

        if (!target)
        {
            useAmmo();
            logger << ">> " << getName() << "SemiAuto fires." << endl;
            logger << "However no robots available to target." << endl;
            return;
        }

        GenericRobot *gtarget = dynamic_cast<GenericRobot *>(target);
        useAmmo();

        logger << ">> " << getName() << "SemiAuto fires 3 consecutive shots at ("
               << gtarget->getX() << "," << gtarget->getY() << ")" << endl;

        bool hitSuccessful = false;
        for (int i = 0; i < 3; i++)
        {
            if (gtarget->isHidden())
            {
                logger << "Shot " << (i + 1) << ": " << gtarget->getName()
                       << " is hidden, attack missed." << endl;
                continue;
            }

            if (hitProbability())
            {
                logger << "Shot " << (i + 1) << " hit " << gtarget->getName() << "!" << endl;
                gtarget->takeDamage();
                fire_count++;
                hitSuccessful = true;
            }
            else
            {
                logger << "Shot " << (i + 1) << " missed!" << endl;
            }
        }

        if (hitSuccessful)
        {
            int t = rand() % upgradeTypes.size();
            string newType = upgradeTypes[t];
            setPendingUpgrade(newType);
            logger << getName() << " will upgrade into " << newType << " next turn!" << endl;
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
class ThirtyShotBot : public virtual GenericRobot
{
private:
    int shell_count;
    const vector<string> upgradeTypes = {"HideThirtyShotBot", "JumpThirtyShotBot", "ThirtyShotScoutBot", "ThirtyShotTrackBot"};

public:
    ThirtyShotBot(const string &name, int x, int y)
        : Robot(name, x, y),
          GenericRobot(name, x, y),
          shell_count(30)
    {
        logger << name << " got 30 shells replace current shells \n";
    }

    void fire() override
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
                if (dx == 0 && dy == 0)
                    continue; // Do not fire at self
                int targetX = x + dx;
                int targetY = y + dy;

                Robot *target = battlefield->getRobotAt(targetX, targetY);
                if (target && target != this)
                {
                    GenericRobot *gtarget = dynamic_cast<GenericRobot *>(target);
                    if (gtarget && gtarget->isHit())
                    {
                        if (hitProbability())
                        {
                            gtarget->takeDamage();
                            shell_count--;
                            logger << getName() << "ThirtyShot fire at (" << targetX << ", " << targetY << "), shell left: " << shell_count << "\n";
                            hitSuccessful = true;
                            logger << "Successful hit on " << gtarget->getName() << "!\n";
                            int t = rand() % upgradeTypes.size();
                            string newType = upgradeTypes[t];
                            setPendingUpgrade(newType);
                            logger << getName() << " will upgrade into " << newType << " next turn!\n";
                        }
                        else
                        {
                            shell_count--;
                            logger << getName() << " fire at (" << targetX << ", " << targetY << "), shell left: " << shell_count << "\n";
                            logger << "Missed!\n";
                        }
                        fired = true;
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

class KnightBot : public virtual GenericRobot
{
private:
    int fire_count = 0;
    const vector<string> upgradeTypes = {"HideKnightBot", "JumpKnightBot", "KnightScoutBot", "KnightTrackBot"};

public:
    KnightBot(const string &name, int x, int y)
        : Robot(name, x, y),
          GenericRobot(name, x, y) {}

    void fire() override
    {
        if (hasFired)
            return;
        hasFired = true;
        int x = getX();
        int y = getY();
        bool fired = false;
        bool hitSuccessful = false;
        vector<string> hitRobots;
        // Diagonal directions: (1,1), (1,-1), (-1,1), (-1,-1)
        const vector<pair<int, int>> diagonals = {{1, 1}, {1, -1}, {-1, 1}, {-1, -1}};
        // Randomly select a diagonal
        int diagIdx = rand() % diagonals.size();
        int dx = diagonals[diagIdx].first;
        int dy = diagonals[diagIdx].second;
        logger << getName() << " selects diagonal (" << dx << "," << dy << ") for attack (length 5)" << endl;
        for (int dist = 1; dist <= 5; ++dist)
        {
            int targetX = x + dx * dist;
            int targetY = y + dy * dist;
            if (!battlefield->isPositionWithinGrid(targetX, targetY))
                break;
            Robot *target = battlefield->getRobotAt(targetX, targetY);
            if (target && target != this && target->getLives() > 0)
            {
                GenericRobot *gtarget = dynamic_cast<GenericRobot *>(target);
                logger << getName() << "Knight fires at (" << targetX << "," << targetY << ")" << endl;
                fired = true;
                if (gtarget && gtarget->isHit())
                {
                    if (hitProbability())
                    {
                        gtarget->takeDamage();
                        fire_count++;
                        hitSuccessful = true;
                        hitRobots.push_back(gtarget->getName());
                        logger << "Hit! (" << gtarget->getName() << ") be killed" << endl;
                    }
                    else
                    {
                        logger << "Missed!" << endl;
                    }
                }
            }
        }
        if (!fired)
        {
            logger << getName() << " no robots in diagonal to fire at\n";
        }
        else if (hitSuccessful)
        {
            logger << getName() << " hit the following robots: ";
            for (size_t i = 0; i < hitRobots.size(); ++i)
            {
                logger << hitRobots[i];
                if (i != hitRobots.size() - 1)
                {
                    logger << ", ";
                }
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
class ScoutBot : public virtual GenericRobot
{
private:
    int scout_count = 0;

public:
    ScoutBot(const string &name, int x, int y)
        : Robot(name, x, y),
          GenericRobot(name, x, y) {}

    void look(int X, int Y)
    {
        if (scout_count >= 3)
        {
            logger << getName() << " reach the limit,cannot scan already\n";
            return;
        }

        if (rand() % 2 == 0)
        {
            logger << getName() << " scan the battlefield\n";

            for (int y = 0; y < battlefield->getHeight(); ++y)
            {
                for (int x = 0; x < battlefield->getWidth(); ++x)
                {
                    Robot *r = battlefield->getRobotAt(x, y);
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

    void fire() override
    {
        if (scout_count >= 3)
        {
            logger << getName() << " reach the limit,cannot fire already\n";
            return;
        }

        bool hitTarget = false;
        if (rand() % 2 == 0)
        {
            logger << getName() << " fires (scout style)\n";
            // Example: just log all visible robots (like scan)
            for (int y = 0; y < battlefield->getHeight(); ++y)
            {
                for (int x = 0; x < battlefield->getWidth(); ++x)
                {
                    Robot *target = battlefield->getRobotAt(x, y);
                    if (target && target != this && target->getLives() > 0 && !target->isHidden())
                    {
                        logger << getName() << " spots and fires at " << target->getName() << " at (" << x << "," << y << ")\n";
                        hitTarget = true;
                        // Optionally, you could reduce target's life or mark as hit here
                    }
                }
            }
            scout_count++;
            if (hitTarget)
            {
                // Randomly choose between HideScoutBot and JumpScoutBot
                static const vector<string> types = {"HideScoutBot", "JumpScoutBot"};
                int t = rand() % types.size();
                setPendingUpgrade(types[t]);
                logger << getName() << " will upgrade to " << types[t] << " next turn!\n";
            }
        }
        else
        {
            logger << getName() << " try fire next round \n";
        }
    }

    void act() override
    {
        logger << "ScoutBot is thinking..." << endl;
        // TO DO : the logic will be implemented later
        look(0, 0);
        fire();
        move();
    }

    int getScoutCount() const
    {
        return scout_count;
    }
};

//******************************************
// TrackBot
//******************************************

class TrackBot : public virtual GenericRobot
{
private:
    int tracker = 3;
    vector<Robot *> track_target;

public:
    TrackBot(const string &name, int x, int y)
        : Robot(name, x, y),
          GenericRobot(name, x, y) {}

    void look(int X, int Y)
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

                Robot *target = battlefield->getRobotAt(targetX, targetY);

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
        logger << "TrackBot is thinking..." << endl;
        look(0, 0);
        fire();
        move();
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

    void fire() override
    {
        if (tracker == 0)
        {
            logger << getName() << " cannot fire, no tracker left\n";
            return;
        }
        bool hitTarget = false;
        if (rand() % 2 == 0)
        {
            logger << getName() << " fires (track style)\n";
            for (Robot *target : track_target)
            {
                if (target && target != this && target->getLives() > 0 && !target->isHidden())
                {
                    logger << getName() << " tracks and fires at " << target->getName() << " at (" << target->getX() << "," << target->getY() << ")\n";
                    hitTarget = true;
                    // Optionally, you could reduce target's life or mark as hit here
                }
            }
            tracker--;
            if (hitTarget)
            {
                static const vector<string> types = {"HideTrackBot", "JumpTrackBot"};
                int t = rand() % types.size();
                setPendingUpgrade(types[t]);
                logger << getName() << " will upgrade to " << types[t] << " next turn!\n";
            }
        }
        else
        {
            logger << getName() << " try fire next round \n";
        }
    }
};

//******************************************
// QueenBot
//******************************************

class QueenBot : public virtual GenericRobot
{
private:
    const vector<pair<int, int>> directions = {
        {0, 1},
        {1, 0},
        {0, -1},
        {-1, 0},
        {1, 1},
        {1, -1},
        {-1, 1},
        {-1, -1}};

public:
    QueenBot(const string &name, int x, int y)
        : Robot(name, x, y),
          GenericRobot(name, x, y) {}

    void fire() override
    {
        if (hasFired)
            return;
        hasFired = true;

        if (!hasAmmo())
        {
            logger << getName() << " has no ammo left. It will self destruct!" << endl;
            lives = 0;
            isDie = true;
            return;
        }

        int x = getX();
        int y = getY();
        bool fired = false;

        for (const auto &dir : directions)
        {
            int dx = dir.first;
            int dy = dir.second;
            for (int dist = 1;; dist++)
            {
                int targetX = x + dx * dist;
                int targetY = y + dy * dist;
                if (targetX < 0 || targetY < 0 || targetX >= battlefield->getWidth() || targetY >= battlefield->getHeight())
                    break;
                if (targetX == x && targetY == y)
                    continue; // Do not fire at self
                Robot *target = battlefield->getRobotAt(targetX, targetY);
                if (target && target != this)
                {
                    GenericRobot *gtarget = dynamic_cast<GenericRobot *>(target);
                    if (gtarget->isHit())
                    {
                        logger << getName() << " Queen fires at (" << targetX << "," << targetY << ")\n";
                        if (hitProbability())
                        {
                            gtarget->takeDamage();
                            logger << getName() << " hit " << gtarget->getName() << endl;
                            static const vector<string> types = {"HideQueenBot", "JumpQueenBot", "QueenScoutBot", "QueenTrackBot"};
                            int t = rand() % types.size();
                            setPendingUpgrade(types[t]);
                            logger << getName() << " will upgrade into " << types[t] << " next turn" << endl;
                        }
                        else
                        {
                            logger << "Missed!" << endl;
                        }
                        fired = true;
                        break;
                    }
                }
            }
            if (fired)
                break;
        }
        useAmmo();
        if (!fired)
        {
            logger << "sadly, " << getName() << " found no target in straight line\n";
        }
    }
};

//******************************************
// VampireBot
// Upgrade description: After killing a robot, gain 1 life, can gain life maximum 3 times
//******************************************
class VampireBot : public virtual GenericRobot
{
private:
    int gainLivesCount = 0;

public:
    // Constructor
    VampireBot(const string &name, int x, int y)
        : Robot(name, x, y),
          GenericRobot(name, x, y) {};

    void fire() override
    {
        if (!hasAmmo())
        {
            logger << getName() << " has no ammo left!" << endl;
            isDie = true;
            return;
        }

        if (!detectedTargets.empty())
        {
            int randomIndex = rand() % detectedTargets.size();
            Robot *target = detectedTargets[randomIndex];
            int targetX = target->getX();
            int targetY = target->getY();
            logger << ">> " << getName() << "Vampire fires at (" << targetX << ", " << targetY << ")" << endl;
            useAmmo();

            if (target->isHidden())
            {
                logger << target->getName() << " is hide, attack miss." << endl;
            }
            else if (hitProbability())
            {
                logger << "Hit! (" << target->getName() << ") be killed" << endl;

                int targetLivesBeforeHit = target->getLives(); // Store lives before damage
                target->takeDamage();                          // Apply damage

                if (target->getLives() <= 0 && targetLivesBeforeHit > 0)
                {
                    logger << getName() << " killed " << target->getName() << "!" << endl;
                    if (getLives() < 3)
                    {
                        if (gainLivesCount <= 3)
                        {
                            setLives(getLives() + 1);
                            logger << getName() << " gained 1 life from killing " << target->getName() << "! (" << gainLivesCount + 1 << "/3)" << endl;
                            gainLivesCount++;
                        }
                        else
                        {
                            logger << getName() << " already gained lives 3 times, cannot gain anymore lives." << endl;
                        }
                    }
                    else
                    {
                        logger << getName() << " is already at max lives (" << getLives() << ")." << endl;
                    }
                }

                static const vector<string> types = {"HideVampireBot", "JumpVampireBot", "VampireScoutBot", "VampireTrackBot"};
                int t = rand() % types.size();
                setPendingUpgrade(types[t]);
                logger << getName() << " will upgrade in to " << types[t] << "next turn" << endl;
            }
            else
            {
                logger << "Missed!" << endl;
            }
        }
        else
        {
            logger << getName() << " has no valid targets to fire at!" << endl;
        }
    }
};

//******************************************
// HideLongShotBot (inherits from HideBot and LongShotBot)
//******************************************
class HideLongShotBot : public HideBot, public LongShotBot
{
public:
    HideLongShotBot(const string &name, int x, int y)
        : Robot(name, x, y),
          GenericRobot(name, x, y),
          HideBot(name, x, y),
          LongShotBot(name, x, y) {}

    void move() override
    {
        HideBot::move();
    }

    void fire() override
    {
        LongShotBot::fire();
    }

    void think() override
    {
        // Disambiguate GenericRobot base
        HideBot::think();
    }

    void act() override
    {
        LongShotBot::look(0, 0); // Use long-range look for targeting
        think();
        fire();
        move();
    }

    void look(int X, int Y) override
    {
        LongShotBot::look(X, Y); // Use long-range look for any explicit look calls
    }

    bool isHit() override
    {
        return HideBot::isHit();
    }

    void setBattlefield(Battlefield *bf)
    {
        GenericRobot::setBattlefield(bf);
    }
};

//******************************************
// HideSemiAutoBot (inherits from HideBot and SemiAutoBot)
//******************************************
class HideSemiAutoBot : public HideBot, public SemiAutoBot
{
public:
    HideSemiAutoBot(const string &name, int x, int y)
        : Robot(name, x, y),
          GenericRobot(name, x, y),
          HideBot(name, x, y),
          SemiAutoBot(name, x, y) {}

    void move() override
    {
        HideBot::move();
    }

    void fire() override
    {
        SemiAutoBot::fire();
    }

    void think() override
    {
        HideBot::think();
    }

    void act() override
    {
        SemiAutoBot::look(0, 0); // Use SemiAutoBot's look for targeting
        think();
        fire();
        move();
    }

    void look(int X, int Y) override
    {
        SemiAutoBot::look(X, Y);
    }

    bool isHit() override
    {
        return HideBot::isHit();
    }

    void setBattlefield(Battlefield *bf)
    {
        SemiAutoBot::setBattlefield(bf);
    }
};

//******************************************
// HideThirtyShotBot (inherits from HideBot and ThirtyShotBot)
//******************************************
class HideThirtyShotBot : public HideBot, public ThirtyShotBot
{
public:
    HideThirtyShotBot(const string &name, int x, int y)
        : Robot(name, x, y),
          GenericRobot(name, x, y),
          HideBot(name, x, y),
          ThirtyShotBot(name, x, y) {}

    void move() override
    {
        HideBot::move();
    }

    void fire() override
    {
        ThirtyShotBot::fire();
    }

    void think() override
    {
        HideBot::think();
    }

    void act() override
    {
        ThirtyShotBot::look(0, 0); // Use ThirtyShotBot's look for targeting
        think();
        fire();
        move();
    }

    void look(int X, int Y) override
    {
        ThirtyShotBot::look(X, Y);
    }

    bool isHit() override
    {
        return HideBot::isHit();
    }

    void setBattlefield(Battlefield *bf)
    {
        ThirtyShotBot::setBattlefield(bf);
    }
};

//******************************************
// HideKnightBot (inherits from HideBot and KnightBot)
//******************************************
class HideKnightBot : public HideBot, public KnightBot
{
public:
    HideKnightBot(const string &name, int x, int y)
        : Robot(name, x, y),
          GenericRobot(name, x, y),
          HideBot(name, x, y),
          KnightBot(name, x, y) {}

    void move() override
    {
        HideBot::move();
    }

    void fire() override
    {
        KnightBot::fire();
    }

    void think() override
    {
        HideBot::think();
    }

    void act() override
    {
        KnightBot::look(0, 0); // Use KnightBot's look for targeting
        think();
        fire();
        move();
    }

    void look(int X, int Y) override
    {
        KnightBot::look(X, Y);
    }

    bool isHit() override
    {
        return HideBot::isHit();
    }

    void setBattlefield(Battlefield *bf)
    {
        KnightBot::setBattlefield(bf);
    }
};

//******************************************
// HideQueenBot (inherits from HideBot and QueenBot)
//******************************************
class HideQueenBot : public HideBot, public QueenBot
{
public:
    HideQueenBot(const string &name, int x, int y)
        : Robot(name, x, y),
          GenericRobot(name, x, y),
          HideBot(name, x, y),
          QueenBot(name, x, y) {}

    void move() override
    {
        HideBot::move();
    }

    void fire() override
    {
        QueenBot::fire();
    }

    void think() override
    {
        HideBot::think();
    }

    void act() override
    {
        QueenBot::look(0, 0); // Use QueenBot's look for targeting
        think();
        fire();
        move();
    }

    void look(int X, int Y) override
    {
        QueenBot::look(X, Y);
    }

    bool isHit() override
    {
        return HideBot::isHit();
    }

    void setBattlefield(Battlefield *bf)
    {
        QueenBot::setBattlefield(bf);
    }
};

//******************************************
// HideVampireBot (inherits from HideBot and VampireBot)
//******************************************
class HideVampireBot : public HideBot, public VampireBot
{
public:
    HideVampireBot(const string &name, int x, int y)
        : Robot(name, x, y),
          GenericRobot(name, x, y),
          HideBot(name, x, y),
          VampireBot(name, x, y) {}

    void move() override
    {
        HideBot::move();
    }

    void fire() override
    {
        VampireBot::fire();
    }

    void think() override
    {
        HideBot::think();
    }

    void act() override
    {
        VampireBot::look(0, 0); // Use VampireBot's look for targeting
        think();
        fire();
        move();
    }

    void look(int X, int Y) override
    {
        VampireBot::look(X, Y);
    }

    bool isHit() override
    {
        return HideBot::isHit();
    }

    void setBattlefield(Battlefield *bf)
    {
        VampireBot::setBattlefield(bf);
    }
};

//******************************************
// HideScoutBot (inherits from HideBot and ScoutBot)
//******************************************
class HideScoutBot : public HideBot, public ScoutBot
{
public:
    HideScoutBot(const string &name, int x, int y)
        : Robot(name, x, y),
          GenericRobot(name, x, y),
          HideBot(name, x, y),
          ScoutBot(name, x, y) {}

    void move() override
    {
        HideBot::move();
    }

    void fire() override
    {
        ScoutBot::fire();
    }

    void think() override
    {
        HideBot::think();
    }

    void act() override
    {
        ScoutBot::look(0, 0); // Use ScoutBot's look for targeting
        think();
        fire();
        move();
    }

    void look(int X, int Y) override
    {
        ScoutBot::look(X, Y);
    }

    bool isHit() override
    {
        return HideBot::isHit();
    }

    void setBattlefield(Battlefield *bf)
    {
        ScoutBot::setBattlefield(bf);
    }
};

//******************************************
// HideTrackBot (inherits from HideBot and TrackBot)
//******************************************
class HideTrackBot : public HideBot, public TrackBot
{
public:
    HideTrackBot(const string &name, int x, int y)
        : Robot(name, x, y),
          GenericRobot(name, x, y),
          HideBot(name, x, y),
          TrackBot(name, x, y) {}

    void move() override
    {
        HideBot::move();
    }

    void fire() override
    {
        TrackBot::fire();
    }

    void think() override
    {
        HideBot::think();
    }

    void act() override
    {
        TrackBot::look(0, 0); // Use TrackBot's look for targeting
        think();
        fire();
        move();
    }

    void look(int X, int Y) override
    {
        TrackBot::look(X, Y);
    }

    bool isHit() override
    {
        return HideBot::isHit();
    }

    void setBattlefield(Battlefield *bf)
    {
        TrackBot::setBattlefield(bf);
    }
};

//******************************************
// JumpLongShotBot (inherits from JumpBot and LongShotBot)
//******************************************
class JumpLongShotBot : public JumpBot, public LongShotBot
{
public:
    JumpLongShotBot(const string &name, int x, int y)
        : Robot(name, x, y),
          GenericRobot(name, x, y),
          JumpBot(name, x, y),
          LongShotBot(name, x, y) {}

    void move() override
    {
        JumpBot::move();
    }

    void fire() override
    {
        LongShotBot::fire();
    }

    void think() override
    {
        JumpBot::think();
    }

    void act() override
    {
        LongShotBot::look(0, 0); // Use long-range look for targeting
        think();
        fire();
        move();
    }

    void look(int X, int Y) override
    {
        LongShotBot::look(X, Y);
    }

    bool isHit() override
    {
        return JumpBot::isHit();
    }

    void setBattlefield(Battlefield *bf)
    {
        LongShotBot::setBattlefield(bf);
    }
};

//******************************************
// JumpSemiAutoBot (inherits from JumpBot and SemiAutoBot)
//******************************************
class JumpSemiAutoBot : public JumpBot, public SemiAutoBot
{
public:
    JumpSemiAutoBot(const string &name, int x, int y)
        : Robot(name, x, y),
          GenericRobot(name, x, y),
          JumpBot(name, x, y),
          SemiAutoBot(name, x, y) {}

    void move() override
    {
        JumpBot::move();
    }

    void fire() override
    {
        SemiAutoBot::fire();
    }

    void think() override
    {
        JumpBot::think();
    }

    void act() override
    {
        SemiAutoBot::look(0, 0); // Use SemiAutoBot's look for targeting
        think();
        fire();
        move();
    }

    void look(int X, int Y) override
    {
        SemiAutoBot::look(X, Y);
    }

    bool isHit() override
    {
        return JumpBot::isHit();
    }

    void setBattlefield(Battlefield *bf)
    {
        SemiAutoBot::setBattlefield(bf);
    }
};
//******************************************
// JumpThirtyShotBot (inherits from JumpBot and ThirtyShotBot)
//******************************************
class JumpThirtyShotBot : public JumpBot, public ThirtyShotBot
{
public:
    JumpThirtyShotBot(const string &name, int x, int y)
        : Robot(name, x, y),
          GenericRobot(name, x, y),
          JumpBot(name, x, y),
          ThirtyShotBot(name, x, y) {}

    void move() override
    {
        JumpBot::move();
    }

    void fire() override
    {
        ThirtyShotBot::fire();
    }

    void think() override
    {
        JumpBot::think();
    }

    void act() override
    {
        ThirtyShotBot::look(0, 0); // Use ThirtyShotBot's look for targeting
        think();
        fire();
        move();
    }

    void look(int X, int Y) override
    {
        ThirtyShotBot::look(X, Y);
    }

    bool isHit() override
    {
        return JumpBot::isHit();
    }

    void setBattlefield(Battlefield *bf)
    {
        ThirtyShotBot::setBattlefield(bf);
    }
};

//******************************************
// JumpKnightBot (inherits from JumpBot and KnightBot)
//******************************************
class JumpKnightBot : public JumpBot, public KnightBot
{
public:
    JumpKnightBot(const string &name, int x, int y)
        : Robot(name, x, y),
          GenericRobot(name, x, y),
          JumpBot(name, x, y),
          KnightBot(name, x, y) {}

    void move() override
    {
        JumpBot::move();
    }

    void fire() override
    {
        KnightBot::fire();
    }

    void think() override
    {
        JumpBot::think();
    }

    void act() override
    {
        KnightBot::look(0, 0); // Use ThirtyShotBot's look for targeting
        think();
        fire();
        move();
    }

    void look(int X, int Y) override
    {
        KnightBot::look(X, Y);
    }

    bool isHit() override
    {
        return JumpBot::isHit();
    }

    void setBattlefield(Battlefield *bf)
    {
        KnightBot::setBattlefield(bf);
    }
};

//******************************************
// JumpQueenBot (inherits from JumpBot and QueenBot)
//******************************************
class JumpQueenBot : public JumpBot, public QueenBot
{
public:
    JumpQueenBot(const string &name, int x, int y)
        : Robot(name, x, y),
          GenericRobot(name, x, y),
          JumpBot(name, x, y),
          QueenBot(name, x, y) {}

    void move() override
    {
        JumpBot::move();
    }

    void fire() override
    {
        QueenBot::fire();
    }

    void think() override
    {
        JumpBot::think();
    }

    void act() override
    {
        QueenBot::look(0, 0); // Use QueenBot's look for targeting
        think();
        fire();
        move();
    }

    void look(int X, int Y) override
    {
        QueenBot::look(X, Y);
    }

    bool isHit() override
    {
        return JumpBot::isHit();
    }

    void setBattlefield(Battlefield *bf)
    {
        QueenBot::setBattlefield(bf);
    }
};

//******************************************
// JumpVampireBot (inherits from JumpBot and VampireBot)
//******************************************
class JumpVampireBot : public JumpBot, public VampireBot
{
public:
    JumpVampireBot(const string &name, int x, int y)
        : Robot(name, x, y),
          GenericRobot(name, x, y),
          JumpBot(name, x, y),
          VampireBot(name, x, y) {}

    void move() override
    {
        JumpBot::move();
    }

    void fire() override
    {
        VampireBot::fire();
    }

    void think() override
    {
        JumpBot::think();
    }

    void act() override
    {
        VampireBot::look(0, 0); // Use QueenBot's look for targeting
        think();
        fire();
        move();
    }

    void look(int X, int Y) override
    {
        VampireBot::look(X, Y);
    }

    bool isHit() override
    {
        return JumpBot::isHit();
    }

    void setBattlefield(Battlefield *bf)
    {
        VampireBot::setBattlefield(bf);
    }
};

//******************************************
// JumpScoutBot (inherits from JumpBot and ScoutBot)
//******************************************
class JumpScoutBot : public JumpBot, public ScoutBot
{
public:
    JumpScoutBot(const string &name, int x, int y)
        : Robot(name, x, y),
          GenericRobot(name, x, y),
          JumpBot(name, x, y),
          ScoutBot(name, x, y) {}

    void move() override
    {
        JumpBot::move();
    }

    void fire() override
    {
        ScoutBot::fire();
    }

    void think() override
    {
        JumpBot::think();
    }

    void act() override
    {
        ScoutBot::look(0, 0); // Use ScoutBot's look for targeting
        think();
        fire();
        move();
    }

    void look(int X, int Y) override
    {
        ScoutBot::look(X, Y);
    }

    bool isHit() override
    {
        return JumpBot::isHit();
    }

    void setBattlefield(Battlefield *bf)
    {
        ScoutBot::setBattlefield(bf);
    }
};

//******************************************
// JumpTrackBot (inherits from JumpBot and TrackBot)
//******************************************
class JumpTrackBot : public JumpBot, public TrackBot
{
public:
    JumpTrackBot(const string &name, int x, int y)
        : Robot(name, x, y),
          GenericRobot(name, x, y),
          JumpBot(name, x, y),
          TrackBot(name, x, y) {}

    void move() override
    {
        JumpBot::move();
    }

    void fire() override
    {
        TrackBot::fire();
    }

    void think() override
    {
        JumpBot::think();
    }

    void act() override
    {
        TrackBot::look(0, 0); // Use ScoutBot's look for targeting
        think();
        fire();
        move();
    }

    void look(int X, int Y) override
    {
        TrackBot::look(X, Y);
    }

    bool isHit() override
    {
        return JumpBot::isHit();
    }

    void setBattlefield(Battlefield *bf)
    {
        TrackBot::setBattlefield(bf);
    }
};

//******************************************
// LongShotScoutBot
// (inherits from LongShotBot and ScoutBot)
//******************************************
class LongShotScoutBot : public LongShotBot, public ScoutBot
{
public:
    LongShotScoutBot(const string &name, int x, int y) : Robot(name, x, y),
                                                         GenericRobot(name, x, y),
                                                         LongShotBot(name, x, y),
                                                         ScoutBot(name, x, y)
    {
    }
    void move() override
    {
        ScoutBot::move();
    }

    void fire() override
    {
        LongShotBot::fire();
    }

    void think() override
    {
        // Disambiguate GenericRobot base
        ScoutBot::think();
    }

    void act() override
    {
        LongShotBot::look(0, 0);
        think();
        fire();
        move();
    }

    void look(int X, int Y) override
    {
        LongShotBot::look(X, Y); // Use long-range look for any explicit look calls
    }

    bool isHit() override
    {
        return ScoutBot::isHit();
    }

    void setBattlefield(Battlefield *bf)
    {
        GenericRobot::setBattlefield(bf);
    }
};

//******************************************
// LongShotTrackBot
// (inherits from LongShotBot and TrackBot)
//******************************************
class LongShotTrackBot : public LongShotBot, public TrackBot
{
public:
    LongShotTrackBot(const string &name, int x, int y) : Robot(name, x, y),
                                                         GenericRobot(name, x, y),
                                                         LongShotBot(name, x, y),
                                                         TrackBot(name, x, y) {}

    void move() override
    {
        TrackBot::move();
    }

    void fire() override
    {
        LongShotBot::fire();
    }

    void think() override
    {
        TrackBot::think();
    }

    void act() override
    {
        LongShotBot::look(0, 0);
        think();
        fire();
        move();
    }

    void look(int X, int Y) override
    {
        TrackBot::look(X, Y); // Use long-range look for any explicit look calls
    }

    bool isHit() override
    {
        return TrackBot::isHit();
    }

    void setBattlefield(Battlefield *bf)
    {
        GenericRobot::setBattlefield(bf);
    }
};

//******************************************
// SemiAutoScoutBot
// (inherits from SemiAutoBot and ScoutBot)
//******************************************
class SemiAutoScoutBot : public SemiAutoBot, public ScoutBot
{
public:
    SemiAutoScoutBot(const string &name, int x, int y) : Robot(name, x, y),
                                                         GenericRobot(name, x, y),
                                                         SemiAutoBot(name, x, y),
                                                         ScoutBot(name, x, y) {}

    void move() override
    {
        ScoutBot::move();
    }

    void fire() override
    {
        SemiAutoBot::fire();
    }

    void think() override
    {
        ScoutBot::think();
    }

    void act() override
    {
        SemiAutoBot::look(0, 0);
        think();
        fire();
        move();
    }

    void look(int X, int Y) override
    {
        ScoutBot::look(X, Y); // Use long-range look for any explicit look calls
    }

    bool isHit() override
    {
        return ScoutBot::isHit();
    }

    void setBattlefield(Battlefield *bf)
    {
        GenericRobot::setBattlefield(bf);
    }
};

//******************************************
// SemiAutoTrackBot
// inherits from SemiAutoBot and TrackBot
//******************************************
class SemiAutoTrackBot : public SemiAutoBot, public TrackBot
{
public:
    SemiAutoTrackBot(const string &name, int x, int y) : Robot(name, x, y), GenericRobot(name, x, y), SemiAutoBot(name, x, y), TrackBot(name, x, y) {}

    void move() override
    {
        TrackBot::move();
    }

    void fire() override
    {
        SemiAutoBot::fire();
    }

    void think() override
    {
        SemiAutoBot::think();
    }

    void act() override
    {
        SemiAutoBot::look(0, 0);
        think();
        fire();
        move();
    }

    void look(int X, int Y) override
    {
        TrackBot::look(X, Y); // Use long-range look for any explicit look calls
    }

    bool isHit() override
    {
        return TrackBot::isHit();
    }

    void setBattlefield(Battlefield *bf)
    {
        GenericRobot::setBattlefield(bf);
    }
};

//******************************************
// ThirtyShotScoutBot
// inherits from ThirtyShotBot and ScoutBot
//******************************************
class ThirtyShotScoutBot : public ThirtyShotBot, public ScoutBot {
public:
    ThirtyShotScoutBot(const string& name, int x, int y) : Robot(name, x, y), GenericRobot(name, x, y), ThirtyShotBot(name, x, y), ScoutBot(name, x, y) {}

    void move() override
    {
        ScoutBot::move();
    }

    void fire() override
    {
        ThirtyShotBot::fire();
    }

    void think() override
    {
        ScoutBot::think();
    }

    void act() override
    {
        ScoutBot::look(0, 0);
        think();
        fire();
        move();
    }

    void look(int X, int Y) override
    {
        ScoutBot::look(X, Y); // Use long-range look for any explicit look calls
    }

    bool isHit() override
    {
        return ScoutBot::isHit();
    }

    void setBattlefield(Battlefield *bf)
    {
        GenericRobot::setBattlefield(bf);
    }
};

//******************************************
// ThirtyShotTrackBot
// inherits from ThirtyShotBot and TrackBot
//******************************************
class ThirtyShotTrackBot : public ThirtyShotBot, public TrackBot {
public:
    ThirtyShotTrackBot(const string& name, int x, int y) : Robot(name, x, y), GenericRobot(name, x, y), ThirtyShotBot(name, x, y), TrackBot(name, x, y) {}

    void move() override
    {
        TrackBot::move();
    }

    void fire() override
    {
        ThirtyShotBot::fire();
    }

    void think() override
    {
        TrackBot::think();
    }

    void act() override
    {
        TrackBot::look(0, 0);
        think();
        fire();
        move();
    }

    void look(int X, int Y) override
    {
        TrackBot::look(X, Y); // Use long-range look for any explicit look calls
    }

    bool isHit() override
    {
        return TrackBot::isHit();
    }

    void setBattlefield(Battlefield *bf)
    {
        GenericRobot::setBattlefield(bf);
    }
};

//******************************************
// KnightScoutBot 
// inherits from KnightBot and ScoutBot
//******************************************
class KnightScoutBot: public KnightBot, public ScoutBot {
public:
    KnightScoutBot(const string& name, int x, int y) : Robot(name, x, y), GenericRobot(name, x, y), KnightBot(name, x, y), ScoutBot(name, x, y) {}

    void move() override
    {
        ScoutBot::move();
    }

    void fire() override
    {
        KnightBot::fire();
    }

    void think() override
    {
        ScoutBot::think();
    }

    void act() override
    {
        ScoutBot::look(0, 0);
        think();
        fire();
        move();
    }

    void look(int X, int Y) override
    {
        ScoutBot::look(X, Y); // Use long-range look for any explicit look calls
    }

    bool isHit() override
    {
        return ScoutBot::isHit();
    }

    void setBattlefield(Battlefield *bf)
    {
        GenericRobot::setBattlefield(bf);
    }
};

//******************************************
// KnightTrackBot 
// inherits from KnightBot and TrackBot 
//******************************************
class KnightTrackBot: public KnightBot, public TrackBot {
public:
    KnightTrackBot(const string& name, int x, int y) : Robot(name, x, y), GenericRobot(name, x, y), KnightBot(name, x, y), TrackBot(name, x, y) {}

    void move() override
    {
        TrackBot::move();
    }

    void fire() override
    {
        KnightBot::fire();
    }

    void think() override
    {
        TrackBot::think();
    }

    void act() override
    {
        TrackBot::look(0, 0);
        think();
        fire();
        move();
    }

    void look(int X, int Y) override
    {
        TrackBot::look(X, Y); // Use long-range look for any explicit look calls
    }

    bool isHit() override
    {
        return TrackBot::isHit();
    }

    void setBattlefield(Battlefield *bf)
    {
        GenericRobot::setBattlefield(bf);
    }
};

//******************************************
// QueenScoutBot 
// inherits from QueenBot and ScoutBot 
//******************************************
class QueenScoutBot: public QueenBot, public ScoutBot {
public:
    QueenScoutBot(const string& name, int x, int y) : Robot(name, x, y), GenericRobot(name, x, y), QueenBot(name, x, y), ScoutBot(name, x, y) {}

    void move() override
    {
        ScoutBot::move();
    }

    void fire() override
    {
        QueenBot::fire();
    }

    void think() override
    {
        ScoutBot::think();
    }

    void act() override
    {
        ScoutBot::look(0, 0);
        think();
        fire();
        move();
    }

    void look(int X, int Y) override
    {
        ScoutBot::look(X, Y); // Use long-range look for any explicit look calls
    }

    bool isHit() override
    {
        return ScoutBot::isHit();
    }

    void setBattlefield(Battlefield *bf)
    {
        GenericRobot::setBattlefield(bf);
    }
};

//******************************************
// QueenTrackBot 
// inherits from QueenBot and TrackBot 
//******************************************
class QueenTrackBot: public QueenBot, public TrackBot {
public:
    QueenTrackBot(const string& name, int x, int y) : Robot(name, x, y), GenericRobot(name, x, y), QueenBot(name, x, y), TrackBot(name, x, y) {}

    void move() override
    {
        TrackBot::move();
    }

    void fire() override
    {
        QueenBot::fire();
    }

    void think() override
    {
        TrackBot::think();
    }

    void act() override
    {
        TrackBot::look(0, 0);
        think();
        fire();
        move();
    }

    void look(int X, int Y) override
    {
        TrackBot::look(X, Y); // Use long-range look for any explicit look calls
    }

    bool isHit() override
    {
        return TrackBot::isHit();
    }

    void setBattlefield(Battlefield *bf)
    {
        GenericRobot::setBattlefield(bf);
    }
};

//******************************************
// VampireScoutBot 
// inherits from VampireBot and ScoutBot 
//******************************************
class VampireScoutBot: public VampireBot, public ScoutBot {
public:
    VampireScoutBot(const string& name, int x, int y) : Robot(name, x, y), GenericRobot(name, x, y), VampireBot(name, x, y), ScoutBot(name, x, y) {}

    void move() override
    {
        ScoutBot::move();
    }

    void fire() override
    {
        VampireBot::fire();
    }

    void think() override
    {
        ScoutBot::think();
    }

    void act() override
    {
        ScoutBot::look(0, 0);
        think();
        fire();
        move();
    }

    void look(int X, int Y) override
    {
        ScoutBot::look(X, Y); // Use long-range look for any explicit look calls
    }

    bool isHit() override
    {
        return ScoutBot::isHit();
    }

    void setBattlefield(Battlefield *bf)
    {
        GenericRobot::setBattlefield(bf);
    }
};

//******************************************
// VampireTrackBot 
// inherits from VampireBot and TrackBot 
//******************************************
class VampireTrackBot: public VampireBot, public TrackBot {
public:
    VampireTrackBot(const string& name, int x, int y) : Robot(name, x, y), GenericRobot(name, x, y), VampireBot(name, x, y), TrackBot(name, x, y) {}

    void move() override
    {
        TrackBot::move();
    }

    void fire() override
    {
        VampireBot::fire();
    }

    void think() override
    {
        TrackBot::think();
    }

    void act() override
    {
        TrackBot::look(0, 0);
        think();
        fire();
        move();
    }

    void look(int X, int Y) override
    {
        TrackBot::look(X, Y); // Use long-range look for any explicit look calls
    }

    bool isHit() override
    {
        return TrackBot::isHit();
    }

    void setBattlefield(Battlefield *bf)
    {
        GenericRobot::setBattlefield(bf);
    }
};

//******************************************
//HideLongShotScoutBot
//******************************************
class HideLongShotScoutBot: public HideLongShotBot,public ScoutBot
{
public:
    HideLongShotScoutBot(const string &name, int x,int y)
    : Robot(name,x,y),
      GenericRobot(name,x,y),
      HideLongShotBot(name,x,y),
      ScoutBot(name,x,y) {}

    void move() override{
        HideBot::move();
    }

    void fire() override{
        LongShotBot::fire();
    }

    void think() override{
        HideBot::think();
    }

    void act() override{
        ScoutBot::look(0,0);
        think();
        fire();
        move();
       
    }

    void look(int X,int Y) override{
        ScoutBot::look(X,Y);
    }

    bool isHit() override{
        return HideBot::isHit();
    }

    void setBattlefield(Battlefield *bf){
        GenericRobot::setBattlefield(bf);
        HideBot::setBattlefield(bf);
        LongShotBot::setBattlefield(bf);
        ScoutBot::setBattlefield(bf);
    }
    
};

//******************************************
//HideSemiAutoScoutBot
//******************************************
class HideSemiAutoScoutBot: public HideSemiAutoBot,public ScoutBot
{
public:
    HideSemiAutoScoutBot(const string &name, int x,int y)
    : Robot(name,x,y),
      GenericRobot(name,x,y),
      HideSemiAutoBot(name,x,y),
      ScoutBot(name,x,y) {}

    void move() override{
        HideBot::move();
    }

    void fire() override{
        SemiAutoBot::fire();
    }

    void think() override{
        HideBot::think();
    }

    void act() override{
        ScoutBot::look(0,0);
        think();
        fire();
        move();
       
    }

    void look(int X,int Y) override{
        ScoutBot::look(X,Y);
    }

    bool isHit() override{
        return HideBot::isHit();
    }

    void setBattlefield(Battlefield *bf){
        GenericRobot::setBattlefield(bf);
        HideBot::setBattlefield(bf);
        SemiAutoBot::setBattlefield(bf);
        ScoutBot::setBattlefield(bf);
    }
    
};

//******************************************
//HideThirtyShotScoutBot
//******************************************
class HideThirtyShotScoutBot: public HideThirtyShotBot,public ScoutBot
{
public:
    HideThirtyShotScoutBot(const string &name, int x,int y)
    : Robot(name,x,y),
      GenericRobot(name,x,y),
      HideThirtyShotBot(name,x,y),
      ScoutBot(name,x,y) {}

    void move() override{
        HideBot::move();
    }

    void fire() override{
        ThirtyShotBot::fire();
    }

    void think() override{
        HideBot::think();
    }

    void act() override{
        ScoutBot::look(0,0);
        think();
        fire();
        move();
       
    }

    void look(int X,int Y) override{
        ScoutBot::look(X,Y);
    }

    bool isHit() override{
        return HideBot::isHit();
    }

    void setBattlefield(Battlefield *bf){
        GenericRobot::setBattlefield(bf);
        HideBot::setBattlefield(bf);
        ThirtyShotBot::setBattlefield(bf);
        ScoutBot::setBattlefield(bf);
    }
    
};

//******************************************
//HideKnightScoutBot
//******************************************
class HideKnightScoutBot: public HideKnightBot,public ScoutBot
{
public:
    HideKnightScoutBot(const string &name, int x,int y)
    : Robot(name,x,y),
      GenericRobot(name,x,y),
      HideKnightBot(name,x,y),
      ScoutBot(name,x,y) {}

    void move() override{
        HideBot::move();
    }

    void fire() override{
        KnightBot::fire();
    }

    void think() override{
        HideBot::think();
    }

    void act() override{
        ScoutBot::look(0,0);
        think();
        fire();
        move();
       
    }

    void look(int X,int Y) override{
        ScoutBot::look(X,Y);
    }

    bool isHit() override{
        return HideBot::isHit();
    }

    void setBattlefield(Battlefield *bf){
        GenericRobot::setBattlefield(bf);
        HideBot::setBattlefield(bf);
        KnightBot::setBattlefield(bf);
        ScoutBot::setBattlefield(bf);
    }
    
};

//******************************************
//HideQueenScoutBot
//******************************************

class HideQueenScoutBot: public HideQueenBot,public ScoutBot
{
public:
    HideQueenScoutBot(const string &name, int x,int y)
    : Robot(name,x,y),
      GenericRobot(name,x,y),
      HideQueenBot(name,x,y),
      ScoutBot(name,x,y) {}

    void move() override{
        HideBot::move();
    }

    void fire() override{
        QueenBot::fire();
    }

    void think() override{
        HideBot::think();
    }

    void act() override{
        ScoutBot::look(0,0);
        think();
        fire();
        move();
       
    }

    void look(int X,int Y) override{
        ScoutBot::look(X,Y);
    }

    bool isHit() override{
        return HideBot::isHit();
    }

    void setBattlefield(Battlefield *bf){
        GenericRobot::setBattlefield(bf);
        HideBot::setBattlefield(bf);
        QueenBot::setBattlefield(bf);
        ScoutBot::setBattlefield(bf);
    }
    
};

//******************************************
//HideVampireScoutBot
//******************************************
class HideVampireScoutBot: public HideVampireBot,public ScoutBot
{
public:
    HideVampireScoutBot(const string &name, int x,int y)
    : Robot(name,x,y),
      GenericRobot(name,x,y),
      HideVampireBot(name,x,y),
      ScoutBot(name,x,y) {}

    void move() override{
        HideBot::move();
    }

    void fire() override{
        VampireBot::fire();
    }

    void think() override{
        HideBot::think();
    }

    void act() override{
        ScoutBot::look(0,0);
        think();
        fire();
        move();
       
    }

    void look(int X,int Y) override{
        ScoutBot::look(X,Y);
    }

    bool isHit() override{
        return HideBot::isHit();
    }

    void setBattlefield(Battlefield *bf){
        GenericRobot::setBattlefield(bf);
        HideBot::setBattlefield(bf);
        VampireBot::setBattlefield(bf);
        ScoutBot::setBattlefield(bf);
    }
    
};

//******************************************
//HideLongShotTrackBot
//******************************************
class HideLongShotTrackBot: public HideLongShotBot,public TrackBot
{
public:
    HideLongShotTrackBot(const string &name, int x,int y)
    : Robot(name,x,y),
      GenericRobot(name,x,y),
      HideLongShotBot(name,x,y),
      TrackBot(name,x,y) {}

    void move() override{
        HideBot::move();
    }

    void fire() override{
        LongShotBot::fire();
    }

    void think() override{
        HideBot::think();
    }

    void act() override{
        TrackBot::look(0,0);
        think();
        fire();
        move();
       
    }

    void look(int X,int Y) override{
        TrackBot::look(X,Y);
    }

    bool isHit() override{
        return HideBot::isHit();
    }

    void setBattlefield(Battlefield *bf){
        GenericRobot::setBattlefield(bf);
        HideBot::setBattlefield(bf);
        LongShotBot::setBattlefield(bf);
        TrackBot::setBattlefield(bf);
    }
    
};


//******************************************
//HideSemiAutoTrackBot
//******************************************
class HideSemiAutoTrackBot: public HideSemiAutoBot,public TrackBot
{
public:
    HideSemiAutoTrackBot(const string &name, int x,int y)
    : Robot(name,x,y),
      GenericRobot(name,x,y),
      HideSemiAutoBot(name,x,y),
      TrackBot(name,x,y) {}

    void move() override{
        HideBot::move();
    }

    void fire() override{
        SemiAutoBot::fire();
    }

    void think() override{
        HideBot::think();
    }

    void act() override{
        TrackBot::look(0,0);
        think();
        fire();
        move();
       
    }

    void look(int X,int Y) override{
        TrackBot::look(X,Y);
    }

    bool isHit() override{
        return HideBot::isHit();
    }

    void setBattlefield(Battlefield *bf){
        GenericRobot::setBattlefield(bf);
        HideBot::setBattlefield(bf);
        SemiAutoBot::setBattlefield(bf);
        TrackBot::setBattlefield(bf);
    }
    
};

//******************************************
//HideThirtyShotTrackBot
//******************************************
class HideThirtyShotTrackBot: public HideThirtyShotBot,public TrackBot
{
public:
    HideThirtyShotTrackBot(const string &name, int x,int y)
    : Robot(name,x,y),
      GenericRobot(name,x,y),
      HideThirtyShotBot(name,x,y),
      TrackBot(name,x,y) {}

    void move() override{
        HideBot::move();
    }

    void fire() override{
        ThirtyShotBot::fire();
    }

    void think() override{
        HideBot::think();
    }

    void act() override{
        TrackBot::look(0,0);
        think();
        fire();
        move();
       
    }

    void look(int X,int Y) override{
        TrackBot::look(X,Y);
    }

    bool isHit() override{
        return HideBot::isHit();
    }

    void setBattlefield(Battlefield *bf){
        GenericRobot::setBattlefield(bf);
        HideBot::setBattlefield(bf);
        ThirtyShotBot::setBattlefield(bf);
        TrackBot::setBattlefield(bf);
    }
    
};

//******************************************
//HideKnightTrackBot
//******************************************
class HideKnightTrackBot: public HideKnightBot,public TrackBot
{
public:
    HideKnightTrackBot(const string &name, int x,int y)
    : Robot(name,x,y),
      GenericRobot(name,x,y),
      HideKnightBot(name,x,y),
      TrackBot(name,x,y) {}

    void move() override{
        HideBot::move();
    }

    void fire() override{
        KnightBot::fire();
    }

    void think() override{
        HideBot::think();
    }

    void act() override{
        TrackBot::look(0,0);
        think();
        fire();
        move();
       
    }

    void look(int X,int Y) override{
        TrackBot::look(X,Y);
    }

    bool isHit() override{
        return HideBot::isHit();
    }

    void setBattlefield(Battlefield *bf){
        GenericRobot::setBattlefield(bf);
        HideBot::setBattlefield(bf);
        KnightBot::setBattlefield(bf);
        TrackBot::setBattlefield(bf);
    }
    
};
//******************************************
//HideQueenTrackBot
//******************************************
class HideQueenTrackBot: public HideQueenBot,public TrackBot
{
public:
    HideQueenTrackBot(const string &name, int x,int y)
    : Robot(name,x,y),
      GenericRobot(name,x,y),
      HideQueenBot(name,x,y),
      TrackBot(name,x,y) {}

    void move() override{
        HideBot::move();
    }

    void fire() override{
        QueenBot::fire();
    }

    void think() override{
        HideBot::think();
    }

    void act() override{
        TrackBot::look(0,0);
        think();
        fire();
        move();
       
    }

    void look(int X,int Y) override{
        TrackBot::look(X,Y);
    }

    bool isHit() override{
        return HideBot::isHit();
    }

    void setBattlefield(Battlefield *bf){
        GenericRobot::setBattlefield(bf);
        HideBot::setBattlefield(bf);
        QueenBot::setBattlefield(bf);
        TrackBot::setBattlefield(bf);
    }
    
};

//******************************************
//HideVampireTrackBot
//******************************************
class HideVampireTrackBot: public HideVampireBot,public TrackBot
{
public:
    HideVampireTrackBot(const string &name, int x,int y)
    : Robot(name,x,y),
      GenericRobot(name,x,y),
      HideVampireBot(name,x,y),
      TrackBot(name,x,y) {}

    void move() override{
        HideBot::move();
    }

    void fire() override{
        VampireBot::fire();
    }

    void think() override{
        HideBot::think();
    }

    void act() override{
        TrackBot::look(0,0);
        think();
        fire();
        move();
       
    }

    void look(int X,int Y) override{
        TrackBot::look(X,Y);
    }

    bool isHit() override{
        return HideBot::isHit();
    }

    void setBattlefield(Battlefield *bf){
        GenericRobot::setBattlefield(bf);
        HideBot::setBattlefield(bf);
        VampireBot::setBattlefield(bf);
        TrackBot::setBattlefield(bf);
    }
    
};

//******************************************
//JumpLongShotScoutBot
//******************************************
class JumpLongShotScoutBot: public JumpLongShotBot,public ScoutBot
{
public:
    JumpLongShotScoutBot(const string &name, int x,int y)
    : Robot(name,x,y),
      GenericRobot(name,x,y),
      JumpLongShotBot(name,x,y),
      ScoutBot(name,x,y) {}

    void move() override{
        JumpBot::move();
    }

    void fire() override{
        LongShotBot::fire();
    }

    void think() override{
        JumpBot::think();
    }

    void act() override{
        ScoutBot::look(0,0);
        think();
        fire();
        move();
       
    }

    void look(int X,int Y) override{
        ScoutBot::look(X,Y);
    }

    bool isHit() override{
        return JumpBot::isHit();
    }

    void setBattlefield(Battlefield *bf){
        GenericRobot::setBattlefield(bf);
        JumpBot::setBattlefield(bf);
        LongShotBot::setBattlefield(bf);
        ScoutBot::setBattlefield(bf);
    }
    
};

//******************************************
//JumpSemiAutoScoutBot
//******************************************
class JumpSemiAutoScoutBot: public JumpSemiAutoBot,public ScoutBot
{
public:
    JumpSemiAutoScoutBot(const string &name, int x,int y)
    : Robot(name,x,y),
      GenericRobot(name,x,y),
      JumpSemiAutoBot(name,x,y),
      ScoutBot(name,x,y) {}

    void move() override{
        JumpBot::move();
    }

    void fire() override{
        SemiAutoBot::fire();
    }

    void think() override{
        JumpBot::think();
    }

    void act() override{
        ScoutBot::look(0,0);
        think();
        fire();
        move();
       
    }

    void look(int X,int Y) override{
        ScoutBot::look(X,Y);
    }

    bool isHit() override{
        return JumpBot::isHit();
    }

    void setBattlefield(Battlefield *bf){
        GenericRobot::setBattlefield(bf);
        JumpBot::setBattlefield(bf);
        SemiAutoBot::setBattlefield(bf);
        ScoutBot::setBattlefield(bf);
    }
    
};

//******************************************
//JumpThirtyShotScoutBot
//******************************************
class JumpThirtyShotScoutBot: public JumpThirtyShotBot,public ScoutBot
{
public:
    JumpThirtyShotScoutBot(const string &name, int x,int y)
    : Robot(name,x,y),
      GenericRobot(name,x,y),
      JumpThirtyShotBot(name,x,y),
      ScoutBot(name,x,y) {}

    void move() override{
        JumpBot::move();
    }

    void fire() override{
        ThirtyShotBot::fire();
    }

    void think() override{
        JumpBot::think();
    }

    void act() override{
        ScoutBot::look(0,0);
        think();
        fire();
        move();
       
    }

    void look(int X,int Y) override{
        ScoutBot::look(X,Y);
    }

    bool isHit() override{
        return JumpBot::isHit();
    }

    void setBattlefield(Battlefield *bf){
        GenericRobot::setBattlefield(bf);
        JumpBot::setBattlefield(bf);
        ThirtyShotBot::setBattlefield(bf);
        ScoutBot::setBattlefield(bf);
    }
    
};

//******************************************
//JumpKnightScoutBot
//******************************************
class JumpKnightScoutBot: public JumpKnightBot,public ScoutBot
{
public:
    JumpKnightScoutBot(const string &name, int x,int y)
    : Robot(name,x,y),
      GenericRobot(name,x,y),
      JumpKnightBot(name,x,y),
      ScoutBot(name,x,y) {}

    void move() override{
        JumpBot::move();
    }

    void fire() override{
        KnightBot::fire();
    }

    void think() override{
        JumpBot::think();
    }

    void act() override{
        ScoutBot::look(0,0);
        think();
        fire();
        move();
       
    }

    void look(int X,int Y) override{
        ScoutBot::look(X,Y);
    }

    bool isHit() override{
        return JumpBot::isHit();
    }

    void setBattlefield(Battlefield *bf){
        GenericRobot::setBattlefield(bf);
        JumpBot::setBattlefield(bf);
        KnightBot::setBattlefield(bf);
        ScoutBot::setBattlefield(bf);
    }
    
};


//******************************************
//JumpQueenScoutBot
//******************************************

class JumpQueenScoutBot: public JumpQueenBot,public ScoutBot
{
public:
    JumpQueenScoutBot(const string &name, int x,int y)
    : Robot(name,x,y),
      GenericRobot(name,x,y),
      JumpQueenBot(name,x,y),
      ScoutBot(name,x,y) {}

    void move() override{
        JumpBot::move();
    }

    void fire() override{
        QueenBot::fire();
    }

    void think() override{
        JumpBot::think();
    }

    void act() override{
        ScoutBot::look(0,0);
        think();
        fire();
        move();
       
    }

    void look(int X,int Y) override{
        ScoutBot::look(X,Y);
    }

    bool isHit() override{
        return JumpBot::isHit();
    }

    void setBattlefield(Battlefield *bf){
        GenericRobot::setBattlefield(bf);
        JumpBot::setBattlefield(bf);
        QueenBot::setBattlefield(bf);
        ScoutBot::setBattlefield(bf);
    }
    
};

//******************************************
//JumpVampireScoutBot
//******************************************
class JumpVampireScoutBot: public JumpVampireBot,public ScoutBot
{
public:
    JumpVampireScoutBot(const string &name, int x,int y)
    : Robot(name,x,y),
      GenericRobot(name,x,y),
      JumpVampireBot(name,x,y),
      ScoutBot(name,x,y) {}

    void move() override{
        JumpBot::move();
    }

    void fire() override{
        VampireBot::fire();
    }

    void think() override{
        JumpBot::think();
    }

    void act() override{
        ScoutBot::look(0,0);
        think();
        fire();
        move();
       
    }

    void look(int X,int Y) override{
        ScoutBot::look(X,Y);
    }

    bool isHit() override{
        return JumpBot::isHit();
    }

    void setBattlefield(Battlefield *bf){
        GenericRobot::setBattlefield(bf);
        JumpBot::setBattlefield(bf);
        VampireBot::setBattlefield(bf);
        ScoutBot::setBattlefield(bf);
    }
    
};

//******************************************
//JumpLongShotTrackBot
//******************************************

class JumpLongShotTrackBot: public JumpLongShotBot,public TrackBot
{
public:
    JumpLongShotTrackBot(const string &name, int x,int y)
    : Robot(name,x,y),
      GenericRobot(name,x,y),
      JumpLongShotBot(name,x,y),
      TrackBot(name,x,y) {}

    void move() override{
        JumpBot::move();
    }

    void fire() override{
        LongShotBot::fire();
    }

    void think() override{
        JumpBot::think();
    }

    void act() override{
        TrackBot::look(0,0);
        think();
        fire();
        move();
       
    }

    void look(int X,int Y) override{
        TrackBot::look(X,Y);
    }

    bool isHit() override{
        return JumpBot::isHit();
    }

    void setBattlefield(Battlefield *bf){
        GenericRobot::setBattlefield(bf);
        JumpBot::setBattlefield(bf);
        LongShotBot::setBattlefield(bf);
        TrackBot::setBattlefield(bf);
    }
    
};

//******************************************
//JumpSemiAutotrackBot
//******************************************

class JumpSemiAutotrackBot: public JumpSemiAutoBot,public TrackBot
{
public:
    JumpSemiAutotrackBot(const string &name, int x,int y)
    : Robot(name,x,y),
      GenericRobot(name,x,y),
      JumpSemiAutoBot(name,x,y),
      TrackBot(name,x,y) {}

    void move() override{
        JumpBot::move();
    }

    void fire() override{
        SemiAutoBot::fire();
    }

    void think() override{
        JumpBot::think();
    }

    void act() override{
        TrackBot::look(0,0);
        think();
        fire();
        move();
       
    }

    void look(int X,int Y) override{
        TrackBot::look(X,Y);
    }

    bool isHit() override{
        return JumpBot::isHit();
    }

    void setBattlefield(Battlefield *bf){
        GenericRobot::setBattlefield(bf);
        JumpBot::setBattlefield(bf);
        SemiAutoBot::setBattlefield(bf);
        TrackBot::setBattlefield(bf);
    }
    
};

//******************************************
//JumpThirtyShotTrackBot
//******************************************
class JumpThirtyShotTrackBot: public JumpThirtyShotBot,public TrackBot
{
public:
    JumpThirtyShotTrackBot(const string &name, int x,int y)
    : Robot(name,x,y),
      GenericRobot(name,x,y),
      JumpThirtyShotBot(name,x,y),
      TrackBot(name,x,y) {}

    void move() override{
        JumpBot::move();
    }

    void fire() override{
        ThirtyShotBot::fire();
    }

    void think() override{
        JumpBot::think();
    }

    void act() override{
        TrackBot::look(0,0);
        think();
        fire();
        move();
       
    }

    void look(int X,int Y) override{
        TrackBot::look(X,Y);
    }

    bool isHit() override{
        return JumpBot::isHit();
    }

    void setBattlefield(Battlefield *bf){
        GenericRobot::setBattlefield(bf);
        JumpBot::setBattlefield(bf);
        ThirtyShotBot::setBattlefield(bf);
        TrackBot::setBattlefield(bf);
    }
    
};

//******************************************
//JumpKnightTrackBot
//******************************************
class JumpKnightTrackBot: public JumpKnightBot,public TrackBot
{
public:
    JumpKnightTrackBot(const string &name, int x,int y)
    : Robot(name,x,y),
      GenericRobot(name,x,y),
      JumpKnightBot(name,x,y),
      TrackBot(name,x,y) {}

    void move() override{
        JumpBot::move();
    }

    void fire() override{
        KnightBot::fire();
    }

    void think() override{
        JumpBot::think();
    }

    void act() override{
        TrackBot::look(0,0);
        think();
        fire();
        move();
       
    }

    void look(int X,int Y) override{
        TrackBot::look(X,Y);
    }

    bool isHit() override{
        return JumpBot::isHit();
    }

    void setBattlefield(Battlefield *bf){
        GenericRobot::setBattlefield(bf);
        JumpBot::setBattlefield(bf);
        KnightBot::setBattlefield(bf);
        TrackBot::setBattlefield(bf);
    }
    
};
//******************************************
//JumpQueenTrackBot
//******************************************
class JumpQueenTrackBot: public JumpQueenBot,public TrackBot
{
public:
    JumpQueenTrackBot(const string &name, int x,int y)
    : Robot(name,x,y),
      GenericRobot(name,x,y),
      JumpQueenBot(name,x,y),
      TrackBot(name,x,y) {}

    void move() override{
        JumpBot::move();
    }

    void fire() override{
        QueenBot::fire();
    }

    void think() override{
        JumpBot::think();
    }

    void act() override{
        TrackBot::look(0,0);
        think();
        fire();
        move();
       
    }

    void look(int X,int Y) override{
        TrackBot::look(X,Y);
    }

    bool isHit() override{
        return JumpBot::isHit();
    }

    void setBattlefield(Battlefield *bf){
        GenericRobot::setBattlefield(bf);
        JumpBot::setBattlefield(bf);
        QueenBot::setBattlefield(bf);
        TrackBot::setBattlefield(bf);
    }
    
};

//******************************************
//JumpVampireTrackBot
//******************************************
class JumpVampireTrackBot: public JumpVampireBot,public TrackBot
{
public:
    JumpVampireTrackBot(const string &name, int x,int y)
    : Robot(name,x,y),
      GenericRobot(name,x,y),
      JumpVampireBot(name,x,y),
      TrackBot(name,x,y) {}

    void move() override{
        JumpBot::move();
    }

    void fire() override{
        VampireBot::fire();
    }

    void think() override{
        JumpBot::think();
    }

    void act() override{
        TrackBot::look(0,0);
        think();
        fire();
        move();
       
    }

    void look(int X,int Y) override{
        TrackBot::look(X,Y);
    }

    bool isHit() override{
        return JumpBot::isHit();
    }

    void setBattlefield(Battlefield *bf){
        GenericRobot::setBattlefield(bf);
        JumpBot::setBattlefield(bf);
        VampireBot::setBattlefield(bf);
        TrackBot::setBattlefield(bf);
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
            else if (type == "VampireBot")
                upgraded = new VampireBot(gen->getName(), gen->getX(), gen->getY());
            else if (type == "HideLongShotBot")
                upgraded = new HideLongShotBot(gen->getName(), gen->getX(), gen->getY());
            else if (type == "HideSemiAutoBot")
                upgraded = new HideSemiAutoBot(gen->getName(), gen->getX(), gen->getY());
            else if (type == "HideThirtyShotBot")
                upgraded = new HideThirtyShotBot(gen->getName(), gen->getX(), gen->getY());
            else if (type == "HideKnightBot")
                upgraded = new HideKnightBot(gen->getName(), gen->getX(), gen->getY());
            else if (type == "HideQueenBot")
                upgraded = new HideQueenBot(gen->getName(), gen->getX(), gen->getY());
            else if (type == "HideVampireBot")
                upgraded = new HideVampireBot(gen->getName(), gen->getX(), gen->getY());
            else if (type == "HideScoutBot")
                upgraded = new HideScoutBot(gen->getName(), gen->getX(), gen->getY());
            else if (type == "HideTrackBot")
                upgraded = new HideTrackBot(gen->getName(), gen->getX(), gen->getY());
            else if (type == "JumpLongShotBot")
                upgraded = new JumpLongShotBot(gen->getName(), gen->getX(), gen->getY());
            else if (type == "JumpSemiAutoBot")
                upgraded = new JumpSemiAutoBot(gen->getName(), gen->getX(), gen->getY());
            else if (type == "JumpThirtyShotBot")
                upgraded = new JumpThirtyShotBot(gen->getName(), gen->getX(), gen->getY());
            else if (type == "JumpKnightBot")
                upgraded = new JumpKnightBot(gen->getName(), gen->getX(), gen->getY());
            else if (type == "JumpQueenBot")
                upgraded = new JumpQueenBot(gen->getName(), gen->getX(), gen->getY());
            else if (type == "JumpVampireBot")
                upgraded = new JumpVampireBot(gen->getName(), gen->getX(), gen->getY());
            else if (type == "JumpScoutBot")
                upgraded = new JumpScoutBot(gen->getName(), gen->getX(), gen->getY());
            else if (type == "JumpTrackBot")
                upgraded = new JumpTrackBot(gen->getName(), gen->getX(), gen->getY());
            else if (type == "LongShotScoutBot")
                upgraded = new LongShotScoutBot(gen->getName(), gen->getX(), gen->getY());
            else if (type == "LongShotTrackBot")
                upgraded = new LongShotTrackBot(gen->getName(), gen->getX(), gen->getY());
            else if (type == "SemiAutoScoutBot")
                upgraded = new SemiAutoScoutBot(gen->getName(), gen->getX(), gen->getY());
            else if (type == "SemiAutoTrackBot")
                upgraded = new SemiAutoTrackBot(gen->getName(), gen->getX(), gen->getY());
            else if (type == "ThirtyShotScoutBot")
                upgraded = new ThirtyShotScoutBot(gen->getName(), gen->getX(), gen->getY());
            else if (type == "ThirtyShotTrackBot")
                upgraded = new ThirtyShotTrackBot(gen->getName(), gen->getX(), gen->getY());
            else if (type == "KnightScoutBot")
                upgraded = new KnightScoutBot(gen->getName(), gen->getX(), gen->getY());
            else if (type == "KnightTrackBot")
                upgraded = new KnightTrackBot(gen->getName(), gen->getX(), gen->getY());
            else if (type == "QueenScoutBot")
                upgraded = new QueenScoutBot(gen->getName(), gen->getX(), gen->getY());
            else if (type == "QueenTrackBot")
                upgraded = new QueenTrackBot(gen->getName(), gen->getX(), gen->getY());
            else if (type == "VampireScoutBot")
                upgraded = new VampireScoutBot(gen->getName(), gen->getX(), gen->getY());
            else if (type == "VampireTrackBot")
                upgraded = new VampireTrackBot(gen->getName(), gen->getX(), gen->getY());

            if (upgraded)
            {
                upgraded->setLives(gen->getLives());
                GenericRobot *upGen = dynamic_cast<GenericRobot *>(upgraded);
                if (upGen)
                {
                    upGen->clearPendingUpgrade();
                    upGen->setBattlefield(this); // Ensure battlefield context is set
                    upGen->resetActionFlags();
                }
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
            gen->resetActionFlags();
        }
        // Skip robot's turn if it is hurt (was hit this turn)
        if (robot->getIsHurt())
        {
            logger << "-----------------------------------" << endl;
            logger << robot->getName() << " was hit and skips this turn." << endl;
            continue;
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
        string nameOfRobotToRespawn = respawnInfo.first;
        int livesLeft = respawnInfo.second;
        reentryQueue.pop();
        logger << "Reentering " << nameOfRobotToRespawn << endl;
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
                       logger << livesLeft << " lives" << " next turn." << endl;
        }
        else
        {
            logger << "No available spot for reentry for " << nameOfRobotToRespawn << ". Will try again next turn." << endl;
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
        if (robot->getLives() <= 0 || robot->getIsDie() || robot->getIsHurt())
        {
            removeRobotFromGrid(robot);
            logger << robot->getName() << " has been removed from the battlefield." << endl;
            if (robot->getIsHurt() && !robot->getIsDie())
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
    else if (tokens[0] == "VampireBot" && tokens.size() >= 4)
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

        Robot *newRobot = new VampireBot(robotName, robotXCoordinates, robotYCoordinates);
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
            GenericRobot *gen = dynamic_cast<GenericRobot *>(robot);

            if (gen)
            {
                if (gen->PendingUpgrade())
                {
                    type = gen->getUpgradeType();
                }
                else
                {
                    if (dynamic_cast<HideBot *>(robot))
                        type = "HideBot";
                    else if (dynamic_cast<JumpBot *>(robot))
                        type = "JumpBot";
                    else if (dynamic_cast<LongShotBot *>(robot))
                        type = "LongShotBot";
                    else if (dynamic_cast<KnightBot *>(robot))
                        type = "KnightBot";
                    else if (dynamic_cast<SemiAutoBot *>(robot))
                        type = "SemiAutoBot";
                    else if (dynamic_cast<ThirtyShotBot *>(robot))
                        type = "ThirtyShotBot";
                    else if (dynamic_cast<ScoutBot *>(robot))
                        type = "ScoutBot";
                    else if (dynamic_cast<TrackBot *>(robot))
                        type = "TrackBot";
                    else if (dynamic_cast<QueenBot *>(robot))
                        type = "QueenBot";
                    else if (dynamic_cast<VampireBot *>(robot))
                        type = "VampireBot";
                    else if (dynamic_cast<HideLongShotBot *>(robot))
                        type = "HideLongShotBot";
                    else if (dynamic_cast<HideSemiAutoBot *>(robot))
                        type = "HideSemiAutoBot";
                    else if (dynamic_cast<HideThirtyShotBot *>(robot))
                        type = "HideThirtyShotBot";
                    else if (dynamic_cast<HideKnightBot *>(robot))
                        type = "HideKnightBot";
                    else if (dynamic_cast<HideQueenBot *>(robot))
                        type = "HideQueenBot";
                    else if (dynamic_cast<HideVampireBot *>(robot))
                        type = "HideVampireBot";
                    else if (dynamic_cast<HideScoutBot *>(robot))
                        type = "HideScoutBot";
                    else if (dynamic_cast<HideTrackBot *>(robot))
                        type = "HideTrackBot";
                    else if (dynamic_cast<JumpLongShotBot *>(robot))
                        type = "JumpLongShotBot";
                    else if (dynamic_cast<JumpSemiAutoBot *>(robot))
                        type = "JumpSemiAutoBot";
                    else if (dynamic_cast<JumpThirtyShotBot *>(robot))
                        type = "JumpThirtyShotBot";
                    else if (dynamic_cast<JumpKnightBot *>(robot))
                        type = "JumpKnightBot";
                    else if (dynamic_cast<JumpQueenBot *>(robot))
                        type = "JumpQueenBot";
                    else if (dynamic_cast<JumpVampireBot *>(robot))
                        type = "JumpVampireBot";
                    else if (dynamic_cast<JumpScoutBot *>(robot))
                        type = "JumpScoutBot";
                    else if (dynamic_cast<JumpTrackBot *>(robot))
                        type = "JumpTrackBot";
                    else if (dynamic_cast<LongShotScoutBot *>(robot))
                        type = "LongShotScoutBot";
                    else if (dynamic_cast<LongShotTrackBot *>(robot))
                        type = "LongShotTrackBot";
                    else if (dynamic_cast<SemiAutoScoutBot *>(robot))
                        type = "SemiAutoScoutBot";
                    else if (dynamic_cast<SemiAutoTrackBot *>(robot))
                        type = "SemiAutoTrackBot";
                    else if (dynamic_cast<ThirtyShotScoutBot *>(robot))
                        type = "ThirtyShotScoutBot";
                    else if (dynamic_cast<ThirtyShotTrackBot *>(robot))
                        type = "ThirtyShotTrackBot";
                    else if (dynamic_cast<QueenScoutBot *>(robot))
                        type = "QueenScoutBot";
                    else if (dynamic_cast<QueenTrackBot *>(robot))
                        type = "QueenTrackBot";
                    else if (dynamic_cast<VampireScoutBot *>(robot))
                        type = "VampireScoutBot";
                    else if (dynamic_cast<VampireTrackBot *>(robot))
                        type = "VampireTrackBot";
                    else if (dynamic_cast<KnightScoutBot *>(robot))
                        type = "KnightScoutBot";
                    else if (dynamic_cast<KnightTrackBot *>(robot))
                        type = "KnightTrackBot";
                    else if (dynamic_cast<GenericRobot *>(robot))
                         type = "GenericRobot";
                    else
                        type = "UnknownType"; // Fallback for any unrecognized types
                }
            }
            else
            {
                type = "Robot";
            }
            // TO DO :able to display current robot type
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