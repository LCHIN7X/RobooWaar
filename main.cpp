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

// Headers
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
#include <set>

using namespace std;

//  Forward declarations
class Robot;

//**********************************************************
// Logger class that logs all activity (cout and output to file) 
//(Implement Overloading of << operator)
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
        // call open() to open input file
        open();
    };

    void open()
    {
        outputFile.open(fileName);
    }

    void close()
    {
        if (outputFile.is_open())
        {
            outputFile.close(); // close output file if it is open
        }
    }

    // Operator Overloading
    //Template function to accept generic data types
    // overload << operator to cout and write to output file
    template <typename T>
    Logger &operator<<(const T &outputContent)
    {
        cout << outputContent;
        outputFile << outputContent;
        return *this;
    }

    // overload << operator to accept stream manipulators (for endl 
    //as endl not a regular data type, but a function pointer)
    Logger &operator<<(ostream &(*manip)(ostream &))
    {
        manip(cout);
        manip(outputFile);
        return *this;
    }

    // Destructor
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
    map<string, int> respawnCounts;
    struct reentryData
    {
        string name;
        int lives;
        int ammo;
    };
    queue<reentryData> reentryQueue; // Queue FIFO for reentry
    set<Robot *> queuedThisRound;    // Track robots queued for reentry this round

public:
    Battlefield() : height(0), width(0), steps(0), numberOfRobots(0) {}; // Constructor, initialize height, width, steps, numberOfRobots to 0
    ~Battlefield();                                                      // Destructor declaration

    // function to set dimension of battlefield
    void setDimensions(int h, int w)
    {
        height = h;
        width = w;
        battlefieldGrid.assign(height, vector<Robot *>(width, nullptr)); // assign(count, value), count = height number of rows, width number of columns, initialize all values in the row to nullptr
    }

    //set simulation steps
    void setSteps(int s)
    {
        steps = s;
    }

    //get simulation steps
    int getSteps() const
    {
        return steps;
    }

    //set number of robot inside simulation
    void setNumberOfRobots(int n)
    {
        numberOfRobots = n;
    }

    //get number robot in simualtion
    int getNumberOfRobots() const
    {
        return numberOfRobots;
    }

    // get list of all robots
    const vector<Robot *> getListOfRobots() const
    {
        return listOfRobots;
    }

    //get width and height battlefield
    int getWidth() const { return width; }
    int getHeight() const { return height; }

    // Function prototypes (definitions are after Robot class)
    void simulationStep(int);                   //perform simulation step
    void addNewRobot(Robot *robot);             //add new robot in battlefield
    int getNumberOfAliveRobots();               //count robot alive number
    void cleanupDestroyedRobots();              //remove destroy robot
    void respawnRobots();                       //respawn hurt robot
    void queueForReentry(Robot *robot);         //robot queue for reentry

    //battlefield function
    Robot *getRobotAt(int x, int y) const;          //get robot location
    void placeRobot(Robot *robot, int x, int y);    //put robot to that location
    void removeRobotFromGrid(Robot *robot);         // remove robot from battlefield
    bool isPositionAvailable(int x, int y);         //check the position have robot or not
    bool isPositionWithinGrid(int x, int y) const;  //check the position valid or not

    void displayBattlefield();                      //display battlefield
};

//******************************************
// Abstract base Robot class
//Implemented as an abstract class with pure virtual functions
//******************************************

class Robot
{
private:
    int positionX;                      //location x
    int positionY;                      //location y
    Battlefield *battlefield = nullptr; // Add pointer to Battlefield

protected:
    string name;                        //robot name
    int lives;                          //number robot lives
    bool hidden;                        //robot hide or not
    bool isDie = false;                 // true if robot is out of the game (no lives or no ammo)
    bool isHurt = false;                // true if robot is hit this turn and should be requeued
    virtual bool canBeHit() = 0;        //robot can be hit or not in thier status
    bool getEnemyDetectedNearby() const;//check got other robot nearby or not

public:
    // Constructor, set name, position, lives and hidden state to specified values
    Robot(string name, int x, int y)
        : name(name), positionX(x), positionY(y), lives(3), hidden(false) {}

    virtual ~Robot() = default; // Virtual destructor

    void setBattlefield(Battlefield *bf) { battlefield = bf; }  // Setter assigns the robot to a specific battlefield.
    Battlefield *getBattlefield() const { return battlefield; } // get battlefield context
                                                                //Returns the pointer to the battlefield the robot is currently in.

    virtual void initializeFrom(const Robot *oldRobot) = 0; //  is here to be overridden by ThirtyShotBot 
                                                             // to preserve its ammo count when upgrading from a ShootingRobot.
    virtual void think() = 0;                               // pure virtual think function to be overriden in derived classes
    virtual void act() = 0;                                 // pure virtual act function to be overriden in derived classes
    virtual void move() = 0;                                // pure virtual move function to be overriden in derived classes
    virtual void fire(int X, int Y) = 0;                    // pure virtual fire function to be overridden in derived classes
    virtual void look(int X, int Y) = 0;                    // pure virtual look function to be overridden in derived classes

    // Common robot functions
    string getName() const { return name; }                 //get robot name
    int getX() const { return positionX; }                  //get robot x location
    int getY() const { return positionY; }                  //get robot y location
    int getLives() const { return lives; }                  //get robot live left
    void setLives(int numOfLives) { lives = numOfLives; }   //set robot live
    bool isHidden() const { return hidden; }                //check robot hide or not
    bool getIsDie() const { return isDie; }                 //check robot die or not
    void setIsDie(bool val) { isDie = val; }                //set robot die (life=0 or no ammo)
    bool getIsHurt() const { return isHurt; }               //check when robot is hurt (being hit)
    void setIsHurt(bool val) { isHurt = val; }              // set robot hurt

    void setPosition(int x, int y)  //set robot position
    {
        positionX = x;
        positionY = y;
    }

    void takeDamage()               //damage take by robot
    {
        if (!hidden) //if robot not hide will damage
        {
            lives--; // decrement lives by 1
            if (lives <= 0)
            {
                logger << name << " has been destroyed!\n";
                isDie = true; // Mark as dead, permanently destroyed
            }
            else{
            isHurt = true; // Mark as hurt for requeue
            if (battlefield) battlefield->queueForReentry(this); }// Immediately queue for reentry
        }
    }

    void setHidden(bool state) { hidden = state; } //set robot hide state
};

//******************************************
// MovingRobot class
// Implementation of Inheritance
//*******************************************
class MovingRobot : public virtual Robot
{
protected:
    int moveCount; //number of move

public:
    // Constructor
    MovingRobot(const string &name, int x, int y)
        : Robot(name, x, y), moveCount(0) {}

    virtual ~MovingRobot() = default; // Virtual destructor
    virtual void move() = 0;          // pure virtual move function to be overridden as involve the battlefiled

    bool isValidMove(int newX, int newY, const Battlefield &battlefield) const //check when the move is valid
    {
        // Only allow moving to adjacent cell (1 step in any direction)
        int dx = abs(newX - getX());
        int dy = abs(newY - getY());

        bool oneStep = (dx + dy == 1); // Manhattan distance of 1 (no diagonal)
        return oneStep && newX >= 0 && newX < battlefield.getWidth() &&
               newY >= 0 && newY < battlefield.getHeight();
    }

    void incrementMoveCount() { moveCount++; } // increase move count
};

//******************************************
// Shooting Robot class
//******************************************
class ShootingRobot : public virtual Robot
{
protected:
    int ammo;  //number of ammo left

public:
    // Constructor
    ShootingRobot(const string &name, int x, int y, int initialAmmo)
        : Robot(name, x, y), ammo(initialAmmo) {}

    virtual ~ShootingRobot() = default;  // virtual destructor
    virtual void fire(int X, int Y) = 0; // pure virtual fire() method to be overridden as involve the battlefiled
    bool hasAmmo() const { return ammo > 0; }

    void useAmmo() //use one ammo
    {
        if (ammo > 0)
            ammo--; // decrements ammo by 1
    }

    int getAmmo() const { return ammo; } //get ammo count
    void setAmmo(int num) 
    {
        ammo = num;
    }

    // Simulate hit probablity of 70 percent
    bool hitProbability() const
    {
        static random_device rd;                   // True random number generator (random device and mt19937)
        static mt19937 gen(rd());                  // Mersenne Twister pseudo-random generator seeded with rd
        uniform_real_distribution<> dis(0.0, 1.0); // Distribution for numbers between 0.0 and 1.0
        return dis(gen) <= 0.7;                    // 70% hit chance
    }

    // Preserve ammo after upgrades (specifically used for ThirtyShotBot)
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
    int visionRange;                     //how far robot can see
    vector<Robot *> detectedTargets;     //list robot been detect
    bool enemyDetectedNearby;            //flag robot nearby

public:
    // Constructor
    SeeingRobot(const string &name, int x, int y, int range)
        : Robot(name, x, y), visionRange(range) {}

    virtual ~SeeingRobot() = default;    // virtual destructor
    virtual void look(int X, int Y) = 0; // pure virtual look method to be overridden as involve the battlefiled

    const vector<Robot *> &getDetectedTargets() const { return detectedTargets; } //get detact robot

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
    int strategyLevel;   //number of strategy level 

public:
    // Constructor
    ThinkingRobot(const string &name, int x, int y, int strategy)
        : Robot(name, x, y), strategyLevel(strategy) {}

    virtual ~ThinkingRobot() = default; // virtual destructor
    void think() override  //override think（） method
    {
        logger << ">> " << name << " is thinking..." << endl;
    }

    int getStrategyLevel() const { return strategyLevel; }  //get strategy level
};

//******************************************
// GenericRobot class
// inherits from MovingRobot, ShootingRobot, SeeingRobot, ThinkingRobot
//Implementation of Multiple Inheritance
//******************************************

class GenericRobot : public MovingRobot, public ShootingRobot, public SeeingRobot, public ThinkingRobot
{
protected:
    bool hasUpgraded[3] = {false, false, false}; // Track upgrades for moving, shooting, seeing
    Battlefield *battlefield = nullptr;          // Pointer to current battlefield
    bool pendingUpgrade = false;                 // flag to indicate if it has an upgrade to be performed
    string upgradeType = "";                     // to store upgrade type
    bool enemyDetectedNearby = false;            // Flag for detecting nearby enemies
    vector<Robot *> detectedTargets;             // to store detected enemies/targets
    vector<pair<int, int>> availableSpaces;      //to store available spaces for movement

    // virtual getUpgradeTypes method that returns the possible upgrade combinations at each stage for each robot (overridden by all robots later on)
    virtual const vector<string> &getUpgradeTypes() const
    {
        static const vector<string> defaultTypes = {"HideBot", "JumpBot", "LongShotBot", "SemiAutoBot", "ThirtyShotBot", "KnightBot", "QueenBot", "VampireBot", "ScoutBot", "TrackBot"};
        return defaultTypes;
    };

    // Action flags for per-round limitation (limit each action to once per round)
    bool hasLooked = false;
    bool hasMoved = false;
    bool hasThought = false;
    bool hasFired = false;

public:
    GenericRobot(const string &name, int x, int y);   //constructor genenric robot
    virtual ~GenericRobot() override;                 //destructor genenric robot
    void setBattlefield(Battlefield *bf);             //set battlefield for robot
    void think() override;                            //robot think function
    void act() override;                              //robot action 
    void move() override;                             //robot move
    void fire(int X, int Y) override;                 //robot fire 
    void look(int X, int Y) override;                 //robot detect other robot
    bool canUpgrade(int area) const;                  //check can upgrade or not
    void setUpgraded(int area);                       //set robot as upgrade robot
    bool canBeHit() override;                         //check robot can hit or not
    void setPendingUpgrade(const string &type);       //set pending upgrade type
    bool PendingUpgrade() const;                      //robot upgrade is pending or not
    string getUpgradeType() const;                    //get robot pending upgreade type
    void clearPendingUpgrade();                       //clear robot pending upgrade
    bool getEnemyDetectedNearby() const;              //get robot nearby

    // Reset all action flags (call at start/end of each round)
    void resetActionFlags()
    {
        hasLooked = false;
        hasMoved = false;
        hasThought = false;
        hasFired = false;
    }
};

// Constructor for GenericRobot
GenericRobot::GenericRobot(const string &name, int x, int y)
    : Robot(name, x, y),                //base robot
      MovingRobot(name, x, y),          //moving robot with limited 1 space can move
      ShootingRobot(name, x, y, 10),    //shooting robot with 10 ammo
      SeeingRobot(name, x, y, 1),       //seeing robot with vision range 1, detected targets
      ThinkingRobot(name, x, y, 1)      //thinking robot 
{
}

GenericRobot::~GenericRobot() = default; // virtual GenericRobot destructor

void GenericRobot::setBattlefield(Battlefield *bf) { battlefield = bf; } //set battlefield 

void GenericRobot::think() // override think()
{
    if (hasThought)
        return;
    hasThought = true;    //prevent multiple thinking in one round
    logger << ">> " << name << " is thinking...\n";
    if (getEnemyDetectedNearby())  //if detact other robot nearby,attack then move
    {
        fire(0, 0);
        move();
    }
    else //else move then attack
    {
        move();
        fire(0, 0);
    }
}
void GenericRobot::act() // when it's a robot's turn, this function is called
{
    if (isDie || isHurt || getLives() <= 0)
        return; // If robot is dead or hurt or lives<=0, skip action
    if (!battlefield) //if battlefield is not set, skip action
    {
        logger << name << " has no battlefield context!" << endl;
        return;
    }
    look(0, 0); //look first to detect other robot, then decide action
    think();    //dedice action based on detected targets
                //Action invloved move, fire
    detectedTargets.clear(); //clear the target robot per round
}

void GenericRobot::move() // override move() method
{
    if (hasMoved)  //prevent multiple move in one round
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
        static random_device rd;  //choose random number
        static mt19937 g(rd());
        uniform_int_distribution<> dist(0, availableSpaces.size() - 1);
        auto [newX, newY] = availableSpaces[dist(g)];  //get random location for move
        battlefield->removeRobotFromGrid(this);     //remove robot from current position
        battlefield->placeRobot(this, newX, newY);  //place robot to new position
        incrementMoveCount();
        logger << name << " moved to (" << newX << ", " << newY << ")\n";
        return;
    }

    logger << name << " could not move (no available adjacent cell).\n";
}

void GenericRobot::fire(int X, int Y) // override fire() method
{
    if (hasFired) //prevent multiple fire in one round
        return;
    hasFired = true;
    if (hasAmmo())
    {
        // Filter detectedTargets to only include robots that are hittable (canBeHit() == true)
        vector<Robot *> validTargets;
        for (Robot *r : detectedTargets)
        {
            if ((r && r != this) && !r->getIsHurt())
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
            int idx = rand() % validTargets.size(); //rondom choose robot as target
            Robot *target = validTargets[idx];
            int targetX = target->getX() + X;
            int targetY = target->getY() + Y;
            logger << ">> " << name << " fires at (" << targetX << ", " << targetY << ")" << endl;
            useAmmo(); //use one ammo
            if (target->isHidden()) //after hit result
            {
                logger << target->getName() << " is hidden, attack miss." << endl;
            }
            else if (hitProbability())  //0.7 chance hit
            {
                logger << "Hit! (" << target->getName() << ") is killed!" << endl;
                target->takeDamage();
                vector<string> upgradeTypes = getUpgradeTypes();  //random choose upgrade type
                if (!upgradeTypes.empty())
                {
                    int t = rand() % upgradeTypes.size();
                    setPendingUpgrade(upgradeTypes[t]);
                    logger << name << " will upgrade into " << upgradeTypes[t] << " next turn!" << endl;
                }
            }
            else
            {
                logger << getName() << " missed!" << endl;
            }
        }
        else
        {
            logger << "No shooting as no robots within shooting range." << endl;
        }
    }
    else
    {
        logger << name << " has no ammo left. It will self destruct!" << endl;
        lives = 0;
        isDie = true;
    }
    detectedTargets.clear();
}

void GenericRobot::look(int X, int Y) // override look() method
{
    if (!battlefield)
    {
        logger << name << " has no battlefield context!" << endl;
        return;
    }
    enemyDetectedNearby = false;
    availableSpaces.clear(); //ensure availableapces is empty before looking
    detectedTargets.clear(); //clear detected targets before looking

    int centerX = getX() + X;
    int centerY = getY() + Y;

    // Scan 3x3 area centered on robot's current position
    for (int dx = -1; dx <= 1; ++dx)  //-1,0,1
    {
        for (int dy = -1; dy <= 1; ++dy) //-1,0,1
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
            if (lookX == getX() && lookY == getY()) //current position
            {
                logger << "Current position" << endl;
                continue;
            }
            if (occupant == nullptr) //empty space
            {
                logger << "Empty space" << endl;
                availableSpaces.emplace_back(lookX, lookY);
            }
            else if (occupant != this && !occupant->getIsDie() && !occupant->getIsHurt()) //get robot that are other (not itself),not die,not hurt
            {
                logger << "Enemy " << occupant->getName() << endl;
                enemyDetectedNearby = true;
                if (find(detectedTargets.begin(), detectedTargets.end(), occupant) == detectedTargets.end())
                {
                    detectedTargets.push_back(occupant);
                }
            }
            else //dead robot or hurt robot
            {
                logger << "Dead Robot" << endl;
            }
        }
    }
}

//upgrade area, 0(moving), 1(shooting), 2(seeing)

bool GenericRobot::canUpgrade(int area) const //check robot can upgrade or not
{
    if (area < 0 || area > 2) 
        return false;
    return !hasUpgraded[area];
}

void GenericRobot::setUpgraded(int area)  //set robot as upgraded in specific area
{
    if (area >= 0 && area < 3)
    {
        hasUpgraded[area] = true;
    }
}

bool GenericRobot::canBeHit() //set robot can be hit or not
{
    return true;
}

void GenericRobot::setPendingUpgrade(const string &type) //set robot pending upgrade is pending and its type
{
    pendingUpgrade = true;
    upgradeType = type;
}
bool GenericRobot::PendingUpgrade() const { return pendingUpgrade; } //upgrade pending or not
string GenericRobot::getUpgradeType() const { return upgradeType; }  //get robot pending upgrade type
void GenericRobot::clearPendingUpgrade()      //clear robot pending upgrade
{
    pendingUpgrade = false;
    upgradeType = "";
}
bool GenericRobot::getEnemyDetectedNearby() const { return enemyDetectedNearby; }  //check if got robot nearby

//******************************************
// HideBot
//******************************************

