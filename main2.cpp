/**********|**********|**********|
Program: main.cpp
Course: OOPDS
Trimester: 2510
Lecture Section: TC1L
Tutorial Section: TT2L

Member 1
Name: CHOW YING TONG
ID: 242UC244NK
Email: chow.ying.tong@student.mmu.edu.my
Phone: 016-576 7692

Member 2
Name: MUI RUI XIN
ID:243UC247CT
Email: mui.rui.xin@student.mmu.edu.my
Phone: 016-614 4391

Member 3
Name:LAW CHIN XUAN
ID:242UC244GC
Email: law.chin.xuan@student.mmu.edu.my
Phone: 011-1098 8658

**********|**********|**********/


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
#include <set>    //GG

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
    struct reentryData {
        string name;
        int lives;
        int ammo;
    };
    queue<reentryData> reentryQueue; // Queue for reentry
    //GG
    std::set<Robot*> queuedThisRound; // Track robots queued for reentry this round

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

    void setSteps(int s)
    {
        steps = s;
    }

    int getSteps() const
    {
        return steps;
    }

    void setNumberOfRobots(int n)
    {
        numberOfRobots = n;
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
    //GG
    Battlefield* battlefield = nullptr; // Add pointer to Battlefield

protected:
    string name;
    int lives;
    bool hidden;
    bool isDie = false;  // true if robot is out of the game (no lives or no ammo)
    bool isHurt = false; // true if robot is hit this turn and should be requeued
    virtual bool canBeHit() = 0;
    bool getEnemyDetectedNearby() const;

public:
    Robot(string name, int x, int y)
        : name(name), positionX(x), positionY(y), lives(3), hidden(false) {}

    virtual ~Robot() = default;
    //GG

    void setBattlefield(Battlefield* bf) { battlefield = bf; } // Setter
    Battlefield* getBattlefield() const { return battlefield; }

    virtual void initializeFrom(const Robot *oldRobot) = 0;
    virtual void think() = 0;
    virtual void act() = 0;
    virtual void move() = 0;
    virtual void fire(int X, int Y) = 0;
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
            //GG
                isHurt = true; // Mark as hurt for requeue
                if (battlefield) battlefield->queueForReentry(this); // Immediately queue for reentry
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
    virtual void fire(int X, int Y) = 0;
    bool hasAmmo() const { return ammo > 0; }

    void useAmmo()
    {
        if (ammo > 0)
            ammo--;
    }

    int getAmmo() const { return ammo; }
    void setAmmo(int num) {
        ammo = num;
    }

    // Stimulate and check hit probablity is 70 percent
    bool hitProbability() const
    {
        static random_device rd;                   // True random number generator
        static mt19937 gen(rd());                  // Mersenne Twister pseudo-random generator seeded with rd
        uniform_real_distribution<> dis(0.0, 1.0); // Distribution for numbers between 0.0 and 1.0
        return dis(gen) <= 0.7;                    // 70% hit chance
    }

    void initializeFrom(const Robot *oldRobot) override
    {
        const ShootingRobot *oldShootingBot = dynamic_cast<const ShootingRobot *>(oldRobot);
        if (oldShootingBot)
        {
            this->setAmmo(oldShootingBot->getAmmo()); // Preserves standard ammo
        }
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

    virtual const vector<string> &getUpgradeTypes() const
    {
        static const vector<string> defaultTypes = {"HideBot", "JumpBot", "LongShotBot", "SemiAutoBot", "ThirtyShotBot", "KnightBot", "QueenBot", "VampireBot", "ScoutBot", "TrackBot"};
        return defaultTypes;
    };

    // Action flags for per-round limitation
    bool hasLooked = false;
    bool hasMoved = false;
    bool hasThought = false;
    bool hasFired = false;

public:
    GenericRobot(const string &name, int x, int y);
    virtual ~GenericRobot() override;
    void setBattlefield(Battlefield *bf);
    void think() override;
    void act() override;
    void move() override;
    void fire(int X, int Y) override;
    void look(int X, int Y) override;
    bool canUpgrade(int area) const;
    void setUpgraded(int area);
    bool canBeHit() override;
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
        fire(0, 0);
        move();
    }
    else
    {
        move();
        fire(0, 0);
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

void GenericRobot::fire(int X, int Y)
{
    if (hasFired)
        return;
    hasFired = true;
    if (hasAmmo())
    {
        // Filter detectedTargets to only include robots that are hittable (canBeHit() == true)
        vector<Robot *> validTargets;
        for (Robot *r : detectedTargets)
        {
            if ((r && r != this) && !isHurt)
            {
                GenericRobot *gtarget = dynamic_cast<GenericRobot *>(r);
                if (gtarget && gtarget->canBeHit())
                {
                    validTargets.push_back(r);
                }
            }
        }
        if (!validTargets.empty())
        {
            int idx = rand() % validTargets.size();
            Robot *target = validTargets[idx];
            int targetX = target->getX() + X;
            int targetY = target->getY() + Y;
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
                vector<string> upgradeTypes = getUpgradeTypes();
                if (!upgradeTypes.empty())
                {
                    int t = rand() % upgradeTypes.size();
                    setPendingUpgrade(upgradeTypes[t]);
                    logger << name << " will upgrade into " << upgradeTypes[t] << " next turn!" << endl;
                }
            }
            else
            {
                logger << "Missed!" << endl;
            }

            if (!hasAmmo())
            {
                logger << getName() << " has no ammo left, it will self-destruct!" << endl;
                lives = 0;
                isDie = true;
            }
        }
        else
        {
            logger << "No shooting as no robots within shooting range ." << endl;
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

bool GenericRobot::canBeHit()
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

protected:
    const vector<string> &getUpgradeTypes() const override
    {
        static const vector<string> upgradeTypes = {"HideLongShotBot", "HideSemiAutoBot", "HideThirtyShotBot", "HideKnightBot", "HideQueenBot", "HideVampireBot", "HideScoutBot", "HideTrackBot"};
        return upgradeTypes;
    }

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
        fire(0, 0);
        move();
    }

    bool canBeHit() override
    {
        return !isHidden;
    }
};

//******************************************
// LongShotBot
//******************************************

class LongShotBot : public virtual GenericRobot
{
private:
    int fire_count = 0;

protected:
    const vector<string> &getUpgradeTypes() const override
    {
        static const vector<string> upgradeTypes = {"HideLongShotBot", "JumpLongShotBot", "LongShotScoutBot", "LongShotTrackBot"};
        return upgradeTypes;
    }

public:
    LongShotBot(const string &name, int x, int y)
        : Robot(name, x, y),
          GenericRobot(name, x, y) {}

    void fire(int X, int Y) override
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

                if (target && target != this&& !isHurt)
                {
                    GenericRobot *gtarget = dynamic_cast<GenericRobot *>(target);
                    logger << ">> " << getName() << " firesssss at (" << targetX << "," << targetY << ")" << endl;
                    useAmmo();

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

                        const vector<string> &upgradeTypes = getUpgradeTypes();

                        if (!upgradeTypes.empty())
                        {
                            int t = rand() % upgradeTypes.size();
                            string newType = upgradeTypes[t];
                            setPendingUpgrade(newType);
                            logger << getName() << " will upgrade into " << newType << " next turn" << endl;
                        }
                    }
                    else
                    {
                        logger << "Missed!" << endl;
                        fired = true;
                    }

                    if (!hasAmmo())
                    {
                        logger << getName() << " has no ammo left, it will self-destruct!" << endl;
                        lives = 0;
                        isDie = true;
                    }

                    break;
                }
            }
            if (fired)
                break;
        }

        if (!fired)
        {
            logger << "No shooting as no robots within shooting range . " << endl;
        }
    }
};

