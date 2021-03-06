#ifndef MAIN_PLAYER_H
#define MAIN_PLAYER_H

#include "Image.h"
#include <vector>


typedef std::vector<std::vector<char>> textt;

struct Point
{
  int x;
  int y;
};

enum class MovementDir
{
  UP,
  DOWN,
  LEFT,
  RIGHT
};


struct Hall { 

  Hall *up = NULL;
  Hall *down = NULL;
  Hall *left = NULL;
  Hall *right = NULL;

  char type;
  std::string path; // путь к спрайту
  int tile_bias_x;  // смещение тайла относительно картинки с тайлами
  int tile_bias_y;

  bool door[4] = {true, true, true, true}; // true - дверь открыта (фактически просто проход), false - закрытая дверь

};


struct Player
{
  explicit Player(Point pos = {.x = 10, .y = 10}) :
                 coords(pos), old_coords(coords) {};
  Point getCoords() {return coords;}
  bool Moved() const;
  void ProcessInput(MovementDir dir);
  void Draw(Image &screen, Image &background, Hall *room);
  bool isWall(MovementDir dir);
  int isDoor();
  bool isQuit();
  void set_coords(int i, int j) {old_coords = coords = {i, j};}
  void cum_time_inc(float delta) {
    cum_time += delta;
    if (cum_time > 0.45) cum_time = 0;
  }
  std::vector<int> get_door_coords_x() {return door_coords_x;}
  std::vector<int> get_door_coords_y() {return door_coords_y;}
  std::vector<int> get_prize_coords_x() {return prize_coords_x;}
  std::vector<int> get_prize_coords_y() {return prize_coords_x;}
  std::vector<std::vector<int>> save_room(textt txt);
  std::vector<std::vector<int>> getPrizeCoords();
  bool isPrize();
  bool isEmpty();

  




private:
  Point coords {.x = 10, .y = 10};
  Point old_coords {.x = 10, .y = 10};
  Pixel color {.r = 255, .g = 255, .b = 0, .a = 255};
  int move_speed = 4;
  textt map;
  float cum_time = 0;
  std::vector<int> door_coords_x;
  std::vector<int> door_coords_y;
  std::vector<int> quit_coords_x;
  std::vector<int> quit_coords_y;
  std::vector<int> prize_coords_x;
  std::vector<int> prize_coords_y;
  std::vector<int> empty_coords_x;
  std::vector<int> empty_coords_y;
};





#endif //MAIN_PLAYER_H
