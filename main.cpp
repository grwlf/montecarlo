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

struct action {
  int i; // 0..2
  int j; // 0..2
};

enum class MoveResult { Draw, Win, Continue };

struct Board {
public:
    enum class cell {X, O, Empty};
    static constexpr int size = 3;

    Board() {
        FOR_EACH_LOOP(i,j) {
            board[i][j] = cell::Empty;
        }
    }
    MoveResult Move(action a, Board::cell who) {
        assert(board[a.i][a.j] == Board::cell::Empty);
        board[a.i][a.j] = who;
        num_empties--;

        auto &b = board;

        for(int i=0; i<3; i++) {
            if(b[0][i] == b[1][i] && b[1][i] == b[2][i] && b[2][i] == who)
                return MoveResult::Win;
        }
        for(int i=0; i<3; i++) {
            if(b[i][0] == b[i][1] && b[i][1] == b[i][2] && b[i][2] == who)
                return MoveResult::Win;
        }
        if(b[0][0] == b[1][1] && b[1][1] == b[2][2] && b[2][2] == who)
            return MoveResult::Win;
        if(b[0][2] == b[1][1] && b[1][1] == b[2][0] && b[2][0] == who)
            return MoveResult::Win;

        if (num_empties == 0) {
            return MoveResult::Draw;
        }
        return MoveResult::Continue;
    }
    cell At(int i, int j) const {
        assert(i >=0 && i < size);
        assert(j >=0 && j < size);
        return board[i][j];
    }
    void Print() const {
        FOR_EACH_LOOP(i,j) {
            const char *c;
            switch(At(i,j)) {
            case cell::X:
                c = "X";
                break;
            case cell::O:
                c = "O";
                break;
            case cell::Empty:
                c = ".";
                break;
            };
            fprintf(stdout,"%s%s", c, (j == size-1) ? "\n":"");
        }
        fprintf(stdout, "\n");
    }
    int NumEmpties() const { return num_empties; }
    cell board[size][size];
private:
    int num_empties = size*size;
};

namespace std {

    template <>
    struct hash<action> {
        size_t operator()(const action& a) const {
            return a.i + a.j * 43;
        }
    };

    bool operator==(const Board &b1, const Board &b2) {
        if (b1.NumEmpties() != b2.NumEmpties()) return false;
        FOR_EACH_LOOP(i,j) {
            if (b1.At(i,j) != b2.At(i,j)) {
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

struct policy {
  unordered_map<Board, action> actmap;
};

action sample_policy( const policy& p, const Board &b )
{
    const auto &p_act = p.actmap.find(b);

    if (p_act == p.actmap.end() || (rand() % (1 << 20)) < EPS) {
        int n = rand() % b.NumEmpties();
        FOR_EACH_EMPTY(b) {
            if (n == 0) return {i,j};
            else n--;
        }
        assert(false);
    } else {
        return p_act->second;
    }
}

typedef pair<Board, action> StateAct;

struct Q {
  unordered_map< StateAct, Averager > qmap;
};

void driver(policy &player1, policy &player2) {
    Q q1;
    Q q2;

    vector<StateAct> episode1;
    vector<StateAct> episode2;
    while(true) { // episode
        Board b;
        episode1.clear();
        episode2.clear();

        int reward = 0;
        while(true) {
            action a1 = sample_policy(player1, b);
            MoveResult res = b.Move(a1, Board::cell::X);
            b.Print();
            if (res == MoveResult::Win) {
                reward = 1;
                break;
            } else if (res == MoveResult::Draw) {
                reward = 0;
                break;
            }
            action a2 = sample_policy(player2, b);
            res = b.Move(a2, Board::cell::O);
            b.Print();
            if (res == MoveResult::Win) {
                reward = -1;
                break;
            } else if (res == MoveResult::Draw) {
                reward = 0;
                break;
            }
        }
        break;
    }
}

int main() {
    policy p1, p2;
    driver(p1,p2);
    return 0;
}



