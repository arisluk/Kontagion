#ifndef ACTOR_H_
#define ACTOR_H_

#include "GraphObject.h"

class StudentWorld;

// Students:  Add code to this file, Actor.cpp, StudentWorld.h, and StudentWorld.cpp

// Base Class for all game objects

class Actor: public GraphObject
{
public:
    // Constructor
    Actor(int imgID, int x, int y, Direction dir, int depth, StudentWorld* worldPtr);
    // Member functions
    bool isDead() const {return m_isDead;}
    void setDead() {m_isDead = true;}
    // Pure Virtual functions
    virtual void doSomething() = 0; // Function for each actor to implement action during each tick
    // Virtual functions
    virtual bool damage(int) {return false;} // Function to recieve damage (if possible) to this actor (also returns whether actor can be damaged or not)
    virtual bool isEdible() const {return false;} // Returns whether bacteria can eat this actor
    virtual bool blocksBacteria() const {return false;} // Returns whether it blocks bacteria mvmt (defaults to false)
    virtual bool preventsLevelCompletion() const {return false;} // Returns whether this actor prevents level completion (defaults to false)
    // Destructor
    virtual ~Actor() {}
    
protected:
    // Member functions
    StudentWorld* getWorld() const {return m_world;}
    
private:
    // Data members
    bool m_isDead; // Data member to track live/dead state
    StudentWorld* m_world; // Ptr to associated world
};

// Actor -> HP Class
// HP class represents Actors that have HP (Socrates/Bacteria)

class HP: public Actor
{
public:
    // Constructor
    HP(int imgID, int hp, int x, int y, Direction dir, StudentWorld* worldPtr);
    // Member functions
    void restoreHP(int hp) {m_hp = hp;} // Set object hp to input hp
    int getHP() const {return m_hp;} // Returns current hp
    // Virtual functions
    virtual bool damage(int hp); // Redefine damage so HP actor can be damaged
    // Destructor
    virtual ~HP() {}
    
protected:
    // Virtual functions
    virtual void actionAtDeath() {} // Virtual function so HP actors can do something at death, defined here so can be called when any HP takes lethal damage
    // Pure Virtual functions
    virtual void soundWhenHurt() const = 0; // Play hurt sound
    virtual void soundWhenDie() const = 0; // Play death sound
    
private:
    // Data members
    int m_hp; // Track HP
};

// Actor -> HP -> Socrates
// Socrates class represents the player

class Socrates: public HP
{
public:
    // Constructor
    Socrates(StudentWorld* worldPtr);
    // Member functions
    void addFlames(int flames) {m_flameCharges += flames;} // Add input flames to player flame count
    int getSprays() const {return m_sprayCharges;} // Return number of sprays
    int getFlames() const {return m_flameCharges;} // Return number of flames
    // Virtual functions (from base class)
    virtual void doSomething();
    // Destructor
    virtual ~Socrates() {}
    
protected:
    // Virtual functions
    virtual void soundWhenHurt() const;
    virtual void soundWhenDie() const;
    
private:
    // Data Members
    int m_sprayCharges, m_flameCharges; // Track spray/flame counts
    Direction m_posAngle; // Track position angle (Not facing direction)
    // Member Functions
    void rotate(int key); // Rotate Socrates around dish
    void fireFlame(); // Fire a flame charge
    void fireSpray(); // Fire a spray charge
};

// Actor -> HP -> Bacteria ABSTRACT Class
// Bacteria class represents all bacteria (salmonella, ecoli, etc.)

class Bacteria: public HP
{
public:
    // Constructor
    Bacteria(int imgID, int hp, int x, int y, StudentWorld* worldPtr);
    // Virtual Functions
    virtual void doSomething();
    virtual bool preventsLevelCompletion() const {return true;} // Redefine function because live bacteria prevents level completion
    // Destructor
    virtual ~Bacteria() {}
    
protected:
    // Member Functions
    void setPlanDist(int dist) {m_mvmtPlanDist = dist;}
    // Pure Virtual Functions
    virtual void divideBacteria(int x, int y) = 0; // Function to divide correct bacteria type
    virtual void plannedMove() = 0; // Planned move function specific to different bacteria
    virtual void backupMove() = 0; // Backup move function specific to different bacteria
    virtual void damageSocrates() = 0; // Do damage to Socrates
    // Virtual Functions
    virtual void actionAtDeath(); // Does something when bacteria dies
    virtual bool attackSocrates() {return false;} // Virtual function that only applies to aggSalmonella, but is here so it can be called in Bacteria's doSomething (does nothing for other types of bacteria)
    
private:
    // Data members
    int m_mvmtPlanDist; // Track mvmt plan distance
    int m_foodEaten; // Track food eaten
};

