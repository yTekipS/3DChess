#include "../Headers/Game.hpp"

int main()
{
    Game *game = new Game();
    game->Run();
    delete game;
    return 0;
}
