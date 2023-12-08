#include "sdfs.hpp"



/* setup_tcp_server_communication_server(std::string port_num)
* Server that all machines not the leader use to communicate
* Args: Port, port is based on machine number
*
*/
void setup_tcp_server_communication_server(std::string port_num) {

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
    char loc_file_name[200];
    std::string file_put("put");
    std::string file_delete("delete");
    std::string file_get("get");
    std::string server_buffer("buffer");
    while(1) {
        
        // server must always be running to serve next grep, returns when client breaks out of loop
        comm_socket_fd = accept(master_sock_tcp_fd, (struct sockaddr *)&client_addr, &addr_size);
        // if (new_fd == -1) {
        //     perror("accept");
        // }

        // Convert IP addresses of querier to correct form to use in send and recieve
        inet_ntop(client_addr.ss_family, get_in_addr((struct sockaddr *)&client_addr), s, sizeof s);


        // reset data buffer for next command  
        memset(command, 0, sizeof(command));
        memset(sys_file_name, 0, sizeof(sys_file_name));
        memset(loc_file_name, 0, sizeof(loc_file_name));


        // recv blocks until data arrives from client with command
        sent_recv_bytes = recv(comm_socket_fd, (char *) command, sizeof(command), 0);
    
        // printf("Server got %d bytes from client \n", sent_recv_bytes);
        if(sent_recv_bytes == 0) {
            // server choice to break if no bytes recieved
            close(comm_socket_fd);
            break;
        }

        sent_recv_bytes = send(comm_socket_fd, (int *)&processed, sizeof(processed), 0); // send acknowledgement
        std::string c_op(command);

        // std::cout << c_op << "\n";
        sent_recv_bytes = recv(comm_socket_fd, (char *) sys_file_name, sizeof(sys_file_name), 0);
        sent_recv_bytes = send(comm_socket_fd, (int *)&processed, sizeof(processed), 0); // send acknowledgement
        std::string file_name(sys_file_name);




        // do server operation when file is wanted to be placed on this machine
        if(c_op == file_get){
            std::string f_name("sysfiles/");
            f_name.append(file_name);
            std::ifstream file(f_name.c_str(), std::ios::in | std::ios::binary);
            sent_recv_bytes = recv(comm_socket_fd, (int *)&processed, sizeof(processed), 0); // syncronize the client and server
            if(file.good()){
                file.clear();
                file.seekg(0);

                // read through file
                std::string file_string_server((std::istreambuf_iterator<char>(file)),(std::istreambuf_iterator<char>()));
                file.close();



                const char* data_ptr  = file_string_server.data();
                std::size_t bytes_to_process = file_string_server.length();
                int for_process = (int) bytes_to_process;


                sent_recv_bytes = send(comm_socket_fd, (int *)&for_process, sizeof(for_process), 0);

                sent_recv_bytes = recv(comm_socket_fd, &processed, sizeof(processed), 0); // recieve acknoledgement



                // WHile loop to  send the TOTAL string to querier by the number of bytes
                while(bytes_to_process > 0) {

                    // send data, incrementing through pointer to string
                    sent_recv_bytes = send(comm_socket_fd, data_ptr, bytes_to_process, 0);
                    // subtract number of bytes left to process
                    bytes_to_process -= sent_recv_bytes;
                    // add to data pointer so starts where last send left off
                    data_ptr += sent_recv_bytes;
                }

                sent_recv_bytes = recv(comm_socket_fd, &processed, sizeof(processed), 0); // sync
            





            }
            else{
                int bad_file = -1;
                sent_recv_bytes = send(comm_socket_fd, (int *)&bad_file, sizeof(bad_file), 0);

                sent_recv_bytes = recv(comm_socket_fd, &processed, sizeof(processed), 0); // recieve acknoledgement


            }

            // sent_recv_bytes = send(comm_socket_fd, (int *)&processed, sizeof(processed), 0); // send acknowledgement
            
            close(comm_socket_fd);
            //printf("YAY\n");
        }



        if(c_op == file_put){
            std::string f_name("sysfiles/");
            f_name.append(sys_file_name);

            std::size_t file_size = 0;

            sent_recv_bytes =  recv(comm_socket_fd, (size_t *)&file_size, sizeof(file_size), 0);
            sent_recv_bytes = send(comm_socket_fd, (int *)&processed, sizeof(processed), 0); // send acknowledgement

            
            std::string file_string_server;
            while(file_size > 0) {
                memset(res_buf, 0, sizeof(res_buf)); // reset buffer

                sent_recv_bytes =  recv(comm_socket_fd, (char *)res_buf, sizeof(res_buf), 0);
                // check if bytes sent within bounds (no weird tcp errors)
                if(file_size >= sent_recv_bytes) { 
                    file_string_server.append(res_buf, sent_recv_bytes); // this was the solution to large files
                  
                }
               
                memset(res_buf, 0, sizeof(res_buf)); // reset buffer (just in case)

                file_size -= sent_recv_bytes;
         
                
            }
            std::ofstream ofs;
            ofs.open(f_name, std::ofstream::out | std::ofstream::trunc);
            ofs.close();
            std::ofstream fileOUT(f_name);
            fileOUT << file_string_server;
            fileOUT.close();
            int finished_file = 1;

            sent_recv_bytes =  send(comm_socket_fd, (int *)&processed, sizeof(processed), 0); // recieve acknowlegement

            sent_recv_bytes = recv(comm_socket_fd, &processed, sizeof(processed), 0); // sync
            
            // close client socket so client will end its connection to server and exit its while loop
            file_string_server.clear();
            close(comm_socket_fd);
            // printf("YAY\n");
        }


        else if(c_op == file_delete){
            std::string to_delete("rm sysfiles/");
            to_delete.append(file_name);
            system(to_delete.c_str());
            sent_recv_bytes = recv(comm_socket_fd, &processed, sizeof(processed), 0); // sync
            close(comm_socket_fd);
            // printf("YAY\n");
        }


        // server buffer needed for multiread to function properly

        else if(c_op == server_buffer){
            sent_recv_bytes = recv(comm_socket_fd, (char *) loc_file_name, sizeof(loc_file_name), 0);
            // sent_recv_bytes = send(comm_socket_fd, (int *)&processed, sizeof(processed), 0); // send acknowledgement
            std::string local_file_name(loc_file_name);
            setup_tcp_communication_client(machine_names[leader_id], std::to_string(ports[host_id]+ -1*10), file_name, local_file_name, file_get);
           
            sent_recv_bytes = send(comm_socket_fd, (int *)&processed, sizeof(processed), 0); // send acknowledgement
            sent_recv_bytes = recv(comm_socket_fd, &processed, sizeof(processed), 0); // sync
            close(comm_socket_fd);
            // printf("YAY\n");
        }

    }
}

