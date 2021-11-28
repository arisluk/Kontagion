#include "StudentWorld.h"
#include "GameConstants.h"
#include <string>
using namespace std;

#include <cmath>
#include <sstream>
#include <iomanip>
#include "Actor.h"

GameWorld* createStudentWorld(string assetPath)
{
    return new StudentWorld(assetPath);
}

// Students:  Add code to this file, StudentWorld.h, Actor.h and Actor.cpp

//////////
// StudentWorld Implementation
//////////

// StudentWorld Constructor
StudentWorld::StudentWorld(string assetPath)
: GameWorld(assetPath)
{
    // Construction sets player pointer to null and number of actors to 0
    m_player = nullptr;
    m_nActors = 0;
}

// StudentWorld Destructor
StudentWorld::~StudentWorld()
{
    // Calls cleanup method to cleanup world
    cleanUp();
}

// StudentWorld Initializer
int StudentWorld::init()
{
    // Construct Socrates
    m_player = new Socrates(this);
    // Construct Pits
    int level = getLevel(); // Gets current level
    int x, y;
    for (int i = 0; i < level; i++) // Loops (level) times
    {
        genUniqueCoords(x, y, m_nActors); // Generate unique coords with respect to all actors so far (pits only)
        addActor(new Pit(x, y, this)); // Add Pit to world with coords
    }
    // Construct Food
    for (int i = 0; i < min(5*level, 25); i++) // Loops specified times as shown in spec for Food
    {
        genUniqueCoords(x, y, m_nActors); // Generate unique coords with respect to all actors so far (pits and food only)
        addActor(new Food(x, y, this)); // Add Food to world with coords
    }
    // Construct Dirt
    int nPitsAndFood = m_nActors; // Record how many pits and food there are currently in the world since only pits and food have been constructed
    for (int i = 0; i < max(180-20*level, 20); i++) // Loop specified times as in spec for Dirt
    {
        genUniqueCoords(x, y, nPitsAndFood); // Generate unique coords with respect to only the Pits and Food (Dirt can overlap each other)
        addActor(new Dirt(x, y, this)); // Add Dirt to world with coords
    }
    
    return GWSTATUS_CONTINUE_GAME; // Continue game
}

// StudentWorld move function for every tick
int StudentWorld::move()
{
    // Make Socrates do its action
    m_player->doSomething();
    if (m_player->isDead()) // Check if player is killed
    {
        decLives(); // Decrease world lives
        return GWSTATUS_PLAYER_DIED; // Return to game that player died
    }
    if (isLevelOver()) // Check if Level is completed
    {
        playSound(SOUND_FINISHED_LEVEL); // Play level completion soun
        return GWSTATUS_FINISHED_LEVEL; // Return to game the level is completed
    }
    
    for (int i = 0; i < m_nActors; i++) // Loops through all actors in world
    {
        if (!m_actors[i]->isDead()) // Check if current actor is alive
        {
            m_actors[i]->doSomething(); // Make actor do something
            
            if (m_player->isDead()) // Check if player is killed
            {
                decLives(); // Decrease world lives
                return GWSTATUS_PLAYER_DIED; // Return to game that player died
            }
            if (isLevelOver()) // Check if Level is completed
            {
                playSound(SOUND_FINISHED_LEVEL); // Play level completion soun
                return GWSTATUS_FINISHED_LEVEL; // Return to game the level is completed
            }
        }
    }
    // Delete dead Actors
    deleteDead();
    // Add Actors
    addItems();
    // Update Status Line
    updateStatusText();
    
    return GWSTATUS_CONTINUE_GAME; // Continue game
}

