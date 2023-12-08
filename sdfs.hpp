#include "gossip.hpp"


void setup_tcp_communication_client(std::string server_ip, std::string port_num, std::string arg_sys_file_name, std::string arg_loc_file_name, std::string op);
void setup_tcp_server_communication_server(std::string port_num);
void setup_tcp_server_communication_server_leader(std::string port_num);


void write_to_self(std::string sys_name);
void leader_write(int socket_fd);

void recover_files(int mach_number);
void failure_detector();


// header file for maple leader
int maple_leader(int comm_socket_fd, std::string maple_exe, int num_tasks, std::string inter_prefix, std::string input_dir);
int juice_leader(int comm_socket_fd, std::string juice_exe, int num_tasks, std::string inter_prefix, std::string output_f_name, int to_delete);


void *get_in_addr(struct sockaddr *sa);




// LEADER CODE:
/* have global vector of a tuple on leader to tell which machines are currently "working" with which client, and which file. */
    
    // int machines_status[10];
    // std::unordered_map<std::string, std::tuple<int, int, int>> file_to_machines;
    /*have global map: filename->machines held on (metadata) fourth int: if someone reading file = 1, if someone writing file =2, else 0 */
extern std::unordered_map<std::string, std::tuple<int, int, int, int>> file_to_machines;

/* have 10 other global vectors: machine->files to use when machine fails to replicate files */
// std::unordered_map<std::string, int> machine0_files;
extern std::vector<std::unordered_map<std::string, int>> machine_files;



extern int leader_id; // current leader id (set as 0 to start)
extern volatile int failure_fixing; // currently fixing failure
extern volatile int file_fixed; // current file that was writing needs to be written again
extern int host_id;

extern std::list<std::tuple<std::string, std::string, int>> working; // working list, doesn't allow for more than 4 writes or 4 reads of same file
// will unqueue before read / write is communicated, so multiple files can be written or read if not the same file, and 2 reads for same file.


extern std::mutex file_lock; // lock used to check working list


extern int failed_write;   // variable to set if machine failed while writing so can restart write

extern std::string file_string;    // global string to save file on to save RAM when doing put




