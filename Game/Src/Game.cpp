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
    if (network.Init())
        TraceLog(LOG_INFO, "Network initialized succesfully.");
    OccupyCells();
}

Game::~Game()
{
    UnloadModels();
    CloseWindow();
}

void Game::Update()
{
    FlipCamera();

    switch (gameState)
    {
    case MENU:
    {
        menu.Update();
        currentTheme = menu.GetTheme();
        if (currentTheme != lastTheme)
        {
            lastTheme = currentTheme;
            InitWhitePieces();
            InitBlackPieces();
        }
        if (menu.host.Clicked())
        {
            network.Host();
            if (gameState != HOSTING)
            {
                gameState = HOSTING;
                TraceLog(LOG_INFO, "Match hosted");
            }
        }

        if (menu.join.Clicked())
        {
            network.Connect(ipToConnectTo);
            if (network.IsConnected() == false)
            {
                throw ERROR_CONNECTION_INVALID;
                return;
            }
            if (gameState != CONNECTING)
            {
                gameState = CONNECTING;
                TraceLog(LOG_INFO, "Connected to ip: %s", ipToConnectTo);
            }
        }
    }
    break;
    case IN_MATCH:
    {
        for (auto &piece : whitePieces)
        {
            piece.second.Update();
        }
        for (auto &piece : blackPieces)
        {
            piece.second.Update();
        }
    }
    break;

    default:
        break;
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
    case HOSTING:
    {
    }
    break;
    case CONNECTING:
    {
    }
    break;
    case IN_MATCH:
    {
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

            chessBoardSquares[i][j].name[0] = 'A' + i;
            chessBoardSquares[i][j].name[1] = '1' + j;
            chessBoardSquares[i][j].position = squarePosition;
            TraceLog(LOG_INFO, "Cell %c%c created", chessBoardSquares[i][j].name[0], chessBoardSquares[i][j].name[1]);
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
            blackPieces.emplace(pieceNames[i], Piece(&turnOrder, "black", "black-" + pieceNames[i], chessBoardSquares[i][7].position, blackPiecesModels[pieceindex], camera, *this));
            continue;
        }
        blackPieces.emplace(pieceNames[i] + "_1", Piece(&turnOrder, "black", "black-" + pieceNames[i], chessBoardSquares[i][7].position, blackPiecesModels[pieceindex], camera, *this));
        blackPieces.emplace(pieceNames[i] + "_2", Piece(&turnOrder, "black", "black-" + pieceNames[i], chessBoardSquares[7 - i][7].position, blackPiecesModels[pieceindex], camera, *this));
    }
    for (int i = 0; i < 8; i++)
    {
        pieceindex = (currentTheme.find("black") != std::string::npos ? darkColors[0] : darkColors[1]) + "-pawn";
        blackPieces.emplace("pawn_" + std::to_string(i + 1), Piece(&turnOrder, "black", "black-pawn", chessBoardSquares[i][6].position, blackPiecesModels[pieceindex], camera, *this));
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
            whitePieces.emplace(pieceNames[i], Piece(&turnOrder, "white", "white-" + pieceNames[i], chessBoardSquares[i][0].position, whitePiecesModels[pieceindex], camera, *this));
            continue;
        }
        whitePieces.emplace(pieceNames[i] + "_1", Piece(&turnOrder, "white", "white-" + pieceNames[i], chessBoardSquares[i][0].position, whitePiecesModels[pieceindex], camera, *this));
        whitePieces.emplace(pieceNames[i] + "_2", Piece(&turnOrder, "white", "white-" + pieceNames[i], chessBoardSquares[7 - i][0].position, whitePiecesModels[pieceindex], camera, *this));
    }
    for (int i = 0; i < 8; i++)
    {
        pieceindex = (currentTheme.find("white") != std::string::npos ? lightColors[0] : lightColors[1]) + "-pawn";
        whitePieces.emplace("pawn_" + std::to_string(i + 1), Piece(&turnOrder, "white", "white-pawn", chessBoardSquares[i][1].position, whitePiecesModels[pieceindex], camera, *this));
    }
}

void Game::OccupyCells()
{
    for (auto &piece : whitePieces)
    {
        for (int i = 0; i < 8; i++)
        {
            for (int j = 0; j < 8; j++)
            {
                if (piece.second.position == chessBoardSquares[i][j].position)
                    chessBoardSquares[i][j].occupyingPiece = &piece.second;
            }
        }
    }
    for (auto &piece : blackPieces)
    {
        for (int i = 0; i < 8; i++)
        {
            for (int j = 0; j < 8; j++)
            {
                if (piece.second.position == chessBoardSquares[i][j].position)
                    chessBoardSquares[i][j].occupyingPiece = &piece.second;
            }
        }
    }
}
