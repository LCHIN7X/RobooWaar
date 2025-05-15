#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <sstream>
#include <random>
#include <stdexcept>
#include <cstdlib>
using namespace std;

// Forward declarations
class Battlefield;
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
    vector<Robot *> listOfRobots; // vector to store all Robots

public:
    // Constructor
    Battlefield() : height(0), width(0), steps(0), numberOfRobots(0) {};

    ~Battlefield();

    // Member functions for Battlefield class
    void setDimensions(int h, int w)
    {
        height = h;
        width = w;
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

    void addNewRobot(Robot *robot)
    {
        listOfRobots.push_back(robot);
    }

    const vector<Robot *> getListOfRobots() const
    {
        return listOfRobots;
    }

    void respawnRobots() {}; // Stub for now

    int getWidth() const { return width; }
    int getHeight() const { return height; }
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

public:
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
        return newX >= 0 && newX < battlefield.getWidth() &&
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
    // NOTE: think() was void and override. Kept as is per instruction.
    virtual void think() override = 0;

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

    // NOTE: Overrides void think() and act(). Kept as is per instruction.
    // These cannot access Battlefield without changing the base class signature.
    void think() override
    {
        cout << name << " is thinking...\n";
    }

    void act() override
    {
        think();
    }

    void move(Battlefield &battlefield) override
    {
        cout << name << " is moving...\n";
        // Basic movement logic would go here, using 'battlefield'
    }

    void fire(Battlefield &battlefield) override
    {
        if (hasAmmo())
        {
            cout << name << " is firing...\n";
            useAmmo();
            // Basic shooting logic would go here, using 'battlefield'
        }
        else
        {
            cout << name << " has no ammo left!\n";
        }
    }

    void look(Battlefield &battlefield) override
    {
        cout << name << " is looking around...\n";
        // Basic vision logic would go here, using 'battlefield'
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
};

//******************************************
//Reentry Queue class
//******************************************
class Reentry{
    private:
    queue<Robot*>reentry_queue;
    Battlefield* battlefield;

    public:
    Reentry(Battlefield* batttlefieeld):battlefield(batttlefieeld){
        srand(time(0));
    }

    void requeue(Robot* robot){
        reentry_queue.push(robot);
        cout << robot->name << "add to reentry quequ\n";
    }

    void reentrying(){
        if (reentry_queue.empty()){
            cout << "No robot queue \n";
            return;
        }

        Robot* robot = reentry_queue.front();
        if (!robot->isReentry()){
            cout << robot->name << "cannot reentry\n";
            reentry_queue.pop();
            return;
        
        }

        // find a empty space put robot
        int tries = 50;
        while (tries--){
            int x = rand() % battlefield->getWidth();
            int y = rand() % battlefield->getHeight();

            if (!battlefield->checkoccupied(x,y)){
                robot->setPosition(x,y);
                robot->onreenter();
                battlefield->place_robot(robot);
                reentry_queue.pop();
                return;
            }
        }

        cout << "No place for "<< robot->name << ",try it next time\n";


    }
};

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
}

int main()
{
    Battlefield battlefield;
    // Pass battlefield by reference to readInputFile to populate the main object directly
    cout << "READING INPUT FILE" << endl;
    readInputFile(battlefield);

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

    // This works
    cout << "Robots on battlefield: " << endl;
    vector<Robot *> robotsList = battlefield.getListOfRobots();
    for (Robot *robot : robotsList)
    {
        string name = robot->getName();
        int x = robot->getX();
        int y = robot->getY();

        cout << "Robot Name: " << name << endl;
        cout << "Robot Coords: (" << x << "," << y << ")" << endl;
    }
    cout << endl;

    return 0; // Battlefield object goes out of scope here, destructor is called.
}