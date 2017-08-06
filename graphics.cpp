#include "logical.hpp"

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

int startGraphics(GameData *gd) {
	gd->win = NULL;
	gd->renderer = NULL;

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
	if (win == NULL) {
		SDL_Log("SDL_CreateWindow failed: %s", SDL_GetError());
		return 2;
	}
	gd->win = win;

	SDL_Renderer *renderer = SDL_CreateRenderer(win, -1, SDL_RENDERER_ACCELERATED);
	if (renderer == NULL) {
		SDL_Log("SDL_CreateRenderer failed: %s", SDL_GetError());
		return 3;
	}
	gd->renderer = renderer;

	return 0;
}

void stopGraphics(GameData *gd) {
	if (gd->renderer != NULL) {
		SDL_DestroyRenderer(gd->renderer);
		gd->renderer = NULL;
	}

	if (gd->win != NULL) {
		SDL_DestroyWindow(gd->win);
		gd->win = NULL;
	}
}

void clearScreen(GameData *gd) {
    SDL_SetRenderDrawColor(gd->renderer, 0, 0, 0, SDL_ALPHA_OPAQUE);
    SDL_RenderClear(gd->renderer);
}

void renderLine(GameData *gd, int i) {
	SDL_SetRenderDrawColor(gd->renderer, 255, 255, 255, SDL_ALPHA_OPAQUE);
	SDL_RenderDrawLine(gd->renderer, (int)gd->lines[i].x1, (int)gd->lines[i].y1, (int)gd->lines[i].x2, (int)gd->lines[i].y2);
}

void renderLines(GameData *gd) {
	for (int i = 0; i < gd->line_count; ++i) {
		renderLine(gd, i);
	}
}

void renderRotor(GameData *gd, int i) {
	SDL_Rect rect;
	rect.x = (int)gd->rotors[i].x - 30;
	rect.y = (int)gd->rotors[i].y - 30;
	rect.w = 60;
	rect.h = 60;
	if (gd->rotors[i].destroyed) {
		SDL_SetRenderDrawColor(gd->renderer, 0x66, 0x66, 0x66, SDL_ALPHA_OPAQUE);
	} else {
		SDL_SetRenderDrawColor(gd->renderer, 0xCC, 0xCC, 0xCC, SDL_ALPHA_OPAQUE);
	}
	SDL_RenderFillRect(gd->renderer, &rect);
	SDL_SetRenderDrawColor(gd->renderer, 0xFF, 0xFF, 0xFF, SDL_ALPHA_OPAQUE);
	SDL_RenderDrawRect(gd->renderer, &rect);
}

void renderRotors(GameData *gd) {
	for (int i = 0; i < gd->rotor_count; ++i) {
		renderRotor(gd, i);
	}
}

void renderBall(GameData *gd, int i) {
	int type = gd->balls[i].type;
	if (type == BALL_TYPE_NONE)
		return;
	float x = gd->balls[i].x;
	float y = gd->balls[i].y;
	Uint8 r = BALL_COLORS[type + 1][0];
	Uint8 g = BALL_COLORS[type + 1][1];
	Uint8 b = BALL_COLORS[type + 1][2];
	Uint8 a = BALL_COLORS[type + 1][3];
	SDL_SetRenderDrawColor(gd->renderer, r, g, b, a);
	SDL_Rect rect = {(int)(x - 10), (int)(y - 10), 20, 20};
	SDL_RenderFillRect(gd->renderer, &rect);
}

void renderBalls(GameData *gd) {
	for (int i = 0; i < NBALLS; ++i) {
		renderBall(gd, i);
	}
}

void renderRotorCenter(GameData *gd, int i) {
	SDL_Rect rect;
	rect.x = (int)gd->rotors[i].x - 10;
	rect.y = (int)gd->rotors[i].y - 10;
	rect.w = 20;
	rect.h = 20;
	if (gd->rotors[i].destroyed) {
		SDL_SetRenderDrawColor(gd->renderer, 0x66, 0x66, 0x66, SDL_ALPHA_OPAQUE);
	} else {
		SDL_SetRenderDrawColor(gd->renderer, 0xCC, 0xCC, 0xCC, SDL_ALPHA_OPAQUE);
	}
	SDL_RenderFillRect(gd->renderer, &rect);
	SDL_SetRenderDrawColor(gd->renderer, 0xFF, 0xFF, 0xFF, SDL_ALPHA_OPAQUE);
	SDL_RenderDrawRect(gd->renderer, &rect);
}

void renderRotorCenters(GameData *gd) {
	for (int i = 0; i < gd->rotor_count; ++i) {
		renderRotorCenter(gd, i);
	}
}

void renderEverything(GameData *gd) {
	clearScreen(gd);
	renderLines(gd);
	renderRotors(gd);
	renderBalls(gd);
	renderRotorCenters(gd);
    SDL_RenderPresent(gd->renderer);
}
