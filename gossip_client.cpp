#include "gossip.hpp"



/* udp_client()
* Client to send heartbeats.
* Args: DNS name of random machine picked to send heatbeat
*
*/
int udp_client(std::string server_ip) {
    int sockfd;
    struct addrinfo hints, *servinfo, *p; int rv;
    int numbytes;

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_INET; 
    hints.ai_socktype = SOCK_DGRAM;
    // get ip from DNS name
    if ((rv = getaddrinfo(server_ip.c_str(), "4950", &hints, &servinfo)) != 0) { 
        fprintf(stderr, "getaddrinfo: %s\n", 
        gai_strerror(rv));
        return 1;
    }

    // loop through all the results and make a socket
    for(p = servinfo; p != NULL; p = p->ai_next) {
        if ((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) { 
            perror("talker: socket"); 
            continue;
        }
        break; 
    }

    if (p == NULL) {
        fprintf(stderr, "talker: failed to create socket\n"); 
        return 2;
    }

    /*Send heartbeat to target */
    if ((numbytes = sendto(sockfd, mem_list, sizeof(mem_list), 0, p->ai_addr, p->ai_addrlen)) == -1) {
        perror("talker: sendto"); 
        exit(1);
    }

    freeaddrinfo(servinfo);
    // bytes += numbytes;
    // printf("talker: sent %d bytes to %s\n", numbytes, server_ip.c_str()); 
    close(sockfd);
    return 0;

}



/* client_main()
* Where the random index is chosen and failures are detected. (Where T_gossip, T_fail are used. T_cleanup is just 2*T_fail)
* Args: DNS name of random machine picked to send heatbeat
*
*/
void client_main() {
    // initialize clocks
    auto client_start = std::chrono::steady_clock::now();
    auto start = std::chrono::steady_clock::now();
    // int byte_flag = 0;
    /*Initilialize global variables to use*/
    memset(mem_list, NO_ENTRY, sizeof(mem_list));
    memset(mem_list_times, NO_ENTRY, sizeof(mem_list_times));

    // make this machine the "introducer"
    std::string introducer("fa23-cs425-1601.cs.illinois.edu");
    
    char host_name[50];
    gethostname(host_name, sizeof(host_name));
    std::string log_name("rm ");
    log_name.append(host_name);
    mem_file.append(host_name);
    log_name.append("mem_list.log");
    mem_file.append("mem_list.log");
    system(log_name.c_str());




    // put yourself at top of membership list
    for(int i = 0; i < (int)machine_names.size(); i++) {    
        if(!strcmp(host_name, machine_names[i].c_str())) {
            std::string mem(std::to_string(int(std::chrono::duration_cast<std::chrono::seconds>(client_start.time_since_epoch()).count())));
            mem.append(std::to_string(i+1));
            int test = std::stoi(mem);
            // printf("%d\n", test);
            mem_list[0] = test;
            mem_list[1] = 1;
            mem_list[2] = 1;
            std::ofstream fileOUT(mem_file, std::ios::app);
            std::string new_mem("Added: ");
            new_mem.append(std::to_string(mem_list[0]));
            new_mem.append(" 1 0");
            fileOUT << new_mem << "\n";
            fileOUT.close();
            int id = (mem_list[0]-1) % 10;
            machines_avail[id] = 1;
            break;
        }
    }

    // put introducer 2nd on membership list, and tell introducer your here
    /*Heatbeat introducer if not introducer (not really needed as while loop will do this)*/
    if(strcmp(host_name, introducer.c_str())) { // if not the introducer
        
        udp_client(introducer);
    }

    
    
    int gossipee;
    srand(time(NULL));


    
    


    while(1){
        
        /*Check if time to send heartbeat*/
        auto check = std::chrono::steady_clock::now();
        int index;
        if(std::chrono::duration<double>(check - start).count() >= T_GOSSIP) {
            m.lock();

           
            // Each node periodically increments its own heartbeat.
            
            // reset timer

            int i;
            int introducer_found = 0;

            // Gets size of mem_list and if introducer is in mem_list
            for(i = 0; i < NUM_MACHINES*ENTRIES_IN_ROW; i=i+ENTRIES_IN_ROW) {
                if(mem_list[i] == 0) break;
                if(mem_list[i] % 10 == 1) introducer_found = 1;
                
            }
            /*Call introducer if not found*/
            if(!introducer_found){
                udp_client(introducer);
            }


            /*Get i as number of machines in memory_list that are not itself*/
            i = i/ENTRIES_IN_ROW - 1;
            // if no other machines
            if(i==0) {
                if(strcmp(host_name, introducer.c_str())) { // if not the introducer
                    mem_list[HEARTBEAT_i] += 1;
                    udp_client(introducer);
                }
            }
            else{
                mem_list[HEARTBEAT_i] += 1;
                /*get random index in memory of other machines*/
                gossipee = ((rand() % i) + 1);
                /*Get machine number of mem_list*/
                index = (mem_list[gossipee*ENTRIES_IN_ROW] % 10) - 1;
                if(index == -1) {
                    index = 9;
                }
                // if(std::chrono::duration<double>(check - client_start).count() > 5 && std::chrono::duration<double>(check - client_start).count() < 55) {
                //     if(index == 0) {
                //         sends++;
                //     }
                // }

                /*Send the heartbeat*/
                udp_client(machine_names[index]);
                
    
            }
            m.unlock();
            
            start = std::chrono::steady_clock::now();
        }

        m.lock();
        /*Get time and update current machine's time (so is never mistakenly marked as dead)*/
        auto current_time = std::chrono::steady_clock::now();
        double time_passed = std::chrono::duration<double>(current_time - client_start).count();
        mem_list_times[0] = std::chrono::duration<double>(current_time - client_start).count();
        int j = 0;

        // if(byte_flag == 0 && std::chrono::duration<double>(current_time - client_start).count() > 60) {
        //     printf("%d\n", bytes);
        //     byte_flag = 1;
        // }
        /*For all rows in memory list, even uninitilaized (for our method, never assume more the 10 machines in memory list at a time)*/
        while(j < NUM_MACHINES*ENTRIES_IN_ROW+3) {
            if(mem_list[j] == NO_ENTRY) break;

            /*Mark machine as dead if T_FAIL time has passed*/
            if(mem_list[j+DEAD_i] != DEAD && time_passed - mem_list_times[(int(j/ENTRIES_IN_ROW))] > 2*T_FAIL) {
                
                mem_list[j+DEAD_i] = DEAD;
                if(sus){
                    printf("\nSuspected: %d has failed!\n", mem_list[j]);
                    std::ofstream fileOUT(mem_file, std::ios::app);
                    std::string new_mem("Suspected: ");
                    new_mem.append(std::to_string(mem_list[j]));
                    new_mem.append(" ");
                    new_mem.append(std::to_string(mem_list[j+HEARTBEAT_i]));
                    new_mem.append(" ");
                    new_mem.append(std::to_string(mem_list_times[int(j/ENTRIES_IN_ROW)]));
                    fileOUT << new_mem << "\n";
                    fileOUT.close();
                }
            }
            /*Remove machine from list if passed 2*T_Fail time (our T_clenaup)*/
            else if(time_passed - mem_list_times[(int(j/ENTRIES_IN_ROW))] > 3*T_FAIL && mem_list[j+DEAD_i] == DEAD){
                /*Remove from list and move down the mem_list (if more the 10 machines w/ clocks, will cause intermediate errors, but will eventually stabilize if only 10 machines )*/
                printf("\nRemoved: %d has failed!\n", mem_list[j]);
                std::ofstream fileOUT(mem_file, std::ios::app);
                std::string new_mem("Removed: ");
                new_mem.append(std::to_string(mem_list[j]));
                new_mem.append(" ");
                new_mem.append(std::to_string(mem_list[j+HEARTBEAT_i]));
                new_mem.append(" ");
                new_mem.append(std::to_string(mem_list_times[int(j/ENTRIES_IN_ROW)]));
                fileOUT << new_mem << "\n";
                fileOUT.close();
                int id = (mem_list[j]-1) % 10;
                machines_avail[id] = -1;
                mem_list[j] = NO_ENTRY;
                mem_list[j+1] = NO_ENTRY;
                mem_list[j+2] = NO_ENTRY;
                mem_list_times[(int(j/3))] = NO_ENTRY;    
                for(int k = j; k+ENTRIES_IN_ROW < (NUM_MACHINES*ENTRIES_IN_ROW)+3; k=k+ENTRIES_IN_ROW) {
                    // if(mem_list[k] == 0) break;
                    mem_list[k] = mem_list[k+3];
                    mem_list[k+HEARTBEAT_i] = mem_list[k+4];
                    mem_list[k+DEAD_i] = mem_list[k+5];
                    mem_list_times[(int(k/ENTRIES_IN_ROW))] = mem_list_times[(int((k+3)/ENTRIES_IN_ROW))];
                }
               
            }
            
            
   
            j=j+ENTRIES_IN_ROW;
        }
        m.unlock();
        
    }
}

