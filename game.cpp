#include "logical.hpp"

void clearGame(GameData *gd) {
	for (int i = 0; i < NBALLS; ++i) {
		gd->balls[i].type = BALL_TYPE_NONE;
	}

	gd->ball_type_count = 0;
	gd->line_count = 0;
	gd->rotor_count = 0;
	gd->inserter_count = 0;
	gd->spawn_count = 0;

	gd->ball_count = 0;

	gd->ball_type_index_next = 0;

	gd->time = 0;
}

int addBallType(GameData *gd, int type) {
	if (gd->ball_type_count < NUM_BALL_TYPES) {
		int ball_type_index= gd->ball_type_count++;
		gd->ball_types[ball_type_index] = type;
		return ball_type_index;
	} else {
		return -1;
	}
}

int addBall(GameData *gd, int type) {
	// find free index
	int ball_index = 0;
	while (ball_index < NBALLS && gd->balls[ball_index].type != BALL_TYPE_NONE) {
		++ball_index;
	}

	if (ball_index < NBALLS) {
		gd->balls[ball_index].type = type;
		gd->balls[ball_index].created = gd->time;
		gd->balls[ball_index].released_counter = 0;
		gd->balls[ball_index].spawn_index = -1;
		SDL_Log("Allocated ball %d (type %d)", ball_index, type);
		return ball_index;
	} else {
		SDL_Log("Allocating ball failed, already full");
		return -1;
	}
}

void removeBall(GameData *gd, int ball_index) {
	if (gd->balls[ball_index].type != BALL_TYPE_NONE) {
		gd->balls[ball_index].type = BALL_TYPE_NONE;
		SDL_Log("Released ball %d", ball_index);
	}
}

int addLine(GameData *gd) {
	if (gd->line_count < NLINES) {
		int line_index = gd->line_count++;
		return line_index;
	} else {
		return -1;
	}
}

int addRotor(GameData *gd) {
	if (gd->rotor_count < NROTORS) {
		int rotor_index = gd->rotor_count++;
		return rotor_index;
	} else {
		return -1;
	}
}

int addSpawn(GameData *gd) {
	if (gd->spawn_count < NSPAWNS) {
		int spawn_index = gd->spawn_count++;
		return spawn_index;
	} else {
		return -1;
	}
}

int addInserter(GameData *gd) {
	if (gd->inserter_count < NINSERTERS) {
		int inserter_index = gd->inserter_count++;
		return inserter_index;
	} else {
		return -1;
	}
}


int placeBallFree(GameData *gd, int ball_type, float x, float y, float vx, float vy) {
	int ball_index = addBall(gd, ball_type);
	if (ball_index >= 0) {
		gd->balls[ball_index].x = x;
		gd->balls[ball_index].y = y;
		gd->balls[ball_index].vx = vx;
		gd->balls[ball_index].vy = vy;
		gd->balls[ball_index].connector.type = CONNECTOR_FREE;
	}
	return ball_index;
}

int placeBallInRotor(GameData *gd, int ball_type, int rotor_index, int rotor_position) {
	// rotor position needs to be empty
	SDL_assert(gd->rotors[rotor_index].balls[rotor_position] == -1);
	int ball_index = addBall(gd, ball_type);
	gd->balls[ball_index].connector.type = CONNECTOR_ROTOR;
	gd->balls[ball_index].connector.target = rotor_index;
	gd->balls[ball_index].connector.rotor.position = rotor_position;
	return ball_index;
}

int placeBallInSpawn(GameData *gd, int ball_type, int spawn_index) {
	int ball_index = addBall(gd, ball_type);
	gd->balls[ball_index].connector.type = CONNECTOR_SPAWN;
	gd->balls[ball_index].connector.target = spawn_index;
	return ball_index;
}

int placeRandomBallInSpawn(GameData *gd, int spawn_index) {
	int i = random_get(&gd->random);
	int type = gd->ball_types[i % gd->ball_type_count];
	return placeBallInSpawn(gd, type, spawn_index);
}


void changeBallConnector(GameData *gd, int ball_index, const Connector *connector) {
	SDL_assert(connector != NULL);
	copyConnector(connector, &gd->balls[ball_index].connector);
	if (connector->type == CONNECTOR_LINE) {
		int line_index = connector->target;
		gd->balls[ball_index].x = gd->lines[line_index].x1;
		gd->balls[ball_index].y = gd->lines[line_index].y1;
		SDL_Log("ball %d is now on line %d", ball_index, line_index);
	} else if (connector->type == CONNECTOR_ROTOR) {
		int rotor_index = gd->balls[ball_index].connector.target;
		int position = gd->balls[ball_index].connector.rotor.position;

		float dx = 0;
		float dy = 0;
		if (position == ROTOR_POSITION_RIGHT) {
			dx = +15.0;
		} else if (position == ROTOR_POSITION_TOP) {
			dy = -15.0;
		} else if (position == ROTOR_POSITION_LEFT) {
			dx = -15.0;
		} else if (position == ROTOR_POSITION_BOTTOM) {
			dy = +15.0;
		}

		gd->balls[ball_index].x = gd->rotors[rotor_index].x + dx;
		gd->balls[ball_index].y = gd->rotors[rotor_index].y + dy;

		gd->rotors[rotor_index].balls[position] = ball_index;

		SDL_Log("ball %d is now on rotor %d(%d)", ball_index, rotor_index, position);

		if (gd->balls[ball_index].released_counter == 0) {
			placeRandomBallInSpawn(gd, gd->balls[ball_index].spawn_index);
		}

		if (gd->rotors[rotor_index].balls[0] != -1) {
			int ball_index = gd->rotors[rotor_index].balls[0];
			int ball_type = gd->balls[ball_index].type;
			bool all_identical = true;
			for (int i = 1; i < 4; ++i) {
				int ball_index_other = gd->rotors[rotor_index].balls[i];
				int ball_type_other = gd->balls[ball_index_other].type;
				if (ball_type != ball_type_other) {
					all_identical = false;
					break;
				}
			}
			if (all_identical) {
				for (int i = 0; i < 4; ++i) {
					int ball_index = gd->rotors[rotor_index].balls[i];
					removeBall(gd, ball_index);
					gd->rotors[rotor_index].balls[i] = -1;
				}
				gd->rotors[rotor_index].destroyed = true;
				SDL_Log("rotor %d destroyed", rotor_index);
			}
		}
	}
}

void copyConnector(const Connector *src, Connector *dst) {
	SDL_assert(src != NULL);
	SDL_assert(dst != NULL);
	dst->type = src->type;
	dst->target = src->target;
	if (src->type == CONNECTOR_ROTOR) {
		dst->rotor.position = src->rotor.position;
	}
}
