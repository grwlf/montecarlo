#include <list>
#include <map>
#include <vector>
#include <unordered_map>
#include <iostream>
#include <cstdlib>

using namespace std;


struct board {
  enum class cell {X, O, Empty};

  static const int size = 3;

  cell board[size][size];
};

struct action {
  int x; // 0..2
  int y; // 0..2
};


bool move ( board &bo, board::cell who, action a) {
  bo.board[a.x][a.y] = who;

#define b bo.board

  for(int i=0; i<3; i++) {
    if(b[0][i] == b[1][i] && b[1][i] == b[2][i] && b[2][i] == who)
      return true;
  }
  for(int i=0; i<3; i++) {
    if(b[i][0] == b[i][1] && b[i][1] == b[i][2] && b[i][2] == who)
      return true;
  }
  if(b[0][0] == b[1][1] && b[1][1] == b[2][2] && b[2][2] == who)
    return true;
  if(b[0][2] == b[1][1] && b[1][1] == b[2][0] && b[2][0] == who)
    return true;

#undef b
}

namespace std {

    template <>
    struct hash<action> {
        size_t operator()(const action& a) const {
            return a.x + a.y * 43;
        }
    };

    template <>
    struct hash<board> {
        size_t operator()(const board& a) const {
            size_t acc;
            for(int y=0; y<3; y++) {
              for(int x=0; x<3; x++) {
                acc = (acc << 2)  ^ (1+ (size_t)a.board[x][y]);
              }
            }
            return acc;
        }
    };

    template <>
    struct hash< pair<board,action> > {
        size_t operator()(const pair<board,action>& a) const {
            return hash<board>()(a.first) ^  hash<action>()(a.second);
        }
    };
}


struct policy {
  unordered_map<board, action> actmap;

};

action sample_policy( const policy& p, const board &b )
{
  int y = (rand() % 1024);
  int x = (rand() % 1024);
  if (y < 3 && x < 3 && b.board[x][y] == board::cell::Empty) {
    action a = action {x, y};
    return a;
  }
  else {
    return p.actmap[b];
  }
}


struct Q {
  unordered_map< pair<board, action>, double > qmap;
};


int main() {
  return 0;
}



