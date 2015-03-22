#include"mpi.h"
#include<stdio.h>
#include<stdlib.h>

int main (int argc , char *argv[]) {
	int numtasks , rank , source = 0, dest , tag = 1 , i;
	MPI_Status stat;
	MPI_Datatype rowtype;
	
	MPI_Init(&argc , &argv);
	MPI_Comm_rank(MPI_COMM_WORLD, & rank);
	MPI_Comm_size(MPI_COMM_WORLD, &numtasks);
	
	MPI_Type_contiguous(10, MPI_INT , &rowtype);
	MPI_Type_commit(&rowtype);
	
	if (rank == 0) {
		int vectoras[10] = {0,1,2,3,4,5,6,7,8,9};
		MPI_Send(vectoras , 1 , rowtype , 1 , tag , MPI_COMM_WORLD);
	}
	else {
		int i;
		int *receiver =  (int *) malloc (10 * sizeof(int));	
		MPI_Recv(receiver , 10 , MPI_INT , 0 , tag,  MPI_COMM_WORLD , &stat);
		for (i = 0 ; i < 10 ; i++) {
			printf ("%d ", receiver[i]);
		}
	}
	MPI_Type_free(&rowtype);
	MPI_Finalize();
}
