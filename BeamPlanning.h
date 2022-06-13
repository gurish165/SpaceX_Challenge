#include <cassert>
#include <iostream>
#include <string>
#include <sstream>
#include <fstream>
#include <algorithm>
#include <vector>
#include <array>
#include <cmath>
#include <cstdlib>
#include <queue>
#include <stack>
#include <cstring>
#include "helpers.h"
#include "unordered_map"
using namespace std;

class BeamPlanning{
    private:
        vector<Coords> Satellites;
        vector<Coords> Users;
        vector<Coords> Interferers;
        // vector that tracks the number of users each Starlink is connected to
        // sums matrix from left to right
        vector<int> Starlink_num_connections;
        // vector that tracks the number of Starlinks each user is connected to
        // Sums matrix from top down
        vector<int> User_num_connections;
        // matrix where each row is each starlink satellite, and each column is a user
        // the character denotes type of connection
        vector<vector<char>> connection_to_Starlink;
        // 3D vector that contains angles between users on the same satellite
        // Each 'layer' represents the sat_id which is a matrix of user to user angles
        // vector<vector<vector<double>>> angles_between_users;
    public:

        // Default Constructor
        // Takes in a filestream
        // Sets Satellites, Users, and Interferes up for 1-indexing
        BeamPlanning(ifstream &fin){
            Satellites.push_back({-1, -1,-1});
            Users.push_back({-1, -1,-1});
            Interferers.push_back({-1, -1,-1});
            read_input(fin);
        } // BeamPlanning
        
        // initializes matrix that shows connection between Starlink and User
        void connection_to_Starlink_init(){
            vector<vector<char>> temp(Satellites.size(), vector<char> (Users.size(), '\0'));
            vector<int> temp_connections(Satellites.size(), 0);
            vector<int> temp_connections2(Users.size(), 0);
            // vector<vector<vector<double>>> angles_between_users(Satellites.size(), 
            //                             vector<vector<double>> (Users.size(), 
            //                             vector<double>(Users.size(), -1))); 
            connection_to_Starlink = temp;
            Starlink_num_connections = temp_connections;
            User_num_connections = temp_connections2;
        } // connection_to_Starlink_init

        // Fills priority queue with number of satellites and their indeces
        void init_pq(priority_queue<pair<int, int>> &pq){
            for (size_t sat_id = 1; sat_id < Satellites.size(); sat_id++){
                int count = Starlink_num_connections[sat_id];
                pq.push(make_pair(count, sat_id));
            }
        } // init_pq

        // Processes line from read_input
        // Adds coordinate objects to Satellites, Users, and Interferers
        void process_line(string line){
            string type = "";
            int id = 0;
            double x, y, z;
            istringstream iss(line);
            while(iss >> type >> id >> x >> y >> z){
                if (type == "sat"){
                    Satellites.push_back({x, y, z});
                }
                else if (type == "user"){
                    Users.push_back({x, y, z});
                }
                else if (type == "interferer"){
                    Interferers.push_back({x, y, z});
                }
                else{
                    cerr << "Invalid object" << endl;
                    cerr << "Usage: sat, user, interferer" << endl;
                    exit(1);
                } // end if
            } // end while
        } // process_line

        // Processes filestream
        // Takes in filestream
        void read_input(ifstream &fin){
            cout << "# Reading input..." << endl;
            string line = "";
            while(getline(fin, line)){
                if(line[0] != '#' && line != ""){
                    process_line(line);
                } // end if
            } // end while
        } // read_input

        // Makes connection between all Starlinks sats and users
        // Only follows the 45 deg constraint
        void connect_all_Starlinks(){
            connection_to_Starlink_init();
            for (size_t sat_id = 1; sat_id < Satellites.size(); sat_id++){
                for (size_t user_id = 1; user_id < Users.size(); user_id++){
                    if(user_in_Starlink_range(sat_id, user_id)){
                        connection_to_Starlink[sat_id][user_id] = 'A';
                        Starlink_num_connections[sat_id]++;
                        User_num_connections[user_id]++;
                    }
                }
            }
        } // connect_all_Starlinks

        // Checks if Starlink is in range of user
        // Beam from Starlink to user must be 45 deg within users vertical
        // All users' normals pass through (0, 0, 0)
        bool user_in_Starlink_range(size_t &sat_id, size_t &user_id){
            Coords sat_coords = Satellites[sat_id];
            Coords user_coords = Users[user_id];
            double theta = -1;
            Coords u = user_coords;
            Coords v = {sat_coords.x - u.x, sat_coords.y - u.y, sat_coords.z - u.z};
            theta = angle_between_vec(u, v);
            return (theta <= 0.785398);
        } // user_in_Starlink_range

