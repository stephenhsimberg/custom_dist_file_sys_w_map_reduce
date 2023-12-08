#include "maple_juice.hpp"

void setup_tcp_server_communication_server_maple(std::string port_num) {

    /*Local variables needed to set up server communication with clients*/
    char res_buf[10000];
    char exec_file_char[200];
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
    while(1) {
        
        // server must always be running to serve next grep, returns when client breaks out of loop
        comm_socket_fd = accept(master_sock_tcp_fd, (struct sockaddr *)&client_addr, &addr_size);
        // if (new_fd == -1) {
        //     perror("accept");
        // }

        // Convert IP addresses of querier to correct form to use in send and recieve
        inet_ntop(client_addr.ss_family, get_in_addr((struct sockaddr *)&client_addr), s, sizeof s);

        memset(exec_file_char, 0, sizeof(exec_file_char));


        // reset data buffer for next command


        // recv blocks until data arrives from client with command
        // sent_recv_bytes = recv(comm_socket_fd, (char *) command, sizeof(command), 0);
    
        // printf("Server got %d bytes from client \n", sent_recv_bytes);
        // if(sent_recv_bytes == 0) {
        //     // server choice to break if no bytes recieved
        //     close(comm_socket_fd);
        //     break;
        // }

        // sent_recv_bytes = send(comm_socket_fd, (int *)&processed, sizeof(processed), 0); // send acknowledgement
        std::size_t file_size = 0;
        sent_recv_bytes =  recv(comm_socket_fd, (size_t *)&file_size, sizeof(file_size), 0);
        sent_recv_bytes = send(comm_socket_fd, (int *)&processed, sizeof(processed), 0); // send acknowledgement

        std::string maple_input;

        while(file_size > 0) {
            memset(res_buf, 0, sizeof(res_buf)); // reset buffer

            sent_recv_bytes =  recv(comm_socket_fd, (char *)res_buf, sizeof(res_buf), 0);
            // check if bytes sent within bounds (no weird tcp errors)
            if(file_size >= sent_recv_bytes) { 
                maple_input.append(res_buf, sent_recv_bytes); // this was the solution to large files
                
            }
            
            memset(res_buf, 0, sizeof(res_buf)); // reset buffer (just in case)

            file_size -= sent_recv_bytes;
    
        
        }
        std::cout << "recieved\n";
        std::string f_name("sysfiles/maple_input");
        std::ofstream ofs;
        ofs.open(f_name, std::ofstream::out | std::ofstream::trunc);
        ofs.close();
        std::ofstream fileOUT(f_name);
        fileOUT << maple_input;
        fileOUT.close();

        sent_recv_bytes =  send(comm_socket_fd, (int *)&processed, sizeof(processed), 0); // recieve acknowlegement

        sent_recv_bytes = recv(comm_socket_fd, (char *) exec_file_char, sizeof(exec_file_char), 0);
        std::string exec("./");
        exec.append(exec_file_char);

        system(exec.c_str());

        std::string f2_name("sysfiles/maple_output");
        std::ifstream file(f2_name, std::ios::in | std::ios::binary);

        std::string maple_output((std::istreambuf_iterator<char>(file)),(std::istreambuf_iterator<char>()));

        const char* data_ptr  = maple_output.data();
        std::size_t bytes_to_process = maple_output.length();

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

        sent_recv_bytes =  send(comm_socket_fd, (int *)&processed, sizeof(processed), 0); // recieve acknowlegement

        sent_recv_bytes = recv(comm_socket_fd, &processed, sizeof(processed), 0); // recieve acknoledgement

        system("rm sysfiles/maple_output");
        system("rm sysfiles/maple_input");




        close(comm_socket_fd);
        //printf("YAY\n");


    }
}

