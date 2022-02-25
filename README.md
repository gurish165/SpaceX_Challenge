
# Starlink Beam Planning Tech Test

This document describes how to use the code package that will complete the Starlink coding challenge for the 2022 software engineering internship.

## Setup and Installation

Download and unzip files from folder shared via email.

This package requires g++ (GNU c++ compiler) to run.


### macOS
macOS has a built-in shell. Open the “Terminal” application.

Install a compiler.

```cmd
$ xcode-select --install
```
This compiler is really Apple LLVM pretending to be `g++`.
```cmd
$ g++ --version
Configured with: --prefix=/Library/Developer/CommandLineTools/usr 
--with-gxx-include-dir=/usr/include/c++/4.2.1
Apple LLVM version 9.0.0 (clang-900.0.38)
Target: x86_64-apple-darwin16.7.0
Thread model: posix
InstalledDir: /Library/Developer/CommandLineTools/usr/bin
```

Install the [Homebrew package manager](https://brew.sh/).

Use Homebrew to install a few command line programs.
```
$ brew install wget git tree
```

### Windows
Use Windows 10 Subsystem for Linux (WSL). WSL runs native Linux command-line tools directly on Windows.

Start PowerShell and run it as administrator. Search for PowerShell in the start menu, then right-click and select “Run as administrator”.

Next, [follow the instructions from Microsoft](https://msdn.microsoft.com/en-us/commandline/wsl/install-win10). Be sure to select “Ubuntu Linux”.

Start an Ubuntu Bash shell (not a Windows PowerShell).

Run the following commands in your Bash shell:
```
$ sudo apt-get update
$ sudo apt-get install g++ make rsync wget git ssh gdb
```

### Check Tools
After you’ve installed a command line interface, you should have all these command line programs installed. Your versions might be different.
```
$ g++ --version
g++ (GCC) 6.4.0
$ make --version
GNU Make 4.2.1
$ gdb --version   # Windows and Linux only
GNU gdb (Ubuntu 8.1-0ubuntu3) 8.1.0.20180409-git
$ lldb --version  # macOS only
lldb-900.0.64
$ rsync --version
rsync  version 3.1.2  protocol version 31
$ wget --version
GNU Wget 1.19.4 built on linux-gnu.
$ git --version
git version 2.15.1
$ ssh -V
OpenSSH_7.6p1, OpenSSL 1.0.2m  2 Nov 2017
```

You should now be able to open and run the files for this challenge. Windows users should ensure they are using the Remote-WSL in their IDE.

## Usage

Locate the unzipped files in your terminal. Mine looks like this:
```python
$ pwd
/mnt/c/Users/gurish/OneDrive/Documents/SpaceX_Challenge
```
Compile the files
```
$ make main_executable
```
Run test cases in the format: 
`./my_executable test_cases/<TEST_CASE> |./evaluate.py test_cases/<TEST_CASE>`

Here is an example:
```
$ ./my_executable test_cases/00_example.txt |./evaluate.py test_cases/00_example.txt
Reading scenario file test_cases/00_example.txt
Reading solution from stdin.
Checking user coverage...
100.00% of 3 total users covered.
Checking each user can see their assigned satellite...
        All users' assigned satellites are visible.
Checking no sat interferes with itself...
        No satellite self-interferes.
Checking no sat interferes with a non-Starlink satellite...
        No satellite interferes with a non-Starlink satellite!

Solution passed all checks!
```

## Testing Results
|Test| Percent Covered | Total Users |Passed Checks |
|--|--|--|--|
| 00_example| 100% |3|Yes|
| 01_simplest_possible| 100% |1|Yes|
| 02_two_users| 100% |2|Yes|
| 03_five_users| 80% |5|Yes|
| 04_one_interferer| 0% |1|Yes|
| 05_equatorial_plane| 98.5% |1000|Yes|
| 06_partially_fullfillable| 64.76% |2500|Yes|
| 07_eighteen_planes| 72.04% |2500|Yes|
| 08_eighteen_planes_northern| 49.68% |2500|Yes|
| 09_ten_thousand_users| 60.02% |10000|Yes|
| 10_ten_thousand_users_geo_belt| 57.12% |10000|Yes|
| 11_one_hundred_thousand_users| 15.887% |100000|Yes|

## Algorithm

The main optimization problem is broken down into 8 steps. Each step addresses a constraint and improves the connections between Starlink satellites and users.

### Preprocessing
In order to run the algorithm, we must read input in from a filestream. Lines are read in, cleaned up, and then added to containers where Users, Starlink satellites, and Interference satellites are kept track of.
```c++
void read_input(ifstream &fin){
	string  line = "";
	while(getline(fin, line)){
		if(line[0] != '#' && line  !=  ""){
			process_line(line);
		} 
	} 
} // read_input
``` 
### Make Multiple Connections
We begin by connecting every possible Starlink satellite to every possible user. We are able to keep track of connections between the two in a matrix. The matrix will have a dummy row and column so we can use the matrix with 1-indexing. This step also enforces that the users are within 45 degrees of the Starlink satellite. We will use 'A' to denote a successful connection.
```c++
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
```
### Meet Interference Constraint
Given that we have a constraint that states that beams from Starlink satellites must not be within 20 degrees of a beam from a non-Starlink satellite, let's clean up beams that are not possible. We will do this by going through every non-Starlink satellite, make a connection with a user, and compare with other Starlink beams the user may be connected to. This will eliminate all the Starlink beams that would be unusable due to the non-Starlink beams
```c++
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
```

### Meet 32 Connection Constraint
At this point we have reduced our connection space to all connections that can possibly be made by a Starlink satellite. Now we have to determine which connections to preserve to meet the remaining constraints. I decided it was most logical to clean up Starlink satellites with >32 connections, starting with the satellite with the most connections.
```c++
void rebalance32(){
	// Priority queue where the key is the number of people the Starlink
	// is connected to and the value is the index of the Satellite
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
```
We are able to accomplish this by taking a look at users the satellite is connected to. If the satellite is connected to a user that is already being served by another satellite, we can terminate the connection. This will reduce the number of connections per satellite.

Once we have cleaned up the satellites with >32 connections in order from greatest load to smallest, we will repeat this process with random selection to put all satellites under the 32 count threshold.

```c++
void cleanExtra(){
	for(size_t sat_id = 1; sat_id < Satellites.size(); sat_id++){
		for(size_t  user_id = 1; user_id < Users.size(); user_id++){
			if(Starlink_num_connections[sat_id] > 32 && connection_to_Starlink[sat_id][user_id]){
				connection_to_Starlink[sat_id][user_id] = '\0';
				Starlink_num_connections[sat_id]--;
				User_num_connections[user_id]--;
			}
		}
	}
} // cleanExtra
```
### Rebalance User Connections
Now that we have guaranteed that each Starlink satellite is connected to at most 32 users, we have to ensure that each user is only connected to 1 Starlink satellite. Suppose that a user is connected to 3 Starlink satellites currently. How do we decide which one to connect to? We will pick the connection that causes the least amount of interference with the satellites other beams. 
Suppose:
* Satellite 1s connection to the user is within 10 degrees of 5 of its own beams
* Satellite 2s connection to the user is within 10 degrees of 4 of its own beams
* Satellite 3s connection to the user is within 10 degrees of 1 of its own beams

We will preserve the connection between the user and Satellite 3 because it causes the least 'load'. Removing the user's connection with the other satellites will also make it easier to determine frequencies for each beam.
```c++
void rebalanceUserMultiConnections(){
	for(size_t  user_id = 1; user_id < Users.size(); user_id++){
		if(User_num_connections[user_id] > 1){
			size_t  sat_id_with_least_interferences = 0;
			int  min_interferences = INT_MAX;
			for(size_t  sat_id = 1; sat_id < Satellites.size(); sat_id++){
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
	for (size_t  user_id = 1; user_id < Users.size(); user_id++){
		assert(User_num_connections[user_id] <= 1);
	}
} // rebalanceUserMultiConnections
```

### Assign Frequencies to Beams

## Authors and acknowledgment
[Gurish Sharma](https://www.linkedin.com/in/gurish-sharma-/)