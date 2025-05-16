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
#include <cstdlib>
#include <map>
#include <algorithm>

using namespace std;

// Forward declarations
class Robot;
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

    // TODO for Battlefield:
    // - fix bugs if any

    // TODO others:
    // - write output to .txt file
    // - log more details to text file and output
    // - Detailed comments
    // - More error handling

public:
    // bool checkOccupied(int a, int y);
    // void placeRobot(Robot *r);

    // Constructor
    Battlefield() : height(0), width(0), steps(0), numberOfRobots(0) {};

    ~Battlefield();

    // Member functions for Battlefield class
    void setDimensions(int h, int w)
    {
        height = h;
        width = w;
        battlefieldGrid.assign(height, vector<Robot *>(width, nullptr));
    }

    void printDimensions() const
    {
        cout << "Height: " << height << endl;
        cout << "Width: " << width << endl;
    }

    void setSteps(int s)
    {
        steps = s;
    }

    void printSteps() const
    {
        cout << steps;
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
        cout << numberOfRobots;
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
    void simulationTurn();
    void addNewRobot(Robot *robot);
    int getNumberOfAliveRobots();
    void cleanupDestroyedRobots();
    void respawnRobots();

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
protected:
    string name;
    int positionX;
    int positionY;
    int lives;
    bool hidden;
    virtual bool isHit() = 0;

public:
    // bool isReentry();
    // void onReenter();

    Robot(string name, int x, int y)
        : name(name), positionX(x), positionY(y), lives(3), hidden(false) {}

    virtual ~Robot() = default;

    // NOTE: think() and act() were void. Kept as is per instruction.
    // This means they cannot directly interact with the Battlefield passed to derived methods.
    virtual void think() = 0;
    virtual void act() = 0;

    // Common robot functions
    string getName() const { return name; }
    int getX() const { return positionX; }
    int getY() const { return positionY; }
    int getLives() const { return lives; }
    void setLives(int numOfLives) { lives = numOfLives; }
    bool isHidden() const { return hidden; }

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
                cout << name << " has been destroyed!\n";
            }
        }
    }

    void setHidden(bool state) { hidden = state; }
};

//******************************************
// Moving Robot class
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
        int dx = abs(newX - positionX);
        int dy = abs(newY - positionY);
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

    bool hitProbability() const
    {
        static random_device rd;
        static mt19937 gen(rd());
        uniform_real_distribution<> dis(0.0, 1.0);
        return dis(gen) <= 0.7; // 70% hit chance
    }
};

//******************************************
// Seeing Robot class
//******************************************

class SeeingRobot : public virtual Robot
{
protected:
    int visionRange;

public:
    SeeingRobot(const string &name, int x, int y, int range)
        : Robot(name, x, y), visionRange(range) {}

    virtual ~SeeingRobot() = default;
    virtual void look(Battlefield &battlefield) = 0;

    int getVisionRange() const { return visionRange; }

    bool canSee(int targetX, int targetY) const
    {
        int dx = targetX - positionX;
        int dy = targetY - positionY;
        // Fixed potential integer overflow by casting to long long before multiplication
        return (long long)dx * dx + (long long)dy * dy <= (long long)visionRange * visionRange;
    }
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
    void think() override {
        cout << name << " is thinking..." << endl;
    }

    int getStrategyLevel() const { return strategyLevel; }

    virtual string decideAction() const = 0;
};

//******************************************
// GenericRobot class
//******************************************

class GenericRobot : public MovingRobot, public ShootingRobot, public SeeingRobot, public ThinkingRobot
{
private:
    bool hasUpgraded[3] = {false, false, false}; // Track upgrades for moving, shooting, seeing
    Battlefield* battlefield = nullptr; // Pointer to current battlefield

public:
    GenericRobot(const string &name, int x, int y)
        : Robot(name, x, y),
          MovingRobot(name, x, y),
          ShootingRobot(name, x, y, 10), // Start with 10 ammo
          SeeingRobot(name, x, y, 1),    // Basic vision range of 1
          ThinkingRobot(name, x, y, 1)
    {
    }

    ~GenericRobot() override = default;

    void setBattlefield(Battlefield* bf) { battlefield = bf; }

    void think() override
    {
        cout << name << " is thinking...\n";
    }

    void act() override
    {
        if (!battlefield) {
            cout << name << " has no battlefield context!" << endl;
            return;
        }
        think();
        look(*battlefield);
        fire(*battlefield);
        move(*battlefield);
    }

