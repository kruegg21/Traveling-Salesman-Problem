#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_VERTICES 64
#define MAX_EDGES ((MAX_VERTICES * (MAX_VERTICES - 1)) / 2)

/* GLOBAL VARIABLE */
int best_path;
int best_path_locations[2][128];

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
int parse_line_list(
	char *line_list[], 
	int ctr, char *vertex_list[], 
	int matrix[MAX_VERTICES][MAX_VERTICES]);	
int add_to_vertex_list(char *token, int *ptr_num_vertices, char *vertex_list[]);
void add_to_matrix(
	int vertex1, 
	int vertex2, 
	int distance, 
	int matrix[MAX_VERTICES][MAX_VERTICES]);
int find_lower_bound(int matrix[MAX_VERTICES][MAX_VERTICES], int num_vertices, Path *node);
void check_implied(Path *node, int num_vertices);
int check_finished(Path *node, int num_vertices);
int next_condition(Path *node, int num_vertices, int next[2]);
int build_child(Path *parent, 
				Path *child,
				int left, 
				int num_vertices, 
				int matrix[MAX_VERTICES][MAX_VERTICES]
);
void print_tree(Path *root);
void branch_and_bound(Path *root, int num_vertices, int matrix[MAX_VERTICES][MAX_VERTICES]);
void add_to_best_path_locations(Path *node, int num_vertices);
int check_cycle(Path *node, int condition[2], int num_vertices);
 
int main(void) {

	/* reads input from standard in and stores each line in line_list
	   ctr -> number of lines read from stdin
	*/ 
	char *line_list[MAX_EDGES];
	int ctr = 0;
	while (!feof(stdin)) {
		char *buffer = malloc(sizeof(char) * 64);
		fgets(buffer, 1024, stdin);
		line_list[ctr] = buffer;
		ctr++;
	}
	
	/* loads each line list into vertex_list and weighted w_adjacency_matrix */
	
	char *vertex_list[MAX_VERTICES] = { NULL };
	int w_adjacency_matrix[MAX_VERTICES][MAX_VERTICES];
	
	int num_vertices = parse_line_list(line_list, ctr, vertex_list, w_adjacency_matrix);
	if (num_vertices == -1) {
		return 0;
	}
	
	/* sets the distance from a vertex to itself to an arbitrarily large number*/
	for (int i = 0; i < num_vertices; i++) {
		w_adjacency_matrix[i][i] = 65536;
	}
	
	// make root of tree
	Path root;
	for (int i = 0; i < num_vertices; i++) {
		root.conditions[i][i] = -1;
	}
	root.left = NULL;
	root.right = NULL;
	
	
	
	//depth first search
	Path *parent = &root;
	int finished = 0;
	int depth_first_lower_bound = 0;
	while (finished != 1) {
		Path *left_child = malloc(sizeof(Path)); 
		finished = build_child(parent, left_child, 1, num_vertices, w_adjacency_matrix);

		int lb_left= find_lower_bound(w_adjacency_matrix, num_vertices, left_child);
		printf("Lower bound of left child: %d\n", lb_left);
		left_child->lower_bound = lb_left;
		
		if (finished == -1) {
			lb_left = 66535;
			finished = 0;
		}
		
		Path *right_child = malloc(sizeof(Path));
		finished = build_child(parent, right_child, 0, num_vertices, w_adjacency_matrix);

		left_child->lower_bound = lb_left;
		if (check_finished(left_child, num_vertices) == 1 || 
			check_finished(right_child, num_vertices) == 1) {
			finished = 1;
		}
		
		int lb_right = find_lower_bound(w_adjacency_matrix, num_vertices, right_child);
		printf("Lower bound of right child: %d\n", lb_right);
		right_child->lower_bound = lb_right;
		
		if (finished == -1) {
			lb_right = 66535;
			finished = 0;
		}
		
		if (lb_right < lb_left) {
			parent = right_child;
			add_to_best_path_locations(right_child, num_vertices);
			depth_first_lower_bound = lb_right;
		}	
		else {
			parent = left_child;
			add_to_best_path_locations(left_child, num_vertices);
			depth_first_lower_bound = lb_left;
		}	
	}
	printf("FINAL LOWER BOUND: %d\n", depth_first_lower_bound);

	
		
	// print tree
	Path *node = &root;
	print_tree(node);
	
	// run through tree and branch and bound
	
	best_path = depth_first_lower_bound;
	branch_and_bound(node, num_vertices, w_adjacency_matrix);
	printf("BEST PATH: ");
	
	// final output (0 2) (0 3) (1 2) (1 4) (3 4) 
	
	// sorts best_path_locations
	int counter = 0;
	printf("%s ", vertex_list[best_path_locations[0][0]]);
	printf("%s ", vertex_list[best_path_locations[1][0]]);
	int match = best_path_locations[1][0];
	for (int i = 0; i < num_vertices; i++) {
		for (int j = 2; j < num_vertices; j++) {
			if (best_path_locations[0][j] == match) {
				match = best_path_locations[1][j];
				printf("%s ", vertex_list[best_path_locations[1][j]]);
				best_path_locations[1][j] = -1;
				j = num_vertices;
			}
			if (best_path_locations[1][j] == match) {
				match = best_path_locations[0][j];
				printf("%s ", vertex_list[best_path_locations[0][j]]);
				best_path_locations[0][j] = -1;
				j = num_vertices;
			}
		}
	}
	printf("%s ", vertex_list[best_path_locations[0][0]]);
	printf("%d\n", best_path);
	
	// TEST CODE
	

	
	/*
	Path node;
	for (int i = 0; i < num_vertices; i++) {
		node.conditions[i][i] = -1;
	}
	node.conditions[0][1] = 1;
	node.conditions[1][0] = 1;
	node.conditions[2][0] = 1;
	node.conditions[0][2] = 1;
	node.conditions[1][2] = 1;
	node.conditions[2][1] = 1;

	
	// print condition matrix
	for (int i = 0; i < num_vertices; i++) {
		for (int j = 0; j < num_vertices; j++) {
			printf("%d ", node.conditions[i][j]);
		}
		printf("\n");
	}
	printf("\n");

	// find next condition and check implied
	int next[2] = {0,0};
	int finished = next_condition(&node, num_vertices, next);
	printf("%d (%d %d)\n", finished, next[0], next[1]);
	check_implied(&node, num_vertices);
	int lower_bound = find_lower_bound(w_adjacency_matrix, num_vertices, &node);
	printf("Lower bound: %d", lower_bound);
	
	// print condition matrix
	for (int i = 0; i < num_vertices; i++) {
		for (int j = 0; j < num_vertices; j++) {
			printf("%d ", node.conditions[i][j]);
		}
		printf("\n");
	}
	*/	
}

