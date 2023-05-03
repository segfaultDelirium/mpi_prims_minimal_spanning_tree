first install networkx for generating matrix
pip install networkx

to generate new matrix in Matrix.txt:
python ./generate_matrix.py

before you compile and run, you need to source this script which adds environment variables for MPI to work.
source /opt/nfs/config/source_mpich401.sh

then compile the prims.c:
make

then run the program:
mpiexec -f ./nodes -n 4 ./prims
with for example 4 processes 

./nodes is a file that lists available machines
to regenerate ./nodes file run:
/opt/nfs/config/station204_name_list.sh 1 16 > nodes


