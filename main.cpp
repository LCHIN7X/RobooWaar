#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <sstream>
using namespace std;

// Forward declarations
class Battlefield;

// GENERAL FUNCTIONS
void parseInputFile(const string &line, Battlefield &battlefield);

//  Robot class (Grandparent Class)
class Robot
{
private:
    int robotPositionX;
    int robotPositionY;
    int shells;
    int lives;

public:
    int getPositionX() const
    {
        return robotPositionX;
    }

    int getPositionY() const
    {
        return robotPositionY;
    }

    void setPositionX(int x)
    {
        robotPositionX = x;
    }

    void setPositionY(int y)
    {
        robotPositionY = y;
    }
};

//  Battlefield class that keeps track of all activity
class Battlefield
{
private:
    int height; // this is the m value from m x n
    int width;  //  this is the n value from m x n
    int steps;  // number of simulation steps
    int numberOfRobots;
    vector<Robot> listOfRobots;

public:
    Battlefield() : height(0), width(0) {};  // Initialize in member initializer list

    void setDimensions(int h, int w)
    {
        height = h;
        width = w;
    }

    void printDimensions() const {
        cout << "Height: " << height << endl;
        cout << "Width: " << width << endl;
    }

    void setSteps(int s) {
        steps = s;
    }

    void printSteps() const {
        cout << "Steps: " << steps << endl;
    }

    void setNumberOfRobots(int n) {
        numberOfRobots = n;
    }

    void getNumberOfRobots() const {
        cout << "Number of robots: " << numberOfRobots << endl;
    }

    void respawnRobots() {};  // Stub for now
};

// Function implementations
void parseInputFile(const string &line, Battlefield &battlefield)
{
    vector<string> tokens;
    istringstream iss(line);
    string token;

    while (iss >> token)
    {
        tokens.push_back(token);
        cout << token << endl;
    }

    if (tokens[0] == "M" && tokens.size() >= 6)
    {
        int height = stoi(tokens[4]);  // stoi = string to int
        int width = stoi(tokens[5]);
        battlefield.setDimensions(height, width);
    }

    else if (tokens[0] == "steps:" && stoi(tokens[1])) {
        int steps = stoi(tokens[1]);
        battlefield.setSteps(steps);
    }

    else if (tokens[0] == "robots:" && stoi(tokens[1])) {
        int numRobots = stoi(tokens[1]);
        battlefield.setNumberOfRobots(numRobots);
    }
}

Battlefield readInputFile(const string &filename = "inputFile.txt")
{
    ifstream inputFile(filename);
    Battlefield battlefield;

    if (!inputFile.is_open())
    {
        cerr << "Error opening input file." << endl;
        return battlefield;
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
    return battlefield;
}

int main()
{
    Battlefield battlefield = readInputFile();
    battlefield.printDimensions();
    battlefield.printSteps();

    return 0;
}