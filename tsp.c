#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#define MAX_VERTICES 64
#define MAX_EDGES (MAX_VERTICES * MAX_VERTICES)
#define MAX_DISTANCE 65536

/* GLOBAL VARIABLES
 * best_path -> stores the lenght of the current best path found
 * best_path_locations -> stores the order of edges for the current
 * 		best path found
 * num_vertices -> number of vertices
 */

int best_path;
int best_path_locations[2][MAX_VERTICES];
int num_vertices;

/* PATH 
 * lower_bound -> the lower bound path length for vertex
 * path *left -> pointer to left child vertex 
 * path *right -> pointer to right child vertex
 */

typedef struct path {
	int current_condition[2];
	int conditions[MAX_VERTICES][MAX_VERTICES];
	int lower_bound;
	struct path *left;
	struct path *right;
} Path;

/* FUNCTION LIST */
bool parse_line_list(char **l_list, int ctr, char **v_list, int **matrix);	
int add_to_vertex_list(char *token, char **v_list);
void add_to_matrix(int vertex1, int vertex2, int distance, int **matrix);
int find_lower_bound(int **matrix, Path *node);
void check_implied(Path *node);
int check_finished(Path *node);
int next_condition(Path *node, int *next);
int build_child(Path *parent, Path *child, int left, int **matrix);
void print_tree(Path *root);
void branch_and_bound(Path *root, int **matrix);
void add_to_best_path_locations(Path *node);
int check_cycle(Path *node, int *condition);
void display_best_path(char **v_list);
void free_tree (Path *root);
Path *initialize_root();
void initial_guess(Path *root, int **adjacency_matrix);
int **build_adjacency_matrix(int size);
void free_matrix(int **matrix, int size);
 
int main(void) {

	/* reads input from stdin and stores each line in line_list
 	 * line_list -> an array of strings, where each string is a line from the
 	 		input file
	 * ctr -> number of lines read from stdin
	 */ 
	char *line_list[MAX_EDGES];
	int ctr = 0;
	while (!feof(stdin)) {
		char *buffer = malloc(sizeof(char) * MAX_VERTICES);
		fgets(buffer, 1024, stdin);
		line_list[ctr] = buffer;
		ctr++;
	}
	
	/* loads each line list into vertex list and weighted adjacency_matrix
	 * v_list -> associates each vertex name with a number 
	 * adjacency_matrix -> matrix with distances between vertices
	 */ 

	int **temp_adjacency_matrix = build_adjacency_matrix(MAX_VERTICES);
	char **temp_vertex_list = malloc(sizeof(char *)*MAX_VERTICES);
	
	/* takes the infromation from line_list and places it into vertex_list 
	 * and the adjacency_matrix. parse_line_list will return true and 
	 * print a warning sign if there is something wrong with the input
	 */
	bool error = parse_line_list(line_list, ctr, temp_vertex_list, temp_adjacency_matrix);
	if (error == true) {
		return 0;
	}

	/* Replaces vertex_list and adjacency list with more managable sized arrays */
	char **vertex_list = malloc(sizeof(char *)*num_vertices);
	int **adjacency_matrix = build_adjacency_matrix(num_vertices);

	for (int i = 0; i < num_vertices; i++) {
		vertex_list[i] = temp_vertex_list[i];
		for (int j = 0; j < num_vertices; j++) {
			adjacency_matrix[i][j] = temp_adjacency_matrix[i][j];
		}
	}

	/* free temp adjacency matrix and temp vertex list */
	free(temp_vertex_list);
	free_matrix(temp_adjacency_matrix, MAX_VERTICES);
	
	/* make root of tree */
	Path *root = initialize_root();
	
	/* depth first search to find initial best guess and assign that initial
	 * guess to global variable best_path 
	 */
	initial_guess(root, adjacency_matrix);

	/* run through tree and branch and bound */
	branch_and_bound(root, adjacency_matrix);
	
	/* show final output */
	display_best_path(vertex_list);	

	/* free tree, adjacency matrix, and vertex list */
	free(vertex_list);
	free_matrix(adjacency_matrix, num_vertices);
	free_tree(root);
	return 0;
}

void free_matrix(int **matrix, int size) {
	for (int i = 0; i < size; i++) {
		free(matrix[i]);
	}
	free(matrix);
}

int **build_adjacency_matrix(int size) {
	int **adjacency_matrix = malloc(sizeof(int *)*MAX_VERTICES);
	for (int i = 0; i < MAX_VERTICES; i++) {
		adjacency_matrix[i] = malloc(sizeof(int)*MAX_VERTICES);
		memset(adjacency_matrix[i], MAX_DISTANCE, sizeof(int)*MAX_VERTICES);
	}
	return adjacency_matrix;
}

