maple_juice: main.cpp gossip_client.cpp gossip_server.cpp sdfs_leader.cpp sdfs_server.cpp sdfs_client.cpp maple_client.cpp maple_server.cpp maple_leader.cpp juice_client.cpp juice_leader.cpp juice_server.cpp
	g++ -g main.cpp gossip_client.cpp gossip_server.cpp sdfs_leader.cpp sdfs_server.cpp sdfs_client.cpp maple_client.cpp maple_server.cpp maple_leader.cpp juice_client.cpp juice_leader.cpp juice_server.cpp -std=c++17 -lstdc++fs -pthread -o maple_juice

clean:
	rm *.o maple_juice