class HideBot : public virtual GenericRobot
{

private:
    int hideCount = 0;      //number of hide robot use, limit to 3
    bool isHidden = false;  //robot hide status

protected:
    // override getUpgradeTypes method from GenericRobot to include its own upgrade areas
    const vector<string> &getUpgradeTypes() const override
    {
        static const vector<string> upgradeTypes = {"HideLongShotBot", "HideSemiAutoBot", "HideThirtyShotBot", "HideKnightBot", "HideQueenBot", "HideVampireBot", "HideScoutBot", "HideTrackBot"};
        return upgradeTypes;
    }

public:
    //constructor
    HideBot(const string &name, int x, int y)
        : Robot(name, x, y),
          GenericRobot(name, x, y) {}

    void move() override // override move()
    {
        if (hideCount < 3 && rand() % 2 == 0) // hideCount limit to 3 and 50 percent chance hide if still can hide
        {
            isHidden = true;
            hideCount++;
            setHidden(true);   //set robot hidden status to true
            logger << getName() << " hide, (" << hideCount << "/3)" << endl;
        }
        else
        {
            isHidden = false;
            setHidden(false);

            if (hideCount >= 3)  //if robot hide 3 times already
                logger << getName() << " finish use hide, keep moving." << endl;
            else   //robot choose not to hide 
                logger << getName() << " did not hide this turn, keep moving." << endl;
        }
    }

    bool getHiddenStatus() const   //get robot hide status
    {
        return isHidden;
    }

    void appear() //robot appear if didnt hide
    {
        isHidden = false;
    }

    bool canBeHit() override //override can be hit when robot not hide
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
    int jumpCount = 0; //number of jump robot use, limit to 3

protected:
    // override getUpgradeTypes method from GenericRobot to include its own upgrade areas
    const vector<string> &getUpgradeTypes() const override
    {
        static const vector<string> upgradeTypes = {"JumpLongShotBot", "JumpSemiAutoBot", "JumpThirtyShotBot", "JumpKnightBot", "JumpQueenBot", "JumpVampireBot", "JumpScoutBot", "JumpTrackBot"};
        return upgradeTypes;
    }

public:
    //constructor
    JumpBot(const string &name, int x, int y)
        : Robot(name, x, y),
          GenericRobot(name, x, y) {}

    void move() override  // override move()
    {
        if (jumpCount < 3 && rand() % 2 == 0) // jumpCount limit to 3 and 50 percent chance hide if still can jump
        {
            int jumpx, jumpy; //new position after jump
            bool positionFound = false; 
            int attempts = 0;
            const int maxAttempt = 10; // maximum attempts to find a valid position limit to 10

            //find valid position that dont have robot
            while (!positionFound && attempts < maxAttempt) // while not found position and attempts less than maxAttempt（10）
            {
                attempts++;
                jumpx = rand() % battlefield->getWidth(); // random x position
                jumpy = rand() % battlefield->getHeight(); // random y position

                if (!battlefield->getRobotAt(jumpx, jumpy))
                {
                    positionFound = true;
                }
            }

            if (positionFound)  //if found valid position
            {
                jumpCount++;
                battlefield->removeRobotFromGrid(this); // remove robot from current position
                battlefield->placeRobot(this, jumpx, jumpy); // place robot to new position
                setPosition(jumpx, jumpy);     // update robot position
                logger << getName() << " jump to (" << jumpx << "," << jumpy << "), (" << jumpCount << "/3)\n";
            }
            else
            {
                logger << getName() << " could not find empty position to jump\n";
            }
        }
        else
        {
            if (jumpCount >= 3) //if robot jump 3 times already
            {
                logger << getName() << " cannot jump already. \n";
            }
            else  //robot choose not to jump
            {
                logger << getName() << " did not jump this turn, keep moving\n";
            }
        }
    }

    int getJumpCount() const //return number of jump robot use   //TBC
    {
        return jumpCount;
    }
};

//******************************************
// LongShotBot
//******************************************

class LongShotBot : public virtual GenericRobot
{
private:
    int fireCount = 0; //number longshotbot fire

protected:
    // override getUpgradeTypes method from GenericRobot to include its own upgrade areas
    const vector<string> &getUpgradeTypes() const override
    {
        static const vector<string> upgradeTypes = {"HideLongShotBot", "JumpLongShotBot", "LongShotScoutBot", "LongShotTrackBot"};
        return upgradeTypes;
    }

public:
    //constructor
    LongShotBot(const string &name, int x, int y)
        : Robot(name, x, y),
          GenericRobot(name, x, y) {}

