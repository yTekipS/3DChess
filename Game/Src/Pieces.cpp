#include "../Headers/Game.hpp"
#include "../Headers/Pieces.hpp"
#include <cmath>

Piece::Piece(Turn *turnOrder, const char *color, std::string name, const Vector3 position, char type, Model &model, Camera3D &camera, Game &game)
    : turnOrder(turnOrder), color(color), position(position), model(&model), name(name), camera(&camera), game(&game), type(type)
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
    validMoveCount = other.validMoveCount; // ADD THIS
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

void Piece::GetKingReference()
{
    if (type == 'k')
    {
        king = this;
    }
    else if (std::string(color) == "white")
    {
        auto it = game->whitePieces.find("king");
        king = (it != game->whitePieces.end()) ? &it->second : nullptr;
    }
    else
    {
        auto it = game->blackPieces.find("king");
        king = (it != game->blackPieces.end()) ? &it->second : nullptr;
    }
}

void Piece::Update()
{
    FindValidMoves();
    if (selected)
    {
        if (showOnce)
        {
            TraceLog(LOG_INFO, "Valid moves for %s:", name.c_str());
            for (int i = 0; i < validMoveCount; i++)
            {
                TraceLog(LOG_INFO, "Valid move %d for %s: (%f, %f, %f)", i, name.c_str(), validMoves[i].x, validMoves[i].y, validMoves[i].z);
            }
            showOnce = false;
        }
        Deselect();
    }
    else
        Select();
    MoveTo();
}

void Piece::Draw()
{
    float rotationAngle = (std::string(color) == "white") ? 0.0f : 180.0f;
    DrawModelEx(*model, position, Vector3{0.0f, 1.0f, 0.0f}, rotationAngle, Vector3One(), selected ? RED : WHITE);
}

