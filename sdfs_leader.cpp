#include "sdfs.hpp"




void *get_in_addr(struct sockaddr *sa) {
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }
    return &(((struct sockaddr_in6*)sa)->sin6_addr); 
}





/* setup_tcp_server_communication_server_leader(std::string port_num)
* Server to that functions as leader server based on leader id
* Args: Port, port is based on leader id * current port
*
*/
void setup_tcp_server_communication_server_leader(std::string port_num) {

    /*Local variables needed to set up server communication with clients*/
    char res_buf[10000];
    int sent_recv_bytes = 0;    // number of bytes from sned or recieve command
    int comm_socket_fd = 0;
    int processed = 0;          // Used for sending and recieving acknowldgements to (value not used)
 
    /*Folloing Beej's Guide*/
    struct sockaddr_storage client_addr; socklen_t addr_size;
    struct addrinfo hints, *servinfo, *p;
    int master_sock_tcp_fd, new_fd;
    int rv;
    int yes=1;
    char s[INET6_ADDRSTRLEN];
    memset(&hints, 0, sizeof hints); // make sure memory has been nullified
    hints.ai_family = AF_UNSPEC; // IPv4 or IPv6, 
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE; // fill in my IP for me
    
    // looks up DNS and server name lookups and fills out hint and servinfo structs
    if ((rv = getaddrinfo(NULL, port_num.c_str(), &hints, &servinfo)) != 0) { 
        return;
    }


        // loop through all the socket results, and binds to the first valid socket
    for(p = servinfo; p != NULL; p = p->ai_next) {
        if ((master_sock_tcp_fd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) { 
            perror("server: socket"); 
            continue;
        }
        // Allows for server to use socket still in kernel mem (allows it to bind)
        if (setsockopt(master_sock_tcp_fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1) {
            perror("setsockopt");
            exit(1); 
        }
        // test if will bind to server
        if (bind(master_sock_tcp_fd, p->ai_addr, p->ai_addrlen) == -1) {
            perror("server: bind");
            continue; 
        }
        break; 
    }
    // make a socket, bind it, and listen on it:
   freeaddrinfo(servinfo);

    if(p==NULL) {
        printf("server: failed to bind\n"); 
        // exit(1);
    }
    
    // can listen to up to 10 clinets (but right now only really supports 1)
    if (listen(master_sock_tcp_fd, 10) == -1) { 
        perror("listen");
        exit(1);
    }

    addr_size = sizeof client_addr;


    // printf("server: got connection from %s\n", s);
    char command[50];   
    char sys_file_name[200];
    char exe_char[200];
    char inter_prefix_char[200];
    char input_dir_char[200];
    char output_file_char[200];
    char input_dir2_char[200];
    std::string file_put("put");
    std::string file_get("get");
    std::string file_delete("delete");
    std::string file_store("store");
    std::string file_ls("ls");
    std::string maple_cmd("maple");
    std::string juice_cmd("juice");
    std::string SELECT("SELECT");
    std::string JOIN("JOIN");
    int check_f = 0;
    int max_r_bef_w = 5;
    int max_w_bef_r = 5;
    while(1) {

        if(check_f){
            close(comm_socket_fd);
        }
        else{
            check_f = 1;
        }
        
            // server must always be running to serve next grep, returns when client breaks out of loop
        comm_socket_fd = accept(master_sock_tcp_fd, (struct sockaddr *)&client_addr, &addr_size);
        // if (new_fd == -1) {
        //     perror("accept");
        // }

        // Convert IP addresses of client to correct form to use in send and recieve
        inet_ntop(client_addr.ss_family, get_in_addr((struct sockaddr *)&client_addr), s, sizeof s);


        // reset data buffer for next command  
        memset(command, 0, sizeof(command));
        memset(sys_file_name, 0, sizeof(sys_file_name));
        memset(exe_char, 0, sizeof(exe_char));
        memset(inter_prefix_char, 0, sizeof(inter_prefix_char));
        memset(input_dir_char, 0, sizeof(input_dir_char));
        memset(output_file_char, 0, sizeof(output_file_char));
        memset(input_dir2_char, 0, sizeof(input_dir2_char));



        // recv blocks until data arrives from client with command
        sent_recv_bytes = recv(comm_socket_fd, (char *) command, sizeof(command), 0);
    
        // printf("Server got %d bytes from client \n", sent_recv_bytes);
        if(sent_recv_bytes == 0) {
            // close server if no bytes recieved and wait for next connection
            // close(comm_socket_fd);
            continue;
        }

        sent_recv_bytes = send(comm_socket_fd, (int *)&processed, sizeof(processed), 0); // send acknowledgement
        std::string c_op(command);
        
        int com_id = std::stoi(port_num) % 10; // get id of machine to differentiate when adding to work queue

        if(c_op == JOIN){
            sent_recv_bytes = recv(comm_socket_fd, (char *) exe_char, sizeof(exe_char), 0);
            std::string maple_exe(exe_char);
            sent_recv_bytes = send(comm_socket_fd, (int *)&processed, sizeof(processed), 0); // send acknowledgement

            int num_tasks;
            sent_recv_bytes = recv(comm_socket_fd, (int *)&num_tasks, sizeof(num_tasks), 0);
            sent_recv_bytes = send(comm_socket_fd, (int *)&processed, sizeof(processed), 0); // send acknowledgement

            sent_recv_bytes = recv(comm_socket_fd, (char *) inter_prefix_char, sizeof(inter_prefix_char), 0);
            std::string inter_prefix(inter_prefix_char);
            sent_recv_bytes = send(comm_socket_fd, (int *)&processed, sizeof(processed), 0); // send acknowledgement

            sent_recv_bytes = recv(comm_socket_fd, (char *) input_dir_char, sizeof(input_dir_char), 0);
            std::string input_dir(input_dir_char);
            sent_recv_bytes = send(comm_socket_fd, (int *)&processed, sizeof(processed), 0); // send acknowledgement

            sent_recv_bytes = recv(comm_socket_fd, (char *) input_dir2_char, sizeof(input_dir2_char), 0);
            std::string input_dir2(input_dir_char);


            std::string f_name("sysfiles/" + input_dir);
            std::ifstream file1(f_name, std::ios::in | std::ios::binary);


            std::string f2_name("sysfiles/" + input_dir2);
            std::ifstream file2(f2_name, std::ios::in | std::ios::binary);
            std::string combined;
            std::string temp1;
            std::string temp2;
            while(std::getline(file1, temp1) && std::getline(file2, temp2)){
                combined.append(temp1 + "\t" + temp2 + "\n");
                // std::cout << temp1 << "\t" << temp2 << "\n";
            }
            file1.close();
            file2.close();

            std::string combined_name("combined");
            std::string file_name("sysfiles/combined");
            std::ofstream ofs;
            ofs.open(file_name, std::ofstream::out | std::ofstream::trunc);
            ofs.close();
            std::ofstream fileOUT(file_name);
            fileOUT << combined;
            fileOUT.close();


            int maple_return = maple_leader(comm_socket_fd, maple_exe, num_tasks, inter_prefix, combined_name);
            sent_recv_bytes = send(comm_socket_fd, (int *)&processed, sizeof(processed), 0); // send acknowledgement

            memset(exe_char, 0, sizeof(exe_char));
            memset(inter_prefix_char, 0, sizeof(inter_prefix_char));
            memset(input_dir_char, 0, sizeof(input_dir_char));
            memset(output_file_char, 0, sizeof(output_file_char));



            // run juice
            sent_recv_bytes = recv(comm_socket_fd, (char *) exe_char, sizeof(exe_char), 0);
            std::string juice_exe(exe_char);
            std::cout << "JUICE: " << juice_exe << "\n";
            sent_recv_bytes = send(comm_socket_fd, (int *)&processed, sizeof(processed), 0); // send acknowledgement

        
            sent_recv_bytes = recv(comm_socket_fd, (int *)&num_tasks, sizeof(num_tasks), 0);
            sent_recv_bytes = send(comm_socket_fd, (int *)&processed, sizeof(processed), 0); // send acknowledgement
            std::cout << "JUICE: " << num_tasks << "\n";
            sent_recv_bytes = recv(comm_socket_fd, (char *) inter_prefix_char, sizeof(inter_prefix_char), 0);
            std::string inter_prefix_j(inter_prefix_char);
            sent_recv_bytes = send(comm_socket_fd, (int *)&processed, sizeof(processed), 0); // send acknowledgement
            std::cout << "JUICE: " << inter_prefix_j << "\n";
            sent_recv_bytes = recv(comm_socket_fd, (char *) output_file_char, sizeof(output_file_char), 0);
            std::string output_file(output_file_char);
            std::cout << "JUICE: " << output_file << "\n";
            sent_recv_bytes = send(comm_socket_fd, (int *)&processed, sizeof(processed), 0); // send acknowledgement

            int to_delete;
            sent_recv_bytes = recv(comm_socket_fd, (int *)&to_delete, sizeof(to_delete), 0);



            int juice_return = juice_leader(comm_socket_fd, juice_exe, num_tasks, inter_prefix_j, output_file, to_delete);

            std::ifstream file(output_file, std::ios::in | std::ios::binary);
            std::string maplejuice_output((std::istreambuf_iterator<char>(file)),(std::istreambuf_iterator<char>()));


            const char* data_ptr  = maplejuice_output.data();
            std::size_t bytes_to_process = maplejuice_output.length();

            sent_recv_bytes = send(comm_socket_fd, (size_t *)&bytes_to_process, sizeof(bytes_to_process), 0);

            sent_recv_bytes = recv(comm_socket_fd, &processed, sizeof(processed), 0); // recieve acknoledgement
            while(bytes_to_process > 0) {

                // send data, incrementing through pointer to string
                sent_recv_bytes = send(comm_socket_fd, data_ptr, bytes_to_process, 0);
                // subtract number of bytes left to process
                bytes_to_process -= sent_recv_bytes;
                // add to data pointer so starts where last send left off
                data_ptr += sent_recv_bytes;
            }

            sent_recv_bytes = recv(comm_socket_fd, &processed, sizeof(processed), 0); // recieve acknoledgement
            sent_recv_bytes =  send(comm_socket_fd, (int *)&processed, sizeof(processed), 0); // sync
            sent_recv_bytes = recv(comm_socket_fd, &processed, sizeof(processed), 0); // recieve acknoledgement
            continue;





        }

        if(c_op == SELECT){
            sent_recv_bytes = recv(comm_socket_fd, (char *) exe_char, sizeof(exe_char), 0);
            std::string maple_exe(exe_char);
            sent_recv_bytes = send(comm_socket_fd, (int *)&processed, sizeof(processed), 0); // send acknowledgement

            int num_tasks;
            sent_recv_bytes = recv(comm_socket_fd, (int *)&num_tasks, sizeof(num_tasks), 0);
            sent_recv_bytes = send(comm_socket_fd, (int *)&processed, sizeof(processed), 0); // send acknowledgement

            sent_recv_bytes = recv(comm_socket_fd, (char *) inter_prefix_char, sizeof(inter_prefix_char), 0);
            std::string inter_prefix(inter_prefix_char);
            sent_recv_bytes = send(comm_socket_fd, (int *)&processed, sizeof(processed), 0); // send acknowledgement

            sent_recv_bytes = recv(comm_socket_fd, (char *) input_dir_char, sizeof(input_dir_char), 0);
            std::string input_dir(input_dir_char);

            int maple_return = maple_leader(comm_socket_fd, maple_exe, num_tasks, inter_prefix, input_dir);
            sent_recv_bytes = send(comm_socket_fd, (int *)&processed, sizeof(processed), 0); // send acknowledgement

            memset(exe_char, 0, sizeof(exe_char));
            memset(inter_prefix_char, 0, sizeof(inter_prefix_char));
            memset(input_dir_char, 0, sizeof(input_dir_char));
            memset(output_file_char, 0, sizeof(output_file_char));



            // run juice
            sent_recv_bytes = recv(comm_socket_fd, (char *) exe_char, sizeof(exe_char), 0);
            std::string juice_exe(exe_char);
            std::cout << "JUICE: " << juice_exe << "\n";
            sent_recv_bytes = send(comm_socket_fd, (int *)&processed, sizeof(processed), 0); // send acknowledgement

        
            sent_recv_bytes = recv(comm_socket_fd, (int *)&num_tasks, sizeof(num_tasks), 0);
            sent_recv_bytes = send(comm_socket_fd, (int *)&processed, sizeof(processed), 0); // send acknowledgement
            std::cout << "JUICE: " << num_tasks << "\n";
            sent_recv_bytes = recv(comm_socket_fd, (char *) inter_prefix_char, sizeof(inter_prefix_char), 0);
            std::string inter_prefix_j(inter_prefix_char);
            sent_recv_bytes = send(comm_socket_fd, (int *)&processed, sizeof(processed), 0); // send acknowledgement
            std::cout << "JUICE: " << inter_prefix_j << "\n";
            sent_recv_bytes = recv(comm_socket_fd, (char *) output_file_char, sizeof(output_file_char), 0);
            std::string output_file(output_file_char);
            std::cout << "JUICE: " << output_file << "\n";
            sent_recv_bytes = send(comm_socket_fd, (int *)&processed, sizeof(processed), 0); // send acknowledgement

            int to_delete;
            sent_recv_bytes = recv(comm_socket_fd, (int *)&to_delete, sizeof(to_delete), 0);



            int juice_return = juice_leader(comm_socket_fd, juice_exe, num_tasks, inter_prefix_j, output_file, to_delete);

            std::ifstream file(output_file, std::ios::in | std::ios::binary);
            std::string maplejuice_output((std::istreambuf_iterator<char>(file)),(std::istreambuf_iterator<char>()));


            const char* data_ptr  = maplejuice_output.data();
            std::size_t bytes_to_process = maplejuice_output.length();

            sent_recv_bytes = send(comm_socket_fd, (size_t *)&bytes_to_process, sizeof(bytes_to_process), 0);

            sent_recv_bytes = recv(comm_socket_fd, &processed, sizeof(processed), 0); // recieve acknoledgement
            while(bytes_to_process > 0) {

                // send data, incrementing through pointer to string
                sent_recv_bytes = send(comm_socket_fd, data_ptr, bytes_to_process, 0);
                // subtract number of bytes left to process
                bytes_to_process -= sent_recv_bytes;
                // add to data pointer so starts where last send left off
                data_ptr += sent_recv_bytes;
            }

            sent_recv_bytes = recv(comm_socket_fd, &processed, sizeof(processed), 0); // recieve acknoledgement
            sent_recv_bytes =  send(comm_socket_fd, (int *)&processed, sizeof(processed), 0); // sync
            sent_recv_bytes = recv(comm_socket_fd, &processed, sizeof(processed), 0); // recieve acknoledgement
            continue;





        }


        if(c_op == maple_cmd){

            sent_recv_bytes = recv(comm_socket_fd, (char *) exe_char, sizeof(exe_char), 0);
            std::string maple_exe(exe_char);
            sent_recv_bytes = send(comm_socket_fd, (int *)&processed, sizeof(processed), 0); // send acknowledgement

            int num_tasks;
            sent_recv_bytes = recv(comm_socket_fd, (int *)&num_tasks, sizeof(num_tasks), 0);
            sent_recv_bytes = send(comm_socket_fd, (int *)&processed, sizeof(processed), 0); // send acknowledgement

            sent_recv_bytes = recv(comm_socket_fd, (char *) inter_prefix_char, sizeof(inter_prefix_char), 0);
            std::string inter_prefix(inter_prefix_char);
            sent_recv_bytes = send(comm_socket_fd, (int *)&processed, sizeof(processed), 0); // send acknowledgement

            sent_recv_bytes = recv(comm_socket_fd, (char *) input_dir_char, sizeof(input_dir_char), 0);
            std::string input_dir(input_dir_char);

            int maple_return = maple_leader(comm_socket_fd, maple_exe, num_tasks, inter_prefix, input_dir);


            sent_recv_bytes = send(comm_socket_fd, (int *)&processed, sizeof(processed), 0); // send acknowledgement
            continue;


        }

        if(c_op == juice_cmd){
            sent_recv_bytes = recv(comm_socket_fd, (char *) exe_char, sizeof(exe_char), 0);
            std::string juice_exe(exe_char);
            sent_recv_bytes = send(comm_socket_fd, (int *)&processed, sizeof(processed), 0); // send acknowledgement

            int num_tasks;
            sent_recv_bytes = recv(comm_socket_fd, (int *)&num_tasks, sizeof(num_tasks), 0);
            sent_recv_bytes = send(comm_socket_fd, (int *)&processed, sizeof(processed), 0); // send acknowledgement

            sent_recv_bytes = recv(comm_socket_fd, (char *) inter_prefix_char, sizeof(inter_prefix_char), 0);
            std::string inter_prefix(inter_prefix_char);
            sent_recv_bytes = send(comm_socket_fd, (int *)&processed, sizeof(processed), 0); // send acknowledgement

            sent_recv_bytes = recv(comm_socket_fd, (char *) output_file_char, sizeof(output_file_char), 0);
            std::string output_file(output_file_char);
            sent_recv_bytes = send(comm_socket_fd, (int *)&processed, sizeof(processed), 0); // send acknowledgement

            int to_delete;
            sent_recv_bytes = recv(comm_socket_fd, (int *)&to_delete, sizeof(to_delete), 0);



            int maple_return = juice_leader(comm_socket_fd, juice_exe, num_tasks, inter_prefix, output_file, to_delete);

            std::ifstream file(output_file, std::ios::in | std::ios::binary);
            std::string maplejuice_output((std::istreambuf_iterator<char>(file)),(std::istreambuf_iterator<char>()));


            const char* data_ptr  = maplejuice_output.data();
            std::size_t bytes_to_process = maplejuice_output.length();

            sent_recv_bytes = send(comm_socket_fd, (size_t *)&bytes_to_process, sizeof(bytes_to_process), 0);

            sent_recv_bytes = recv(comm_socket_fd, &processed, sizeof(processed), 0); // recieve acknoledgement
            while(bytes_to_process > 0) {

                // send data, incrementing through pointer to string
                sent_recv_bytes = send(comm_socket_fd, data_ptr, bytes_to_process, 0);
                // subtract number of bytes left to process
                bytes_to_process -= sent_recv_bytes;
                // add to data pointer so starts where last send left off
                data_ptr += sent_recv_bytes;
            }

            sent_recv_bytes = recv(comm_socket_fd, &processed, sizeof(processed), 0); // recieve acknoledgement
            sent_recv_bytes =  send(comm_socket_fd, (int *)&processed, sizeof(processed), 0); // sync
            sent_recv_bytes = recv(comm_socket_fd, &processed, sizeof(processed), 0); // recieve acknoledgement
            continue;

        }



        if(c_op == file_store){
            file_lock.lock();
            std::tuple<std::string, std::string, int> temp("", file_store, com_id); // add request to queue
            working.push_back(temp);
            file_lock.unlock();
            while(1){
                file_lock.lock();
                if(working.front() == temp){ // if add top of queue break
                    break;
                }
                file_lock.unlock();
            }
            // get files stored on current machine, give to client to print
            int store_mach = 0;
            sent_recv_bytes = recv(comm_socket_fd, (int *)&store_mach, sizeof(store_mach), 0);
            std::string cur_files("\n");
            for (auto& it: machine_files[store_mach]) {
                cur_files.append(it.first);
                cur_files.append("\n");
            }

            working.pop_front();
            file_lock.unlock();

            sent_recv_bytes = send(comm_socket_fd, cur_files.c_str(), cur_files.length(), 0);
            sent_recv_bytes = recv(comm_socket_fd, (int *)&processed, sizeof(processed), 0); // syncronize the client and server
            // printf("YAY\n");
            continue;



        }

        // get system file name
        sent_recv_bytes = recv(comm_socket_fd, (char *) sys_file_name, sizeof(sys_file_name), 0);
        sent_recv_bytes = send(comm_socket_fd, (int *)&processed, sizeof(processed), 0); // send acknowledgement
        std::string file_name(sys_file_name);
        std::string f_name("sysfiles/");
        f_name.append(sys_file_name);

        
        // get string to give client of where file requested is stored
        if(c_op == file_ls){
            file_lock.lock();
            std::tuple<std::string, std::string, int> temp(file_name, file_ls, com_id);

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
            if(file_to_machines.find(file_name) == file_to_machines.end()){
                to_return.append("No file found");
            }
            else {
                std::tuple<int, int, int, int> mach_tuple(file_to_machines.at(file_name));
                to_return.append("Machine: ");
                to_return.append(std::to_string(leader_id + 1));
                to_return.append("\nMachine: ");
                to_return.append(std::to_string(std::get<0>(mach_tuple) + 1));
                to_return.append("\nMachine: ");
                to_return.append(std::to_string(std::get<1>(mach_tuple) + 1));
                to_return.append("\nMachine: ");
                to_return.append(std::to_string(std::get<2>(mach_tuple) + 1));
            }

            working.pop_front();
            file_lock.unlock();

            sent_recv_bytes = send(comm_socket_fd, to_return.c_str(), to_return.length(), 0);
            sent_recv_bytes = recv(comm_socket_fd, (int *)&processed, sizeof(processed), 0); // syncronize the client and server
            // printf("YAY\n");
            continue;
        }



        // if reading:
        else if(c_op == file_get){

            file_lock.lock();
            std::tuple<std::string, std::string, int> temp(file_name, file_get, com_id);
            int count = 0;
            int inserted = 0;
            // if find more then 4 consecutive writes on file, put before 5th write
            for (auto it = working.begin(); it != working.end(); ++it) {
                if (std::get<0>(*it) == file_name && std::get<1>(*it) == file_get) {
                    count++;
                    if (count == max_w_bef_r) {
                        working.insert(it, temp); // insert element before 5th write to that same file
                        inserted = 1;
                        break;
                    }
                }
                else if (std::get<0>(*it) == file_name && std::get<1>(*it) == file_put) {
                    count = 0;
                }
            }
            if(inserted == 0){ // push to back of queue if good
                working.push_back(temp);
            }

            file_lock.unlock();
            sent_recv_bytes = recv(comm_socket_fd, (int *)&processed, sizeof(processed), 0); // syncronize the client and server

            // loop until at top of queue
            while(1){
                file_lock.lock();
                if(working.front() == temp){
                    if(file_to_machines.find(file_name) != file_to_machines.end()){
                        std::tuple<int, int, int, int> mach_tuple(file_to_machines.at(file_name));
                        if(std::get<3>(mach_tuple) == 2){ //another thread is writing to that file
                            file_lock.unlock();
                            continue;
                        }
                    }
                    break;
                }
                file_lock.unlock();
            }

            // open system file
            std::ifstream file(f_name.c_str(), std::ios::in | std::ios::binary);
            if(file.good()){
                file.clear();
                file.seekg(0);

                // read through file
                std::string file_string_leader((std::istreambuf_iterator<char>(file)),(std::istreambuf_iterator<char>()));
                file.close();
                working.pop_front();
                file_lock.unlock(); // pop here since got string needed


                const char* data_ptr  = file_string_leader.data();
                std::size_t bytes_to_process = file_string_leader.length();
                int for_process = (int) bytes_to_process;


                sent_recv_bytes = send(comm_socket_fd, (int *)&for_process, sizeof(for_process), 0);

                sent_recv_bytes = recv(comm_socket_fd, &processed, sizeof(processed), 0); // recieve acknoledgement



                // WHile loop to  send the TOTAL string to client by the number of bytes
                while(bytes_to_process > 0) {

                    // send data, incrementing through pointer to string
                    sent_recv_bytes = send(comm_socket_fd, data_ptr, bytes_to_process, 0);
                    // subtract number of bytes left to process
                    bytes_to_process -= sent_recv_bytes;
                    // add to data pointer so starts where last send left off
                    data_ptr += sent_recv_bytes;
                }

                sent_recv_bytes = recv(comm_socket_fd, &processed, sizeof(processed), 0); // sync
                file_string_leader.clear();
            


                



            }
            // file did not exist
            else{
                working.pop_front();
                file_lock.unlock();
                int bad_file = -1;
                sent_recv_bytes = send(comm_socket_fd, (int *)&bad_file, sizeof(bad_file), 0);

                sent_recv_bytes = recv(comm_socket_fd, &processed, sizeof(processed), 0); // recieve acknoledgement


            }

            // sent_recv_bytes = send(comm_socket_fd, (int *)&processed, sizeof(processed), 0); // send acknowledgement
            //printf("YAY\n");
            continue;
        }



        // if client wants to write file
        else if(c_op == file_put) {
            // put on queue
            file_lock.lock();
            std::tuple<std::string, std::string, int> temp_w(file_name, file_put, com_id);
            int count = 0;
            int inserted = 0;

            // if more than 4 consecutive reads on same file, put write before 5th read
            for (auto it = working.begin(); it != working.end(); ++it) {
                if (std::get<0>(*it) == file_name && std::get<1>(*it) == file_put) {
                    count++;
                    if (count == max_r_bef_w) {
                        working.insert(it, temp_w); // insert this write before the 5th read of same file
                        inserted = 1;
                        break;
                    }
                }
                else if (std::get<0>(*it) == file_name && std::get<1>(*it) == file_get) {
                    count = 0;
                }
            }

            if(inserted == 0){
                working.push_back(temp_w);
            }

            file_lock.unlock();
            sent_recv_bytes = recv(comm_socket_fd, (int *)&processed, sizeof(processed), 0); // syncronize the client and server

            // wait until at top of queue
            while(1){
                file_lock.lock();

                if(working.front() == temp_w){
                    if(file_to_machines.find(file_name) != file_to_machines.end()){
                        std::tuple<int, int, int, int> mach_tuple(file_to_machines.at(file_name));
                        if(std::get<3>(mach_tuple) == 2){ //another thread is writing to that file
                            file_lock.unlock();
                            continue;
                        }
                    }
                    break;
                }
                file_lock.unlock();
            }

            int start_flag = 1;


            // std::ifstream file(f_name.c_str());
            
            // if file is saved of system
            // look in map to see other ids in system
            int file_new = 0;
            // do while loop so if machine fails while writing, able to recover from it 
            while(1){
                if(!start_flag){
                    file_lock.lock();
                }
                std::string mach_w_file;

                // if file has not been currently stored, randomly choose 3 machines to put on based on machines available
                if(file_to_machines.find(file_name) == file_to_machines.end()){
                    // randomly choose 3 machines to put machine on
                    file_new = 1;
                    std::vector<int> choose;
                    for(int i = 0; i < 10; i++){
                        if(machines_avail[i] == 1 && i != leader_id){
                            choose.push_back(i);
                        }
                    }
                    int num = (int)choose.size();

                    int rand_mach1 = (rand() % num);
                    mach_w_file.append(std::to_string(choose[rand_mach1]));
                    int rand_mach2 = (rand() % num);
                    while(rand_mach2 == rand_mach1){
                        rand_mach2 = (rand() % num);
                    }
                    mach_w_file.append(std::to_string(choose[rand_mach2]));
                    int rand_mach3 = (rand() % num);
                    while(rand_mach3 == rand_mach1 || rand_mach3 == rand_mach2){
                        rand_mach3 = (rand() % num);
                    }
                    mach_w_file.append(std::to_string(choose[rand_mach3]));

                    std::tuple<int, int, int, int> mach_tuple(choose[rand_mach1], choose[rand_mach2], choose[rand_mach3], 2);
                    //write new machines into data
                    file_to_machines[file_name] = mach_tuple;

                }
                // read machines from map if file is stored
                else {
                    std::tuple<int, int, int, int> mach_tuple(file_to_machines.at(file_name));
                    mach_w_file.append(std::to_string(std::get<0>(mach_tuple)));
                    mach_w_file.append(std::to_string(std::get<1>(mach_tuple)));
                    mach_w_file.append(std::to_string(std::get<2>(mach_tuple)));
                    std::tuple<int, int, int, int> temp_tuple(std::get<0>(mach_tuple), std::get<1>(mach_tuple), std::get<2>(mach_tuple), 2);
                    file_to_machines[file_name] = temp_tuple;

            
                }
                if(start_flag){
                    working.pop_front();
                    file_lock.unlock();
                    start_flag = 0;
                }
                else{
                    file_lock.unlock();
                }

                sent_recv_bytes = send(comm_socket_fd, mach_w_file.c_str(), mach_w_file.length(), 0);


                // read file from client since leader

                std::size_t file_size = 0;
                sent_recv_bytes =  recv(comm_socket_fd, (size_t *)&file_size, sizeof(file_size), 0);
                sent_recv_bytes = send(comm_socket_fd, (int *)&processed, sizeof(processed), 0); // send acknowledgement

                
                std::string file_string_leader;
                while(file_size > 0) {
                    memset(res_buf, 0, sizeof(res_buf)); // reset buffer

                    sent_recv_bytes =  recv(comm_socket_fd, (char *)res_buf, sizeof(res_buf), 0);
                    // check if bytes sent within bounds (no weird tcp errors)
                    if(file_size >= sent_recv_bytes) { 
                        file_string_leader.append(res_buf, sent_recv_bytes); // this was the solution to large files
                    
                    }
                
                    // reset all buffers
                    memset(res_buf, 0, sizeof(res_buf)); // reset buffer (just in case)

                    file_size -= sent_recv_bytes;
                
                    
                }
                std::ofstream ofs;
                ofs.open(f_name, std::ofstream::out | std::ofstream::trunc);
                ofs.close();
                std::ofstream fileOUT(f_name);
                fileOUT << file_string_leader;
                fileOUT.close();
                int finished_file = 1;

                sent_recv_bytes =  send(comm_socket_fd, (int *)&processed, sizeof(processed), 0); // recieve acknowlegement

                // restart process if any servers writing to client failed
                int test = 0;

                // gets confirmation that all machines have been finished being written into
                sent_recv_bytes = recv(comm_socket_fd, (int *)&test, sizeof(test), 0); // All files have been placed
                if(test == 1){
                    continue;
                }
                else {
                    break;
                }
            }

            // since all machines finished, say write is done and add to list
            file_lock.lock();
            std::tuple<int, int, int, int> mach_tuple(file_to_machines.at(file_name));
            int machs[3];
            machs[0] = std::get<0>(mach_tuple);
            machs[1] = std::get<1>(mach_tuple);
            machs[2] = std::get<2>(mach_tuple);
            std::tuple<int, int, int, int> temp(std::tuple<int, int, int, int>(machs[0], machs[1], machs[2], 0));
            file_to_machines[file_name] = temp;
            if(file_new){
                for(int i = 0; i < 3; i++){
                    machine_files[machs[i]][file_name] = 0;
                }
                machine_files[leader_id][file_name] = 0;
            }
            file_lock.unlock();
            sent_recv_bytes =  send(comm_socket_fd, (int *)&processed, sizeof(processed), 0);
            // printf("YAY\n");
            continue;




    



        }

        // delete file
        else if(c_op == file_delete){

            // first add to queue
            file_lock.lock();
            std::tuple<std::string, std::string, int> temp_w(file_name, file_delete, com_id);
            working.push_back(temp_w);
            file_lock.unlock();
            sent_recv_bytes = recv(comm_socket_fd, (int *)&processed, sizeof(processed), 0); // syncronize the client and server
            while(1){
                file_lock.lock();
                if(working.front() == temp_w){
                    if(file_to_machines.find(file_name) != file_to_machines.end()){
                        std::tuple<int, int, int, int> mach_tuple(file_to_machines.at(file_name));
                        if(std::get<3>(mach_tuple) == 2){ //another thread is writing to that file
                            file_lock.unlock();
                            continue;
                        }
                    }
                    break;
                }
                file_lock.unlock();
            }

            std::string mach_w_file;

            // file to be deleted not found
            if(file_to_machines.find(file_name) == file_to_machines.end()){
                int bad_file = -1;
                working.pop_front();
                file_lock.unlock();
                sent_recv_bytes = send(comm_socket_fd, (int *)&bad_file, sizeof(bad_file), 0);

                sent_recv_bytes = recv(comm_socket_fd, &processed, sizeof(processed), 0); // recieve acknoledgement

            }
            else {
                // send all files to delete, and remove from map and machine files map.
                file_lock.unlock();
                int good_file = 1;
                sent_recv_bytes = send(comm_socket_fd, (int *)&good_file, sizeof(good_file), 0);
                sent_recv_bytes = recv(comm_socket_fd, (int *)&processed, sizeof(processed), 0); // syncronize the client and server
                file_lock.lock();
                std::tuple<int, int, int, int> mach_tuple(file_to_machines.at(file_name));
                mach_w_file.append(std::to_string(std::get<0>(mach_tuple)));
                mach_w_file.append(std::to_string(std::get<1>(mach_tuple)));
                mach_w_file.append(std::to_string(std::get<2>(mach_tuple)));
                file_lock.unlock();

                sent_recv_bytes = send(comm_socket_fd, mach_w_file.c_str(), mach_w_file.length(), 0);
                std::string to_delete("rm sysfiles/");
                to_delete.append(file_name);
                system(to_delete.c_str());
                // recieves notification that client has finished delete threads
                sent_recv_bytes = recv(comm_socket_fd, (int *)&processed, sizeof(processed), 0); // syncronize the client and server

                file_lock.lock();
                int machs[3];
                machs[0] = std::get<0>(mach_tuple);
                machs[1] = std::get<1>(mach_tuple);
                machs[2] = std::get<2>(mach_tuple);
                std::tuple<int, int, int, int> temp(std::tuple<int, int, int, int>(machs[0], machs[1], machs[2], 0));
                file_to_machines[file_name] = temp;
           
                for(int i = 0; i < 3; i++){
                    machine_files[machs[i]].erase(file_name);
                }
                machine_files[leader_id].erase(file_name);
                
                file_to_machines.erase(file_name);

                working.pop_front();
                file_lock.unlock();
                sent_recv_bytes =  send(comm_socket_fd, (int *)&processed, sizeof(processed), 0);
            }
            //printf("YAY\n");
            continue;
            
            
        }



    }

}





/* write_to_self(std::string sys_name)
* Helper function to write file onto self if host_id is SELECTed as file
* Args: sys_name to save local file as
*
*/
void write_to_self(std::string sys_name){
     std::string f_name("sysfiles/");
    f_name.append(sys_name);
    std::ofstream ofs;
    ofs.open(f_name, std::ofstream::out | std::ofstream::trunc);
    ofs.close();
    std::ofstream fileOUT(f_name);
    fileOUT << file_string;
    fileOUT.close();
}

/* leader_write(int socket_fd)
* Helper function to save file if is the leader we need to save file on
* Args: sys_name to save local file as
*
*/
void leader_write(int socket_fd){
    const char* data_ptr  = file_string.data();
    int sent_recv_bytes;
    int processed = 1;
    std::size_t bytes_to_process = file_string.length();


    sent_recv_bytes = send(socket_fd, (size_t *)&bytes_to_process, sizeof(bytes_to_process), 0);

    sent_recv_bytes = recv(socket_fd, &processed, sizeof(processed), 0); // recieve acknoledgement

    while(bytes_to_process > 0) {

        // send data, incrementing through pointer to string
        sent_recv_bytes = send(socket_fd, data_ptr, bytes_to_process, 0);
        // subtract number of bytes left to process
        bytes_to_process -= sent_recv_bytes;
        // add to data pointer so starts where last send left off
        data_ptr += sent_recv_bytes;
    }

    sent_recv_bytes = recv(socket_fd, &processed, sizeof(processed), 0); // sync
}





// Used to replicate files on failure
void recover_files(int mach_number){
    std::string file_put("put");

    int counter = 0;
    std::vector<std::thread> threads;
    for (auto& it: machine_files[mach_number]) {
        // Do stuff
        while(1){
            std::tuple<int, int, int, int> mach_tuple(file_to_machines.at(it.first));
            int machs[3];
            machs[0] = std::get<0>(mach_tuple);
            machs[1] = std::get<1>(mach_tuple);
            machs[2] = std::get<2>(mach_tuple);

            std::vector<int> choose;
            for(int i = 0; i < 10; i++){
                if(machines_avail[i] == 1 && i != leader_id && i != machs[0] && i != machs[1] && i != machs[2]){
                    choose.push_back(i);
                }
            }
            int num = (int)choose.size();
            int rand_mach = (rand() % num);
            std::string to_file("sysfiles/");
            to_file.append(it.first);
            setup_tcp_communication_client(machine_names[choose[rand_mach]], std::to_string(ports[leader_id]+ choose[rand_mach]*10), it.first, to_file, file_put);

            if(file_fixed == 1){
                if(machs[0] == mach_number){
                    machs[0] = choose[rand_mach];
                    machine_files[machs[0]][it.first] = 0;
                }
                else if(machs[1] == mach_number){
                    machs[1] = choose[rand_mach];
                    machine_files[machs[1]][it.first] = 0;
                }
                else if(machs[2] == mach_number){
                    machs[2] = choose[rand_mach];
                    machine_files[machs[2]][it.first] = 0;
                }
                std::tuple<int, int, int, int> mach_tuple(machs[0], machs[1], machs[2], 0);
                file_to_machines[it.first] = mach_tuple;
                file_fixed = 0;
                break;

            }
        }
        
    }
    // reset machine
    machine_files[mach_number].clear();
    machines_avail[mach_number] = 0;
}


// constantly looping to see if failure needs to be handled
void failure_detector(){
    int start_cleanup = -1;
    while(1){
        // int finished_cleanup = 1;
        for(int i = 0; i < 10; i++){
            if(machines_avail[i] == -1){
                failure_fixing = 1;
                start_cleanup = i;
                //  auto start_failure = std::chrono::high_resolution_clock::now();
                recover_files(i);
                // auto current_time = std::chrono::high_resolution_clock::now();
                // int time_ = std::chrono::duration_cast<std::chrono::milliseconds>(current_time - start_failure).count();
                // std::cout << time_ << "\n";
                continue;
                

            }
            if(start_cleanup == i){
                failure_fixing = 0;
            }
        }
    }
}
