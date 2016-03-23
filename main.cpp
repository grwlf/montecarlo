#include <assert.h>

#include <list>
#include <map>
#include <vector>
#include <unordered_map>
#include <iostream>
#include <cstdlib>

using namespace std;

constexpr int EPS = 100000; // 10%-greedy

#define FOR_EACH_LOOP(i,j)                  \
    for (int i=0; i < Board::size; i++)     \
        for (int j=0; j < Board::size; j++) \

#define FOR_EACH_EMPTY(b)                               \
    FOR_EACH_LOOP(i,j)                                  \
        if (b.board[i][j] == Board::cell::Empty)

struct Board {
    enum class cell {X, O, Empty};
    static constexpr int size = 3;

    Board() {
        FOR_EACH_LOOP(i,j) {
            board[i][j] = cell::Empty;
        }
    }

  cell board[size][size];
};

struct action {
  int x; // 0..2
  int y; // 0..2
};

bool move ( Board &bo, Board::cell who, action a) {
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

    bool operator==(const Board &b1, const Board &b2) {
        FOR_EACH_LOOP(i,j) {
            if (b1.board[i][j] != b2.board[i][j]) {
                return false;
            }
        }
        return true;
    }

    template <>
    struct hash<Board> {
        size_t operator()(const Board& a) const {
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
    struct hash< pair<Board,action> > {
        size_t operator()(const pair<Board,action>& a) const {
            return hash<Board>()(a.first) ^  hash<action>()(a.second);
        }
    };
}

class Averager {
public:
    Averager(): avg(0), n(0) {}
    void Add(double x) {
        n++;
        avg = avg + (x - avg) / n;
    }
    double Avg() const {
        return avg;
    }
private:
    double avg;
    int n;
};

struct ActionStats {
    action act;
    Averager stats;
};

struct policy {
  unordered_map<Board, ActionStats> actmap;
};

action sample_policy( const policy& p, const Board &b )
{
    int num_empties = 0;
    FOR_EACH_EMPTY(b) {
        num_empties++;
    }

    const auto &p_act = p.actmap.find(b);

    if (p_act == p.actmap.end() || (rand() % (1 << 20)) < EPS) {
        int n = rand() % num_empties;
        FOR_EACH_EMPTY(b) {
            if (n == 0) return {i,j};
            else n--;
        }
        assert(false);
    } else {
        return p_act->second.act;
    }
}

struct Q {
  unordered_map< pair<Board, action>, double > qmap;
};

void driver(policy &player1, policy &player2) {
    while(true) { // episode
        Board b;
    }
}

int main() {
  return 0;
}



