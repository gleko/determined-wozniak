#include <iostream>
#include <vector>
#include <algorithm>
#include <chrono>
#include <thread>
#include <cmath>
#include <SDL2/SDL.h>

#include <utils.h>
#include <tinyraycaster.h>

int main() {
    if (SDL_Init(SDL_INIT_VIDEO)) {
        std::cerr << "Failed to initialize SDL: " << SDL_GetError() << std::endl;
        return -1;
    }

    FrameBuffer framebuffer{1024, 512, std::vector<uint32_t>(1024*512, pack_color(255, 255, 255))};
    GameState gamestate{ Map(),                                // game map
                  {3.456, 2.345, 1.523, M_PI/3., 0, 0}, // player
                  { {3.523, 3.812, 2, 0},               // monsters lists
                    {1.834, 8.765, 0, 0},
                    {5.323, 5.365, 1, 0},
                    {14.32, 13.36, 3, 0},
                    {4.123, 10.76, 1, 0} },
                  Texture("../assets/walltext.bmp", SDL_PIXELFORMAT_ABGR8888),  // textures for the walls
                  Texture("../assets/monsters.bmp", SDL_PIXELFORMAT_ABGR8888)}; // textures for the monsters
    if (!gamestate.tex_walls.count || !gamestate.tex_monst.count) {
        std::cerr << "Failed to load textures" << std::endl;
        return -1;
    }

    SDL_Window   *window   = nullptr;
    SDL_Renderer *renderer = nullptr;

    if (SDL_CreateWindowAndRenderer(framebuffer.width, framebuffer.height, SDL_WINDOW_SHOWN | SDL_WINDOW_INPUT_FOCUS, &window, &renderer)) {
        std::cerr << "Failed to create window and renderer: " << SDL_GetError() << std::endl;
        return -2;
    }

    SDL_Texture *framebuffer_texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ABGR8888, SDL_TEXTUREACCESS_STREAMING, framebuffer.width, framebuffer.height);
    if (!framebuffer_texture) {
        std::cerr << "Failed to create framebuffer texture : " << SDL_GetError() << std::endl;
        return -3;
    }

    auto t1 = std::chrono::high_resolution_clock::now();
    while (1) {
        { // sleep if less than 20 ms since last re-rendering; TODO: decouple rendering and event polling frequencies
            auto t2 = std::chrono::high_resolution_clock::now();
            std::chrono::duration<double, std::milli> fp_ms = t2 - t1;
            if (fp_ms.count()<20) {
                std::this_thread::sleep_for(std::chrono::milliseconds(3));
                continue;
            }
            t1 = t2;
        }

        { // poll events and update player's state (walk/turn flags); TODO: move this block to a more appropriate place
            SDL_Event event;
            if (SDL_PollEvent(&event)) {
                if (SDL_QUIT==event.type || (SDL_KEYDOWN==event.type && SDLK_ESCAPE==event.key.keysym.sym)) break;
                if (SDL_KEYUP==event.type) {
                    if ('a'==event.key.keysym.sym || 'd'==event.key.keysym.sym) gamestate.player.turn = 0;
                    if ('w'==event.key.keysym.sym || 's'==event.key.keysym.sym) gamestate.player.walk = 0;
                }
                if (SDL_KEYDOWN==event.type) {
                    if ('a'==event.key.keysym.sym) gamestate.player.turn = -1;
                    if ('d'==event.key.keysym.sym) gamestate.player.turn =  1;
                    if ('w'==event.key.keysym.sym) gamestate.player.walk =  1;
                    if ('s'==event.key.keysym.sym) gamestate.player.walk = -1;
                }
            }
        }

        { // update player's position; TODO: move this block to a more appropriate place
            gamestate.player.a += float(gamestate.player.turn)*.05; // TODO measure elapsed time and modify the speed accordingly
            float nx = gamestate.player.x + gamestate.player.walk*cos(gamestate.player.a)*.05;
            float ny = gamestate.player.y + gamestate.player.walk*sin(gamestate.player.a)*.05;

            if (int(nx)>=0 && int(nx)<int(gamestate.map.w) && int(ny)>=0 && int(ny)<int(gamestate.map.h)) {
                if (gamestate.map.is_empty(nx, gamestate.player.y)) gamestate.player.x = nx;
                if (gamestate.map.is_empty(gamestate.player.x, ny)) gamestate.player.y = ny;
            }
            for (size_t i=0; i<gamestate.monsters.size(); i++) { // update the distances from the player to each sprite
                gamestate.monsters[i].player_dist = std::sqrt(pow(gamestate.player.x - gamestate.monsters[i].x, 2) + pow(gamestate.player.y - gamestate.monsters[i].y, 2));
            }
            std::sort(gamestate.monsters.begin(), gamestate.monsters.end()); // sort it from farthest to closest
        }

        render(framebuffer, gamestate); // render the scene to the frambuffer

        { // copy the framebuffer contents to the screen
            SDL_UpdateTexture(framebuffer_texture, NULL, reinterpret_cast<void *>(framebuffer.img.data()), framebuffer.width*4);
            SDL_RenderClear(renderer);
            SDL_RenderCopy(renderer, framebuffer_texture, NULL, NULL);
            SDL_RenderPresent(renderer);
        }
    }

    SDL_DestroyTexture(framebuffer_texture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}

