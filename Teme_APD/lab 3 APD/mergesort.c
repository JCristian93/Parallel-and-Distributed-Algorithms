#include<stdio.h>
#include<stdlib.h>

#define N 1048576 // needs to be power of 2
//#define N 1024
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

int main(int argc, char argv[]) {
	srand(42);
	int *v = (int*)malloc(sizeof(int)*N);
	int *vNew = (int*)malloc(sizeof(int)*N);
	int *aux;
	int i, width;
	int sorted = 0;

	// generate the vector v with random values
	for(i = 0; i < N; i++) {
		v[i] = rand()%N;
	}
//	int nr_procs = omp_get_num_proc();
	omp_set_num_threads(1);
	double start_time = omp_get_wtime();
	// sort the vector v

		for (width = 1; width < N; width = 2 * width) {
			#pragma omp parallel for 
			for (i = 0; i < N; i = i + 2 * width) {
				merge(v, i, i+width, i + 2*width, vNew);
			}
		//	#pragma omp critical {
				aux = v;
				v= vNew;
				vNew=aux;
		//	}
		}
	double time = omp_get_wtime() - start_time;
	// display the vector v
//	#pragma omp parallel for 
/*	for(i = 0; i < N; i++) {
		printf("%i\t", v[i]);
	}
	printf("\n");
*/
	int sw=0;
	for(i = 0; i < N-1; i++)
		if(v[i]>v[i+1])
			sw=1;
	if(sw)
		printf("Sorted incorrectly\n");
	else
		printf("Sorted correctly\n");
	printf("\n time : %lf" , time);
}