    void move(Battlefield &battlefield) override
    {
        cout << name << " is moving...\n";
        int dx[] = {0, 1, 0, -1};
        int dy[] = {1, 0, -1, 0};
        vector<int> directions = {0, 1, 2, 3};
        static std::random_device rd;
        static std::mt19937 g(rd());
        std::shuffle(directions.begin(), directions.end(), g);
        for (int dir : directions) {
            int newX = positionX + dx[dir];
            int newY = positionY + dy[dir];
            // Ensure only one robot at (newX, newY)
            if (isValidMove(newX, newY, battlefield) && battlefield.isPositionAvailable(newX, newY)) {
                if (battlefield.getRobotAt(newX, newY) == nullptr) {
                    battlefield.removeRobotFromGrid(this);
                    battlefield.placeRobot(this, newX, newY);
                    incrementMoveCount();
                    cout << name << " moved to (" << newX << ", " << newY << ")\n";
                    return;
                }
            }
        }
        cout << name << " could not move (no available adjacent cell).\n";
    }

    void fire(Battlefield &battlefield) override
    {
        if (hasAmmo()) {
            // Collect all possible targets (other robots, not self, and alive)
            vector<Robot*> possibleTargets;
            for (Robot* r : battlefield.getListOfRobots()) {
                if (r != nullptr && r != this && r->getLives() > 0) {
                    possibleTargets.push_back(r);
                }
            }
            if (!possibleTargets.empty()) {
                // Pick a random target from the list
                int idx = rand() % possibleTargets.size();
                Robot* target = possibleTargets[idx];
                int targetX = target->getX();
                int targetY = target->getY();
                cout << name << " fires at (" << targetX << ", " << targetY << ")" << endl;
                useAmmo();
                if (hitProbability()) {
                    cout << "Hit! (" << target->getName() << ") be killed" << endl;
                    target->takeDamage();
                } else {
                    cout << "Missed!" << endl;
                }
            } else {
                cout << name << " has no valid targets to fire at!" << endl;
            }
        } else {
            cout << name << " has no ammo left!" << endl;
        }
    }

    void look(Battlefield &battlefield) override {
        cout << name << " is scanning surroundings...." << endl;
        int cx = positionX;
        int cy = positionY;
        for (int dx = -1; dx <= 1; ++dx) {
            for (int dy = -1; dy <= 1; ++dy) {
                int nx = cx + dx;
                int ny = cy + dy;
                cout << "Checking (" << nx << ", " << ny << "): ";
                if (!battlefield.isPositionWithinGrid(nx, ny)) {
                    cout << "Exceed Boundary" << endl;
                } else if (nx == cx && ny == cy) {
                    cout << "Current Position" << endl;
                } else {
                    Robot* r = battlefield.getRobotAt(nx, ny);
                    if (r == nullptr) {
                        cout << "In Boundary but Empty" << endl;
                    } else {
                        cout << "Enemy detected: " << r->getName() << " at (" << nx << "," << ny << ")" << endl;
                    }
                }
            }
        }
    }

    string decideAction() const override
    {
        if (hasAmmo())
            return "fire";
        return "move";
    }

    bool canUpgrade(int area) const
    {
        if (area < 0 || area > 2)
            return false;
        return !hasUpgraded[area];
    }

    void setUpgraded(int area)
    {
        if (area >= 0 && area < 3)
        {
            hasUpgraded[area] = true;
            // Upgrade effects would go here
        }
    }

    bool isHit() override {
        return true;
    }
};

//******************************************
//HideBot
//******************************************

class HideBot : public GenericRobot{

    private:
    int hide_count = 0;
    bool isHidden=false;

    public:
    HideBot(const string &name, int x, int y) 
        : Robot(name,x,y),
          GenericRobot(name,x,y){}

    void move(Battlefield &battlefield)override{
        if (hide_count <  3){
            hide_count++;
            isHidden = true;
            cout << getName() << "hide,("<< hide_count<<"/3)"<< endl;            
        }
        else{
            isHidden = false;
            cout << getName() << "finish use hide,keep moving"<< endl;
        }
    }

    bool getHiddenStatus() const{
        return isHidden;
    }

    void appear(){
        isHidden = false;
    }

    bool isHit() override{
        if (isHidden){
            cout << getName() << "is hiding,cannot attack"<< endl ;
            return false;

        }
        else {
            cout << getName() << "is hit"<< endl;
            return true;
        }

    }
};

//******************************************
//Jumpbot
//******************************************
class JumpBot : public GenericRobot{
    private:
    
