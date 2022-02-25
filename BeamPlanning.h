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
#include <cstring>
#include "helpers.h"
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
        vector<vector<vector<double>>> angles_between_users;
    public:

        // Default Constructor
        // Takes in a filestream
        // Sets Satellites, Users, and Interferes up for 1-indexing
        BeamPlanning(ifstream &fin){
            Satellites.push_back({-1, -1,-1});
            Users.push_back({-1, -1,-1});
            Interferers.push_back({-1, -1,-1});
            read_input(fin);
        }
        
        // initializes matrix that shows connection between Starlink and User
        void connection_to_Starlink_init(){
            vector<vector<char>> temp(Satellites.size(), vector<char> (Users.size(), '\0'));
            vector<int> temp_connections(Satellites.size(), 0);
            vector<int> temp_connections2(Users.size(), 0);
            vector<vector<vector<double>>> angles_between_users(Satellites.size(), 
                                        vector<vector<double>> (Users.size(), 
                                        vector<double>(Users.size(), -1))); 
            connection_to_Starlink = temp;
            Starlink_num_connections = temp_connections;
            User_num_connections = temp_connections2;
        }

        // Fills priority queue with number of satellites and their indeces
        void init_pq(priority_queue<pair<int, int>> &pq){
            for (size_t sat_id = 1; sat_id < Satellites.size(); sat_id++){
                int count = Starlink_num_connections[sat_id];
                pq.push(make_pair(count, sat_id));
            }
        }

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
        }

        // Processes filestream
        // Takes in filestream
        void read_input(ifstream &fin){
            string line = "";
            while(getline(fin, line)){
                if(line[0] != '#' && line != ""){
                    process_line(line);
                } // end if
            } // end while
        }

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
        }

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
        }

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
            for(size_t user_id = 1; user_id < Users.size(); user_id++){
                if (User_num_connections[user_id] > 1){
                    cout << "User " << user_id << " connected to " << User_num_connections[user_id] << " satellites" << endl; 
                }
            }
        }

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
        }
        
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
        }

        // For satellites with more than 32 connections, we clean out connection to
        //      users that already have service
        void rebalance32(){
            // Priority queue where the key is the number of people the Starlink
            //      is connected to and the value is the index of the Satellite
            priority_queue<pair<int, int>> pq_connections;
            init_pq(pq_connections);
            if (pq_connections.empty()) return;
            while(!pq_connections.empty()){
                pair<int, int> curr = pq_connections.top();
                if(curr.first > 32){
                    int sat_id = curr.second;
                    disconnect_multi_served_users(sat_id);
                }
                pq_connections.pop();
            }
        }   

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
        }

        // Looks for satellites with more than 32 connections
        // Terminate extra connections
        void cleanExtra(){
            for(size_t sat_id = 1; sat_id < Satellites.size(); sat_id++){
                for(size_t user_id = 1; user_id < Users.size(); user_id++){
                    if(Starlink_num_connections[sat_id] > 32 && connection_to_Starlink[sat_id][user_id]){
                        // Double check that the user will still be connected if this is terminated
                        assert(User_num_connections[user_id] > 1);
                        connection_to_Starlink[sat_id][user_id] = '\0';
                        Starlink_num_connections[sat_id]--;
                        User_num_connections[user_id]--;
                    }
                }
            }
        }

        // Loop through users
        // If a user is connected to multiple satellites, cut down to 1
        // Choose the ones ones to connect by exploring how many <10 deg issues that
        //      beam causes (cut the ones that cause the most interference 
        //      within their satellite)
        // Save angles between connections to use later
        void rebalanceUserMultiConnections(){

        }

        // Runs optimization model
        void runOptimization(){
            // Connects valid Starlinks to users
            connect_all_Starlinks();
            // Removes connections with interference
            cleanInterference();
            // Rebalances Starlinks with >32 connections
            rebalance32();
            // Removes connections for Starlinks with >32 connections
            cleanExtra();
            // Rebalances so each user is only connected to 1 satellite
            rebalanceUserMultiConnections();
            // Uses BFS to color the beams for every satellite
            // Prints output
            printResults();
        }
};

