#include "../Headers/Game.hpp"
#include "../Headers/Pieces.hpp"

Piece::Piece(Turn *turnOrder, const char *color, const Vector3 position, Model &model, Camera3D &camera, Game &game)
    : turnOrder(turnOrder), color(color), position(position), model(&model), camera(&camera), game(&game)
{
}

Piece &Piece::operator=(const Piece &other)
{
    if (this == &other)
        return *this;

    name = other.name;
    turnOrder = other.turnOrder;
    color = other.color;
    position = other.position;
    for (int i = 0; i < 28; i++)
    {
        validMoves[i] = other.validMoves[i];
    }
    model = other.model;
    camera = other.camera;
    game = other.game;
    hasMoved = other.hasMoved;
    selected = other.selected;

    return *this;
}

Piece::~Piece()
{
}

void Piece::Update()
{
    if (selected)
        Deselect();
    else
        Select();
}

void Piece::Draw()
{
    float rotationAngle = (std::string(color) == "white") ? 0.0f : 180.0f;
    DrawModelEx(*model, position, Vector3 {0.0f,1.0f,0.0f}, rotationAngle, Vector3One(), selected ? RED : WHITE);
}

void Piece::Select()
{
    Turn pieceTurn = (std::string(color) == "white") ? Turn::White : Turn::Black;
    if (*turnOrder != pieceTurn || game->selectedPiece != nullptr)
        return;

    Ray mouseRay = GetScreenToWorldRay(GetMousePosition(), *camera);

    Matrix worldTransform = MatrixMultiply(model->transform, MatrixTranslate(position.x, position.y, position.z));
    RayCollision meshCollision = GetRayCollisionMesh(mouseRay, model->meshes[0], worldTransform);

    if (meshCollision.hit && IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
    {
        TraceLog(LOG_INFO, "Selected piece at position: (%f, %f, %f)", position.x, position.y, position.z);

        selected = true;
        game->selectedPiece = this;
    }
}

void Piece::Deselect()
{
    Turn pieceTurn = (std::string(color) == "white") ? Turn::White : Turn::Black;
    if (*turnOrder != pieceTurn || game->selectedPiece != this || game->selectedPiece == nullptr)
        return;

    Ray mouseRay = GetScreenToWorldRay(GetMousePosition(), *camera);

    Matrix worldTransform = MatrixMultiply(model->transform, MatrixTranslate(position.x, position.y, position.z));
    RayCollision meshCollision = GetRayCollisionMesh(mouseRay, model->meshes[0], worldTransform);

    if (meshCollision.hit && IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
    {
        TraceLog(LOG_INFO, "Deselected piece at position: (%f, %f, %f)", position.x, position.y, position.z);
        selected = false;
        game->selectedPiece = nullptr;
    }
}