    int jump_count = 0;

    public:
    JumpBot(const string &name,int x, int y)
    :Robot(name,x,y),
     GenericRobot(name,x,y){}

    void move(Battlefield &battlefield) override {
        if (jump_count < 3){
            jump_count++;
            int jumpx = rand() % battlefield.getWidth();
            int jumpy = rand() % battlefield.getHeight();

            setPosition(jumpx,jumpy);
            cout << getName() << "jump to ("<< jumpx << ","<<jumpy<< ")\n";

        }
        else{
            cout<< getName() << "cannot jump already\n";
        }
    }

    int getJumpCount() const{
        return jump_count;
    }

};


//******************************************
//Testbot
//******************************************

class TestRobot : public Robot
{
public:
    TestRobot(const string &name, int x, int y) : Robot(name, x, y)
    {
        cout << "TestRobot " << name << " created at (" << x << ", " << y << ")" << endl;
    }

    void think() override
    {
        cout << getName() << "TestRobot is thinking...";
    }

    void act() override
    {
        cout << getName() << "TestRobot is acting. It will take 1 damage here for testing purposes" << endl;
        takeDamage();
    }

    // void requeue(Robot* robot) {
        
    //     reentry_queue.push(robot);
    //     cout << robot->getName() << " add to queue\n";

    // }

    // void reentrying() {
    //     if (reentry_queue.empty()) {
    //         cout << "No robot queue\n";
    //         return;
    //     }

    //     Robot* robot = reentry_queue.front();
    //     if (!robot->isReentry()) {
    //         cout << robot->getName() << " cannot reenter\n";
    //         reentry_queue.pop();
    //         return;
    //     }


    //     //find a empty place put robot
    //     int tries = 50;
    //     while (tries--) {
    //         int x = rand() % battlefield->getWidth();
    //         int y = rand() % battlefield->getHeight();

    //         if (!battlefield->checkOccupied(x, y)) { 
    //             robot->setPosition(x, y);
    //             robot->onReenter(); 
    //             battlefield->placeRobot(robot); 
    //             reentry_queue.pop();
    //             return;
    //         }
    //     }

    //     cout << "No place put " << robot->getName() << ", try it next time.\n";
    // }

    bool isHit() override{
        return true;
    }
};


//******************************************
// simulationTurn member function of Battlefield class (declared later to avoid issues with code not seeing each other when they need to)
//******************************************

void Battlefield::simulationTurn()
{
    vector<Robot *> currentlyAliveRobots;

    for (Robot *robot : listOfRobots)
    {
        if (robot->getLives() > 0)
        {
            currentlyAliveRobots.push_back(robot);
        }
    }

    for (Robot *robot : currentlyAliveRobots)
    {
        // Set battlefield context if robot is a GenericRobot
        if (auto gen = dynamic_cast<GenericRobot*>(robot)) {
            gen->setBattlefield(this);
        }
        cout << robot->getName() << "'s turn: " << endl;
        robot->act();
        cout << robot->getName() << " is done." << endl;
    }

    cleanupDestroyedRobots();
    respawnRobots();
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

// PARTIALLY COMPLETED: respawnRobots member function definition
void Battlefield::respawnRobots()
{
    if (!respawnQueue.empty())
    {
        pair<string, int> respawnInfo = respawnQueue.front();
        string nameOfRobotToRespawn = respawnInfo.first;
        int livesLeft = respawnInfo.second;
        respawnQueue.erase(respawnQueue.begin());

        cout << "Respawning " << nameOfRobotToRespawn << endl;

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
            newRobot->setLives(livesLeft);
            placeRobot(newRobot, randomX, randomY);
            listOfRobots.push_back(newRobot);
            cout << nameOfRobotToRespawn << " respawned successfully at (" << randomX << ", " << randomY << ")" << endl;
        }
    }
}

//  PARTIALLY COMPLETED: cleanupDestroyedRobots member function
void Battlefield::cleanupDestroyedRobots()
{
    cout << "Battlefield::cleanupDestroyedRobots()" << endl;
    auto iterator = listOfRobots.begin();
    while (iterator != listOfRobots.end())
    {
        Robot *robot = *iterator;
        if (robot != nullptr && (*iterator)->getLives() <= 0)
        {
            removeRobotFromGrid(robot);
            cout << (*iterator)->getName() << " has been deleted from the grid." << endl;

            string robotName = robot->getName();

            if (respawnCounts.count(robotName) && respawnCounts.at(robotName) > 0)
            {
                cout << robotName << " added to respawn queue (" << respawnCounts.at(robotName) << " respawns left." << endl;
                respawnQueue.push_back({robotName, robot->getLives()});
                respawnCounts[robotName]--;

                delete robot;

                iterator = listOfRobots.erase(iterator);
            }
            else
            {
                cout << robotName << " does not have any lives left. Removing it permanently from battlefield..." << endl;
                delete robot;

                iterator = listOfRobots.erase(iterator);
            }
        }
        iterator++;
    }
}

