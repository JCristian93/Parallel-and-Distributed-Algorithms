#include "mpi.h"
#include <stdio.h>
#include <stdlib.h>
#define NUM_COLORS 256

//mandelbrot primeste o poza si datele necesare calculului fractalului si returneaza imaginea fractalului
int ** mandelbrot (int ** matrice_poza , int linie_start , int linie_finish, int nr_iteratii,
						int img_width , double xmin , double ymin, double rezolutie) {
	int i , j;
	double c_x , c_y , z_x , z_y , temp;
	for (j = linie_start ; j <= linie_finish ; j++) {
   		for (i = 0 ; i < img_width ; i++){
   			int step = 0;
   			c_x = xmin + i*rezolutie;
   			c_y = ymin + j*rezolutie;
   			z_x = 0;
   			z_y = 0;
   			while (((z_x*z_x + z_y*z_y) < 4) && (step < nr_iteratii)) {	
   				temp = z_x * z_x - z_y * z_y + c_x;
   				z_y = 2 * z_x * z_y + c_y;
   				step++;
   				z_x = temp;
   			}
   			matrice_poza[j][i] = step%NUM_COLORS;
   		}
    }
   	return matrice_poza;
}
//julia face acelasi lucru ca mandelbrot cu diferenta ca c-ul e contant in julia si z e variabil
//mai exact julia e un caz particular al setului mandelbrot
int ** julia (int ** matrice_poza , int linie_start , int linie_finish, int nr_iteratii,
						int img_width , double xmin , double ymin, double rezolutie, double julia_c_x , double julia_c_y) {
	int i , j;
	double c_x , c_y , z_x , z_y , temp;	
	for (j = linie_start ; j <= linie_finish ; j++) {
   		for (i = 0 ; i < img_width ; i++) {
   			int step = 0;
   			c_y = julia_c_y;
   			c_x = julia_c_x;
   			z_x = xmin + i*rezolutie;
   			z_y = ymin + j*rezolutie;
   			while (((z_x*z_x + z_y*z_y) < 4) && (step < nr_iteratii)) {	
   				temp = z_x * z_x - z_y * z_y + c_x;
   				z_y = 2 * z_x * z_y + c_y;
   				step++;
   				z_x = temp;
   			}														
   			matrice_poza[j][i] = step%NUM_COLORS;
   		}
   	}
   	return matrice_poza;
}