Path *initialize_root() {
	Path *root = malloc(sizeof(Path));
	for (int i = 0; i < num_vertices; i++) {
		root->conditions[i][i] = -1;
	}
	return root;
}

/* frees tree */
void free_tree (Path *root) {
	Path *node = root;
	if (node->right != NULL) {
		free_tree(node->right);
	}
	if (node->left != NULL) {
		free_tree(node->left);
	}
	free(node);
}

void initial_guess(Path *root, int **adjacency_matrix) {
	Path *parent = root;
	int finished = 0;
	int depth_first_lower_bound = 0;
	int left = 0;
	while (finished != 1) {
		Path *left_child = malloc(sizeof(Path)); 
		left = 1;
		finished = build_child(parent, left_child, left, adjacency_matrix);

		int lb_left= find_lower_bound(adjacency_matrix, left_child);
		left_child->lower_bound = lb_left;
		
		if (finished == -1) {
			lb_left = MAX_DISTANCE;
			finished = 0;
		}
		
		Path *right_child = malloc(sizeof(Path));
		left = 0;
		finished = build_child(parent, right_child, left, adjacency_matrix);

		left_child->lower_bound = lb_left;
		if (check_finished(left_child) == 1 || 
			check_finished(right_child) == 1) {
			finished = 1;
		}
		
		int lb_right = find_lower_bound(adjacency_matrix, right_child);
		right_child->lower_bound = lb_right;
		
		if (finished == -1) {
			lb_right = MAX_DISTANCE;
			finished = 0;
		}
		
		if (lb_right < lb_left) {
			parent = right_child;
			add_to_best_path_locations(right_child);
			depth_first_lower_bound = lb_right;
		}	
		else {
			parent = left_child;
			add_to_best_path_locations(left_child);
			depth_first_lower_bound = lb_left;
		}	
	}
	best_path = depth_first_lower_bound;
}


/* Prints locations in best path and the final score */
void display_best_path(char **v_list) {
	printf("BEST PATH: ");
	// sorts best_path_locations
	int counter = 0;
	printf("%s ", v_list[best_path_locations[0][0]]);
	printf("%s ", v_list[best_path_locations[1][0]]);
	int match = best_path_locations[1][0];
	for (int i = 0; i < num_vertices; i++) {
		for (int j = 2; j < num_vertices; j++) {
			if (best_path_locations[0][j] == match) {
				match = best_path_locations[1][j];
				printf("%s ", v_list[best_path_locations[1][j]]);
				best_path_locations[1][j] = -1;
				j = num_vertices;
			}
			if (best_path_locations[1][j] == match) {
				match = best_path_locations[0][j];
				printf("%s ", v_list[best_path_locations[0][j]]);
				best_path_locations[0][j] = -1;
				j = num_vertices;
			}
		}
	}
	printf("%s\n", v_list[best_path_locations[0][0]]);
	printf("SCORE: %d\n", best_path / 2);
}

/* Performs branch and bound technique */
void branch_and_bound(Path *root, int **matrix) {
	// condition if we have reached a node with no children
	if (root->left == NULL && root->right == NULL) {
	
		/* check if we should build children nodes, if not, check to see if we beat
		current best and return current best
		*/
		int finished = check_finished(root);
		// if finished check if we beat current best and return
		if (finished == 1) {
			if (root->lower_bound < best_path) {
				best_path = root->lower_bound;
				add_to_best_path_locations(root);
			}
			return;
		}
		// else build nodes if needed
		else {
			Path *left_child = malloc(sizeof(Path));
			int left_unbuildable = build_child(root, left_child, 1, matrix);
			int lb_left = find_lower_bound(matrix, left_child);
			left_child->lower_bound = lb_left;
			
			Path *right_child = malloc(sizeof(Path));
			int right_unbuildable = build_child(root, right_child, 0, matrix);
			int lb_right = find_lower_bound(matrix, right_child);
			right_child->lower_bound = lb_right;
			
			// branch and bound both nodes if they have been built
			if (!left_unbuildable) {
				if (lb_left < best_path) {
					branch_and_bound(left_child, matrix);	
				}
			}			
			
			if (!right_unbuildable) {
				if (lb_right < best_path) {
					branch_and_bound(right_child, matrix);
				}
			}
		}
		return;
	}
	
	// condition if we meet a node with only a right child
	if (root->left == NULL) {
		if (root->right->lower_bound < best_path) {
			branch_and_bound(root->right, matrix);			
		}	
		else {
		}
		
		// build node if possible
		Path *left_child = malloc(sizeof(Path));
		int left_unbuildable = build_child(root, left_child, 1, matrix);
		int lb_left = find_lower_bound(matrix, left_child);
		left_child->lower_bound = lb_left;
		
		if (!left_unbuildable) {
			branch_and_bound(left_child, matrix);
		}
		return;
	}
	
	// condition if we meet a node with only a left child
	if (root->right == NULL) {
		if (root->left->lower_bound < best_path) {
			branch_and_bound(root->left, matrix);
		}
		else {
		}
		
		// build node if possible
		Path *right_child = malloc(sizeof(Path));
		int right_unbuildable = build_child(root, right_child, 0, matrix);
		int lb_right = find_lower_bound(matrix, right_child);
		right_child->lower_bound = lb_right;
		
		if (!right_unbuildable) {
			branch_and_bound(right_child, matrix);
		}
		return;
	}
	
	// condition if we meet a node with two children
	if (root->left != NULL && root->right != NULL) {
		if (root->left->lower_bound < best_path) {
			branch_and_bound(root->left, matrix);
		}
		
		if (root->right->lower_bound < best_path) {
			branch_and_bound(root->right, matrix);
		}
		return;
	}
}

