#include <iostream>
#include <utility>
#include <cstring>
#include <vector>
#include <cmath>
#include <cstdlib>
#include <ctime>
#include <conio.h>

#define TILE_MAP_SIZE 75

int rnd_range(int min, int max) { // both inclusive
  return std::rand() % ((max + 1) - min) + min;
}

int rnd_range(const std::pair<int, int>& range) {
  return rnd_range(range.first, range.second);
}

enum TileType {
  tile_empty = ' ',
  tile_vertical_wall = '-',
  tile_horizontal_wall = '|',
  tile_floor = '.',
  tile_door = '+',
  tile_corridor = '#',
  tile_player = '@',
  tile_treasure = '$'
};

enum class Dir {
  up,
  down,
  left,
  right,
  none
};

class Room {
public:
  const std::pair<int, int> room_bounds = {5, 10};

  std::pair<int, int> size;
  std::pair<int, int> pos;

  const int& width() const { return size.first; };
  const int& height() const { return size.second; };
  const int& c() const { return pos.first; };
  const int& r() const { return pos.second; };

  Room(std::pair<int, int> pos = {0, 0}) {
    this->size = {rnd_range(room_bounds), rnd_range(room_bounds)};
    this->pos = pos;
  }

  bool c_intersects(const Room& room) const {
    return c() < room.c() + room.width() && c() + width() > room.c();
  }

  bool r_intersects(const Room& room) const {
    return r() < room.r() + room.height() && r() + height() > room.r();
  }

  bool intersects(const Room& room) const {
    return c_intersects(room) && r_intersects(room);
  }
};

// void print_pair(std::pair<int, int> pair) {
//   std::cout << '(' << pair.first << ", " << pair.second << ')';
// }

class TileMap {
  char data[TILE_MAP_SIZE][TILE_MAP_SIZE];
  std::vector<Room> rooms;
  std::pair<int, int> player_pos;
  std::pair<int, int> treasure_pos;

  std::pair<int, int> gen_room_pos(const Room& room) {
    return {
      rnd_range(0, TILE_MAP_SIZE - room.size.first),
      rnd_range(0, TILE_MAP_SIZE - room.size.second)
    };
  }

  void gen_rooms(int n) {
    for (int i = 0; i < n; i++) {
      Room new_room = Room();
      new_room.pos = gen_room_pos(new_room);

      bool valid_room = true;

      for (const auto& room : rooms) {
        if (new_room.intersects(room)) {
          valid_room = false;

          break;
        }
      }

      if (valid_room)
        rooms.push_back(new_room);
    }
  }

  void place_room(const Room& room) {
    for (int c = room.c(); c < room.c() + room.width(); c++) {
      for (int r = room.r(); r < room.r() + room.height(); r++)
      {
        TileType tile_type = tile_floor;

        if (r == room.r() || r == room.r() + room.height() - 1)
          tile_type = tile_vertical_wall;

        else if (c == room.c() || c == room.c() + room.width() - 1)
          tile_type = tile_horizontal_wall;

        data[r][c] = tile_type;
      }
    }
  }

  bool make_vertical_corridor(int col, std::pair<int, int> row_range) {
    for (int r = row_range.first + 1; r < row_range.second; r++)
      if (data[r][col] != tile_empty)
        return false;

    for (int r = row_range.first + 1; r < row_range.second; r++)
      data[r][col] = tile_corridor;

    return true;
  }

  bool make_horizontal_corridor(int row, std::pair<int, int> col_range) {
    for (int c = col_range.first + 1; c < col_range.second; c++)
      if (data[row][c] != tile_empty)
        return false;

    for (int c = col_range.first + 1; c < col_range.second; c++)
      data[row][c] = tile_corridor;
    
    return true;
  }

  bool connect_rooms(const Room& room1, const Room& room2) {
    if (room1.c_intersects(room2)) {
      std::pair<int, int> c_intersect_range;

      c_intersect_range.first = std::max(room1.c(), room2.c());

      c_intersect_range.second = std::min(room1.c() + room1.width() - 1,
        room2.c() + room2.width() - 1);

      if (c_intersect_range.second - c_intersect_range.first >= 2) {
        int col = rnd_range(c_intersect_range.first + 1,
          c_intersect_range.second - 1);

        std::pair<int, int> row_range;

        if (room1.r() < room2.r())
          row_range = {room1.r() + room1.height() - 1, room2.r()};

        else
          row_range = {room2.r() + room2.height() - 1, room1.r()};

        if (make_vertical_corridor(col, row_range)) {
          if (data[row_range.first + 1][col] == tile_corridor && // fix mysterious bug
            data[row_range.second - 1][col == tile_corridor])
          {
            data[row_range.first][col] = tile_door;
            data[row_range.second][col] = tile_door;

            return true;
          }
        }
      }
    }

    else if (room1.r_intersects(room2)) {
      std::pair<int, int> r_intersect_range;

      r_intersect_range.first = std::max(room1.r(), room2.r());

      r_intersect_range.second = std::min(room1.r() + room1.height() - 1,
        room2.r() + room2.height() - 1);

      if (r_intersect_range.second - r_intersect_range.first >= 2) {
        int row = rnd_range(r_intersect_range.first + 1,
          r_intersect_range.second - 1);

        std::pair<int, int> col_range;
        
        if (room1.c() < room2.c())
          col_range = {room1.c() + room1.width() - 1, room2.c()};

        else
          col_range = {room2.c() + room2.width() - 1, room1.c()};

        if (make_horizontal_corridor(row, col_range)) {
          if (data[row][col_range.first + 1] == tile_corridor &&
            data[row][col_range.second - 1] == tile_corridor)
          {
            data[row][col_range.first] = tile_door;
            data[row][col_range.second] = tile_door;

            return true;
          }
        }
      }
    }

    return false;
  }

