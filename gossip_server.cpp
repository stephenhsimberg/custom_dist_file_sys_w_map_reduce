#include "gossip.hpp"







/* udp_server()
* Server to recieve heartbeats from other machines and process them.
* Args:None (Every port is 4950)
*
*/
int udp_server() {
    auto start_server = std::chrono::steady_clock::now();
    int sockfd;
    struct addrinfo hints, *servinfo, *p; int rv;
    int numbytes;
    struct sockaddr_storage their_addr; char buf[100];
    socklen_t addr_len;
    char s[INET6_ADDRSTRLEN];
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_INET; 
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_flags = AI_PASSIVE; // use my IP

    // resolve host to read from port 4950
    if ((rv = getaddrinfo(NULL, "4950", &hints, &servinfo)) != 0) { 
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv)); return 1;
    }

    // loop through all the results and bind to the first we can
    for(p = servinfo; p != NULL; p = p->ai_next) {
        if ((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) { 
            perror("listener: socket"); 
            continue;
        }
        if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) { 
            close(sockfd);
            perror("listener: bind");
            continue; 
        }
        break; 
    }

    if (p == NULL) {
        fprintf(stderr, "listener: failed to bind socket\n"); 
        return 2;
    }
 
    freeaddrinfo(servinfo);
    // printf("listener: waiting to recvfrom...\n");

    /*Create a temp mem_list to hold sender's memory list*/
    int temp[30+3];
    memset(temp, NO_ENTRY, sizeof(temp));

    while(1) {

        // if machine we get message from is not in list, add it, else update it with ne time
        addr_len = sizeof their_addr;
        /*Wait here until heartbeat is received*/
        if ((numbytes = recvfrom(sockfd, temp, sizeof(temp), 0, (struct sockaddr *)&their_addr, &addr_len)) != -1) { 

            /*Get current time to set the time for the variables*/
            auto current_time = std::chrono::steady_clock::now();
            int time_ = std::chrono::duration<double>(current_time - start_server).count();

            /*Let i be the index counter for temp*/
            int i = 0;
            

            m.lock();
            
            while(i < NUM_MACHINES*ENTRIES_IN_ROW) {

                /*If have reached end of memory_list, break*/
                if(temp[i] == NO_ENTRY) {
                    break;
                }

                /*Check all values in memory list for senders current member being checked*/
                for(int j = 0; j < NUM_MACHINES*ENTRIES_IN_ROW; j=j+ENTRIES_IN_ROW) {

                    
                    // if not suspecting
                    if(!sus) {
                        // if entry from gossiper mem_list is in our mem_list and neither is dead
                        if(temp[i] == mem_list[j] && !(temp[i+DEAD_i] == DEAD || mem_list[j+DEAD_i] == DEAD)) {
                            if(temp[i+HEARTBEAT_i] > mem_list[j+HEARTBEAT_i]) {
                                // set biggest heartbeat counter
                                mem_list[j+HEARTBEAT_i] = temp[i+HEARTBEAT_i];
                                // set new local time
                                mem_list_times[int(j/ENTRIES_IN_ROW)] = time_;
                            }
                           
                            break;
                        }
                        // if in the memory lists but dead, do nothing
                        else if(temp[i] == mem_list[j] && (temp[i+DEAD_i] == DEAD || mem_list[j+DEAD_i] == DEAD)){
                            break;
                        }
                    }
                    // if in suspect mode
                    else {
                        // only check for the temp variable being dead
                        if(temp[i] == mem_list[j] && temp[i+DEAD_i] != DEAD) {
                            if(temp[i+HEARTBEAT_i] > mem_list[j+HEARTBEAT_i]) {
                                // set biggest heartbeat counter
                                mem_list[j+HEARTBEAT_i] = temp[i+HEARTBEAT_i];
                                // reset dead if was dead
                                mem_list[j+DEAD_i] = 1;
                                
                                mem_list_times[int(j/ENTRIES_IN_ROW)] = time_;
                            }
                            
                            break;
                        }
                        // if temp was dead do nothing
                        else if(temp[i] == mem_list[j] && temp[i+DEAD_i] == DEAD) {
                            break;
                        }
                    }

                    // if entry was not found in our membership list, add it
                    if(mem_list[j]==NO_ENTRY && temp[i+DEAD_i] != DEAD) {
                        mem_list[j] = temp[i];
                        mem_list[j+HEARTBEAT_i] = temp[i+HEARTBEAT_i];
                        mem_list[j+DEAD_i] = 1;
                        mem_list_times[int(j/ENTRIES_IN_ROW)] = time_;
                        std::ofstream fileOUT(mem_file, std::ios::app);
                        std::string new_mem("Added: ");
                        new_mem.append(std::to_string(temp[i]));
                        new_mem.append(" ");
                        new_mem.append(std::to_string(temp[i+HEARTBEAT_i]));
                        new_mem.append(" ");
                        new_mem.append(std::to_string(mem_list_times[int(j/ENTRIES_IN_ROW)]));
                        fileOUT << new_mem << "\n";
                        fileOUT.close();
                        int id = (temp[i]-1) % 10;
                        machines_avail[id] = 1;
                        break;
                    }

                    


                }

                i += ENTRIES_IN_ROW;
                
                
            }
            
            m.unlock();

            memset(temp, NO_ENTRY, sizeof(temp)); // reset temp memory list to 0
        }

        
    }
    /*Never reaches*/
    close(sockfd); 
    return 0;

}

