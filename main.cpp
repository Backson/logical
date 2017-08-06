#include "logical.hpp"

#include "game.hpp"

#include <cstdio>
#include <cmath>

bool should_quit = false;

const int ROTOR_POSITIONS[] = {
	ROTOR_POSITION_RIGHT,
	ROTOR_POSITION_TOP,
	ROTOR_POSITION_LEFT,
	ROTOR_POSITION_BOTTOM,
};

const Uint8 BALL_COLORS[][4] = {
	{   0,   0,   0, SDL_ALPHA_OPAQUE },
	{ 255,   0,   0, SDL_ALPHA_OPAQUE },
	{   0, 255,   0, SDL_ALPHA_OPAQUE },
	{   0,   0, 255, SDL_ALPHA_OPAQUE },
	{   0, 255, 255, SDL_ALPHA_OPAQUE },
	{ 255,   0, 255, SDL_ALPHA_OPAQUE },
	{ 255, 255,   0, SDL_ALPHA_OPAQUE },
	{ 255, 255, 255, SDL_ALPHA_OPAQUE },
};

void printDisplayInfo(int display_index) {
	// print display name
	const char *display_name = SDL_GetDisplayName(display_index);
	if (display_name != NULL) {
		SDL_Log("Display %i: %s", display_index, display_name);
	}

	// how many modes it knows
	int modes_count = SDL_GetNumDisplayModes(display_index);
	if (modes_count < 1) {
		SDL_Log("SDL_GetNumDisplayModes(%i) failed (%i): %s", display_index, modes_count, SDL_GetError());
		return;
	}
	SDL_Log("Display %i reported %i modes", display_index, modes_count);

	// print all the modes
	for (int mode_index = 0; mode_index < modes_count; ++mode_index) {
		SDL_DisplayMode mode;
		if (SDL_GetDisplayMode(display_index, mode_index, &mode) != 0) {
			SDL_Log("SDL_GetDisplayMode(%i,%i,mode) failed: %s", display_index, mode_index, SDL_GetError());
			continue;
		}
		SDL_Log("Display %i mode %i: %ix%i@%i", display_index, mode_index, mode.w, mode.h, mode.refresh_rate);
	}
}

void printAllDisplaysInfo() {
	// get number of displays
	int display_count = SDL_GetNumVideoDisplays();
    if (display_count < 1) {
		SDL_Log("SDL_GetNumVideoDisplays() failed (%i): %s", display_count, SDL_GetError());
        return;
    }
	SDL_Log("Video Displays found: %i", display_count);

	// print display information
	for (int display_index = 0; display_index < display_count; ++display_index) {
		printDisplayInfo(display_index);
	}
}

void resetGame(GameData *gd) {
	clearGame(gd);
	random_seed(&gd->random, 42);
	buildMap4(gd);

	addBallType(gd, BALL_TYPE_BLUE);
	addBallType(gd, BALL_TYPE_GREEN);
	addBallType(gd, BALL_TYPE_YELLOW);
	addBallType(gd, BALL_TYPE_MAGENTA);

	placeRandomBallInSpawn(gd, 0);
}

void handleAllEvents(GameData *gd) {
    SDL_Event e;
    while (!should_quit && SDL_PollEvent(&e)) {
		handleEvent(gd, &e);
    }
}