// StudentWorld cleanUp method to deallocate all actors in world
void StudentWorld::cleanUp()
{
    delete m_player; // Delete Socrates
    m_player = nullptr; // Set player pointer to null to prevent double deletion
    auto it = m_actors.begin(); // Create iterator pointing to start of actor vector
    while (it != m_actors.end()) // Loop through all actors in vector
    {
        delete (*it); // Deallocate actor
        it = m_actors.erase(it); // Erase actor pointer from vector
    }
    if (m_actors.size() != 0) // Check if all actors in vector have been deleted
    {
        // Error message and exit if cleanup fails (vector isn't empty)
        // Should never reach here
        cerr << "BAD CLEANUP" << endl;
        exit(1);
    }
    m_nActors = 0; // Reset number of actors tracker to 0
}

// StudentWorld method to add an actor to the world
void StudentWorld::addActor(Actor* a)
{
    m_actors.push_back(a); // Pushes passed in actor pointer to the actor vector
    m_nActors++; // Increments actor number tracker
}

// StudentWorld method to damage a nearby overlapping Actor
bool StudentWorld::damageNearbyActor(Actor* a, int dmg)
{
    // Loop through whole actor vector using iterator
    auto it = m_actors.begin();
    while (it != m_actors.end())
    {
        if (!(*it)->isDead() && overlaps(a, (*it))) // Check if current Actor in vector and passed in Actor overlap, and if current is alive
        {
            if ((*it)->damage(dmg)) // Attempts to damage current Actor
                return true; // Return true if damage succeeds
        }
        it++;
    }
    return false; // Return false if no overlapping actor or overlapping actors can't be damaged
}

// StudentWorld method that returns if passed in actor overlaps Socrates
bool StudentWorld::overlapsPlayer(Actor* a)
{
    if (overlaps(a, m_player)) // Checks if passed in actor overlaps with Socrates
        return true;
    return false;
}

// StudentWorld method that deletes overlapping edible
bool StudentWorld::eatNearbyFood(Actor* a)
{
    // Loop through actor vector using iterator
    auto it = m_actors.begin();
    while (it != m_actors.end())
    {
        if (!(*it)->isDead() && overlaps(a, (*it)) && (*it)->isEdible()) // Check if current Actor and passed in actor overlaps and that current Actor is edible, and if it is alive
        {
            (*it)->setDead(); // Kill the edible Actor and return true
            return true;
        }
        it++;
    }
    return false;
}

// StudentWorld method that returns whether a passed in actor and specified polar movement is valid (for bacteria movement so it uses mvmt overlap definition)
bool StudentWorld::validMove(Actor* a, int dir, int dist)
{
    int proposeX, proposeY; // Set up temp variables to save cartesian movement coords
    polarToCartesian(a->getX(), a->getY(), dist, dir, proposeX, proposeY); // Convert input polar coords to cartesian coords
    
    // Loop through all actors in World
    auto it = m_actors.begin();
    while (it != m_actors.end())
    {
        // Check if current actor will block bacteria and movement overlaps with proposed coords
        if ((*it)->blocksBacteria() && getDistance((*it), proposeX, proposeY) <= SPRITE_RADIUS)
            return false;
        else if (getDistanceFromCenter(proposeX, proposeY) >= VIEW_RADIUS) // Check if proposed coords will go out of dish
            return false;
        it++;
    }
    return true; // Return true if proposed movement is not blocked
}

// StudentWorld method that returns the angle from an actor to the nearest edible (within specified distance "dist")
bool StudentWorld::getAngleToNearestFood(Actor* a, int dist, int &angle)
{
    // Loop through all actors in World
    auto it = m_actors.begin();
    while (it != m_actors.end())
    {
        if ((*it)->isEdible() && getDistance(a, (*it)) <= dist) // Check if current Actor is edible and is within input distance
        {
            int radius; // Set up dummy variable to be passed into cartesianToPolar (won't be used)
            // Convert cartesian vector coords from actor to edible to polar coords
            // Pass angle through reference parameter back to caller
            cartesianToPolar(a->getX(), a->getY(), (*it)->getX(), (*it)->getY(), radius, angle);
            return true; // Return true since we found a valid edible
        }
        it++;
    }
    return false; // Return false if no edible found
}