  std::pair<int, int> random_pos() {
    bool valid_room = false;
    Room tmp;

    while (!valid_room) {
      Room room = rooms[rnd_range(0, rooms.size() - 1)];
      tmp.size = room.size;
      tmp.pos = room.pos;

      for (int c = room.c(); c < room.c() + room.width() - 1; c++) {
        if (data[room.r()][c] == tile_door ||
          data[room.r() + room.height() - 1][c] == tile_door)
        {
          valid_room = true;

          break;
        }
      }

      for (int r = room.r(); r < room.r() + room.height() - 1; r++) {
        if (data[r][room.c()] == tile_door ||
          data[room.c() + room.width() - 1][r] == tile_door)
        {
          valid_room = true;

          break;
        }
      }
    }

    std::pair<int, int> result;

    result.first = rnd_range(tmp.c() + 1, tmp.c() + tmp.width() - 2);
    result.second = rnd_range(tmp.r() + 1, tmp.r() + tmp.height() - 2);

    return result;
  }

  std::pair<int, int> next_tile_pos(std::pair<int, int> curr, Dir dir) {
    switch (dir) {
      case Dir::up:
        curr.second -= 1;

        break;

      case Dir::down:
        curr.second += 1;

        break;

      case Dir::left:
        curr.first -= 1;

        break;

      case Dir::right:
        curr.first += 1;

        break;
    }

    return curr;
  }

  char& tile_at(std::pair<int, int> pos) {
    return data[pos.second][pos.first];
  }

public:
  TileMap() {
    memset(data, tile_empty,
      sizeof(data[0][0]) * TILE_MAP_SIZE * TILE_MAP_SIZE);

    Room room;

    gen_rooms((TILE_MAP_SIZE / room.room_bounds.first) * 4);

    for (const auto& room : rooms)
      place_room(room);

    for (int i = 0; i < rooms.size() - 1; i++) {
      const Room& room1 = rooms[i];

      for (int j = 1; j < rooms.size() - 1; j++) {
        const Room& room2 = rooms[j];

        connect_rooms(room1, room2);
      }
    }

    player_pos = random_pos();
    treasure_pos = random_pos();
  }

  bool move_player(Dir dir) {
    std::pair<int, int> new_pos = next_tile_pos(player_pos, dir);
    
    if (new_pos == treasure_pos)
      return true;

    if (new_pos.first >= 0 && new_pos.first < TILE_MAP_SIZE &&
      new_pos.second >= 0 && new_pos.second < TILE_MAP_SIZE)
    {
      if (tile_at(new_pos) == tile_floor ||
        tile_at(new_pos) == tile_corridor ||
        tile_at(new_pos) == tile_door)
      {
        player_pos = new_pos;
      }
    }

    return false;
  }

  int golden_compass() {
    std::pair<int, int> diff = {
      std::abs(treasure_pos.first - player_pos.first),
      std::abs(treasure_pos.second - player_pos.second)
    };

    return std::sqrt(std::pow(diff.first, 2) + std::pow(diff.second, 2));
  }

  void print() {
    for (int r = 0; r < TILE_MAP_SIZE; r++) {
      for (int c = 0; c < TILE_MAP_SIZE; c++) {
        char tile = data[r][c];

        if (r == player_pos.second && c == player_pos.first)
          tile = tile_player;

        else if (r == treasure_pos.second && c == treasure_pos.first)
          tile = tile_treasure;

        std::cout << tile << ' ';
      }

      std::cout << std::endl;
    }
  }

  void print_fog(const int amount = 5) {
    for (int r = player_pos.second - amount; r < player_pos.second + amount; r++) {
      if (r >= 0 && r < TILE_MAP_SIZE) {
        for (int c = player_pos.first - amount; c < player_pos.first + amount; c++) {
          if (c >= 0 && c < TILE_MAP_SIZE) {
            char tile = data[r][c];

            if (r == player_pos.second && c == player_pos.first)
              tile = tile_player;

            else if (r == treasure_pos.second && c == treasure_pos.first)
              tile = tile_treasure;
      
            std::cout << tile << ' ';
          }
        }

        std::cout << std::endl;
      }
    }
  }
};

void print_treasure_art() {
  std::cout << "         __________\n"
               "        /\\____;;___\\\n"
               "       | /         /\n"
               "       `. ())oo() .\n"
               "        |\\(%()*^^()^\\\n"
               "       %| |-%-------|\n"
               "      % \\ | %  ))   |\n"
               "      %  \\|%________|\n"
               "       %%%%\n";

  std::cout << std::endl;
}

int main() {
  std::srand(std::time(nullptr));

  TileMap tile_map;

  auto clear_console = []() {
    system("cls");
  };

  auto print_tile_map = [&clear_console, &tile_map]() {
    clear_console();
    tile_map.print_fog();

    std::cout << std::endl;
    std::cout << tile_map.golden_compass() << " spaces away ";
  };

  print_tile_map();

  while (true) {
    char ch = std::tolower(getch());
    Dir dir = Dir::none;

    switch (std::tolower(ch)) {
      case 'q':
        return 0;
      
      case 'w':
        dir = Dir::up;

        break;
      
      case 's':
        dir = Dir::down;

        break;
      
      case 'a':
        dir = Dir::left;

        break;

      case 'd':
        dir = Dir::right;

        break;
    }

    if (dir != Dir::none) {
      if (tile_map.move_player(dir)) {
        clear_console();
        std::cout << "well done matey!\n" << std::endl;

        print_treasure_art();

        break;
      }

      print_tile_map();
    }
  }

  return 0;
}