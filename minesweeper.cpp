
#include <array>
#include <cmath>
#include <ctime>
#include <iostream>
#include <raylib.h>
#include <tuple>

const int TILE_SIZE = 80;
const int SCREEN_WIDTH_TILE = 10;
const int SCREEN_HEIGHT_TILE = 8;

const int SCREEN_WIDTH = TILE_SIZE * SCREEN_WIDTH_TILE;
const int SCREEN_HEIGHT = TILE_SIZE * SCREEN_HEIGHT_TILE;

Color COLOR_GROUND_1 = Color{230, 195, 160, 255};
Color COLOR_GROUND_2 = Color{215, 184, 153, 255};

Color COLOR_GRASS_1 = {170, 215, 82, 255};
Color COLOR_GRASS_2 = {162, 209, 74, 255};

bool gameOver = false;
bool firstClick = false;

bool gameWon = false;

int flagsPlaced = 0;

int mineAmount = 10;

std::array<std::tuple<int, int>, 8> transforms{
    {{-1, -1}, {0, -1}, {1, -1}, {-1, 0}, {1, 0}, {-1, 1}, {0, 1}, {1, 1}}};

bool IsEven(int number) { return number % 2 == 0; }

bool IsOdd(int number) { return number % 2 == 1; }

class Tile {
public:
  bool isCovered = true;
  int value = 0;
  int x = 0;
  int y = 0;
  bool flagged = false;
};

using Board =
    std::array<std::array<Tile, SCREEN_WIDTH_TILE>, SCREEN_HEIGHT_TILE>;
using Coords = std::tuple<int, int>;

Board tiles;

void DrawTiles(Board board, Texture2D flag) {
  Color drawColor;
  std::string drawText = "";
  for (int i = 0; i < SCREEN_WIDTH_TILE; i++) {
    for (int j = 0; j < SCREEN_HEIGHT_TILE; j++) {

      // drawColor
      switch (board[j][i].value) {
      case -1:
        drawText = "@";
        break;
      case 0:
        drawText = "";
        break;
      default:
        drawText = std::to_string(board[j][i].value);
      }

      if (board[j][i].isCovered) {
        if (IsEven(i + j)) {
          drawColor = COLOR_GRASS_1;
          drawText = "";
        }
        else {
          drawColor = COLOR_GRASS_2;
          drawText = "";
        }
      }
      else {
        if (IsEven(i + j)) {
          drawColor = COLOR_GROUND_1;
        }
        else {
          drawColor = COLOR_GROUND_2;
        }
      }
      DrawRectangle(i * TILE_SIZE, j * TILE_SIZE, TILE_SIZE, TILE_SIZE,
                    drawColor);
      if (board[j][i].flagged) {
        DrawTexturePro(flag,
                       {0.0f, 0.0f, (float)flag.width, (float)flag.height},
                       {(float)i * TILE_SIZE, (float)j * TILE_SIZE,
                        TILE_SIZE * 0.95, TILE_SIZE * 0.95},
                       {0.0f, 0.0f}, 0.0f, WHITE);
      }
      DrawText(drawText.c_str(), i * TILE_SIZE + TILE_SIZE / 4,
               j * TILE_SIZE + TILE_SIZE / 4, TILE_SIZE / 2, DARKGRAY);
    }
  }
}

Coords GetMouseCoords() {
  int x = GetMouseX();
  int y = GetMouseY();
  int grid_x = std::floor(x / TILE_SIZE);
  int grid_y = std::floor(y / TILE_SIZE);
  return {grid_x, grid_y};
}

void DrawMouseHover() {
  Coords coords = GetMouseCoords();
  if (std::get<0>(coords) >= 0 && std::get<0>(coords) < SCREEN_WIDTH_TILE &&
      std::get<1>(coords) >= 0 && std::get<1>(coords) < SCREEN_HEIGHT_TILE) {
    DrawRectangle(std::get<0>(coords) * TILE_SIZE,
                  std::get<1>(coords) * TILE_SIZE, TILE_SIZE, TILE_SIZE,
                  Color{255, 255, 255, 100});
  }
}

void PlaceMines(Board &board) {
  int mines = 0;

  while (mines < mineAmount) {
    int random_x = std::rand() % SCREEN_WIDTH_TILE;
    int random_y = std::rand() % SCREEN_HEIGHT_TILE;

    if (board[random_y][random_x].value != -1 &&
        board[random_y][random_x].isCovered) {
      board[random_y][random_x].value = -1;
      mines++;
    }
  }
}

int GetSurroundingTiles(Board &board, int x, int y) {
  int mines = 0;

  for (int i = 0; i < 8; i++) {
    int nx = x + std::get<0>(transforms[i]);
    int ny = y + std::get<1>(transforms[i]);
    if (nx >= 0 && nx < SCREEN_WIDTH_TILE && ny >= 0 &&
        ny < SCREEN_HEIGHT_TILE) {
      if (board[ny][nx].value == -1) {
        mines++;
      }
    }
  }
  return mines;
}

void UpdateTiles(Board &board) {
  for (int i = 0; i < SCREEN_WIDTH_TILE; i++) {
    for (int j = 0; j < SCREEN_HEIGHT_TILE; j++) {
      if (board[j][i].value != -1) {
        board[j][i].value = GetSurroundingTiles(board, i, j);
      }
    }
  }
}