//******************************************
// To display the battlefield state
//******************************************
void Battlefield::displayBattlefield()
{
    // Print column numbers header
    cout << "   ";
    for (int x = 0; x < width; x++)
    {
        cout << x % 10 << " "; // Single digit for each column
    }
    cout << endl;

    // Print top border
    cout << "  +";
    for (int x = 0; x < width; x++)
    {
        cout << "--";
    }
    cout << "+" << endl;

    // Print each row
    for (int y = 0; y < height; y++)
    {
        // Print row number
        cout << y % 10 << " |"; // Single digit for each row

        // Print cells
        for (int x = 0; x < width; x++)
        {
            Robot *robot = battlefieldGrid[y][x];
            if (robot != nullptr)
            {
                if (robot->getLives() <= 0)
                {
                    cout << "X "; // Dead robot
                }
                else if (robot->isHidden())
                {
                    cout << "H "; // Hidden robot
                }
                else
                {
                    // Show first letter of robot's name
                    cout << robot->getName()[0] << " ";
                }
            }
            else
            {
                cout << ". "; // Empty space
            }
        }
        cout << "| " << y % 10 << endl; // Row number on right side
    }

    // Print bottom border
    cout << "  +";
    for (int x = 0; x < width; x++)
    {
        cout << "--";
    }
    cout << "+" << endl;

    // Print column numbers footer
    cout << "   ";
    for (int x = 0; x < width; x++)
    {
        cout << x % 10 << " ";
    }
    cout << endl
         << endl;

    // Print legend
    cout << "LEGEND:" << endl;
    cout << "  . - Empty space" << endl;
    cout << "  X - Destroyed robot" << endl;
    cout << "  H - Hidden robot" << endl;
    cout << "  [Letter] - First letter of robot's name" << endl
         << endl;
}
// Reentry Queue class
//******************************************

// class Reentry
// {
// private:
//     queue<Robot *> reentry_queue;
//     Battlefield *battlefield;

// public:
//     Reentry(Battlefield *battlefield_) : battlefield(battlefield_)
//     {
//         srand(time(0));
//     }

//     void requeue(Robot *robot)
//     {
//         if (robot->isReentry())
//         {
//             reentry_queue.push(robot);
//             cout << robot->getName() << " added to reentry queue.\n";
//         }
//         else
//         {
//             cout << robot->getName() << " cannot reenter anymore.\n";
//         }
//     }

//     void reentrying()
//     {
//         if (reentry_queue.empty())
//         {
//             cout << "No robot in reentry queue.\n";
//             return;
//         }

//         Robot *robot = reentry_queue.front();

//         if (!robot->isReentry())
//         {
//             cout << robot->getName() << " cannot reenter.\n";
//             reentry_queue.pop();
//             return;
//         }

//         // find a empty place put robot
//         int tries = 50;
//         while (tries--)
//         {
//             int x = rand() % battlefield->getWidth();
//             int y = rand() % battlefield->getHeight();

//             if (!battlefield->checkOccupied(x, y))
//             {
//                 robot->setPosition(x, y);
//                 robot->onReenter();
//                 battlefield->placeRobot(robot);
//                 reentry_queue.pop();
//                 return;
//             }
//         }