        // Loops through all interference satellites
        // Checks if there is interference <20 deg
        // If there is, the connection from Starlink to user becomes /0
        void cleanInterference(){
            for(size_t int_id = 1; int_id < Interferers.size(); int_id++){
                for (size_t sat_id = 1; sat_id < Satellites.size(); sat_id++){
                    for (size_t user_id = 1; user_id < Users.size(); user_id++){
                        if(connection_to_Starlink[sat_id][user_id] && interferes_with_Starlink(int_id, sat_id, user_id)){
                            connection_to_Starlink[sat_id][user_id] = '\0';
                            Starlink_num_connections[sat_id]--;
                            User_num_connections[user_id]--;
                        }
                    }
                }
            }
        } // cleanInterference
        
        // Determines if Interference Sat interferes with Starlink
        // Interference beam cannot be with 20 deg of Starlink beam from user's perspective
        bool interferes_with_Starlink(size_t &intf_id, size_t &sat_id, size_t &user_id){
            Coords sat_coords = Satellites[sat_id];
            Coords user_coords = Users[user_id];
            Coords intf_coords = Interferers[intf_id];
            double theta = -1;
            Coords u = {sat_coords.x - user_coords.x, sat_coords.y - user_coords.y, sat_coords.z - user_coords.z};
            Coords v = {intf_coords.x - user_coords.x, intf_coords.y - user_coords.y, intf_coords.z - user_coords.z};
            theta = angle_between_vec(u, v);
            return (theta < 0.349066);
        } // interferes_with_Starlink

        // For satellites with more than 32 connections, we clean out connection to
        //      users that already have service
        void rebalance32(){
            // Priority queue where the key is the number of people the Starlink
            //      is connected to and the value is the index of the Satellite
            priority_queue<pair<int, int>> pq_connections;
            init_pq(pq_connections);
            while(!pq_connections.empty()){
                pair<int, int> curr = pq_connections.top();
                if(curr.first > 32){
                    int sat_id = curr.second;
                    disconnect_multi_served_users(sat_id);
                }
                pq_connections.pop();
            }
        } // rebalance32

        // Loops through a Satellites connections (left to right)
        // For every user connected to that satellite see if they are connected to another
        // If they are, terminate connection to sat_id
        // Update row and column totals
        void disconnect_multi_served_users(int sat_id){
            for (size_t user_id = 1; user_id < Users.size(); user_id++){
                if(connection_to_Starlink[sat_id][user_id]){
                    if(User_num_connections[user_id] > 1){
                        // terminate current connection
                        connection_to_Starlink[sat_id][user_id] = '\0';
                        Starlink_num_connections[sat_id]--;
                        User_num_connections[user_id]--;
                    }
                }
            }
        } // disconnect_mutli_served_users

        // Looks for satellites with more than 32 connections
        // Terminate extra connections
        void cleanExtra(){
            for(size_t sat_id = 1; sat_id < Satellites.size(); sat_id++){
                for(size_t user_id = 1; user_id < Users.size(); user_id++){
                    if(Starlink_num_connections[sat_id] > 32 && connection_to_Starlink[sat_id][user_id]){
                        // Double check that the user will still be connected if this is terminated
                        if(User_num_connections[user_id] > 1){
                            connection_to_Starlink[sat_id][user_id] = '\0';
                            Starlink_num_connections[sat_id]--;
                            User_num_connections[user_id]--;
                        }
                    }
                }
            }
            for(size_t sat_id = 1; sat_id < Satellites.size(); sat_id++){
                for(size_t user_id = 1; user_id < Users.size(); user_id++){
                    if(Starlink_num_connections[sat_id] > 32 && connection_to_Starlink[sat_id][user_id]){
                        connection_to_Starlink[sat_id][user_id] = '\0';
                        Starlink_num_connections[sat_id]--;
                        User_num_connections[user_id]--;
                    }
                }
            }
        } // cleanExtra

