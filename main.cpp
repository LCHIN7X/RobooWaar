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
    void simulationStep();
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
    bool isDie = false;
    virtual bool isHit() = 0;
    bool getEnemyDetectedNearby() const;
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
    bool getIsDie() const { return isDie; }
    void setIsDie(bool val) { isDie = val; }    

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
            isDie = true; // Mark as dead for this turn
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


    //Stimulate and check hit probablity is 70 percent
    bool hitProbability() const
    {
        static random_device rd; // True random number generator 
        static mt19937 gen(rd()); // Mersenne Twister pseudo-random generator seeded with rd
        uniform_real_distribution<> dis(0.0, 1.0); // Distribution for numbers between 0.0 and 1.0
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
    void think() override {
        cout <<">> "<< name << " is thinking..." << endl;
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
    cout << ">> " << name << " is thinking...\n";
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
    if (isDie) return;
    if (!battlefield) {
        cout << name << " has no battlefield context!" << endl;
        return;
    }
    look(*battlefield);
    think();
    detectedTargets.clear();
}

void GenericRobot::move(Battlefield &battlefield)
{
    cout << ">> " << name << " is moving...\n";
    int dx[] = {0, 1, 0, -1};
    int dy[] = {1, 0, -1, 0};
    static std::random_device rd;
    static std::mt19937 g(rd());
    for (int i = 0; i < 4; ++i) {
        int newX = positionX + dx[i];
        int newY = positionY + dy[i];
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

void GenericRobot::fire(Battlefield &battlefield)
{
    if (hasAmmo()) {
        if (!detectedTargets.empty()) {
            int idx = rand() % detectedTargets.size();
            Robot* target = detectedTargets[idx];
            int targetX = target->getX();
            int targetY = target->getY();
            std::cout << ">> " << name << " fires at (" << targetX << ", " << targetY << ")" << std::endl;
            useAmmo();
            if (target->isHidden()) {
                std::cout << target->getName() << " is hidden, attack miss." << std::endl;
            }
            else if (hitProbability()) {
                std::cout << "Hit! (" << target->getName() << ") be killed" << std::endl;
                target->takeDamage();
                static const std::vector<std::string> types = {"HideBot","JumpBot","LongShotBot","SemiAutoBot","ThirtyShotBot","ScoutBot","TrackBot","KnightBot"};
                int t = rand() % types.size();
                setPendingUpgrade(types[t]);
                std::cout << name << " will upgrade into " << types[t] << " next turn!" << std::endl;
            }
            else {
                std::cout << "Missed!" << std::endl;
            }
        }
        else {
            useAmmo();
            std::cout << ">> " << name << " fires." << endl;
            cout << "However no robots within shooting range ." << std::endl;
        }
    }
    else {
        std::cout << name << " has no ammo left!" << std::endl;
    }
    detectedTargets.clear();
}

void GenericRobot::look(Battlefield &battlefield)
{
    cout << ">> " << name << " is scanning surroundings...." << endl;
    int cx = positionX;
    int cy = positionY;
    enemyDetectedNearby = false;
    //detectedTargets.clear();
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
                } else if (r->getLives() > 0 && !r->getIsDie()){
                    cout << "Enemy detected: " << r->getName() << " at (" << nx << "," << ny << ")" << endl;
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
                    cout << "Dead Robot" << endl;
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

    void move(Battlefield &battlefield) override {
    if (hide_count < 3 && rand() % 2 == 0) { //put 50/50 first,then i ask lec how 
        isHidden = true;
        hide_count++;
        setHidden(true);
        cout << getName() << " hide,(" << hide_count << "/3)" << endl;
    }
    else {
        isHidden = false;
        setHidden(false);

        if (hide_count >= 3)
            cout << getName() << " finish use hide , keep moving" << endl;
        else
            cout << getName() << " did not hide this turn, keep moving" << endl;
    }
}

    bool getHiddenStatus() const{
        return isHidden;
    }

    void appear(){
        isHidden = false;
    }

    void act() override {
        think();
        look(*battlefield);
        fire(*battlefield);
        move(*battlefield);
    }

    bool isHit() override {
        return !isHidden;
    } ///
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
        if (jump_count < 3 && rand() % 2 == 0){
            jump_count++;
            int jumpx = rand() % battlefield.getWidth();
            int jumpy = rand() % battlefield.getHeight();

            setPosition(jumpx,jumpy);
            cout << getName() << "jump to ("<< jumpx << ","<<jumpy<< ")\n";

        }
        else{
            cout<< getName() << " cannot jump already\n";
        }
    }

    void act() override {
    think();
    look(*battlefield);
    fire(*battlefield);
    move(*battlefield);
    }

    int getJumpCount() const{
        return jump_count;
    }

};

//******************************************
//LongShotBot
//******************************************

class LongShotBot : public GenericRobot{
    private:
    int fire_count = 0;
    const std::vector<std::string> upgradeTypes = {"HideBot", "JumpBot", "ScoutBot", "TrackBot"};

    public:
    LongShotBot(const string &name,int x,int y)
    : Robot(name,x,y),
      GenericRobot(name,x,y){}

    void fire(Battlefield &battlefield) override {

        bool fired = false;
        int x = getX();
        int y = getY();


        for (int dx = -3; dx <= 3; dx++){
            for (int dy = -3; dy <= 3;dy++){
                if (dx == 0 && dy == 0) continue;
                if (abs(dx)+abs(dy) >3) continue;

                int targetX = x+dx;
                int targetY = y+dy;

                Robot* target = battlefield.getRobotAt(targetX,targetY);

                if (target && target != this){

                    GenericRobot* gtarget = dynamic_cast<GenericRobot*>(target);
                    cout << getName() << " fire ("<< targetX<<","<<targetY<<")"<<endl;
                    if (gtarget->isHit()){
                        gtarget->takeDamage();
                        cout << getName() << " hit the target " <<gtarget->getName()<< endl;
                        fire_count++;
                        fired= true;
                        int t = rand() % upgradeTypes.size();
                        string newType = upgradeTypes[t];
                        setPendingUpgrade(newType);
                        cout << getName() << " will upgrade into " << newType << " next turn!" << endl;
                        break;
                    }
                }
            }
            if (fired)break;
        }
            if (!fired){
                cout<<getName()<<" no robot there\n";
        }
        
    }

};

//******************************************
//SemiAutoBot
//******************************************

class SemiAutoBot : public GenericRobot{
    private:
    int fire_count = 0;
    const std::vector<std::string> upgradeTypes = {"HideBot", "JumpBot", "ScoutBot", "TrackBot"};

    public:
    SemiAutoBot(const string &name,int x,int y)
    : Robot(name,x,y),
      GenericRobot(name,x,y){}

    void fire(Battlefield &battlefield) override{
        int x = getX();
        int y = getY();

        Robot* target = nullptr;
        for (Robot* r : battlefield.getListOfRobots()) {
            if (r != nullptr && r != this && r->getLives() > 0) {
                target = r;
                break; 
             }
}
        
        if(!target){
            cout<< getName()<<" sadly didnt hit any robot\n";
            return;
        }

       GenericRobot* gtarget = dynamic_cast<GenericRobot*>(target);

        cout << getName() << "fire 3 consecutive shoot at ("<<x<<","<<y<<")\n";

        bool hitSuccessful = false;
        for (int i = 0;i <3;i ++){
            double chance = (double)rand()/RAND_MAX;
            if (chance < 0.7) {
                cout << "shot " << (i + 1) <<" successful hit the robot\n"<< gtarget->getName() << "!\n";
                if (gtarget->isHit()){
                    gtarget->takeDamage();
                    fire_count++;
                    hitSuccessful = true;
                }
                
            }
            else{
                cout << "shot "<< (i+1)<< " is miss\n";
                    
                }
        }
        if (hitSuccessful) {
            int t= rand() % upgradeTypes.size();
            string newType = upgradeTypes[t];
            setPendingUpgrade(newType);
            cout << getName() << " will upgrade into " << newType << " next turn!\n";
        }
    }
    int getFireCount() const{
        return fire_count;
    }
};

//******************************************
//ThirtyShotBot
//******************************************
class ThirtyShotBot : public GenericRobot {
private:
    int shell_count;
    const std::vector<std::string> upgradeTypes = {"HideBot", "JumpBot", "ScoutBot", "TrackBot"};

public:
    ThirtyShotBot(const string &name, int x, int y)
        : Robot(name, x, y),
          GenericRobot(name, x, y),
          shell_count(30) 
    {
        cout << name << " got 30 shells replace current shells \n";
    }

    void fire(Battlefield &battlefield) override {
        if (shell_count <= 0) {
            cout << getName() << " shell is finish\n";
            return;
        }

        int x = getX();
        int y = getY();
        bool fired = false;
        bool hitSuccessful = false;

        for (int dx = -1; dx <= 1 && !fired; dx++) {
            for (int dy = -1; dy <= 1 && !fired; dy++) {
                int targetX = x + dx;
                int targetY = y + dy;

                Robot* target = battlefield.getRobotAt(targetX, targetY);
                if (target && target != this) {
                    GenericRobot* gtarget = dynamic_cast<GenericRobot*>(target);
                    if (gtarget && gtarget->isHit()) {
                        gtarget->takeDamage();
                        shell_count--;
                        cout << getName() << " fire at (" << targetX << "," << targetY<< "), shell left: " << shell_count << "\n";
                        fired = true;

                        //Display target that be hitted
                        if (gtarget->isHit()) {
                            gtarget->takeDamage();
                            hitSuccessful = true;
                            cout << "Successful hit on " << gtarget->getName() << "!\n";

                            int t = rand() % upgradeTypes.size();
                            string newType = upgradeTypes[t];
                            setPendingUpgrade(newType);
                            cout << getName() << " will upgrade into " << newType << " next turn!\n";
                        }
                    }
                }
            }
        }

        if (!fired) {
            cout << getName() << " no target to shoot\n";
        }
    }

    int getShellCount() const {
        return shell_count;
    }
};

//******************************************
// KnightBot
//******************************************

class KnightBot : public GenericRobot {
private:
    int fire_count = 0;
     const std::vector<std::string> upgradeTypes = {"HideBot", "JumpBot", "ScoutBot", "TrackBot"};

public:
    KnightBot(const string &name, int x, int y)
        : Robot(name, x, y),
          GenericRobot(name, x, y) {}

    void fire(Battlefield &battlefield) override {
        int x = getX();
        int y = getY();
        bool anyFired = false;
        bool hitSuccessful = false;
        std::vector<std::string> hitRobots;

        for (Robot* target : battlefield.getListOfRobots()) {
            if (target && target != this && target->getLives() > 0) {
                int dx = target->getX() - x;
                int dy = target->getY() - y;
                double distance = sqrt(dx * dx + dy * dy);
                if (distance <= 8.0) {
                    GenericRobot* gtarget = dynamic_cast<GenericRobot*>(target);
                    cout << getName() << " fires at (" << target->getX() << "," << target->getY() << ")" << endl;
                    anyFired = true;
                    if (gtarget && gtarget->isHit()) {
                        gtarget->takeDamage();
                        fire_count++;
                        hitSuccessful = true;
                        hitRobots.push_back(gtarget->getName());
                    }
                }
            }
        }
        if (!anyFired) {
            cout << getName() << " no robots in range to fire at\n";
        } else {
            cout << getName() << " hit the following robots: ";
            for (size_t i = 0; i < hitRobots.size(); ++i) {
                cout << hitRobots[i];
                if (i != hitRobots.size() - 1) cout << ", ";
            }
            cout << endl;
            int t = rand() % upgradeTypes.size();
            string newType = upgradeTypes[t];
            setPendingUpgrade(newType);
            cout << getName() << " will upgrade into " << newType << " next turn!" << endl;
        }
    }
};

//******************************************
//ScoutBot
//******************************************
class ScoutBot : public GenericRobot {
private:
    int scout_count = 0;

public:
    ScoutBot(const string &name, int x, int y)
        : Robot(name, x, y),
          GenericRobot(name, x, y) {}

    void look(Battlefield &battlefield) {
        if (scout_count >= 3) {
            cout << getName() << " reach the limit,cannot scan already\n";
            return;
        }

        if (rand() % 2 == 0) {
            cout << getName() << " scan the battlefield\n";

            for (int y = 0; y < battlefield.getHeight(); ++y) {
                for (int x = 0; x < battlefield.getWidth(); ++x) {
                    Robot* r = battlefield.getRobotAt(x, y);
                    if (r) {
                        cout << "got robot: " << r->getName()
                             << " at (" << x << "," << y << ")\n";
                    }
                }
            }

            scout_count++; 
        } else {
            cout << getName() << " try scan it next round \n";
        }
    }

    void act() override {
        think();
        look(*battlefield);
        fire(*battlefield);
        move(*battlefield);
    }

    int getScoutCount() const {
        return scout_count;
    }
};


//******************************************
//TrackBot
//******************************************

class TrackBot: public GenericRobot{
    private:
    int tracker = 3;
    vector<Robot*> track_target;

    public:
    TrackBot(const string &name,int x,int y)
    : Robot(name,x,y),
      GenericRobot(name,x,y){}

    void look(Battlefield &battlefield){
        if (tracker == 0){
            cout << getName() << " cannot track robot already\n";
            return;
        }

        int x = getX();
        int y = getY();
        bool plant = false;

        for (int dx = -1;dx <= 1 && !plant; dx++){
            for (int dy = -1;dy <= 1 && !plant; dy++){
                int targetX = x + dx;
                int targetY = y + dy;

                Robot* target = battlefield.getRobotAt(targetX,targetY);

                if (target && target != this){
                    track_target.push_back(target);
                    tracker--;
                    cout << getName() << " track "<< target -> getName()<< " at ("<<targetX<<","<<targetY<<")\n";
                    plant = true;

                }
            }
        }
        if (!plant){
            cout<<getName()<<" no target can track\n";
        }
    }

    void act() override {
        think();
        look(*battlefield);
        fire(*battlefield);
        move(*battlefield);
    }

    void showTrackTarget(){
        if (track_target.empty()){
            cout<<getName()<<" didnt track any robot\n";
            return;
        }
        cout<< getName()<<" is tracking:\n";
        for (Robot* r :track_target){
            cout<<r->getName()<<" at ("<<r->getX()<<","<<r->getY()<<")\n";
        }
    }
    int getTracker() const{
        return tracker;
    }


};


//******************************************
//Queue , Respawnn and Reentry Logic
//******************************************

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

//******************************************
// simulationStep member function of Battlefield class (declared later to avoid issues with code not seeing each other when they need to)
//******************************************

void Battlefield::simulationStep()
{
    // Handle upgrades first
    for (size_t i = 0; i < listOfRobots.size(); ++i) {
        GenericRobot* gen = dynamic_cast<GenericRobot*>(listOfRobots[i]);
        if (gen && gen->PendingUpgrade()) {
            std::string type = gen->getUpgradeType();
            Robot* upgraded = nullptr;
            if (type == "HideBot") upgraded = new HideBot(gen->getName(), gen->getX(), gen->getY());
            else if (type == "JumpBot") upgraded = new JumpBot(gen->getName(), gen->getX(), gen->getY());
            else if (type == "LongShotBot") upgraded = new LongShotBot(gen->getName(), gen->getX(), gen->getY());
            else if (type == "KnightBot") upgraded = new KnightBot(gen->getName(), gen->getX(), gen->getY());
            else if (type == "SemiAutoBot") upgraded = new SemiAutoBot(gen->getName(), gen->getX(), gen->getY());
            else if (type == "ThirtyShotBot") upgraded = new ThirtyShotBot(gen->getName(), gen->getX(), gen->getY());
            else if (type == "ScoutBot") upgraded = new ScoutBot(gen->getName(), gen->getX(), gen->getY());
            else if (type == "TrackBot") upgraded = new TrackBot(gen->getName(), gen->getX(), gen->getY());
            if (upgraded) {
                upgraded->setLives(gen->getLives());
                // Fix: clear the upgrade flag so the new bot doesn't try to upgrade again
                GenericRobot* upGen = dynamic_cast<GenericRobot*>(upgraded);
                if (upGen) upGen->clearPendingUpgrade();
                removeRobotFromGrid(gen);
                placeRobot(upgraded, gen->getX(), gen->getY());
                delete gen;
                listOfRobots[i] = upgraded;
                cout << upgraded->getName() << " has upgraded to " << type << "!" << endl;
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
        if (auto gen = dynamic_cast<GenericRobot*>(robot)) {
            gen->setBattlefield(this);
        }
        cout<<"----------------------------------------" << endl;
        cout << robot->getName() << "'s turn: " << endl;
        robot->act();
        cout << robot->getName() << " is done." << endl;
    }

    cleanupDestroyedRobots();
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
    cout << "----------------------------------------" << endl;
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

            // Check if the robot can respawn
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

void parseInputFile(const string &line, Battlefield &battlefield) {
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
    else if (tokens[0] == "LongShotBot" && tokens.size() >= 4){
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

        Robot* newRobot = new LongShotBot(robotName,robotXCoordinates,robotYCoordinates);
        battlefield.addNewRobot(newRobot);
        battlefield.placeRobot(newRobot,robotXCoordinates,robotYCoordinates);
    }
    else if (tokens[0] == "SemiAutoBot" && tokens.size() >= 4){
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

        Robot* newRobot = new SemiAutoBot(robotName,robotXCoordinates,robotYCoordinates);
        battlefield.addNewRobot(newRobot);
        battlefield.placeRobot(newRobot,robotXCoordinates,robotYCoordinates);
    }
    else if (tokens[0] == "ThirtyShotBot" && tokens.size() >= 4){
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

        Robot* newRobot = new ThirtyShotBot(robotName,robotXCoordinates,robotYCoordinates);
        battlefield.addNewRobot(newRobot);
        battlefield.placeRobot(newRobot,robotXCoordinates,robotYCoordinates);
    }
    else if (tokens[0] == "ScoutBot" && tokens.size() >= 4){
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

        Robot* newRobot = new ScoutBot(robotName,robotXCoordinates,robotYCoordinates);
        battlefield.addNewRobot(newRobot);
        battlefield.placeRobot(newRobot,robotXCoordinates,robotYCoordinates);
    }
    else if (tokens[0] == "TrackBot" && tokens.size() >= 4){
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

        Robot* newRobot = new TrackBot(robotName,robotXCoordinates,robotYCoordinates);
        battlefield.addNewRobot(newRobot);
        battlefield.placeRobot(newRobot,robotXCoordinates,robotYCoordinates);
    }
    else if (tokens[0] == "KnightBot" && tokens.size() >= 4){
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

        Robot* newRobot = new KnightBot(robotName,robotXCoordinates,robotYCoordinates);
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
   
    for (Robot *robot : battlefield.getListOfRobots())
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

        //TO DO:able to display proper simulation step
        cout << "Robot Status before Step" << currentStep + 1 << ":" << endl;
        for (Robot* robot : battlefield.getListOfRobots()) {
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
                else type = "GenericRobot";
            }
        } else {
            type = "Robot";
        }
            //TO DO :able to display current robot type
            cout << "  Type: " << type
                 << ", Name: " << robot->getName()
                 << ", Coords: (" << robot->getX() << "," << robot->getY() << ")"
                 << ", Life: " << robot->getLives() << endl;
        }

        battlefield.simulationStep(); // Executes turns, cleans up, respawns

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