void ExcavateAround(Board &board, int x, int y) {
  if (x < 0 || x >= SCREEN_WIDTH_TILE || y < 0 || y >= SCREEN_HEIGHT_TILE)
    return;
  if (!board[y][x].isCovered)
    return;

  board[y][x].isCovered = false;
  if (board[y][x].flagged) {
    flagsPlaced--;
  }
  board[y][x].flagged = false;

  if (board[y][x].value == 0) {
    for (int i = 0; i < 8; i++) {
      int nx = x + std::get<0>(transforms[i]);
      int ny = y + std::get<1>(transforms[i]);

      ExcavateAround(board, nx, ny);
    }
  }
}

void CheckWin(Board &board) {
  int tilesLeft = 0;
  for (int i = 0; i < SCREEN_WIDTH_TILE; i++) {
    for (int j = 0; j < SCREEN_HEIGHT_TILE; j++) {
      if (board[j][i].isCovered) {
        tilesLeft++;
      }
    }
  }
  if (tilesLeft == mineAmount) {
    gameWon = true;
  }
}

void MouseInput(Board &board) {
  Coords coords = GetMouseCoords();
  int mx = std::get<0>(coords);
  int my = std::get<1>(coords);

  if (mx >= 0 && mx < SCREEN_WIDTH_TILE && my >= 0 && my < SCREEN_HEIGHT_TILE) {
    Tile &tile = board[my][mx];

    if (!firstClick) {
      firstClick = true;
      tile.isCovered = false;
      tile.isCovered = true;
    }

    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
      if (tile.isCovered) {
        if (tile.value == -1) {
          tile.isCovered = false;
          tile.flagged = false;
          gameOver = true;
        }
        else if (tile.value == 0) {
          ExcavateAround(board, mx, my);
        }
        else {
          tile.isCovered = false;
          tile.flagged = false;
        }
        CheckWin(board);
      }
    }
    else if (IsMouseButtonPressed(MOUSE_BUTTON_RIGHT)) {
      if (tile.isCovered) {
        if (tile.flagged) {
          tile.flagged = false;
          flagsPlaced--;
        }
        else {
          tile.flagged = true;
          flagsPlaced++;
        }
      }
    }
  }
}

void DrawBottomBar() {
  Color textColor = WHITE;
  if (flagsPlaced > mineAmount) {
    textColor = RED;
  }
  std::string text_str = std::to_string(flagsPlaced) + " / " + "10";
  const char *text = text_str.c_str();
  DrawRectangle(0, SCREEN_HEIGHT, SCREEN_WIDTH, TILE_SIZE, DARKGREEN);
  DrawText(text, 20, SCREEN_HEIGHT + 20, 40, textColor);
}

void ResetGame(Board &board) {
  for (int j = 0; j < SCREEN_HEIGHT_TILE; j++) {
    for (int i = 0; i < SCREEN_WIDTH_TILE; i++) {
      tiles[j][i].isCovered = true;
      tiles[j][i].value = 0;
      tiles[j][i].x = i;
      tiles[j][i].y = j;
      tiles[j][i].flagged = false;
    }
  }
  gameOver = false;
  gameWon = false;
  flagsPlaced = 0;
  PlaceMines(board);
  UpdateTiles(board);
}

void GameOverMouse(Board &board) {
  if (IsKeyPressed(KEY_R)) {
    ResetGame(board);
  }
}

int main() {
  InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT + TILE_SIZE, "Minesweeper");
  Texture2D flag = LoadTexture("resources/flag.png");
  SetTargetFPS(60);
  std::srand(std::time(0));

  ResetGame(tiles);

  while (WindowShouldClose() == false) {
    if (!gameOver && !gameWon) {
      MouseInput(tiles);
    }
    BeginDrawing();
    ClearBackground(RAYWHITE);

    DrawTiles(tiles, flag);
    DrawBottomBar();

    if (!gameOver && !gameWon) {
      DrawMouseHover();
    }
    else if (gameWon) {
      const char *text = "You Win";
      const char *text2 = "Press 'R' To Play Again";
      DrawText(text, (SCREEN_WIDTH / 2) - (MeasureText(text, 80) / 2),
               SCREEN_HEIGHT / 2 - 40, 80, RED);
      DrawText(text2, (SCREEN_WIDTH / 2) - (MeasureText(text2, 40) / 2),
               SCREEN_HEIGHT / 2 + 40, 40, RED);
      GameOverMouse(tiles);
    }
    else {
      const char *text = "Game Over";
      const char *text2 = "Press 'R' To Retry";
      DrawText(text, (SCREEN_WIDTH / 2) - (MeasureText(text, 80) / 2),
               SCREEN_HEIGHT / 2 - 40, 80, RED);
      DrawText(text2, (SCREEN_WIDTH / 2) - (MeasureText(text2, 40) / 2),
               SCREEN_HEIGHT / 2 + 40, 40, RED);
      GameOverMouse(tiles);
    }

    EndDrawing();
  }

  UnloadTexture(flag);
  CloseWindow();
  return 0;
}
