# Custom Distributed File System with custom Map Reduce and Failure Detector.

1) In main.cpp, change sysfile_path to be path to this directory.
2) compile using 'make maple_juice', the name of our custom map reduce.
3) Use ./maple_juice to run.


# Commands:
# For failure detector:
1) list_mem: Print out current machine's membership list
2) list_self: lists self from the membership list
3) enable suspicion: enables Gossip Failure Detector with Suspicion for that machine (on by default)
4) disable suspicion: disables suspicion of failure detector for that machine

For next commands an acknoledgement is displayed at end of each operation in SDFS.
# For Distributed File System:
1) put localfilename sdfsfilename: Writes file to distributed filesystem from local directory (directory where .cpp files are)
2) get sdfsfilename localfilename: Reads files from distributed filesystem to local directory
3) delete sdfsfilename: deletes file frm distributed filesystem
4) ls sdfsfilename: list all VM addresses where this file is currently replicated
5) store: list the set of file names that are replicated (stored) on SDFS at this (local) process/VM

# For Maple Juice (Custom Map Reduce)
1) maple <maple_exe> <num_maples> <sdfs_intermediate_filename_prefix> <sdfs_src_directory> <arg>:
       * This allows for users to use their own exe map file when executable is in local directory of every machine. arg is 'none' if executable takes no arguments.
       * num_maples is the number of map tasks assigned, max is number of machines (will default to if too high).
       * input_dir and intermediate_dir MUST be different.  
3) juice <juice_exe> <num_juices> <sdfs_intermediate_filename_prefix> <sdfs_dest_filename> delete_input={0,1}:
       * Same as map, but for reduce.
       * intermediate_dir MUST be same as maple
4) SELECT <sdfs file_dir or file> <regex>: Combines map and reduce tasks to one SQL query lauch: Took 10s on >300 MB file (faster than Hadoop!)
5) JOIN <dataset1> <integer_colomn_of_D1> = <dataset2> <integer_colomn_of_D2>: launches SQL style JOIN on csv file.

All input files must have been written to the SDFS before launching.






Thank you!
       
