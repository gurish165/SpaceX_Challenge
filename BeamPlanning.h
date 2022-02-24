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
#include <cstring>
#include "helpers.h"
using namespace std;

class BeamPlanning{
    private:
        vector<Coords> Satellites;
        vector<Coords> Users;
        vector<Coords> Interferers;
        // vector that tracks the number of users each Starlink is connected to
        vector<int> num_connections;
        // matrix where each row is each starlink satellite, and each column is a user
        // the character denotes type of connection
        vector<vector<char>> connection_to_Starlink;
    public:

        // Default Constructor
        // Takes in a filestream
        // Sets Satellites, Users, and Interferes up for 1-indexing
        BeamPlanning(ifstream &fin){
            Satellites.push_back({-1, -1,-1});
            Users.push_back({-1, -1,-1});
            Interferers.push_back({-1, -1,-1});
            num_connections.push_back(-1);
            read_input(fin);
        }
        
        // initializes matrix that shows connection between Starlink and User
        void connection_to_Starlink_init(){
            vector<vector<char>> temp(Satellites.size(), vector<char> (Users.size(), '\0'));
            vector<int> temp_connections(Satellites.size(), 0)
            connection_to_Starlink = temp;
            num_connections = temp_connections;
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
                        num_connections[sat_id]++;
                    }
                }
            }
        }

        // Checks if Starlink is in range of user
        // Beam from Starlink to user must be 45 deg within users vertical
        // All users' normals pass through (0, 0, 0)
        bool user_in_Starlink_range(int &sat_id, int &user_id){
            Coords sat_coords = Satellites[sat_id];
            Coords user_coords = Users[user_id];
            double theta = -1;
            Coords u = user_coords;
            Coords v = {sat_coords.x - u.x, sat_coords.y - u.y, sat_coords.z - u.z};
            theta = angle_between_vec(u, v)
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
                            num_connections[sat_id]--;
                        }
                    }
                }
            }
        }
        
        // Determines if Interference Sat interferes with Starlink
        bool interferes_with_Starlink(int &intf_id, int &sat_id, int &user_id){
            Coords sat_coords = Satellites[sat_id];
            Coords user_coords = Users[user_id];
            Coords intf_coords = Interferers[intf_id];
            double theta = -1;
            Coords u = {sat_coords.x - user_coords.x, sat_coords.y - user_coords.y, sat_coords.z - user_coords.z};
            Coords v = {intf_coords.x - user_coords.x, intf_coords.y - user_coords.y, intf_coords.z - user_coords.z};
            theta = angle_between_vec(u, v);
            return (theta < 20);
        }

        // Connects valid Starlinks to users
        // Removes connections with interference
        // Rebalances Starlinks with >32 connections
        // Uses BFS to color the beams for every satellite
        // Prints output
        void runOptimization(){
            connect_all_Starlinks();
            cleanInterference();
            printResults();
        }
};

