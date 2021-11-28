#include "Actor.h"
#include "StudentWorld.h"

// Students:  Add code to this file, Actor.h, StudentWorld.h, and StudentWorld.cpp

using namespace std;

// Actor implementation

// Actor constructor passes neccessary info to GraphObject and initializes m_isDead to false (actors always start out alive) and m_world to point to the associated StudentWorld
Actor::Actor(int imgID, int x, int y, Direction dir, int depth, StudentWorld* worldPtr) : GraphObject(imgID, x, y, dir, depth), m_isDead(false), m_world(worldPtr) {}

// Actor -> HP Implementation

// HP constructor passes neccessary info to Actor with a depth of 0
HP::HP(int imgID, int hp, int x, int y, Direction dir, StudentWorld* worldPtr) : Actor(imgID, x, y, dir, 0, worldPtr)
{
    m_hp = hp; // Sets hp to input hp
}

// HP damage method damages actor for input damage
bool HP::damage(int hp)
{
    soundWhenHurt(); // Play hurt sound
    m_hp -= hp; // Decrease hp by input amt
    if (m_hp <= 0) // If damage is lethal
    {
        setDead(); // Set actor dead
        soundWhenDie(); // Play death sound
        actionAtDeath(); // Do special action at dead (if any)
    }
    return true; // return true because HP can be damaged (unlike some other classes)
}

// Actor -> HP -> Socrates Implementation

// Socrates constructo passes the spec's specified starting point and direction to HP along with correct image ID and world pointer
Socrates::Socrates(StudentWorld* worldPtr) : HP(IID_PLAYER, 100, 0, VIEW_HEIGHT/2, 0, worldPtr)
{
    m_sprayCharges = 20; // Set new Socrates spray count to default 20
    m_flameCharges = 5; // Set new Socrates flame count to default 5
    m_posAngle = 180; // Set positional angle to 180 (starting angle)
}

// Socrates does something every tick
void Socrates::doSomething()
{
    if (getHP() <= 0) // If no HP kill Socrates and return
    {
        setDead();
        return;
    }
    
    int ch;
    if (getWorld()->getKey(ch)) // Get keystroke
    {
        switch (ch) {
            // For keystrokes corresponding to a socrates movement (left, right, a, d)
            case 'a':
            case KEY_PRESS_LEFT:
            case 'd':
            case KEY_PRESS_RIGHT:
                rotate(ch); // Rotate socrates the correct way using rotate
                break;
            case KEY_PRESS_SPACE: // If key is space
                if (m_sprayCharges > 0) // Check if there are sprays left
                    fireSpray(); // Fire spray
                break;
            case KEY_PRESS_ENTER: // If key is enter
                if (m_flameCharges > 0) // Check if there are flames left
                    fireFlame(); // Fire flame
                break;
        }
    }
    else if (m_sprayCharges < 20) // If no keystroke
        m_sprayCharges++; // Replenish spray charges by 1
}

// Socrates method plays sound when it is hurt
void Socrates::soundWhenHurt() const
{
    getWorld()->playSound(SOUND_PLAYER_HURT);
}

// Socrates method plays sound when it dies
void Socrates::soundWhenDie() const
{
    getWorld()->playSound(SOUND_PLAYER_DIE);
}

// Socrates method for rotating around dish
void Socrates::rotate(int key)
{
    // Change angle according to keypress
    if (key == KEY_PRESS_LEFT || key == 'a')
        m_posAngle += 5;
    else if (key == KEY_PRESS_RIGHT || key == 'd')
        m_posAngle -= 5;
    else // Do nothing if invalid key
        return;
    
    int x, y; // Declare temp vars to store new position
    polarToCartesian(VIEW_WIDTH/2, VIEW_HEIGHT/2, VIEW_WIDTH/2, m_posAngle, x, y); // Convert new posAngle to cartesian along dish edge
    moveTo(x, y); // Move Socrates
    setDirection(m_posAngle+180); // Update direction facing
}

// Socrates method to fire a flame
void Socrates::fireFlame()
{
    int x, y; // Declare temp vars to store flame coords
    Direction theta = getDirection(); // Get direction Socrates is facing
    for (int i = 0; i < 16; i++) // Loop for 16 flames
    {
        polarToCartesian(getX(), getY(), 2*SPRITE_RADIUS, theta, x, y); // Get cartesian coords 2*SPRITE_RADIUS away from socrates in specified direction
        getWorld()->addActor(new Flame(x, y, theta, getWorld())); // Add flame
        theta += 22; // increment around socrates in a circle by 22 deg
    }
    m_flameCharges--; // Decrement flame charges
    getWorld()->playSound(SOUND_PLAYER_FIRE); // Play flame sound
}