    void fire(int X, int Y) override //ovveride fire method
    {
        if (hasFired)  //prevent multiple fire in one round
            return;
        hasFired = true;

        if (!hasAmmo()) //if no ammo left
        {
            logger << name << " has no ammo left. It will self destruct!" << endl;
            lives = 0;
            isDie = true;
            return;
        }

        bool fired = false;  //flag to check if fire successful
        int x = getX();      //get robot x location
        int y = getY();      //get robot y location

        for (int dx = -3; dx <= 3; dx++) //scan 7x7 battlefield (-3,-2,-1,0,1,2,3)
        {
            for (int dy = -3; dy <= 3; dy++)
            {
                if (dx == 0 && dy == 0) //skip current position
                    continue;
                if (abs(dx) + abs(dy) > 3) // skip if not in 7x7 range
                    continue;

                int targetX = x + dx;  //calculate target x location
                int targetY = y + dy;  //calculate target y location

                Robot *target = battlefield->getRobotAt(targetX, targetY); //get robot in the target location 

                if (target && target != this&& !target->getIsHurt())      // if target is not null, not itself, and not hurt,can be hit
                {
                    GenericRobot *gtarget = dynamic_cast<GenericRobot *>(target);
                    logger << ">> " << getName() << " fire at (" << targetX << "," << targetY << ")" << endl;
                    useAmmo();

                    if (gtarget->isHidden()) //if robot hide, attack miss
                    {
                        logger << gtarget->getName() << " is hidden, attack miss." << endl;
                        fired = true;
                    }
                    else if (hitProbability()) //successful hit
                    {
                        gtarget->takeDamage();
                        logger << "Hit! (" << gtarget->getName() << ") be killed" << endl;
                        fireCount++;
                        fired = true;

                        const vector<string> &upgradeTypes = getUpgradeTypes(); //after hit,robot can upgrade 

                        if (!upgradeTypes.empty())
                        {
                            int t = rand() % upgradeTypes.size();
                            string newType = upgradeTypes[t];
                            setPendingUpgrade(newType);
                            logger << getName() << " will upgrade into " << newType << " next turn" << endl;
                        }
                    }
                    else //if hit miss
                    {
                        logger << "Missed!" << endl; 
                        fired = true;
                    }

                    if (!hasAmmo()) //if no ammo left after fire, robot self-destruct
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

        if (!fired) //if no fire as no robot in the 7x7 range
        {
            logger << "No shooting as no robots within shooting range. " << endl;
        }
    }
};

//******************************************
// SemiAutoBot
//******************************************

class SemiAutoBot : public virtual GenericRobot
{
private:
    int fireCount = 0; //number of robot shoot

protected:
    // override getUpgradeTypes method from GenericRobot to include its own upgrade areas
    const vector<string> &getUpgradeTypes() const override
    {
        static const vector<string> upgradeTypes = {"HideSemiAutoBot", "JumpSemiAutoBot", "SemiAutoScoutBot", "SemiAutoTrackBot"};
        return upgradeTypes;
    }

public:
    //constructor
    SemiAutoBot(const string &name, int x, int y)
        : Robot(name, x, y),
          GenericRobot(name, x, y) {}

    void fire(int X, int Y) override //triple shot fire method
    {
        if (hasFired) //prevent multiple fire in one round
            return;
        hasFired = true;

        if (!hasAmmo()) //if finish use ammo
        {
            logger << name << " has no ammo left. It will self destroy!" << endl;
            lives = 0;
            isDie = true;
            return;
        }

        int x = getX(); // get robot x location
        int y = getY(); // get robot y location

        Robot *target = nullptr; //find target in battlefield
        for (Robot *r : battlefield->getListOfRobots()) // loop through all robots in battlefield
        {
            if (r != nullptr && r != this && r->getLives() > 0&& !r->getIsHurt()) //if target is not null, not itself, and not hurt
            {
                target = r;
                break;
            }
        }

        if (!target) //no target can shoot, thus no shooting
        {
            logger << "No shooting as no robots within shooting range." << endl;
            return;
        }

        GenericRobot *gtarget = dynamic_cast<GenericRobot *>(target); 
        useAmmo(); //use ammo

        logger << ">> " << getName() << " fires 3 consecutive shots at ("
               << gtarget->getX() << "," << gtarget->getY() << ")" << endl;

        bool hitSuccessful = false; // flag to check if hit successful
        for (int i = 0; i < 3; i++) //fire triple shot
        {
            if (gtarget->isHidden()) //if target robot hide, attack miss
            {
                logger << "Shot " << (i + 1) << ": " << gtarget->getName()
                       << " is hidden, attack missed." << endl;
                continue;
            }

            if (hitProbability()) //successful hit 
            {
                logger << "Shot " << (i + 1) << " hit " << gtarget->getName() << "!" << endl;
                gtarget->takeDamage();
                fireCount++;
                hitSuccessful = true;
            }
            else //if hit miss
            {
                logger << "Shot " << (i + 1) << " missed!" << endl;
            }

            if (!hasAmmo()) //if no ammo left after fire, robot self-destruct
            {
                logger << getName() << " has no ammo left, it will self-destruct!" << endl;
                lives = 0;
                isDie = true;
            }
        }

        if (hitSuccessful) //upgrade robot if successful hit robot
        {
            const vector<string> &upgradeTypes = getUpgradeTypes();// get upgrade types
            if (!upgradeTypes.empty())
            {
                int t = rand() % upgradeTypes.size(); //random choose robot upgrade from upgrage type
                string newType = upgradeTypes[t];
                setPendingUpgrade(newType);
                logger << getName() << " will upgrade into " << newType << " next turn" << endl;
            }
        }
    }

    int getFireCount() const //return number of fire count use   //TBC
    {
        return fireCount;
    }
};

//******************************************
// ThirtyShotBot
//******************************************
class ThirtyShotBot : public virtual GenericRobot
{
private:
    int shellCount; // number of shells left, default is 30

protected:
    // override getUpgradeTypes method from GenericRobot to include its own upgrade areas
    const vector<string> &getUpgradeTypes() const override
    {
        static const vector<string> upgradeTypes = {"HideThirtyShotBot", "JumpThirtyShotBot", "ThirtyShotScoutBot", "ThirtyShotTrackBot"};
        return upgradeTypes;
    }

public:
    //constructor
    ThirtyShotBot(const string &name, int x, int y)
        : Robot(name, x, y),
          GenericRobot(name, x, y),
          shellCount(30)
    {
    }
    //set nmber of shells to s (30)
    void setShellCount(int s)
    {
        shellCount = s;
    }
    //get current shell count
    int getShellCount() const
    {
        return shellCount;
    }

    //  This is to prevent shellCount from being set to 30 everytime ThirtyShotBot's constructor is called (during upgrades)
    void initializeFrom(const Robot *oldRobot) override
    {
        GenericRobot::initializeFrom(oldRobot); // call ShootingRobot's initializeFrom

        // if the previous robot was already a ThirtyShotBot, shellCount is preserved
        const ThirtyShotBot *oldThirtyShotBot = dynamic_cast<const ThirtyShotBot *>(oldRobot);
        if (oldThirtyShotBot)
        {
            this->shellCount = oldThirtyShotBot->getShellCount(); // Preserve shellCount
            logger << ">> " << getName() << " (upgraded) preserves "
                   << this->shellCount << " shells from its previous form: " << oldRobot->getName() << endl;
        }
        else
        { // if previous robot was not a ThirtyShotBot, shellCount remains as 30
            logger << getName() << " got 30 shells, replacing its current shells!" << endl;
        }
    }

    void fire(int X, int Y) override
    {
        if (shellCount <= 0) //check shell if finish
        {
            logger << getName() << " shell is finish\n";
            return;
        }

        if (hasFired)
            return;
        hasFired = true; //prevent multiple fire in one round

        int x = getX(); //get robot x location
        int y = getY(); //get robot y location
        bool fired = false;
        bool hitSuccessful = false;

        for (int dx = -1; dx <= 1 && !fired; dx++) //shoot inside 3x3 area (-1,0,1)
        {
            for (int dy = -1; dy <= 1 && !fired; dy++)
            {
                if (dx == 0 && dy == 0) // Do not fire at self
                    continue; 
                int targetX = x + dx; // calculate target x location
                int targetY = y + dy; // calculate target y location

                // Check if target position is within battlefield grid
                Robot *target = battlefield->getRobotAt(targetX, targetY);
                if (target && target != this&& !target->getIsHurt()) // if target is not null, not itself, and not hurt
                {
                    GenericRobot *gtarget = dynamic_cast<GenericRobot *>(target); 
                    if (gtarget && gtarget->canBeHit())
                    {
                        if (hitProbability())
                        {
                            gtarget->takeDamage();
                            shellCount--;
                            logger << getName() << " fires at (" << targetX << ", " << targetY << "), shell left: " << getShellCount() << "\n";
                            hitSuccessful = true;
                            logger << "Hit! " << gtarget->getName() << " is killed!\n";
                            const vector<string> &upgradeTypes = getUpgradeTypes();
                            if (!upgradeTypes.empty())
                            {
                                int t = rand() % upgradeTypes.size();
                                string newType = upgradeTypes[t];
                                setPendingUpgrade(newType);
                                logger << getName() << " will upgrade into " << newType << " next turn" << endl;
                            }
                        }
                        else //if hit miss
                        {
                            shellCount--;
                            logger << getName() << " fire at (" << targetX << ", " << targetY << "), shell left: " << shellCount << "\n";
                            logger << "Missed!\n";
                        }
                        fired = true;
                    }
                }
            }
        }

        if (!fired) //if no fire as no robot in the 3x3 range
        {
            logger << " No shooting as no robots within shooting range.";
        }
    }
};

//******************************************
// KnightBot
// Upgrade description: Can fire in diagonal directions (5 units), can upgrade to HideKnightBot, JumpKnightBot, KnightScoutBot, KnightTrackBot
//******************************************

class KnightBot : public virtual GenericRobot
{
private:
    int fireCount = 0; // number of knightbot fire

protected:
    // override getUpgradeTypes method from GenericRobot to include its own upgrade areas
    const vector<string> &getUpgradeTypes() const override
    {
        static const vector<string> upgradeTypes = {"HideKnightBot", "JumpKnightBot", "KnightScoutBot", "KnightTrackBot"};
        return upgradeTypes;
    }

public:
    //constructor
    KnightBot(const string &name, int x, int y)
        : Robot(name, x, y),
          GenericRobot(name, x, y) {}

    void fire(int X, int Y) override // override fire method
    {
        if (hasFired) 
            return;
        hasFired = true; //prevent multiple fire in one round
        int x = getX();  //get robot x location
        int y = getY();  //get robot y location
        bool fired = false;
        bool hitSuccessful = false; 
        vector<string> hitRobots;
        // Diagonal directions: (1,1), (1,-1), (-1,1), (-1,-1),four direction
        const vector<pair<int, int>> diagonals = {{1, 1}, {1, -1}, {-1, 1}, {-1, -1}};
        int diagIdx = rand() % diagonals.size();  //random choose diagonal direction
        int dx = diagonals[diagIdx].first; 
        int dy = diagonals[diagIdx].second;
        logger << getName() << " selects diagonal (" << dx << "," << dy << ") for attack (length 5)" << endl;
        for (int dist = 1; dist <= 5; ++dist) //attack robot in diagonal   (1,2,3,4,5)
        {
            int targetX = x + dx * dist;
            int targetY = y + dy * dist;
            if (!battlefield->isPositionWithinGrid(targetX, targetY))
                break;
            Robot *target = battlefield->getRobotAt(targetX, targetY);
            if (target && target != this && target->getLives() > 0&& !target->getIsHurt()) //if target is not null, not itself, and not hurt
            {
                GenericRobot *gtarget = dynamic_cast<GenericRobot *>(target);
                logger << getName() << " fires at (" << targetX << "," << targetY << ")" << endl;
                useAmmo();
                fired = true;
                if (gtarget && gtarget->canBeHit()) //if target can be hit
                {
                    if (hitProbability()) //if hit Successful 
                    {
                        gtarget->takeDamage();
                        fireCount++;
                        hitSuccessful = true;
                        hitRobots.push_back(gtarget->getName());
                        logger << "Hit! (" << gtarget->getName() << ") is killed!" << endl;
                    }
                    else //if hit miss
                    {
                        logger << "Missed!" << endl;
                    }
                    {
                        logger << getName() << " missed!" << endl;
                    }
                }
            }
        }
        if (!fired) //if no fire as no robot in the diagonal range
        {
            logger << " No shooting as no robots in diagonal to fire at\n";
        }
        else if (hitSuccessful)  //print the successful hit robot
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
            const vector<string> &upgradeTypes = getUpgradeTypes(); //check for upgrade chance
            if (!upgradeTypes.empty())
            {
                int t = rand() % upgradeTypes.size(); //random choose upgrade from upgrade type
                string newType = upgradeTypes[t];
                setPendingUpgrade(newType);
                logger << getName() << " will upgrade into " << newType << " next turn!\n";
            }
        }
    }
};

//******************************************
// QueenBot
// Upgrade description: Can fire in 8 directions surrounding (1 unit), can upgrade to HideQueenBot, JumpQueenBot, QueenScoutBot, QueenTrackBot
//******************************************
class QueenBot : public virtual GenericRobot
{
private:
    const vector<pair<int, int>> directions = { //8 direction north,east,south.....
        {0, 1},
        {1, 0},
        {0, -1},
        {-1, 0},
        {1, 1},
        {1, -1},
        {-1, 1},
        {-1, -1}};

protected:
    // override getUpgradeTypes method from GenericRobot to include its own upgrade areas
    const vector<string> &getUpgradeTypes() const override
    {
        static const vector<string> upgradeTypes = {"HideQueenBot", "JumpQueenBot", "QueenScoutBot", "QueenTrackBot"};
        return upgradeTypes;
    }

public:
    //constructor
    QueenBot(const string &name, int x, int y)
        : Robot(name, x, y),
          GenericRobot(name, x, y) {}
    //ovveride fire method
    void fire(int X, int Y) override 
    {
        if (hasFired) //prevent multiple fire in one round
            return;
        hasFired = true;

        if (!hasAmmo()) //if finish use ammo
        {
            logger << getName() << " has no ammo left. It will self destruct!" << endl;
            lives = 0;
            isDie = true;
            return;
        }

        int x = getX(); //get robot x location
        int y = getY(); //get robot y location
        bool fired = false;

        //attack in 8 direction
        for (const auto &dir : directions)
        {
            int dx = dir.first;
            int dy = dir.second;
            for (int dist = 1;; dist++) //attack until the end of battlefield
            {
                int targetX = x + dx * dist;
                int targetY = y + dy * dist;
                if (targetX < 0 || targetY < 0 || targetX >= battlefield->getWidth() || targetY >= battlefield->getHeight()) //check battlefield bound
                    break;
                if (targetX == x && targetY == y)
                    continue; // Do not fire at self
                Robot *target = battlefield->getRobotAt(targetX, targetY);
                if (target && target != this&& !target->getIsHurt())
                {
                    GenericRobot *gtarget = dynamic_cast<GenericRobot *>(target);
                    if (gtarget->canBeHit())
                    {
                        logger << getName() << " fires at (" << targetX << "," << targetY << ")\n";
                        useAmmo();
                        if (hitProbability()) //if hit succcessful
                        {
                            gtarget->takeDamage();
                            logger << getName() << " hit " << gtarget->getName() << endl;
                            const vector<string> upgradeTypes = getUpgradeTypes();  //robot upgrade
                            if (!upgradeTypes.empty()) 
                            {
                                int t = rand() % upgradeTypes.size(); //random choose from upgrade type to upgrade
                                string newType = upgradeTypes[t];
                                setPendingUpgrade(newType);
                                logger << getName() << " will upgrade into " << newType << " next turn!\n";
                            }
                        }
                        else
                        {
                            logger << "Missed!" << endl;
                        }
                        fired = true;
                        break; //stop after hit first target
                    }
                }
            }
            if (fired)
                break; //stop after successful hit 
        }
        if (!fired) //if no fire as no robot in the 8 direction
        {
            logger << "No shooting, as sadly, " << getName() << " found no target in straight line\n";
        }
    }
};

//******************************************
// VampireBot
// Upgrade description: After hitting a robot, gain 1 life, can gain life maximum 3 times
//******************************************
class VampireBot : public virtual GenericRobot
{
private:
    int gainLivesCount = 0; // number of times the VampireBot has gained lives

protected:
    // override getUpgradeTypes method from GenericRobot to include its own upgrade areas
    const vector<string> &getUpgradeTypes() const override
    {
        static const vector<string> upgradeTypes = {"HideVampireBot", "JumpVampireBot", "VampireScoutBot", "VampireTrackBot"};
        return upgradeTypes;
    }

public:
    // Constructor
    VampireBot(const string &name, int x, int y)
        : Robot(name, x, y),
          GenericRobot(name, x, y) {};

    //fire method
    void fire(int X, int Y) override
    {
        if (!hasAmmo()) //if finish use ammo
        {
            logger << getName() << " has no ammo left!" << endl;
            isDie = true;
            return;
        }

        if (hasFired) //prevent multiple fire in one round
            return;
        hasFired = true;

        if (!detectedTargets.empty()) //detact target robot
        {
            int randomIndex = rand() % detectedTargets.size(); //select random target
            Robot *target = detectedTargets[randomIndex];
            int targetX = target->getX(); // get target x location
            int targetY = target->getY(); // get target y location
            logger << ">> " << getName() << " fires at (" << targetX << ", " << targetY << ")" << endl;
            useAmmo();

            if (target->isHidden()) //check robot hide or not
            {
                logger << target->getName() << " is hidden, attack missed." << endl;
            }
            else if (hitProbability()) //if hit successful
            {
                logger << "Hit! (" << target->getName() << ") is killed!" << endl;

                target->takeDamage();
                if (getLives() < 3) //check if robot lives less than 3
                {
                    if (gainLivesCount < 3) //if not reach gain 3 live
                    {
                        setLives(getLives() + 1); //gain 1 live
                        logger << getName() << " gained 1 life from killing " << target->getName() << "! (" << gainLivesCount + 1 << "/3)" << endl;
                        gainLivesCount++;
                    }
                    else // if already gain 3 lives
                    {
                        logger << getName() << " already gained lives 3 times, cannot gain anymore lives." << endl;
                    }
                }
                else //if robot already at max lives (3 lives)
                {
                    logger << getName() << " is already at max lives (" << getLives() << "), cannot gain extra life from this kill." << endl;
                }

                const vector<string> upgradeTypes = getUpgradeTypes(); //robot upgrade chance
                if (!upgradeTypes.empty())
                {
                    int t = rand() % upgradeTypes.size(); //random choose from upgrade type 
                    string newType = upgradeTypes[t];
                    setPendingUpgrade(newType);
                    logger << getName() << " will upgrade into " << newType << " next turn!\n";
                }
            }
        }
        else //no shooting as no target robot
        {
            logger << "No shooting as no robots within shooting range." << endl;
        }
    }
};
//******************************************
// ScoutBot
//******************************************
class ScoutBot : public virtual GenericRobot
{
private:
    int scoutCount = 0; //number of scoutbot scan, limit to 3

protected:
    // override getUpgradeTypes method from GenericRobot to include its own upgrade areas
    const vector<string> &getUpgradeTypes() const override
    {
        static const vector<string> upgradeTypes = {"HideScoutBot", "JumpScoutBot"};
        return upgradeTypes;
    }

public:
    //constructor
    ScoutBot(const string &name, int x, int y)
        : Robot(name, x, y),
          GenericRobot(name, x, y) {}

    void look(int X, int Y) override //override look method
    {

        availableSpaces.clear(); // clear available spaces before scanning

        if (scoutCount >= 3) //if scan more than 3 time
        {
            logger << getName() << " reach the limit,cannot scan already\n";
        }
        else if (rand() % 2 == 0) //50 percent to use scan function 
        {
            logger << getName() << " scan the battlefield\n";
            for (int y = 0; y < battlefield->getHeight(); ++y) //scan the entire battlefield
            {
                for (int x = 0; x < battlefield->getWidth(); ++x) 
                {
                    Robot *r = battlefield->getRobotAt(x, y);
                    if (r) //print all robot found in battlefield
                    {
                        logger << "got robot: " << r->getName()
                               << " at (" << x << "," << y << ")\n";
                    }
                }
            }
            scoutCount++; //+1 after use
        }
        else //if not use scan function
        {
            logger << getName() << " try scan it next round \n";
        }

        int x = getX(); //get robot x location
        int y = getY(); //get robot y location
        for (int dx = -1; dx <= 1; dx++) //movement space (-1,0,1)
        {
            for (int dy = -1; dy <= 1; dy++)
            {
                if (dx == 0 && dy == 0)
                    continue;

                int newX = x + dx; // calculate new x position
                int newY = y + dy; // calculate new y position

                if (battlefield->isPositionAvailable(newX, newY))
                {
                    availableSpaces.emplace_back(newX, newY);
                }
            }
        }
    }

    int getScoutCount() const //get robot scan count
    {
        return scoutCount;
    }
};

//******************************************
// TrackBot
//******************************************

class TrackBot : public virtual GenericRobot
{
private:
    int tracker = 3; // number of robots that TrackBot can track, limit to 3
    vector<Robot *> track_target; //list of robot track

protected:
    // override getUpgradeTypes method from GenericRobot to include its own upgrade areas
    const vector<string> &getUpgradeTypes() const override
    {
        static const vector<string> upgradeTypes = {"HideTrackBot", "JumpTrackBot"};
        return upgradeTypes;
    }

public:
    //constructor
    TrackBot(const string &name, int x, int y)
        : Robot(name, x, y),
          GenericRobot(name, x, y) {}

    void look(int X, int Y) override
    {

        availableSpaces.clear(); // clear available spaces before scanning

        if (tracker == 0) //if finish use tracking
        {
            logger << getName() << " cannot track robot already\n";
        }
        else
        {
            int x = getX();
            int y = getY();
            bool plant = false; //track if find robot

            for (int dx = -1; dx <= 1 && !plant; dx++) //scan 3x3 battlefield (-1,0,1)
            {
                for (int dy = -1; dy <= 1 && !plant; dy++)
                {
                    int targetX = x + dx; // calculate target x location
                    int targetY = y + dy; // calculate target y location

                    Robot *target = battlefield->getRobotAt(targetX, targetY);
                    if (target && target != this) //if taget is not null, not itself, not hurt, able to be tracked
                    {
                        track_target.push_back(target); //add to track list
                        tracker--; //-1 tracker
                        logger << getName() << " track " << target->getName()
                               << " at (" << targetX << "," << targetY << ")\n";
                        plant = true; //target found
                    }
                }
            }
            if (!plant) //no target found
            {
                logger << getName() << " no target can track\n";
            }
        }

        // movement space(like GenericRobot)
        int x = getX();
        int y = getY();
        for (int dx = -1; dx <= 1; dx++)
        {
            for (int dy = -1; dy <= 1; dy++)
            {
                if (dx == 0 && dy == 0)
                    continue;

                int newX = x + dx;
                int newY = y + dy;

                if (battlefield->isPositionAvailable(newX, newY))
                {
                    availableSpaces.emplace_back(newX, newY);
                }
            }
        }
    }

    void showTrackTarget() //display track robot
    {
        if (track_target.empty()) //if the list empty
        {
            logger << getName() << " didnt track any robot\n";
            return;
        }
        //print all robot in track list
        logger << getName() << " is tracking:\n"; 
        for (Robot *r : track_target) //list down all robot
        {
            logger << r->getName() << " at (" << r->getX() << "," << r->getY() << ")\n";
        }
    }
    int getTracker() const //get tracker number remaining
    {
        return tracker;
    }
};

//******************************************
// HideLongShotBot (inherits from HideBot and LongShotBot)
//Implementation of Multiple Inheritance
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
    //constructor
    HideLongShotBot(const string &name, int x, int y)
        : Robot(name, x, y),
          GenericRobot(name, x, y),
          HideBot(name, x, y),
          LongShotBot(name, x, y) {}

    void move() override
    {
        HideBot::move();  // use HideBot's move()
    }

    void fire(int X, int Y) override
    {
        LongShotBot::fire(X, Y); // use LongShotBot's move()
    }

    void think() override
    {
        HideBot::think(); // use HideBot's think()
    }

    void act() override
    {
        look(0, 0);  // GenericRobot's look
        think();
        fire(0, 0);
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
// HideSemiAutoBot (inherits from HideBot and SemiAutoBot)
//******************************************
class HideSemiAutoBot : public HideBot, public SemiAutoBot
{
protected:
    const vector<string> &getUpgradeTypes() const override
    {
        static const vector<string> upgradeTypes = {"HideSemiAutoScoutBot", "HideSemiAutoTrackBot"};
        return upgradeTypes;
    }

public:
    //constructor
    HideSemiAutoBot(const string &name, int x, int y)
        : Robot(name, x, y),
          GenericRobot(name, x, y),
          HideBot(name, x, y),
          SemiAutoBot(name, x, y) {}

    void move() override
    {
        HideBot::move();  // HideBot's move()
    }

    void fire(int X, int Y) override
    {
        SemiAutoBot::fire(X, Y);  // SemiAutoBot's fire()
    }

    void think() override
    {
        HideBot::think();  // HideBot's think
    }

    void act() override
    {
        look(0, 0);  // GenericRobot's look
        think();
        fire(0, 0);
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
// HideThirtyShotBot (inherits from HideBot and ThirtyShotBot)
//******************************************
class HideThirtyShotBot : public HideBot, public ThirtyShotBot
{
protected:
    const vector<string> &getUpgradeTypes() const override
    {
        static const vector<string> upgradeTypes = {"HideThirtyShotScoutBot", "HideThirtyShotTrackBot"};
        return upgradeTypes;
    }

public:
    HideThirtyShotBot(const string &name, int x, int y)
        : Robot(name, x, y),
          GenericRobot(name, x, y),
          HideBot(name, x, y),
          ThirtyShotBot(name, x, y) {}

    void move() override
    {
        HideBot::move(); //hidebot move
    }

    void fire(int X, int Y) override
    {
        ThirtyShotBot::fire(X, Y);  //thirtyshotbot fire
    }

    void think() override
    {
        HideBot::think(); //hidebot think
    }

    void act() override
    {
        look(0, 0);  // GenericRobot's look
        think();
        fire(0, 0);
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
// HideKnightBot (inherits from HideBot and KnightBot)
//******************************************
class HideKnightBot : public HideBot, public KnightBot
{
protected:
    const vector<string> &getUpgradeTypes() const override
    {
        static const vector<string> upgradeTypes = {"HideKnightScoutBot", "HideKnightTrackBot"};
        return upgradeTypes;
    }

public:
    //constructor
    HideKnightBot(const string &name, int x, int y)
        : Robot(name, x, y),
          GenericRobot(name, x, y),
          HideBot(name, x, y),
          KnightBot(name, x, y) {}

    void move() override
    {
        HideBot::move(); //hidebot move
    }

    void fire(int X, int Y) override
    { 
        KnightBot::fire(X, Y); //knightbot fire
    }

    void think() override
    {
        HideBot::think(); //hidebot think
    }

    void act() override
    {
        look(0, 0); //generic robot look 
        think();
        fire(0, 0);
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
// HideQueenBot (inherits from HideBot and QueenBot)
//******************************************
class HideQueenBot : public HideBot, public QueenBot
{
protected:
    const vector<string> &getUpgradeTypes() const override
    {
        static const vector<string> upgradeTypes = {"HideQueenScoutBot", "HideQueenTrackBot"};
        return upgradeTypes;
    }

public:
    //constructor
    HideQueenBot(const string &name, int x, int y)
        : Robot(name, x, y),
          GenericRobot(name, x, y),
          HideBot(name, x, y),
          QueenBot(name, x, y) {}

    void move() override
    {
        HideBot::move(); //hidebot move
    }

    void fire(int X, int Y) override
    {
        QueenBot::fire(X, Y); //queenbot fire
    }

    void think() override
    {
        HideBot::think(); //hidebot think
    }

    void act() override
    {
        look(0, 0);   // GenericRobot's look
        think();
        fire(0, 0);
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
// HideVampireBot (inherits from HideBot and VampireBot)
//******************************************
class HideVampireBot : public HideBot, public VampireBot
{
protected:
    const vector<string> &getUpgradeTypes() const override
    {
        static const vector<string> upgradeTypes = {"HideVampireScoutBot", "HideVampireTrackBot"};
        return upgradeTypes;
    }

public:
    //constructor
    HideVampireBot(const string &name, int x, int y)
        : Robot(name, x, y),
          GenericRobot(name, x, y),
          HideBot(name, x, y),
          VampireBot(name, x, y) {}

    void move() override
    {
        HideBot::move(); //hidebot move
    }

    void fire(int X, int Y) override
    {
        VampireBot::fire(X, Y); //vampirebot fire
    }

    void think() override
    {
        HideBot::think(); //hidebot think
    }

    void act() override
    {
        look(0, 0);  // GenericRobot's look
        think();
        fire(0, 0);
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
// HideScoutBot (inherits from HideBot and ScoutBot)
//******************************************
class HideScoutBot : public HideBot, public ScoutBot
{
protected:
    const vector<string> &getUpgradeTypes() const override
    {
        static const vector<string> upgradeTypes = {
            "HideVampireScoutBot",
            "HideLongShotScoutBot",
            "HideSemiAutoScoutBot",
            "HideThirtyShotScoutBot",
            "HideKnightScoutBot",
            "HideQueenScoutBot"};
        return upgradeTypes;
    }

public:
    //constructor
    HideScoutBot(const string &name, int x, int y)
        : Robot(name, x, y),
          GenericRobot(name, x, y),
          HideBot(name, x, y),
          ScoutBot(name, x, y) {}

    void move() override
    {
        HideBot::move(); //hidebot move
    }

    void think() override
    {
        HideBot::think(); //hidebot think
    }

    void act() override
    {
        look(0, 0); 
        think();
        fire(0, 0);  // GenericRobot's fire
    }

    void look(int X, int Y) override
    {
        ScoutBot::look(X, Y); //scoutbot look 
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
// HideTrackBot (inherits from HideBot and TrackBot)
//******************************************
class HideTrackBot : public HideBot, public TrackBot
{
protected:
    const vector<string> &getUpgradeTypes() const override
    {
        static const vector<string> upgradeTypes = {
            "HideVampireTrackBot",
            "HideLongShotTrackBot",
            "HideSemiAutoTrackBot",
            "HideThirtyShotScoutBot",
            "HideKnightScoutBot",
            "HideQueenScoutBot"};
        return upgradeTypes;
    }

public:
    //constructor 
    HideTrackBot(const string &name, int x, int y)
        : Robot(name, x, y),
          GenericRobot(name, x, y),
          HideBot(name, x, y),
          TrackBot(name, x, y) {}

    void move() override
    {
        HideBot::move(); //hidebot move
    }

    void think() override
    {
        HideBot::think(); //hidebot think
    }

    void act() override
    {
        look(0, 0);
        think();
        fire(0, 0); // GenericRobot's fire
    }

    void look(int X, int Y) override
    {
        TrackBot::look(X, Y); //trackbot look
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
// JumpLongShotBot (inherits from JumpBot and LongShotBot)
//******************************************
class JumpLongShotBot : public JumpBot, public LongShotBot
{
protected:
    const vector<string> &getUpgradeTypes() const override
    {
        static const vector<string> upgradeTypes = {"JumpLongShotScoutBot", "JumpLongShotTrackBot"};
        return upgradeTypes;
    }

public:
    //constructor
    JumpLongShotBot(const string &name, int x, int y)
        : Robot(name, x, y),
          GenericRobot(name, x, y),
          JumpBot(name, x, y),
          LongShotBot(name, x, y) {}

    void move() override
    {
        JumpBot::move(); //jumpbot move
    }

    void fire(int X, int Y) override
    {
        LongShotBot::fire(X, Y); //longshotbot fire
    }

    void think() override
    {
        JumpBot::think(); //jumpbot think
    }

    void act() override
    {
        look(0, 0);  // GenericRobot's look
        think();
        fire(0, 0);
    }

    bool canBeHit() override
    {
        return JumpBot::canBeHit();
    }

    void setBattlefield(Battlefield *bf)
    {
        GenericRobot::setBattlefield(bf);
    }
};

//******************************************
// JumpSemiAutoBot (inherits from JumpBot and SemiAutoBot)
//******************************************
class JumpSemiAutoBot : public JumpBot, public SemiAutoBot
{
protected:
    const vector<string> &getUpgradeTypes() const override
    {
        static const vector<string> upgradeTypes = {"JumpSemiAutoScoutBot", "JumpSemiAutoTrackBot"};
        return upgradeTypes;
    }

public:
    //constructor
    JumpSemiAutoBot(const string &name, int x, int y)
        : Robot(name, x, y),
          GenericRobot(name, x, y),
          JumpBot(name, x, y),
          SemiAutoBot(name, x, y) {}

    void move() override
    {
        JumpBot::move(); //jumpbot move
    }

    void fire(int X, int Y) override
    {
        SemiAutoBot::fire(X, Y); //semiautobot fire
    }

    void think() override
    {
        JumpBot::think(); //jumpbot think
    }

    void act() override
    {
        look(0, 0); //genericrobot look 
        think();
        fire(0, 0);
    }

    bool canBeHit() override
    {
        return JumpBot::canBeHit();
    }

    void setBattlefield(Battlefield *bf)
    {
        GenericRobot::setBattlefield(bf);
    }
};
//******************************************
// JumpThirtyShotBot (inherits from JumpBot and ThirtyShotBot)
//******************************************
class JumpThirtyShotBot : public JumpBot, public ThirtyShotBot
{
protected:
    const vector<string> &getUpgradeTypes() const override
    {
        static const vector<string> upgradeTypes = {"JumpThirtyShotScoutBot", "JumpThirtyShotTrackBot"};
        return upgradeTypes;
    }

public:
    //constructor
    JumpThirtyShotBot(const string &name, int x, int y)
        : Robot(name, x, y),
          GenericRobot(name, x, y),
          JumpBot(name, x, y),
          ThirtyShotBot(name, x, y) {}

    void move() override
    {
        JumpBot::move(); //jumpbot move
    }

    void fire(int X, int Y) override
    {
        ThirtyShotBot::fire(X, Y); //thirtyshotbot fire
    }

    void think() override
    {
        JumpBot::think(); //jumpbot think
    }

    void act() override
    {
        look(0, 0); //genericrobot look
        think();
        fire(0, 0);
    }

    bool canBeHit() override
    {
        return JumpBot::canBeHit();
    }

    void setBattlefield(Battlefield *bf)
    {
        GenericRobot::setBattlefield(bf);
    }
};

//******************************************
// JumpKnightBot (inherits from JumpBot and KnightBot)
//******************************************
class JumpKnightBot : public JumpBot, public KnightBot
{
protected:
    const vector<string> &getUpgradeTypes() const override
    {
        static const vector<string> upgradeTypes = {"JumpKnightScoutBot", "JumpKnightTrackBot"};
        return upgradeTypes;
    }

public:
    //constructot
    JumpKnightBot(const string &name, int x, int y)
        : Robot(name, x, y),
          GenericRobot(name, x, y),
          JumpBot(name, x, y),
          KnightBot(name, x, y) {}

    void move() override
    {
        JumpBot::move(); //jumpbot move
    }

    void fire(int X, int Y) override
    {
        KnightBot::fire(X, Y); //knightbot fire
    }

    void think() override
    {
        JumpBot::think(); //jumpbot think
    }

    void act() override
    {
        look(0, 0); //genericrobot look
        think();
        fire(0, 0);
    }

    bool canBeHit() override
    {
        return JumpBot::canBeHit();
    }

    void setBattlefield(Battlefield *bf)
    {
        GenericRobot::setBattlefield(bf);
    }
};

//******************************************
// JumpQueenBot (inherits from JumpBot and QueenBot)
//******************************************
class JumpQueenBot : public JumpBot, public QueenBot
{
protected:
    const vector<string> &getUpgradeTypes() const override
    {
        static const vector<string> upgradeTypes = {"JumpQueenScoutBot", "JumpQueenTrackBot"};
        return upgradeTypes;
    }

public:
    //constructor
    JumpQueenBot(const string &name, int x, int y)
        : Robot(name, x, y),
          GenericRobot(name, x, y),
          JumpBot(name, x, y),
          QueenBot(name, x, y) {}

    void move() override
    {
        JumpBot::move(); //jumpbot move
    }

    void fire(int X, int Y) override
    {
        QueenBot::fire(X, Y); //queenbot fire
    }

    void think() override
    {
        JumpBot::think(); //jumpbot think
    }

    void act() override
    {
        look(0, 0); //genericrobot look
        think();
        fire(0, 0);
    }

    bool canBeHit() override
    {
        return JumpBot::canBeHit();
    }

    void setBattlefield(Battlefield *bf)
    {
        GenericRobot::setBattlefield(bf);
    }
};

//******************************************
// JumpVampireBot (inherits from JumpBot and VampireBot)
//******************************************
class JumpVampireBot : public JumpBot, public VampireBot
{
protected:
    const vector<string> &getUpgradeTypes() const override
    {
        static const vector<string> upgradeTypes = {"JumpVampireScoutBot", "JumpVampireTrackBot"};
        return upgradeTypes;
    }

public:
    //constructor
    JumpVampireBot(const string &name, int x, int y)
        : Robot(name, x, y),
          GenericRobot(name, x, y),
          JumpBot(name, x, y),
          VampireBot(name, x, y) {}

    void move() override
    {
        JumpBot::move(); //jumpbot move
    }

    void fire(int X, int Y) override
    {
        VampireBot::fire(X, Y); //vampirebot fire
    }

    void think() override
    {
        JumpBot::think(); //jumpbot think
    }

    void act() override
    {
        look(0, 0); //genericrobot look
        think();
        fire(0, 0);
    }

    bool canBeHit() override
    {
        return JumpBot::canBeHit();
    }

    void setBattlefield(Battlefield *bf)
    {
        GenericRobot::setBattlefield(bf);
    }
};

//******************************************
// JumpScoutBot (inherits from JumpBot and ScoutBot)
//******************************************
class JumpScoutBot : public JumpBot, public ScoutBot
{
protected:
    const vector<string> &getUpgradeTypes() const override
    {
        static const vector<string> upgradeTypes = {
            "JumpLongShotScoutBot",
            "JumpSemiAutoScoutBot",
            "JumpThirtyShotScoutBot",
            "JumpKnightScoutBot",
            "JumpQueenScoutBot",
            "JumpVampireScoutBot"};
        return upgradeTypes;
    }

public:
    //constructor
    JumpScoutBot(const string &name, int x, int y)
        : Robot(name, x, y),
          GenericRobot(name, x, y),
          JumpBot(name, x, y),
          ScoutBot(name, x, y) {}

    void move() override
    {
        JumpBot::move(); //jumpbot move
    }

    void think() override
    {
        JumpBot::think(); //jumpbot think
    }

    void act() override
    {
        ScoutBot::look(0, 0); // Use ScoutBot's look for targeting
        think();
        fire(0, 0);
    }

    void look(int X, int Y) override
    {
        ScoutBot::look(X, Y);
    }

    bool canBeHit() override
    {
        return JumpBot::canBeHit();
    }

    void setBattlefield(Battlefield *bf)
    {
        GenericRobot::setBattlefield(bf);
    }
};

//******************************************
// JumpTrackBot (inherits from JumpBot and TrackBot)
//******************************************
class JumpTrackBot : public JumpBot, public TrackBot
{
protected:
    const vector<string> &getUpgradeTypes() const override
    {
        static const vector<string> upgradeTypes = {
            "JumpLongShotTrackBot",
            "JumpSemiAutoTrackBot",
            "JumpThirtyShotTrackBot",
            "JumpKnightTrackBot",
            "JumpQueenTrackBot",
            "JumpVampireTrackBot"};
        return upgradeTypes;
    }

public:
    //constructor
    JumpTrackBot(const string &name, int x, int y)
        : Robot(name, x, y),
          GenericRobot(name, x, y),
          JumpBot(name, x, y),
          TrackBot(name, x, y) {}

    void move() override
    {
        JumpBot::move(); //jumpbot move
    }

    void think() override
    {
        JumpBot::think(); //jumpbot think
    }

    void act() override
    {
        TrackBot::look(0, 0); // Use ScoutBot's look for targeting
        think();
        fire(0, 0);
    }

    void look(int X, int Y) override
    {
        TrackBot::look(X, Y);
    }

    bool canBeHit() override
    {
        return JumpBot::canBeHit();
    }

    void setBattlefield(Battlefield *bf)
    {
        GenericRobot::setBattlefield(bf);
    }
};

//******************************************
// LongShotScoutBot
// (inherits from LongShotBot and ScoutBot)
//******************************************
class LongShotScoutBot : public LongShotBot, public ScoutBot
{
protected:
    const vector<string> &getUpgradeTypes() const override
    {
        static const vector<string> upgradeTypes = {"HideLongShotScoutBot", "JumpLongShotScoutBot"};
        return upgradeTypes;
    }

public:
    LongShotScoutBot(const string &name, int x, int y) : Robot(name, x, y),
                                                         GenericRobot(name, x, y),
                                                         LongShotBot(name, x, y),
                                                         ScoutBot(name, x, y)
    {
    }

    void fire(int X, int Y) override
    {
        LongShotBot::fire(X, Y);
    }

    void think() override
    {
        // Disambiguate GenericRobot base
        ScoutBot::think();
    }

    void act() override
    {
        look(0, 0);
        think();
        fire(0, 0);
        move();
    }

    void look(int X, int Y) override
    {
        ScoutBot::look(X, Y); //scoutbot look
    }

    bool canBeHit() override
    {
        return ScoutBot::canBeHit();
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
protected:
    const vector<string> &getUpgradeTypes() const override
    {
        static const vector<string> upgradeTypes = {"HideLongShotTrackBot", "JumpLongShotTrackBot"};
        return upgradeTypes;
    }

public:
    LongShotTrackBot(const string &name, int x, int y) : Robot(name, x, y),
                                                         GenericRobot(name, x, y),
                                                         LongShotBot(name, x, y),
                                                         TrackBot(name, x, y) {}

    void fire(int X, int Y) override
    {
        LongShotBot::fire(X, Y);
    }

    void think() override
    {
        TrackBot::think();
    }

    void act() override
    {
        look(0, 0);
        think();
        fire(0, 0);
        move();
    }

    void look(int X, int Y) override
    {
        TrackBot::look(X, Y); // Use long-range look for any explicit look calls
    }

    bool canBeHit() override
    {
        return TrackBot::canBeHit();
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
protected:
    const vector<string> &getUpgradeTypes() const override
    {
        static const vector<string> upgradeTypes = {"HideSemiAutoScoutBot", "JumpSemiAutoScoutBot"};
        return upgradeTypes;
    }

public:
    SemiAutoScoutBot(const string &name, int x, int y) : Robot(name, x, y),
                                                         GenericRobot(name, x, y),
                                                         SemiAutoBot(name, x, y),
                                                         ScoutBot(name, x, y) {}

    void fire(int X, int Y) override
    {
        SemiAutoBot::fire(X, Y);
    }

    void think() override
    {
        ScoutBot::think();
    }

    void act() override
    {
        look(0, 0);
        think();
        fire(0, 0);
        move();
    }

    void look(int X, int Y) override
    {
        ScoutBot::look(X, Y); // Use long-range look for any explicit look calls
    }

    bool canBeHit() override
    {
        return ScoutBot::canBeHit();
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
protected:
    const vector<string> &getUpgradeTypes() const override
    {
        static const vector<string> upgradeTypes = {"HideSemiAutoTrackBot", "JumpSemiAutoTrackBot"};
        return upgradeTypes;
    }

public:
    SemiAutoTrackBot(const string &name, int x, int y) : Robot(name, x, y), GenericRobot(name, x, y), SemiAutoBot(name, x, y), TrackBot(name, x, y) {}

    void fire(int X, int Y) override
    {
        SemiAutoBot::fire(X, Y); //fire inherit from SemiAutoBot
    }

    void think() override
    {
        TrackBot::think();
    }

    void act() override
    {
        look(0, 0);
        think();
        fire(0, 0);
        move();
    }

    void look(int X, int Y) override
    {
        TrackBot::look(X, Y); // Use long-range look for any explicit look calls
    }

    bool canBeHit() override
    {
        return TrackBot::canBeHit();
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
class ThirtyShotScoutBot : public ThirtyShotBot, public ScoutBot
{
protected:
    const vector<string> &getUpgradeTypes() const override
    {
        static const vector<string> upgradeTypes = {"HideThirtyShotScoutBot", "JumpThirtyShotScoutBot"};
        return upgradeTypes;
    }

public:
    ThirtyShotScoutBot(const string &name, int x, int y) : Robot(name, x, y), GenericRobot(name, x, y), ThirtyShotBot(name, x, y), ScoutBot(name, x, y) {}

    void fire(int X, int Y) override
    {
        ThirtyShotBot::fire(X, Y); //fire inherit from ThirtyShotBot
    }

    void think() override
    {
        ScoutBot::think();
    }

    void act() override
    {
        look(0, 0);
        think();
        fire(0, 0);
        move();
    }

    void look(int X, int Y) override
    {
        ScoutBot::look(X, Y);
    }

    bool canBeHit() override
    {
        return ScoutBot::canBeHit();
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
class ThirtyShotTrackBot : public ThirtyShotBot, public TrackBot
{
protected:
    const vector<string> &getUpgradeTypes() const override
    {
        static const vector<string> upgradeTypes = {"HideThirtyShotTrackBot", "JumpThirtyShotTrackBot"};
        return upgradeTypes;
    }

public:
    ThirtyShotTrackBot(const string &name, int x, int y) : Robot(name, x, y), GenericRobot(name, x, y), ThirtyShotBot(name, x, y), TrackBot(name, x, y) {}

    void fire(int X, int Y) override  //fire inherit from ThirtyShotBot
    {
        ThirtyShotBot::fire(X, Y);
    }

    void think() override
    {
        TrackBot::think();
    }

    void act() override
    {
        look(0, 0);
        think();
        fire(0, 0);
        move();
    }

    void look(int X, int Y) override
    {
        TrackBot::look(X, Y); // Use long-range look for any explicit look calls
    }

    bool canBeHit() override
    {
        return TrackBot::canBeHit();
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
class KnightScoutBot : public KnightBot, public ScoutBot
{
protected:
    const vector<string> &getUpgradeTypes() const override
    {
        static const vector<string> upgradeTypes = {"HideKnightScoutBot", "JumpKnightScoutBot"};
        return upgradeTypes;
    }

public:
    KnightScoutBot(const string &name, int x, int y) : Robot(name, x, y), GenericRobot(name, x, y), KnightBot(name, x, y), ScoutBot(name, x, y) {}

    void fire(int X, int Y) override
    {
        KnightBot::fire(X, Y);  //fire inherit from KnightBot
    }

    void think() override
    {
        ScoutBot::think();
    }

    void act() override
    {
        look(0, 0);
        think();
        fire(0, 0);
        move();
    }

    void look(int X, int Y) override
    {
        ScoutBot::look(X, Y); // Use long-range look for any explicit look calls
    }

    bool canBeHit() override
    {
        return ScoutBot::canBeHit();
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
class KnightTrackBot : public KnightBot, public TrackBot
{
protected:
    const vector<string> &getUpgradeTypes() const override
    {
        static const vector<string> upgradeTypes = {"HideKnightTrackBot", "JumpKnightTrackBot"};
        return upgradeTypes;
    }

public:
    KnightTrackBot(const string &name, int x, int y) : Robot(name, x, y), GenericRobot(name, x, y), KnightBot(name, x, y), TrackBot(name, x, y) {}

    void fire(int X, int Y) override
    {
        KnightBot::fire(X, Y);  //fire inherit from KnightBot
    }

    void think() override
    {
        TrackBot::think();
    }

    void act() override
    {
        look(0, 0);
        think();
        fire(0, 0);
        move();
    }

    void look(int X, int Y) override
    {
        TrackBot::look(X, Y); // Use long-range look for any explicit look calls
    }

    bool canBeHit() override
    {
        return TrackBot::canBeHit();
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
class QueenScoutBot : public QueenBot, public ScoutBot
{
protected:
    const vector<string> &getUpgradeTypes() const override
    {
        static const vector<string> upgradeTypes = {"HideQueenScoutBot", "JumpQueenScoutBot"};
        return upgradeTypes;
    }

public:
    QueenScoutBot(const string &name, int x, int y) : Robot(name, x, y), GenericRobot(name, x, y), QueenBot(name, x, y), ScoutBot(name, x, y) {}

    void fire(int X, int Y) override
    {
        QueenBot::fire(X, Y);  //fire inherit from QueenBot
    }

    void think() override
    {
        ScoutBot::think();
    }

    void act() override
    {
        look(0, 0);
        think();
        fire(0, 0);
        move();
    }

    void look(int X, int Y) override
    {
        ScoutBot::look(X, Y); // Use long-range look for any explicit look calls
    }

    bool canBeHit() override
    {
        return ScoutBot::canBeHit();
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
class QueenTrackBot : public QueenBot, public TrackBot
{
protected:
    const vector<string> &getUpgradeTypes() const override
    {
        static const vector<string> upgradeTypes = {"HideQueenTrackBot", "JumpQueenTrackBot"};
        return upgradeTypes;
    }

public:
    QueenTrackBot(const string &name, int x, int y) : Robot(name, x, y), GenericRobot(name, x, y), QueenBot(name, x, y), TrackBot(name, x, y) {}

    void fire(int X, int Y) override
    {
        QueenBot::fire(X, Y);  //fire inherit from QueenBot
    }

    void think() override
    {
        TrackBot::think();
    }

    void act() override
    {
        look(0, 0);
        think();
        fire(0, 0);
        move();
    }

    void look(int X, int Y) override
    {
        TrackBot::look(X, Y); // Use long-range look for any explicit look calls
    }

    bool canBeHit() override
    {
        return TrackBot::canBeHit();
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
class VampireScoutBot : public VampireBot, public ScoutBot
{
protected:
    const vector<string> &getUpgradeTypes() const override
    {
        static const vector<string> upgradeTypes = {"HideVampireScoutBot", "JumpVampireScoutBot"};
        return upgradeTypes;
    }

public:
    VampireScoutBot(const string &name, int x, int y) : Robot(name, x, y), GenericRobot(name, x, y), VampireBot(name, x, y), ScoutBot(name, x, y) {}

    void fire(int X, int Y) override
    {
        VampireBot::fire(X, Y);  //fire inherit from VampireBot
    }

    void think() override
    {
        ScoutBot::think();
    }

    void act() override
    {
        look(0, 0);
        think();
        fire(0, 0);
        move();
    }

    void look(int X, int Y) override
    {
        ScoutBot::look(X, Y); // Use long-range look for any explicit look calls
    }

    bool canBeHit() override
    {
        return ScoutBot::canBeHit();
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
class VampireTrackBot : public VampireBot, public TrackBot
{
protected:
    const vector<string> &getUpgradeTypes() const override
    {
        static const vector<string> upgradeTypes = {"HideVampireTrackBot", "JumpVampireTrackBot"};
        return upgradeTypes;
    }

public:
    VampireTrackBot(const string &name, int x, int y) : Robot(name, x, y), GenericRobot(name, x, y), VampireBot(name, x, y), TrackBot(name, x, y) {}

    void fire(int X, int Y) override
    {
        VampireBot::fire(X, Y);  //fire inherit from VampireBot
    }

    void think() override
    {
        TrackBot::think();
    }

    void act() override
    {
        look(0, 0);
        think();
        fire(0, 0);
        move();
    }

    void look(int X, int Y) override
    {
        TrackBot::look(X, Y); // Use long-range look for any explicit look calls
    }

    bool canBeHit() override
    {
        return TrackBot::canBeHit();
    }

    void setBattlefield(Battlefield *bf)
    {
        GenericRobot::setBattlefield(bf);
    }
};

//******************************************
// HideLongShotScoutBot
//******************************************
class HideLongShotScoutBot : public HideLongShotBot, public ScoutBot
{
protected:
    // This method returns an empty vector since HideLongShotScoutBot does not have any specific upgrades as it is the last upgradebot
    const vector<string> &getUpgradeTypes() const override
    {
        static const vector<string> upgradeTypes = {};
        return upgradeTypes;
    }

public:
    //constructor
    HideLongShotScoutBot(const string &name, int x, int y)
        : Robot(name, x, y),
          GenericRobot(name, x, y),
          HideLongShotBot(name, x, y),
          ScoutBot(name, x, y) {}

    void move() override // Override the move method to use HideLongShotBot's move
    {
        HideLongShotBot::move();  
    }

    void fire(int X, int Y) override
    {
        HideLongShotBot::fire(0, 0);
    }

    void think() override
    {
        HideLongShotBot::think();
    }

    void act() override
    {
        look(0, 0);
        think();
        fire(0, 0);
    }

    void look(int X, int Y) override // Override the look method to use ScoutBot's look as Scoutbot is look class
    {
        ScoutBot::look(X, Y);
    }

    bool canBeHit() override      
    {
        return HideLongShotBot::canBeHit(); 
    }

    void setBattlefield(Battlefield *bf)
    {
        GenericRobot::setBattlefield(bf);
    }
};

//******************************************
// HideSemiAutoScoutBot
//******************************************
class HideSemiAutoScoutBot : public HideSemiAutoBot, public ScoutBot
{
protected:
    // This method returns an empty vector since HideSemiAutoScoutBot does not have any specific upgrades as it is the last upgradebot
    const vector<string> &getUpgradeTypes() const override
    {
        static const vector<string> upgradeTypes = {};
        return upgradeTypes;
    }

public:
    //constructor
    HideSemiAutoScoutBot(const string &name, int x, int y)
        : Robot(name, x, y),
          GenericRobot(name, x, y),
          HideSemiAutoBot(name, x, y),
          ScoutBot(name, x, y) {}

    void move() override   // Override the move method to use HideSemiAutoBot's move
    {
        HideSemiAutoBot::move();
    }

    void fire(int X, int Y) override
    {
        HideSemiAutoBot::fire(0, 0);
    }

    void think() override
    {
        HideSemiAutoBot::think();
    }

    void act() override
    {
        look(0, 0);
        think();
        fire(0, 0);
    }

    void look(int X, int Y) override // Override the look method to use ScoutBot's look as Scoutbot is look class
    {
        ScoutBot::look(X, Y);
    }

    bool canBeHit() override
    {
        return HideSemiAutoBot::canBeHit();
    }

    void setBattlefield(Battlefield *bf)
    {
        GenericRobot::setBattlefield(bf);
    }
};

//******************************************
// HideThirtyShotScoutBot
//******************************************
class HideThirtyShotScoutBot : public HideThirtyShotBot, public ScoutBot
{
protected:
    //This method returns an empty vector since HideThirtyShotScoutBot does not have any specific upgrades as it is the last upgradebot
    const vector<string> &getUpgradeTypes() const override
    {
        static const vector<string> upgradeTypes = {};
        return upgradeTypes;
    }

public:
    //constructor
    HideThirtyShotScoutBot(const string &name, int x, int y)
        : Robot(name, x, y),
          GenericRobot(name, x, y),
          HideThirtyShotBot(name, x, y),
          ScoutBot(name, x, y) {}

    void move() override   // Override the move method to use HideThirtyShotBot's move
    {
        HideThirtyShotBot::move();
    }

    void fire(int X, int Y) override
    {
        HideThirtyShotBot::fire(0, 0);
    }

    void think() override   
    {
        HideThirtyShotBot::think();
    }

    void act() override
    {
        look(0, 0);
        think();
        fire(0, 0);
    }

    void look(int X, int Y) override  // Override the look method to use ScoutBot's look as Scoutbot is look class
    {
        ScoutBot::look(X, Y);
    }

    bool canBeHit() override
    {
        return HideThirtyShotBot::canBeHit();
    }

    void setBattlefield(Battlefield *bf)
    {
        GenericRobot::setBattlefield(bf);
    }
};

//******************************************
// HideKnightScoutBot
//******************************************
class HideKnightScoutBot : public HideKnightBot, public ScoutBot
{
protected:
    // This method returns an empty vector since HideKnightScoutBot does not have any specific upgrades as it is the last upgradebot
    const vector<string> &getUpgradeTypes() const override
    {
        static const vector<string> upgradeTypes = {};
        return upgradeTypes;
    }

public:
    //constructor
    HideKnightScoutBot(const string &name, int x, int y)
        : Robot(name, x, y),
          GenericRobot(name, x, y),
          HideKnightBot(name, x, y),
          ScoutBot(name, x, y) {}

    void move() override        // Override the move method to use HideKnightBot's move
    {
        HideKnightBot::move();
    }

    void fire(int X, int Y) override
    {
        HideKnightBot::fire(0, 0);
    }

    void think() override
    {
        HideKnightBot::think();
    }

    void act() override
    {
        look(0, 0);
        think();
        fire(0, 0);
    }

    void look(int X, int Y) override     // Override the look method to use ScoutBot's look as Scoutbot is look class
    {
        ScoutBot::look(X, Y);
    }

    bool canBeHit() override
    {
        return HideKnightBot::canBeHit();
    }

    void setBattlefield(Battlefield *bf)
    {
        GenericRobot::setBattlefield(bf);
    }
};

//******************************************
// HideQueenScoutBot
//******************************************

class HideQueenScoutBot : public HideQueenBot, public ScoutBot
{
protected:
    // This method returns an empty vector since HideQueenScoutBot does not have any specific upgrades as it is the last upgradebot
    const vector<string> &getUpgradeTypes() const override
    {
        static const vector<string> upgradeTypes = {};
        return upgradeTypes;
    }

public:
    //constructor
    HideQueenScoutBot(const string &name, int x, int y)
        : Robot(name, x, y),
          GenericRobot(name, x, y),
          HideQueenBot(name, x, y),
          ScoutBot(name, x, y) {}

    void move() override      // Override the move method to use HideQueenBot's move
    {
        HideQueenBot::move();
    }

    void fire(int X, int Y) override
    {
        HideQueenBot::fire(0, 0);
    }

    void think() override
    {
        HideQueenBot::think();
    }

    void act() override
    {
        look(0, 0);
        think();
        fire(0, 0);
    }

    void look(int X, int Y) override     // Override the look method to use ScoutBot's look as Scoutbot is look class
    {
        ScoutBot::look(X, Y);
    }

    bool canBeHit() override
    {
        return HideQueenBot::canBeHit();
    }

    void setBattlefield(Battlefield *bf)
    {
        GenericRobot::setBattlefield(bf);
    }
};

//******************************************
// HideVampireScoutBot
//******************************************
class HideVampireScoutBot : public HideVampireBot, public ScoutBot
{
protected:
    // This method returns an empty vector since HideVampireScoutBot does not have any specific upgrades as it is the last upgradebot
    const vector<string> &getUpgradeTypes() const override
    {
        static const vector<string> upgradeTypes = {};
        return upgradeTypes;
    }

public:
    HideVampireScoutBot(const string &name, int x, int y)
        : Robot(name, x, y),
          GenericRobot(name, x, y),
          HideVampireBot(name, x, y),
          ScoutBot(name, x, y) {}

    void move() override   // Override the move method to use HideVampireBot's move
    {
        HideVampireBot::move();
    }

    void fire(int X, int Y) override
    {
        HideVampireBot::fire(0, 0);
    }

    void think() override
    {
        HideVampireBot::think();
    }

    void act() override
    {
        look(0, 0);
        think();
        fire(0, 0);
    }

    void look(int X, int Y) override  // Override the look method to use ScoutBot's look as Scoutbot is look class
    {
        ScoutBot::look(X, Y);
    }

    bool canBeHit() override
    {
        return HideVampireBot::canBeHit();
    }

    void setBattlefield(Battlefield *bf)
    {
        GenericRobot::setBattlefield(bf);
    }
};

//******************************************
// HideLongShotTrackBot
//******************************************
class HideLongShotTrackBot : public HideLongShotBot, public TrackBot
{
protected:
    // This method returns an empty vector since HideLongShotTrackBot does not have any specific upgrades as it is the last upgradebot
    const vector<string> &getUpgradeTypes() const override
    {
        static const vector<string> upgradeTypes = {};
        return upgradeTypes;
    }

public:
    //constructor
    HideLongShotTrackBot(const string &name, int x, int y)
        : Robot(name, x, y),
          GenericRobot(name, x, y),
          HideLongShotBot(name, x, y),
          TrackBot(name, x, y) {}

    void move() override         // Override the move method to use HideLongShotBot's move
    {
        HideLongShotBot::move();
    }

    void fire(int X, int Y) override
    {
        HideLongShotBot::fire(0, 0);
    }

    void think() override
    {
        HideLongShotBot::think();
    }

    void act() override
    {
        look(0, 0);
        think();
        fire(0, 0);
    }

    void look(int X, int Y) override  // Override the look method to use TrackBot's look as Trackbot is look class
    {
        TrackBot::look(X, Y);
    }

    bool canBeHit() override
    {
        return HideLongShotBot::canBeHit();
    }

    void setBattlefield(Battlefield *bf)
    {
        GenericRobot::setBattlefield(bf);
    }
};

//******************************************
// HideSemiAutoTrackBot
//******************************************
class HideSemiAutoTrackBot : public HideSemiAutoBot, public TrackBot
{
protected:
    // This method returns an empty vector since HideSemiAutoTrackBot does not have any specific upgrades as it is the last upgradebot
    const vector<string> &getUpgradeTypes() const override
    {
        static const vector<string> upgradeTypes = {};
        return upgradeTypes;
    }

public:
    //constructor
    HideSemiAutoTrackBot(const string &name, int x, int y)
        : Robot(name, x, y),
          GenericRobot(name, x, y),
          HideSemiAutoBot(name, x, y),
          TrackBot(name, x, y) {}

    void move() override  // Override the move method to use HideSemiAutoBot's move
    {
        HideSemiAutoBot::move();
    }

    void fire(int X, int Y) override
    {
        HideSemiAutoBot::fire(0, 0);
    }

    void think() override
    {
        HideSemiAutoBot::think();
    }

    void act() override
    {
        look(0, 0);
        think();
        fire(0, 0);
    }

    void look(int X, int Y) override    // Override the look method to use TrackBot's look as Trackbot is look class
    {
        TrackBot::look(X, Y);
    }

    bool canBeHit() override
    {
        return HideSemiAutoBot::canBeHit();
    }

    void setBattlefield(Battlefield *bf)
    {
        GenericRobot::setBattlefield(bf);
    }
};

//******************************************
// HideThirtyShotTrackBot
//******************************************
class HideThirtyShotTrackBot : public HideThirtyShotBot, public TrackBot
{
protected:
    // This method returns an empty vector since HideThirtyShotTrackBot does not have any specific upgrades as it is the last upgradebot
    const vector<string> &getUpgradeTypes() const override
    {
        static const vector<string> upgradeTypes = {};
        return upgradeTypes;
    }

public:
    //constructor
    HideThirtyShotTrackBot(const string &name, int x, int y)
        : Robot(name, x, y),
          GenericRobot(name, x, y),
          HideThirtyShotBot(name, x, y),
          TrackBot(name, x, y) {}

    void move() override    // Override the move method to use HideThirtyShotBot's move
    {
        HideThirtyShotBot::move();
    }

    void fire(int X, int Y) override
    {
        HideThirtyShotBot::fire(0, 0);
    }

    void think() override
    {
        HideThirtyShotBot::think();
    }

    void act() override
    {
        look(0, 0);
        think();
        fire(0, 0);
    }

    void look(int X, int Y) override   // Override the look method to use TrackBot's look as Trackbot is look class
    {
        TrackBot::look(X, Y);
    }

    bool canBeHit() override
    {
        return HideThirtyShotBot::canBeHit();
    }

    void setBattlefield(Battlefield *bf)
    {
        GenericRobot::setBattlefield(bf);
    }
};

//******************************************
// HideKnightTrackBot
//******************************************
class HideKnightTrackBot : public HideKnightBot, public TrackBot
{
protected:
    // This method returns an empty vector since HideKnightTrackBot does not have any specific upgrades as it is the last upgradebot
    const vector<string> &getUpgradeTypes() const override
    {
        static const vector<string> upgradeTypes = {};
        return upgradeTypes;
    }

public:
    //constructor
    HideKnightTrackBot(const string &name, int x, int y)
        : Robot(name, x, y),
          GenericRobot(name, x, y),
          HideKnightBot(name, x, y),
          TrackBot(name, x, y) {}

    void move() override     // Override the move method to use HideKnightBot's move
    {
        HideKnightBot::move();
    }

    void fire(int X, int Y) override
    {
        HideKnightBot::fire(0, 0);
    }

    void think() override
    {
        HideKnightBot::think();
    }

    void act() override
    {
        look(0, 0);
        think();
        fire(0, 0);
    }

    void look(int X, int Y) override   // Override the look method to use TrackBot's look as Trackbot is look class
    {
        TrackBot::look(X, Y);
    }

    bool canBeHit() override
    {
        return HideKnightBot::canBeHit();
    }

    void setBattlefield(Battlefield *bf)
    {
        GenericRobot::setBattlefield(bf);
    }
};
//******************************************
// HideQueenTrackBot
//******************************************
class HideQueenTrackBot : public HideQueenBot, public TrackBot
{
protected:
    // This method returns an empty vector since HideQueenTrackBot does not have any specific upgrades as it is the last upgradebot
    const vector<string> &getUpgradeTypes() const override
    {
        static const vector<string> upgradeTypes = {};
        return upgradeTypes;
    }

public:
    //constructor
    HideQueenTrackBot(const string &name, int x, int y)
        : Robot(name, x, y),
          GenericRobot(name, x, y),
          HideQueenBot(name, x, y),
          TrackBot(name, x, y) {}

    void move() override         // Override the move method to use HideQueenBot's move
    {
        HideQueenBot::move();
    }

    void fire(int X, int Y) override
    {
        HideQueenBot::fire(0, 0);
    }

    void think() override
    {
        HideQueenBot::think();
    }

    void act() override
    {
        look(0, 0);
        think();
        fire(0, 0);
    }

    void look(int X, int Y) override      // Override the look method to use TrackBot's look as Trackbot is look class
    {
        TrackBot::look(X, Y);
    }

    bool canBeHit() override
    {
        return HideQueenBot::canBeHit();
    }

    void setBattlefield(Battlefield *bf)
    {
        GenericRobot::setBattlefield(bf);
    }
};

//******************************************
// HideVampireTrackBot
//******************************************
class HideVampireTrackBot : public HideVampireBot, public TrackBot
{
protected:
    // This method returns an empty vector since HideVampireTrackBot does not have any specific upgrades as it is the last upgradebot
    const vector<string> &getUpgradeTypes() const override
    {
        static const vector<string> upgradeTypes = {};
        return upgradeTypes;
    }

public:
    //constructor
    HideVampireTrackBot(const string &name, int x, int y)
        : Robot(name, x, y),
          GenericRobot(name, x, y),
          HideVampireBot(name, x, y),
          TrackBot(name, x, y) {}

    void move() override       // Override the move method to use HideVampireBot's move
    {
        HideVampireBot::move();
    }

    void fire(int X, int Y) override
    {
        HideVampireBot::fire(0, 0);
    }

    void think() override
    {
        HideVampireBot::think();
    }

    void act() override
    {
        look(0, 0);
        think();
        fire(0, 0);
    }

    void look(int X, int Y) override      // Override the look method to use TrackBot's look as Trackbot is look class
    {
        TrackBot::look(X, Y);
    }

    bool canBeHit() override
    {
        return HideVampireBot::canBeHit();
    }

    void setBattlefield(Battlefield *bf)
    {
        GenericRobot::setBattlefield(bf);
    }
};

//******************************************
// JumpLongShotScoutBot
//******************************************
class JumpLongShotScoutBot : public JumpLongShotBot, public ScoutBot
{
protected:
    // This method returns an empty vector since JumpLongShotScoutBot does not have any specific upgrades as it is the last upgradebot
    const vector<string> &getUpgradeTypes() const override
    {
        static const vector<string> upgradeTypes = {};
        return upgradeTypes;
    }

public:
    //constructor
    JumpLongShotScoutBot(const string &name, int x, int y)
        : Robot(name, x, y),
          GenericRobot(name, x, y),
          JumpLongShotBot(name, x, y),
          ScoutBot(name, x, y) {}

    void move() override           // Override the move method to use JumpLongShotBot's move
    {
        JumpLongShotBot::move();
    }

    void fire(int X, int Y) override
    {
        JumpLongShotBot::fire(0, 0);
    }

    void think() override
    {
        JumpLongShotBot::think();
    }

    void act() override
    {
        look(0, 0);
        think();
        fire(0, 0);
    }

    void look(int X, int Y) override         // Override the look method to use ScoutBot's look as Scoutbot is look class
    {
        ScoutBot::look(X, Y);
    }

    bool canBeHit() override
    {
        return JumpLongShotBot::canBeHit();
    }

    void setBattlefield(Battlefield *bf)
    {
        GenericRobot::setBattlefield(bf);
    }
};

//******************************************
// JumpSemiAutoScoutBot
//******************************************
class JumpSemiAutoScoutBot : public JumpSemiAutoBot, public ScoutBot
{
protected:
    // This method returns an empty vector since JumpSemiAutoScoutBot does not have any specific upgrades as it is the last upgradebot
    const vector<string> &getUpgradeTypes() const override
    {
        static const vector<string> upgradeTypes = {};
        return upgradeTypes;
    }

public:
    //constructor
    JumpSemiAutoScoutBot(const string &name, int x, int y)
        : Robot(name, x, y),
          GenericRobot(name, x, y),
          JumpSemiAutoBot(name, x, y),
          ScoutBot(name, x, y) {}

    void move() override      // Override the move method to use JumpSemiAutoBot's move
    {
        JumpSemiAutoBot::move();
    }

    void fire(int X, int Y) override
    {
        JumpSemiAutoBot::fire(0, 0);
    }

    void think() override
    {
        JumpSemiAutoBot::think();
    }

    void act() override
    {
        look(0, 0);
        think();
        fire(0, 0);
    }

    void look(int X, int Y) override  // Override the look method to use ScoutBot's look as Scoutbot is look class
    {
        ScoutBot::look(X, Y);
    }

    bool canBeHit() override
    {
        return JumpSemiAutoBot::canBeHit();
    }

    void setBattlefield(Battlefield *bf)
    {
        GenericRobot::setBattlefield(bf);
    }
};

//******************************************
// JumpThirtyShotScoutBot
//******************************************
class JumpThirtyShotScoutBot : public JumpThirtyShotBot, public ScoutBot
{
protected:
    // This method returns an empty vector since JumpThirtyShotScoutBot does not have any specific upgrades as it is the last upgradebot
    const vector<string> &getUpgradeTypes() const override
    {
        static const vector<string> upgradeTypes = {};
        return upgradeTypes;
    }

public:
    //constructor
    JumpThirtyShotScoutBot(const string &name, int x, int y)
        : Robot(name, x, y),
          GenericRobot(name, x, y),
          JumpThirtyShotBot(name, x, y),
          ScoutBot(name, x, y) {}

    void move() override     // Override the move method to use JumpThirtyShotBot's move
    {
        JumpThirtyShotBot::move();
    }

    void fire(int X, int Y) override
    {
        JumpThirtyShotBot::fire(0, 0);
    }

    void think() override
    {
        JumpThirtyShotBot::think();
    }

    void act() override
    {
        look(0, 0);
        think();
        fire(0, 0);
    }

    void look(int X, int Y) override    // Override the look method to use ScoutBot's look as Scoutbot is look class
    {
        ScoutBot::look(X, Y);
    }

    bool canBeHit() override
    {
        return JumpThirtyShotBot::canBeHit();
    }

    void setBattlefield(Battlefield *bf)
    {
        GenericRobot::setBattlefield(bf);
    }
};

//******************************************
// JumpKnightScoutBot
//******************************************
class JumpKnightScoutBot : public JumpKnightBot, public ScoutBot
{
protected:
    // This method returns an empty vector since JumpKnightScoutBot does not have any specific upgrades as it is the last upgradebot
    const vector<string> &getUpgradeTypes() const override
    {
        static const vector<string> upgradeTypes = {};
        return upgradeTypes;
    }

public:
    //constructor
    JumpKnightScoutBot(const string &name, int x, int y)
        : Robot(name, x, y),
          GenericRobot(name, x, y),
          JumpKnightBot(name, x, y),
          ScoutBot(name, x, y) {}

    void move() override       // Override the move method to use JumpKnightBot's move
    {
        JumpKnightBot::move();
    }

    void fire(int X, int Y) override
    {
        JumpKnightBot::fire(0, 0);
    }

    void think() override
    {
        JumpKnightBot::think();
    }

    void act() override
    {
        look(0, 0);
        think();
        fire(0, 0);
    }

    void look(int X, int Y) override    // Override the look method to use ScoutBot's look as Scoutbot is look class
    {
        ScoutBot::look(X, Y);
    }

    bool canBeHit() override
    {
        return JumpKnightBot::canBeHit();
    }

    void setBattlefield(Battlefield *bf)
    {
        GenericRobot::setBattlefield(bf);
    }
};

//******************************************
// JumpQueenScoutBot
//******************************************

class JumpQueenScoutBot : public JumpQueenBot, public ScoutBot
{
protected:
    // This method returns an empty vector since JumpQueenScoutBot does not have any specific upgrades as it is the last upgradebot
    const vector<string> &getUpgradeTypes() const override
    {
        static const vector<string> upgradeTypes = {};
        return upgradeTypes;
    }

public:
    //constructor
    JumpQueenScoutBot(const string &name, int x, int y)
        : Robot(name, x, y),
          GenericRobot(name, x, y),
          JumpQueenBot(name, x, y),
          ScoutBot(name, x, y) {}

    void move() override      // Override the move method to use JumpQueenBot's move
    {
        JumpQueenBot::move();
    }

    void fire(int X, int Y) override
    {
        JumpQueenBot::fire(0, 0);
    }

    void think() override
    {
        JumpQueenBot::think();
    }

    void act() override
    {
        look(0, 0);
        think();
        fire(0, 0);
    }

    void look(int X, int Y) override     // Override the look method to use ScoutBot's look as Scoutbot is look class
    {
        ScoutBot::look(X, Y);
    }

    bool canBeHit() override
    {
        return JumpQueenBot::canBeHit();
    }

    void setBattlefield(Battlefield *bf)
    {
        GenericRobot::setBattlefield(bf);
    }
};

//******************************************
// JumpVampireScoutBot
//******************************************
class JumpVampireScoutBot : public JumpVampireBot, public ScoutBot
{
protected:
    // This method returns an empty vector since JumpVampireScoutBot does not have any specific upgrades as it is the last upgradebot
    const vector<string> &getUpgradeTypes() const override
    {
        static const vector<string> upgradeTypes = {};
        return upgradeTypes;
    }

public:
    //constructor
    JumpVampireScoutBot(const string &name, int x, int y)   
        : Robot(name, x, y),
          GenericRobot(name, x, y),
          JumpVampireBot(name, x, y),
          ScoutBot(name, x, y) {}

    void move() override     // Override the move method to use JumpVampireBot's move
    {
        JumpVampireBot::move();
    }

    void fire(int X, int Y) override
    {
        JumpVampireBot::fire(0, 0);
    }

    void think() override
    {
        JumpVampireBot::think();
    }

    void act() override
    {
        look(0, 0);
        think();
        fire(0, 0);
    }

    void look(int X, int Y) override    // Override the look method to use ScoutBot's look as Scoutbot is look class
    {
        ScoutBot::look(X, Y);
    }

    bool canBeHit() override
    {
        return JumpVampireBot::canBeHit();
    }

    void setBattlefield(Battlefield *bf)
    {
        GenericRobot::setBattlefield(bf);
    }
};

//******************************************
// JumpLongShotTrackBot
//******************************************

class JumpLongShotTrackBot : public JumpLongShotBot, public TrackBot
{
protected:
    // This method returns an empty vector since JumpLongShotTrackBot does not have any specific upgrades as it is the last upgradebot
    const vector<string> &getUpgradeTypes() const override
    {
        static const vector<string> upgradeTypes = {};
        return upgradeTypes;
    }

public:
    //constructor
    JumpLongShotTrackBot(const string &name, int x, int y)
        : Robot(name, x, y),
          GenericRobot(name, x, y),
          JumpLongShotBot(name, x, y),
          TrackBot(name, x, y) {}

    void move() override     // Override the move method to use JumpLongShotBot's move
    {
        JumpLongShotBot::move();
    }

    void fire(int X, int Y) override
    {
        JumpLongShotBot::fire(0, 0);
    }

    void think() override
    {
        JumpLongShotBot::think();
    }

    void act() override
    {
        look(0, 0);
        think();
        fire(0, 0);
    }

    void look(int X, int Y) override    // Override the look method to use TrackBot's look as Trackbot is look class
    {
        TrackBot::look(X, Y);
    }

    bool canBeHit() override
    {
        return JumpLongShotBot::canBeHit();
    }

    void setBattlefield(Battlefield *bf)
    {
        GenericRobot::setBattlefield(bf);
    }
};

//******************************************
// JumpSemiAutoTrackBot
//******************************************

class JumpSemiAutoTrackBot : public JumpSemiAutoBot, public TrackBot
{
protected:
    // This method returns an empty vector since JumpSemiAutoTrackBot does not have any specific upgrades as it is the last upgradebot
    const vector<string> &getUpgradeTypes() const override
    {
        static const vector<string> upgradeTypes = {};
        return upgradeTypes;
    }

public:
    //constructor
    JumpSemiAutoTrackBot(const string &name, int x, int y)
        : Robot(name, x, y),
          GenericRobot(name, x, y),
          JumpSemiAutoBot(name, x, y),
          TrackBot(name, x, y) {}

    void move() override      // Override the move method to use JumpSemiAutoBot's move
    {
        JumpSemiAutoBot::move();
    }

    void fire(int X, int Y) override
    {
        JumpSemiAutoBot::fire(0, 0);
    }

    void think() override
    {
        JumpSemiAutoBot::think();
    }

    void act() override      
    {
        look(0, 0);
        think();
        fire(0, 0);
    }

    void look(int X, int Y) override  // Override the look method to use TrackBot's look as Trackbot is look class
    {
        TrackBot::look(X, Y);
    }

    bool canBeHit() override
    {
        return JumpSemiAutoBot::canBeHit();
    }

    void setBattlefield(Battlefield *bf)
    {
        GenericRobot::setBattlefield(bf);
    }
};

//******************************************
// JumpThirtyShotTrackBot
//******************************************
class JumpThirtyShotTrackBot : public JumpThirtyShotBot, public TrackBot
{
protected:
    // This method returns an empty vector since JumpThirtyShotTrackBot does not have any specific upgrades as it is the last upgradebot
    const vector<string> &getUpgradeTypes() const override
    {
        static const vector<string> upgradeTypes = {};
        return upgradeTypes;
    }

public:
    //constructor
    JumpThirtyShotTrackBot(const string &name, int x, int y)
        : Robot(name, x, y),
          GenericRobot(name, x, y),
          JumpThirtyShotBot(name, x, y),
          TrackBot(name, x, y) {}

    void move() override    // Override the move method to use JumpThirtyShotBot's move
    {
        JumpThirtyShotBot::move();
    }

    void fire(int X, int Y) override
    {
        JumpThirtyShotBot::fire(0, 0);
    }

    void think() override
    {
        JumpThirtyShotBot::think();
    }

    void act() override
    {
        look(0, 0);
        think();
        fire(0, 0);
    }

    void look(int X, int Y) override    // Override the look method to use TrackBot's look as Trackbot is look class
    {
        TrackBot::look(X, Y);
    }

    bool canBeHit() override
    {
        return JumpThirtyShotBot::canBeHit();
    }

    void setBattlefield(Battlefield *bf)
    {
        GenericRobot::setBattlefield(bf);
    }
};

//******************************************
// JumpKnightTrackBot
//******************************************
class JumpKnightTrackBot : public JumpKnightBot, public TrackBot
{
protected:
    // This method returns an empty vector since JumpKnightTrackBot does not have any specific upgrades as it is the last upgradebot
    const vector<string> &getUpgradeTypes() const override
    {
        static const vector<string> upgradeTypes = {};
        return upgradeTypes;
    }

public:
    //constructor
    JumpKnightTrackBot(const string &name, int x, int y)
        : Robot(name, x, y),
          GenericRobot(name, x, y),
          JumpKnightBot(name, x, y),
          TrackBot(name, x, y) {}

    void move() override     // Override the move method to use JumpKnightBot's move
    {
        JumpKnightBot::move();
    }

    void fire(int X, int Y) override
    {
        JumpKnightBot::fire(0, 0);
    }

    void think() override
    {
        JumpKnightBot::think();
    }

    void act() override
    {
        look(0, 0);
        think();
        fire(0, 0);
    }

    void look(int X, int Y) override    // Override the look method to use TrackBot's look as Trackbot is look class
    {
        TrackBot::look(X, Y);
    }

    bool canBeHit() override
    {
        return JumpKnightBot::canBeHit();
    }

    void setBattlefield(Battlefield *bf)
    {
        GenericRobot::setBattlefield(bf);
    }
};
//******************************************
// JumpQueenTrackBot
//******************************************
class JumpQueenTrackBot : public JumpQueenBot, public TrackBot
{
protected:
    // This method returns an empty vector since JumpQueenTrackBot does not have any specific upgrades as it is the last upgradebot
    const vector<string> &getUpgradeTypes() const override
    {
        static const vector<string> upgradeTypes = {};
        return upgradeTypes;
    }

public:
    //constructor
    JumpQueenTrackBot(const string &name, int x, int y)
        : Robot(name, x, y),
          GenericRobot(name, x, y),
          JumpQueenBot(name, x, y),
          TrackBot(name, x, y) {}

    void move() override      // Override the move method to use JumpQueenBot's move
    {
        JumpQueenBot::move();
    }

    void fire(int X, int Y) override
    {
        JumpQueenBot::fire(0, 0);
    }

    void think() override
    {
        JumpQueenBot::think();
    }

    void act() override
    {
        look(0, 0);
        think();
        fire(0, 0);
    }

    void look(int X, int Y) override    // Override the look method to use TrackBot's look as Trackbot is look class
    {
        TrackBot::look(X, Y);
    }

    bool canBeHit() override
    {
        return JumpQueenBot::canBeHit();
    }

    void setBattlefield(Battlefield *bf)
    {
        GenericRobot::setBattlefield(bf);
    }
};

//******************************************
// JumpVampireTrackBot
//******************************************
class JumpVampireTrackBot : public JumpVampireBot, public TrackBot
{
protected:
    // This method returns an empty vector since JumpVampireTrackBot does not have any specific upgrades as it is the last upgradebot
    const vector<string> &getUpgradeTypes() const override
    {
        static const vector<string> upgradeTypes = {};
        return upgradeTypes;
    }

public:
    //constructor
    JumpVampireTrackBot(const string &name, int x, int y)
        : Robot(name, x, y),
          GenericRobot(name, x, y),
          JumpVampireBot(name, x, y),
          TrackBot(name, x, y) {}

    void move() override     // Override the move method to use JumpVampireBot's move
    {
        JumpVampireBot::move();
    }

    void fire(int X, int Y) override
    {
        JumpVampireBot::fire(0, 0);
    }

    void think() override    //
    {
        JumpVampireBot::think();
    }

    void act() override
    {
        look(0, 0);
        think();
        fire(0, 0);
    }
 
    void look(int X, int Y) override      // Override the look method to use TrackBot's look as Trackbot is look class
    {
        TrackBot::look(X, Y);
    }

    bool canBeHit() override
    {
        return JumpVampireBot::canBeHit();
    }

    void setBattlefield(Battlefield *bf)
    {
        GenericRobot::setBattlefield(bf);
    }
};

//******************************************
// simulationStep member function of Battlefield class (declared later to avoid issues with code not seeing each other when they need to)
//******************************************

void Battlefield::simulationStep(int stepNumber) // Added stepNumber parameter
{
    
    // Clear queuedThisRound at the start of each round to ensure it only contains robots queued in the current round
    queuedThisRound.clear();
    // Handle upgrades first
    for (size_t i = 0; i < listOfRobots.size(); ++i)
    {
        GenericRobot *gen = dynamic_cast<GenericRobot *>(listOfRobots[i]);
        if (gen && gen->PendingUpgrade())
        {
            string type = gen->getUpgradeType();
            Robot *upgraded = nullptr;

            //$$
            // Create the new upgraded robot based on the pending upgrade type
            if (type == "GenericRobot")
                upgraded = new GenericRobot(gen->getName(), gen->getX(), gen->getY());
            else if (type == "HideBot")
                upgraded = new HideBot(gen->getName(), gen->getX(), gen->getY());
            else if (type == "JumpBot")
                upgraded = new JumpBot(gen->getName(), gen->getX(), gen->getY());
            else if (type == "SemiAutoBot")
                upgraded = new SemiAutoBot(gen->getName(), gen->getX(), gen->getY());
            else if (type == "LongShotBot")
                upgraded = new LongShotBot(gen->getName(), gen->getX(), gen->getY());
            else if (type == "ThirtyShotBot")
                upgraded = new ThirtyShotBot(gen->getName(), gen->getX(), gen->getY());
            else if (type == "KnightBot")
                upgraded = new KnightBot(gen->getName(), gen->getX(), gen->getY());
            else if (type == "QueenBot")
                upgraded = new QueenBot(gen->getName(), gen->getX(), gen->getY());
            else if (type == "VampireBot")
                upgraded = new VampireBot(gen->getName(), gen->getX(), gen->getY());
            else if (type == "ScoutBot")
                upgraded = new ScoutBot(gen->getName(), gen->getX(), gen->getY());
            else if (type == "TrackBot")
                upgraded = new TrackBot(gen->getName(), gen->getX(), gen->getY());

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
            {
                upgraded = new VampireTrackBot(gen->getName(), gen->getX(), gen->getY());
            }

            else if (type == "HideLongShotScoutBot")
                upgraded = new HideLongShotScoutBot(gen->getName(), gen->getX(), gen->getY());
            else if (type == "HideSemiAutoScoutBot")
                upgraded = new HideSemiAutoScoutBot(gen->getName(), gen->getX(), gen->getY());
            else if (type == "HideThirtyShotScoutBot")
                upgraded = new HideThirtyShotScoutBot(gen->getName(), gen->getX(), gen->getY());
            else if (type == "HideKnightScoutBot")
                upgraded = new HideKnightScoutBot(gen->getName(), gen->getX(), gen->getY());
            else if (type == "HideQueenScoutBot")
                upgraded = new HideQueenScoutBot(gen->getName(), gen->getX(), gen->getY());
            else if (type == "HideVampireScoutBot")
                upgraded = new HideVampireScoutBot(gen->getName(), gen->getX(), gen->getY());
            else if (type == "HideLongShotTrackBot")
                upgraded = new HideLongShotTrackBot(gen->getName(), gen->getX(), gen->getY());
            else if (type == "HideSemiAutoTrackBot")
                upgraded = new HideSemiAutoTrackBot(gen->getName(), gen->getX(), gen->getY());
            else if (type == "HideThirtyShotTrackBot")
                upgraded = new HideThirtyShotTrackBot(gen->getName(), gen->getX(), gen->getY());
            else if (type == "HideKnightTrackBot")
                upgraded = new HideKnightTrackBot(gen->getName(), gen->getX(), gen->getY());
            else if (type == "HideQueenTrackBot")
                upgraded = new HideQueenTrackBot(gen->getName(), gen->getX(), gen->getY());
            else if (type == "HideVampireTrackBot")
                upgraded = new HideVampireTrackBot(gen->getName(), gen->getX(), gen->getY());

            else if (type == "JumpLongShotScoutBot")
                upgraded = new JumpLongShotScoutBot(gen->getName(), gen->getX(), gen->getY());
            else if (type == "JumpSemiAutoScoutBot")
                upgraded = new JumpSemiAutoScoutBot(gen->getName(), gen->getX(), gen->getY());
            else if (type == "JumpThirtyShotScoutBot")
                upgraded = new JumpThirtyShotScoutBot(gen->getName(), gen->getX(), gen->getY());
            else if (type == "JumpKnightScoutBot")
                upgraded = new JumpKnightScoutBot(gen->getName(), gen->getX(), gen->getY());
            else if (type == "JumpQueenScoutBot")
                upgraded = new JumpQueenScoutBot(gen->getName(), gen->getX(), gen->getY());
            else if (type == "JumpVampireScoutBot")
                upgraded = new JumpVampireScoutBot(gen->getName(), gen->getX(), gen->getY());
            else if (type == "JumpLongShotTrackBot")
                upgraded = new JumpLongShotTrackBot(gen->getName(), gen->getX(), gen->getY());
            else if (type == "JumpSemiAutoTrackBot")
                upgraded = new JumpSemiAutoTrackBot(gen->getName(), gen->getX(), gen->getY());
            else if (type == "JumpThirtyShotTrackBot")
                upgraded = new JumpThirtyShotTrackBot(gen->getName(), gen->getX(), gen->getY());
            else if (type == "JumpKnightTrackBot")
                upgraded = new JumpKnightTrackBot(gen->getName(), gen->getX(), gen->getY());
            else if (type == "JumpQueenTrackBot")
                upgraded = new JumpQueenTrackBot(gen->getName(), gen->getX(), gen->getY());
            else if (type == "JumpVampireTrackBot")
                upgraded = new JumpVampireTrackBot(gen->getName(), gen->getX(), gen->getY());

            if (upgraded)
            {
                upgraded->setLives(gen->getLives()); // Copy lives from the old robot
                upgraded->initializeFrom(gen);    // Copy other necessary attributes
                                                  // This is where the ammo is correctly set for ThirtyShotBot
                GenericRobot *upGen = dynamic_cast<GenericRobot *>(upgraded);
                if (upGen)
                {
                    upGen->clearPendingUpgrade();  // Clear the pending upgrade flag
                    upGen->setBattlefield(this);  // Ensure battlefield context is set
                    upGen->resetActionFlags();     // Reset action flags for the new robot
                }
                removeRobotFromGrid(gen);   // Remove the old robot from the grid
                placeRobot(upgraded, gen->getX(), gen->getY());  // Place the upgraded robot at the same position
                delete gen;    // Delete the old robot to avoid memory leaks
                listOfRobots[i] = upgraded;   // Replace the old robot in the list with the upgraded one
                logger << upgraded->getName() << " has upgraded to " << type << "!" << endl;
            }
        }
    }

    // Log robot status after upgrades have been processed for this step
    logger << "Robot Status before Step " << stepNumber << ":" << endl;
    for (Robot *robot : listOfRobots)
    {
        string typeName;
        int currentAmmo = 0;

        GenericRobot *gen = dynamic_cast<GenericRobot *>(robot);   
        if (gen)
        {
            // Dynamically determine type for logging (replicated from your main's logic)
            if (dynamic_cast<HideLongShotScoutBot *>(robot))
                typeName = "HideLongShotScoutBot";
            else if (dynamic_cast<HideSemiAutoScoutBot *>(robot))
                typeName = "HideSemiAutoScoutBot";
            else if (dynamic_cast<HideThirtyShotScoutBot *>(robot))
                typeName = "HideThirtyShotScoutBot";
            else if (dynamic_cast<HideKnightScoutBot *>(robot))
                typeName = "HideKnightScoutBot";
            else if (dynamic_cast<HideQueenScoutBot *>(robot))
                typeName = "HideQueenScoutBot";
            else if (dynamic_cast<HideVampireScoutBot *>(robot))
                typeName = "HideVampireScoutBot";
            else if (dynamic_cast<HideLongShotTrackBot *>(robot))
                typeName = "HideLongShotTrackBot";
            else if (dynamic_cast<HideSemiAutoTrackBot *>(robot))
                typeName = "HideSemiAutoTrackBot";
            else if (dynamic_cast<HideThirtyShotTrackBot *>(robot))
                typeName = "HideThirtyShotTrackBot";
            else if (dynamic_cast<HideKnightTrackBot *>(robot))
                typeName = "HideKnightTrackBot";
            else if (dynamic_cast<HideQueenTrackBot *>(robot))
                typeName = "HideQueenTrackBot";
            else if (dynamic_cast<HideVampireTrackBot *>(robot))
                typeName = "HideVampireTrackBot";

            else if (dynamic_cast<JumpLongShotScoutBot *>(robot))
                typeName = "JumpLongShotScoutBot";
            else if (dynamic_cast<JumpSemiAutoScoutBot *>(robot))
                typeName = "JumpSemiAutoScoutBot";
            else if (dynamic_cast<JumpThirtyShotScoutBot *>(robot))
                typeName = "JumpThirtyShotScoutBot";
            else if (dynamic_cast<JumpKnightScoutBot *>(robot))
                typeName = "JumpKnightScoutBot";
            else if (dynamic_cast<JumpQueenScoutBot *>(robot))
                typeName = "JumpQueenScoutBot";
            else if (dynamic_cast<JumpVampireScoutBot *>(robot))
                typeName = "JumpVampireScoutBot";
            else if (dynamic_cast<JumpLongShotTrackBot *>(robot))
                typeName = "JumpLongShotTrackBot";
            else if (dynamic_cast<JumpSemiAutoTrackBot *>(robot))
                typeName = "JumpSemiAutoTrackBot";
            else if (dynamic_cast<JumpThirtyShotTrackBot *>(robot))
                typeName = "JumpThirtyShotTrackBot";
            else if (dynamic_cast<JumpKnightTrackBot *>(robot))
                typeName = "JumpKnightTrackBot";
            else if (dynamic_cast<JumpQueenTrackBot *>(robot))
                typeName = "JumpQueenTrackBot";
            else if (dynamic_cast<JumpVampireTrackBot *>(robot))
                typeName = "JumpVampireTrackBot";

            else if (dynamic_cast<HideLongShotBot *>(robot))
                typeName = "HideLongShotBot";
            else if (dynamic_cast<HideSemiAutoBot *>(robot))
                typeName = "HideSemiAutoBot";
            else if (dynamic_cast<HideThirtyShotBot *>(robot))
                typeName = "HideThirtyShotBot";
            else if (dynamic_cast<HideKnightBot *>(robot))
                typeName = "HideKnightBot";
            else if (dynamic_cast<HideQueenBot *>(robot))
                typeName = "HideQueenBot";
            else if (dynamic_cast<HideVampireBot *>(robot))
                typeName = "HideVampireBot";
            else if (dynamic_cast<HideScoutBot *>(robot))
                typeName = "HideScoutBot";
            else if (dynamic_cast<HideTrackBot *>(robot))
                typeName = "HideTrackBot";
            else if (dynamic_cast<JumpLongShotBot *>(robot))
                typeName = "JumpLongShotBot";
            else if (dynamic_cast<JumpSemiAutoBot *>(robot))
                typeName = "JumpSemiAutoBot";
            else if (dynamic_cast<JumpThirtyShotBot *>(robot))
                typeName = "JumpThirtyShotBot";
            else if (dynamic_cast<JumpKnightBot *>(robot))
                typeName = "JumpKnightBot"; 
            else if (dynamic_cast<JumpQueenBot *>(robot))
                typeName = "JumpQueenBot";
            else if (dynamic_cast<JumpVampireBot *>(robot))
                typeName = "JumpVampireBot";
            else if (dynamic_cast<JumpScoutBot *>(robot))
                typeName = "JumpScoutBot";
            else if (dynamic_cast<JumpTrackBot *>(robot))
                typeName = "JumpTrackBot";
            else if (dynamic_cast<LongShotScoutBot *>(robot))
                typeName = "LongShotScoutBot";
            else if (dynamic_cast<LongShotTrackBot *>(robot))
                typeName = "LongShotTrackBot";
            else if (dynamic_cast<SemiAutoScoutBot *>(robot))
                typeName = "SemiAutoScoutBot";
            else if (dynamic_cast<SemiAutoTrackBot *>(robot))
                typeName = "SemiAutoTrackBot";
            else if (dynamic_cast<ThirtyShotScoutBot *>(robot))
                typeName = "ThirtyShotScoutBot";
            else if (dynamic_cast<ThirtyShotTrackBot *>(robot))
                typeName = "ThirtyShotTrackBot";
            else if (dynamic_cast<QueenScoutBot *>(robot))
                typeName = "QueenScoutBot";
            else if (dynamic_cast<QueenTrackBot *>(robot))
                typeName = "QueenTrackBot";
            else if (dynamic_cast<VampireScoutBot *>(robot))
                typeName = "VampireScoutBot";
            else if (dynamic_cast<VampireTrackBot *>(robot))
                typeName = "VampireTrackBot";
            else if (dynamic_cast<KnightScoutBot *>(robot))
                typeName = "KnightScoutBot";
            else if (dynamic_cast<KnightTrackBot *>(robot))
                typeName = "KnightTrackBot";
            else if (dynamic_cast<HideBot *>(robot))
                typeName = "HideBot";
            else if (dynamic_cast<JumpBot *>(robot))
                typeName = "JumpBot";
            else if (dynamic_cast<LongShotBot *>(robot))
                typeName = "LongShotBot";
            else if (dynamic_cast<KnightBot *>(robot))
                typeName = "KnightBot";
            else if (dynamic_cast<SemiAutoBot *>(robot))
                typeName = "SemiAutoBot";
            else if (dynamic_cast<ThirtyShotBot *>(robot))
                typeName = "ThirtyShotBot";
            else if (dynamic_cast<ScoutBot *>(robot))
                typeName = "ScoutBot";
            else if (dynamic_cast<TrackBot *>(robot))
                typeName = "TrackBot";
            else if (dynamic_cast<QueenBot *>(robot))
                typeName = "QueenBot";
            else if (dynamic_cast<VampireBot *>(robot))
                typeName = "VampireBot";
            else if (dynamic_cast<GenericRobot *>(robot))
                typeName = "GenericRobot";
            else
                typeName = "UnknownType"; // Fallback for any unrecognized types

            // Ammo check: Prioritize ThirtyShotBot's shell count, then general ShootingRobot ammo
            ThirtyShotBot *TSB = dynamic_cast<ThirtyShotBot *>(robot);
            ShootingRobot *shooter = dynamic_cast<ShootingRobot *>(robot);

            if (TSB)
            {
                currentAmmo = TSB->getShellCount();
            }
            else if (shooter)
            {
                currentAmmo = shooter->getAmmo();
            }

            logger << "  Type: " << typeName        // Log the type of the robot
                   << ", Name: " << robot->getName()   // Log the name of the robot
                   << ", Coords: (" << robot->getX() << "," << robot->getY() << ")"   // Log the coordinates
                   << ", Life: " << robot->getLives();    // Log the number of lives
            if (shooter)
            { // Only print ammo if it's a shooting robot
                logger << ", Ammo: " << currentAmmo;
            }
            logger << endl;
        }
    }
    vector<Robot *> currentlyAliveRobots;

    for (Robot *robot : listOfRobots)
    {
        if (robot->getLives() > 0 && !robot->getIsDie())  // Check if the robot is alive and not marked for destruction
        {
            currentlyAliveRobots.push_back(robot);   // Add to the list of currently alive robots
        }
    }

    for (Robot *robot : currentlyAliveRobots)
    {
        // Set battlefield context if robot is a GenericRobot
        if (auto gen = dynamic_cast<GenericRobot *>(robot))
        {
            gen->setBattlefield(this);  // Ensure battlefield context is set
            gen->resetActionFlags();    // Reset action flags for the new robot
        }
        // Skip robot's turn if it is hurt (was hit this turn)
        if (robot->getIsHurt())
        {
            logger << "-----------------------------------" << endl;
            logger << robot->getName() << " was hit and skips this turn." << endl;
            continue;
        }
        logger << "----------------------------------------" << endl;
        logger << robot->getName() << "'s turn: " << endl;   // Log the robot's turn
        robot->act();                                        // Call the act method(think, look, move, fire) of the robot
        logger << robot->getName() << " is done." << endl;   // Log that the robot's turn is done
    }

    cleanupDestroyedRobots();      // Cleanup robots that are marked for destruction
    logger << "----------------------------------------" << endl;
    respawnRobots();     // Handle respawning robots that were queued for reentry
    for (Robot *robot : listOfRobots)
    {
        robot->setIsDie(false); // Reset isDie flag for next turn
    }
}

// To get the number of alive robots
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

// To add new robot to battlefield during initialization
void Battlefield::addNewRobot(Robot *robot)
{
    listOfRobots.push_back(robot); // Add the robot to the list of robots
    robot->setBattlefield(this); // Set battlefield pointer
    if (respawnCounts.find(robot->getName()) == respawnCounts.end())  // Check if the robot is already in respawnCounts
    {
        respawnCounts[robot->getName()] = 3; // limit respawn count to 3
    }
}

// To check if the given coordinates are within the grid
bool Battlefield::isPositionWithinGrid(int x, int y) const
{
    return (x >= 0 && x < width) && (y >= 0 && y < height);
}

// To get the Robot at given coordinates, if not found then return nullptr
Robot *Battlefield::getRobotAt(int x, int y) const
{
    if (!isPositionWithinGrid(x, y))
    {
        return nullptr;
    }
    return battlefieldGrid[y][x];
}

// To check if the specified coordinates is available (not occupied by another robot)
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

// To put robot at specified coordinates
void Battlefield::placeRobot(Robot *robot, int x, int y)
{
    if (isPositionAvailable(x, y) && isPositionWithinGrid(x, y))
    {
        battlefieldGrid[y][x] = robot;
        robot->setPosition(x, y);
    }
}

// To remove robot from grid position
void Battlefield::removeRobotFromGrid(Robot *robot)
{
    int x = robot->getX();
    int y = robot->getY();

    if (isPositionWithinGrid(x, y) && battlefieldGrid[y][x] == robot)
    {
        battlefieldGrid[y][x] = nullptr;
    }
}

// To queue robot for reentry
void Battlefield::queueForReentry(Robot *robot)
{
    
    if (queuedThisRound.find(robot) != queuedThisRound.end()) return;  // Prevent re-queuing the same robot in the same round
    queuedThisRound.insert(robot);   // Add robot to the set of queued robots for this round
    int currentAmmo;
    // Try to get ammo, prioritizing ThirtyShotBot's shellCount
    ThirtyShotBot *TSB = dynamic_cast<ThirtyShotBot *>(robot);
    if (TSB)
    {
        currentAmmo = TSB->getShellCount(); // Gets specific shell count
    }
    else
    {
        ShootingRobot *shooter = dynamic_cast<ShootingRobot *>(robot);
        if (shooter)
        {
            currentAmmo = shooter->getAmmo(); // Gets standard ammo
        }
    }
    reentryQueue.push({robot->getName(), robot->getLives(), currentAmmo});  // Push the robot's name, lives, and ammo into the reentry queue
    logger << robot->getName() << " queued for reentry with " << robot->getLives()
           << " lives and " << currentAmmo << " ammo." << endl;
}

// To handle robot respawns
void Battlefield::respawnRobots()
{
    if (!reentryQueue.empty())
    {
        auto respawnInfo = reentryQueue.front();   // Get the front of the queue which contains the robot's respawn info
        string nameOfRobotToRespawn = respawnInfo.name;  // Get the name of the robot to respawn
        int livesLeft = respawnInfo.lives;   // Get the lives left for the robot to respawn
        int ammoLeft = respawnInfo.ammo;     // Get the ammo left for the robot to respawn
        reentryQueue.pop();                  // Remove the front of the queue after processing
        logger << "Reentering " << nameOfRobotToRespawn << endl;
        int randomX;
        int randomY;
        int attempts = 50;  //Set a limit of 50 attempts to find a valid and unoccupied grid position
        bool spotFound = false;
        // Try to find a random position within the grid that is available
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
            newRobot->setLives(livesLeft);  // Restore the lives before being hit
            newRobot->setBattlefield(this); // Set battlefield pointer
            ShootingRobot *newShooter = dynamic_cast<ShootingRobot *>(newRobot);
            if (newShooter)
            {
                newShooter->setAmmo(respawnInfo.ammo); // Set the preserved ammo
            }
            placeRobot(newRobot, randomX, randomY);  // Place the new robot at the random position
            listOfRobots.push_back(newRobot);  // Add the new robot to the list of robots
            logger << nameOfRobotToRespawn << " reentered as GenericRobot at (" << randomX << ", " << randomY << ") with ";
            logger << livesLeft << " lives and " << ammoLeft << " ammo next turn." << endl;
        }
        else
        {
            logger << "No available spot for reentry for " << nameOfRobotToRespawn << ". Will try again next turn." << endl;
            // If no spot, requeue for next turn
            reentryQueue.push({nameOfRobotToRespawn, livesLeft, ammoLeft});
        }
    }
}

//COMPLETED: cleanupDestroyedRobots member function
void Battlefield::cleanupDestroyedRobots()
{
    logger << "----------------------------------------" << endl;
    auto iterator = listOfRobots.begin();
    while (iterator != listOfRobots.end())
    {
        Robot *robot = *iterator;
        // If robot is hurt but not dead, queue for reentry and remove from grid, then delete and remove from list
        if (robot->getIsHurt() && robot->getLives() > 0 && !robot->getIsDie()) { // Check if the robot is hurt but not dead
            queueForReentry(robot);  // Queue the robot for reentry
            removeRobotFromGrid(robot);  // Remove the robot from the grid
            logger << robot->getName() << " has been removed from the battlefield." << endl;
            robot->setIsHurt(false);  // Reset the hurt status
            delete robot;  // Delete the robot to free memory
            iterator = listOfRobots.erase(iterator); // Remove the robot from the list
            continue;
        }
        if (robot->getLives() <= 0 || robot->getIsDie())
        {
            removeRobotFromGrid(robot);  // Remove the robot from the grid
            logger << robot->getName() << " has been destroyed and removed from the battlefield." << endl;
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
    logger << "  H - Hidden Robot" << endl;
    logger << "  [Letter] - First letter of robot's name" << endl
           << endl;
}

//******************************************
// Battlefield Destructor
//******************************************

Battlefield::~Battlefield()
{
    // clear robot pointers
    for (Robot *robot : listOfRobots)
    {
        delete robot;
    }
    listOfRobots.clear();   // Clear the list of robots
}

//******************************************
// Function implementations
//******************************************

void parseInputFile(const string &line, Battlefield &battlefield)
{
    vector<string> tokens;  // Vector to hold tokens from the line
    istringstream iss(line);  // Use istringstream to split the line into tokens
    string token;    // Temporary string to hold each token

    while (iss >> token)   // Extract tokens from the line
    {
        tokens.push_back(token);
    }

    if (tokens.empty())  // Check if the line is empty
    {
        return; // Skip empty lines
    }

    if (tokens[0] == "M" && tokens.size() >= 6)  //Check for battlefield dimensions
    {
        int height = stoi(tokens[4]);
        int width = stoi(tokens[5]);
        battlefield.setDimensions(height, width);
    }
    else if (tokens[0] == "steps:" && tokens.size() >= 2)   //Check for steps
    {
        int steps = stoi(tokens[1]);
        battlefield.setSteps(steps);
    }
    else if (tokens[0] == "robots:" && tokens.size() >= 2)   //Check for number of robots
    {
        int numRobots = stoi(tokens[1]);
        battlefield.setNumberOfRobots(numRobots);
    }
    else if (tokens[0] == "GenericRobot" && tokens.size() >= 4) // Check for GenericRobot creation
    {

        string robotName = tokens[1];  // Get the robot name
        int robotXCoordinates;         // Get the robot's X coordinates
        int robotYCoordinates;         // Get the robot's Y coordinates

        if (tokens[2] == "random" && tokens[3] == "random")   // Check if coordinates are random
        {
            robotXCoordinates = rand() % battlefield.getWidth();   // Generate random X coordinate
            robotYCoordinates = rand() % battlefield.getHeight();  // Generate random Y coordinate
        }
        else
        {
            robotXCoordinates = stoi(tokens[2]);    // Convert string to integer for X coordinate
            robotYCoordinates = stoi(tokens[3]);    // Convert string to integer for Y coordinate
        }

        Robot *newRobot = new GenericRobot(robotName, robotXCoordinates, robotYCoordinates);  // Create a new GenericRobot object
        battlefield.addNewRobot(newRobot);   // Add the new robot to the battlefield
        battlefield.placeRobot(newRobot, robotXCoordinates, robotYCoordinates);  // Place the robot at the specified coordinates
    }
    else if (tokens[0] == "HideBot" && tokens.size() >= 4)  // Check for HideBot creation
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
    else if (tokens[0] == "HideLongShotBot" && tokens.size() >= 4)
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

        Robot *newRobot = new HideLongShotBot(robotName, robotXCoordinates, robotYCoordinates);
        battlefield.addNewRobot(newRobot);
        battlefield.placeRobot(newRobot, robotXCoordinates, robotYCoordinates);
    }

    else if (tokens[0] == "HideSemiAutoBot" && tokens.size() >= 4)
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

        Robot *newRobot = new HideSemiAutoBot(robotName, robotXCoordinates, robotYCoordinates);
        battlefield.addNewRobot(newRobot);
        battlefield.placeRobot(newRobot, robotXCoordinates, robotYCoordinates);
    }

    else if (tokens[0] == "HideThirtyShotBot" && tokens.size() >= 4)
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

        Robot *newRobot = new HideThirtyShotBot(robotName, robotXCoordinates, robotYCoordinates);
        battlefield.addNewRobot(newRobot);
        battlefield.placeRobot(newRobot, robotXCoordinates, robotYCoordinates);
    }

    else if (tokens[0] == "HideKnightBot" && tokens.size() >= 4)
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

        Robot *newRobot = new HideKnightBot(robotName, robotXCoordinates, robotYCoordinates);
        battlefield.addNewRobot(newRobot);
        battlefield.placeRobot(newRobot, robotXCoordinates, robotYCoordinates);
    }

    else if (tokens[0] == "HideQueenBot" && tokens.size() >= 4)
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

        Robot *newRobot = new HideQueenBot(robotName, robotXCoordinates, robotYCoordinates);
        battlefield.addNewRobot(newRobot);
        battlefield.placeRobot(newRobot, robotXCoordinates, robotYCoordinates);
    }

    else if (tokens[0] == "HideVampireBot" && tokens.size() >= 4)
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

        Robot *newRobot = new HideVampireBot(robotName, robotXCoordinates, robotYCoordinates);
        battlefield.addNewRobot(newRobot);
        battlefield.placeRobot(newRobot, robotXCoordinates, robotYCoordinates);
    }

    else if (tokens[0] == "JumpLongShotBot" && tokens.size() >= 4)
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

        Robot *newRobot = new JumpLongShotBot(robotName, robotXCoordinates, robotYCoordinates);
        battlefield.addNewRobot(newRobot);
        battlefield.placeRobot(newRobot, robotXCoordinates, robotYCoordinates);
    }

    else if (tokens[0] == "JumpSemiAutoBot" && tokens.size() >= 4)
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

        Robot *newRobot = new JumpSemiAutoBot(robotName, robotXCoordinates, robotYCoordinates);
        battlefield.addNewRobot(newRobot);
        battlefield.placeRobot(newRobot, robotXCoordinates, robotYCoordinates);
    }

    else if (tokens[0] == "JumpThirtyShotBot" && tokens.size() >= 4)
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

        Robot *newRobot = new JumpThirtyShotBot(robotName, robotXCoordinates, robotYCoordinates);
        battlefield.addNewRobot(newRobot);
        battlefield.placeRobot(newRobot, robotXCoordinates, robotYCoordinates);
    }

    else if (tokens[0] == "JumpKnightBot" && tokens.size() >= 4)
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

        Robot *newRobot = new JumpKnightBot(robotName, robotXCoordinates, robotYCoordinates);
        battlefield.addNewRobot(newRobot);
        battlefield.placeRobot(newRobot, robotXCoordinates, robotYCoordinates);
    }

    else if (tokens[0] == "JumpQueenBot" && tokens.size() >= 4)
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

        Robot *newRobot = new JumpQueenBot(robotName, robotXCoordinates, robotYCoordinates);
        battlefield.addNewRobot(newRobot);
        battlefield.placeRobot(newRobot, robotXCoordinates, robotYCoordinates);
    }

    else if (tokens[0] == "JumpVampireBot" && tokens.size() >= 4)
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

        Robot *newRobot = new JumpVampireBot(robotName, robotXCoordinates, robotYCoordinates);
        battlefield.addNewRobot(newRobot);
        battlefield.placeRobot(newRobot, robotXCoordinates, robotYCoordinates);
    }

    else if (tokens[0] == "HideScoutBot" && tokens.size() >= 4)
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

        Robot *newRobot = new HideScoutBot(robotName, robotXCoordinates, robotYCoordinates);
        battlefield.addNewRobot(newRobot);
        battlefield.placeRobot(newRobot, robotXCoordinates, robotYCoordinates);
    }

    else if (tokens[0] == "HideTrackBot" && tokens.size() >= 4)
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

        Robot *newRobot = new HideTrackBot(robotName, robotXCoordinates, robotYCoordinates);
        battlefield.addNewRobot(newRobot);
        battlefield.placeRobot(newRobot, robotXCoordinates, robotYCoordinates);
    }

    else if (tokens[0] == "JumpScoutBot" && tokens.size() >= 4)
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

        Robot *newRobot = new JumpScoutBot(robotName, robotXCoordinates, robotYCoordinates);
        battlefield.addNewRobot(newRobot);
        battlefield.placeRobot(newRobot, robotXCoordinates, robotYCoordinates);
    }

    else if (tokens[0] == "JumpTrackBot" && tokens.size() >= 4)
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

        Robot *newRobot = new JumpTrackBot(robotName, robotXCoordinates, robotYCoordinates);
        battlefield.addNewRobot(newRobot);
        battlefield.placeRobot(newRobot, robotXCoordinates, robotYCoordinates);
    }

    else if (tokens[0] == "LongShotScoutBot" && tokens.size() >= 4)
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

        Robot *newRobot = new LongShotScoutBot(robotName, robotXCoordinates, robotYCoordinates);
        battlefield.addNewRobot(newRobot);
        battlefield.placeRobot(newRobot, robotXCoordinates, robotYCoordinates);
    }

    else if (tokens[0] == "LongShotTrackBot" && tokens.size() >= 4)
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

        Robot *newRobot = new LongShotTrackBot(robotName, robotXCoordinates, robotYCoordinates);
        battlefield.addNewRobot(newRobot);
        battlefield.placeRobot(newRobot, robotXCoordinates, robotYCoordinates);
    }

    else if (tokens[0] == "SemiAutoScoutBot" && tokens.size() >= 4)
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

        Robot *newRobot = new SemiAutoScoutBot(robotName, robotXCoordinates, robotYCoordinates);
        battlefield.addNewRobot(newRobot);
        battlefield.placeRobot(newRobot, robotXCoordinates, robotYCoordinates);
    }

    else if (tokens[0] == "SemiAutoTrackBot" && tokens.size() >= 4)
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

        Robot *newRobot = new SemiAutoTrackBot(robotName, robotXCoordinates, robotYCoordinates);
        battlefield.addNewRobot(newRobot);
        battlefield.placeRobot(newRobot, robotXCoordinates, robotYCoordinates);
    }

    else if (tokens[0] == "ThirtyShotScoutBot" && tokens.size() >= 4)
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

        Robot *newRobot = new ThirtyShotScoutBot(robotName, robotXCoordinates, robotYCoordinates);
        battlefield.addNewRobot(newRobot);
        battlefield.placeRobot(newRobot, robotXCoordinates, robotYCoordinates);
    }

    else if (tokens[0] == "ThirtyShotTrackBot" && tokens.size() >= 4)
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

        Robot *newRobot = new ThirtyShotTrackBot(robotName, robotXCoordinates, robotYCoordinates);
        battlefield.addNewRobot(newRobot);
        battlefield.placeRobot(newRobot, robotXCoordinates, robotYCoordinates);
    }

    else if (tokens[0] == "KnightScoutBot" && tokens.size() >= 4)
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

        Robot *newRobot = new KnightScoutBot(robotName, robotXCoordinates, robotYCoordinates);
        battlefield.addNewRobot(newRobot);
        battlefield.placeRobot(newRobot, robotXCoordinates, robotYCoordinates);
    }

    else if (tokens[0] == "QueenScoutBot" && tokens.size() >= 4)
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

        Robot *newRobot = new QueenScoutBot(robotName, robotXCoordinates, robotYCoordinates);
        battlefield.addNewRobot(newRobot);
        battlefield.placeRobot(newRobot, robotXCoordinates, robotYCoordinates);
    }

    else if (tokens[0] == "VampireScoutBot" && tokens.size() >= 4)
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

        Robot *newRobot = new VampireScoutBot(robotName, robotXCoordinates, robotYCoordinates);
        battlefield.addNewRobot(newRobot);
        battlefield.placeRobot(newRobot, robotXCoordinates, robotYCoordinates);
    }

    else if (tokens[0] == "HideLongShotScoutBot" && tokens.size() >= 4)
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

        Robot *newRobot = new HideLongShotScoutBot(robotName, robotXCoordinates, robotYCoordinates);
        battlefield.addNewRobot(newRobot);
        battlefield.placeRobot(newRobot, robotXCoordinates, robotYCoordinates);
    }

    else if (tokens[0] == "HideLongShotTrackBot" && tokens.size() >= 4)
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

        Robot *newRobot = new HideLongShotTrackBot(robotName, robotXCoordinates, robotYCoordinates);
        battlefield.addNewRobot(newRobot);
        battlefield.placeRobot(newRobot, robotXCoordinates, robotYCoordinates);
    }

    else if (tokens[0] == "HideSemiAutoScoutBot" && tokens.size() >= 4)
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

        Robot *newRobot = new HideSemiAutoScoutBot(robotName, robotXCoordinates, robotYCoordinates);
        battlefield.addNewRobot(newRobot);
        battlefield.placeRobot(newRobot, robotXCoordinates, robotYCoordinates);
    }

    else if (tokens[0] == "HideSemiAutoTrackBot" && tokens.size() >= 4)
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

        Robot *newRobot = new HideSemiAutoTrackBot(robotName, robotXCoordinates, robotYCoordinates);
        battlefield.addNewRobot(newRobot);
        battlefield.placeRobot(newRobot, robotXCoordinates, robotYCoordinates);
    }

    else if (tokens[0] == "HideThirtySHotScoutBot" && tokens.size() >= 4)
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

        Robot *newRobot = new HideThirtyShotScoutBot(robotName, robotXCoordinates, robotYCoordinates);
        battlefield.addNewRobot(newRobot);
        battlefield.placeRobot(newRobot, robotXCoordinates, robotYCoordinates);
    }

    else if (tokens[0] == "HideThirtyShotTrackBot" && tokens.size() >= 4)
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

        Robot *newRobot = new HideThirtyShotTrackBot(robotName, robotXCoordinates, robotYCoordinates);
        battlefield.addNewRobot(newRobot);
        battlefield.placeRobot(newRobot, robotXCoordinates, robotYCoordinates);
    }

    else if (tokens[0] == "HideKnightScoutBot" && tokens.size() >= 4)
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

        Robot *newRobot = new HideKnightScoutBot(robotName, robotXCoordinates, robotYCoordinates);
        battlefield.addNewRobot(newRobot);
        battlefield.placeRobot(newRobot, robotXCoordinates, robotYCoordinates);
    }

    else if (tokens[0] == "HideKnightTrackBot" && tokens.size() >= 4)
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

        Robot *newRobot = new HideKnightTrackBot(robotName, robotXCoordinates, robotYCoordinates);
        battlefield.addNewRobot(newRobot);
        battlefield.placeRobot(newRobot, robotXCoordinates, robotYCoordinates);
    }

    else if (tokens[0] == "HideQueenScoutBot" && tokens.size() >= 4)
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

        Robot *newRobot = new HideQueenScoutBot(robotName, robotXCoordinates, robotYCoordinates);
        battlefield.addNewRobot(newRobot);
        battlefield.placeRobot(newRobot, robotXCoordinates, robotYCoordinates);
    }

    else if (tokens[0] == "HideQueenTrackBot" && tokens.size() >= 4)
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

        Robot *newRobot = new HideQueenTrackBot(robotName, robotXCoordinates, robotYCoordinates);
        battlefield.addNewRobot(newRobot);
        battlefield.placeRobot(newRobot, robotXCoordinates, robotYCoordinates);
    }

    else if (tokens[0] == "HideVampireScoutBot" && tokens.size() >= 4)
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

        Robot *newRobot = new HideVampireScoutBot(robotName, robotXCoordinates, robotYCoordinates);
        battlefield.addNewRobot(newRobot);
        battlefield.placeRobot(newRobot, robotXCoordinates, robotYCoordinates);
    }

    else if (tokens[0] == "HideVampireTrackBot" && tokens.size() >= 4)
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

        Robot *newRobot = new HideVampireTrackBot(robotName, robotXCoordinates, robotYCoordinates);
        battlefield.addNewRobot(newRobot);
        battlefield.placeRobot(newRobot, robotXCoordinates, robotYCoordinates);
    }
}

