#ifndef MENU_HPP
#define MENU_HPP
#pragma once

#include <raylib.h>
#include <string>

#define MAX_INPUT_CHARS 15
#define THEMES_AMOUNT 4

class Button
{
public:
    Rectangle btnRec = {0};
    Color btnColor;
    const char *btnText;

    Button() {}
    Button(Rectangle rec, Color color, const char *text = nullptr);
    bool Clicked();
};

class Menu
{
private:
    Vector2 screenSize = {0};
    Rectangle sidePanel = {0};
    int themeIndex = 0;
    const char *themes[THEMES_AMOUNT] = {"black-white", "black-tan", "brown-white", "brown-tan"};
    Button shifThemeLeft;
    Button shifThemeRight;
    Rectangle themeRec;
    Font font;
    Color darkGray = Color{200, 200, 200, 255};
    Button ipInputBox;
    char ip[MAX_INPUT_CHARS] = "192.168.0.0";
    int letterCounter =0;
    bool typing = false;

public:
    Button host;
    Button join;

    Menu() {}
    void Draw();
    void Update();
    void Init();
    std::string GetTheme();
    void TypeInIp();
    std::string GetIp();
    void DrawTextCentered(Font font, const char *text, Vector2 pos, float size, float spacing, Color color);
};

#endif // MENU_HPP