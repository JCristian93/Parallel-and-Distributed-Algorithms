/******************************************************************************
* FILE: sort.c
* AUTHOR: Cristian Chilipirea
* Generates serially sorts and displays a vector
******************************************************************************/
#include<stdio.h>
#include<stdlib.h>
#include<math.h>
#define N 10

void bubble_sort_cresc (int vectoras[]) {
	
	
	int aux , ok , i; 
	do {
		ok = 0;
		for (i = 0 ; i < N - 1 ; i ++) {
			if (vectoras [i] > vectoras [i+1]) {
				aux = vectoras [i];
				vectoras [i] = vectoras[i+1];
				vectoras [i+1] = aux;			
				ok = 1;
			}
		}
	
	 }while (ok);	
	
}

void bubble_sort_descresc (int vectoras[]) {

	int aux , ok , i; 
	do {
		ok = 0;
		for (i = 0 ; i < N - 1 ; i ++) {
			if (vectoras [i] < vectoras [i+1]) {
				aux = vectoras [i];
				vectoras [i] = vectoras[i+1];
				vectoras [i+1] = aux;			
				ok = 1;
			}
		}
	
	 }while (ok);	
	
}

int main(int argc, char argv[]) {
	srand(42);
	int v[N][N];
	int i,j;
	int sorted = 0;
	int aux;

	// generate the vector v with random values
	for(i = 0; i < N; i++)
		for(j = 0; j < N; j++)
			v[i][j] = rand()%N; 

	// sort the matrix v
	int l ;
	omp_set_num_threads(1);
	for (l = 0 ; l < log2(N) ; l++) {
		//sortam liniile	
		#pragma omp parallel for private(i)
		for(i = 0; i < N; i++)
			if (i % 2 == 0) {
				bubble_sort_descresc(v[i]);
			}
			else 
				bubble_sort_cresc(v[i]);		

		//sortam coloanele
		int auxv[N] , contor = 0 ;
		int k;
		for(i = 0; i < N; i++){
			#pragma omp parallel for private(k)
			for (k = 0 ; k < N ; k++)
				auxv[k] = v[k][i];
			bubble_sort_cresc (auxv);
			#pragma omp parallel for private(k)
			for (k = 0 ; k < N ; k++)
				v[k][i] = auxv[k];
		}	
	}
			
	// display the vector v
	for(i = 0; i < N; i++) {
		if(i%2) {
			for(j = 0; j < N; j++) {
				printf("%i ", v[i][j]);
			}
		} else {
			for(j = N-1; j >= 0; j--) {
				printf("%i ", v[i][j]);
			}
		}
		printf ("\n");
	}
	printf("\n");

/*	printf ("Verificam bubble sort : \n");
	bubble_sort_cresc(v[0]);
	for (i = 0 ; i < N ; i ++)
		printf ("%d ", v[0][i]);			
*/	
}
