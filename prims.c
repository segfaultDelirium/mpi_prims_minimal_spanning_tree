#include <limits.h>
#include <math.h>
#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#define MY_INFINITY 10000

// valgrind detects non-freed blocks even though everything is freed.
// This happens also when program is almost empty which led me to conclusion that mpi library leaks memory.

struct EdgeAndNodes{
  int edge_weight;
  int start_node_id;
  int end_node_id;
};

int read_adjacency_matrix_from_input_file(char *filename, int **adjacency_matrix){
  FILE *fp;
  fp = fopen(filename, "r");
  if (fp == NULL) {
      printf("File not found!\n");
      exit(1);
  }
  int global_row_count = 0;
  fscanf(fp, "%d", &global_row_count);
  printf("Nodes count read from file: %d\n", global_row_count);
  *adjacency_matrix = (int *) malloc(global_row_count * global_row_count * sizeof(int)); 

  for (int i = 0; i < global_row_count; i++) {
      for (int j = 0; j < global_row_count; j++) {
          int * pointer_to_cell = &((*adjacency_matrix)[i * global_row_count + j]);
          fscanf(fp, "%d", pointer_to_cell);
          if(*pointer_to_cell == -1) *pointer_to_cell = MY_INFINITY;
      }
  }
  fclose(fp);
  return global_row_count;
}

void print_adjacency_matrix(int *adjacency_matrix, int n){
  if(adjacency_matrix == NULL) {
    printf("adjacency matrix pointer is NULL\n");
    return;
  }
  for(int i = 0; i < n; i++){
    for(int j = 0; j < n; j++){
      printf("%d ", adjacency_matrix[i * n + j]);
    }
    printf("\n");
  }
}

void print_array(int *array, int array_length, char *prefix){
  char result_string[1000] = "";
  char temp_buffer[100] = "";
  strcat(result_string, prefix);
  for(int i = 0; i < array_length; i++){
    snprintf(temp_buffer, 100, "%d ", array[i]);
    strcat(result_string, temp_buffer);
  }
  printf("%s\n", result_string);
}

void print_array_bools(bool *array, int array_length, char *prefix){
  char result_string[1000] = "";
  char temp_buffer[100] = "";
  strcat(result_string, prefix);
  for(int i = 0; i < array_length; i++){
    snprintf(temp_buffer, 100, "%d ", array[i]);
    strcat(result_string, temp_buffer);
  }
  printf("%s\n", result_string);
}

// this is due to possibility of global_row_count being not divisable by number of proceses
int get_local_row_count(int global_row_count, int process_count, int process_rank){
  int number_of_local_rows = global_row_count / process_count;
  int number_of_additional_rows = global_row_count % process_count;
  if (process_rank < number_of_additional_rows) return number_of_local_rows + 1;
  return number_of_local_rows;
}

// this is due to possibility of global_row_count being not divisable by number of proceses
void calculate_send_counts_and_send_displacements(int global_row_count, int process_count, int *send_counts, int * send_displacements){
  int number_of_local_rows = global_row_count / process_count;
  int number_of_additional_rows = global_row_count % process_count;
  for (int i = 0; i < process_count; i++) {
    send_counts[i] = i < number_of_additional_rows ? (number_of_local_rows + 1) * global_row_count : number_of_local_rows * global_row_count;
    send_displacements[i] = i * number_of_local_rows * global_row_count + (i < number_of_additional_rows ? i : number_of_additional_rows) * global_row_count;
  }
}