//*****************************************************************************************
// Modified to take Battlefield by reference to avoid copying/double-free memory error
//*****************************************************************************************

void readInputFile(Battlefield &battlefield, const string &filename = "inputFile.txt") // Default filename
{
    ifstream inputFile(filename);  // Open the input file

    if (!inputFile.is_open())  // Check if the file opened successfully
    {
        cerr << "Error opening input file." << endl; // Log an error message
        return;
    }

    string line;  // Variable to hold each line read from the file
    while (getline(inputFile, line))   //Read each line from the file
    {
        if (!line.empty())    //Check if the line is not empty
        {
            parseInputFile(line, battlefield);  //Call the function to parse the line and update the battlefield
        }
    }

    inputFile.close();   // Close the input file after reading all lines
}

int main()
{
    // Seed the random number generator once
    srand(static_cast<unsigned>(time(0)));

    Battlefield battlefield;     // Create a Battlefield object
    readInputFile(battlefield); // This sets dimensions and initializes grid

    logger << "Battlefield Dimensions: " << endl;
    logger << "Width: " << battlefield.getWidth() << endl;  // Log the width of the battlefield
    logger << "Height: " << battlefield.getHeight() << endl; // Log the height of the battlefield

    logger << "Battlefield steps: " << battlefield.getSteps() << endl;  // Log the number of steps for the simulation

    logger << "Battlefield number of robots: " << battlefield.getNumberOfRobots() << endl; // Log the number of robots on the battlefield

    logger << "Initial Robots on battlefield: " << endl; // Log the initial state of robots on the battlefield

    for (Robot *robot : battlefield.getListOfRobots())
    {
        string name = robot->getName();
        int x = robot->getX();
        int y = robot->getY();
        int lives = robot->getLives();
        logger << "Robot Name: " << name << ", Coords: (" << x << "," << y << "), Lives: " << lives <<  endl;
    }
    logger << endl;

    logger << "Initial Battlefield State:" << endl;
    battlefield.displayBattlefield(); // Display the initial state of the battlefield
    logger << endl;

    // --- Simulation Loop ---
    int currentStep = 0; // Initialize the current step counter
    int maxSteps = battlefield.getSteps(); // Get the maximum number of steps from the battlefield 

    logger << "--- Starting Simulation ---" << endl;

    // Loop while max steps not reached AND there's more than one robot alive
    while (currentStep < maxSteps && battlefield.getNumberOfAliveRobots() > 1)
    {
        // Increment step for the current iteration
        currentStep++;

        logger << "\n--- Simulation Step " << currentStep << " ---" << endl;

        // Call simulationStep, passing the current step number
        // The robot status logging will happen inside simulationStep, after upgrades.
        battlefield.simulationStep(currentStep); // Pass the current step number

        logger << "\nBattlefield State after Step " << currentStep << ":" << endl;
        battlefield.displayBattlefield(); // Display the updated state of the battlefield
    }

    // --- End of Simulation ---
    logger << "\n--- Simulation Ended ---" << endl;

    size_t remainingRobots = battlefield.getNumberOfAliveRobots();  // Get the number of remaining robots after the simulation ends

    if (remainingRobots <= 1) // If one or zero robots are left after the simulation
    {
        if (remainingRobots == 0) // If no robots are left
        {
            logger << "Result: All robots destroyed!" << endl;
        }
        else // If exactly one robot is left
        {
            Robot *lastRobot = nullptr;  // Pointer to hold the last remaining robot
            const vector<Robot *> &finalRobotList = battlefield.getListOfRobots(); // Get the final list of robots
            // Iterate through the final list of robots to find the last one standing
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
    else // If more than one robot is still alive after the simulation ends
    {
        logger << "Result: Maximum steps reached (" << maxSteps << " steps)." << endl;
        logger << "Remaining robots: " << remainingRobots << endl;
        const vector<Robot *> &finalRobotList = battlefield.getListOfRobots();
        for (const Robot *remainingRobot : finalRobotList) {
            logger << remainingRobot->getName() << endl;
        }
    }

    return 0;
}