#include"mpi.h"
#include<stdio.h>
#include<stdlib.h>

int main (int argc , char *argv[]) {
	int numtasks , rank , source = 0, dest , tag = 1 , i;
	typedef struct {
		int vector_proc[3];
		int a;
	} Proc_struct;
	Proc_struct p[2] , part[2];
	
	MPI_Status stat;
	MPI_Datatype structura , oldtypes[2];
	int blockcounts[2];
	MPI_Aint offsets[2] , extent;
	
	MPI_Init(&argc , &argv);
	MPI_Comm_rank(MPI_COMM_WORLD, & rank);
	MPI_Comm_size(MPI_COMM_WORLD, &numtasks);

	offsets[0] = 0;
	oldtypes[0] = MPI_INT;
	blockcounts[0] = numtasks;
	MPI_Type_extent(MPI_INT, &extent);
	offsets[1] = numtasks * extent;
	oldtypes[1] = MPI_INT;
	blockcounts[1] = 1;
	
	MPI_Type_struct(2 , blockcounts , offsets , oldtypes , &structura);
	MPI_Type_commit(&structura);
	
	if (rank == 0) {
		Proc_struct send_struct ;
		//send_struct.vector_proc = (int *) calloc (numtasks , sizeof(int));
		send_struct.vector_proc[0] = 2;
		send_struct.vector_proc[1] = 0;
		send_struct.vector_proc[2] = 0;
		send_struct.a = 2;
		printf ("rank 0 trimit: %d %d %d %d\n" , send_struct.vector_proc[0] ,send_struct.vector_proc[1] ,
									 send_struct.vector_proc[2], send_struct.a );
		MPI_Send(&send_struct , 1 , structura , 1 , tag , MPI_COMM_WORLD);
		sleep (2);
		MPI_Send(&send_struct , 1 , structura , 2 , tag , MPI_COMM_WORLD);
	}
	else if (rank == 1){
		Proc_struct receiver_struct , receiver_struct1;
		receiver_struct1.vector_proc[0] = 0;
		receiver_struct1.vector_proc[1] = 0;
		receiver_struct1.vector_proc[2] = 0;
		MPI_Recv(&receiver_struct , 1 , structura , MPI_ANY_SOURCE , tag,  MPI_COMM_WORLD , &stat);
		int i , ok = 0 ;
		for (i = 0 ; i < 3 ; i++) {
			if (i != rank && receiver_struct1.vector_proc[i] > receiver_struct.vector_proc[i]){
				printf ("EROARE \n");
				ok = 1;
				break;
			}
		}	
		if (ok == 0) {
			printf ("E BUN , merg mai departe \n");
		}
		receiver_struct.vector_proc[1]++;
		printf ("rank 1 primesc: %d %d %d %d\n" , receiver_struct.vector_proc[0] ,receiver_struct.vector_proc[1] ,
									 receiver_struct.vector_proc[2], receiver_struct.a );	
		receiver_struct.vector_proc[1]++;
		receiver_struct.a ++;
		printf ("rank 1 trimit: %d %d %d %d\n" , receiver_struct.vector_proc[0] ,receiver_struct.vector_proc[1] ,
									 receiver_struct.vector_proc[2], receiver_struct.a );
		MPI_Send(&receiver_struct , 1 , structura , 2 , tag , MPI_COMM_WORLD);
	}
	else {
		Proc_struct receiver_struct1 , receiver_struct2;
		MPI_Recv(&receiver_struct1 , 1 , structura , MPI_ANY_SOURCE , tag,  MPI_COMM_WORLD , &stat);	
		printf ("rank 2 primesc: %d %d %d %d\n" , receiver_struct1.vector_proc[0] ,receiver_struct1.vector_proc[1] ,
									 receiver_struct1.vector_proc[2], receiver_struct1.a );	
		MPI_Recv(&receiver_struct2 , 1 , structura , MPI_ANY_SOURCE , tag,  MPI_COMM_WORLD , &stat);	
		printf ("rank 2 primesc: %d %d %d %d\n" , receiver_struct2.vector_proc[0] ,receiver_struct2.vector_proc[1] ,
									 receiver_struct2.vector_proc[2], receiver_struct2.a );	
		int i;
		for (i = 0 ; i < 3 ; i++) {
			if (i != rank && receiver_struct1.vector_proc[i] > receiver_struct2.vector_proc[i]){
				printf ("EROARE \n");
				break;
			}
		}							 
	}
	MPI_Type_free(&structura);
	MPI_Finalize();
}
