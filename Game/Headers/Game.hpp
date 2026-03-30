#ifndef GAME_HPP
#define GAME_HPP
#pragma once

#include "Network.hpp"
#include "Pieces.hpp"
#include "Menu.hpp"

constexpr float SQUARE_EPS = 0.01f;

class Game;

enum GameState
{
    MENU = 1,
    HOSTING,
    CONNECTING,
    IN_MATCH
};


struct BoardSquare
{
    char name[3]{};
    Vector3 position;
    Piece *occupyingPiece = nullptr;
    bool isOccupied = false;
    Mesh mesh = {0};
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
    std::map<std::string, Model> chessBoardModels;
    // Pieces
    std::map<std::string, Model> blackPiecesModels;
    std::map<std::string, Model> whitePiecesModels;
    std::string ipToConnectTo = "";
    NetMove lastReceivedMove = {nullptr, nullptr};

public:
    const Vector3 boardOrigin = {0.0f, 0.0f, 0.0f};
    Vector3 lookingAt = {0.0f, 0.0f, 0.0f};

    const float squareSize = 2.4f;
    Turn playersTurn = Turn::NONE;
    Turn turnOrder = Turn::White;

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
    void OccupyCells();
    BoardSquare *GetSquareAtPosition(const Vector3 &position);
    BoardSquare *GetSquareByName( char *name);
    bool SameSquarePos(const Vector3& a, const Vector3& b);
};

#endif // GAME_HPP