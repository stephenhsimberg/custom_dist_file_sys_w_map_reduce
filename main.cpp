#include "maple_juice.hpp"

volatile int sus;

std::mutex m;

int mem_list[(NUM_MACHINES*3)+3];

std::string mem_file;

std::vector<int> ports;

int mem_list_times[NUM_MACHINES+1];

std::vector<std::string> machine_names;


std::vector<int> machines_avail;

std::unordered_map<std::string, std::tuple<int, int, int, int>> file_to_machines;

/* have 10 other global vectors: machine->files to use when machine fails to replicate files */
// std::unordered_map<std::string, int> machine0_files;
std::vector<std::unordered_map<std::string, int>> machine_files;



int leader_id; // current leader id (set as 0 to start)
volatile int failure_fixing; // currently fixing failure
volatile int file_fixed; // current file that was writing needs to be written again
int host_id;

std::list<std::tuple<std::string, std::string, int>> working; // working list, doesn't allow for more than 4 writes or 4 reads of same file
// will unqueue before read / write is communicated, so multiple files can be written or read if not the same file, and 2 reads for same file.


std::mutex file_lock; // lock used to check working list

int failed_write;   // variable to set if machine failed while writing so can restart write

std::string file_string;    // global string to save file on to save RAM when doing put

std::mutex to_append;
std::vector<int> failed;

std::string sysfile_path;





