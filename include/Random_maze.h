#ifndef R_MAZE
#define R_MAZE
#include <vector>

class Random_maze{

    public:
    int N; // size of maze
    int *maze;
    int initial_state;
    int final_state; // final state defined by user
    std::vector<int> nodes_x;
    std::vector<int> nodes_y;
    std::vector<int> nodes;

    Random_maze(int n, int is, int fs);

    ~Random_maze();

    std::vector<int> create();

    std::vector<int> state_to_state(int init_x, int init_y, int fin_x, int fin_y);
    
    std::vector<int> create_submaze(int N, int initial, int final);

};


#endif