// StudentWorld method that returns the angle from an Actor to Socrates within specified distance dist
bool StudentWorld::getAngleToSocrates(Actor* a, int dist, int &angle)
{
    if (getDistance(a, m_player) <= dist) // Check if passed Actor is within dist distance from Socrates
    {
        int radius; // Set up dummy variable to be passed into cartesianToPolar (won't be used)
        // Convert cartesian vector coords from actor to Socrates to polar coords
        // Pass angle through reference parameter back to caller
        cartesianToPolar(a->getX(), a->getY(), m_player->getX(), m_player->getY(), radius, angle);
        return true; // Return true since we found Socrates within input bounds
    }
    return false; // Return false if Socrates not found in input bound
}

// StudentWorld method to set Socrates HP to input hp
void StudentWorld::restorePlayerHP(int hp)
{
    m_player->restoreHP(hp); // Calls Socrates' restoreHP method
}

// StudentWorld method to add specified flames to Socrates
void StudentWorld::addPlayerFlames(int flames)
{
    m_player->addFlames(flames); // Calls Socrates' addFlames method
}

// StudentWorld method to damage Socrates by input hp
void StudentWorld::damagePlayer(int hp)
{
    if (!m_player->isDead())
        m_player->damage(hp); // Calls Socrates' damage method
}

// StudentWorld method to get unique coords in the petri dish that doesn't overlap the first nActors in the World's Actor vector
void StudentWorld::genUniqueCoords(int &x, int &y, int nActors)
{
    int tempX, tempY; // Declare temp x, y vars
    bool overlapping = false; // Declare boolean to check if proposed coords overlap with existing actor
    do
    {
        overlapping = false; // Reset overlapping tracker to false at each attempt
        tempX = randInt(0, VIEW_WIDTH); // Get random x coord
        tempY = randInt(0, VIEW_HEIGHT); // Get random y coord
        for (int i = 0; i < nActors; i++) // Check requested actors (first nActors) for overlap
            if (overlaps(m_actors[i], tempX, tempY))
                overlapping = true;
    } while (getDistanceFromCenter(tempX, tempY) > 120 || overlapping); // Generate coords until it is within radius and not overlapping with any of the first nActors
    // After getting a unique coord, pass the coords to the reference params
    x = tempX;
    y = tempY;
}

// StudentWorld method to randomly add goodies at each tick
void StudentWorld::addItems()
{
    int x, y; // Declare vars to store generated coords for goodies
    // Add Fungus
    int chanceFungus = max(510 - getLevel() * 10, 200); // Get chanceFungus chance from spec
    if (randInt(0, chanceFungus-1) == 0) // If chance succeeds
    {
        // Add fungus
        int randTheta = randInt(0, 359); // Get random direction
        polarToCartesian(VIEW_WIDTH/2, VIEW_HEIGHT/2, VIEW_RADIUS, randTheta, x, y); // Convert direction to cartesian coords along edge of petri dish
        addActor(new Fungus(x, y, this)); // Add fungus to World
    }
    // Add Goodies
    int chanceGoodie = max(510 - getLevel() * 10, 250);
    if (randInt(0, chanceGoodie-1) == 0)
    {
        // Add goodie
        int randTheta = randInt(0, 359); // Get random direction
        polarToCartesian(VIEW_WIDTH/2, VIEW_HEIGHT/2, VIEW_RADIUS, randTheta, x, y); // Convert direction to cartesian coords along edge of petri dish
        int goodieType = randInt(0, 9); // Generate random int 0-9 (each int has a .1 chance)
        if (goodieType < 6) // .6 chance of RestoreHealthGoodie (ints 0-5)
            addActor(new RestoreHealthGoodie(x, y, this));
        else if (goodieType < 9) // .3 chance of FlameThrowerGoodie (ints 6-8)
            addActor(new FlameThrowerGoodie(x, y, this));
        else // .1 chance of ExtraLifeGoodie (int 9)
            addActor(new ExtraLifeGoodie(x, y, this));
    }
}

