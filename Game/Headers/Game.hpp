#ifndef GAME_HPP
#define GAME_HPP
#pragma once

#include "Network.hpp"
#include "Pieces.hpp"
#include "Menu.hpp"


class Game;

enum GameState
{
    MENU = 1,
    HOSTING,
    CONNECTING
};

enum class Turn
{
    White,
    Black
};

struct BoardSquare
{
    const char *name;
    Vector3 position;
    Piece *occupyingPiece = nullptr;
    bool isOccupied = false;
};

class Game
{
private:
    // Window and camera
    Network network;
    Menu menu;
    GameState gameState = MENU;
    const int screenWidth = 1280;
    const int screenHeight = 720;
    Camera3D camera = {0};
    // Chessboard
    const Vector3 boardOrigin = {0.0f, 0.0f, 0.0f};
    const float squareSize = 2.4f;
    std::map<std::string, Model> chessBoardModels;
    Turn turnOrder = Turn::White;
    // Pieces
    std::map<std::string, Model> blackPiecesModels;
    std::map<std::string, Model> whitePiecesModels;

public:
    Piece *selectedPiece = nullptr;
    BoardSquare chessBoardSquares[8][8];
    std::map<std::string, Piece> whitePieces;
    std::map<std::string, Piece> blackPieces;
    std::string currentTheme = menu.GetTheme();
    std::string lastTheme = currentTheme;
    Game();
    ~Game();
    void Update();
    void Draw();
    void Run();
    void LoadModels();
    void UnloadModels();
    void FillChessBoardSquares();
    void CameraControl();
    void FlipCamera();
    void InitPieces();
    void InitBlackPieces();
    void InitWhitePieces();
};

#endif // GAME_HPP