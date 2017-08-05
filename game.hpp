#ifndef GAME_HPP_
#define GAME_HPP_

#include "logical.hpp"

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

#endif
