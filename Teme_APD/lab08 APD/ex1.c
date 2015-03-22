#include "mpi.h"
#include <stdio.h>

main(int argc, char *argv[])  {
int numtasks, rank, dest, source, rc, count, tag=1;  


char inmsg, outmsg='x';
int home_brewed_vector[4];
MPI_Status Stat;

MPI_Init(&argc,&argv);
MPI_Comm_size(MPI_COMM_WORLD, &numtasks);
MPI_Comm_rank(MPI_COMM_WORLD, &rank);

if (rank == 0) {
  dest = 1;
  source = 1;
  int i;  
  int vectoras[] = {10,11,12,13};
  for (i = 0 ; i < 4 ; i ++) { 
    rc = MPI_Send(&vectoras[i], 1, MPI_INT, dest, i, MPI_COMM_WORLD);
  //rc = MPI_Recv(&inmsg, 1, MPI_CHAR, source, tag, MPI_COMM_WORLD, &Stat);
    printf ("Trimit catre %d, valoarea %d\n",dest , vectoras[i]);
  }
} 

else if (rank == 1) {
  dest = 0;
  source = 0;
  int i ;
    for (i = 0 ; i < 4; i++) {
        rc = MPI_Recv(home_brewed_vector + i, 1, MPI_INT, 
                    source, MPI_ANY_TAG, MPI_COMM_WORLD, &Stat);
        printf ("Am primit de la %d valoarea %d\n",source , home_brewed_vector[i]);
    }
}

//rc = MPI_Get_count(&Stat, MPI_CHAR, &count);
//printf("Task %d: Received %d char(s) from task %d with tag %d \n",
 //      rank, count, Stat.MPI_SOURCE, Stat.MPI_TAG);


MPI_Finalize();
}
