#include "logical.hpp"

int placeRotor(GameData *gd, float x, float y) {
	SDL_assert(gd->rotor_count <= NROTORS - 1);
	int rotor_index = addRotor(gd);
	gd->rotors[rotor_index].x = x;
	gd->rotors[rotor_index].y = y;
	for (int pos = 0; pos < 4; ++pos) {
		gd->rotors[rotor_index].connectors[pos].type = CONNECTOR_WALL;
		gd->rotors[rotor_index].balls[pos] = -1;
	}
	gd->rotors[rotor_index].destroyed = false;
	return rotor_index;
}

void placeLine(GameData *gd, const Connector *c1, const Connector *c2) {
	SDL_assert(gd->line_count <= NLINES - 2);
	// figure out the coordinates of the two connectors
	float x[2], y[2];
	const Connector *connectors[2] = { c1, c2 };
	for (int i = 0; i < 2; ++i) {
		if (connectors[i]->type == CONNECTOR_ROTOR) {
			int rotor_index = connectors[i]->target;
			x[i] = gd->rotors[rotor_index].x;
			y[i] = gd->rotors[rotor_index].y;
			if (connectors[i]->rotor.position == ROTOR_POSITION_RIGHT) {
				x[i] += 15.0;
			} else if (connectors[i]->rotor.position == ROTOR_POSITION_TOP) {
				y[i] -= 15.0;
			} else if (connectors[i]->rotor.position == ROTOR_POSITION_LEFT) {
				x[i] -= 15.0;
			} else if (connectors[i]->rotor.position == ROTOR_POSITION_BOTTOM) {
				y[i] += 15.0;
			}
		} else {
			SDL_assert(false);
		}
	}
	// place the lines
	for (int from_i = 0; from_i < 2; ++from_i) {
		int to_i = 1 - from_i;
		int line_index = gd->line_count++;
		gd->lines[line_index].x1 = x[from_i];
		gd->lines[line_index].x2 = x[to_i];
		gd->lines[line_index].y1 = y[from_i];
		gd->lines[line_index].y2 = y[to_i];
		gd->lines[line_index].connector.type = connectors[to_i]->type;
		gd->lines[line_index].connector.target = connectors[to_i]->target;
		if (connectors[to_i]->type == CONNECTOR_ROTOR) {
			int rotor_index = connectors[to_i]->target;
			gd->lines[line_index].connector.rotor.position = connectors[to_i]->rotor.position;
		}
		if (connectors[from_i]->type == CONNECTOR_ROTOR) {
			int rotor_index = connectors[from_i]->target;
			int rotor_position = connectors[from_i]->rotor.position;
			gd->rotors[rotor_index].connectors[rotor_position].type = CONNECTOR_LINE;
			gd->rotors[rotor_index].connectors[rotor_position].target = line_index;
		}
	}
}

void placeLineBetweenRotors(GameData *gd, int rotor_index_1, int rotor_index_2) {
	int rotor_indices[2] = { rotor_index_1, rotor_index_2 };
	// get rotor coordinates
	float x[2], y[2];
	for (int i = 0; i < 2; ++i) {
		int rotor_index = rotor_indices[i];
		x[i] = gd->rotors[rotor_index].x;
		y[i] = gd->rotors[rotor_index].y;
	}

	Connector connectors[2];
	for (int i = 0; i < 2; ++i) {
		connectors[i].type = CONNECTOR_ROTOR;
		connectors[i].target = rotor_indices[i];
	}

	if (x[0] == x[1]) {
		// the rotors are vertically arranged
		if (y[0] < y[1]) {
			connectors[0].rotor.position = ROTOR_POSITION_BOTTOM;
			connectors[1].rotor.position = ROTOR_POSITION_TOP;
		} else {
			connectors[0].rotor.position = ROTOR_POSITION_TOP;
			connectors[1].rotor.position = ROTOR_POSITION_BOTTOM;
		}
	} else if (y[0] == y[1]) {
		// the rotors are horizontally arranged
		if (x[0] < x[1]) {
			connectors[0].rotor.position = ROTOR_POSITION_RIGHT;
			connectors[1].rotor.position = ROTOR_POSITION_LEFT;
		} else {
			connectors[0].rotor.position = ROTOR_POSITION_LEFT;
			connectors[1].rotor.position = ROTOR_POSITION_RIGHT;
		}
	} else {
		SDL_assert(false);
	}
	placeLine(gd, &connectors[0], &connectors[1]);
}

