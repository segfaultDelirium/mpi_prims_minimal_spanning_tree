CC      = mpicc

EXEC    = prims
main: prims.c
	$(CC) -o $(EXEC) $(EXEC).c
clean:
	/bin/rm -f $(EXEC) $(EXEC)*.o $(EXEC)*.s