// Actor -> HP -> EColi Class
// EColi class represents the bacteria enemy EColi

class EColi: public Bacteria
{
public:
    // Constructor passes correct img ID and hp and input info to Bacteria
    EColi(int x, int y, StudentWorld* worldPtr) : Bacteria(IID_ECOLI, 5, x, y, worldPtr) {}
    // Destructor
    virtual ~EColi() {}
    
protected:
    // Virtual Functions
    virtual void soundWhenHurt() const;
    virtual void soundWhenDie() const;
    virtual void divideBacteria(int x, int y); // Divide into 2 EColi
    virtual void plannedMove() {} // EColi has not planned move (for mvmtDist > 10)
    virtual void backupMove(); // backupMove specific to EColi
    virtual void damageSocrates(); // Damage Socrates correctly for EColi
};

// Actor -> HP -> Salmonella ABSTRACT Class
// Salmonella represents all potential salmonella type bacteria (reg, aggro)
// It is abstract because it doesn't implement the pure virtuals damageSocrates and divideBacteria

class Salmonella: public Bacteria
{
public:
    // Constructor passes correct img ID, and input info to Bacteria
    Salmonella(int hp, int x, int y, StudentWorld* worldPtr) : Bacteria(IID_SALMONELLA, hp, x, y, worldPtr) {}
    // Destructor
    virtual ~Salmonella() {}
    
protected:
    // Virtual Functions
    virtual void soundWhenHurt() const;
    virtual void soundWhenDie() const;
    virtual void plannedMove(); // planned move specific to all Salmonella
    virtual void backupMove(); // backup move specific to all Salmonella
};

// Actor -> HP -> Salmonella -> RegSalmonella Class
// RegSalmonella represents Regular Salmonella enemy bacteria

class RegSalmonella: public Salmonella
{
public:
    // Constructor passes input info and specific hp 4 to Salmonella
    RegSalmonella(int x, int y, StudentWorld* worldPtr) : Salmonella(4, x, y, worldPtr) {}
    // Destructor
    virtual ~RegSalmonella() {}
    
protected:
    // Virtual Functions
    virtual void divideBacteria(int x, int y); // Divide into 2 RegSalmonella
    virtual void damageSocrates(); // Damage socrates correctly for regular salmonella
};

// Actor -> HP -> Salmonella -> AggSalmonella Class
// RegSalmonella represents Aggressive Salmonella enemy bacteria

class AggSalmonella: public Salmonella
{
public:
    // Constructor passes input info and specific hp 10 to Salmonella
    AggSalmonella(int x, int y, StudentWorld* worldPtr) : Salmonella(10, x, y, worldPtr) {}
    // Destructor
    virtual ~AggSalmonella() {}
    
protected:
    // Virtual Functions
    virtual void divideBacteria(int x, int y); // Divide into 2 AggSalmonella
    virtual bool attackSocrates(); // Redefine special AggSalmonella movement that tracks Socrates
    virtual void damageSocrates(); // Damage socrates correctly for aggressive salmonella
};

// Actor -> Dirt Class
// Dirt class represents Dirt actor that blocks bacteria

class Dirt: public Actor
{
public:
    // Constructor correct img ID, direction 0, and depth 1 to Actor
    Dirt(int x, int y, StudentWorld* worldPtr) : Actor(IID_DIRT, x, y, 0, 1, worldPtr) {}
    // Virtual functions
    virtual void doSomething() {} // Do nothing
    virtual bool damage(int); // Redefine damage since dirt can be damaged
    virtual bool blocksBacteria() const {return true;} // Redefine since dirt block bacteria mvmt
    // Destructor
    virtual ~Dirt() {}
};

// Actor -> Food Class
// Food class represents Food actor that can be eaten by bacteria

class Food: public Actor
{
public:
    // Constructor passes correct img ID, direction 90, and depth 1 to Actor
    Food(int x, int y, StudentWorld* worldPtr) : Actor(IID_FOOD, x, y, 90, 1, worldPtr) {}
    // Virtual functions
    virtual void doSomething() {} // Do nothing
    virtual bool isEdible() const {return true;} // Redefine since Food is edible
    // Destructor
    virtual ~Food() {}
};

// Actor -> (Bacteria) Pit Class
// Pit class represents Pit Actor that emits bacteria

