#include <cstdlib>
#include <ctime>
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <array>
#include <vector>
#include <cassert>
#include <cmath>

#define INF 0x3f3f3f3f
using namespace std;

struct Point {
    int x, y;
	Point() : Point(0, 0) {}
	Point(int x, int y) : x(x), y(y) {}
	bool operator==(const Point& rhs) const {
		return x == rhs.x && y == rhs.y;
	}
	bool operator!=(const Point& rhs) const {
		return !operator==(rhs);
	}
	Point operator+(const Point& rhs) const {
		return Point(x + rhs.x, y + rhs.y);
	}
	Point operator-(const Point& rhs) const {
		return Point(x - rhs.x, y - rhs.y);
	}
};


class OthelloBoard {
public: 
    enum SPOT_STATE {
        EMPTY = 0,
        BLACK = 1,
        WHITE = 2
    };
    static const int SIZE = 8;
    const std::array<Point, 8> directions{{
        Point(-1, -1), Point(-1, 0), Point(-1, 1),
        Point(0, -1), /*{0, 0}, */Point(0, 1),
        Point(1, -1), Point(1, 0), Point(1, 1)
    }};
    std::array<std::array<int, SIZE>, SIZE> board;
    std::vector<Point> next_valid_spots;
    std::array<int, 3> disc_count;
    int cur_player;
    bool done;
    int winner;
    double heuristic;
    Point played_disc;
private:
    int get_next_player(int player) const {
        return 3 - player;
    }
    bool is_spot_on_board(Point p) const {
        return 0 <= p.x && p.x < SIZE && 0 <= p.y && p.y < SIZE;
    }
    int get_disc(Point p) const {
        return board[p.x][p.y];
    }
    void set_disc(Point p, int disc) {
        board[p.x][p.y] = disc;
    }
    bool is_disc_at(Point p, int disc) const {
        if (!is_spot_on_board(p))
            return false;
        if (get_disc(p) != disc)
            return false;
        return true;
    }
    bool is_spot_valid(Point center) const {
        if (get_disc(center) != EMPTY)
            return false;
        for (Point dir: directions) {
            // Move along the direction while testing.
            Point p = center + dir;
            if (!is_disc_at(p, get_next_player(cur_player)))
                continue;
            p = p + dir;
            while (is_spot_on_board(p) && get_disc(p) != EMPTY) {
                if (is_disc_at(p, cur_player))
                    return true;
                p = p + dir;
            }
        }
        return false;
    }
    void flip_discs(Point center) {
        for (Point dir: directions) {
            // Move along the direction while testing.
            Point p = center + dir;
            if (!is_disc_at(p, get_next_player(cur_player)))
                continue;
            std::vector<Point> discs({p});
            p = p + dir;
            while (is_spot_on_board(p) && get_disc(p) != EMPTY) {
                if (is_disc_at(p, cur_player)) {
                    for (Point s: discs) {
                        set_disc(s, cur_player);
                    }
                    disc_count[cur_player] += discs.size();
                    disc_count[get_next_player(cur_player)] -= discs.size();
                    break;
                }
                discs.push_back(p);
                p = p + dir;
            }
        }
    }
public:
    OthelloBoard(const OthelloBoard& rhs) {
        for(int i=0;i<SIZE;i++){
            for(int j=0;j<SIZE;j++){
                board[i][j] = rhs.board[i][j];
            }
        }
        for(long unsigned int i=0;i<rhs.next_valid_spots.size();i++){
            next_valid_spots.push_back(rhs.next_valid_spots[i]);
        }
        for(int i=0;i<3;i++){
            disc_count[i] = rhs.disc_count[i];
        }
        cur_player = rhs.cur_player;
        done = rhs.done;
        heuristic = rhs.heuristic;
        winner = rhs.winner;
        played_disc = rhs.played_disc;
    }
    OthelloBoard() {
        reset();
    }
    OthelloBoard(const std::vector<Point>&input_valid_point , \
    const std::array<std::array<int, SIZE>, SIZE>&input_board, int player):board(input_board),\
    next_valid_spots(input_valid_point),cur_player(player){
        count_disc();
        heuristic = find_heuristic();
        done = false;
        winner = -1;
        played_disc = {-1,-1};
    }
    void reset() {
        for (int i = 0; i < SIZE; i++) {
            for (int j = 0; j < SIZE; j++) {
                board[i][j] = EMPTY;
            }
        }
        board[3][4] = board[4][3] = BLACK;
        board[3][3] = board[4][4] = WHITE;
        cur_player = BLACK;
        disc_count[EMPTY] = 8*8-4;
        disc_count[BLACK] = 2;
        disc_count[WHITE] = 2;
        next_valid_spots = get_valid_spots();
        done = false;
        winner = -1;
    }
    std::vector<Point> get_valid_spots() const {
        std::vector<Point> valid_spots;
        for (int i = 0; i < SIZE; i++) {
            for (int j = 0; j < SIZE; j++) {
                Point p = Point(i, j);
                if (board[i][j] != EMPTY)
                    continue;
                if (is_spot_valid(p))
                    valid_spots.push_back(p);
            }
        }
        return valid_spots;
    }
    bool put_disc(Point p) {
        if(!is_spot_valid(p)) {
            cout << cur_player << " " << p.x << " " << p.y << endl;
            winner = get_next_player(cur_player);
            done = true;
            return false;
        }
        set_disc(p, cur_player);
        disc_count[cur_player]++;
        disc_count[EMPTY]--;
        flip_discs(p);
        // Give control to the other player.
        cur_player = get_next_player(cur_player);
        next_valid_spots = get_valid_spots();
        // Check Win
        if (next_valid_spots.size() == 0) {
            cur_player = get_next_player(cur_player);
            next_valid_spots = get_valid_spots();
            if (next_valid_spots.size() == 0) {
                // Game ends
                done = true;
                int white_discs = disc_count[WHITE];
                int black_discs = disc_count[BLACK];
                if (white_discs == black_discs) winner = EMPTY;
                else if (black_discs > white_discs) winner = BLACK;
                else winner = WHITE;
            }
        }
        return true;
    }
    void count_disc(){
        for(int i=0;i<3;i++){
            disc_count[i] = 0;
        }
        for(int i=0;i<SIZE;i++){
            for(int j=0;j<SIZE;j++){
                disc_count[board[i][j]]++;
            }
        }
    }
    double find_heuristic(){
        double heu = 0;
        /*for(int i=0;i<SIZE;i++){
            for(int j=0;j<SIZE;j++){
                cout << board[i][j];
            }
            cout << endl;
        }*/
        /*for(int i=0;i<SIZE;i++){
            for(int j=0;j<SIZE;j++){
                if(board[i][j]== 3-cur_player){
                    if(i==0 || j == 0 || i == SIZE-1 || j == SIZE-1){
                        heu += 1;
                    }
                    if((i == 0 && j== 0) || (i == 0 && j== SIZE-1) || \
                     (i == SIZE-1 && j== SIZE-1) || (i == SIZE-1 && j== 0)){
                        heu += 0;
                     }
                    else{
                        heu += 1;
                    }
                }
                else if(board[i][j] == cur_player){
                    if(i==0 || j == 0 || i == SIZE-1 || j == SIZE-1){
                        heu -= 1;
                    }
                    if((i == 0 && j== 0) || (i == 0 && j== SIZE-1) || \
                     (i == SIZE-1 && j== SIZE-1) || (i == SIZE-1 && j== 0)){
                        heu -= 0;
                     }
                    else{
                        heu -= 1;
                    }
                }
            }
        }
        cout << heu << endl;*/
        heu = disc_count[3-cur_player] - disc_count[cur_player];
        return heu;
    }
    
