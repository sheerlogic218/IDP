
const int NORTH = 0;
const int EAST = 1;
const int SOUTH = 2;
const int WEST = 3;


int node_map[][] = {
    {{1,NORTH}},//0
    {{0,SOUTH},{3,EAST},{11,WEST}},//1
    {{11,NORTH}},//2
    {{5,NORTH},{4,SOUTH},{1,WEST}},//3
    {{3,NORTH}},//4
    {{7,NORTH},{3,SOUTH},{6,WEST}},//5
    {{14,NORTH},{5,EAST},{10,WEST}},//6
    {{5,SOUTH},{12,WEST}},//7
    {{12,EAST},{14,SOUTH},{9,WEST}}, //8
    {{8,EAST},{10,SOUTH}},//9
    {{9,NORHT},{6,EAST},{11,SOUTH}},//10
    {{10,NORTH},{1,EAST},{2,SOUTH}},//11
    {{7,EAST},{13,SOUTH},{8,WEST}},//12
    {{12,NORTH}},//13
    {{6,SOUTH},{8,NORTH},{15,WEST}},//14
    {{14,EAST}}//15
}

int current_node = 0;
int current_direction = NORTH;

//find path from current node to target node
int find_path(int current_node, int target_node){
    int path[] = {};
    int path_index = 0;
    int current_node = current_node;
    int target_node = target_node;
    while(current_node != target_node){
        for(int i = 0; i < sizeof(node_map[current_node])/sizeof(node_map[current_node][0]); i++){
            if(node_map[current_node][i][0] == target_node){
                path[path_index] = node_map[current_node][i][1];
                path_index++;
                current_node = target_node;
            }
        }
    }
    return path;
}

//test pathing from node 0 to node 2
int path[] = find_path(0,5);
for(int i = 0; i < sizeof(path)/sizeof(path[0]); i++){
    printf("Path: %d\n", path[i]);
}