//         cout << "No free space for " << robot->getName() << ", will try again next time.\n";
//     }
// };

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
    cout << "Battlefield: Destructor" << endl;
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
    else if (tokens[0] == "HideBot" && tokens.size() >= 4){
        string robotName = tokens[1];
        int robotXCoordinates;
        int robotYCoordinates;

        if (tokens[2] == "random" && tokens[3] == "random"){
            robotXCoordinates = rand() % battlefield.getWidth();
            robotYCoordinates = rand() % battlefield.getHeight();

        }
        else{
            robotXCoordinates = stoi(tokens[2]);
            robotYCoordinates = stoi(tokens[3]);
        }

        Robot* newRobot = new HideBot(robotName,robotXCoordinates,robotYCoordinates);
        battlefield.addNewRobot(newRobot);
        battlefield.placeRobot(newRobot,robotXCoordinates,robotYCoordinates);
    }
    else if (tokens[0] == "JumpBot" && tokens.size() >= 4){
        string robotName = tokens[1];
        int robotXCoordinates;
        int robotYCoordinates;

        if (tokens[2] == "random" && tokens[3] == "random"){
            robotXCoordinates = rand() % battlefield.getWidth();
            robotYCoordinates = rand() % battlefield.getHeight();

        }
        else{
            robotXCoordinates = stoi(tokens[2]);
            robotYCoordinates = stoi(tokens[3]);
        }

        Robot* newRobot = new JumpBot(robotName,robotXCoordinates,robotYCoordinates);
        battlefield.addNewRobot(newRobot);
        battlefield.placeRobot(newRobot,robotXCoordinates,robotYCoordinates);
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

void writeOutputToFile(const Battlefield &battlefield)
{
    const string fileName = "outputFile.txt";
    ofstream outputFile(fileName);

}

int main()
{
    // Seed the random number generator once
    srand(static_cast<unsigned>(time(0)));

    Battlefield battlefield;
    cout << "READING INPUT FILE" << endl;
    readInputFile(battlefield); // This sets dimensions and initializes grid

    cout << "TESTING BATTLEFIELD CLASS" << endl;
    cout << "Battlefield Dimensions: ";
    battlefield.printDimensions();
    cout << endl;

    cout << "Battlefield steps: ";
    battlefield.printSteps();
    cout << endl;

    cout << "Battlefield number of robots: ";
    battlefield.printNumberOfRobots();
    cout << endl;

    cout << "Initial Robots on battlefield: " << endl;
    vector<Robot *> robotsList = battlefield.getListOfRobots(); // This is the initial list
    for (Robot *robot : robotsList)
    {
        string name = robot->getName();
        int x = robot->getX();
        int y = robot->getY();
        int lives = robot->getLives();
        cout << "Robot Name: " << name << ", Coords: (" << x << "," << y << "), Lives: " << lives << endl;
    }
    cout << endl;

    cout << "Initial Battlefield State:" << endl;
    battlefield.displayBattlefield(); // Display the grid
    cout << endl;

    // --- Simulation Loop ---
    int currentStep = 0;
    int maxSteps = battlefield.getSteps();

    cout << "--- Starting Simulation ---" << endl;

    // Loop while max steps not reached AND there's more than one robot alive
    while (currentStep < maxSteps && battlefield.getNumberOfAliveRobots() > 1)
    {
        cout << "\n--- Simulation Step " << currentStep + 1 << " ---" << endl;

        // Display robot status right after the simulation step title
        cout << "Robot Status before Step " << currentStep + 1 << ":" << endl;
        for (Robot* robot : battlefield.getListOfRobots()) {
            string typeName;
            if (dynamic_cast<HideBot*>(robot)) typeName = "HideBot";
            else if (dynamic_cast<JumpBot*>(robot)) typeName = "JumpBot";
            else if (dynamic_cast<GenericRobot*>(robot)) typeName = "GenericRobot";
            else if (dynamic_cast<TestRobot*>(robot)) typeName = "TestRobot";
            else typeName = "Robot";
            cout << "  Type: " << typeName
                 << ", Name: " << robot->getName()
                 << ", Coords: (" << robot->getX() << "," << robot->getY() << ")"
                 << ", Life: " << robot->getLives() << endl;
        }

        battlefield.simulationTurn(); // Executes turns, cleans up, respawns

        cout << "\nBattlefield State after Step " << currentStep + 1 << ":" << endl;
        battlefield.displayBattlefield(); // Display the updated grid
        writeOutputToFile(battlefield);
        // writeOutputToFile(battlefield);

        currentStep++;
    }

    // --- End of Simulation ---
    cout << "\n--- Simulation Ended ---" << endl;

    size_t remainingRobots = battlefield.getNumberOfAliveRobots();

    if (remainingRobots <= 1)
    {
        if (remainingRobots == 0)
        {
            cout << "Result: All robots destroyed!" << endl;
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
                cout << "Result: " << lastRobot->getName() << " is the last one standing!" << endl;
            }
            else
            {
                cout << "Result: Simulation ended with one robot expected, but couldn't find the last one." << endl;
            }
        }
    }
    else
    {
        cout << "Result: Maximum steps reached (" << maxSteps << " steps)." << endl;
        cout << "Remaining robots: " << remainingRobots << endl;
    }

    return 0; // Battlefield object goes out of scope here, destructor is called.
}