// StudentWorld method to delete all dead actors in world
void StudentWorld::deleteDead()
{
    // Loop through whole Actor vector using iterator
    auto it = m_actors.begin();
    while (it != m_actors.end())
    {
        if ((*it)->isDead()) // Check if current actor is dead
        {
            delete (*it); // Deallocate actor
            m_nActors--; // Decrement actor counter
            it = m_actors.erase(it); // Erase actor from Vector
        }
        else // If not dead move on to next actor
            it++;
    }
}

// StudentWorld method to update game status text using an ostringstream
void StudentWorld::updateStatusText()
{
    ostringstream oss; // Declare oss
    oss.fill('0'); // Set fill to 0
    oss << "Score: " << setw(6) << getScore() << "  "; // Output score
    oss << "Level: " << getLevel() << "  "; // Output level
    oss << "Lives: " << getLives() << "  "; // Output lives
    oss << "Health: " << m_player->getHP() << "  "; // Output health
    oss << "Sprays: " << m_player->getSprays() << "  "; // Output sprays
    oss << "Flames: " << m_player->getFlames() << "  "; // Output flames
    setGameStatText(oss.str()); // Pass the full string to the GameWorld's stat displayer
}

// StudentWorld method to check if level is over
bool StudentWorld::isLevelOver()
{
    bool isOver = true; // Declare bool to record if level is over (start with true)
    // Loop through all actors in world
    auto it = m_actors.begin();
    while (it != m_actors.end())
    {
        if ((*it)->preventsLevelCompletion()) // Check if current actor prevents level completion
            isOver = false; // Set bool to false because level is not over
        it++;
    }
    return isOver; // Return if level is over (false if any actor prevents completion)
}

////////////
// Utility Function Implementations
////////////

// Returns distance between two actors
double getDistance(Actor* a, Actor* b)
{
    // Uses pythagorean theorem
    return abs(sqrt(pow((a->getX() - b->getX()), 2.0) +  pow((a->getY() - b->getY()), 2.0)));
}

// Returns distance between actor and xy coords
double getDistance(Actor* a, int x, int y)
{
    // Uses pythagorean theorem
    return abs(sqrt(pow((a->getX() - x), 2.0) +  pow((a->getY() - y), 2.0)));
}

// Returns distance from center to xy coords
double getDistanceFromCenter(int x, int y)
{
    // Uses pythagorean theorem
    return abs(sqrt(pow((x - VIEW_WIDTH/2), 2.0) +  pow((y - VIEW_HEIGHT/2), 2.0)));
}

// Converts input polar coords from origin oX,oY to cartesian coords x,y
void polarToCartesian(int oX, int oY, int r, int theta, int &x, int &y)
{
    // Convert theta degrees to radians
    double radTheta = (theta*1.0) * (M_PI/180);
    // Convert polar to cartesian
    x = (r*cos(radTheta)) + oX;
    y = (r*sin(radTheta)) + oY;
}

// Converts input cartesian vector from origin oX,oY, to end x,y to polar coords r (radius) and theta (angle)
void cartesianToPolar(int oX, int oY, int x, int y, int &r, int &theta)
{
    // Get delta x and delta y
    double deltaX = (x - oX);
    double deltaY = (y - oY);
    
    // Calculate/return radius using pythagorean theorem
    r = sqrt(pow(deltaX, 2) + pow(deltaY, 2));
    
    // Calculate/return theta angle using arctan
    int degTheta = static_cast<int>(atan2(deltaY, deltaX) * (180/M_PI)); // Calculate thete in deg
    theta = ((degTheta + 360) % 360); // Convert degrees to be between 0-359
}

// Returns if two actors overlap
bool overlaps(Actor* a, Actor* b)
{
    if (getDistance(a, b) <= 2*SPRITE_RADIUS) // Check if distance is overlap
        return true;
    return false;
}

// Returns if an actor and a coord xy overlap
bool overlaps(Actor* a, int x, int y)
{
    if (getDistance(a, x, y) <= 2*SPRITE_RADIUS) // Check if distance is overlap
        return true;
    return false;
}