void Socrates::fireSpray()
{
    int x, y; // Temp vars to store spray coords
    polarToCartesian(getX(), getY(), 2*SPRITE_RADIUS, getDirection(), x, y); // Get cartesian coords 2*SPRITE_RADIUS in front of Socrates
    getWorld()->addActor(new Spray(x, y, getDirection(), getWorld())); // Add spray to world
    m_sprayCharges--; // Decrement spray charges
    getWorld()->playSound(SOUND_PLAYER_SPRAY); // Play spray sound
}

// Actor -> HP -> Bacteria Implementation

// Bacteria Constructor passes neccessary info to HP and a specified direction of 90 as stated in spec
Bacteria::Bacteria(int imgID, int hp, int x, int y, StudentWorld* worldPtr) : HP(imgID, hp, x, y, 90, worldPtr)
{
    m_mvmtPlanDist = 0; // Set default movement plan to 0
    m_foodEaten = 0; // Set default food eaten to 0
}

// Bacteria method for doing something each tick
void Bacteria::doSomething()
{
    if (isDead() || getHP() <= 0) // Check if it is dead
    {
        setDead(); // Set to dead and return
        return;
    }
    
    bool attackedSocrates = attackSocrates(); // Attempt to track and attack Socrates (true only if this bacteria is aggressive salmonella)
    
    if (getWorld()->overlapsPlayer(this)) // Check if current bacteria overlaps with Socrates
        damageSocrates(); // Damage socrates (specific to each type of bacteria
    else if (m_foodEaten == 3) // Otherwise if it ate 3 food
    {
        // Get current coords
        int newX = getX();
        int newY = getY();
        
        // Calculate newX according to spec
        if (getX() < VIEW_WIDTH/2)
            newX = getX() + SPRITE_RADIUS;
        else if (getX() > VIEW_WIDTH/2)
            newX = getX() - SPRITE_RADIUS;
        
        // Calculate newY according to spec
        if (getY() < VIEW_HEIGHT/2)
            newY = getY() + SPRITE_RADIUS;
        else if (getY() > VIEW_HEIGHT/2)
            newY = getY() - SPRITE_RADIUS;
        
        divideBacteria(newX, newY); // Divide correct bacteria type using new coords
        getWorld()->playSound(SOUND_BACTERIUM_BORN); // Play birth sound
        
        m_foodEaten = 0; // Reset food counter
    }
    else if (getWorld()->eatNearbyFood(this)) // Otherwise if there is food to be eaten nearby
    {
        m_foodEaten++; // Increase food count
    }
    
    if (attackedSocrates) // If it is aggressive salmonella and it tracked socrates, return here and don't do next steps as stated in spec
        return;
    
    if (m_mvmtPlanDist > 0) // If there is a movement plan dist (never true for EColi)
    {
        m_mvmtPlanDist--; // Decrement movement plan
        plannedMove(); // Do planned move
    }
    else // Otherwise do backup move
        backupMove();
}

// Bacteria method for action at death
void Bacteria::actionAtDeath()
{
    getWorld()->increaseScore(100); // All bacteria increase score by 100
    if (randInt(0, 1) == 0) // .5 chance to add food at current coord to world
        getWorld()->addActor(new Food(getX(), getY(), getWorld()));
}

// Actor -> HP -> EColi Implementation

// EColi method to play ecoli hurt sound
void EColi::soundWhenHurt() const
{
    getWorld()->playSound(SOUND_ECOLI_HURT);
}

// EColi method to play ecoli death sound
void EColi::soundWhenDie() const
{
    getWorld()->playSound(SOUND_ECOLI_DIE);
}

// EColi method to divide bacteria and make new EColi at specified coords
void EColi::divideBacteria(int x, int y)
{
    getWorld()->addActor(new EColi(x, y, getWorld()));
}

// EColi only has a backup move, not mvmt plan move
void EColi::backupMove()
{
    int angleToSocrates;
    if (getWorld()->getAngleToSocrates(this, 256, angleToSocrates)) // Attempt to get an angle to socrates within 256 pixels away
    {
        // If getting angle succeeds
        for (int i = 0; i < 10; i++) // Try up to 10 times
        {
            if (getWorld()->validMove(this, angleToSocrates, 2)) // If moving 2 pixels towards socrates is valid
            {
                // Set direction, move, and return
                setDirection(angleToSocrates);
                double x, y; // Vars to store new position
                getPositionInThisDirection(angleToSocrates, 2, x, y); // Get cartesian coords for polar move
                moveTo(x, y); // Move to new coords
                return;
            }
            else // If move is invalid increment angle by 10 (wrapping around 360)
                angleToSocrates = (angleToSocrates + 10) % 360;
        }
    }
}

// EColid method to damage player for 4 hp
void EColi::damageSocrates()
{
    getWorld()->damagePlayer(4);
}

// Actor -> HP -> Salmonella Implementation