void Piece::Select()
{
    Turn pieceTurn = (std::string(color) == "white") ? Turn::White : Turn::Black;
    if (*turnOrder != pieceTurn || game->selectedPiece != nullptr)
        return;

    if (model == nullptr || model->meshCount <= 0)
        return;

    Ray mouseRay = GetScreenToWorldRay(GetMousePosition(), *camera);

    Matrix worldTransform = MatrixMultiply(model->transform, MatrixTranslate(position.x, position.y, position.z));
    RayCollision meshCollision = GetRayCollisionMesh(mouseRay, model->meshes[0], worldTransform);

    if (meshCollision.hit && IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
    {
        TraceLog(LOG_INFO, "Selected piece at position: (%f, %f, %f)", position.x, position.y, position.z);
        game->lookingAt = position; // For debugging: log the piece's position when selected

        selected = true;
        game->selectedPiece = this;
    }
}

void Piece::Deselect()
{
    Turn pieceTurn = (std::string(color) == "white") ? Turn::White : Turn::Black;
    if (*turnOrder != pieceTurn || game->selectedPiece != this || game->selectedPiece == nullptr)
        return;

    if (model == nullptr || model->meshCount <= 0)
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

void Piece::FindValidMoves()
{
    validMoveCount = 0; // RESET BEFORE FINDING NEW MOVES
    switch (type)
    {
    case 'p':
        validMoveCount = FindValidPawnMoves(0);
        break;
    case 'r':
        validMoveCount = FindValidRookMoves(0);
        break;
    case 'n':
        validMoveCount = FindValidKnightMoves(0);
        break;
    case 'b':
        validMoveCount = FindValidBishopMoves(0);
        break;
    case 'q':
        validMoveCount = FindValidQueenMoves(0);
        break;
    case 'k':
        validMoveCount = FindValidKingMoves(0);
        break;
    default:
        break;
    }
}

int Piece::FindValidPawnMoves(int moveId)
{
    Vector2 A1position = {game->chessBoardSquares[0][0].position.x, game->chessBoardSquares[0][0].position.z}; // Assuming A1 is at (0,0) in the chessBoardSquares array
    int currentX = static_cast<int>((A1position.x - position.x) / game->squareSize);
    int currentZ = static_cast<int>((A1position.y + position.z) / game->squareSize);

   
    int direction = (std::string(color) == "white") ? 1 : -1;

    // Move forward
    int moveIndex = moveId;

    if (currentZ + direction >= 0 && currentZ + direction < 8)
    {
        if (!game->chessBoardSquares[currentX][currentZ + direction].isOccupied)
        {
            if (!PutsKingInCheck(game->chessBoardSquares[currentX][currentZ + direction].position))
                validMoves[moveIndex++] = game->chessBoardSquares[currentX][currentZ + direction].position;

            // Move two squares forward on the first move
            int twoStepZ = currentZ + 2 * direction;
            if (!hasMoved && twoStepZ >= 0 && twoStepZ < 8 && !game->chessBoardSquares[currentX][twoStepZ].isOccupied)
            {
                if (!PutsKingInCheck(game->chessBoardSquares[currentX][twoStepZ].position))
                    validMoves[moveIndex++] = game->chessBoardSquares[currentX][twoStepZ].position;
            }
        }
    }

    // Capture diagonally
    if (currentX - 1 >= 0 && currentZ + direction >= 0 && currentZ + direction < 8)
    {
        if (game->chessBoardSquares[currentX - 1][currentZ + direction].isOccupied)
        {
            if (!PutsKingInCheck(game->chessBoardSquares[currentX - 1][currentZ + direction].position))
                validMoves[moveIndex++] = game->chessBoardSquares[currentX - 1][currentZ + direction].position;
        }
    }

    if (currentX + 1 < 8 && currentZ + direction >= 0 && currentZ + direction < 8)
    {
        if (game->chessBoardSquares[currentX + 1][currentZ + direction].isOccupied)
        {
            if (!PutsKingInCheck(game->chessBoardSquares[currentX + 1][currentZ + direction].position))
                validMoves[moveIndex++] = game->chessBoardSquares[currentX + 1][currentZ + direction].position;
        }
    }
    return moveIndex; // Maximum of 4 valid moves for a pawn (2 forward, 2 diagonal captures)
}

int Piece::FindValidRookMoves(int moveId)
{
    int currentX = (int)((position.x - game->boardOrigin.x) / game->squareSize);
    int currentZ = (int)((position.z - game->boardOrigin.z) / game->squareSize);

    int directions[4][2] = {
        {1, 0}, {-1, 0}, {0, 1}, {0, -1}};

    int moveIndex = moveId;
    for (int i = 0; i < 4; i++)
    {
        for (int step = 1; step < 8; step++)
        {
            int targetX = currentX + step * directions[i][0];
            int targetZ = currentZ + step * directions[i][1];

            if (targetX >= 0 && targetX < 8 && targetZ >= 0 && targetZ < 8)
            {
                if (game->chessBoardSquares[targetX][targetZ].isOccupied)
                {
                    if (game->chessBoardSquares[targetX][targetZ].occupyingPiece != nullptr && game->chessBoardSquares[targetX][targetZ].occupyingPiece->color != color)
                    {
                        if (!PutsKingInCheck(game->chessBoardSquares[targetX][targetZ].position))
                            validMoves[moveIndex++] = game->chessBoardSquares[targetX][targetZ].position;
                    }
                    break;
                }

                if (!PutsKingInCheck(game->chessBoardSquares[targetX][targetZ].position))
                    validMoves[moveIndex++] = game->chessBoardSquares[targetX][targetZ].position;
            }
            else
            {
                break;
            }
        }
    }
    return moveIndex;
}

int Piece::FindValidKnightMoves(int moveId)
{
    // Knight moves in an L shape: 2 squares in one direction and 1 square perpendicular to that
    int currentX = (int)((position.x - game->boardOrigin.x) / game->squareSize);
    int currentZ = (int)((position.z - game->boardOrigin.z) / game->squareSize);

    int moveOffsets[8][2] = {
        {2, 1}, {2, -1}, {-2, 1}, {-2, -1}, {1, 2}, {1, -2}, {-1, 2}, {-1, -2}};

    int moveIndex = moveId;

    for (int i = 0; i < 8; i++)
    {
        int targetX = currentX + moveOffsets[i][0];
        int targetZ = currentZ + moveOffsets[i][1];

        if (targetX >= 0 && targetX < 8 && targetZ >= 0 && targetZ < 8)
        {
            if (!game->chessBoardSquares[targetX][targetZ].isOccupied ||
                (game->chessBoardSquares[targetX][targetZ].occupyingPiece != nullptr && game->chessBoardSquares[targetX][targetZ].occupyingPiece->color != color))
            {
                if (!PutsKingInCheck(game->chessBoardSquares[targetX][targetZ].position))
                    validMoves[moveIndex++] = game->chessBoardSquares[targetX][targetZ].position;
            }
        }
    }
    return moveIndex;
}

int Piece::FindValidBishopMoves(int moveId)
{
    // Bishop moves diagonally in all four directions
    int currentX = (int)((position.x - game->boardOrigin.x) / game->squareSize);
    int currentZ = (int)((position.z - game->boardOrigin.z) / game->squareSize);

    int directions[4][2] = {
        {1, 1}, {1, -1}, {-1, 1}, {-1, -1}};

    int moveIndex = moveId;

    for (int i = 0; i < 4; i++)
    {
        for (int step = 1; step < 8; step++)
        {
            int targetX = currentX + step * directions[i][0];
            int targetZ = currentZ + step * directions[i][1];

            if (targetX >= 0 && targetX < 8 && targetZ >= 0 && targetZ < 8)
            {
                if (game->chessBoardSquares[targetX][targetZ].isOccupied)
                {
                    if (game->chessBoardSquares[targetX][targetZ].occupyingPiece != nullptr && game->chessBoardSquares[targetX][targetZ].occupyingPiece->color != color)
                    {
                        if (!PutsKingInCheck(game->chessBoardSquares[targetX][targetZ].position))
                            validMoves[moveIndex++] = game->chessBoardSquares[targetX][targetZ].position;
                    }
                    break;
                }
                if (!PutsKingInCheck(game->chessBoardSquares[targetX][targetZ].position))
                    validMoves[moveIndex++] = game->chessBoardSquares[targetX][targetZ].position;
            }
            else
            {
                break;
            }
        }
    }
    return moveIndex;
}

int Piece::FindValidQueenMoves(int moveId)
{
    // Queen moves like both a rook and a bishop
    int moveIndex = moveId;
    moveIndex = FindValidRookMoves(moveIndex);
    moveIndex = FindValidBishopMoves(moveIndex);
    return moveIndex;
}

int Piece::FindValidKingMoves(int moveId)
{
    // King moves one square in any direction
    int currentX = (int)((position.x - game->boardOrigin.x) / game->squareSize);
    int currentZ = (int)((position.z - game->boardOrigin.z) / game->squareSize);

    int moveOffsets[8][2] = {
        {1, 0}, {1, 1}, {0, 1}, {-1, 1}, {-1, 0}, {-1, -1}, {0, -1}, {1, -1}};

    int moveIndex = moveId;

    for (int i = 0; i < 8; i++)
    {
        int targetX = currentX + moveOffsets[i][0];
        int targetZ = currentZ + moveOffsets[i][1];

        if (targetX >= 0 && targetX < 8 && targetZ >= 0 && targetZ < 8)
        {
            if (!game->chessBoardSquares[targetX][targetZ].isOccupied ||
                (game->chessBoardSquares[targetX][targetZ].occupyingPiece != nullptr && game->chessBoardSquares[targetX][targetZ].occupyingPiece->color != color))
            {
                if (!PutsKingInCheck(game->chessBoardSquares[targetX][targetZ].position))
                    validMoves[moveIndex++] = game->chessBoardSquares[targetX][targetZ].position;
            }
        }
    }
    return moveIndex;
}

bool Piece::IsInCheck()
{
    if (king == nullptr)
        return false; // Should never happen, but just in case

    int kingX = (int)((king->position.x - game->boardOrigin.x) / game->squareSize);
    int kingZ = (int)((king->position.z - game->boardOrigin.z) / game->squareSize);

    // Check whether any opponent piece attacks the king's square.
    // Important: this uses attack patterns directly and does NOT call
    // FindValidMoves()/PutsKingInCheck(), which avoids recursive loops.
    for (auto &piecePair : (std::string(color) == "white" ? game->blackPieces : game->whitePieces))
    {
        Piece &opponentPiece = piecePair.second;

        int currentX = (int)((opponentPiece.position.x - game->boardOrigin.x) / game->squareSize);
        int currentZ = (int)((opponentPiece.position.z - game->boardOrigin.z) / game->squareSize);

        switch (opponentPiece.type)
        {
        case 'p':
        {
            int direction = (std::string(opponentPiece.color) == "white") ? 1 : -1;
            if ((currentX - 1 == kingX || currentX + 1 == kingX) && (currentZ + direction == kingZ))
            {
                return true;
            }
            break;
        }
        case 'n':
        {
            int moveOffsets[8][2] = {
                {2, 1}, {2, -1}, {-2, 1}, {-2, -1}, {1, 2}, {1, -2}, {-1, 2}, {-1, -2}};

            for (int i = 0; i < 8; i++)
            {
                if (currentX + moveOffsets[i][0] == kingX && currentZ + moveOffsets[i][1] == kingZ)
                    return true;
            }
            break;
        }
        case 'k':
        {
            int dx = currentX - kingX;
            int dz = currentZ - kingZ;
            if (dx >= -1 && dx <= 1 && dz >= -1 && dz <= 1)
                return true;
            break;
        }
        case 'r':
        case 'b':
        case 'q':
        {
            int directions[8][2] = {
                {1, 0}, {-1, 0}, {0, 1}, {0, -1}, {1, 1}, {1, -1}, {-1, 1}, {-1, -1}};

            int start = 0;
            int end = 8;
            if (opponentPiece.type == 'r')
                end = 4;
            else if (opponentPiece.type == 'b')
                start = 4;

            for (int i = start; i < end; i++)
            {
                for (int step = 1; step < 8; step++)
                {
                    int targetX = currentX + step * directions[i][0];
                    int targetZ = currentZ + step * directions[i][1];

                    if (targetX < 0 || targetX >= 8 || targetZ < 0 || targetZ >= 8)
                        break;

                    if (targetX == kingX && targetZ == kingZ)
                        return true;

                    if (game->chessBoardSquares[targetX][targetZ].isOccupied)
                        break;
                }
            }
            break;
        }
        default:
            break;
        }
    }
    return false; // King is not in check
}

bool Piece::IsInCheckmate()
{
    if (!IsInCheck())
        return false; // Not checkmate if not in check
    if (FindValidKingMoves(0))
        return false; // Not checkmate if the king has valid moves
    return true;      // Checkmate if the king is in check and has no valid moves
}

bool Piece::IsInStalemate()
{
    if (IsInCheck())
        return false; // Not stalemate if in check
    if (FindValidKingMoves(0))
        return false; // Not stalemate if the king has valid moves

    return true; // Stalemate if the king has no valid moves and is not in check
}

bool Piece::PutsKingInCheck(const Vector3 &targetPosition)
{
    if (king == nullptr || king->game == nullptr)
        return false; // Should never happen, but just in case
    Vector3 originalPosition = position;
    position = targetPosition;
    bool putsInCheck = king->IsInCheck();
    position = originalPosition; // Revert to original position
    return putsInCheck;
}

void Piece::MoveTo()
{
    Turn pieceTurn = (std::string(color) == "white") ? Turn::White : Turn::Black;
    if (pieceTurn != *turnOrder || game->selectedPiece != this)
        return;

    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
    {
        Ray mouseRay = GetScreenToWorldRay(GetMousePosition(), *camera);

        BoardSquare *targetSquare = nullptr;
        float nearestHitDistance = 1000000.0f;

        for (int i = 0; i < 8; i++)
        {
            for (int j = 0; j < 8; j++)
            {
                BoardSquare &square = game->chessBoardSquares[i][j];
                Matrix squareTransform = MatrixTranslate(square.position.x, square.position.y, square.position.z);
                RayCollision squareCollision = GetRayCollisionMesh(mouseRay, square.mesh, squareTransform);

                if (squareCollision.hit && squareCollision.distance < nearestHitDistance)
                {
                    nearestHitDistance = squareCollision.distance;
                    targetSquare = &square;
                }
            }
        }

        if (targetSquare == nullptr)
            return;

        TraceLog(LOG_INFO, "Target cell selected: %c%c", targetSquare->name[0], targetSquare->name[1]);

        if (!IsValidMove(targetSquare->position))
        {
            TraceLog(LOG_INFO, "Target %c%c is not a valid move for %s", targetSquare->name[0], targetSquare->name[1], name.c_str());
            return;
        }
        BoardSquare *currentSquare = game->GetSquareAtPosition(position);
        if (currentSquare == nullptr)
        {
            TraceLog(LOG_ERROR, "Current square not found for piece %s at position (%f, %f, %f)", name.c_str(), position.x, position.y, position.z);
            return;
        }

        // COPY THE STRINGS, DON'T STORE POINTERS
        strcpy_s(pieceLastNetMove.fromSquare, sizeof(pieceLastNetMove.fromSquare), currentSquare->name);
        strcpy_s(pieceLastNetMove.toSquare, sizeof(pieceLastNetMove.toSquare), targetSquare->name);

        position = targetSquare->position;
        hasMoved = true;
        game->OccupyCells();
        moved = true;
        selected = false;
        game->selectedPiece = nullptr;
        game->turnOrder = (game->turnOrder == Turn::White) ? Turn::Black : Turn::White;
        TraceLog(LOG_INFO, "%s moved to %c%c", name.c_str(), targetSquare->name[0], targetSquare->name[1]);
    }
}

bool Piece::IsValidMove(const Vector3 &targetPosition)
{
    for (int i = 0; i < validMoveCount; i++)
    {
        if (std::fabs(validMoves[i].x - targetPosition.x) < SQUARE_EPS &&
            std::fabs(validMoves[i].y - targetPosition.y) < SQUARE_EPS &&
            std::fabs(validMoves[i].z - targetPosition.z) < SQUARE_EPS)
        {
            return true;
        }
    }
    return false;
}