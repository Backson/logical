#include "logical.hpp"

#include <cstdio>

bool should_quit = false;

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

void handleAllEvents(GameData *gd) {
    SDL_Event e;
    while (!should_quit && SDL_PollEvent(&e)) {
		handleEvent(gd, &e);
    }
}

int main(int argc, char *argv[]) {
	// init SDL
	SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_TIMER | SDL_INIT_EVENTS);

	// game data
	GameData gd;

	// start renderer
	if (startGraphics(&gd) != 0) {
		SDL_Log("Error during graphics initialization, quitting...");
		stopGraphics(&gd);
		return 1;
	}

	// timer stuff
	Time start_time = getCurrentTime();
	int target_fps = 60;
	Time time_per_frame = seconds(1) / target_fps;
	int64 frame = 0;

	// reset game
	resetGame(&gd);

	// game loop
    while (!should_quit) {
		Time frame_time = frame * time_per_frame;
		handleAllEvents(&gd);
		progressLogic(&gd, time_per_frame);
		renderEverything(&gd);
		++frame;
		sleepUntil(start_time + frame_time);
    }

	// finishing
	stopGraphics(&gd);
	SDL_Quit();
	return 0;
}
