#ifndef PIECES_HPP
#define PIECES_HPP
#pragma once

#include <raylib.h>
#include <raymath.h>
#include <map>
#include <string>
#include <cstdio>

class Game;
enum class Turn;

class Piece
{
private:
    Turn *turnOrder;
    const char *color; // "black" or "white"
    Vector3 validMoves[28];
    int validMoveCount = 0;
    Model *model;
    Camera3D *camera;
    Game *game;
    bool hasMoved = false;
    bool selected = false;
    char type = ' '; // p, r, n, b, q, k (pawn, rook, knight, bishop, queen, king)
    Piece *king = nullptr;
    bool showOnce = true; // For debugging: only log valid moves once per selection

public:
    Vector3 position;
    std::string name;
    bool moved = false;

    struct NetMove
    {
        char fromSquare[3];  // "A2\0"
        char toSquare[3];    // "A4\0"
    } pieceLastNetMove;

    Piece() {};
    Piece(const Piece &) = default;
    Piece &operator=(const Piece &other);
    Piece(Turn *turnOrder, const char *color, std::string name, const Vector3 position,char type, Model &model, Camera3D &camera, Game &game);
    ~Piece();
    void GetKingReference();
    void Update();
    void Draw();
    void Select();
    void Deselect();
    void FindValidMoves();
    int FindValidPawnMoves(int moveId = 0);
    int FindValidRookMoves(int moveId = 0);
    int FindValidKnightMoves(int moveId = 0);
    int FindValidBishopMoves(int moveId = 0);
    int FindValidQueenMoves(int moveId = 0);
    int FindValidKingMoves(int moveId = 0);
    bool IsInCheck();
    bool IsInCheckmate();
    bool IsInStalemate();
    bool PutsKingInCheck(const Vector3 &targetPosition);
    void MoveTo();
    bool IsValidMove(const Vector3 &targetPosition);
};

#endif // PIECES_HPP