class Pit: public Actor
{
public:
    // Constructor passes to Actor correct img ID, direction 0, depth 1, and initializes the bacteria storage counters to specifications
    Pit(int x, int y, StudentWorld* worldPtr) : Actor(IID_PIT, x, y, 0, 1, worldPtr), m_regSalm(5), m_aggSalm(3), m_EColi(2) {}
    // Virtual functions
    virtual void doSomething();
    virtual bool preventsLevelCompletion() const {return true;}
    // Destructor
    virtual ~Pit() {}
    
private:
    // Data members
    int m_regSalm, m_aggSalm, m_EColi; // Track how many stored bacteria pit has
    // Private Member functions
    void emitBacteria(); // Method to emit a random stored bacteria
};

// Actor -> Item ABSTRACT Class
// Item class represents all Goodies and Fungi that can be picked up by player

class Item: public Actor
{
public:
    // Constructor
    Item(int imgID, int x, int y, StudentWorld* worldPtr);
    // Virtual functions
    virtual void doSomething();
    virtual bool damage(int); // Goodies can be damaged
    // Destructor
    virtual ~Item() {}
    
protected:
    // Pure Virtual functions
    virtual void applyEffect() = 0; // Pure virtual function to be redefined for each goodie's effect
    virtual void playSound() const; // Play potential goodie pickup sound (default plays)
    
private:
    // Data members
    int m_lifetime; // Track goodie lifetime
};

// Actor -> Item -> Restore Health Goodie Class

class RestoreHealthGoodie: public Item
{
public:
    // Constructor passes correct img ID and input info to Item
    RestoreHealthGoodie(int x, int y, StudentWorld* worldPtr) : Item(IID_RESTORE_HEALTH_GOODIE, x, y, worldPtr) {}
    // Destructor
    virtual ~RestoreHealthGoodie() {}
    
protected:
    // Virtual functions
    virtual void applyEffect(); // Restore Health effect
};

// Actor -> Item -> Flame Thrower Goodie Class

class FlameThrowerGoodie: public Item
{
public:
    // Constructor passes correct img ID and input info to Item
    FlameThrowerGoodie(int x, int y, StudentWorld* worldPtr) : Item(IID_FLAME_THROWER_GOODIE, x, y, worldPtr) {}
    // Destructor
    virtual ~FlameThrowerGoodie() {}
    
protected:
    // Virtual functions
    virtual void applyEffect(); // Add Flame effect
};

// Actor -> Item -> Extra Life Goodie Class

class ExtraLifeGoodie: public Item
{
public:
    // Constructor passes correct img ID and input info to Item
    ExtraLifeGoodie(int x, int y, StudentWorld* worldPtr) : Item(IID_EXTRA_LIFE_GOODIE, x, y, worldPtr) {}
    // Destructor
    virtual ~ExtraLifeGoodie() {}
    
protected:
    // Virtual functions
    virtual void applyEffect(); // Add life effect
};

// Actor -> Item -> Fungus Class

class Fungus: public Item
{
public:
    // Constructor passes correct img ID and input info to Item
    Fungus(int x, int y, StudentWorld* worldPtr) : Item(IID_FUNGUS, x, y, worldPtr) {}
    // Destructor
    virtual ~Fungus() {}
    
protected:
    // Virtual functions
    virtual void applyEffect(); // Damage Socrates effect
    virtual void playSound() const {} // Redefine since Fungus doesn't play goodie sound
};

// Actor -> Projectile ABSTRACT Class
// Projectile representa all possible projectiles that can be shot by player and can damage things

class Projectile: public Actor
{
public:
    // Constructor
    Projectile(int imgID, int x, int y, Direction dir, int maxDist, StudentWorld* worldPtr);
    // Virtual Functions
    virtual void doSomething();
    // Destructor
    virtual ~Projectile() {}
    
protected:
    // Pure Virtual Functions
    virtual bool attemptDamage() = 0; // Pure virtual to attempt specific amount of damage to nearby actors
    
private:
    // Data members
    int m_maxDist;
};

// Actor -> Projectile -> Flame Class

class Flame: public Projectile
{
public:
    // Constructor passes correct img ID and maxDist 32 to Projectile
    Flame(int x, int y, Direction dir, StudentWorld* worldPtr) : Projectile(IID_FLAME, x, y, dir, 32, worldPtr) {}
    // Destructor
    virtual ~Flame() {}
    
protected:
    // Virtual Functions
    virtual bool attemptDamage(); // Redefine for flame's damage amount
};

// Actor -> Projectile -> Spray Class
class Spray: public Projectile
{
public:
    // Constructor passes correct img ID and maxDist 112 to Projectile
    Spray(int x, int y, Direction dir, StudentWorld* worldPtr) : Projectile(IID_SPRAY, x, y, dir, 112, worldPtr) {}
    // Destructor
    virtual ~Spray() {}
    
protected:
    // Virtual Functions
    virtual bool attemptDamage(); // Redefine for spray's damage amount
};

#endif // ACTOR_H_