int main(int argc, char *argv[]) {

  int rank, process_count;

  MPI_Status status;
  MPI_Init(&argc, &argv);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Comm_size(MPI_COMM_WORLD, &process_count);

  char process_name[MPI_MAX_PROCESSOR_NAME]; // MPI_MAX_PROCESSOR_NAME = 128
  int name_len;
  MPI_Get_processor_name(process_name, &name_len);
  if(name_len >= MPI_MAX_PROCESSOR_NAME){
    printf("Process name has overflown the reserved space for process name string. Reserved space: %d, process name length: %d\n", MPI_MAX_PROCESSOR_NAME, name_len);
  }
  printf("hello from process %s, rank %d out of %d processes\n", process_name, rank, process_count);
  MPI_Barrier(MPI_COMM_WORLD);
  
  // this is three int continuous array, 0-element is edge weight, 1-element is start_node id, 2-element is end_node-id 
  MPI_Datatype EdgeAndNodes_MPI;
  MPI_Type_contiguous(3, MPI_INT, &EdgeAndNodes_MPI);
  MPI_Type_commit(&EdgeAndNodes_MPI);
  
  int *full_adjacency_matrix = NULL;
  
  int global_row_count = 0;
  if(rank == 0){
    printf("process 0 reads the adjacency_matrix_from_file\n");
    
    global_row_count = read_adjacency_matrix_from_input_file("Matrix.txt", &full_adjacency_matrix);

    print_adjacency_matrix(full_adjacency_matrix, global_row_count);
  }
  // printf("before synchronizing global_row_count\n");
  MPI_Bcast(&global_row_count, 1, MPI_INT, 0, MPI_COMM_WORLD);
  // printf("before barrier synchronizing global_row_count\n");
  MPI_Barrier(MPI_COMM_WORLD);
  // printf("after barrier synchronizing global_row_count\n");
  
  int local_row_count = get_local_row_count(global_row_count, process_count, rank);
  printf("local_row_count array in process %d: %d\n", rank, local_row_count);

  // Allocate memory for the local matrix
  int * local_matrix = (int*) calloc(local_row_count * global_row_count, sizeof(int));
  int * send_counts = (int *)calloc(process_count, sizeof(int));
  int * send_displacements = (int *)calloc(process_count, sizeof(int));
  calculate_send_counts_and_send_displacements(global_row_count, process_count, send_counts, send_displacements);

  if(rank == 0){

    char prefix[100] = "";
    snprintf(prefix, 100, "send_counts array in process %d: ", rank);
    print_array(send_counts, process_count, prefix);
    snprintf(prefix, 100, "send_displacements array in process %d: ", rank);
    print_array(send_displacements, process_count, prefix);

  }
  MPI_Barrier(MPI_COMM_WORLD);
  
  MPI_Scatterv(full_adjacency_matrix, send_counts, send_displacements, MPI_INT, local_matrix, local_row_count * global_row_count, MPI_INT, 0, MPI_COMM_WORLD);
  // now each process has its own rows to analyze
  
  MPI_Barrier(MPI_COMM_WORLD);
  {
    char prefix[100] = "";
    snprintf(prefix, 100, "printing_local_matrix in process %d: ", rank);
    print_array(local_matrix, local_row_count * global_row_count, prefix);
  }

  MPI_Barrier(MPI_COMM_WORLD);

  // we need to store information about which graph node is already included in MST.
  // list of 1 and 0s. 0 means not included, 1 means included.
  bool *are_nodes_included_in_mst = calloc(global_row_count, sizeof(bool));

  // this struct will be used by process with rank 0 to keep history about edges and nodes that were connected. 
  // This will be later used for printing the result.
  // minimal spanning tree has n -1 edges where n = amount of graph nodes
  struct EdgeAndNodes *edge_and_nodes = NULL;
  if(rank == 0){
    edge_and_nodes = malloc( (global_row_count -1) * sizeof(struct EdgeAndNodes));
  }
  // choose first node as starting node 
  are_nodes_included_in_mst[0] = true;

  // each iteration of this loop a new edge is found
  for(int k = 0; k < global_row_count -1; k++){

    // {
    //   char prefix[100] = "";
    //   snprintf(prefix, 100, "for rank: %d, in iteration k = %d, are_nodes_included_in_mst array like like:  ", rank, k);
    //   print_array_bools(are_nodes_included_in_mst, global_row_count, prefix);
    // }
    int local_minimal_edge_weight = MY_INFINITY;
    // nodes connected by the edge with local_minimal_edge_weight
    int global_id_of_start_node = 0;
    int global_id_of_end_node = 0;

    for(int i = 0; i < local_row_count; i++){ // for every local row

      int global_row_id = i + send_displacements[rank] / global_row_count;
      // printf("rank: %d. global row id currently analyzed: %d. i = %d, k = %d\n", rank, global_row_id, i, k);
      bool is_node_included_in_mst = are_nodes_included_in_mst[global_row_id];
      // printf("rank: %d. is start node included in mst: %d\n", rank, is_node_included_in_mst);
      if(is_node_included_in_mst == false) continue;

      // we check each edge to find minimal weight
      for(int j = 0; j < global_row_count; j++){ // for each column
        if(i == j) continue; 
        
        // we ignore the edges to nodes which are already included in minimal spanning tree
        bool is_compared_node_included_in_mst = are_nodes_included_in_mst[j];
        // printf("rank: %d. is compared node included in mst: %d\n", rank, is_compared_node_included_in_mst);
        if(is_compared_node_included_in_mst) continue;


        int current_edge_weight = local_matrix[i * global_row_count + j];
        // printf("rank: %d, i: %d, j: %d, k: %d . current edge weight = %d, compared to local_minimum_edge_weight = %d \n", rank, i, j, k, current_edge_weight, local_minimal_edge_weight);
        // {
        //   char prefix[100] = "";
        //   snprintf(prefix, 100, "printing_local_matrix in process %d, currently, local_minimal_edge_weight is %d\t", rank, local_minimal_edge_weight);
        //   print_array(local_matrix, local_row_count * global_row_count, prefix);
        // }
        if(current_edge_weight < local_minimal_edge_weight){
          // printf("rank: %d. found new best weight %d. i = %d, j = %d, k = %d\n", rank, current_edge_weight, i, j, k);
          local_minimal_edge_weight = current_edge_weight;
          global_id_of_start_node = global_row_id;
          global_id_of_end_node = j;
        }
      }
    }

    MPI_Barrier(MPI_COMM_WORLD);
    // printf("process %d has found minimal weight %d of edge between nodes: %d and %d \n", rank, local_minimal_edge_weight, global_id_of_start_node, global_id_of_end_node);

    // processes send their solution to process 0 who decides which weight and edge to add to minimal spanning tree.
    if(rank != 0){
      int edge_and_nodes_to_send[3] = {local_minimal_edge_weight, global_id_of_start_node, global_id_of_end_node};
      MPI_Send(edge_and_nodes_to_send, 3, MPI_INT, 0, 0, MPI_COMM_WORLD);
    }
    if(rank == 0){
      int globally_minimal_weight = local_minimal_edge_weight;
      int start_node = global_id_of_start_node;
      int end_node = global_id_of_end_node;

      int received_counter = 0;
      while(received_counter < process_count -1){
        int received_edge_and_nodes[3] = {0, 0, 0};
        
        MPI_Status mpi_status;
        MPI_Recv(received_edge_and_nodes, 3, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &mpi_status);
        if(received_edge_and_nodes[0] < globally_minimal_weight){
          globally_minimal_weight = received_edge_and_nodes[0];
          start_node = received_edge_and_nodes[1];
          end_node = received_edge_and_nodes[2];
        }
        received_counter += 1;
      }
      
      printf("globally minimal edge weight: %d of edge connecting nodes: %d and %d\n", globally_minimal_weight, start_node, end_node);
      are_nodes_included_in_mst[end_node] = true;
      edge_and_nodes[k].edge_weight = globally_minimal_weight;
      edge_and_nodes[k].start_node_id = start_node;
      edge_and_nodes[k].end_node_id = end_node;

    }
    MPI_Barrier(MPI_COMM_WORLD);

    // synchronization of mst
    MPI_Bcast(are_nodes_included_in_mst, global_row_count, MPI_C_BOOL, 0, MPI_COMM_WORLD);

    MPI_Barrier(MPI_COMM_WORLD);

  } 



  
  
  

  MPI_Barrier(MPI_COMM_WORLD);
  if(rank == 0){
    
    int total_weight = 0;
    for(int i = 0; i < global_row_count -1; i++){
      printf("edge: %d  -  %d with weight: %d\n", edge_and_nodes[i].start_node_id, edge_and_nodes[i].end_node_id, edge_and_nodes[i].edge_weight);
      total_weight += edge_and_nodes[i].edge_weight;
    }
    printf("cumulative weight of the minimal spanning tree: %d\n", total_weight);

    printf("\n");
  }
  MPI_Barrier(MPI_COMM_WORLD);

  printf("cleaning up allocated memory\n");
  free(are_nodes_included_in_mst);
  free(send_counts);
  free(send_displacements);
  free(local_matrix);
  if(rank == 0){
    free(edge_and_nodes);
    free(full_adjacency_matrix);
  }
  printf("finished cleaning up allocated memory\n");

  MPI_Type_free(&EdgeAndNodes_MPI);

  // printf("before MPI_Finalize\n");
  MPI_Finalize();
  // printf("after MPI_Finalize\n");

  return 0;
}
