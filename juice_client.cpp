#include "maple_juice.hpp"

void setup_tcp_communication_client_juice(std::string server_ip, std::string port_num, std::string exec_file, std::vector<std::string> task_file, std::string file_out){
    // Following Networking Guide: Beej’s Guide to Network Programming.
    // Same setup as server
    char res_buf[10000];
    int socket_fd, numbytes;
    struct addrinfo hints, *servinfo, *p; 
    int rv;
    char s[INET6_ADDRSTRLEN];
    int sent_recv_bytes;
    int processed = 0;
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
    std::size_t num_keys = task_file.size();

    sent_recv_bytes = send(socket_fd, (size_t *)&num_keys, sizeof(num_keys), 0);
    sent_recv_bytes = recv(socket_fd, (int *)&processed, sizeof(processed), 0); // recieve acknoledgement
    std::cout << num_keys << "\n";
    for(int i = 0; i < (int) task_file.size(); i++){

        const char* data_ptr  = task_file[i].data();
        std::size_t bytes_to_process = task_file[i].length();

        sent_recv_bytes = send(socket_fd, (size_t *)&bytes_to_process, sizeof(bytes_to_process), 0);

        sent_recv_bytes = recv(socket_fd, (int *)&processed, sizeof(processed), 0); // recieve acknoledgement
        while(bytes_to_process > 0) {

            // send data, incrementing through pointer to string
            sent_recv_bytes = send(socket_fd, data_ptr, bytes_to_process, 0);
            // subtract number of bytes left to process
            bytes_to_process -= sent_recv_bytes;
            // add to data pointer so starts where last send left off
            data_ptr += sent_recv_bytes;
        }

        std::cout << "task_file sent\n";


        sent_recv_bytes = recv(socket_fd, (int *)&processed, sizeof(processed), 0); // sync
        // sent_recv_bytes = send(socket_fd, task_file.c_str(), task_file.length(), 0);

        sent_recv_bytes = send(socket_fd, exec_file.c_str(), exec_file.length(), 0);


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

        to_append.lock();
        std::ofstream fileOUT(file_out, std::ios::app);
        juice_output.append("\n");
        fileOUT << juice_output;
        fileOUT.close();
        to_append.unlock();

        sent_recv_bytes = recv(socket_fd, (int *)&processed, sizeof(processed), 0); // sync



    }
    









    close(socket_fd);



}


