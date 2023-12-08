#include "maple_juice.hpp"



int juice_leader(int comm_socket_fd, std::string juice_exe, int num_tasks, std::string inter_prefix, std::string output_f_name, int to_delete){
    std::string path(sysfile_path);
    inter_prefix.append("_");

    std::ofstream ofs;
    ofs.open(output_f_name, std::ofstream::out | std::ofstream::trunc);
    ofs.close();

    /* range vs hash:

    range: A-C = 1602
           D-F = 1603
           G-I = 1604
           J-L = 1605
           M-O = 1606
           P-R = 1608
           S-U = 1609
           W-Z = 1610


    */
    std::vector<int> avail;
    int max_num_tasks = 0;
    for(int i = 1; i < 10; i++){
        if(machines_avail[i] == 1 && failed[i-1] != -1){
            std::cout << i << "\n";
            max_num_tasks++;
            avail.push_back(i);
        }

    }

    // if(num_tasks > max_num_tasks){
        num_tasks = max_num_tasks;
    // }



    std::vector<std::vector<std::string>> task_files(num_tasks);
    std::cout << inter_prefix;
    for (auto &file_iterator : std::filesystem::recursive_directory_iterator(path)) {
        std::string cur_file_name = file_iterator.path().filename().string();
        // std::cout << cur_file_name.substr(0, inter_prefix.length());
        if(cur_file_name.substr(0, inter_prefix.length()) == inter_prefix){
           

            int machine_id = ((int)cur_file_name.substr(inter_prefix.length())[0]) % num_tasks;
            // while(machines_avail[machine_id] != 1 || failed[machine_id] == 1){
            //     machine_id = (rand() % num_tasks);
            // }
            std::ifstream file(file_iterator.path(), std::ios::in | std::ios::binary);
            task_files[machine_id].push_back(std::string((std::istreambuf_iterator<char>(file)),(std::istreambuf_iterator<char>())));




            
            

        }
       
    }

    // use for testing
    // std::ofstream ofs;
    // std::ofstream fileOUT("testing.txt");
    // for(int i = 0; i < num_tasks; i++){
    //     for(int j = 0; j < (int)task_files[i].size(); i++){
    //         // std::cout << task_files[i];
    //         fileOUT << task_files[i][j];
    //         fileOUT << "break\n";
    //         // std::cout << "break\n";
    //     }
    // }
    // fileOUT.close();

     
    std::vector<std::thread> threads;
    std::string port("4101");
    for(int i = 0; i < num_tasks; i++){


        threads.emplace_back(setup_tcp_communication_client_juice, machine_names[avail[i]], port, juice_exe, task_files[i], output_f_name);
    }

    for(int i = 0; i < num_tasks; i++) {
        threads[i].join();
    }
        

    std::string to_remove("rm -rf sysfiles/" + inter_prefix + "*");
    system(to_remove.c_str());
    




    return 0;
}

