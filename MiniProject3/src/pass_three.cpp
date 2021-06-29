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
int player;
double best_choice = -INF;
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
        heuristic = 0;
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

    double find_heuristic(OthelloBoard& input){
        double heu = 0;
        double my_tiles = 0, opp_tiles = 0,my_front_tiles = 0, opp_front_tiles = 0;
        double price[8][8] = {
             20, -3, 11, 8, 8, 11, -3, 20,
            -3, -7, -4, 1, 1, -4, -7, -3,
             11, -4, 2, 2, 2, 2, -4, 11,
             8, 1, 2, -3, -3, 2, 1, 8,
             8, 1, 2, -3, -3, 2, 1, 8,
             11, -4, 2, 2, 2, 2, -4, 11,
            -3, -7, -4, 1, 1, -4, -7, -3,
             20, -3, 11, 8, 8, 11, -3, 20,
        };
        double p = 0, c = 0, l = 0, m = 0, f = 0, d = 0;
        // p ->piece difference 
        for(int i=0;i<SIZE;i++){
            for(int j=0;j<SIZE;j++){
                if(input.board[i][j] == player){
                    d += price[i][j];
                    my_tiles ++ ;
                }
                else if(input.board[i][j] == 3- player){
                    d -= price[i][j];
                    opp_tiles ++ ;
                }
                else{
                    for(int k=0;k<8;k++){
                        if(i+directions[k].x >= 0 && i+directions[k].x < 8 && j+directions[k].y >= 0 && j+directions[k].y < 8){
                            if(input.board[i+directions[k].x][j+directions[k].y] == player ){
                                my_front_tiles ++;
                                break;
                            }
                            else if(input.board[i+directions[k].x][j+directions[k].y] == 3-player){
                                opp_front_tiles ++; 
                                break;
                            }
                        }
                    }
                }
            }
        }
        if(my_tiles > opp_tiles){
            p = (my_tiles)*100/(my_tiles + opp_tiles);
        }
        else if(my_tiles < opp_tiles){
            p = -(opp_tiles)*100/(my_tiles + opp_tiles);
        }
        else p = 0;
        if(my_front_tiles > opp_front_tiles){
            f = -(my_front_tiles)*100/(my_front_tiles + opp_front_tiles);
        }
        else if(my_front_tiles < opp_front_tiles){
            f = (opp_front_tiles)*100/(my_front_tiles + opp_front_tiles);
        }
        else f = 0;
        

        // corner occupation
        my_tiles = opp_tiles = 0;
	    if(input.board[0][0] == player) my_tiles++;
	    else if(input.board[0][0] == 3 - player) opp_tiles++;
	    if(input.board[0][7] == player) my_tiles++;
	    else if(input.board[0][7] == 3 - player) opp_tiles++;
	    if(input.board[7][0] == player) my_tiles++;
	    else if(input.board[7][0] == 3 - player) opp_tiles++;
	    if(input.board[7][7] == player) my_tiles++;
	    else if(input.board[7][7] == 3 - player) opp_tiles++;
	    c = 25 * (my_tiles - opp_tiles);

        // corner closeness
	    my_tiles = opp_tiles = 0;
	    if(input.board[0][0] == 0)   {
		    if(input.board[0][1] == player) my_tiles++;
		    else if(input.board[0][1] == 3 - player) opp_tiles++;
		    if(input.board[1][1] == player) my_tiles++;
		    else if(input.board[1][1] == 3 - player) opp_tiles++;
		    if(input.board[1][0] == player) my_tiles++;
		    else if(input.board[1][0] == 3 - player) opp_tiles++;
	    }
	    if(input.board[0][7] == 0)   {
		    if(input.board[0][6] == player) my_tiles++;
		    else if(input.board[0][6] == 3 - player) opp_tiles++;
		    if(input.board[1][6] == player) my_tiles++;
		    else if(input.board[1][6] == 3 - player) opp_tiles++;
		    if(input.board[1][7] == player) my_tiles++;
		    else if(input.board[1][7] == 3 - player) opp_tiles++;
	    }
	    if(input.board[7][0] == 0)   {
		    if(input.board[7][1] == player) my_tiles++;
		    else if(input.board[7][1] == 3 - player) opp_tiles++;
		    if(input.board[6][1] == player) my_tiles++;
		    else if(input.board[6][1] == 3 - player) opp_tiles++;
		    if(input.board[6][0] == player) my_tiles++;
		    else if(input.board[6][0] == 3 - player) opp_tiles++;
	    }
	    if(input.board[7][7] == 0)   {
		    if(input.board[6][7] == player) my_tiles++;
		    else if(input.board[6][7] == 3 - player) opp_tiles++;
		    if(input.board[6][6] == player) my_tiles++;
		    else if(input.board[6][6] == 3 - player) opp_tiles++;
		    if(input.board[7][6] == player) my_tiles++;
		    else if(input.board[7][6] == 3 - player) opp_tiles++;
	    }
	    l = -12.5 * (my_tiles - opp_tiles);  
        // movable 
        my_tiles = opp_tiles = 0;
        input.cur_player = get_next_player(input.cur_player);
        std::vector<Point> temp  = get_valid_spots();
        opp_tiles = temp.size();
        input.cur_player = get_next_player(input.cur_player);
        my_tiles = input.next_valid_spots.size();
	    if(my_tiles > opp_tiles)
		    m = (100.0 * my_tiles)/(my_tiles + opp_tiles);
	    else if(my_tiles < opp_tiles)
		    m = -(100.0 * opp_tiles)/(my_tiles + opp_tiles);
	    else m = 0;
        //cout << "m is " << m <<endl;
        heu  = (2 * p) + (20 * c) + (12 * l) + (15 * m) + (-5 * f) + (20 * d);
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
    create.put_disc(place);
    create.played_disc = place;
    create.heuristic = create.find_heuristic(create);
    return create;
}
// state 1 -> find max / state 0 ->find min
double search(OthelloBoard& board , double& player_strategy , double& opponent_strategy , int depth , int state){
    if(board.done || depth == 0){
        if(board.winner == 3 - player){
            return -INF + 100;
        }
        else if(board.winner == player){
            return INF;
        }
        return board.heuristic;
    }
    int k = depth;
    k--;
    if(state == 1){
        double val = -INF;
        for(auto it:board.next_valid_spots){
            OthelloBoard next = board;
            next.put_disc(it);
            next.heuristic = next.find_heuristic(next);
            double value = search(next,player_strategy,opponent_strategy,k,0);
            if(val < value){
                val= value;
            }
            if(val > player_strategy){
                player_strategy = val;
            }
            if(player_strategy >= opponent_strategy){
                break;
            }
        }
        return val;
    }
    else if(state == 0){
        double val = INF;
        for(auto it:board.next_valid_spots){
            OthelloBoard next = board;
            next.put_disc(it);
            next.heuristic = next.find_heuristic(next);
            double value = search(next,player_strategy,opponent_strategy,k,1);
            if(val > value){
                val= value;
            }
            if(val < opponent_strategy){
                opponent_strategy = val;
            }
            if(player_strategy >= opponent_strategy){
                //cout << "cutoff" << endl;
                break;
            }
        }
        return val;
    }
    return -1;
}
// player 1 -> x  // player 2 -> o
void write_valid_spot(std::ofstream& fout) {
    OthelloBoard cur(next_valid_spots,board,player);
    double max = -INF;
    double min = INF;
    double desicion = -INF;
    for(auto it:cur.next_valid_spots){
        double val;
        OthelloBoard new_one;
        new_one = cur;
        new_one.put_disc(it);
        new_one.heuristic = new_one.find_heuristic(new_one);
        val = search(new_one,max,min,5,0);
        cout << "current val " << val ;
        if(val > desicion){
            desicion = val;
            cur.played_disc = it;
        }
    }
    cout << endl;
    cout << cur.played_disc.x << " " << cur.played_disc.y << std::endl;
    fout << cur.played_disc.x << " " << cur.played_disc.y << std::endl;
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
