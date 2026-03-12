#ifndef GAME_HPP
#define GAME_HPP
#pragma once

#include "raylib.h"
#include "raymath.h"
#include "map"
#include "string"
#include "cstdio"

class Game;

enum class Turn
{
    White,
    Black
};

class Piece
{
private:
    std::string name;
    Turn *turnOrder;
    const char *color; // "black" or "white"
    Vector3 position;
    Vector3 validMoves[28];
    Model *model;
    Camera3D *camera;
    Game *game;
    bool hasMoved = false;
    bool selected = false;

public:
    Piece() {}
    Piece(Turn *turnOrder, const char *color, const Vector3 position, Model &model, Camera3D &camera, Game &game);
    ~Piece();
    void Update();
    void Draw();
    void Select();
    void Deselect();
    void MoveTo(const Vector3 &newPosition);
    bool IsValidMove(const Vector3 &targetPosition);
};

struct BoardSquare
{
    std::string name;
    Vector3 position;
    bool isOccupied = false;
};

class Game
{
private:
    // Window and camera
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
    std::map<std::string, Model> tanPiecesModels;
    std::map<std::string, Model> brownPiecesModels;

public:
    Piece *selectedPiece = nullptr;
    BoardSquare chessBoardSquares[8][8];
    Piece piece;
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
};

#endif // GAME_HPP