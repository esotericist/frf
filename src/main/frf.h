#include "sglib.h"
#include "sds.h"
struct node_state {
    /**
     * int nextPID
     * map int atomtoprimtable
     * map int primtoatomtable
     * 
     * map int atomtowordtable
     * map int wordtoatomtable
     * 
     * map string defines
     * 
     * process_state interpreter_process
     * 
     * map process_state process_pool
     * 
     * list int activeprocesses
     * list int inactiveprocesses
     * list int deadprocesses
     * 
     */ 


};

struct process_state {
    /**
     * 
     * int pid
     * node_state node
     * 
     * int parsemode
     * int errorstate
     * int executestate
     * 
     * int processlistptr -- pointer to this process' position in the relevant process list of the owning node
     * 
     * int currentop
     * code_stream current_codestream
     * 
     */    
};

struct code_set {
    /**
     * 
     */

};