/* Adds node to best_path_locations */
void add_to_best_path_locations(Path *node) {
	int counter = 0;
	for (int i = 0; i < num_vertices; i++) {
		for (int j = i; j < num_vertices; j++) {
			if (node->conditions[i][j] == 1) {
				best_path_locations[0][counter] = i;
				best_path_locations[1][counter] = j;
				counter++;
			}
		}
	}
}

/* recursively prints all nodes in a tree */
void print_tree(Path *root) {
	printf("Node (%d %d) with LB: %d\n",
	root->current_condition[0], root->current_condition[1], 
	root->lower_bound);
	if (root->left != NULL && root->right != NULL) {
		if (root->left->lower_bound < root->right->lower_bound) {
			print_tree(root->left);
			print_tree(root->right);
		}
		else {
			print_tree(root->right);
			print_tree(root->left);
		} 
	}
	else if (root->left != NULL) {
		print_tree(root->left);
	}
	else if (root->right != NULL) {
		print_tree(root->right);
	}
	else {
		return;
	}
	
}	

/* takes line_list and parses the information into a multidimensional array of 
   vertex distances. Returns 1 if the data fed in to program is usable and 0
   if not.
*/ 

bool parse_line_list(char **line_list, int ctr, char **v_list, int **matrix) {
	/* ptr_num_vertices -> pointer to num_vertices to allow for editing inside function
	   vertex1 -> holds the index in vertex_list of first vertex read from a line
	   vertex2 -> holds the index in vertex_list of second vertex read from a line
	   distance -> holds the distance between vertex1 and vertex2
	*/
	
	bool error = false;
	int vertex1;
	int vertex2;
	int distance; 
	
	for (int i = 0; i < ctr; i++) {
		if (line_list[i][0] != '#') {
			char *token;
			// ERROR CHECK
			token = strtok(line_list[i], " ");
			vertex1 = add_to_vertex_list(token, v_list);
			token = strtok(NULL, " ");
			vertex2 = add_to_vertex_list(token, v_list);	
			token = strtok(NULL, " ");
			distance = atoi(token);
			if (distance < 1) {
				printf("Error: negative or zero distance\n");
				error = true;
				return error;
			}
			add_to_matrix(vertex1, vertex2, distance, matrix);
		}
	}
	return error;
}

/* add token to vertex list and increment num_locations, returns the token's 
   location in list 
*/

int add_to_vertex_list(char *token, char **v_list) {

	int count = 0;
		
	while (1) {
		if (v_list[count] == NULL) {
			v_list[count] = token;
			num_vertices++;
			return count;
		}
		else {
			if (strcmp(v_list[count], token) == 0) {
				return count;
			}
			count++;
		}
	}
}

void add_to_matrix(int v1, int v2, int distance, int **matrix) {
	matrix[v1][v2] = distance;
	matrix[v2][v1] = distance;	
}

/* takes a Path pointer, adjacency_matrix, and sorted matrix and returns the sum of the matrix 
	indices in the path and the smallest edges to fill out the rest of the lower bound
	path = [1,2,1,3,3,2,0,1,-1]
	sums matrix indices (1,2),(1,3),(3,2),(0,1)
*/

