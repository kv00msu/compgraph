#include "Player.h"
#include <iostream>
bool Player::Moved() const
{
  if(coords.x == old_coords.x && coords.y == old_coords.y)
    return false;
  else
    return true;
}

void Player::ProcessInput(MovementDir dir)
{
  int move_dist = move_speed * 1;

  if (!(this->isWall(dir))) {

    switch(dir)
    {
      case MovementDir::UP:
        old_coords.y = coords.y;
        coords.y += move_dist;
        break;
      case MovementDir::DOWN:
        old_coords.y = coords.y;
        coords.y -= move_dist;
        break;
      case MovementDir::LEFT:
        old_coords.x = coords.x;
        coords.x -= move_dist;
        break;
      case MovementDir::RIGHT:
        old_coords.x = coords.x;
        coords.x += move_dist;
        break;
      default:
        break;
    }

  }
}


void Player::Draw(Image &screen, Image &background, Hall *room)
{
  Image run_up("../resources/hero.png");

  if(Moved())
  {
    for(int y = old_coords.y; y <= old_coords.y + tileSize; ++y)
    {
      for(int x = old_coords.x; x <= old_coords.x + tileSize; ++x)
      {
        screen.PutPixel(x, y, background.GetPixel(x, y));
      }
    }

    int bias_x, bias_y;

    if (coords.y < old_coords.y) {       // go down;
      bias_y = 0;
    }
    else if (coords.y > old_coords.y) {  // go up;
      bias_y = 3;
    }
    else if (coords.x < old_coords.x) {  // go left;
      bias_y = 1;
    }
    else if (coords.x > old_coords.x) {  // go right;
      bias_y = 2;
    }
    if (cum_time <= 0.15) bias_x = 0;
    else if (cum_time > 0.15 && cum_time <= 0.3) bias_x = 1;
    else bias_x = 2;

    for(int y = coords.y, m = 0; y <= coords.y + tileSize; ++y, m++)
    {
      for(int x = coords.x, k = 0; x <= coords.x + tileSize; ++x, k++)
      {
        //screen.PutPixel(x, y, color);
        Pixel current_pixel = run_up.GetPixel(bias_x * tileSize + k, (bias_y + 1) * tileSize - m);
        if (current_pixel.a != 0) {
          screen.PutPixel(x, y, current_pixel);
        }
      }
    }

    old_coords = coords;
  }

  else {
    for(int y = coords.y, m = 0; y <= coords.y + tileSize; ++y, m++)
    {
      for(int x = coords.x, k = 0; x <= coords.x + tileSize; ++x, k++)
      {
        Pixel current_pixel = run_up.GetPixel(k, tileSize - m);
        if (current_pixel.a != 0) {
          screen.PutPixel(x, y, current_pixel);
        }
      }
    }
  }


}




bool Player::isWall(MovementDir dir) {
  int move_dist = move_speed * 1;
  if (coords.y + move_dist + tileSize > 800 && dir == MovementDir::UP    ||
      coords.y - move_dist - 1 < 0              && dir == MovementDir::DOWN  ||
      coords.x - move_dist - 1 < 0              && dir == MovementDir::LEFT  ||
      coords.x + move_dist + 1 + tileSize > 800 && dir == MovementDir::RIGHT) {
      
      return true;
  }
  
  switch (dir)
  {
  case MovementDir::UP:
    if (map[(coords.y + tileSize) / tileSize][coords.x / tileSize] == '#' ||
      map[(coords.y + tileSize) / tileSize][(coords.x + tileSize) / tileSize] == '#' && coords.x % tileSize != 0)
      return true;
    break;
  
  case MovementDir::DOWN:
    if (map[(coords.y - move_dist) / tileSize][coords.x / tileSize] == '#' ||
      map[(coords.y - move_dist) / tileSize][(coords.x + tileSize) / tileSize] == '#' && coords.x % tileSize != 0)
      return true;
    break;

  case MovementDir::LEFT:
    if (map[coords.y / tileSize][(coords.x - move_dist) / tileSize] == '#' ||
      map[(coords.y + tileSize) / tileSize][(coords.x - move_dist) / tileSize] == '#' && coords.y % tileSize != 0)
      return true;
    break;

  case MovementDir::RIGHT:
    if (map[coords.y / tileSize][(coords.x + tileSize) / tileSize] == '#' ||
      map[(coords.y + tileSize) / tileSize][(coords.x + tileSize) / tileSize] == '#' && coords.y % tileSize != 0)
      return true;
    break;

  
  default:
    break;
  }



  return false;
}


