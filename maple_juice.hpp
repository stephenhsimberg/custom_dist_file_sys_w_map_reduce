#include "sdfs.hpp"

extern std::mutex to_append;
extern std::vector<int> failed;

void setup_tcp_communication_client_maple(std::string server_ip, std::string port_num, std::string exec_file, std::string task_file, std::string prefix, int comm_id);
void setup_tcp_server_communication_server_maple(std::string port_num);

void setup_tcp_communication_client_juice(std::string server_ip, std::string port_num, std::string exec_file, std::vector<std::string> task_file, std::string file_out);
void setup_tcp_server_communication_server_juice(std::string port_num);

