#include <cassert>
#include <iostream>
#include <string>
#include <sstream>
#include <fstream>
#include <algorithm>
#include <vector>
#include <array>
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
            read_input(fin);
            Satellites.push_back({-1, -1,-1});
            Users.push_back({-1, -1,-1});
            Interferers.push_back({-1, -1,-1});
            num_connections.push_back(-1);
        }
        
        // initializes matrix that shows connection between Starlink and User
        void connection_to_Starlink_init(){
            vector<vector<char>> temp(Satellites.size(), vector<char> (Users.size(), '\0'));
            connection_to_Starlink = temp;
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
                    }
                }
            }
        }

        // Checks if Starlink is in range of user
        // Beam from Starlink to user must be 45 deg within users vertical
        // All users' normals pass through (0, 0, 0)
        bool user_in_Starlink_range(int sat_id, int user_id){
            Coords sat_coords = Satellites[sat_id];
            Coords user_coords = Users[user_id];
            double theta = -1;
            Coords u = user_coords;
            Coords v = {sat_coords.x - u.x, sat_coords.y - u.y, sat_coords.z - u.z};
            double u_dot_v = dot_product(u, v);
            return (theta <= 45);
        }
};