std::vector<std::vector<int>> Player::save_room(textt txt) { 
                               
  map = txt;
  quit_coords_x.clear();
  quit_coords_y.clear();
  prize_coords_x.clear();
  prize_coords_y.clear();
  empty_coords_x.clear();
  empty_coords_y.clear();
  for (int i = 0; i < map.size(); i++) {
    for (int j = 0; j < map[i].size(); j++) {
      if (map[i][j] == 'X') {
        door_coords_x.push_back(j);
        door_coords_y.push_back(i);
      }

      else if (map[i][j] == 'Q') {  
        quit_coords_x.push_back(j);
        quit_coords_y.push_back(i);
      }
      else if (map[i][j] == 'G') {
        prize_coords_x.push_back(j);
        prize_coords_y.push_back(i);
      }
      else if (map[i][j] == ' ') {
        empty_coords_x.push_back(j);
        empty_coords_y.push_back(i);
      }
    }
  }
  std::vector<std::vector<int>> res;
  res.push_back(door_coords_x);
  res.push_back(door_coords_y);
  return res;
};


int Player::isDoor() { 
  int res;             
  int n = door_coords_x.size();
  for (int i = 0; i < n; i++) {
    
    if (coords.x >= door_coords_x[i] * tileSize - 5 && coords.x <= door_coords_x[i] * tileSize + 5) {
      if (coords.y >= door_coords_y[i] * tileSize - 5 && coords.y <= door_coords_y[i] * tileSize + 5) {
        
        if (door_coords_y[i] > 22) return 1;
        else if (door_coords_y[i] < 2) return 2;
        else if (door_coords_x[i] < 2) return 3;
        else if (door_coords_x[i] > 22) return 4; 
      }
    }
  }
  return 0;
}

std::vector<std::vector<int>> Player::getPrizeCoords() { 
  int n = prize_coords_x.size();
  std::vector<std::vector<int>> res;
  for (int i = 0; i < n; i++) {
    if (coords.x >= prize_coords_x[i] * tileSize - 32 && coords.x <= prize_coords_x[i] * tileSize + 32) {
      if (coords.y >= prize_coords_y[i] * tileSize - 32 && coords.y <= prize_coords_y[i] * tileSize + 32) {
        res.push_back(prize_coords_x);
        res.push_back(prize_coords_y);
        return res;
      }
    }
  }
  return res;
}

bool Player::isPrize() {
  int n = prize_coords_x.size();
  for (int i = 0; i < n; i++) {
    if (coords.x >= prize_coords_x[i] * tileSize - 32 && coords.x <= prize_coords_x[i] * tileSize + 32) {
      if (coords.y >= prize_coords_y[i] * tileSize - 32 && coords.y <= prize_coords_y[i] * tileSize + 32) {
        return true;
      }
    }
  }
  return false;

}

bool Player::isEmpty() {
  int n = empty_coords_x.size();
  for (int i = 0; i < n; i++) {
    if (coords.x >= empty_coords_x[i] * tileSize - 32 && coords.x <= empty_coords_x[i] * tileSize + 32) {
      if (coords.y >= empty_coords_y[i] * tileSize - 32 && coords.y <= empty_coords_y[i] * tileSize + 32) {
        return true;
      }
    }
  }
  return false;
}


bool Player::isQuit() {

  int n = quit_coords_x.size();
  for (int i = 0; i < n; i++) {
    
    if (coords.x >= quit_coords_x[i] * tileSize - 32 && coords.x <= quit_coords_x[i] * tileSize + 32) {
      if (coords.y >= quit_coords_y[i] * tileSize - 32 && coords.y <= quit_coords_y[i] * tileSize + 32) {
        return true;
      }
    }
  }
  return false;
}