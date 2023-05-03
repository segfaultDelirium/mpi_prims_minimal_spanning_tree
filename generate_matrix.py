# import random
#
# num_vertices = 5
# max_weight = 10
#
# with open("Matrix.txt", "w") as f:
#     f.write(str(num_vertices) + "\n")
#     matrix = [[0] * num_vertices for _ in range(num_vertices)]
#     for i in range(num_vertices):
#         for j in range(i, num_vertices):
#             if i == j:
#                 matrix[i][j] = 0
#             else:
#                 weight = random.randint(1, max_weight)
#                 matrix[i][j] = weight
#                 matrix[j][i] = weight
#     for i in range(num_vertices):
#         row = " ".join(str(x) for x in matrix[i])
#         f.write(row + "\n")

# import random
#
# num_vertices = 5
# max_weight = 10
# sparsity = 0.3
#
# # Generate a fully connected graph
# edges = []
# for i in range(num_vertices):
#     for j in range(i+1, num_vertices):
#         weight = random.randint(1, max_weight)
#         edges.append((i, j, weight))
#         
# # Sort the edges by weight
# edges.sort(key=lambda e: e[2])
#
# # Initialize the adjacency matrix with all -1's
# matrix = [[-1] * num_vertices for _ in range(num_vertices)]
#
# # Add edges that don't create cycles to the adjacency matrix
# parent = list(range(num_vertices))
# rank = [0] * num_vertices
# def find(x):
#     if parent[x] != x:
#         parent[x] = find(parent[x])
#     return parent[x]
# def union(x, y):
#     xroot = find(x)
#     yroot = find(y)
#     if xroot == yroot:
#         return False
#     if rank[xroot] < rank[yroot]:
#         parent[xroot] = yroot
#     elif rank[xroot] > rank[yroot]:
#         parent[yroot] = xroot
#     else:
#         parent[yroot] = xroot
#         rank[xroot] += 1
#     return True
# for i, j, weight in edges:
#     if union(i, j):
#         matrix[i][j] = weight
#         matrix[j][i] = weight
#
# # Set some entries to -1 to create a sparse graph
# for i in range(num_vertices):
#     for j in range(i+1, num_vertices):
#         if matrix[i][j] == -1 and random.random() < sparsity:
#             matrix[i][j] = -1
#             matrix[j][i] = -1
#
# # Write the matrix to a file
# with open("Matrix.txt", "w") as f:
#     f.write(str(num_vertices) + "\n")
#     for i in range(num_vertices):
#         row = " ".join(str(x) if x != -1 else "-1" for x in matrix[i])
#         f.write(row + "\n")

# import random
#
# def generate_input_file(num_vertices):
#     # Generate a fully connected graph
#     graph = [[random.randint(1, 10) for _ in range(num_vertices)] for _ in range(num_vertices)]
#     
#     # Randomly disconnect edges until the graph becomes connected
#     while not is_connected(graph):
#         i, j = random.sample(range(num_vertices), 2)
#         if graph[i][j] != -1:
#             graph[i][j] = -1
#             graph[j][i] = -1
#     
#     # Write the graph to file
#     with open(f'input_{num_vertices}.txt', 'w') as f:
#         f.write(f'{num_vertices}\n')
#         for row in graph:
#             f.write(' '.join(str(x) for x in row) + '\n')
#
# def is_connected(graph):
#     num_vertices = len(graph)
#     visited = [False] * num_vertices
#     
#     def dfs(node):
#         visited[node] = True
#         for neighbor, weight in enumerate(graph[node]):
#             if neighbor != node and weight != -1 and not visited[neighbor]:
#                 dfs(neighbor)
#     

# import random
# import networkx as nx
# import numpy as np
#
# # Generate a random connected graph with weighted nodes
# n = 10 # number of nodes
# p = 0.3 # probability of edge creation
# weights = {i: random.randint(10, 50) for i in range(n)} # dictionary of node weights
# while True:
#     G = nx.fast_gnp_random_graph(n, p)
#     if nx.is_connected(G):
#         break
# nx.set_node_attributes(G, weights, "weight") # set the "weight" attribute for each node
#
# # Generate the adjacency matrix with weights of the connections
# adj_matrix = np.full((n, n), -1) # initialize the matrix with -1
# for i, j in G.edges():
#     weight = (G.nodes[i]['weight'] + G.nodes[j]['weight']) / 2 # average weight of nodes i and j
#     adj_matrix[i][j] = weight
#     adj_matrix[j][i] = weight
#
# # Print the adjacency matrix
# print(adj_matrix)#     dfs(0)

import random
import networkx as nx
import numpy as np

# Generate a random connected graph with weighted nodes
n = 5 # number of nodes
p = 0.8 # probability of edge creation
while True:
    G = nx.fast_gnp_random_graph(n, p)
    if nx.is_connected(G):
        break

# Set the weights for the edges
weights = {}
for edge in G.edges():
    weight = random.randint(10, 50)
    weights[edge] = weight
nx.set_edge_attributes(G, weights, "weight")

# Generate the adjacency matrix with weights of the connections
adj_matrix = np.full((n, n), -1) # initialize the matrix with -1
for i, j in G.edges():
    weight = G.edges[(i, j)]['weight']
    adj_matrix[i][j] = weight
    adj_matrix[j][i] = weight

# Print the adjacency matrix
print(adj_matrix)

# Write the adjacency matrix to file
with open("Matrix.txt", "w") as f:
    f.write(str(n) + "\n") # write the number of nodes on the first line
    for row in adj_matrix:
        f.write(" ".join([str(x) for x in row]) + "\n") # write each row of the matrix
