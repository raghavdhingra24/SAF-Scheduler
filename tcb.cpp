#include <iostream>
#include <cstdlib>
#include <vector>
#include <ucontext.h>

using namespace std;

//Stck size
const size_t STACK_SIZE = 64 * 1024;

// Thread states
enum State {
    READY,
    RUNNING,
    FINISHED
};

// Thread Control Block
struct TCB {
    int id;
    ucontext_t context;
    void* stack;
    State state;
};