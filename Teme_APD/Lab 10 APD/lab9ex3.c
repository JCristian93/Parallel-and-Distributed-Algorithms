#include"mpi.h"
#include<stdio.h>
#include<stdlib.h>

merge(int A[], int Left, int Right, int End, int B[])
{
	int iL = Left;
	int iR = Right;
	int j;

	for (j = Left; j < End; j++)
	{
		if (iL < Right && (iR >= End || A[iL] <= A[iR])) {
			B[j] = A[iL];
			iL++;
		} else {
			B[j] = A[iR];
			iR++;
		}
	}
}

int main (int argc , char *argv[]) {
	int numtasks , rank , source = 0, dest , tag = 1 ;
	MPI_Status stat;
		
	MPI_Init(&argc , &argv);
	MPI_Comm_rank(MPI_COMM_WORLD, & rank);
	MPI_Comm_size(MPI_COMM_WORLD, &numtasks);

	const int recvsize = 10;
	int * sendbuf, recvbuf[recvsize];
	int sendsize = numtasks * recvsize ; 
	sendbuf = (int *)malloc (sendsize * sizeof (int));
	int i , j;	
	if (rank == 0) {

		for ( i = 0 ; i < sendsize ; i++) {
			if (i % 2 == 0) sendbuf[i] = i * 2;
			else sendbuf[i] = i * 3;
		}

	}
	MPI_Scatter (sendbuf , recvsize , MPI_INT, recvbuf , recvsize , MPI_INT , 0 , MPI_COMM_WORLD);
	
	if (rank != 0) {
		
		int ok = 1 , aux;
		while (ok == 1) {
			ok = 0;
			for (i = 0 ; i < recvsize - 1 ; i++) {
				if (recvbuf[i] < recvbuf[i+1]) {
					aux = recvbuf[i];
					recvbuf[i] = recvbuf[i+1];
					recvbuf[i+1] = aux;
					ok = 1;
				}
			}
		}
	}
	MPI_Gather (recvbuf , recvsize , MPI_INT , sendbuf , recvsize , MPI_INT , 0 , MPI_COMM_WORLD);
	
	if (rank == 0) {
		for (j = 0 ; j < recvsize ; j++) {
			printf ("%d " , recvbuf[j]);
		}
		printf ("\n");
	}
	
	MPI_Finalize();
}
