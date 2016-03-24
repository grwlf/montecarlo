#include <assert.h>

#include <list>
#include <map>
#include <vector>
#include <unordered_map>
#include <iostream>
#include <cstdlib>

using namespace std;

#define FOR_EACH_LOOP(i,j)                  \
    for (int i=0; i < Board::size; i++)     \
        for (int j=0; j < Board::size; j++) \

#define FOR_EACH_EMPTY(b)                               \
    FOR_EACH_LOOP(i,j)                                  \
        if (b.At(i,j) == Board::cell::Empty)

struct action {
    int i; // 0..2
    int j; // 0..2
    void Print() const {
        fprintf(stdout, "<%d;%d>\n", i,j);
    }
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
            fprintf(stdout,"%s%s", CellToStr(At(i,j)), (j == size-1) ? "\n":"");
        }
        fprintf(stdout, "\n");
    }
    int NumEmpties() const { return num_empties; }
    static const char *CellToStr(cell ce) {
        const char *c;
        switch(ce) {
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
        return c;
    }

private:
    cell board[size][size];
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

    bool operator==(const action a1, const action a2) {
        return a1.i == a2.i && a1.j == a2.j;
    }

    template <>
    struct hash<Board> {
        size_t operator()(const Board& b) const {
            size_t acc = 0;
            FOR_EACH_LOOP(i,j) {
                acc = (acc << 2)  ^ (1+ (size_t)b.At(i,j));
            }
            return acc;
        }
    };

    template <>
    struct hash< pair<Board,action> > {
        size_t operator()(const pair<Board,action>& a) const {
            return hash<Board>()(a.first) ^ hash<action>()(a.second);
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

typedef pair<Board, action> StateAct;

struct Q {
  unordered_map< StateAct, Averager > qmap;
};

class MCLearner {
public:
    static constexpr int DENOM = 1<<20;

    MCLearner(double eps): int_eps(DENOM * eps) {}

    action Sample(const Board &b) {
        action a;
        const auto &p_act = pol.actmap.find(b);

        if (p_act == pol.actmap.end() || (rand() % (1 << 20)) < int_eps) {
            int n = rand() % b.NumEmpties();
            FOR_EACH_EMPTY(b) {
                if (n == 0) {
                    a = {i,j};
                    goto store;
                } else n--;
            }
            assert(false);
        } else {
            return a = p_act->second;
        }
    store:
        episode.push_back({b,a});
        return a;
    }

    void Reward(int rew) {
        single_reward = rew;
    }
    void UpdatePolicy() {
        // Updating action values
        for (const auto &sa : episode) {
            act_values.qmap[sa].Add(single_reward);
        }
        // Updating policy
        for (const auto &sa : episode) {
            const Board &sb = sa.first;
            double best = -1e6;
            action best_a;
            bool found=false;
            FOR_EACH_LOOP(i,j) {
                action a{i,j};
                auto q = act_values.qmap.find({sb, a});
                if (q != act_values.qmap.end()) {
                    double v= q->second.Avg();
                    if (v > best) {
                        best_a = a;
                        best = v;
                        found = true;
                    }
                }
            }
            assert(found);
            pol.actmap[sb] = best_a;
        }
        episode.clear();
        single_reward = 0;
    }
    const Q &ActValues() const {
        return act_values;
    }
    const policy &Policy() const {
        return pol;
    }
private:
    int int_eps;
    Q act_values;
    policy pol;

    vector<StateAct> episode;
    int single_reward;
};

void driver(MCLearner &player1, MCLearner &player2) {
    double avg_reward = 0;

    for (int k=0; k < 100000; k++) { // episode
        Board b;

        int reward = 0;
        while(true) {
            action a1 = player1.Sample(b);
            MoveResult res = b.Move(a1, Board::cell::X);
//            b.Print();
            if (res == MoveResult::Win) {
                reward = 1;
                break;
            } else if (res == MoveResult::Draw) {
                reward = 0;
                break;
            }

            action a2 = player2.Sample(b);
            res = b.Move(a2, Board::cell::O);
//            b.Print();
            if (res == MoveResult::Win) {
                reward = -1;
                break;
            } else if (res == MoveResult::Draw) {
                reward = 0;
                break;
            }
        }
        player1.Reward(reward);
        player1.UpdatePolicy();

        player2.Reward(-reward);
        player2.UpdatePolicy();


        avg_reward = avg_reward * 0.99 + reward * 0.01;

        printf("q %d, p %d r %d ar %f\n", player1.ActValues().qmap.size(), player1.Policy().actmap.size(), reward, avg_reward);
    }
}

int main(int argc, char *argv[]) {
    if (argc != 3) goto print_usage;
    else {
        double eps1 = std::stod(string(argv[1]));
        double eps2 = std::stod(string(argv[2]));
        if (eps1 < 0 || eps1 > 1 || eps2 < 0 || eps2 > 1) {
            goto print_usage;
        } else {
            MCLearner p1(eps1);
            MCLearner p2(eps2);

            driver(p1,p2);
            return 0;
        }
    }
print_usage:
    fprintf(stderr, "pass two epsilon-parameters for learners\n");
    return 1;
}
