#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netdb.h>
#include <string.h>
#include <fstream>
#include <vector>
#include <queue>
#include <list>
#include <unordered_map>
#include <filesystem>
#include <thread>
#include <ctime>
#include <algorithm>
#include <sstream>
#include <mutex>
#include <chrono>

extern volatile int sus;



#define DEAD 2
#define NO_ENTRY 0
#define ENTRIES_IN_ROW 3
#define ADDRESS_i 0
#define HEARTBEAT_i 1
#define DEAD_i 2
#define NUM_MACHINES 10

#define T_FAIL 4
#define T_GOSSIP 0.2



extern std::mutex m;

extern int mem_list[(NUM_MACHINES*3)+3];

extern std::string mem_file;

extern std::vector<int> ports;

extern int mem_list_times[NUM_MACHINES+1];

extern std::vector<std::string> machine_names;


extern std::vector<int> machines_avail;

extern std::string sysfile_path;