int main(int argc, char *argv[]) {
	int  numtasks, rank, len, rc; 
	char hostname[MPI_MAX_PROCESSOR_NAME];
	//comenzi de rutina pentru orice program scris in MPI
	rc = MPI_Init(&argc,&argv);
  
   	if (rc != MPI_SUCCESS) {
   		printf ("Error starting MPI program. Terminating.\n");
   	 	MPI_Abort(MPI_COMM_WORLD, rc);
   	}

   	MPI_Comm_size(MPI_COMM_WORLD,&numtasks);
   	MPI_Comm_rank(MPI_COMM_WORLD,&rank);
   	MPI_Get_processor_name(hostname, &len);
   	MPI_Status status;
   	printf ("Number of tasks= %d My rank= %d Running on %s\n", numtasks,rank,hostname);

   	if (rank == 0) {
   		//citim fisierul in care se afla datele necesare pentru calcul
   		double xmin,  xmax , ymin , ymax , rezolutie;
   		int tip_fractal , nr_iteratii , img_height, img_width;
   	
  	 	FILE * fisier_intrare;	
   		FILE * fisier_iesire;	
   		fisier_intrare = fopen (argv[1] , "r");
   		int ret = fscanf(fisier_intrare , "%d\n%lf %lf %lf %lf\n%lf\n%d", &tip_fractal , 
   										&xmin, &xmax , &ymin , &ymax, &rezolutie , &nr_iteratii);
   		if (ret == 0) return 0;	
   		double julia_c_x , julia_c_y;
   		if (tip_fractal == 1) {
   			//inseamna ca e Julia si la final mai avem de citit c_x si c_y
   			int retj = fscanf(fisier_intrare , "%lf %lf", &julia_c_x , &julia_c_y);
   			if (retj == 0) return 0;
   		}

	   	fclose(fisier_intrare);
   		//calculam dimensiunile pozei si alocam matricea de pixeli
   		int i ,j;
		img_width = (int)((xmax - xmin)/rezolutie);
		img_height = (int)((ymax - ymin)/rezolutie);
		int **matrice_poza = (int **) calloc (img_height , sizeof(int *));
   		for (i = 0 ; i < img_height ; i++)
   			matrice_poza[i] = (int*) calloc (img_width , sizeof(int));
   		//pregatesc "pachetele" ce vor fi transmise la celelalte procese pentru a calcula si ele in paralel 
   		//portiunea lor de poza
   		double *pachet_doubles;		
				pachet_doubles = (double *) calloc (10 ,  sizeof (double));
		pachet_doubles[0] = xmin;
		pachet_doubles[1] = xmax;
		pachet_doubles[2] = ymin;
		pachet_doubles[3] = ymax;
		pachet_doubles[4] = rezolutie;
		pachet_doubles[5] = julia_c_x;
		pachet_doubles[6] = julia_c_y;
		int *pachet_ints;
			pachet_ints = (int *) calloc (10 , sizeof(int));
		pachet_ints[0] = nr_iteratii;
		pachet_ints[3] = img_width;							
		pachet_ints[4] = img_height;
		pachet_ints[5] = tip_fractal;
		int * antet;
		int linie;
		antet = (int *) malloc (10 * sizeof(int)); 
   		
   			//masterul isi face partea sa de calcul aici 
   			if (tip_fractal == 0) 	
   				matrice_poza = mandelbrot(matrice_poza , 0, img_height/numtasks - 1, nr_iteratii,
   						img_width , xmin , ymin , rezolutie);
   			else 	matrice_poza = julia (matrice_poza , 0, img_height/numtasks - 1, nr_iteratii,
   						img_width , xmin , ymin , rezolutie, julia_c_x , julia_c_y);
			
   			//trimit pachetele la fiecare proces
			for (i = 1 ; i < numtasks ; i++) {
				pachet_ints[1] = i * img_height / numtasks;			//linia de inceput
				pachet_ints[2] = (i+1) * img_height / numtasks - 1;	//linia de sfarsit
				if (i == numtasks - 1)	pachet_ints[2] = img_height - 1;
				MPI_Send(pachet_ints , 10 , MPI_INT, i , 42 , MPI_COMM_WORLD);	
				MPI_Send(pachet_doubles , 10 , MPI_DOUBLE, i , 42 , MPI_COMM_WORLD);
			}   
			for (i = 1 ; i < numtasks ; i++){
				//primesc reply de la procese
				//mai intai vine un pachet ce imi va spune cata informatie trebuie sa ajunga de la procesul respectiv
				MPI_Recv (antet , 10 , MPI_INT , i , 42 , MPI_COMM_WORLD, &status);
				if (antet[0] != i) {
					printf ("eroare de trimitere a antetului de la rank %d", antet[0]);
				}
				else {
					for (linie = antet[1] ; linie <= antet[2] ; linie ++) {
						//fiecare proces transmite catre master linie cu linie bucata sa de poza
						MPI_Recv (matrice_poza[linie] , img_width , MPI_INT , antet[0], 42, MPI_COMM_WORLD, &status);
					}
				}
			}	
   			
   		//acum ca am construit matricea pozei , e timpul sa afisez poza in forma ei finala
    	fisier_iesire = fopen (argv[2] , "w");
 		fprintf(fisier_iesire, "P2\n");
 		fprintf(fisier_iesire, "%d %d\n",img_width , img_height);
 		fprintf(fisier_iesire, "%d\n",NUM_COLORS - 1);
    	for (j = img_height - 1; j >= 0 ; j--) {
    		for (i = 0 ; i < img_width ; i++) {
    			fprintf (fisier_iesire , "%d ", matrice_poza[j][i]);
    		}
    		fprintf (fisier_iesire , "\n");
    	}
    	fclose(fisier_iesire);	
   	}
   	else {	
   		//pentru orice alt rank , ma ocup de prelucrare
   		int i;
   		int * pachet_ints = (int *) malloc (10 * sizeof(int));
   		double * pachet_doubles = (double *) malloc (10 * sizeof(double));
   		//primesc datele de la rank 0 , apoi creez matricea partiala a pozei
   		MPI_Recv (pachet_ints , 10 , MPI_INT , 0 , 42 , MPI_COMM_WORLD , &status);
   		MPI_Recv (pachet_doubles , 10 , MPI_DOUBLE , 0 , 42 , MPI_COMM_WORLD , &status);
   		int ** poza_partiala;	
   		poza_partiala = (int **) calloc (pachet_ints[4] , sizeof (int));
   		for (i = 0 ; i < pachet_ints[4] ; i++) {
   			poza_partiala[i] = (int *) calloc (pachet_ints[3] , sizeof (int));
   		}
   		//procesul isi face aici partea sa de munca
		if (pachet_ints[5] == 0)
			poza_partiala = mandelbrot (poza_partiala , pachet_ints[1] , pachet_ints[2] , pachet_ints[0],
   						pachet_ints[3] , pachet_doubles[0], pachet_doubles[2], pachet_doubles[4]);
   		else {
   			poza_partiala =  julia (poza_partiala , pachet_ints[1] , pachet_ints[2] , pachet_ints[0],
   						pachet_ints[3] , pachet_doubles[0], pachet_doubles[2], pachet_doubles[4] , pachet_doubles[5] , pachet_doubles[6]);
   		}
   		//pregatesc antetul ce va spune masterului cate linii de poza or sa vina pe comunicator
		int * antet = (int *) malloc (10 * sizeof (int));
		antet[0] = rank;
		antet[1] = pachet_ints[1];
		antet[2] = pachet_ints[2];
		//o voi transmite linie cu linie , nu am cum altfel ca nu poate comunicatorul sa transmita prea multe date
		MPI_Send(antet , 10 , MPI_INT , 0 , 42 , MPI_COMM_WORLD);
		for (i = pachet_ints[1] ; i <= pachet_ints[2] ; i++) {
			MPI_Send(poza_partiala[i] , pachet_ints[3] , MPI_INT , 0 , 42 , MPI_COMM_WORLD);
		}
   	}
   	MPI_Finalize();
}