/* branch and bound baby

*/

void branch_and_bound(Path *root, int num_vertices, int matrix[MAX_VERTICES][MAX_VERTICES]) {
	printf("On node: (%d %d)\n", root->current_condition[0], root->current_condition[1]);
	// condition if we have reached a node with no children
	if (root->left == NULL && root->right == NULL) {
	
		/* check if we should build children nodes, if not, check to see if we beat
		current best and return current best
		*/
		int finished = check_finished(root, num_vertices);
		// if finished check if we beat current best and return
		if (finished == 1) {
			printf("Node is finished\n");
			if (root->lower_bound < best_path) {
				best_path = root->lower_bound;
				add_to_best_path_locations(root, num_vertices);
				printf("Updated best path with %d\n", root->lower_bound);
			}
			printf("\n");
			return;
		}
		// else build nodes if needed
		else {
			Path *left_child = malloc(sizeof(Path));
			int left_unbuildable = build_child(root, left_child, 1, num_vertices, matrix);
			int lb_left = find_lower_bound(matrix, num_vertices, left_child);
			left_child->lower_bound = lb_left;
			
			Path *right_child = malloc(sizeof(Path));
			int right_unbuildable = build_child(root, right_child, 0, num_vertices, matrix);
			int lb_right = find_lower_bound(matrix, num_vertices, right_child);
			right_child->lower_bound = lb_right;
			
			// branch and bound both nodes if they have been built
			if (!left_unbuildable) {
				if (lb_left < best_path) {
					printf("Built left node (%d %d) with LB %d\n", left_child->current_condition[0], left_child->current_condition[1], left_child->lower_bound);
					branch_and_bound(left_child, num_vertices, matrix);	
				}
				else {
					printf("Pruned\n\n");
				}
			}			
			
			if (!right_unbuildable) {
				if (lb_right < best_path) {
					printf("Built right node (%d %d) with LB %d\n", right_child->current_condition[0], right_child->current_condition[1], right_child->lower_bound);
					branch_and_bound(right_child, num_vertices, matrix);
				}
				else {
					printf("Pruned\n\n");
				}
			}
		}
		return;
	}
	
	// condition if we meet a node with only a right child
	if (root->left == NULL) {
		printf("made it here");
		if (root->right->lower_bound < best_path) {
			branch_and_bound(root->right, num_vertices, matrix);			
		}	
		else {
			printf("Pruned!\n\n");
		}
		
		// build node if possible
		Path *left_child = malloc(sizeof(Path));
		int left_unbuildable = build_child(root, left_child, 1, num_vertices, matrix);
		int lb_left = find_lower_bound(matrix, num_vertices, left_child);
		left_child->lower_bound = lb_left;
		
		if (!left_unbuildable) {
			printf("Built left node (%d %d) with LB %d\n", left_child->current_condition[0], left_child->current_condition[1], left_child->lower_bound);
			branch_and_bound(left_child, num_vertices, matrix);
		}
		return;
	}
	
	// condition if we meet a node with only a left child
	if (root->right == NULL) {
		if (root->left->lower_bound < best_path) {
			branch_and_bound(root->left, num_vertices, matrix);
		}
		else {
			printf("Pruned!\n\n");
		}
		
		// build node if possible
		Path *right_child = malloc(sizeof(Path));
		int right_unbuildable = build_child(root, right_child, 0, num_vertices, matrix);
		int lb_right = find_lower_bound(matrix, num_vertices, right_child);
		right_child->lower_bound = lb_right;
		
		if (!right_unbuildable) {
			printf("Built right node (%d %d) with LB %d\n", right_child->current_condition[0], right_child->current_condition[1], right_child->lower_bound);
			branch_and_bound(right_child, num_vertices, matrix);
		}
		return;
	}
	
	// condition if we meet a node with two children
	if (root->left != NULL && root->right != NULL) {
		if (root->left->lower_bound < best_path) {
			branch_and_bound(root->left, num_vertices, matrix);
		}
		
		if (root->right->lower_bound < best_path) {
			branch_and_bound(root->right, num_vertices, matrix);
		}
		return;
	}
}

