#include "mpi.h"
#include <stdio.h>

main(int argc, char *argv[])  {
int numtasks, rank, dest, source, rc, count, tag=1;  
int vectoras[] = {10,11,12,13};

char inmsg, outmsg='x';
int home_brewed_vector[4];
MPI_Status Stat;

MPI_Init(&argc,&argv);
MPI_Comm_size(MPI_COMM_WORLD, &numtasks);
MPI_Comm_rank(MPI_COMM_WORLD, &rank);

if (rank == 0) {
  dest = 4;
  source = 0;
  int i = 10;  
    rc = MPI_Send(&i, 1, MPI_INT, dest, i, MPI_COMM_WORLD);
  //rc = MPI_Recv(&inmsg, 1, MPI_CHAR, source, tag, MPI_COMM_WORLD, &Stat);
    printf ("Trimit catre %d, valoarea %d\n",dest , i);
 
} 

else if (rank == 1) {
  dest = 4;
  source = 1;
  int i = 11;  
    rc = MPI_Send(&i, 1, MPI_INT, dest, i, MPI_COMM_WORLD);
  //rc = MPI_Recv(&inmsg, 1, MPI_CHAR, source, tag, MPI_COMM_WORLD, &Stat);
    printf ("Trimit catre %d, valoarea %d\n",dest , i);
  
}
else if (rank == 2) {
  dest = 4;
  source = 1;
  int i = 12;  
    rc = MPI_Send(&i, 1, MPI_INT, dest, i, MPI_COMM_WORLD);
  //rc = MPI_Recv(&inmsg, 1, MPI_CHAR, source, tag, MPI_COMM_WORLD, &Stat);
    printf ("Trimit catre %d, valoarea %d\n",dest , i);
     
}
else if (rank == 3) {
  dest = 4;
  source = 1;
  int i = 13;  
    rc = MPI_Send(&i, 1, MPI_INT, dest, i, MPI_COMM_WORLD);
  //rc = MPI_Recv(&inmsg, 1, MPI_CHAR, source, tag, MPI_COMM_WORLD, &Stat);
    printf ("Trimit catre %d, valoarea %d\n",dest , i);
  
}

else if (rank == 4) {
  dest = 0;
  source = 0;
  int i ;
    for (i = 0 ; i < 4; i++) {
        rc = MPI_Recv(home_brewed_vector + i, 1, MPI_INT, 
                    MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &Stat);
        printf ("Am primit valoarea %d\n", home_brewed_vector[i]);
    }
}
MPI_Finalize();
}
