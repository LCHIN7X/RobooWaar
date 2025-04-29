#include <iostream>
#include <fstream>
#include <vector>
using namespace std;

//  Robot class (Grandparent Class)
class Robot
{
    private:
        int robotPositionX;
        int robotPositionY;
        int shells;
        int lives;
};


//  Battlefield class that keeps track of all activity
class Battlefield {
    private:
        int height;  // this is the m value from m x n
        int width;   //  this is the n value from m x n
        vector<Robot> listOfRobots;        
};

int main()
{
    //  This function should:
    //    1. Read input file
    //    2. Initialize battlefield 
    //    3. Place robots into the environment and add into environment robot list
    //    4. Start simulation:
    //       - Let each robot take a turn
    //       - Repeat... till only one robot remains or simulation time is finished

    return 0;
}