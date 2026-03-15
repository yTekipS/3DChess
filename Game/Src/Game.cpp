#include "../Headers/Game.hpp"

Game::Game()
{
    InitWindow(screenWidth, screenHeight, "3D Chess");
    SetTargetFPS(60);
    camera.position = {0.0f, 23.5f, 28.5f};
    camera.target = boardOrigin;
    camera.up = {0.0f, 1.0f, 0.0f};
    camera.fovy = 47.0f;
    camera.projection = CAMERA_PERSPECTIVE;
    LoadModels();
    FillChessBoardSquares();
    InitPieces();
    menu.Init();
}

Game::~Game()
{
    UnloadModels();
    CloseWindow();
}

void Game::Update()
{
    FlipCamera();

    if (gameState == MENU)
    {
        menu.Update();
        currentTheme = menu.GetTheme();
        if (currentTheme != lastTheme)
        {
            lastTheme = currentTheme;
            InitWhitePieces();
            InitBlackPieces();
        }
        return;
    }

    for (auto &piece : whitePieces)
    {
        piece.second.Update();
    }
    for (auto &piece : blackPieces)
    {
        piece.second.Update();
    }
}

void Game::Draw()
{
    BeginDrawing();
    BeginMode3D(camera);

    ClearBackground(Color{100, 100, 100, 255});
    DrawModel(chessBoardModels[currentTheme], boardOrigin, 1.0f, WHITE);
    for (auto &piece : whitePieces)
    {
        piece.second.Draw();
    }
    for (auto &piece : blackPieces)
    {
        piece.second.Draw();
    }
    EndMode3D();
    switch (gameState)
    {
    case MENU:
    {
        menu.Draw();
    }
    break;
    default:
        break;
    }
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
    std::string darkColors[] = {"black", "brown"};
    std::string lightColors[] = {"white", "tan"};
    std::string pieces[] = {"pawn", "rook", "knight", "bishop", "queen", "king"};
    for (int i = 0; i < 6; i++)
    {
        blackPiecesModels.emplace(darkColors[0] + "-" + pieces[i], LoadModel((commonPath + "Pieces/black-" + pieces[i] + ".glb").c_str()));
    }
    for (int i = 0; i < 6; i++)
    {
        blackPiecesModels.emplace(darkColors[1] + "-" + pieces[i], LoadModel((commonPath + "Pieces/brown-" + pieces[i] + ".glb").c_str()));
    }
    for (int i = 0; i < 6; i++)
    {
        whitePiecesModels.emplace(lightColors[0] + "-" + pieces[i], LoadModel((commonPath + "Pieces/white-" + pieces[i] + ".glb").c_str()));
    }
    for (int i = 0; i < 6; i++)
    {
        whitePiecesModels.emplace(lightColors[1] + "-" + pieces[i], LoadModel((commonPath + "Pieces/tan-" + pieces[i] + ".glb").c_str()));
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
            const char *squareName = "A" + i + 1 + j;
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
    InitWhitePieces();
    InitBlackPieces();
}

void Game::InitBlackPieces()
{
    blackPieces.clear();
    std::string darkColors[] = {"black", "brown"};
    std::string pieceNames[] = {"rook", "knight", "bishop", "queen", "king"};
    std::string pieceindex;

    for (int i = 0; i < 5; i++)
    {
        pieceindex = (currentTheme.find("black") != std::string::npos ? darkColors[0] : darkColors[1]) + "-" + pieceNames[i];
        if (pieceNames[i] == "queen" || pieceNames[i] == "king")
        {
            blackPieces.emplace(pieceNames[i], Piece(&turnOrder, "black", chessBoardSquares[i][7].position, blackPiecesModels[pieceindex], camera, *this));
            continue;
        }
        blackPieces.emplace(pieceNames[i] + "_1", Piece(&turnOrder, "black", chessBoardSquares[i][7].position, blackPiecesModels[pieceindex], camera, *this));
        blackPieces.emplace(pieceNames[i] + "_2", Piece(&turnOrder, "black", chessBoardSquares[7 - i][7].position, blackPiecesModels[pieceindex], camera, *this));
    }
    for (int i = 0; i < 8; i++)
    {
        pieceindex = (currentTheme.find("black") != std::string::npos ? darkColors[0] : darkColors[1]) + "-pawn";

        blackPieces.emplace("pawn_" + std::to_string(i + 1), Piece(&turnOrder, "black", chessBoardSquares[i][6].position, blackPiecesModels[pieceindex], camera, *this));
    }
}

void Game::InitWhitePieces()
{
    whitePieces.clear();
    std::string lightColors[] = {"white", "tan"};
    std::string pieceNames[] = {"rook", "knight", "bishop", "queen", "king"};
    std::string pieceindex;

    for (int i = 0; i < 5; i++)
    {
        pieceindex = (currentTheme.find("white") != std::string::npos ? lightColors[0] : lightColors[1]) + "-" + pieceNames[i];
        if (pieceNames[i] == "queen" || pieceNames[i] == "king")
        {
            whitePieces.emplace(pieceNames[i], Piece(&turnOrder, "white", chessBoardSquares[i][0].position, whitePiecesModels[pieceindex], camera, *this));
            continue;
        }
        whitePieces.emplace(pieceNames[i] + "_1", Piece(&turnOrder, "white", chessBoardSquares[i][0].position, whitePiecesModels[pieceindex], camera, *this));
        whitePieces.emplace(pieceNames[i] + "_2", Piece(&turnOrder, "white", chessBoardSquares[7 - i][0].position, whitePiecesModels[pieceindex], camera, *this));
    }
    for (int i = 0; i < 8; i++)
    {
        pieceindex = (currentTheme.find("white") != std::string::npos ? lightColors[0] : lightColors[1]) + "-pawn";
        whitePieces.emplace("pawn_" + std::to_string(i + 1), Piece(&turnOrder, "white", chessBoardSquares[i][1].position, whitePiecesModels[pieceindex], camera, *this));
    }
}