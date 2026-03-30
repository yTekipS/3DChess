#include "../Headers/Game.hpp"
#include <cmath>

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
    switch (gameState)
    {
    case MENU:
    {
        menu.Update();
        currentTheme = menu.GetTheme();

        if (currentTheme != lastTheme)
        {
            lastTheme = currentTheme;
            InitPieces();
        }

        if (menu.host.Clicked())
        {
            gameState = HOSTING;
        }

        if (menu.join.Clicked())
        {
            ipToConnectTo = menu.GetIp();
            TraceLog(LOG_INFO, "Trying to connect to: %s", ipToConnectTo.c_str());

            if (!network.Connect(ipToConnectTo))
            {
                TraceLog(LOG_WARNING, "Connection failed to %s", ipToConnectTo.c_str());
            }
            else
            {
                gameState = CONNECTING;
            }
        }
        break;
    }

    case HOSTING:
    {
        if (network.Host())
        {
            playersTurn = GetRandomValue(0, 1) == 0 ? Turn::White : Turn::Black;
            network.SendTurn(playersTurn == Turn::White ? Turn::Black : Turn::White);
            camera.position = {0.0f, 23.5f, 10.0f};
            gameState = IN_MATCH;

            if (playersTurn == Turn::Black)
                FlipCamera();

            TraceLog(LOG_INFO, "Host accepted opponent. Starting match.");
            TraceLog(LOG_INFO, "My turn: %s", playersTurn == Turn::White ? "White" : "Black");
        }
        break;
    }

    case CONNECTING:
    {
        if (network.IsConnected() && network.RecvTurn(playersTurn))
        {
            gameState = IN_MATCH;
            camera.position = {0.0f, 23.5f, 10.0f};
            if (playersTurn == Turn::Black)
                FlipCamera();

            TraceLog(LOG_INFO, "Client connected. Turn received. Starting match.");
            TraceLog(LOG_INFO, "My turn: %s", playersTurn == Turn::White ? "White" : "Black");
        }
        break;
    }

    case IN_MATCH:
    {
        for (auto &piece : whitePieces)
        {
            piece.second.Update();
            if (piece.second.moved)
            {
                NetMove moveToSend{};
                moveToSend.fromSquare = piece.second.pieceLastNetMove.fromSquare;
                moveToSend.toSquare = piece.second.pieceLastNetMove.toSquare;

                if (network.SendMove(moveToSend))
                {
                    TraceLog(LOG_INFO, "Sent move: %s -> %s", moveToSend.fromSquare, moveToSend.toSquare);
                    piece.second.moved = false;  // Reset moved flag after sending
                }
                else
                {
                    TraceLog(LOG_WARNING, "Failed to send move: %s -> %s", moveToSend.fromSquare, moveToSend.toSquare);
                }
            }
        }

        for (auto &piece : blackPieces)
        {
            piece.second.Update();
            if (piece.second.moved)
            {
                NetMove moveToSend{};
                moveToSend.fromSquare = piece.second.pieceLastNetMove.fromSquare;
                moveToSend.toSquare = piece.second.pieceLastNetMove.toSquare;

                if (network.SendMove(moveToSend))
                {
                    TraceLog(LOG_INFO, "Sent move: %s -> %s", moveToSend.fromSquare, moveToSend.toSquare);
                    piece.second.moved = false;  // Reset moved flag after sending
                }
                else
                {
                    TraceLog(LOG_WARNING, "Failed to send move: %s -> %s", moveToSend.fromSquare, moveToSend.toSquare);
                }
            }
        }

        if (network.RecvMove(lastReceivedMove))
        {
            TraceLog(LOG_INFO, "Received move: %s -> %s", lastReceivedMove.fromSquare, lastReceivedMove.toSquare);

            for (int i = 0; i < 8; ++i)
            {
                for (int j = 0; j < 8; ++j)
                {
                    BoardSquare &sq = chessBoardSquares[i][j];
                    if (sq.name[0] == lastReceivedMove.fromSquare[0] &&
                        sq.name[1] == lastReceivedMove.fromSquare[1] &&
                        sq.occupyingPiece != nullptr)
                    {
                        BoardSquare *to = GetSquareByName(lastReceivedMove.toSquare);
                        if (to != nullptr)
                        {
                            sq.occupyingPiece->position = to->position;
                            sq.occupyingPiece->moved = true;
                        }
                        break;
                    }
                }
            }
        }

        break;
    }

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
        break;
    }
    case HOSTING:
    {

        break;
    }
    case CONNECTING:
    {
        break;
    }
    case IN_MATCH:
    {
        DrawText(TextFormat("Looking at: (%f, %f, %f)", lookingAt.x, lookingAt.y, lookingAt.z), 10, 10, 20, RED);
        break;
    }
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
            chessBoardSquares[i][j].name[2] = '\0';
            chessBoardSquares[i][j].position = squarePosition;
            chessBoardSquares[i][j].mesh = GenMeshCube(squareSize, 0.2f, squareSize);
            TraceLog(LOG_INFO, "Cell %c%c created", chessBoardSquares[i][j].name[0], chessBoardSquares[i][j].name[1]);
        }
    }
}

void Game::FlipCamera()
{

    camera.position.z *= -1.0f;
}

void Game::InitPieces()
{
    InitWhitePieces();
    InitBlackPieces();
    for (auto &piece : whitePieces)
    {
        piece.second.GetKingReference();
    }
    for (auto &piece : blackPieces)
    {
        piece.second.GetKingReference();
    }
    OccupyCells();
}

