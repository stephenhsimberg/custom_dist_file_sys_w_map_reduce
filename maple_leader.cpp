#include "maple_juice.hpp"

// MP4 Code:
int maple_leader(int comm_socket_fd, std::string maple_exe, int num_tasks, std::string inter_prefix, std::string input_dir){
    for(int i = 0; i < 9; i++){
        failed.push_back(0);
    }

    std::string path(sysfile_path);
    for (auto &file_iterator : std::filesystem::recursive_directory_iterator(path)) {
        std::string cur_file_name = file_iterator.path().filename().string();
        if(cur_file_name.substr(0, input_dir.length()) == input_dir){
            std::cout << "found file\n";
            std::ifstream file(file_iterator.path(), std::ios::in | std::ios::binary);
            std::string temp;
            int line_num = 0;
            while (std::getline(file, temp)) {line_num++;}
            temp.clear();
            temp.shrink_to_fit();
            std::cout << line_num << " lines found in " << cur_file_name << "\n";
            int max_num_tasks = 0;
            for(int i = 1; i < 10; i++){
                if(machines_avail[i] == 1)
                    max_num_tasks++;

            }

            if(num_tasks > max_num_tasks){
                num_tasks = max_num_tasks;
            }

            for(int i = 1; i <= max_num_tasks; i++){
                if(num_tasks <= i){
                    break;
                }
                if(line_num <= i*100) {
                    num_tasks = i;
                    break;
                }
            }


            int lines_per_task = line_num / num_tasks;
            

            file.clear();
            file.seekg(0);
            std::vector<std::string> task_files(num_tasks);
            int num_lines = 0;
            std::string temp_file;
            // std::string st("select");
            // if(input_dir == st){
            //     getline(file, temp_file);
            //     for(int i = 0; i < num_tasks -1; i++){
            //         task_files[i].append(temp_file);
            //         temp_file.clear();
            //     }

            // }

            for(int i = 0; i < num_tasks -1; i++){
                while(num_lines <= lines_per_task){
                    getline(file, temp_file);
                    task_files[i].append(temp_file);
                    task_files[i].append("\n");
                    temp_file.clear();
                    num_lines++;
                }
                num_lines = 0;
            }
            task_files[num_tasks -1].append((std::istreambuf_iterator<char>(file)),(std::istreambuf_iterator<char>()));

            std::vector<int> working;
            for(int i = 0; i < 9; i++){
                working.push_back(0);
            }
            std::vector<std::thread> threads;
            std::string port("4100");
            int cur_index = 1;
            int i = 0; 
            while(i < num_tasks){
                if(machines_avail[cur_index]){
                    working[cur_index-1] = i;
                    std::cout << machine_names[cur_index] << "has task file" << i << "\n";
                    threads.emplace_back(setup_tcp_communication_client_maple, machine_names[cur_index], port, maple_exe, task_files[i], inter_prefix, cur_index-1);
                    i++;
                    if(i >= num_tasks){
                        break;
                    }
                }
                cur_index++;

                if(cur_index == 10){
                    std::cout << "should never hit\n";
                    break;
                }
                

            }

            
        
            for(i = 0; i < num_tasks; i++) {
                threads[i].join();
            }
            // threads.clear();
            std::vector<int> completed;
            num_tasks = 0;
            for(i = 0; i < 9; i++){
                int help_flag = 0;
                for(int k = 0; k < (int)completed.size(); k++){
                    if(completed[k] == i){
                        help_flag = 1;
                    }
                }
                if(failed[i] == -1 && help_flag == 0){
                    int new_idx;
                    std::cout << "map failure detected";
                    int task_to_sch = working[i];
                    int j;
                    for(j = 0; j < 9; j++){
                        if(machines_avail[j+1] == 1 && failed[j] != -1){
                            working[j] = task_to_sch;
                            std::cout << j << "\n";
                            break;
                        }
                    }
                    completed.push_back(i);

                    setup_tcp_communication_client_maple(machine_names[j+1], port, maple_exe, task_files[task_to_sch], inter_prefix, j);
                    i = 0;
                }

                
            }

            // std::cout << num_tasks << "\n";
            


            // use for testing
            // std::ofstream ofs;
            // std::ofstream fileOUT("testing.txt");
            // for(int i = 0; i < num_tasks; i++){
            //     std::cout << task_files[i];
            //     fileOUT << task_files[i];
            //     fileOUT << "break";
            //     std::cout << "break\n";
            // }
            // fileOUT.close();




            
        }
        // else {
        //     std::cout << cur_file_name << " was not input directory requested\n";
        // }









    }
    std::string join_name("combined");
    if(input_dir == join_name){
        system("rm sysfiles/combined");
    }
    

    return 0;
}