/* Add to best_path_locations

*/
void add_to_best_path_locations(Path *node, int num_vertices) {
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

/* recursively prints all nodes in a tree

*/

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
   vertex distances
*/ 

int parse_line_list(
	char *line_list[], 
	int ctr, char *vertex_list[], 
	int matrix[MAX_VERTICES][MAX_VERTICES]
){
	/* num_vertices -> total number of vertices from input
	   ptr_num_vertices -> pointer to num_vertices to allow for editing inside function
	   vertex1 -> holds the index in vertex_list of first vertex read from a line
	   vertex2 -> holds the index in vertex_list of second vertex read from a line
	   distance -> holds the distance between vertex1 and vertex2
	*/
	
	int num_vertices = 0;
	int *ptr_num_vertices = &num_vertices;
	int vertex1;
	int vertex2;
	int distance; 
	
	for (int i = 0; i < ctr; i++) {
		if (line_list[i][0] != '#') {
			char *token;
			// ERROR CHECK
			token = strtok(line_list[i], " ");
			if (token == NULL) {
				printf("Error: incomplete line\n");
				return -1;
			}
			vertex1 = add_to_vertex_list(token, ptr_num_vertices, vertex_list);
			token = strtok(NULL, " ");
			if (token == NULL) {
				printf("Error: incomplete line\n");
				return -1;
			}
			vertex2 = add_to_vertex_list(token, ptr_num_vertices, vertex_list);	
			if (token == NULL) {
				printf("Error: incomplete line\n");
				return -1;
			}		
			token = strtok(NULL, " ");
			distance = atoi(token);
			if (distance < 1) {
				printf("Error: negative or zero distance\n");
				return -1;
			}
			add_to_matrix(vertex1, vertex2, distance, matrix);
		}
	}
	return num_vertices;
}

/* add token to vertex list and increment num_locations, returns the token's 
   location in list 
*/

int add_to_vertex_list(char *token, int *ptr_num_vertices, char *vertex_list[]) {

	int count = 0;
		
	while (1) {
		if (vertex_list[count] == NULL) {
			vertex_list[count] = token;
			*ptr_num_vertices = *ptr_num_vertices + 1;
			return count;
		}
		else {
			if (strcmp(vertex_list[count], token) == 0) {
				return count;
			}
			count++;
		}
	}
}

void add_to_matrix(int v1, int v2, int distance, int matrix[MAX_VERTICES][MAX_VERTICES]) {
	matrix[v1][v2] = distance;
	matrix[v2][v1] = distance;	
}

/* takes a Path pointer, adjacency_matrix, and sorted matrix and returns the sum of the matrix 
	indices in the path and the smallest edges to fill out the rest of the lower bound
	path = [1,2,1,3,3,2,0,1,-1]
	sums matrix indices (1,2),(1,3),(3,2),(0,1)
*/

int find_lower_bound(
	int matrix[MAX_VERTICES][MAX_VERTICES], 
	int num_vertices,
	Path *node
) {
	int lower_bound = 0;
	for (int i = 0; i < num_vertices; i++) {
		int count = 0;
		int smallest = 65536;
		int second_smallest = 65536;
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

void check_implied(Path *node, int num_vertices) {
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
		check_implied(node, num_vertices);
	}	
}

int check_finished(Path *node, int num_vertices) {
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

int next_condition(Path *node, int num_vertices, int next[2]) {
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

int build_child(Path *parent, 
				Path *child,
				int left,
				int num_vertices, 
				int matrix[MAX_VERTICES][MAX_VERTICES]
) {	
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
	int finished = next_condition(child, num_vertices, condition);
	if (finished == 1) {
		return 1;
	}
	child->current_condition[0] = condition[0];
	child->current_condition[1] = condition[1];
	
	// add condition to child's conditions matrix and check if this makes a cycle
	if (left) {
		int cycle = check_cycle(child, condition, num_vertices);
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
	check_implied(child, num_vertices);
	
	return 0;
}	

int check_cycle(Path *node, int condition[2], int num_vertices) {
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