void buildMap1(GameData *gd) {
	// available colors
	gd->ball_type_count = 0;
	addBallType(gd, BALL_TYPE_BLUE);
	addBallType(gd, BALL_TYPE_GREEN);
	addBallType(gd, BALL_TYPE_YELLOW);
	addBallType(gd, BALL_TYPE_MAGENTA);

	// lines
	int line_index_1 = gd->line_count;
	int line_index_2 = line_index_1 + 1;
	gd->lines[line_index_1].x1 = 100;
	gd->lines[line_index_1].y1 = 100;
	gd->lines[line_index_1].x2 = 300;
	gd->lines[line_index_1].y2 = 150;
	gd->lines[line_index_2].x1 = 300;
	gd->lines[line_index_2].y1 = 150;
	gd->lines[line_index_2].x2 = 100;
	gd->lines[line_index_2].y2 = 100;
	//connectors
	gd->lines[line_index_2].connector.target = line_index_1;
	gd->lines[line_index_2].connector.type = CONNECTOR_LINE;
	gd->lines[line_index_1].connector.target = line_index_2;
	gd->lines[line_index_1].connector.type = CONNECTOR_LINE;
	gd->line_count += 2;

	// place a ball
	int ball_index = placeBallFree(gd, BALL_TYPE_BLUE, 200, 100, 5, 0);
	gd->balls[ball_index].connector.type = CONNECTOR_LINE;
	gd->balls[ball_index].connector.target = 0;

	// more lines
	gd->lines[gd->line_count + 0].x1 = 400;
	gd->lines[gd->line_count + 0].y1 = 400;
	gd->lines[gd->line_count + 0].x2 = 500;
	gd->lines[gd->line_count + 0].y2 = 400;
	gd->lines[gd->line_count + 1].x1 = 500;
	gd->lines[gd->line_count + 1].y1 = 400;
	gd->lines[gd->line_count + 1].x2 = 500;
	gd->lines[gd->line_count + 1].y2 = 500;
	gd->lines[gd->line_count + 2].x1 = 500;
	gd->lines[gd->line_count + 2].y1 = 500;
	gd->lines[gd->line_count + 2].x2 = 400;
	gd->lines[gd->line_count + 2].y2 = 500;
	gd->lines[gd->line_count + 3].x1 = 400;
	gd->lines[gd->line_count + 3].y1 = 500;
	gd->lines[gd->line_count + 3].x2 = 400;
	gd->lines[gd->line_count + 3].y2 = 400;
	// put the connectors
	gd->lines[gd->line_count + 0].connector.type = CONNECTOR_LINE;
	gd->lines[gd->line_count + 0].connector.target = gd->line_count + 1;
	gd->lines[gd->line_count + 1].connector.type = CONNECTOR_LINE;
	gd->lines[gd->line_count + 1].connector.target = gd->line_count + 2;
	gd->lines[gd->line_count + 2].connector.type = CONNECTOR_LINE;
	gd->lines[gd->line_count + 2].connector.target = gd->line_count + 3;
	gd->lines[gd->line_count + 3].connector.type = CONNECTOR_LINE;
	gd->lines[gd->line_count + 3].connector.target = gd->line_count + 0;
	gd->line_count += 4;

	// place a ball
	ball_index = placeBallFree(gd, BALL_TYPE_GREEN, 400, 400, 5, 0);
	gd->balls[ball_index].connector.type = CONNECTOR_LINE;
	gd->balls[ball_index].connector.target = 3;
}