//******************************************
// HideLongShotBot (inherits from HideBot and LongShotBot)
//******************************************
class HideLongShotBot : public HideBot, public LongShotBot
{
protected:
    const vector<string> &getUpgradeTypes() const override
    {
        static const vector<string> upgradeTypes = {"HideLongShotScoutBot", "HideLongShotTrackBot"};
        return upgradeTypes;
    }

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

    void fire(int X, int Y) override
    {
        LongShotBot::fire(X, Y);
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
        fire(0, 0);
    }

    void look(int X, int Y) override
    {
        LongShotBot::look(X, Y); // Use long-range look for any explicit look calls
    }

    bool canBeHit() override
    {
        return HideBot::canBeHit();
    }

    void setBattlefield(Battlefield *bf)
    {
        GenericRobot::setBattlefield(bf);
    }
};

//******************************************
// simulationStep member function of Battlefield class (declared later to avoid issues with code not seeing each other when they need to)
//******************************************

void Battlefield::simulationStep()
{
    //GG
    // Clear queuedThisRound at the start of each round
    queuedThisRound.clear();

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
            else if (type == "LongShotBot")
                upgraded = new LongShotBot(gen->getName(), gen->getX(), gen->getY());
            
            else if (type == "HideLongShotBot")
                upgraded = new HideLongShotBot(gen->getName(), gen->getX(), gen->getY());
            if (upgraded)
            {
                upgraded->setLives(gen->getLives());
                upgraded->initializeFrom(gen);
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
    //GG
    robot->setBattlefield(this); // Set battlefield pointer
    if (respawnCounts.find(robot->getName()) == respawnCounts.end())
    {
        respawnCounts[robot->getName()] = 3; //limit respawn count to 3
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
    //GG
    if (queuedThisRound.find(robot) != queuedThisRound.end()) return;
    queuedThisRound.insert(robot);
    reentryQueue.push({robot->getName(), robot->getLives()});
    logger << robot->getName() << " queued for reentry with " << robot->getLives()
           << " lives and " << endl;
}

// PARTIALLY COMPLETED: respawnRobots member function definition
void Battlefield::respawnRobots()
{
    if (!reentryQueue.empty())
    {
        auto respawnInfo = reentryQueue.front();
        string nameOfRobotToRespawn = respawnInfo.name;
        int livesLeft = respawnInfo.lives;
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
            //GG
            newRobot->setBattlefield(this); // Set battlefield pointer
            ShootingRobot* newShooter = dynamic_cast<ShootingRobot*>(newRobot);
            if (newShooter) {
                newShooter->setAmmo(respawnInfo.ammo); // Set the preserved ammo
            }
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
            //GG
            // No need to queueForReentry here anymore
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
    logger << "Width: " << battlefield.getWidth() << endl;
    logger << "Height: " << battlefield.getHeight() << endl;

    logger << "Battlefield steps: " << battlefield.getSteps() << endl;

    logger << "Battlefield number of robots: " << battlefield.getNumberOfRobots() << endl;

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

                    if (dynamic_cast<HideLongShotBot *>(robot))
                        type = "HideLongShotBot";

                    else if (dynamic_cast<HideBot *>(robot))
                        type = "HideBot";
                    else if (dynamic_cast<LongShotBot *>(robot))
                        type = "LongShotBot";

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
                   << ", Life: " << robot->getLives();

            logger << endl;
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