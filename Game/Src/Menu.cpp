#include "../Headers/Menu.hpp"

Button::Button(Rectangle rec, Color color, const char *text) : btnRec(rec), btnText(text), btnColor(color)
{
}

bool Button::Clicked()
{
    if (CheckCollisionPointRec(GetMousePosition(), btnRec) && IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
    {
        return true;
    }
    return false;
}

void Menu::Update()
{
    if (shifThemeLeft.Clicked() && themeIndex > 0)
        themeIndex--;
    else if (shifThemeLeft.Clicked() && themeIndex == 0)
        themeIndex = THEMES_AMOUNT - 1;
    if (shifThemeRight.Clicked() && themeIndex < THEMES_AMOUNT - 1)
        themeIndex++;
    else if (shifThemeRight.Clicked() && themeIndex == THEMES_AMOUNT - 1)
        themeIndex = 0;
    if (ipInputBox.Clicked() && !typing)
    {
        TraceLog(LOG_INFO, "Started typing");
        typing = true;
    }
    else if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON) && typing)
    {
        TraceLog(LOG_INFO, "Ended typing");

        typing = false;
    }
    if (typing)
    {
        TypeInIp();
    }
}

void Menu::Draw()
{
    DrawRectangleRec(sidePanel, Color{220, 220, 220, 255});
    DrawTextCentered(font, "3D Chess", {sidePanel.x + sidePanel.width / 2, sidePanel.y + 50}, 50, 5, BLACK);
    DrawTextCentered(font, "Theme", {sidePanel.x + sidePanel.width / 2, sidePanel.y + 150}, 50, 5, BLACK);
    DrawRectangleRec(themeRec, darkGray);
    DrawTextCentered(font, themes[themeIndex], {themeRec.x + themeRec.width / 2, themeRec.y + themeRec.height / 2}, 20, 5, BLACK);
    DrawRectangleRec(shifThemeLeft.btnRec, shifThemeLeft.btnColor);
    DrawTextCentered(font, shifThemeLeft.btnText, {shifThemeLeft.btnRec.x + shifThemeLeft.btnRec.width / 2, shifThemeLeft.btnRec.y + shifThemeLeft.btnRec.height / 2}, 35, 5, BLACK);
    DrawRectangleRec(shifThemeRight.btnRec, shifThemeRight.btnColor);
    DrawTextCentered(font, shifThemeRight.btnText, {shifThemeRight.btnRec.x + shifThemeRight.btnRec.width / 2, shifThemeRight.btnRec.y + shifThemeRight.btnRec.height / 2}, 35, 5, BLACK);
    DrawRectangleRec(host.btnRec, host.btnColor);
    DrawTextCentered(font, host.btnText, {host.btnRec.x + host.btnRec.width / 2, host.btnRec.y + host.btnRec.height / 2}, 30, 5, BLACK);
    DrawRectangleRec(ipInputBox.btnRec, darkGray);
    DrawTextCentered(font, ipInputBox.btnText, {ipInputBox.btnRec.x + ipInputBox.btnRec.width / 2, ipInputBox.btnRec.y + ipInputBox.btnRec.height / 2}, 30, 5, BLACK);
    DrawRectangleRec(join.btnRec, join.btnColor);
    DrawTextCentered(font, join.btnText, {join.btnRec.x + join.btnRec.width / 2, join.btnRec.y + join.btnRec.height / 2}, 30, 5, BLACK);
}

void Menu::Init()
{
    screenSize = {static_cast<float>(GetScreenWidth()), static_cast<float>(GetScreenHeight())};
    sidePanel.x = (screenSize.x * 3) / 4;
    sidePanel.y = 0.0f;
    sidePanel.width = screenSize.x - sidePanel.x;
    sidePanel.height = screenSize.y;
    themeRec = {sidePanel.x + sidePanel.width / 2 - 75, sidePanel.y + 200, 150, 50};
    font = GetFontDefault();
    shifThemeLeft = Button(Rectangle{themeRec.x - 60, themeRec.y, 50, 50}, darkGray, "<");
    shifThemeRight = Button(Rectangle{themeRec.x + themeRec.width + 10, themeRec.y, 50, 50}, darkGray, ">");
    host = Button(Rectangle{sidePanel.x + sidePanel.width / 2 - 120 / 2, sidePanel.height / 2, 120, 50}, darkGray, "HOST");
    join = Button(Rectangle{sidePanel.x + sidePanel.width / 2 - 120 / 2, sidePanel.height / 2 + 170, 120, 50}, darkGray, "JOIN");
    ipInputBox = Button(Rectangle{sidePanel.x + sidePanel.width / 2 - 120, sidePanel.height / 2 + 100, 240, 50}, darkGray, ip);
    letterCounter = TextLength(ip);
    TraceLog(LOG_INFO, "Menu initialized");
}

std::string Menu::GetTheme()
{
    return themes[themeIndex];
}

void Menu::TypeInIp()
{
    int keyPressed = GetCharPressed();

    while (keyPressed > 0)
    {
        if ((keyPressed == '.' || (keyPressed >= '0' && keyPressed <= '9')) && letterCounter < MAX_INPUT_CHARS - 1)
        {
            ip[letterCounter] = (char)keyPressed;
            ip[letterCounter + 1] = '\0';
            letterCounter++;
        }
        keyPressed = GetCharPressed();
    }

    if (IsKeyPressed(KEY_BACKSPACE) && letterCounter > 0)
    {
        letterCounter--;
        ip[letterCounter] = '\0';
    }
}

std::string Menu::GetIp()
{
    return ip;
}

void Menu::DrawTextCentered(Font font, const char *text, Vector2 pos, float size, float spacing, Color color)
{
    Vector2 textSize = MeasureTextEx(font, text, size, spacing);
    Vector2 drawPos = {pos.x - textSize.x / 2.0f, pos.y - textSize.y / 2.0f};
    DrawTextEx(font, text, drawPos, size, spacing, color);
}