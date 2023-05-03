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
