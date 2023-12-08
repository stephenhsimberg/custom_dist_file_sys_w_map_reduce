#include "sdfs.hpp"


/* setup_tcp_communication_client(std::string server_ip, std::string port_num, std::string arg_sys_file_name, std::string arg_loc_file_name, std::string op)
* Client function used to both communicate with leader and other machine servers
* Args: server_ip: DNS name of server to connect to
*       port_num: Port number to connect to server with, determined by machine
*       arg_sys_file_name: Used when communicating with machines other than the leader, holds system name to save local file as 
*       arg_loc_file_name: Used when communicating with machines other than the leader, holds local file name
*       op: "leader" when want to start connection with leader, operation to do if connected to other machines
*
*/
void setup_tcp_communication_client(std::string server_ip, std::string port_num, std::string arg_sys_file_name, std::string arg_loc_file_name, std::string op){


    char res_buf[10000]; // buf that reads grep output from server
    // char file_name[300]; // holds file name outout from server

    // Following Networking Guide: Beej’s Guide to Network Programming.
    // Same setup as server
    int socket_fd, numbytes;
    struct addrinfo hints, *servinfo, *p; 
    int rv;
    char s[INET6_ADDRSTRLEN];
    
    memset(&hints, 0, sizeof hints); 
    hints.ai_family = AF_UNSPEC; 
    hints.ai_socktype = SOCK_STREAM;

    // Takes current machine DNS name as argument, w/ port number
    if ((rv = getaddrinfo(server_ip.c_str(), port_num.c_str(), &hints, &servinfo)) != 0) {  
        return;
    }
        // loop through all the socket results, and binds to the first valid socket
    for(p = servinfo; p != NULL; p = p->ai_next) {
        if ((socket_fd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) {  
            continue;
        }
        // no need for socket opt rebind for client

        // connect is what binds the client to a server and allows to communicate with the server
        if (connect(socket_fd, p->ai_addr, p->ai_addrlen) == -1) { 
            close(socket_fd);
            continue; 
        }
        break; 
    }
    if (p == NULL) {
    //     m1.lock();
    //     num_threads--;
        printf("client: failed to connect\n"); 
    //     m1.unlock();
        return;
    }




        // fill in address
    inet_ntop(p->ai_family, get_in_addr((struct sockaddr *)p->ai_addr), s, sizeof s);
    // printf("client: connecting to %s\n", s);
    freeaddrinfo(servinfo); 


  
   
    
    int sent_recv_bytes;
    int processed = 0;


    std::string file_put("put");
    std::string file_get("get");
    std::string file_multiread("multiread");
    std::string file_delete("delete");
    std::string file_leader("leader");
    std::string file_store("store");
    std::string file_ls("ls");
    std::string buffer_client("buffer_client");
    std::string maple_cmd("maple");
    std::string juice_cmd("juice");
    std::string SELECT("SELECT");
    std::string JOIN("JOIN");
    // Communicating with leader
    if(op == file_leader) {

        std::string list_mem("list_mem");
        std::string list_self("list_self");
        std::string list_sus("enable suspicion");
        std::string list_no_sus("disable suspicion");
        
        // char command[50];   
        char sys_files[2048];
        char machs_to_save[5];
        // while loop to get commands
        while(1) {
            memset(machs_to_save, 0, sizeof(machs_to_save));
            memset(sys_files, 0, sizeof(sys_files));
            // memset(sys_file_name, 0, sizeof(sys_file_name));

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

            // print out files stored on cuurent machine
            else if(buf == file_store){
                sent_recv_bytes = send(socket_fd, file_store.c_str(), file_store.length(), 0);

                sent_recv_bytes =  recv(socket_fd, (int *)&processed, sizeof(processed), 0); // recieve acknowlegement

                // send sdfs name
                sent_recv_bytes = send(socket_fd, (int *)&host_id, sizeof(host_id), 0);

                sent_recv_bytes =  recv(socket_fd, (char *)sys_files, sizeof(sys_files), 0);
                std::string to_print(sys_files);
                std::cout << sys_files;
                sent_recv_bytes = send(socket_fd, &processed, sizeof(processed), 0); // sync
                printf("Finished Processing\n");
                break;
            }

            // Get other commands that have arguments
            int i = 0;
            for (i = 0; i < buf.length(); i++) 
                if (isspace(buf[i]))
                    break;
            if(i == (int)buf.length()){
                std::cout << "invalid command!" << "\n";
                continue;
            }
            std::string command = buf.substr(0, i);
            if(command != file_put && command != file_get && command != file_delete && command != file_ls && command != file_multiread  && command != maple_cmd && command != juice_cmd && command != SELECT && command != JOIN){
                 std::cout << "invalid command!" << "\n";
                continue;
            }
            buf = buf.substr(i+1);


            // Get arguments of commands
            std::string sdfsfilename;
            std::string localfilename;
            std::vector<int> multiread_machs;
            if(command == file_put) {
                for (i = 0; i < buf.length(); i++)
                    if (isspace(buf[i]))
                        break;

                localfilename = buf.substr(0, i);
                sdfsfilename = buf.substr(i+1);
            }

            else if(command == file_get) {
                for (i = 0; i < buf.length(); i++)
                    if (isspace(buf[i]))
                        break;

                sdfsfilename = buf.substr(0, i);
                localfilename = buf.substr(i+1);
            }

            else if(command == file_delete){
                sdfsfilename = buf;
            }

            else if(command == file_ls){
                sdfsfilename = buf;

            }

            else if(command == file_multiread){
                for (i = 0; i < buf.length(); i++)
                    if (isspace(buf[i]))
                        break;
                if(i == (int)buf.length()){
                    std::cout << "invalid command!" << "\n";
                    continue;
                }
                sdfsfilename = buf.substr(0, i);
                buf = buf.substr(i+1);
                int j;
                for (j = 0; j < buf.length(); j++)
                    if (isspace(buf[j]))
                        break;

                if(j == (int)buf.length()){
                    std::cout << "invalid command!" << "\n";
                    continue;
                }
                localfilename = buf.substr(0, j);
                buf = buf.substr(j+1);
                std::istringstream iss(buf);   
                int n;
                while(iss >> n) {   
                    multiread_machs.push_back(n - 1);
                }

                // for(int a = 0; a < (int)multiread_machs.size(); a++ ){
                //     // std::cout << multiread_machs[a] << " ";
                // }
                // // std::cout << "\n";

            }

            // NEW CODE FOR MP4
            std::string maple_exe;
            std::string inter_prefix;
            std::string input_dir;
            std::string num_maples;
            std::string arg;
            int num_tasks;
            if(command == maple_cmd){
                for (i = 0; i < buf.length(); i++)
                    if (isspace(buf[i]))
                        break;

                if(i == (int)buf.length()){
                    std::cout << "invalid command!" << "\n";
                    continue;
                }
                maple_exe = buf.substr(0, i);
                buf = buf.substr(i+1);
                for (i = 0; i < buf.length(); i++)
                    if (isspace(buf[i]))
                        break;

                if(i == (int)buf.length()){
                    std::cout << "invalid command!" << "\n";
                    continue;
                }
                num_maples = buf.substr(0, i);
                num_tasks = stoi(num_maples);
                buf = buf.substr(i+1);
                for (i = 0; i < buf.length(); i++)
                    if (isspace(buf[i]))
                        break;

                if(i == (int)buf.length()){
                    std::cout << "invalid command!" << "\n";
                    continue;
                }
                inter_prefix = buf.substr(0, i);
                buf= buf.substr(i+1);
                for (i = 0; i < buf.length(); i++)
                    if (isspace(buf[i]))
                        break;

                if(i == (int)buf.length()){
                    std::cout << "invalid command!" << "\n";
                    continue;
                }
                input_dir = buf.substr(0, i);
                arg = buf.substr(i+1);

                std::cout << command << " " << maple_exe << " " << num_tasks << " " << inter_prefix << " " << input_dir << " " << arg <<  "\n";
                std::string none("none");
                if(arg != none)
                    maple_exe.append(" " + arg);
            
            }

            std::string juice_exe;
            std::string num_juices;
            std::string j_inter_prefix;
            std::string dest_file_name;
            std::string delete_input;
            int to_delete;
            if(command == juice_cmd){
                for (i = 0; i < buf.length(); i++)
                    if (isspace(buf[i]))
                        break;

                if(i == (int)buf.length()){
                    std::cout << "invalid command!" << "\n";
                    continue;
                }
                juice_exe = buf.substr(0, i);
                buf = buf.substr(i+1);
                for (i = 0; i < buf.length(); i++)
                    if (isspace(buf[i]))
                        break;

                if(i == (int)buf.length()){
                    std::cout << "invalid command!" << "\n";
                    continue;
                }
                num_juices = buf.substr(0, i);
                num_tasks = stoi(num_juices);
                buf = buf.substr(i+1);
                for (i = 0; i < buf.length(); i++)
                    if (isspace(buf[i]))
                        break;

                if(i == (int)buf.length()){
                    std::cout << "invalid command!" << "\n";
                    continue;
                }
                j_inter_prefix = buf.substr(0, i);
                buf = buf.substr(i+1);
                for (i = 0; i < buf.length(); i++)
                    if (isspace(buf[i]))
                        break;

                if(i == (int)buf.length()){
                    std::cout << "invalid command!" << "\n";
                    continue;
                }
                dest_file_name = buf.substr(0, i);
                delete_input = buf.substr(i+1);
                std::cout << delete_input;
                to_delete = stoi(delete_input);
                

                std::cout << command << " " << juice_exe << " " << num_tasks << " " << j_inter_prefix << " " << dest_file_name << " " << to_delete << "\n";

            }

            std::string dataset;
            std::string regex;
            if(command == SELECT){
                for (i = 0; i < buf.length(); i++)
                    if (isspace(buf[i]))
                        break;

                if(i == (int)buf.length()){
                    std::cout << "invalid command!" << "\n";
                    continue;
                }
                dataset = buf.substr(0, i);
                regex = buf.substr(i+1);

            }


            std::string dataset1;
            std::string field1;
            std::string dataset2;
            std::string field2;
            if(command == JOIN){
                for (i = 0; i < buf.length(); i++)
                    if (isspace(buf[i]))
                        break;

                if(i == (int)buf.length()){
                    std::cout << "invalid command!" << "\n";
                    continue;
                }
                dataset1 = buf.substr(0, i);
                buf = buf.substr(i+1);
                for (i = 0; i < buf.length(); i++)
                    if (isspace(buf[i]))
                        break;

                if(i == (int)buf.length()){
                    std::cout << "invalid command!" << "\n";
                    continue;
                }
                field1 = buf.substr(0, i);
                buf = buf.substr(i+1);
                for (i = 0; i < buf.length(); i++)
                    if (isspace(buf[i]))
                        break;

                if(i == (int)buf.length()){
                    std::cout << "invalid command!" << "\n";
                    continue;
                }
                // jsut to skip = 
                buf = buf.substr(i+1);
                for (i = 0; i < buf.length(); i++)
                    if (isspace(buf[i]))
                        break;

                if(i == (int)buf.length()){
                    std::cout << "invalid command!" << "\n";
                    continue;
                }
                dataset2 = buf.substr(0, i);
                field2 =  buf.substr(i+1);
                

            }



            
            int start_file = 1;

            // send command
            sent_recv_bytes = send(socket_fd, command.c_str(), command.length(), 0);

            sent_recv_bytes =  recv(socket_fd, (int *)&processed, sizeof(processed), 0); // recieve acknowlegement

            if(command != maple_cmd && command != juice_cmd && command != SELECT && command != JOIN){
                // send sdfs name
                sent_recv_bytes = send(socket_fd, sdfsfilename.c_str(), sdfsfilename.length(), 0);

                sent_recv_bytes =  recv(socket_fd, (int *)&processed, sizeof(processed), 0); // recieve acknowlegement
            }
            else{

                if(command == SELECT){
                    auto start_get = std::chrono::high_resolution_clock::now();


                    maple_exe = "select_maple";
                    maple_exe.append(" " + regex);
                    num_tasks = 9;
                    inter_prefix = "select";
                    input_dir = dataset;
                    sent_recv_bytes = send(socket_fd, maple_exe.c_str(), maple_exe.length(), 0);
                    sent_recv_bytes =  recv(socket_fd, (int *)&processed, sizeof(processed), 0); // recieve acknowlegement

                    sent_recv_bytes = send(socket_fd,(int *)&num_tasks, sizeof(num_tasks), 0);
                    sent_recv_bytes =  recv(socket_fd, (int *)&processed, sizeof(processed), 0); // recieve acknowlegement

                    sent_recv_bytes = send(socket_fd, inter_prefix.c_str(), inter_prefix.length(), 0);
                    sent_recv_bytes =  recv(socket_fd, (int *)&processed, sizeof(processed), 0); // recieve acknowlegement

                    sent_recv_bytes = send(socket_fd, input_dir.c_str(), input_dir.length(), 0);
                    



                    int maple_success = 0;
                    sent_recv_bytes =  recv(socket_fd, (int *)&processed, sizeof(processed), 0); // recieve acknowlegement
                    printf("Finished Maple Processing\n");

                    juice_exe = "select_juice";
                    j_inter_prefix = "select";
                    dest_file_name = "select_output";
                    to_delete = 0;






                    sent_recv_bytes = send(socket_fd, juice_exe.c_str(), juice_exe.length(), 0);
                    sent_recv_bytes =  recv(socket_fd, (int *)&processed, sizeof(processed), 0); // recieve acknowlegement

                    sent_recv_bytes = send(socket_fd,(int *)&num_tasks, sizeof(num_tasks), 0);
                    sent_recv_bytes =  recv(socket_fd, (int *)&processed, sizeof(processed), 0); // recieve acknowlegement

                    sent_recv_bytes = send(socket_fd, j_inter_prefix.c_str(), j_inter_prefix.length(), 0);
                    sent_recv_bytes =  recv(socket_fd, (int *)&processed, sizeof(processed), 0); // recieve acknowlegement

                    sent_recv_bytes = send(socket_fd, dest_file_name.c_str(), dest_file_name.length(), 0);
                    sent_recv_bytes =  recv(socket_fd, (int *)&processed, sizeof(processed), 0); // recieve acknowlegement

                    sent_recv_bytes = send(socket_fd,(int *)&to_delete, sizeof(to_delete), 0);

                    std::size_t file_size = 0;
                    sent_recv_bytes = recv(socket_fd, (size_t *)&file_size, sizeof(file_size), 0); // sync
                    sent_recv_bytes = send(socket_fd, (int *)&processed, sizeof(processed), 0); // sync
                    std::string juice_output;

                    while(file_size > 0) {
                        memset(res_buf, 0, sizeof(res_buf)); // reset buffer

                        sent_recv_bytes =  recv(socket_fd, (char *)res_buf, sizeof(res_buf), 0);
                        // check if bytes sent within bounds (no weird tcp errors)
                        if(file_size >= sent_recv_bytes) { 
                            juice_output.append(res_buf, sent_recv_bytes); // this was the solution to large files
                        }
                        
                        memset(res_buf, 0, sizeof(res_buf)); // reset buffer (just in case)

                        file_size -= sent_recv_bytes;

                    }

                    sent_recv_bytes = send(socket_fd, (int *)&processed, sizeof(processed), 0); // sync

                    std::ofstream ofs;
                    ofs.open(dest_file_name, std::ofstream::out | std::ofstream::trunc);
                    ofs.close();
                    std::ofstream fileOUT(dest_file_name);
                    fileOUT << juice_output;
                    fileOUT.close();

                    int juice_success = 0;
                    sent_recv_bytes =  recv(socket_fd, (int *)&processed, sizeof(processed), 0); // recieve acknowlegement
                    printf("Finished Processing\n");
                    auto stop_get = std::chrono::high_resolution_clock::now();
                    int time_ = std::chrono::duration_cast<std::chrono::milliseconds>(stop_get - start_get).count();
                    std::cout << time_ << "\n";
                    sent_recv_bytes = send(socket_fd, (int *)&processed, sizeof(processed), 0); // sync
                    break;



                }

                if(command == JOIN){
                    auto start_get = std::chrono::high_resolution_clock::now();


                    maple_exe = "join_maple";
                    maple_exe.append(" " + field1 + " " + field2);
                    num_tasks = 9;
                    inter_prefix = "join";


                    sent_recv_bytes = send(socket_fd, maple_exe.c_str(), maple_exe.length(), 0);
                    sent_recv_bytes =  recv(socket_fd, (int *)&processed, sizeof(processed), 0); // recieve acknowlegement

                    sent_recv_bytes = send(socket_fd,(int *)&num_tasks, sizeof(num_tasks), 0);
                    sent_recv_bytes =  recv(socket_fd, (int *)&processed, sizeof(processed), 0); // recieve acknowlegement

                    sent_recv_bytes = send(socket_fd, inter_prefix.c_str(), inter_prefix.length(), 0);
                    sent_recv_bytes =  recv(socket_fd, (int *)&processed, sizeof(processed), 0); // recieve acknowlegement

                    sent_recv_bytes = send(socket_fd, dataset1.c_str(), dataset1.length(), 0);
                    sent_recv_bytes =  recv(socket_fd, (int *)&processed, sizeof(processed), 0); // recieve acknowlegement

                    sent_recv_bytes = send(socket_fd, dataset2.c_str(), dataset2.length(), 0);
                    



                    int maple_success = 0;
                    sent_recv_bytes =  recv(socket_fd, (int *)&processed, sizeof(processed), 0); // recieve acknowlegement
                    printf("Finished Maple Processing\n");

                    juice_exe = "join_juice";
                    j_inter_prefix = "join";
                    dest_file_name = "join_output";
                    to_delete = 0;






                    sent_recv_bytes = send(socket_fd, juice_exe.c_str(), juice_exe.length(), 0);
                    sent_recv_bytes =  recv(socket_fd, (int *)&processed, sizeof(processed), 0); // recieve acknowlegement

                    sent_recv_bytes = send(socket_fd,(int *)&num_tasks, sizeof(num_tasks), 0);
                    sent_recv_bytes =  recv(socket_fd, (int *)&processed, sizeof(processed), 0); // recieve acknowlegement

                    sent_recv_bytes = send(socket_fd, j_inter_prefix.c_str(), j_inter_prefix.length(), 0);
                    sent_recv_bytes =  recv(socket_fd, (int *)&processed, sizeof(processed), 0); // recieve acknowlegement

                    sent_recv_bytes = send(socket_fd, dest_file_name.c_str(), dest_file_name.length(), 0);
                    sent_recv_bytes =  recv(socket_fd, (int *)&processed, sizeof(processed), 0); // recieve acknowlegement

                    sent_recv_bytes = send(socket_fd,(int *)&to_delete, sizeof(to_delete), 0);

                    std::size_t file_size = 0;
                    sent_recv_bytes = recv(socket_fd, (size_t *)&file_size, sizeof(file_size), 0); // sync
                    sent_recv_bytes = send(socket_fd, (int *)&processed, sizeof(processed), 0); // sync
                    std::string juice_output;

                    while(file_size > 0) {
                        memset(res_buf, 0, sizeof(res_buf)); // reset buffer

                        sent_recv_bytes =  recv(socket_fd, (char *)res_buf, sizeof(res_buf), 0);
                        // check if bytes sent within bounds (no weird tcp errors)
                        if(file_size >= sent_recv_bytes) { 
                            juice_output.append(res_buf, sent_recv_bytes); // this was the solution to large files
                        }
                        
                        memset(res_buf, 0, sizeof(res_buf)); // reset buffer (just in case)

                        file_size -= sent_recv_bytes;

                    }

                    sent_recv_bytes = send(socket_fd, (int *)&processed, sizeof(processed), 0); // sync

                    std::ofstream ofs;
                    ofs.open(dest_file_name, std::ofstream::out | std::ofstream::trunc);
                    ofs.close();
                    std::ofstream fileOUT(dest_file_name);
                    fileOUT << juice_output;
                    fileOUT.close();

                    int juice_success = 0;
                    sent_recv_bytes =  recv(socket_fd, (int *)&processed, sizeof(processed), 0); // recieve acknowlegement
                    printf("Finished Processing\n");
                    auto stop_get = std::chrono::high_resolution_clock::now();
                    int time_ = std::chrono::duration_cast<std::chrono::milliseconds>(stop_get - start_get).count();
                    std::cout << time_ << "\n";
                    sent_recv_bytes = send(socket_fd, (int *)&processed, sizeof(processed), 0); // sync
                    break;



                }




                if(command == maple_cmd){
                    sent_recv_bytes = send(socket_fd, maple_exe.c_str(), maple_exe.length(), 0);
                    sent_recv_bytes =  recv(socket_fd, (int *)&processed, sizeof(processed), 0); // recieve acknowlegement

                    sent_recv_bytes = send(socket_fd,(int *)&num_tasks, sizeof(num_tasks), 0);
                    sent_recv_bytes =  recv(socket_fd, (int *)&processed, sizeof(processed), 0); // recieve acknowlegement

                    sent_recv_bytes = send(socket_fd, inter_prefix.c_str(), inter_prefix.length(), 0);
                    sent_recv_bytes =  recv(socket_fd, (int *)&processed, sizeof(processed), 0); // recieve acknowlegement

                    sent_recv_bytes = send(socket_fd, input_dir.c_str(), input_dir.length(), 0);
                    sent_recv_bytes =  recv(socket_fd, (int *)&processed, sizeof(processed), 0); // recieve acknowlegement


                    sent_recv_bytes = send(socket_fd, input_dir.c_str(), input_dir.length(), 0);

                    



                    int maple_success = 0;
                    sent_recv_bytes =  recv(socket_fd, (int *)&processed, sizeof(processed), 0); // recieve acknowlegement
                    printf("Finished Processing\n");
                    break;
                }

                if(command == juice_cmd){
                    sent_recv_bytes = send(socket_fd, juice_exe.c_str(), juice_exe.length(), 0);
                    sent_recv_bytes =  recv(socket_fd, (int *)&processed, sizeof(processed), 0); // recieve acknowlegement

                    sent_recv_bytes = send(socket_fd,(int *)&num_tasks, sizeof(num_tasks), 0);
                    sent_recv_bytes =  recv(socket_fd, (int *)&processed, sizeof(processed), 0); // recieve acknowlegement

                    sent_recv_bytes = send(socket_fd, j_inter_prefix.c_str(), j_inter_prefix.length(), 0);
                    sent_recv_bytes =  recv(socket_fd, (int *)&processed, sizeof(processed), 0); // recieve acknowlegement

                    sent_recv_bytes = send(socket_fd, dest_file_name.c_str(), dest_file_name.length(), 0);
                    sent_recv_bytes =  recv(socket_fd, (int *)&processed, sizeof(processed), 0); // recieve acknowlegement

                    sent_recv_bytes = send(socket_fd,(int *)&to_delete, sizeof(to_delete), 0);

                    std::size_t file_size = 0;
                    sent_recv_bytes = recv(socket_fd, (size_t *)&file_size, sizeof(file_size), 0); // sync
                    sent_recv_bytes = send(socket_fd, (int *)&processed, sizeof(processed), 0); // sync
                    std::string juice_output;

                    while(file_size > 0) {
                        memset(res_buf, 0, sizeof(res_buf)); // reset buffer

                        sent_recv_bytes =  recv(socket_fd, (char *)res_buf, sizeof(res_buf), 0);
                        // check if bytes sent within bounds (no weird tcp errors)
                        if(file_size >= sent_recv_bytes) { 
                            juice_output.append(res_buf, sent_recv_bytes); // this was the solution to large files
                        }
                        
                        memset(res_buf, 0, sizeof(res_buf)); // reset buffer (just in case)

                        file_size -= sent_recv_bytes;

                    }

                    sent_recv_bytes = send(socket_fd, (int *)&processed, sizeof(processed), 0); // sync

                    std::ofstream ofs;
                    ofs.open(dest_file_name, std::ofstream::out | std::ofstream::trunc);
                    ofs.close();
                    std::ofstream fileOUT(dest_file_name);
                    fileOUT << juice_output;
                    fileOUT.close();

                    int juice_success = 0;
                    sent_recv_bytes =  recv(socket_fd, (int *)&processed, sizeof(processed), 0); // recieve acknowlegement
                    printf("Finished Processing\n");
                    sent_recv_bytes = send(socket_fd, (int *)&processed, sizeof(processed), 0); // sync
                    break;
                }


            }



            // print out machines file is being stored on
            if(command == file_ls){
                sent_recv_bytes =  recv(socket_fd, (char *)sys_files, sizeof(sys_files), 0);
                std::string to_print(sys_files);
                std::cout << sys_files << "\n";
                sent_recv_bytes = send(socket_fd, &processed, sizeof(processed), 0); // sync
                break;
                
            }

            // if reading:
            else if(command == file_get){
                // auto start_get = std::chrono::high_resolution_clock::now();
                // ready to recieve file
                sent_recv_bytes = send(socket_fd, (int *)&start_file, sizeof(start_file), 0);
                 //     // recieve size of grep output being sent
                int for_process = 0;
                sent_recv_bytes =  recv(socket_fd, (int *)&for_process, sizeof(for_process), 0);

                if(for_process == -1){
                    std::cout << "file not found in system" << "\n";
                    sent_recv_bytes = send(socket_fd, (int *)&start_file, sizeof(start_file), 0); // send acknowlegement

                }
                else{
                    std::size_t file_size = (std::size_t) for_process;
                    sent_recv_bytes = send(socket_fd, (int *)&start_file, sizeof(start_file), 0); // send acknowlegement



                    // std::ofstream fileOUT(localfilename, std::ios::app);
                    std::string file_string_get;
                    // Use while loop to recieve the entire grep output through recvies.
                    while(file_size > 0) {
                        memset(res_buf, 0, sizeof(res_buf)); // reset buffer

                        sent_recv_bytes =  recv(socket_fd, (char *)res_buf, sizeof(res_buf), 0);
                        // check if bytes sent within bounds (no weird tcp errors)
                        if(file_size >= sent_recv_bytes) { 
                            file_string_get.append(res_buf, sent_recv_bytes); // this was the solution to large files
                           
                        }
                        
                        memset(res_buf, 0, sizeof(res_buf)); // reset buffer (just in case)

                        file_size -= sent_recv_bytes;
                     
                            
                    }
                    std::ofstream ofs;
                    ofs.open(localfilename, std::ofstream::out | std::ofstream::trunc);
                    ofs.close();
                    std::ofstream fileOUT(localfilename);
                    fileOUT << file_string_get;
                    fileOUT.close();
                    int finished_file = 1;
                    sent_recv_bytes = send(socket_fd, (int *)&finished_file, sizeof(finished_file), 0);
                    // auto current_time = std::chrono::high_resolution_clock::now();
                    // int time_ = std::chrono::duration_cast<std::chrono::milliseconds>(current_time - start_get).count();
                    // std::cout << time_ << "\n";
                }


                sent_recv_bytes =  recv(socket_fd, (int *)&processed, sizeof(processed), 0); // recieve acknowlegement
                std::ofstream fileOUT(mem_file, std::ios::app);
                std::string new_mem("Got File: ");
                new_mem.append(sdfsfilename);
                new_mem.append(" to ");
                new_mem.append(localfilename);
                fileOUT << new_mem << "\n";
                fileOUT.close();
                printf("Finished Processing\n");
                break;

            }

            // Start multiread
            else if(command == file_multiread){
                // auto start_put = std::chrono::high_resolution_clock::now();
                std::vector<std::thread> threads;
                int host_flag = 0;
                int count = 0;
                for(int cur_char = 0; cur_char < (int) multiread_machs.size(); cur_char++){
                    if(host_id == multiread_machs[cur_char]){
                        host_flag = 1;
                        continue;
                    }
                    threads.emplace_back(setup_tcp_communication_client, machine_names[multiread_machs[cur_char]], std::to_string(ports[host_id]+ multiread_machs[cur_char]*10), sdfsfilename, localfilename, buffer_client);
                    count++;
                    if(count == 2){
                        threads[0].join();
                        threads[1].join();
                        threads.clear();
                        count = 0;
                    }
                }

                if(host_flag == 1){
                    threads.emplace_back(setup_tcp_communication_client, machine_names[leader_id], std::to_string(ports[host_id]+ -1*10), sdfsfilename, localfilename, file_get);
                }

                for (auto& th : threads) 
                    th.join();
                    
                // auto current_time = std::chrono::high_resolution_clock::now();
                // int time_ = std::chrono::duration_cast<std::chrono::milliseconds>(current_time - start_put).count();
                // std::cout << time_ << "\n";
                std::ofstream fileOUT(mem_file, std::ios::app);
                std::string new_mem("Multiread: ");
                new_mem.append(sdfsfilename);
                new_mem.append(" to ");
                new_mem.append(localfilename);
                fileOUT << new_mem << "\n";
                fileOUT.close();
                printf("Finished Processing\n");
                break;

            }


            // if writing:
            else if(command == file_put){
                //synchronize server and client
                // auto start_put = std::chrono::high_resolution_clock::now();
                sent_recv_bytes = send(socket_fd, (int *)&start_file, sizeof(start_file), 0);
                while(1){
                    memset(machs_to_save, 0, sizeof(machs_to_save));
                    sent_recv_bytes =  recv(socket_fd, (char *)machs_to_save, sizeof(machs_to_save), 0);
                    std::string needed_machs(machs_to_save);
                    // easiest way to get integer from char
                    int machine1 = int(needed_machs[0] - '0');
                    int machine2 = int(needed_machs[1] - '0');
                    int machine3 = int(needed_machs[2] - '0');

                    // check if machine1, machine2, machine3 
                    std::vector<std::thread> threads;
                    std::ifstream file(localfilename.c_str(), std::ios::in | std::ios::binary);
                    file.clear();
                    file.seekg(0);
                    file_string.assign((std::istreambuf_iterator<char>(file)),(std::istreambuf_iterator<char>()));
                    file.close();

                    // if machine1 is same machine as host
                    if(machine1 == host_id){
                        threads.emplace_back(setup_tcp_communication_client, machine_names[machine2], std::to_string(ports[host_id]+ machine2*10), sdfsfilename, localfilename, file_put);
                        threads.emplace_back(setup_tcp_communication_client, machine_names[machine3], std::to_string(ports[host_id]+ machine3*10), sdfsfilename, localfilename, file_put);
                        threads.emplace_back(write_to_self, sdfsfilename);
                        threads.emplace_back(leader_write, socket_fd);
                        

                    }

                    else if(machine2 == host_id){
                        threads.emplace_back(setup_tcp_communication_client, machine_names[machine1], std::to_string(ports[host_id]+ machine1*10), sdfsfilename, localfilename, file_put);
                        threads.emplace_back(setup_tcp_communication_client, machine_names[machine3], std::to_string(ports[host_id]+ machine3*10), sdfsfilename, localfilename, file_put);
                        threads.emplace_back(write_to_self, sdfsfilename);
                        threads.emplace_back(leader_write, socket_fd);
                      

                    }

                    else if(machine3 == host_id){
                        threads.emplace_back(setup_tcp_communication_client, machine_names[machine1], std::to_string(ports[host_id]+ machine1*10), sdfsfilename, localfilename, file_put);
                        threads.emplace_back(setup_tcp_communication_client, machine_names[machine2], std::to_string(ports[host_id]+ machine2*10), sdfsfilename, localfilename, file_put);
                        threads.emplace_back(write_to_self, sdfsfilename);
                        threads.emplace_back(leader_write, socket_fd);
                       

                    }

                    else {
                        threads.emplace_back(setup_tcp_communication_client, machine_names[machine1], std::to_string(ports[host_id]+ machine1*10), sdfsfilename, localfilename, file_put);
                        threads.emplace_back(setup_tcp_communication_client, machine_names[machine2], std::to_string(ports[host_id]+ machine2*10), sdfsfilename, localfilename, file_put);
                        threads.emplace_back(setup_tcp_communication_client, machine_names[machine3], std::to_string(ports[host_id]+ machine3*10), sdfsfilename, localfilename, file_put);
                        threads.emplace_back(leader_write, socket_fd);
                    }

                


                   
                    threads[0].join();
                    threads[1].join();
                    threads[2].join();
                    threads[3].join();
                    file_fixed = 0;
                    // }
                    // auto current_time = std::chrono::high_resolution_clock::now();
                    // int time_ = std::chrono::duration_cast<std::chrono::milliseconds>(current_time - start_put).count();
                    // std::cout << time_ << "\n";

                    sent_recv_bytes = send(socket_fd, (int *)&failed_write, sizeof(failed_write), 0);
                
                
                    if(failed_write){
                        failed_write = 0;
                        continue;
                    }
                    else {
                        break;

                    }
                }
                
                sent_recv_bytes = recv(socket_fd, &processed, sizeof(processed), 0); // sync
                std::ofstream fileOUT(mem_file, std::ios::app);
                std::string new_mem("Wrote: ");
                new_mem.append(localfilename);
                new_mem.append(" to ");
                new_mem.append(sdfsfilename);
                fileOUT << new_mem << "\n";
                fileOUT.close();
                printf("Finished Processing\n");
                break;

            }

            // if delete file, same operation as write
            else if(command == file_delete) {
                sent_recv_bytes = send(socket_fd, (int *)&start_file, sizeof(start_file), 0);

                int for_process = 0;
                sent_recv_bytes =  recv(socket_fd, (int *)&for_process, sizeof(for_process), 0);
                if(for_process == -1){
                    std::cout << "file not found in system" << "\n";
                    sent_recv_bytes = send(socket_fd, (int *)&start_file, sizeof(start_file), 0); // send acknowlegement

                }
                else {
                    sent_recv_bytes = send(socket_fd, (int *)&start_file, sizeof(start_file), 0);

                    sent_recv_bytes =  recv(socket_fd, (char *)machs_to_save, sizeof(machs_to_save), 0);
                    std::string needed_machs(machs_to_save);
                    // easiest way to get integer from char
                    int machine1 = int(needed_machs[0] - '0');
                    int machine2 = int(needed_machs[1] - '0');
                    int machine3 = int(needed_machs[2] - '0');

                    std::vector<std::thread> threads;

                    if(machine1 == host_id){
                        threads.emplace_back(setup_tcp_communication_client, machine_names[machine2], std::to_string(ports[host_id]+ machine2*10), sdfsfilename, "", file_delete);
                        threads.emplace_back(setup_tcp_communication_client, machine_names[machine3], std::to_string(ports[host_id]+ machine3*10), sdfsfilename, "", file_delete);
                        std::string to_delete("rm sysfiles/");
                        to_delete.append(sdfsfilename);
                        system(to_delete.c_str());

                    }
                    else if(machine2 == host_id){
                        threads.emplace_back(setup_tcp_communication_client, machine_names[machine1], std::to_string(ports[host_id]+ machine1*10), sdfsfilename, "", file_delete);
                        threads.emplace_back(setup_tcp_communication_client, machine_names[machine3], std::to_string(ports[host_id]+ machine3*10), sdfsfilename, "", file_delete);
                        std::string to_delete("rm sysfiles/");
                        to_delete.append(sdfsfilename);
                        system(to_delete.c_str());
                    }
                    else if(machine3 == host_id){
                        threads.emplace_back(setup_tcp_communication_client, machine_names[machine1], std::to_string(ports[host_id]+ machine1*10), sdfsfilename, "", file_delete);
                        threads.emplace_back(setup_tcp_communication_client, machine_names[machine2], std::to_string(ports[host_id]+ machine2*10), sdfsfilename, "", file_delete);
                        std::string to_delete("rm sysfiles/");
                        to_delete.append(sdfsfilename);
                        system(to_delete.c_str());
                    }
                    else{
                        threads.emplace_back(setup_tcp_communication_client, machine_names[machine1], std::to_string(ports[host_id]+ machine1*10), sdfsfilename, "", file_delete);
                        threads.emplace_back(setup_tcp_communication_client, machine_names[machine2], std::to_string(ports[host_id]+ machine2*10), sdfsfilename, "", file_delete);
                        threads.emplace_back(setup_tcp_communication_client, machine_names[machine3], std::to_string(ports[host_id]+ machine3*10), sdfsfilename, "", file_delete);
                    }

                    if(machine1 == host_id || machine2 == host_id || machine3 == host_id){
                        threads[0].join();
                        threads[1].join();
                    }
                    else{
                        threads[0].join();
                        threads[1].join();
                        threads[2].join();
                    }
                    sent_recv_bytes = send(socket_fd, &processed, sizeof(processed), 0); // sync
                    sent_recv_bytes = recv(socket_fd, &processed, sizeof(processed), 0); // sync




                }

                // sent_recv_bytes =  recv(socket_fd, (int *)&processed, sizeof(processed), 0); // recieve acknowlegement
                std::ofstream fileOUT(mem_file, std::ios::app);
                std::string new_mem("Deleted: ");
                new_mem.append(sdfsfilename);
                fileOUT << new_mem << "\n";
                fileOUT.close();
                printf("Finished Processing\n");
                break;
            }



            

        }


    }
    
    // if putting onto server:
    else if(op == file_put){
        int start_file = 1;

        // send command
        sent_recv_bytes = send(socket_fd, file_put.c_str(), file_put.length(), 0);

        sent_recv_bytes =  recv(socket_fd, (int *)&processed, sizeof(processed), 0); // recieve acknowlegement

        // send sdfs name
        sent_recv_bytes = send(socket_fd, arg_sys_file_name.c_str(), arg_sys_file_name.length(), 0);

        sent_recv_bytes =  recv(socket_fd, (int *)&processed, sizeof(processed), 0); // recieve acknowlegement
        int com_id = std::stoi(port_num) % 10;

        // if leader is replicating file after failure, read that file
        if(com_id == leader_id + 1){
            std::ifstream file(arg_loc_file_name.c_str(), std::ios::in | std::ios::binary);
            file.clear();
            file.seekg(0);
            std::string file_string_leader((std::istreambuf_iterator<char>(file)),(std::istreambuf_iterator<char>()));
            file.close();
            const char* data_ptr  = file_string_leader.data();
            std::size_t bytes_to_process = file_string_leader.length();


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
        }
        // else do current put operation
        else {

            const char* data_ptr  = file_string.data();
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
        }

        sent_recv_bytes = recv(socket_fd, &processed, sizeof(processed), 0); // sync
        if(sent_recv_bytes == 0){
            failed_write = 1;
            // printf(":(\n");
        }
        sent_recv_bytes =  send(socket_fd, (int *)&processed, sizeof(processed), 0); // recieve acknowlegement
        // will return 0 once server closes, and will close socket
        
        file_fixed = 1;
        // printf("YAY\n");




    }


    else if(op == file_delete){
         // send command
        sent_recv_bytes = send(socket_fd, file_delete.c_str(), file_delete.length(), 0);

        sent_recv_bytes =  recv(socket_fd, (int *)&processed, sizeof(processed), 0); // recieve acknowlegement

        // send sdfs name
        sent_recv_bytes = send(socket_fd, arg_sys_file_name.c_str(), arg_sys_file_name.length(), 0);

        sent_recv_bytes =  recv(socket_fd, (int *)&processed, sizeof(processed), 0); // recieve acknowlegement

        sent_recv_bytes =  send(socket_fd, (int *)&processed, sizeof(processed), 0); // recieve acknowlegement
        // printf("YAY\n");

    }


    else if(op == file_get){
         int start_file = 1;

        // send command
        sent_recv_bytes = send(socket_fd, file_get.c_str(), file_get.length(), 0);

        sent_recv_bytes =  recv(socket_fd, (int *)&processed, sizeof(processed), 0); // recieve acknowlegement

        // send sdfs name
        sent_recv_bytes = send(socket_fd, arg_sys_file_name.c_str(), arg_sys_file_name.length(), 0);

        sent_recv_bytes =  recv(socket_fd, (int *)&processed, sizeof(processed), 0); // recieve acknowlegement
                // ready to recieve file
        sent_recv_bytes = send(socket_fd, (int *)&start_file, sizeof(start_file), 0);
            //     // recieve size of grep output being sent
        int for_process = 0;
        sent_recv_bytes =  recv(socket_fd, (int *)&for_process, sizeof(for_process), 0);

        if(for_process == -1){
            std::cout << "file not found in system" << "\n";
            sent_recv_bytes = send(socket_fd, (int *)&start_file, sizeof(start_file), 0); // send acknowlegement

        }
        else{
            std::size_t file_size = (std::size_t) for_process;
            sent_recv_bytes = send(socket_fd, (int *)&start_file, sizeof(start_file), 0); // send acknowlegement



            // std::ofstream fileOUT(arg_loc_file_name, std::ios::app);
            std::string file_string_get;
            // Use while loop to recieve the entire grep output through recvies.
            while(file_size > 0) {
                memset(res_buf, 0, sizeof(res_buf)); // reset buffer

                sent_recv_bytes =  recv(socket_fd, (char *)res_buf, sizeof(res_buf), 0);
                // check if bytes sent within bounds (no weird tcp errors)
                if(file_size >= sent_recv_bytes) { 
                    file_string_get.append(res_buf, sent_recv_bytes); // this was the solution to large files
                    
                }
                
                memset(res_buf, 0, sizeof(res_buf)); // reset buffer (just in case)

                file_size -= sent_recv_bytes;
                
                    
            }
            std::ofstream ofs;
            ofs.open(arg_loc_file_name, std::ofstream::out | std::ofstream::trunc);
            ofs.close();
            std::ofstream fileOUT(arg_loc_file_name);
            fileOUT << file_string_get;
            fileOUT.close();
            int finished_file = 1;
            sent_recv_bytes = send(socket_fd, (int *)&finished_file, sizeof(finished_file), 0);
            // printf("YAY\n");
        }
    }

    //used to process multiread
    else if(op == buffer_client){
        std::string server_buffer("buffer");
         // send command
        sent_recv_bytes = send(socket_fd,  server_buffer.c_str(),  server_buffer.length(), 0);

        sent_recv_bytes =  recv(socket_fd, (int *)&processed, sizeof(processed), 0); // recieve acknowlegement

        // send sdfs name
        sent_recv_bytes = send(socket_fd, arg_sys_file_name.c_str(), arg_sys_file_name.length(), 0);

        sent_recv_bytes =  recv(socket_fd, (int *)&processed, sizeof(processed), 0); // recieve acknowlegement
        // send local name
        sent_recv_bytes = send(socket_fd, arg_loc_file_name.c_str(), arg_loc_file_name.length(), 0);

        // server is done
        sent_recv_bytes =  recv(socket_fd, (int *)&processed, sizeof(processed), 0); // recieve acknowlegement

        sent_recv_bytes =  send(socket_fd, (int *)&processed, sizeof(processed), 0); // recieve acknowlegement

        // printf("MultiYAY\n");


    }
   





    sent_recv_bytes = recv(socket_fd, &processed, sizeof(processed), 0); // sync
    close(socket_fd);




}

