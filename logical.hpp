#ifndef LOGICAL_HPP_
#define LOGICAL_HPP_

// essential headers

#include <SDL2/SDL.h>

#include "std_types.hpp"
#include "time.hpp"
#include "random.hpp"

// constants

#define BALL_TYPE_NONE    -1
#define BALL_TYPE_RED     0
#define BALL_TYPE_GREEN   1
#define BALL_TYPE_BLUE    2
#define BALL_TYPE_CYAN    3
#define BALL_TYPE_MAGENTA 4
#define BALL_TYPE_YELLOW  5
#define BALL_TYPE_WHITE   6

#define	NUM_BALL_TYPES    7

#define ROTOR_POSITION_RIGHT  0
#define ROTOR_POSITION_TOP    1
#define ROTOR_POSITION_LEFT   2
#define ROTOR_POSITION_BOTTOM 3

#define CONNECTOR_WALL         0
#define CONNECTOR_LINE         1
#define CONNECTOR_ROTOR        2
#define CONNECTOR_SPAWN        3
#define CONNECTOR_INSERTER     4
#define CONNECTOR_FREE         5

#define ROTOR_CLOCKWISE     0
#define ROTOR_ANTICLOCKWISE 1

#define NBALLS 500
#define NROTORS 50
#define NLINES 200
#define NINSERTERS 50
#define NSPAWNS 4

// forward-declare types

struct GameData;
struct Ball;
struct Line;
struct Connector;
struct InsertPoint;
struct SpawnPoint;

// types

struct Connector {
	int type;
	int target;
	union {
		struct {
			int position;
		} rotor;
	};
};

struct Ball {
	// position (center of the ball, in pixels)
	float x;
	float y;
	// velocity (in pixels / time)
	float vx;
	float vy;

	// counts how many times the ball was released from a rotor by being clicked
	int released_counter;
	// from which spawn did this ball get released initially
	int spawn_index;
	int type;
	Time created;
	Connector connector;
};

struct Rotor {
	float x;
	float y;
	Connector connectors[4];
	int balls[4];
	bool destroyed;
};

struct Line {
	float x1;
	float y1;
	float x2;
	float y2;
	Connector connector;
};

struct Inserter {
	Connector connector_success;
	Connector connector_failure;
};

struct Spawn {
	Connector connector;
};

struct GameData {
	Ball balls[NBALLS];
	Rotor rotors[NROTORS];
	Line lines[NLINES];
	Inserter inserters[NINSERTERS];
	Spawn spawns[NSPAWNS];

	int ball_count;
	int rotor_count;
	int line_count;
	int inserter_count;
	int spawn_count;

	int ball_type_count;
	int ball_types[NUM_BALL_TYPES];

	int ball_type_index_next;

	Time time;

	Random random;

	SDL_Renderer *renderer;
};

// globals

extern bool should_quit;
extern const int ROTOR_POSITIONS[];
extern const Uint8 BALL_COLORS[][4];

// functions

void buildMap1(GameData *);
void buildMap2(GameData *);
void buildMap3(GameData *);
void buildMap4(GameData *);

void turnRotor(GameData *, int, int);
void releaseBallFromRotor(GameData *, int, int);

void resetGame(GameData *);
void handleAllEvents(GameData *);
void handleEvent(GameData *, const SDL_Event *);
void progressLogic(GameData *, Time);
void updateBallPosition(GameData *, int);
void renderEverything(GameData *, SDL_Renderer *);

void clearGame(GameData *);

int addBallType(GameData *, int);
int addBall(GameData *, int);
void removeBall(GameData *, int);
int addLine(GameData *);
int addRotor(GameData *);
int addInserter(GameData *);
int addSpawn(GameData *);

int placeBallFree(GameData *gd, int ball_type, float x, float y, float vx, float vy);
int placeBallInRotor(GameData *, int rotor_index, int rotor_position, int ball_type);
int placeBallInSpawn(GameData *gd, int ball_type, int spawn_index);
int placeRandomBallInSpawn(GameData *gd, int spawn_index);
	
void changeBallConnector(GameData *, int ball_index, const Connector *connector);
void copyConnector(const Connector *src, Connector *dst);

#endif // LOGICAL_HPP_