void Game::InitBlackPieces()
{
    blackPieces.clear();
    std::string darkColors[] = {"black", "brown"};
    std::string pieceNames[] = {"rook", "knight", "bishop", "queen", "king"};
    char type[] = {'r', 'n', 'b', 'q', 'k'};
    std::string pieceindex;

    for (int i = 0; i < 5; i++)
    {
        pieceindex = (currentTheme.find("black") != std::string::npos ? darkColors[0] : darkColors[1]) + "-" + pieceNames[i];
        if (pieceNames[i] == "queen" || pieceNames[i] == "king")
        {
            blackPieces.emplace(pieceNames[i], Piece(&turnOrder, "black", "black-" + pieceNames[i], chessBoardSquares[i][7].position, type[i], blackPiecesModels[pieceindex], camera, *this));
            continue;
        }
        blackPieces.emplace(pieceNames[i] + "_1", Piece(&turnOrder, "black", "black-" + pieceNames[i], chessBoardSquares[i][7].position, type[i], blackPiecesModels[pieceindex], camera, *this));
        blackPieces.emplace(pieceNames[i] + "_2", Piece(&turnOrder, "black", "black-" + pieceNames[i], chessBoardSquares[7 - i][7].position, type[i], blackPiecesModels[pieceindex], camera, *this));
    }
    for (int i = 0; i < 8; i++)
    {
        pieceindex = (currentTheme.find("black") != std::string::npos ? darkColors[0] : darkColors[1]) + "-pawn";
        blackPieces.emplace("pawn_" + std::to_string(i + 1), Piece(&turnOrder, "black", "black-pawn", chessBoardSquares[i][6].position, 'p', blackPiecesModels[pieceindex], camera, *this));
    }
}

void Game::InitWhitePieces()
{
    whitePieces.clear();
    std::string lightColors[] = {"white", "tan"};
    std::string pieceNames[] = {"rook", "knight", "bishop", "queen", "king"};
    char type[] = {'r', 'n', 'b', 'q', 'k'};
    std::string pieceindex;

    for (int i = 0; i < 5; i++)
    {
        pieceindex = (currentTheme.find("white") != std::string::npos ? lightColors[0] : lightColors[1]) + "-" + pieceNames[i];
        if (pieceNames[i] == "queen" || pieceNames[i] == "king")
        {
            whitePieces.emplace(pieceNames[i], Piece(&turnOrder, "white", "white-" + pieceNames[i], chessBoardSquares[i][0].position, type[i], whitePiecesModels[pieceindex], camera, *this));
            continue;
        }
        whitePieces.emplace(pieceNames[i] + "_1", Piece(&turnOrder, "white", "white-" + pieceNames[i], chessBoardSquares[i][0].position, type[i], whitePiecesModels[pieceindex], camera, *this));
        whitePieces.emplace(pieceNames[i] + "_2", Piece(&turnOrder, "white", "white-" + pieceNames[i], chessBoardSquares[7 - i][0].position, type[i], whitePiecesModels[pieceindex], camera, *this));
    }
    for (int i = 0; i < 8; i++)
    {
        pieceindex = (currentTheme.find("white") != std::string::npos ? lightColors[0] : lightColors[1]) + "-pawn";
        whitePieces.emplace("pawn_" + std::to_string(i + 1), Piece(&turnOrder, "white", "white-pawn", chessBoardSquares[i][1].position, 'p', whitePiecesModels[pieceindex], camera, *this));
    }
}

void Game::OccupyCells()
{
    for (int i = 0; i < 8; i++)
    {
        for (int j = 0; j < 8; j++)
        {
            chessBoardSquares[i][j].occupyingPiece = nullptr;
            chessBoardSquares[i][j].isOccupied = false;
        }
    }

    for (auto &piece : whitePieces)
    {
        for (int i = 0; i < 8; i++)
        {
            for (int j = 0; j < 8; j++)
            {
                if (SameSquarePos(piece.second.position, chessBoardSquares[i][j].position))
                {
                    chessBoardSquares[i][j].occupyingPiece = &piece.second;
                    chessBoardSquares[i][j].isOccupied = true;
                }
            }
        }
    }

    for (auto &piece : blackPieces)
    {
        for (int i = 0; i < 8; i++)
        {
            for (int j = 0; j < 8; j++)
            {
                if (SameSquarePos(piece.second.position, chessBoardSquares[i][j].position))
                {
                    chessBoardSquares[i][j].occupyingPiece = &piece.second;
                    chessBoardSquares[i][j].isOccupied = true;
                }
            }
        }
    }
}

BoardSquare *Game::GetSquareAtPosition(const Vector3 &position)
{
    for (int i = 0; i < 8; i++)
    {
        for (int j = 0; j < 8; j++)
        {
            if (SameSquarePos(position, chessBoardSquares[i][j].position))
            {
                return &chessBoardSquares[i][j];
            }
        }
    }
    return nullptr;
}

BoardSquare *Game::GetSquareByName(char *name)
{
    for (int i = 0; i < 8; i++)
    {
        for (int j = 0; j < 8; j++)
        {
            if (chessBoardSquares[i][j].name[0] == name[0] && chessBoardSquares[i][j].name[1] == name[1])
            {
                return &chessBoardSquares[i][j];
            }
        }
    }
    return nullptr; // No square found with the given name;
}

bool Game::SameSquarePos(const Vector3 &a, const Vector3 &b)
{
    return std::fabs(a.x - b.x) < SQUARE_EPS &&
           std::fabs(a.z - b.z) < SQUARE_EPS;
}