// Salmonella method to play its sound when hurt
void Salmonella::soundWhenHurt() const
{
    getWorld()->playSound(SOUND_SALMONELLA_HURT);
}

// Salmonella method to play its death sound
void Salmonella::soundWhenDie() const
{
    getWorld()->playSound(SOUND_SALMONELLA_DIE);
}

// Salmonella method for it mvmt plan distance move
void Salmonella::plannedMove()
{
    if (getWorld()->validMove(this, getDirection(), 3)) // If it can validly go foward 3 pixels
    {
        double x, y; // Declare temp vars to store new coords
        getPositionInThisDirection(getDirection(), 3, x, y); // Get cartesian coords for polar move
        moveTo(x, y); // Use moveTo to move forward 3 pixels
    }
    else // Otherwise set random direction and reset plan to 10
    {
        setDirection(randInt(0, 359));
        setPlanDist(10);
    }
}

// Salmonella method for move if it has no mvmt plan
void Salmonella::backupMove()
{
    int angleToNearestFood;
    if (getWorld()->getAngleToNearestFood(this, 128, angleToNearestFood)) // Attempt getting angle to food within 128 distance
    {
        setDirection(angleToNearestFood);
        
        if (!getWorld()->validMove(this, getDirection(), 3)) // Check if moving toward food 3 pixels is INVALID
        {
            // Set new random direction and reset plan distance and return
            setDirection(randInt(0, 359));
            setPlanDist(10);
            return;
        }
        else // Otherwise move 3 pixels toward food
        {
            double x, y; // Vars to store new position
            getPositionInThisDirection(getDirection(), 3, x, y); // Get cartesian coords for polar move
            moveTo(x, y); // Move to new coords
        }
    }
    else // If no food found
    {
        // Set new rand direction, reset plan dist, return
        setDirection(randInt(0, 359));
        setPlanDist(10);
        return;
    }
}

// Actor -> HP -> Salmonella -> RegSalmonella Implementation

// RegSalmonella method to divide into 2 RegSalmonellas
void RegSalmonella::divideBacteria(int x, int y)
{
    getWorld()->addActor(new RegSalmonella(x, y, getWorld()));
}

// RegSalmonella method to damage socrates by 1 hp
void RegSalmonella::damageSocrates()
{
    getWorld()->damagePlayer(1);
}

// Actor -> HP -> Salmonella -> AggSalmonella Implementation

// AggSalmonella method to divide into 2 AggSalmonellas
void AggSalmonella::divideBacteria(int x, int y)
{
    getWorld()->addActor(new AggSalmonella(x, y, getWorld()));
}

// Special function for aggressive salmonella to track and attack socrates at the beginning of each tick
bool AggSalmonella::attackSocrates()
{
    int angleToSocrates;
    if (getWorld()->getAngleToSocrates(this, 72, angleToSocrates)) // Attempt to get angle to socrates within 72 pixels away
    {
        // If angle can be gotten
        if (getWorld()->validMove(this, angleToSocrates, 3)) // Check if 3 pixels toward socrates is a valid move
        {
            // If valid, setDirection and move forward
            setDirection(angleToSocrates);
            double x, y; // Vars to store new position
            getPositionInThisDirection(getDirection(), 3, x, y); // Get cartesian coords for polar move
            moveTo(x, y); // Move to new coords
        }
        return true; // Return true that socrates is within 72 pixels away
    }
    return false; // Return false if socrates no found
}

// AggSalmonella method to damage socrates for 2 hp
void AggSalmonella::damageSocrates()
{
    getWorld()->damagePlayer(2);
}

// Actor -> Dirt implementation

// Dirt method to recieve damage
bool Dirt::damage(int)
{
    // Dirt instantly dies no matter how much damage is caused
    setDead();
    return true; // Return true for can be damaged
}

// Actor -> Food implementation
// Food has no out-of-line implementation (just a placeholder)

// Actor -> (Bacteria) Pit implementation

// Pit method to do something each tick
void Pit::doSomething()
{
    if (m_regSalm == 0 && m_aggSalm == 0 && m_EColi == 0) // Check if it ran out of bacteria, and setDead if true
    {
        setDead();
        return;
    }
    else if (randInt(0, 49) == 0) // otherwise 1/50 chance to emit a bacteria using emitBacteria
    {
        emitBacteria();
    }
}