void handleEvent(GameData *gd, const SDL_Event *e) {
    if (e->type == SDL_QUIT) {
        should_quit = true;
	} else if (e->type == SDL_KEYDOWN) {
		if (e->key.keysym.sym == SDLK_r) {
			resetGame(gd);
		} else if (e->key.keysym.sym == SDLK_ESCAPE) {
			should_quit = true;
		} else if (e->key.keysym.sym == SDLK_b) {
			placeBallInSpawn(gd, BALL_TYPE_GREEN, 0);
		}
	} else if (e->type == SDL_MOUSEBUTTONDOWN) {
		//int type = gd->ball_types[gd->ball_type_index_next];
		//placeBall(gd, (float)e->button.x, (float)e->button.y, 5.0, 0.0, type);
		//gd->ball_type_index_next = (gd->ball_type_index_next + 1) % gd->ball_type_count;
		for (int i = 0; i < gd->rotor_count; ++i) {
			// check if we clicked this rotor
			float mouse_x = (float)e->button.x - 0.5f;
			float mouse_y = (float)e->button.y - 0.5f;
			float rotor_x = gd->rotors[i].x;
			float rotor_y = gd->rotors[i].y;
			float x = mouse_x - rotor_x;
			float y = mouse_y - rotor_y;
			if (x < -30 || 30 < x || y < -30 || 30 < y) {
				// not clicked this rotor at all
				continue;
			} else if (-10 < x && x < 10 && -10 < y && y < 10) {
				// clicked rotor in the center -> turn rotor
				int direction = -1;
				if (e->button.button == 1)
					direction = ROTOR_CLOCKWISE;
				else if (e->button.button == 3)
					direction = ROTOR_ANTICLOCKWISE;
				if (direction != -1)
					turnRotor(gd, i, direction);
			} else if (e->button.button == 1) {
				// clicked rotor a bit away from the center -> release ball
				int position = -1;
				if (x * x > y * y && x > 0) {
					position = ROTOR_POSITION_RIGHT;
					SDL_Log("Rotor %d was clicked right", i);
				} else if (x * x > y * y && x < 0) {
					position = ROTOR_POSITION_LEFT;
					SDL_Log("Rotor %d was clicked left", i);
				} else if (x * x < y * y && y < 0) {
					position = ROTOR_POSITION_TOP;
					SDL_Log("Rotor %d was clicked top", i);
				} else if (x * x < y * y && y > 0) {
					position = ROTOR_POSITION_BOTTOM;
					SDL_Log("Rotor %d was clicked bottom", i);
				}
				if (position == -1)
					return;
				releaseBallFromRotor(gd, i, position);
			}
		}
	}
}

void turnRotor(GameData *gd, int rotor_index, int direction) {
	int *balls = gd->rotors[rotor_index].balls;
	// update rotor
	if (direction == ROTOR_CLOCKWISE) {
		int temp = balls[ROTOR_POSITION_RIGHT];
		balls[ROTOR_POSITION_RIGHT] = balls[ROTOR_POSITION_TOP];
		balls[ROTOR_POSITION_TOP] = balls[ROTOR_POSITION_LEFT];
		balls[ROTOR_POSITION_LEFT] = balls[ROTOR_POSITION_BOTTOM];
		balls[ROTOR_POSITION_BOTTOM] = temp;
		SDL_Log("Rotor %d was turned clockwise", rotor_index);
	} else if (direction == ROTOR_ANTICLOCKWISE) {
		int temp = balls[ROTOR_POSITION_RIGHT];
		balls[ROTOR_POSITION_RIGHT] = balls[ROTOR_POSITION_BOTTOM];
		balls[ROTOR_POSITION_BOTTOM] = balls[ROTOR_POSITION_LEFT];
		balls[ROTOR_POSITION_LEFT] = balls[ROTOR_POSITION_TOP];
		balls[ROTOR_POSITION_TOP] = temp;
		SDL_Log("Rotor %d was turned anticlockwise", rotor_index);
	} else {
		SDL_LogWarn(0, "Rotor %d illegal turn direction (%d)", rotor_index, direction);
		return;
	}

	// update balls
	for (int dir = 0; dir < 4; ++dir) {
		int ball_index = balls[ROTOR_POSITIONS[dir]];
		if (ball_index >= 0) {
			gd->balls[ball_index].connector.rotor.position = ROTOR_POSITIONS[dir];
			updateBallPosition(gd, ball_index);
		}
	}
}

void releaseBallFromRotor(GameData *gd, int rotor_index, int position) {
	int ball_index = gd->rotors[rotor_index].balls[position];
	if (ball_index >= 0 && gd->rotors[rotor_index].connectors[position].type != CONNECTOR_WALL) {
		changeBallConnector(gd, ball_index, &gd->rotors[rotor_index].connectors[position]);
		gd->balls[ball_index].released_counter++;
		gd->rotors[rotor_index].balls[position] = -1;
		SDL_Log("Released ball %d from rotor %d(%d)", ball_index, rotor_index, position);
	}
}

void updateBallPosition(GameData *gd, int ball_index) {
	if (gd->balls[ball_index].connector.type == CONNECTOR_ROTOR) {
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
	}
}