        // Loop through users
        // If a user is connected to multiple satellites, cut down to 1
        // Choose the ones ones to connect by exploring how many <10 deg issues that
        //      beam causes (cut the ones that cause the most interference 
        //      within their satellite)
        // Save angles between connections to use later
        void rebalanceUserMultiConnections(){
            for(size_t user_id = 1; user_id < Users.size(); user_id++){
                if(User_num_connections[user_id] > 1){
                    size_t sat_id_with_least_interferences = 0;
                    int min_interferences = 21474836;
                    for(size_t sat_id = 1; sat_id < Satellites.size(); sat_id++){
                        if(connection_to_Starlink[sat_id][user_id]){
                            int num_interferences = calculate_num_interferences(sat_id, user_id);
                            if(num_interferences < min_interferences){
                                min_interferences = num_interferences;
                                sat_id_with_least_interferences = sat_id;
                            }
                        }
                    }
                    reduce_to_one(user_id, sat_id_with_least_interferences);
                }
            }
            // make sure no user is connected to more than one Starlink
            for (size_t user_id = 1; user_id < Users.size(); user_id++){
                assert(User_num_connections[user_id] <= 1);
            }
        } // rebalanceUserMultiConnections

        // The User and Starlink sat form a connection
        // Calculates how many self interferences this connection makes
        // User angles_between_users to preserve angle measurements
        int calculate_num_interferences(size_t sat_id, size_t user_id){
            int total = 0;
            Coords sat_coords = Satellites[sat_id];
            Coords user_coords = Users[user_id];
            Coords u = {user_coords.x - sat_coords.x, user_coords.y - sat_coords.y, user_coords.z - sat_coords.z};
            for(size_t other_user_id = 1; other_user_id < Users.size(); other_user_id++){
                // Satellite must serve other user
                // Other user must not be themselves
                if (connection_to_Starlink[sat_id][other_user_id] && user_id != other_user_id){
                    Coords other_user = Users[other_user_id];
                    Coords v = {other_user.x - sat_coords.x, other_user.y - sat_coords.y, other_user.z - sat_coords.z};
                    // see if we alreay have angle_between saved, otherwise calculate it
                    double theta = angle_between_vec(u, v);
                    // see if angle is smaller than 10 degrees
                    if (theta < 0.174533){
                        total++;
                    }
                }
                
            }
            return total;
        } // calculate_num_interferences

        // Makes the user only connect to the Starlink with least conflicts
        // Updates User_num_connections and Starlink_num_connections
        // Updates connection_to_Starlink
        void reduce_to_one(size_t user_id, size_t Starlink_with_least_conflicts){
            for (size_t sat_id = 1; sat_id < Satellites.size(); sat_id++){
                if(connection_to_Starlink[sat_id][user_id] && sat_id != Starlink_with_least_conflicts){
                    connection_to_Starlink[sat_id][user_id] = '\0';
                    Starlink_num_connections[sat_id]--;
                    User_num_connections[user_id]--;
                }
            }
        } // reduce_to_one

        // Searches satellite connections using breadth first search
        // Labels connections with A, B, C, D
        void DFS(size_t sat_id){
            // hashmap that maps from user_id to visited bool
            unordered_map<size_t, bool> visited;
            for(size_t user_id = 1; user_id < Users.size(); user_id++){
                // Add connected users to visited
                if(connection_to_Starlink[sat_id][user_id]){
                    visited[user_id] = false;
                }
            }
            size_t root_user = 0;
            while(available_root(root_user, visited)){
                visited[root_user] = true;
                // stack of user_ids
                stack<size_t> user_stack;
                size_t letter = 1;
                user_stack.push(root_user);
                while(!user_stack.empty()){
                    size_t top_user = user_stack.top();
                    user_stack.pop();
                    if(connection_to_Starlink[sat_id][top_user]){
                        connection_to_Starlink[sat_id][top_user] = get_label(letter);
                    }
                    letter++;
                    vector<size_t> adjacent_users = get_adjacent_users(top_user, sat_id);
                    for(size_t adj_user_idx = 0; adj_user_idx < adjacent_users.size(); adj_user_idx++){
                        if(!visited[adjacent_users[adj_user_idx]]){
                            visited[adjacent_users[adj_user_idx]] = true;
                            user_stack.push(adjacent_users[adj_user_idx]);
                        }
                    }
                } // while
            } // while
        } // DFS

        // Checks to see if there is another available root
        // If there is, update root_user
        bool available_root(size_t &root_user, unordered_map<size_t, bool> &visited){
            for(auto user : visited){
                if(!user.second){
                    // cout << "Found new root: " << user.first << endl;
                    root_user = user.first;
                    return true;
                }
            }
            return false;
        } // available_root