int find_lower_bound(int **matrix, Path *node) {
	int lower_bound = 0;
	for (int i = 0; i < num_vertices; i++) {
		int count = 0;
		int smallest = MAX_DISTANCE;
		int second_smallest = MAX_DISTANCE;
		for (int j = 0; j < num_vertices; j++) {
			if (node->conditions[i][j] == 1) {
				count++;
				lower_bound += matrix[i][j];
			}
			if (node->conditions[i][j] == 0) {
				if (matrix[i][j] < smallest) {
					second_smallest = smallest;
					smallest = matrix[i][j];
				}
				else if (matrix[i][j] < second_smallest) {
					second_smallest = matrix[i][j];
				}				
			}			 
		}
		if (count == 1) {
			lower_bound += smallest;
		}
		else if (count == 0) {
			lower_bound += smallest;
			lower_bound += second_smallest;
		}
	}
	return lower_bound;
}

void check_implied(Path *node) {
	int max_negations = num_vertices - 2;
	int propagate = 0;
	// check for new edges that can be added to path
	for (int i = 0; i < num_vertices; i++) {
		int counter = 0;
		for (int j = 0; j < num_vertices; j++) {
			if (node->conditions[i][j] == 1) {
				counter++;
			}
		}
		if (counter >= 2) {
			// negate all other options on row
			for (int j = 0; j < num_vertices; j++) {
				if (node->conditions[i][j] == 0) {
					propagate = 1;			
					node->conditions[i][j] = -1;
					node->conditions[j][i] = -1;
				}
			}
		}
	}
	
	// check for edges that can be negated
	for (int i = 0; i < num_vertices; i++) {
		int counter = 0;
		for (int j = 0; j < num_vertices; j++) {
			if (node->conditions[i][j] == -1) {
				counter++;
			}
		}
		if (counter >= max_negations) {
			// add the remaining options to path
			for (int j = 0; j < num_vertices; j++) {
				if (node->conditions[i][j] == 0) {
					propagate = 1;
					node->conditions[i][j] = 1;
					node->conditions[j][i] = 1;
				}
			}
		}
	}
	if (propagate) {
		check_implied(node);
	}	
}

int check_finished(Path *node) {
	int finished = 1;
	for (int i = 0; i < num_vertices; i++) {
		int counter = 0;
		for (int j = 0; j < num_vertices; j++) {
			if (node->conditions[i][j] == 1) {
				counter++;
			}
		}
		if (counter != 2) {
			finished = 0;
			return finished;
		}
	}
	return finished;
}

int next_condition(Path *node, int *next) {
	int finished = 0;
	for (int i = 0; i < num_vertices; i++) {
		for (int j = i; j < num_vertices; j++) {
			if (node->conditions[i][j] == 0) {
				next[0] = i;
				next[1] = j;
				return finished;
			}
		}
	}
	finished = 1;
	return finished;
}
/* builds a child for the parent, if this is not possible, returns 1. Otherwise, it will
   build a child using child node and return 0. Copies the parent's condition matrix, adds
   the next possible condition (returns 1 if there are no more possible), checks for 
   implied conditions. DOES NOT ADD LOWER BOUND. Returns -1 if a cycle is made
*/

int build_child(Path *parent, Path *child, int left, int **matrix) {	
	// set children of child to NULL
	child->left = NULL;
	child->right = NULL;

	// copy parents condition matrix
	for (int i = 0; i < num_vertices; i++) {
		for (int j = 0; j < num_vertices; j++) {
			child->conditions[i][j] = parent->conditions[i][j];
		}
	}
		
	// finds next condition to test
	int condition[2] = {0,0};
	int finished = next_condition(child, condition);
	if (finished == 1) {
		return 1;
	}
	child->current_condition[0] = condition[0];
	child->current_condition[1] = condition[1];
	
	// add condition to child's conditions matrix and check if this makes a cycle
	if (left) {
		int cycle = check_cycle(child, condition);
		if (cycle) {
			return -1;
		}
		child->conditions[condition[0]][condition[1]] = 1;
		child->conditions[condition[1]][condition[0]] = 1;
		parent->left = child;
	}
	else if (!left) {
		child->conditions[condition[0]][condition[1]] = -1;
		child->conditions[condition[1]][condition[0]] = -1;
		parent->right = child;
	}
	
	// check implied conditions
	check_implied(child);
	
	return 0;
}	

int check_cycle(Path *node, int *condition) {
	int counter = 0;
	int loop = 0;
	int loop_start = condition[0];
	int current = condition[0];
	int next = condition[1];
	for (int j = 0; j < num_vertices; j++) {
		for (int i = 0; i < num_vertices; i++) {
			if (node->conditions[next][i] == 1 && i != current) {
				if (i == loop_start) {
					loop = 1;
				}
				current = next;
				next = i;				
				counter++;
				i = num_vertices;
			}
		}
	}
	if (loop == 1 && counter < num_vertices) {
		return 1;
	}
	return 0;
}	