void progressBall(GameData *gd, int ball_index, Time t) {
	while (gd->balls[ball_index].type != BALL_TYPE_NONE) {
		// spawns are where new balls are created
		if (gd->balls[ball_index].connector.type == CONNECTOR_SPAWN) {
			// remember the index of the spawn of this ball for later
			int spawn_index = gd->balls[ball_index].connector.target;
			gd->balls[ball_index].spawn_index = spawn_index;
			// put the ball onto the first line (or something else)
			changeBallConnector(gd, ball_index, &gd->spawns[spawn_index].connector);
			continue;
		}

		// inserters are like an if-else branch: You go one way or another
		if (gd->balls[ball_index].connector.type == CONNECTOR_INSERTER) {
			// find whatever we are trying to insert into
			int inserter_index = gd->balls[ball_index].connector.target;
			const Connector *connector_success = &gd->inserters[inserter_index].connector_success;
			if (connector_success->type == CONNECTOR_ROTOR) {
				// check if the rotor is free
				int rotor_index = connector_success->target;
				int rotor_position = connector_success->rotor.position;
				bool is_free = gd->rotors[rotor_index].balls[rotor_position] == -1;
				if (is_free) {
					// put the ball into the rotor
					changeBallConnector(gd, ball_index, connector_success);
				} else {
					// put the ball on something else instead
					const Connector *connector_failure = &gd->inserters[inserter_index].connector_failure;
					changeBallConnector(gd, ball_index, connector_failure);
				}
			} else {
				// if we are trying to insert into something else then a rotor, it always succeeds
				changeBallConnector(gd, ball_index, connector_success);
			}
			// we may have inserted into another inserter, so keep going
			continue;
		}

		// lines go from one place to another
		if (gd->balls[ball_index].connector.type == CONNECTOR_LINE) {
			int line_index = gd->balls[ball_index].connector.target;
			// line vector
			float line_x = gd->lines[line_index].x2 - gd->lines[line_index].x1;
			float line_y = gd->lines[line_index].y2 - gd->lines[line_index].y1;
			// length of line vector
			float line_norm = sqrt(line_x * line_x + line_y * line_y);
			// line vector direction
			float dir_x = line_x / line_norm;
			float dir_y = line_y / line_norm;
			// ball position relative to line start
			float ball_x = gd->balls[ball_index].x - gd->lines[line_index].x1;
			float ball_y = gd->balls[ball_index].y - gd->lines[line_index].y1;
			// projection value of ball position onto line
			float proj = ball_x * dir_x + ball_y * dir_y;
			// move ball along line
			float dt = t / (float)seconds(1);
			float velocity = 160.0; // pixels per second
			float new_proj = proj + velocity * dt;
			// check whether the ball reached the end of the line
			if (new_proj > line_norm) {
				const Connector *connector_success = &gd->lines[line_index].connector;
				if (connector_success->type == CONNECTOR_ROTOR) {
					int rotor_index = connector_success->target;
					int rotor_position = connector_success->rotor.position;
					bool is_free = gd->rotors[rotor_index].balls[rotor_position] == -1;
					if (is_free) {
						changeBallConnector(gd, ball_index, connector_success);
					} else {
						const Connector *connector_failure = &gd->rotors[rotor_index].connectors[rotor_position];
						changeBallConnector(gd, ball_index, connector_failure);
					}
				} else {
					changeBallConnector(gd, ball_index, connector_success);
				}
			} else {
				float new_x = gd->lines[line_index].x1 + dir_x * new_proj;
				float new_y = gd->lines[line_index].y1 + dir_y * new_proj;
				gd->balls[ball_index].x = new_x;
				gd->balls[ball_index].y = new_y;
			}
			break;
		}

		if (gd->balls[ball_index].connector.type == CONNECTOR_FREE) {
			gd->balls[ball_index].x += gd->balls[ball_index].vx;
			gd->balls[ball_index].y += gd->balls[ball_index].vy;

			if (gd->time - gd->balls[ball_index].created > seconds(20)) {
				SDL_Log("Ball %i decayed", ball_index);
				removeBall(gd, ball_index);
			}
			break;
		}

		// balls in rotors do nothing
		if (gd->balls[ball_index].connector.type == CONNECTOR_ROTOR) {
			break;
		}
	}
}

void progressLogic(GameData *gd, Time t) {
	for (int i = 0; i < NBALLS; ++i) {
		progressBall(gd, i, t);
	}

	gd->time += t;
}