        // Explores all adjacent users to top_user
        // Adds their user_id and angle from top_user to vector
        vector<size_t> get_adjacent_users(size_t user_id, size_t sat_id){
            vector<size_t> output;
            Coords sat_coords = Satellites[sat_id];
            Coords user_coords = Users[user_id];
            Coords u = {user_coords.x - sat_coords.x, user_coords.y - sat_coords.y, user_coords.z - sat_coords.z};
            for(size_t other_user_id = 1; other_user_id < Users.size(); other_user_id++){
                if (connection_to_Starlink[sat_id][other_user_id] && user_id != other_user_id){
                    Coords other_user = Users[other_user_id];
                    Coords v = {other_user.x - sat_coords.x, other_user.y - sat_coords.y, other_user.z - sat_coords.z};
                    // see if we alreay have angle_between saved, otherwise calculate it
                    double theta = angle_between_vec(u, v);
                    // see if angle is smaller than 10 degrees
                    if (theta < 0.174533){
                        output.push_back(other_user_id);
                    }
                }
            }
            return output;
        } // get_adjacent_users

        // Changes frequency of beams for every satellite
        void labelWithDFS(){
            for(size_t sat_id = 1; sat_id < Satellites.size(); sat_id++){
                if(Starlink_num_connections[sat_id] > 1){
                    DFS(sat_id);
                }
            }
        } // labelWithDFS

        // For every satellite, removes conflicting frequencies
        void removeConflictingFreq(){
            for(size_t sat_id = 1; sat_id < Satellites.size(); sat_id++){
                for(size_t user1 = 1; user1 < Users.size(); user1++){
                    if(connection_to_Starlink[sat_id][user1]){
                        for(size_t user2 = user1+1; user2 < Users.size(); user2++){
                            if(connection_to_Starlink[sat_id][user2] && 
                               connection_to_Starlink[sat_id][user1] == connection_to_Starlink[sat_id][user2] && 
                               freq_interferes(user1, user2, sat_id)){
                                connection_to_Starlink[sat_id][user1] = '\0';
                                Starlink_num_connections[sat_id]--;
                                User_num_connections[user1]--;
                            }
                        }
                    }
                }
            }
        } // removeConflictingFreq

        bool freq_interferes(size_t user_id, size_t other_user_id, size_t sat_id){
            Coords sat_coords = Satellites[sat_id];
            Coords user_coords = Users[user_id];
            Coords u = {user_coords.x - sat_coords.x, user_coords.y - sat_coords.y, user_coords.z - sat_coords.z};
            Coords other_user = Users[other_user_id];
            Coords v = {other_user.x - sat_coords.x, other_user.y - sat_coords.y, other_user.z - sat_coords.z};
            // see if we alreay have angle_between saved, otherwise calculate it
            double theta = angle_between_vec(u, v);
            // see if angle is smaller than 10 degrees
            if (theta < 0.174533){
                return true;
            }
            return false;
        } // freq_interferes

        // Prints out all the connections between Starlink and Users
        // Style:
        // sat 1 beam 1 user 1 color A
        // sat 2 beam 1 user 2 color A
        void printResults(){
            for(size_t sat_id = 1; sat_id < Satellites.size(); sat_id++){
                int beam_count = 1;
                for (size_t user_id = 1; user_id < Users.size(); user_id++){
                    if(connection_to_Starlink[sat_id][user_id]){
                        cout << "sat " << sat_id << " beam " << beam_count
                            << " user " << user_id << " color " << connection_to_Starlink[sat_id][user_id]
                            << endl;
                            beam_count++;
                    }
                }
            }
            int total = 0;
            for(size_t user_id = 1; user_id < Users.size(); user_id++){
                if(User_num_connections[user_id] > 0)total++;
            }
            cout << "# " << total << " users served" << endl;
        } // printResults

        // Runs optimization model
        void runOptimization(){
            // Connects valid Starlinks to users
            cout << "# connect_all_Starlinks() in progress..." << endl;
            connect_all_Starlinks();
            // Removes connections with interference
            cout << "# cleanInterference() in progress..." << endl;
            cleanInterference();
            // Rebalances Starlinks with >32 connections
            cout << "# rebalance32() in progress..." << endl;
            rebalance32();
            // Removes connections for Starlinks with >32 connections
            cout << "# cleanExtra() in progress..." << endl;
            cleanExtra();
            // Rebalances so each user is only connected to 1 satellite
            cout << "# rebalanceUserMultiConnections() in progress..." << endl;
            rebalanceUserMultiConnections();
            // Uses DFS to color the beams for every satellite
            cout << "# labelWithDFS() in progress..." << endl;
            labelWithDFS();
            // Remove conflicting beam frequencies
            cout << "# removeConflictingFreq() in progress..." << endl;
            removeConflictingFreq();
            // Prints output
            printResults();
        } // runOptimization
};

