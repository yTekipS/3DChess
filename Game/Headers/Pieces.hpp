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
    Piece(const Piece &) = default;
    Piece &operator=(const Piece &other);
    Piece(Turn *turnOrder, const char *color, const Vector3 position, Model &model, Camera3D &camera, Game &game);
    ~Piece();
    void Update();
    void Draw();
    void Select();
    void Deselect();
    void MoveTo(const Vector3 &newPosition);
    bool IsValidMove(const Vector3 &targetPosition);
};

#endif