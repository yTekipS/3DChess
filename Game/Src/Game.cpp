#include "../Headers/Game.hpp"

Piece::Piece(Turn *turnOrder, const char *color, const Vector3 position, Model &model, Camera3D &camera, Game &game)
    : turnOrder(turnOrder), color(color), position(position), model(&model), camera(&camera), game(&game)
{
}

Piece::~Piece()
{
}

void Piece::Update()
{
    Select();
    Deselect();
}

void Piece::Draw()
{
    DrawModel(*model, position, 1.0f, selected ? RED : WHITE);
}

void Piece::Select()
{
    if (*turnOrder != (color == "white" ? Turn::White : Turn::Black) || game->selectedPiece != nullptr)
        return;
    Ray mouseRay = GetMouseRay(GetMousePosition(), *camera);
    RayCollision collision = GetRayCollisionMesh(mouseRay, model->meshes[0], model->transform);
    if (collision.hit && IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
    {
        selected = true;
        game->selectedPiece = this;
    }
}

void Piece::Deselect()
{

    if (*turnOrder != (color == "white" ? Turn::White : Turn::Black) ||game->selectedPiece != this || game->selectedPiece == nullptr)
        return;
    Ray mouseRay = GetMouseRay(GetMousePosition(), *camera);
    RayCollision collision = GetRayCollisionMesh(mouseRay, model->meshes[0], model->transform);
    if (collision.hit && IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
    {
        selected = false;
        game->selectedPiece = nullptr;
    }
}

Game::Game()
{
    InitWindow(screenWidth, screenHeight, "3D Chess");
    SetTargetFPS(60);
    camera.position = {0.0f, 20.5f, 17.5f};
    camera.target = boardOrigin;
    camera.up = {0.0f, 1.0f, 0.0f};
    camera.fovy = 47.0f;
    camera.projection = CAMERA_PERSPECTIVE;
    LoadModels();
    FillChessBoardSquares();
    InitPieces();
}

Game::~Game()
{
    UnloadModels();
    CloseWindow();
}

void Game::Update()
{
    FlipCamera();
}

void Game::Draw()
{
    BeginDrawing();
    BeginMode3D(camera);

    ClearBackground(RAYWHITE);
    DrawModel(chessBoardModels["brown-tan"], boardOrigin, 1.0f, WHITE);

    EndMode3D();
    EndDrawing();
}

void Game::Run()
{

    while (!WindowShouldClose())
    {
        Update();
        Draw();
    }
}

void Game::LoadModels()
{
    std::string commonPath = "Game/Assets/";
    std::string lightColors[] = {"white", "tan"};
    std::string darkColors[] = {"black", "brown"};
    std::string pieces[] = {"pawn", "rook", "knight", "bishop", "queen", "king"};
    for (int i = 0; i < 6; i++)
    {
        blackPiecesModels.emplace(pieces[i], LoadModel((commonPath + "Pieces/black-" + pieces[i] + ".glb").c_str()));
    }
    for (int i = 0; i < 6; i++)
    {
        whitePiecesModels.emplace(pieces[i], LoadModel((commonPath + "Pieces/white-" + pieces[i] + ".glb").c_str()));
    }
    for (int i = 0; i < 6; i++)
    {
        tanPiecesModels.emplace(pieces[i], LoadModel((commonPath + "Pieces/tan-" + pieces[i] + ".glb").c_str()));
    }
    for (int i = 0; i < 6; i++)
    {
        brownPiecesModels.emplace(pieces[i], LoadModel((commonPath + "Pieces/brown-" + pieces[i] + ".glb").c_str()));
    }
    for (int i = 0; i < 2; i++)
    {
        for (int j = 0; j < 2; j++)
        {
            chessBoardModels.emplace(darkColors[i] + "-" + lightColors[j], LoadModel((commonPath + "Boards/chess-board-" + darkColors[i] + "-" + lightColors[j] + ".gltf").c_str()));
        }
    }
}

void Game::UnloadModels()
{
    for (const auto &model : blackPiecesModels)
    {
        UnloadModel(model.second);
    }
    for (const auto &model : whitePiecesModels)
    {
        UnloadModel(model.second);
    }
    for (const auto &model : tanPiecesModels)
    {
        UnloadModel(model.second);
    }
    for (const auto &model : brownPiecesModels)
    {
        UnloadModel(model.second);
    }
    for (const auto &model : chessBoardModels)
    {
        UnloadModel(model.second);
    }
}

void Game::FillChessBoardSquares()
{
    Vector3 offsetFromBoardOrigin = {-8.4f, 0.7f, 8.7f};

    for (int i = 0; i < 8; i++)
    {
        for (int j = 0; j < 8; j++)
        {
            Vector3 squarePosition = {offsetFromBoardOrigin.x + i * squareSize, offsetFromBoardOrigin.y, offsetFromBoardOrigin.z - j * squareSize};
            std::string squareName = std::string(1, static_cast<char>('A' + i)) + std::to_string(1 + j);
            chessBoardSquares[i][j].name = squareName;
            chessBoardSquares[i][j].position = squarePosition;
        }
    }
}

void Game::FlipCamera()
{
    if (IsKeyPressed(KEY_LEFT) || IsKeyPressed(KEY_RIGHT))
    {
        camera.position.z *= -1.0f;
    }
}

void Game::InitPieces()
{
    // Example of initializing a piece on the board
    piece = Piece(&turnOrder, "white", chessBoardSquares[1][1].position, whitePiecesModels["pawn"], camera, *this);
}