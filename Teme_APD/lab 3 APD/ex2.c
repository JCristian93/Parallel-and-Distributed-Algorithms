/******************************************************************************
* FILE: sort.c
* AUTHOR: Cristian Chilipirea
* Generates serially sorts and displays a vector
******************************************************************************/
#include<stdio.h>
#include<stdlib.h>

#define N 1000

int main(int argc, char argv[]) {
	srand(42);
	int v[N];
	int i;
	int sorted = 0;
	int aux;

	// generate the vector v with random values
	for(i = 0; i < N; i++) {
		v[i] = rand()%N;
	}
	omp_set_num_threads(N);
	// sort the vector v
	while(!sorted) {
	
		sorted = 1;
		#pragma omp parallel 
		{

			#pragma omp for private(i,aux)	
			for(i = 0; i < N-1; i+= 2) {
				if(v[i] > v[i + 1]) {
					aux = v[i];
					v[i] = v[i + 1];
					v[i + 1] = aux;
					sorted =0;
				}
			}
			#pragma omp for private (i,aux)
			for(i = 1; i < N-1; i+= 2) {
				if(v[i] > v[i + 1]) {
					aux = v[i];
					v[i] = v[i + 1];
					v[i + 1] = aux;
					sorted =0;
				}
			}
		}
	}

	// display the vector v
/*	for(i = 0; i < N; i++) {
		printf("%i\t", v[i]);
	}
*/	
	int sw=0;
	for(i = 0; i < N-1; i++)
		if(v[i]>v[i+1])
			sw=1;
	if(sw)
		printf("Sorted incorrectly\n");
	else
		printf("Sorted correctly\n");
	printf("\n");
}