void buildMap2(GameData *gd) {
	// available colors
	gd->ball_type_count = 0;
	addBallType(gd, BALL_TYPE_BLUE);
	addBallType(gd, BALL_TYPE_GREEN);
	addBallType(gd, BALL_TYPE_YELLOW);
	addBallType(gd, BALL_TYPE_MAGENTA);
	
	int rotor_index = gd->rotor_count;
	int line_index = gd->line_count;
	gd->rotors[rotor_index].x = 400;
	gd->rotors[rotor_index].y = 300;
	gd->rotors[rotor_index].connectors[ROTOR_POSITION_RIGHT].type = CONNECTOR_LINE;
	gd->rotors[rotor_index].connectors[ROTOR_POSITION_RIGHT].target = line_index;
	gd->rotors[rotor_index].connectors[ROTOR_POSITION_TOP].type = CONNECTOR_LINE;
	gd->rotors[rotor_index].connectors[ROTOR_POSITION_TOP].target = line_index + 1;
	gd->rotors[rotor_index].connectors[ROTOR_POSITION_LEFT].type = CONNECTOR_LINE;
	gd->rotors[rotor_index].connectors[ROTOR_POSITION_LEFT].target = line_index + 2;
	gd->rotors[rotor_index].connectors[ROTOR_POSITION_BOTTOM].type = CONNECTOR_LINE;
	gd->rotors[rotor_index].connectors[ROTOR_POSITION_BOTTOM].target = line_index + 3;
	gd->rotors[rotor_index].balls[ROTOR_POSITION_RIGHT] = -1;
	gd->rotors[rotor_index].balls[ROTOR_POSITION_TOP] = -1;
	gd->rotors[rotor_index].balls[ROTOR_POSITION_LEFT] = -1;
	gd->rotors[rotor_index].balls[ROTOR_POSITION_BOTTOM] = -1;
	
	gd->lines[line_index + 0].x1 = 400 + 15;
	gd->lines[line_index + 0].x2 = 400 + 15 + 200;
	gd->lines[line_index + 0].y1 = 300;
	gd->lines[line_index + 0].y2 = 300;
	gd->lines[line_index + 0].connector.type = CONNECTOR_LINE;
	gd->lines[line_index + 0].connector.target = line_index + 4;
	
	gd->lines[line_index + 4].x1 = 400 + 15 + 200;
	gd->lines[line_index + 4].x2 = 400 + 15;
	gd->lines[line_index + 4].y1 = 300;
	gd->lines[line_index + 4].y2 = 300;
	gd->lines[line_index + 4].connector.type = CONNECTOR_ROTOR;
	gd->lines[line_index + 4].connector.rotor.position = ROTOR_POSITION_RIGHT;
	gd->lines[line_index + 4].connector.target = rotor_index;
	
	gd->lines[line_index + 1].x1 = 400;
	gd->lines[line_index + 1].x2 = 400;
	gd->lines[line_index + 1].y1 = 300 - 15;
	gd->lines[line_index + 1].y2 = 300 - 15 - 200;
	gd->lines[line_index + 1].connector.type = CONNECTOR_LINE;
	gd->lines[line_index + 1].connector.target = line_index + 5;
	
	gd->lines[line_index + 5].x1 = 400;
	gd->lines[line_index + 5].x2 = 400;
	gd->lines[line_index + 5].y1 = 300 - 15 - 200;
	gd->lines[line_index + 5].y2 = 300 - 15;
	gd->lines[line_index + 5].connector.type = CONNECTOR_ROTOR;
	gd->lines[line_index + 5].connector.rotor.position = ROTOR_POSITION_TOP;
	gd->lines[line_index + 5].connector.target = rotor_index;
	
	gd->lines[line_index + 2].x1 = 400 - 15;
	gd->lines[line_index + 2].x2 = 400 - 15 - 200;
	gd->lines[line_index + 2].y1 = 300;
	gd->lines[line_index + 2].y2 = 300;
	gd->lines[line_index + 2].connector.type = CONNECTOR_LINE;
	gd->lines[line_index + 2].connector.target = line_index + 6;
	
	gd->lines[line_index + 6].x1 = 400 - 15 - 200;
	gd->lines[line_index + 6].x2 = 400 - 15;
	gd->lines[line_index + 6].y1 = 300;
	gd->lines[line_index + 6].y2 = 300;
	gd->lines[line_index + 6].connector.type = CONNECTOR_ROTOR;
	gd->lines[line_index + 6].connector.rotor.position = ROTOR_POSITION_LEFT;
	gd->lines[line_index + 6].connector.target = rotor_index;
	
	gd->lines[line_index + 3].x1 = 400;
	gd->lines[line_index + 3].x2 = 400;
	gd->lines[line_index + 3].y1 = 300 + 15;
	gd->lines[line_index + 3].y2 = 300 + 15 + 200;
	gd->lines[line_index + 3].connector.type = CONNECTOR_LINE;
	gd->lines[line_index + 3].connector.target = line_index + 7;
	
	gd->lines[line_index + 7].x1 = 400;
	gd->lines[line_index + 7].x2 = 400;
	gd->lines[line_index + 7].y1 = 300 + 15 + 200;
	gd->lines[line_index + 7].y2 = 300 + 15;
	gd->lines[line_index + 7].connector.type = CONNECTOR_ROTOR;
	gd->lines[line_index + 7].connector.rotor.position = ROTOR_POSITION_BOTTOM;
	gd->lines[line_index + 7].connector.target = rotor_index;

	int ball_index;
	ball_index = placeBallFree(gd, BALL_TYPE_GREEN, 0, 0, 0, 0);
	gd->balls[ball_index].connector.type = CONNECTOR_ROTOR;
	gd->balls[ball_index].connector.rotor.position = ROTOR_POSITION_RIGHT;
	gd->balls[ball_index].connector.target = gd->rotor_count;
	gd->rotors[gd->rotor_count].balls[ROTOR_POSITION_RIGHT] = ball_index;
	updateBallPosition(gd, ball_index);
	ball_index = placeBallFree(gd, BALL_TYPE_RED, 0, 0, 0, 0);
	gd->balls[ball_index].connector.type = CONNECTOR_ROTOR;
	gd->balls[ball_index].connector.rotor.position = ROTOR_POSITION_LEFT;
	gd->balls[ball_index].connector.target = gd->rotor_count;
	gd->rotors[gd->rotor_count].balls[ROTOR_POSITION_LEFT] = ball_index;
	updateBallPosition(gd, ball_index);

	gd->rotor_count += 1;
	gd->line_count += 8;
}

