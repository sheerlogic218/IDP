#include <vector>
#include <map>
#include <string>
#include <iostream> // Include iostream if you plan to use std::cout
using namespace std;

// Define direction constants as const to prevent modification
const int NORTH = 0;
const int EAST = 1;
const int SOUTH = 2;
const int WEST = 3;

int ROBOT_DIRECTION = NORTH; // Direction the robot is currently facing

// Maximum number of nodes in the navigation map
const int MAX_NODES = 16; // Updated to 16 to include node 15
bool junction_present = false; // Flag to indicate if a junction is present

// Adjacency map representing connections between nodes and their directions
map<int, std::map<int, int> > adjacent;

// Populates the adjacency map with node connections and directions
void populate_node_map()
{
    // Populate the adjacency list
    adjacent[0] = { {1, NORTH} };
    adjacent[1] = { {3, EAST}, {0, SOUTH}, {11, WEST} };
    adjacent[2] = { {11, NORTH} };
    adjacent[3] = { {5, NORTH}, {4, SOUTH}, {1, WEST} };
    adjacent[4] = { {3, NORTH} };
    adjacent[5] = { {7, NORTH}, {3, SOUTH}, {6, WEST} };
    adjacent[6] = { {14, NORTH}, {5, EAST}, {10, WEST} };
    adjacent[7] = { {5, SOUTH}, {12, WEST} };
    adjacent[8] = { {12, EAST}, {14, SOUTH}, {9, WEST} };
    adjacent[9] = { {8, EAST}, {10, SOUTH} };
    adjacent[10] = { {9, NORTH}, {6, EAST}, {11, SOUTH} };
    adjacent[11] = { {10, NORTH}, {1, EAST}, {2, SOUTH} };
    adjacent[12] = { {7, EAST}, {13, SOUTH}, {8, WEST} };
    adjacent[13] = { {12, NORTH} };
    adjacent[14] = { {8, NORTH}, {6, SOUTH}, {15, WEST} };
    adjacent[15] = { {14, EAST} };
}

// Generates a navigation path from the current node to the target node
vector<int> generateNavigationPath(int current_node, int target_node)
{
    int search_node = current_node; // Node currently being searched
    int depth = 0; // Current depth of the search
    int new_node; // Newly discovered node
    vector<int> current_route = {search_node}; // Current navigation route
    vector<vector<int>> possible_routes = vector<vector<int>>({current_route}); // List of possible routes to explore
    vector<int> found_nodes = {search_node}; // List of nodes that have been visited

    cout << "Starting navigation from node " << current_node << " to node " << target_node << endl;

    while(depth < 10) // Limit the search depth to prevent infinite loops
    {
        cout << "Depth: " << depth << ", Possible routes: " << possible_routes.size() << endl;

        for(int j = 0; j < possible_routes.size(); j++) // Iterate through all possible routes
        {
            current_route = possible_routes[j];
            search_node = current_route.back(); // Get the last node in the current route
            cout << "Extending route: ";
            for (int node : current_route) {
                cout << node << " ";
            }
            cout << endl;

            // Iterate over all adjacent nodes from the current search node
            for (pair<const int, int>& direction_pair : adjacent[search_node])
            {
                new_node = direction_pair.first; // Get the connected node
                vector<int> new_route = current_route;
                // Check if the new node has not been visited yet
                if(find(found_nodes.begin(), found_nodes.end(), new_node) == found_nodes.end())
                {
                    found_nodes.push_back(new_node); // Mark the new node as visited
                    new_route.push_back(new_node); // Add the new node to the current route
                    cout << "Found new node " << new_node << " from node " << search_node << endl;
                    if(new_node == target_node) // Check if the target node is reached
                    {
                        cout << "Target node " << target_node << " reached!" << endl;
                        return new_route; // Return the completed route
                    }
                    possible_routes.push_back(new_route); // Add the new route to the list of possible routes
                }  
            }
        }
        depth++; // Increment the search depth
    }
    cout << "No path found to target node " << target_node << endl;
    return vector<int>(); // Return empty vector if no path is found
}

void traverseToNode(int initial_behind_node, int initial_ahead_node, int goal_node)
{
    int behind = initial_behind_node;
    int ahead = initial_ahead_node;

    int route_progress = 1; //has the location of the 

    vector<int> path = generateNavigationPath(initial_ahead_node, goal_node); // Generate path from node a to node b

    while(route_progress < path.size())
    {
        handle_non_navigation_events_until_junction();

        // Junction reached
        // Set the robot direction based on the initial nodes
        ROBOT_DIRECTION = adjacent[behind][ahead];
        ahead = path[route_progress+1]; // Assuming the first node is the current node
        
        int next_direction = adjacent[ahead][next_node];
        int desired_turn = next_direction - ROBOT_DIRECTION;

        switch(desired_turn) {
            case NORTH:
                // Handle north turn
                break;
            case EAST:
                // Handle east turn
                break;
            case SOUTH:
                // Handle south turn
                break;
            case WEST:
                // Handle west turn
                break;
            default:
                // Handle default case
                break;
        }
        route_progress ++;
    }

}

void handle_non_navigation_events_until_junction()
{
    cout << "moving forward..." << endl;
}

int main() {

    populate_node_map(); // Initialize the adjacency map

    vector<int> path = generateNavigationPath(15, 0); // Generate path from node a to node b

    if (!path.empty()) {
        cout << "Path found: ";
        for (int node : path) {
            cout << node << " ";
        }
        cout << endl;
    } else {
        cout << "No path found." << endl;
    }

    return 0;
}