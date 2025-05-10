#include <iostream>
#include <string>
#include <vector>
#include <memory>
#include <random>

class BattleField;

// Abstract base Robot class
class Robot {
protected:
    std::string name;
    int positionX;
    int positionY;
    int lives;
    bool hidden;
    
public:
    Robot(const std::string& name, int x, int y) 
        : name(name), positionX(x), positionY(y), lives(3), hidden(false) {}
    
    virtual ~Robot() = default;
    virtual void think() = 0;
    virtual void act() = 0;
    
    // Common robot functions
    std::string getName() const { return name; }
    int getX() const { return positionX; }
    int getY() const { return positionY; }
    int getLives() const { return lives; }
    bool isHidden() const { return hidden; }
    
    void setPosition(int x, int y) {
        positionX = x;
        positionY = y;
    }
    
    void takeDamage() {
        if (!hidden) {
            lives--;
            if (lives <= 0) {
                std::cout << name << " has been destroyed!\n";
            }
        }
    }
    
    void setHidden(bool state) { hidden = state; }
};

//Moving Robot class
class MovingRobot : public virtual Robot {
protected:
    int moveCount;
    
public:
    MovingRobot(const std::string& name, int x, int y) 
        : Robot(name, x, y), moveCount(0) {}
    
    virtual ~MovingRobot() = default;
    virtual void move(BattleField& battlefield) = 0;
    bool isValidMove(int newX, int newY, const BattleField& battlefield) const;
    void incrementMoveCount() { moveCount++; }
};

//Shooting Robot class
class ShootingRobot : public virtual Robot {
protected:
    int ammo;
    
public:
    ShootingRobot(const std::string& name, int x, int y, int initialAmmo) 
        : Robot(name, x, y), ammo(initialAmmo) {}
    
    virtual ~ShootingRobot() = default;
    virtual void fire(BattleField& battlefield) = 0;
    bool hasAmmo() const { return ammo > 0; }
    void useAmmo() { if (ammo > 0) ammo--; }
    int getAmmo() const { return ammo; }
    
    bool hitProbability() const {
        static std::random_device rd;
        static std::mt19937 gen(rd());
        std::uniform_real_distribution<> dis(0.0, 1.0);
        return dis(gen) <= 0.7; // 70% hit chance
    }
};

//Seeing Robot class
class SeeingRobot : public virtual Robot {
protected:
    int visionRange;
    
public:
    SeeingRobot(const std::string& name, int x, int y, int range) 
        : Robot(name, x, y), visionRange(range) {}
    
    virtual ~SeeingRobot() = default;
    virtual void look(BattleField& battlefield) = 0;
    int getVisionRange() const { return visionRange; }
    bool canSee(int targetX, int targetY) const;
};

// Thinking Robot class
class ThinkingRobot : public virtual Robot {
protected:
    int strategyLevel;
    
public:
    ThinkingRobot(const std::string& name, int x, int y, int strategy) 
        : Robot(name, x, y), strategyLevel(strategy) {}
    
    virtual ~ThinkingRobot() = default;
    virtual void think() override = 0;
    int getStrategyLevel() const { return strategyLevel; }
    virtual std::string decideAction() const = 0;
};

// GenericRobot class
class GenericRobot : public MovingRobot, public ShootingRobot, 
                     public SeeingRobot, public ThinkingRobot {
private:
    bool hasUpgraded[3] = {false, false, false}; // Track upgrades for moving, shooting, seeing
    
public:
    GenericRobot(const std::string& name, int x, int y) 
        : Robot(name, x, y),
          MovingRobot(name, x, y),
          ShootingRobot(name, x, y, 10), // Start with 10 ammo
          SeeingRobot(name, x, y, 1),    // Basic vision range of 1
          ThinkingRobot(name, x, y, 1) {} // Basic strategy level
    
    ~GenericRobot() override = default;
    void think() override {
        std::cout << name << " is thinking...\n";
        // to be added Basic decision making logic
    }
    
    void act() override {
        think();
        //to be added Perform actions based on decision
    }
    
    void move(BattleField& battlefield) override {
        //to be added Basic movement logic
        std::cout << name << " is moving...\n";
    }
    
    void fire(BattleField& battlefield) override {
        if (hasAmmo()) {
            std::cout << name << " is firing...\n";
            useAmmo();
        } else {
            std::cout << name << " has no ammo left!\n";
        }
    }
    
    void look(BattleField& battlefield) override {
        std::cout << name << " is looking around...\n";
        //to be added Basic vision logic
    }
    
    std::string decideAction() const override {
        if (hasAmmo()) return "fire";
        return "move";
    }
    
    // Upgrade functions
    bool canUpgrade(int area) const {
        if (area < 0 || area > 2) return false;
        return !hasUpgraded[area];
    }
    
    void setUpgraded(int area) {
        if (area >= 0 && area < 3) {
            hasUpgraded[area] = true;
        }
    }
};

// BattleField class declaration (later relaced by battlefield code ******)
class BattleField {
private:
    int width;
    int height;
    std::vector<std::shared_ptr<Robot>> robots;
    
public:
    BattleField(int w, int h) : width(w), height(h) {}
    
    int getWidth() const { return width; }
    int getHeight() const { return height; }
    
    void addRobot(std::shared_ptr<Robot> robot) {
        robots.push_back(robot);
    }
};

// Implementation of common functions
bool MovingRobot::isValidMove(int newX, int newY, const BattleField& battlefield) const {
    return newX >= 0 && newX < battlefield.getWidth() &&
           newY >= 0 && newY < battlefield.getHeight();
}

bool SeeingRobot::canSee(int targetX, int targetY) const {
    int dx = targetX - positionX;
    int dy = targetY - positionY;
    return dx*dx + dy*dy <= visionRange * visionRange;
}

int main() {
    // Example hardcode (later relaced by read input file code ******)
    BattleField battlefield(80, 50);
    
    auto robot1 = std::make_shared<GenericRobot>("Kidd", 3, 6);
    auto robot2 = std::make_shared<GenericRobot>("Jet", 12, 1);
    
    battlefield.addRobot(robot1);
    battlefield.addRobot(robot2);
    
    std::cout << "Robot War Simulator Initialized!\n";
    std::cout << "Battlefield size: " << battlefield.getWidth() << "x" << battlefield.getHeight() << "\n";
    
    return 0;
}