void buildMap3(GameData *gd) {
	placeRotor(gd, 300.0, 200.0);
	placeRotor(gd, 300.0, 400.0);
	placeRotor(gd, 500.0, 200.0);
	placeRotor(gd, 500.0, 400.0);
	placeRotor(gd, 700.0, 200.0);
	placeRotor(gd, 700.0, 400.0);

	placeBallInRotor(gd, 0, ROTOR_POSITION_LEFT, BALL_TYPE_RED);
	
	placeLineBetweenRotors(gd, 0, 1);
	placeLineBetweenRotors(gd, 2, 3);
	placeLineBetweenRotors(gd, 0, 2);
	placeLineBetweenRotors(gd, 1, 3);
	placeLineBetweenRotors(gd, 4, 5);
	placeLineBetweenRotors(gd, 3, 5);
	placeLineBetweenRotors(gd, 2, 4);
}

void buildMap4(GameData *gd) {
	placeRotor(gd, 300.0, 100.0);
	placeRotor(gd, 500.0, 100.0);
	placeRotor(gd, 300.0, 300.0);
	placeRotor(gd, 500.0, 300.0);
	placeRotor(gd, 300.0, 500.0);
	placeRotor(gd, 500.0, 500.0);
	
	placeLineBetweenRotors(gd, 0, 1);
	placeLineBetweenRotors(gd, 2, 3);
	placeLineBetweenRotors(gd, 4, 5);
	placeLineBetweenRotors(gd, 0, 2);
	placeLineBetweenRotors(gd, 2, 4);
	placeLineBetweenRotors(gd, 1, 3);
	placeLineBetweenRotors(gd, 3, 5);
	
	int inserter_index;
	for (int i = 0; i < 4; ++i) {
		inserter_index = addInserter(gd);
		gd->inserters[inserter_index].connector_success.type = CONNECTOR_ROTOR;
		gd->inserters[inserter_index].connector_success.target = (i % 2);
		gd->inserters[inserter_index].connector_success.rotor.position = ROTOR_POSITION_TOP;
	}

	int line_indices[6];
	static float polygon[] = { 800.0, 500.0, 300.0, 0.0, 300.0, 500.0, 800.0 };
	for (int i = 0; i < 6; ++i) {
		int line_index = addLine(gd);
		line_indices[i] = line_index;
		gd->lines[line_index].x1 = polygon[i];
		gd->lines[line_index].x2 = polygon[i + 1];
		gd->lines[line_index].y1 = 50.0;
		gd->lines[line_index].y2 = 50.0;
	}
	
	gd->lines[line_indices[0]].connector.type = CONNECTOR_INSERTER;
	gd->lines[line_indices[0]].connector.target = 1;
	gd->inserters[1].connector_failure.type = CONNECTOR_LINE;
	gd->inserters[1].connector_failure.target = line_indices[1];
	gd->lines[line_indices[1]].connector.type = CONNECTOR_INSERTER;
	gd->lines[line_indices[1]].connector.target = 0;
	gd->inserters[0].connector_failure.type = CONNECTOR_LINE;
	gd->inserters[0].connector_failure.target = line_indices[2];
	gd->lines[line_indices[2]].connector.type = CONNECTOR_LINE;
	gd->lines[line_indices[2]].connector.target = line_indices[3];
	gd->lines[line_indices[3]].connector.type = CONNECTOR_INSERTER;
	gd->lines[line_indices[3]].connector.target = 2;
	gd->inserters[2].connector_failure.type = CONNECTOR_LINE;
	gd->inserters[2].connector_failure.target = line_indices[4];
	gd->lines[line_indices[4]].connector.type = CONNECTOR_INSERTER;
	gd->lines[line_indices[4]].connector.target = 3;
	gd->inserters[3].connector_failure.type = CONNECTOR_LINE;
	gd->inserters[3].connector_failure.target = line_indices[5];
	gd->lines[line_indices[5]].connector.type = CONNECTOR_LINE;
	gd->lines[line_indices[5]].connector.target = line_indices[0];

	int spawn_index = addSpawn(gd);
	gd->spawns[spawn_index].connector.type = CONNECTOR_LINE;
	gd->spawns[spawn_index].connector.target = line_indices[0];
}