void renderEverything(GameData *gd, SDL_Renderer *renderer) {
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, SDL_ALPHA_OPAQUE);
    SDL_RenderClear(renderer);
	
	for (int i = 0; i < gd->line_count; ++i) {
		SDL_SetRenderDrawColor(renderer, 255, 255, 255, SDL_ALPHA_OPAQUE);
		SDL_RenderDrawLine(renderer, (int)gd->lines[i].x1, (int)gd->lines[i].y1, (int)gd->lines[i].x2, (int)gd->lines[i].y2);
	}

	for (int i = 0; i < gd->rotor_count; ++i) {
		SDL_Rect rect;
		rect.x = (int)gd->rotors[i].x - 30;
		rect.y = (int)gd->rotors[i].y - 30;
		rect.w = 60;
		rect.h = 60;
		if (gd->rotors[i].destroyed) {
			SDL_SetRenderDrawColor(renderer, 0x66, 0x66, 0x66, SDL_ALPHA_OPAQUE);
		} else {
			SDL_SetRenderDrawColor(renderer, 0xCC, 0xCC, 0xCC, SDL_ALPHA_OPAQUE);
		}
		SDL_RenderFillRect(renderer, &rect);
		SDL_SetRenderDrawColor(renderer, 0xFF, 0xFF, 0xFF, SDL_ALPHA_OPAQUE);
		SDL_RenderDrawRect(renderer, &rect);
	}

	for (int i = 0; i < NBALLS; ++i) {
		int type = gd->balls[i].type;
		if (type == BALL_TYPE_NONE)
			continue;
		float x = gd->balls[i].x;
		float y = gd->balls[i].y;
		Uint8 r = BALL_COLORS[type + 1][0];
		Uint8 g = BALL_COLORS[type + 1][1];
		Uint8 b = BALL_COLORS[type + 1][2];
		Uint8 a = BALL_COLORS[type + 1][3];
		SDL_SetRenderDrawColor(renderer, r, g, b, a);
		SDL_Rect rect = {(int)(x - 10), (int)(y - 10), 20, 20};
		SDL_RenderFillRect(renderer, &rect);
	}

	for (int i = 0; i < gd->rotor_count; ++i) {
		SDL_Rect rect;
		rect.x = (int)gd->rotors[i].x - 10;
		rect.y = (int)gd->rotors[i].y - 10;
		rect.w = 20;
		rect.h = 20;
		if (gd->rotors[i].destroyed) {
			SDL_SetRenderDrawColor(renderer, 0x66, 0x66, 0x66, SDL_ALPHA_OPAQUE);
		} else {
			SDL_SetRenderDrawColor(renderer, 0xCC, 0xCC, 0xCC, SDL_ALPHA_OPAQUE);
		}
		SDL_RenderFillRect(renderer, &rect);
		SDL_SetRenderDrawColor(renderer, 0xFF, 0xFF, 0xFF, SDL_ALPHA_OPAQUE);
		SDL_RenderDrawRect(renderer, &rect);
	}

    SDL_RenderPresent(renderer);
}

int main(int argc, char *argv[]) {
	// init SDL
	SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_TIMER | SDL_INIT_EVENTS);

	printAllDisplaysInfo();

	SDL_Rect display_bounds;
	if (SDL_GetDisplayBounds(0, &display_bounds) < 0) {
		SDL_Log("SDL_GetDisplayBounds failed: %s", SDL_GetError());
		return 1;
	}

	int width = 800, height = 600;
	int x = display_bounds.x + (display_bounds.w - width) / 2;
	int y = display_bounds.y + (display_bounds.h - height) / 2;

    SDL_Window *win = SDL_CreateWindow("Logical", x, y, width, height, 0);
	SDL_Renderer *renderer = SDL_CreateRenderer(win, -1, SDL_RENDERER_ACCELERATED);

	// timer stuff
	Time start_time = getCurrentTime();
	int target_fps = 60;
	Time time_per_frame = seconds(1) / target_fps;
	int64 frame = 0;

	// game data
	GameData gd;
	gd.renderer = renderer;
	resetGame(&gd);

	// game loop
    while (!should_quit) {
		Time frame_time = frame * time_per_frame;
		handleAllEvents(&gd);
		progressLogic(&gd, time_per_frame);
		renderEverything(&gd, renderer);
		++frame;
		sleepUntil(start_time + frame_time);
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(win);

	SDL_Quit();

	return 0;
}