// Pit method to emit a bacteria
void Pit::emitBacteria()
{
    int type; // Declare integer to get random type
    bool invalidType = true; // Bool to track if the type is valid (starts invalid)
    do
    {
        type = randInt(0, 2); // Random type 0->2 for each type of bacteria (0=regSal, 1=aggSal, 2=EColi)
        if (type == 0 && m_regSalm > 0) // If regSalm is chosen and there are more left
            invalidType = false; // Set invalidType to false and exit loop
        else if (type == 1 && m_aggSalm > 0) // If aggSalm is chosen and there are more left
            invalidType = false; // Set invalidType to false and exit loop
        else if (type == 2 && m_EColi > 0) // If EColi is chosen and there are more left
            invalidType = false; // Set invalidType to false and exit loop
    } while (invalidType); // Loop while invalidType is still true
        
    if (type == 0) // If type chosen is regSalm (0) then add it to the world and decrement regSalm count
    {
        getWorld()->addActor(new RegSalmonella(getX(), getY(), getWorld()));
        m_regSalm--;
    }
    else if (type == 1) // If type chosen is aggSalm (1) then add it to the world and decrement aggSalm count
    {
        getWorld()->addActor(new AggSalmonella(getX(), getY(), getWorld()));
        m_aggSalm--;
    }
    else if (type == 2) // If type chosen is EColi (2) then add it to the world and decrement EColi count
    {
        getWorld()->addActor(new EColi(getX(), getY(), getWorld()));
        m_EColi--;
    }
    
    getWorld()->playSound(SOUND_BACTERIUM_BORN); // Play bacteria birth sound
}

// Actor -> Item implementation

// Item constructor that passes necessary info to Actor with direction 0 and depth 1
Item::Item(int imgID, int x, int y, StudentWorld* worldPtr) : Actor(imgID, x, y, 0, 1, worldPtr)
{
    m_lifetime = max(rand()%(300-10*(getWorld()->getLevel())), 50); // Set lifetime to spec's random function
}

// Item method to do something every tick
void Item::doSomething()
{
    if (isDead()) // Check if it is dead, return if it is
        return;
    else if (getWorld()->overlapsPlayer(this)) // Check if it overlaps the player
    {
        // If it overlaps, set itself to dead, play its pickup sound (no sound for fungus), and apply its effect
        setDead();
        playSound();
        applyEffect();
        return;
    }
    // Otherwise
    m_lifetime--; // Decrement lifetime
    if (m_lifetime <= 0) // Check if lifetime is over and set it to dead if it is
        setDead();
    return;
}

// Item method for item to take damage
bool Item::damage(int)
{
    // All damage is instant kill
    setDead();
    return true;
}

// Item method to play sound for getting goodie
void Item::playSound() const
{
    getWorld()->playSound(SOUND_GOT_GOODIE);
}

// Actor -> Item -> Restore Health Goodie Implementations

// RestoreHealthGoodie for applying its special effect
void RestoreHealthGoodie::applyEffect()
{
    getWorld()->increaseScore(250);
    getWorld()->restorePlayerHP(100);
    return;
}

// Actor -> Item -> Flame Thrower Goodie Implementations

// FlameThrowerGoodie for applying its special effect
void FlameThrowerGoodie::applyEffect()
{
    getWorld()->increaseScore(300);
    getWorld()->addPlayerFlames(5);
    return;
}

// Actor -> Item -> Extra Life Goodie Implementations

// ExtraLifeGoodie for applying its special effect
void ExtraLifeGoodie::applyEffect()
{
    getWorld()->increaseScore(500);
    getWorld()->incLives();
    return;
}

// Actor -> Item -> Fungus Implementations

// Fungus for applying its special effect
void Fungus::applyEffect()
{
    getWorld()->increaseScore(-50);
    getWorld()->damagePlayer(20);
    return;
}

// Actor -> Projectile Implementation

// Projectile Constructor passes input info to Actor
Projectile::Projectile(int imgID, int x, int y, Direction dir, int maxDist, StudentWorld* worldPtr) : Actor(imgID, x, y, dir, 1, worldPtr)
{
    m_maxDist = maxDist; // Set max distance tracker to input max distance
}

// Projectile method to do something each tick
void Projectile::doSomething()
{
    if (isDead()) // Check if it is dead and return if true
        return;
    if (attemptDamage()) // Attempt damage specific to projectile type
    {
        // If damage succeeds, set dead and return
        setDead();
        return;
    }
    else // Otherwise move forward 2*SPRITE_RADIUS and decrement max distance counter
    {
        double x, y; // Vars to store new position
        getPositionInThisDirection(getDirection(), 2*SPRITE_RADIUS, x, y); // Get cartesian coords for polar move
        moveTo(x, y); // Move to new coords
        m_maxDist -= 2*SPRITE_RADIUS;
    }
    if (m_maxDist <= 0) // If projectile runs out of distance, set it to dead
        setDead();
}

// Actor -> Projectile -> Flame Implementation

// Flame method to attempt to damage nearby actors for 5 hp
bool Flame::attemptDamage()
{
    return getWorld()->damageNearbyActor(this, 5);
}

// Actor -> Projectile -> Spray Implementation

// Spray method to attempt to damage nearby actors for 2 hp
bool Spray::attemptDamage()
{
    return getWorld()->damageNearbyActor(this, 2);
}