int main() {

    sysfile_path = "/home/sysfiles";

    std::string path(sysfile_path);
    system("rm -r sysfiles");
    system("mkdir sysfiles");
    // for (auto &file_iterator : std::filesystem::recursive_directory_iterator(path)) {
    //     std::string to_remove("rm sysfiles/");
    //     to_remove.append(file_iterator.path().filename().string());
    //     system(to_remove.c_str());
    // }


    for(int i = 0; i < 10; i++) {
        machines_avail.push_back(0);
    }

    machine_names.push_back("fa23-cs425-1601.cs.illinois.edu");
    machine_names.push_back("fa23-cs425-1602.cs.illinois.edu");
    machine_names.push_back("fa23-cs425-1603.cs.illinois.edu");
    machine_names.push_back("fa23-cs425-1604.cs.illinois.edu");
    machine_names.push_back("fa23-cs425-1605.cs.illinois.edu");
    machine_names.push_back("fa23-cs425-1606.cs.illinois.edu");
    machine_names.push_back("fa23-cs425-1607.cs.illinois.edu");
    machine_names.push_back("fa23-cs425-1608.cs.illinois.edu");
    machine_names.push_back("fa23-cs425-1609.cs.illinois.edu");
    machine_names.push_back("fa23-cs425-1610.cs.illinois.edu");
    ports.push_back(1601);
    ports.push_back(1602);
    ports.push_back(1603);
    ports.push_back(1604);
    ports.push_back(1605);
    ports.push_back(1606);
    ports.push_back(1607);
    ports.push_back(1608);
    ports.push_back(1609);
    ports.push_back(1610);

    sus = 1;
    leader_id = 0;
    failure_fixing = 0;
    file_fixed = 0;
    failed_write = 0;

    machine_files.resize(10);







    char host_name[50];

    // get machine name of terminal being grepped
    gethostname(host_name, sizeof(host_name));

    std::vector<std::thread> threads;
    threads.emplace_back(udp_server); // will be server for this machine
    threads.emplace_back(client_main);

    // idea for leader failure
    // for(int i = 1; i < 10; i++){
    //     if(machines_avail[i] == 1){
    //         setup_tcp_communication_client(machine_names[i],  std::to_string(ports[0] + i*10), "", "", "check_leader");
    //     }
    // }

     

    if(!strcmp(host_name, machine_names[leader_id].c_str())) {
        threads.emplace_back(failure_detector);
        threads.emplace_back(setup_tcp_server_communication_server_leader, std::to_string(ports[1]+ leader_id*10));
        threads.emplace_back(setup_tcp_server_communication_server_leader, std::to_string(ports[2]+ leader_id*10));
        threads.emplace_back(setup_tcp_server_communication_server_leader, std::to_string(ports[3]+ leader_id*10));
        threads.emplace_back(setup_tcp_server_communication_server_leader, std::to_string(ports[4]+ leader_id*10));
        threads.emplace_back(setup_tcp_server_communication_server_leader, std::to_string(ports[5]+ leader_id*10));
        threads.emplace_back(setup_tcp_server_communication_server_leader, std::to_string(ports[6]+ leader_id*10));
        threads.emplace_back(setup_tcp_server_communication_server_leader, std::to_string(ports[7]+ leader_id*10));
        threads.emplace_back(setup_tcp_server_communication_server_leader, std::to_string(ports[8]+ leader_id*10));
        threads.emplace_back(setup_tcp_server_communication_server_leader, std::to_string(ports[9]+ leader_id*10));
        std::string list_mem("list_mem");
        std::string list_self("list_self");
        std::string list_sus("enable suspicion");
        std::string list_no_sus("disable suspicion");
        std::string file_store("store");
        std::string file_ls("ls");

        
        threads.emplace_back(setup_tcp_server_communication_server, std::to_string(ports[0]+ -1*10));
        threads.emplace_back(setup_tcp_server_communication_server, std::to_string(ports[1]+ -1*10));
        threads.emplace_back(setup_tcp_server_communication_server, std::to_string(ports[2]+ -1*10));
        threads.emplace_back(setup_tcp_server_communication_server, std::to_string(ports[3]+ -1*10));
        threads.emplace_back(setup_tcp_server_communication_server, std::to_string(ports[4]+ -1*10));
        threads.emplace_back(setup_tcp_server_communication_server, std::to_string(ports[5]+ -1*10));
        threads.emplace_back(setup_tcp_server_communication_server, std::to_string(ports[6]+ -1*10));
        threads.emplace_back(setup_tcp_server_communication_server, std::to_string(ports[7]+ -1*10));
        threads.emplace_back(setup_tcp_server_communication_server, std::to_string(ports[8]+ -1*10));
        threads.emplace_back(setup_tcp_server_communication_server, std::to_string(ports[9]+ -1*10));
        while(1) {
            
            std::string buf;
        
            std::getline(std::cin, buf);
    
            /*Print out memory list*/
            if(buf == list_mem) {
                m.lock();
                for(int j = 0; j < ENTRIES_IN_ROW*NUM_MACHINES; j=j+ENTRIES_IN_ROW) {
                    if(mem_list[j] == NO_ENTRY) break;
                    printf("%d %d %d\n", mem_list[j], mem_list[j+HEARTBEAT_i], mem_list_times[(int(j/ENTRIES_IN_ROW))]);
                    
                }
                m.unlock();
                continue;
            }

            /*Print out self in memory list*/
            if(buf == list_self) {
                m.lock();
                printf("%d %d %d\n", mem_list[0], mem_list[HEARTBEAT_i], mem_list_times[0]);
                m.unlock();
                continue;
            }

            /*Enable Gossip w/ suspicion*/
            if(buf == list_sus) {
                m.lock();
                sus = 1;
                m.unlock();
                printf("Suspicion Enabled!\n");
                continue;
            }

            /*Switch to just Gossip*/
            else if(buf == list_no_sus) {
                m.lock();
                sus = 0;
                m.unlock();
                printf("Suspicion Disabled!\n");
                continue;
            }
            else if(buf == file_store){
                file_lock.lock();
                std::tuple<std::string, std::string, int>temp("", file_store, leader_id);
                working.push_back(temp);
                file_lock.unlock();
                while(1){
                    file_lock.lock();
                    if(working.front() == temp){
                        break;
                    }
                    file_lock.unlock();
                }
                std::string cur_files("\n");
                for (auto& it: machine_files[leader_id]) {
                    cur_files.append(it.first);
                    cur_files.append("\n");
                }
                std::cout << cur_files;
                working.pop_front();
                file_lock.unlock();
                printf("Finished Processing\n");
                continue;

            }

            int i = 0;
            for (i = 0; i < buf.length(); i++) 
                if (isspace(buf[i]))
                    break;
            if(i == (int)buf.length()){
                std::cout << "invalid command!" << "\n";
                continue;
            }
            std::string command = buf.substr(0, i);
            buf = buf.substr(i+1);


            if(command == file_ls){
                file_lock.lock();
                std::tuple<std::string, std::string, int> temp(buf, file_ls, leader_id);
                working.push_back(temp);
                file_lock.unlock();
                while(1){
                    file_lock.lock();
                    if(working.front() == temp){
                        break;
                    }
                    file_lock.unlock();
                }

                std::string to_return;
                if(file_to_machines.find(buf) == file_to_machines.end()){
                    to_return.append("No file found");
                }
                else {
                    std::tuple<int, int, int, int> mach_tuple(file_to_machines.at(buf));
                    to_return.append("Machine: ");
                    to_return.append(std::to_string(leader_id + 1));
                    to_return.append("\nMachine: ");
                    to_return.append(std::to_string(std::get<0>(mach_tuple) + 1));
                    to_return.append("\nMachine: ");
                    to_return.append(std::to_string(std::get<1>(mach_tuple) + 1));
                    to_return.append("\nMachine: ");
                    to_return.append(std::to_string(std::get<2>(mach_tuple) + 1));
                }

                std::cout << to_return << "\n";
                printf("Finished Processing\n");

                working.pop_front();
                file_lock.unlock();

            }

        }

    }
    else {
        // find client
        int i = 0;
        for(i = 0; i < (int)machine_names.size(); i++) {    
            if(!strcmp(host_name, machine_names[i].c_str())) {
                break;
            }
        }
        threads.emplace_back(setup_tcp_server_communication_server, std::to_string(ports[0]+ i*10));
        threads.emplace_back(setup_tcp_server_communication_server, std::to_string(ports[1]+ i*10));
        threads.emplace_back(setup_tcp_server_communication_server, std::to_string(ports[2]+ i*10));
        threads.emplace_back(setup_tcp_server_communication_server, std::to_string(ports[3]+ i*10));
        threads.emplace_back(setup_tcp_server_communication_server, std::to_string(ports[4]+ i*10));
        threads.emplace_back(setup_tcp_server_communication_server, std::to_string(ports[5]+ i*10));
        threads.emplace_back(setup_tcp_server_communication_server, std::to_string(ports[6]+ i*10));
        threads.emplace_back(setup_tcp_server_communication_server, std::to_string(ports[7]+ i*10));
        threads.emplace_back(setup_tcp_server_communication_server, std::to_string(ports[8]+ i*10));
        threads.emplace_back(setup_tcp_server_communication_server, std::to_string(ports[9]+ i*10));
        std::string port("4100");
        threads.emplace_back(setup_tcp_server_communication_server_maple, port);
        port = "4101";
        threads.emplace_back(setup_tcp_server_communication_server_juice, port);
            //printf("%s query of port num %d\n", host_name, 1601 + i);
        host_id = i;
        // Launch leader client
        while(1){
            setup_tcp_communication_client(machine_names[leader_id],  std::to_string(ports[i]), "", "", "leader");
        }
    }

    
    // threads.emplace_back(command_main);
    
    
    // threads[0].join();
    // threads[1].join();
    // threads[2].join();



   

    return 0;
    
}


