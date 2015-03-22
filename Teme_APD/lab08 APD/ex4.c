#include "mpi.h"
#include <stdio.h>
#define NPROCS 9

main(int argc, char *argv[])  {
int        rank, new_rank, sendbuf, recvbuf, numtasks,
           ranks1[3]={0,1,2}, ranks2[3]={3,4,5} , ranks3[3]={6,7,8};
MPI_Group  orig_group, new_group;
MPI_Comm   new_comm;

MPI_Init(&argc,&argv);
MPI_Comm_rank(MPI_COMM_WORLD, &rank);
MPI_Comm_size(MPI_COMM_WORLD, &numtasks);

if (numtasks != NPROCS) {
  printf("Must specify MP_PROCS= %d. Terminating.\n",NPROCS);
  MPI_Finalize();
  exit(0);
  }

sendbuf = rank;


/* Extract the original group handle */
MPI_Comm_group(MPI_COMM_WORLD, &orig_group);
int valoare = 0;
/* Divide tasks into two distinct groups based upon rank */
if (rank < NPROCS/3) {
  if (rank == 0)  valoare = 1993;
  MPI_Group_incl(orig_group, NPROCS/3, ranks1, &new_group);
}
else if (rank < 2*NPROCS/3){
  if (rank == 3)valoare = 1994;
  MPI_Group_incl(orig_group, NPROCS/3, ranks2, &new_group);
}
else {
  if (rank == 6)valoare = 1995;
  MPI_Group_incl(orig_group, NPROCS/3, ranks3, &new_group);      
}

/* Create new new communicator and then perform collective communications */
MPI_Comm_create(MPI_COMM_WORLD, new_group, &new_comm);
MPI_Allreduce(&sendbuf, &recvbuf, 1, MPI_INT, MPI_SUM, new_comm);


MPI_Group_rank (new_group, &new_rank);

MPI_Bcast (&valoare,1,MPI_INT,0,new_comm); 

printf("rank= %d newrank= %d recvbuf= %d valoare= %d\n",rank,new_rank,recvbuf,valoare);





MPI_Finalize();
}