    OthelloBoard& operator=(const OthelloBoard& rhs){
    for(int i=0;i<SIZE;i++){
        for(int j=0;j<SIZE;j++){
            board[i][j] = rhs.board[i][j];
        }
    }
    for(long unsigned int i=0;i<rhs.next_valid_spots.size();i++){
        next_valid_spots.push_back(rhs.next_valid_spots[i]);
    }
    for(int i=0;i<3;i++){
        disc_count[i] = rhs.disc_count[i];
    }
    cur_player = rhs.cur_player;
    done = rhs.done;
    heuristic = rhs.heuristic;
    winner = rhs.winner;
    played_disc = rhs.played_disc;
    return *this;
    }
};


int player;
const int SIZE = 8;
std::array<std::array<int, SIZE>, SIZE> board;
std::vector<Point> next_valid_spots;

void read_board(std::ifstream& fin) {
    fin >> player;
    for (int i = 0; i < SIZE; i++) {
        for (int j = 0; j < SIZE; j++) {
            fin >> board[i][j];
        }
    }
}

void read_valid_spots(std::ifstream& fin) {
    int n_valid_spots;
    fin >> n_valid_spots;
    int x, y;
    for (int i = 0; i < n_valid_spots; i++) {
        fin >> x >> y;
        next_valid_spots.push_back({x, y});
    }
}
int count = 0;
OthelloBoard update(const OthelloBoard& in,Point place){
    count++;
    OthelloBoard create(in);
    bool useless = create.put_disc(place);
    create.played_disc = place;
    create.heuristic = create.find_heuristic();
    return create;
}
// state 1 -> find max / state 0 ->find min
OthelloBoard search(OthelloBoard& board , double& player_strategy , double& opponent_strategy , int depth , int state){
    if(board.done || depth == 0){
        return board;
    }
    int k = depth;
    k--;
    if(state == 1){
        double val = -INF;
        for(auto it:board.next_valid_spots){
            OthelloBoard next = board;
            next.next_valid_spots.clear();
            next = update(next,it);
            OthelloBoard value = search(next,player_strategy,opponent_strategy,k,0);
            if(val < value.heuristic){
                val= value.heuristic;
                board.played_disc = it;
            }
            if(val > player_strategy){
                player_strategy = val;
            }
            if(player_strategy >= opponent_strategy){
                break;
            }
        }
        return board;
    }
    else if(state == 0){
        double val = INF;
        for(auto it:board.next_valid_spots){
            OthelloBoard next = board;
            next.next_valid_spots.clear();
            next = update(next,it);
            OthelloBoard value = search(next,player_strategy,opponent_strategy,k,1);
            if(value.heuristic < val){
                val= value.heuristic;
                board.played_disc = it;
            }
            if(val < opponent_strategy){
                opponent_strategy = val;
            }
            if(player_strategy <= opponent_strategy){
                break;
            }
        }
    }
    return board;
}
// player 1 -> x  // player 2 -> o
void write_valid_spot(std::ofstream& fout) {
    OthelloBoard cur(next_valid_spots,board,player);
    double max = -INF ;
    double min = INF;
    OthelloBoard final_desicion = search(cur,max,min,7,1);
    cout << count << endl;
    fout << final_desicion.played_disc.x << " " << final_desicion.played_disc.y << std::endl;
    fout.flush();
}

int main(int, char** argv) {
    std::ifstream fin(argv[1]);
    std::ofstream fout(argv[2]);
    read_board(fin);
    read_valid_spots(fin);
    write_valid_spot(fout);
    fin.close();
    fout.close();
    return 0;
}
