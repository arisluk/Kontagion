#ifndef STUDENTWORLD_H_
#define STUDENTWORLD_H_

#include "GameWorld.h"
#include <string>

#include <vector>
class Actor;
class Socrates;

// Students:  Add code to this file, StudentWorld.cpp, Actor.h, and Actor.cpp

//////////
// Utility Function declarations
//////////

double getDistance(Actor* a, Actor* b); // Get Euclidean distance between two actors
double getDistance(Actor* a, int x, int y); // Overload getDistance for instances between actor and point (x,y)
double getDistanceFromCenter(int x, int y); // Get Euclidian distance from center to (x,y) coords
double getDistanceFromCenter(Actor* a); // Overload for distance from center to an actor
void polarToCartesian(int oX, int oY, int r, int theta, int &x, int &y); // Convert polar to cartesian
void cartesianToPolar(int oX, int oY, int x, int y, int &r, int &theta); // Convert cartesian to polar
bool overlaps(Actor* a, Actor* b); // Checks if two actors overlap
bool overlaps(Actor* a, int x, int y); // Overload for instances to check between Actor and point (x,y)

//////////
// StudentWorld class declaration
//////////

class StudentWorld : public GameWorld
{
public:
    StudentWorld(std::string assetPath);
    // Virtual functions
    virtual int init();
    virtual int move();
    virtual void cleanUp();
    // Member functions
    void addActor(Actor* a); // Add an actor to world
    bool damageNearbyActor(Actor* a, int dmg); // Damage nearby overlapping actor that can be damaged
    bool overlapsPlayer(Actor* a); // Check if actor overlaps Socrates
    bool eatNearbyFood(Actor* a); // Removes nearest overlapping edible actor
    bool validMove(Actor* a, int dir, int dist); // Checks if passed actor and dir/dist is a valid move for bacteria
    bool getAngleToNearestFood(Actor* a, int dist, int &angle); // Gets the angle to nearest food
    bool getAngleToSocrates(Actor* a, int dist, int &angle); // Gets the angle to socrates
    void restorePlayerHP(int hp); // Sets Socrates hp to new value
    void addPlayerFlames(int flames); // adds flames to Socrates
    void damagePlayer(int hp); // Subtracts hp from socrates.
    // Destructor
    ~StudentWorld();

private:
    // Data Members
    Socrates* m_player; // Pointer to Socrates player
    std::vector<Actor*> m_actors; // Vector of pointers to Actors in world
    int m_nActors; // Tracks number of actors in vector
    // Member Functions
    void genUniqueCoords(int &x, int &y, int nActors); // Generates unique valid coords inside petri dish
    void addItems(); // Method to randomly add goodies at each tick
    void deleteDead(); // Clears dead actors and removes them from world
    void updateStatusText(); // Updates the game status text
    bool isLevelOver(); // Checks if the level is over
};

#endif // STUDENTWORLD_H_
