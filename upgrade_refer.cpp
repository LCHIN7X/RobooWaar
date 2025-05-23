//******************************************
// HideBot
//******************************************

class HideBot : public virtual GenericRobot *****  //Important: inheric from puclic virtual GenericRobot
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
********************************************************************************************
    void fire() override {
        GenericRobot::fire();
        // HideBot always upgrades to HideLongShotBot after a hit
        if (PendingUpgrade()) {
            setPendingUpgrade("HideLongShotBot");
            logger << getName() << " will upgrade into HideLongShotBot next turn!" << endl;
        }
    }
***********************************************************************************************
    bool isHit() override
    {
        return !isHidden;
    }
};

//******************************************
// HideLongShotBot (inherits from HideBot and LongShotBot)
//******************************************
class HideLongShotBot : public HideBot, public LongShotBot {
public:
    HideLongShotBot(const string &name, int x, int y)
        : Robot(name, x, y),
          GenericRobot(name, x, y),
          HideBot(name, x, y),
          LongShotBot(name, x, y) {}

    void move() override {
        HideBot::move();
    }

    void fire() override {
        LongShotBot::fire();
    }

    void think() override {
        // Disambiguate GenericRobot base
        HideBot::think();
    }

    void act() override {
        LongShotBot::look(0, 0); // Use long-range look for targeting
        think();
        fire();
        move();
    }

    void look(int X, int Y) override {
        LongShotBot::look(X, Y); // Use long-range look for any explicit look calls
    }

    bool isHit() override {
        return HideBot::isHit();
    }

    void setBattlefield(Battlefield* bf) {
        GenericRobot::setBattlefield(bf);
    }
};

//******************************************
// simulationStep member function of Battlefield class (declared later to avoid issues with code not seeing each other when they need to)
//**
else if (type == "HideLongShotBot")
                upgraded = new HideLongShotBot(gen->getName(), gen->getX(), gen->getY());

int main
else if (dynamic_cast<HideLongShotBot *>(robot))
                        type = "HideLongShotBot";