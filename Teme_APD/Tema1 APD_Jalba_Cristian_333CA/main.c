//author: Jalba Cristian 333CA 

#include<omp.h>
#include<stdio.h>
#include<stdlib.h>

int main(int argc , const char * argv[]) {

	FILE *file_in , *file_out;
	file_in = fopen(argv[3], "r");
	file_out = fopen(argv[4], "w");
	int w_harta, h_harta, w, h, i, j, nr_steps;
	//cand switch e 1 , ramanem pe matrix_out , la -1 comutam pe matrix_aux
	int switcher = 1;	
	//cate generatii avem de creat
	nr_steps = atoi(argv[2]);			
	//cate thread-uri trebuie sa pun
	int num_threads = atoi(argv[1]);	
	//daca e P atunci output-ul este un plan , daca e T atunci este un toroid
	char stance;						
	fscanf (file_in , "%c %d %d %d %d",&stance , &w_harta , &h_harta , &w , &h);
	//matrix_aux e un buffer ce il folosesc pentru a retine urmatoarea generatie 
	int ** matrix_out, ** matrix_aux;
	int ** paux;
	paux = matrix_aux;
	matrix_aux = matrix_out;
	matrix_out = paux;
	//determinam maximul ca sa putem aloca suficient de mult inca sa nu
	//	crape programul cand prelucram matricile
	int h_max, w_max;
	if (h_harta > h) h_max = h_harta;
		else h_max = h;
	if (w_harta > w) w_max = w_harta;
		else w_max = w;
		
		matrix_out = (int **)calloc((h_max*2), sizeof(int));
		matrix_aux = (int **)calloc((h_max*2) , sizeof(int));	
		for (i = 0; i < h_max + 2; i++)
			matrix_out[i] = (int *)calloc((w_max*2), sizeof(int));

		for (i = 0 ; i < h_max + 2 ; i++)
			matrix_aux[i] = (int *)calloc((w_max*2) , sizeof(int));
	//citesc matricea de intrare
	for (i = 1; i < h_harta + 1; i++)
		for (j = 1; j < w_harta + 1; j++)
			fscanf (file_in , "%d ", &matrix_out[i][j]);
		int aux , vec_sum , l;
	
	//setez numarul de thread-uri
	omp_set_num_threads(num_threads);
	//nr_steps e nr de generatii
	for (l = 0 ; l < nr_steps ; l++) {
		//bordez mai intai matricea , aceasta poate fi matrix_out 
		//	sau matrix_aux , depinde doar la ce generatie sunt
		//	(switcher se schimba la crearea unei noi generatii)
		if (switcher == 1){	
			if (stance == 'T') {
				//bordare daca e toroid
				for (i = 1; i <= w; i++) {
					matrix_out[0][i] = matrix_out[h][i];
					matrix_out[h + 1][i] = matrix_out[1][i];
				}
				for (i = 1; i <= h; i++) {
					matrix_out[i][0] = matrix_out[i][w];
					matrix_out[i][w + 1] = matrix_out[i][1];
				}
				matrix_out[0][0] = matrix_out[h][w];
				matrix_out[h + 1][w + 1] = matrix_out[1][1];
				matrix_out[0][w + 1] = matrix_out[h][1];
				matrix_out[h + 1][0] = matrix_out[1][w];
			}
			else {
				//bordare daca e plan 
				for (i = 1; i <= w; i++) {
					matrix_out[0][i] = 0;
					matrix_out[h + 1][i] = 0;
				}
				for (i = 1; i <= h; i++) {
					matrix_out[i][0] = 0;
					matrix_out[i][w + 1] = 0;
				}
				matrix_out[0][0] = 0;
				matrix_out[h + 1][w + 1] = 0;
				matrix_out[0][w + 1] = 0;
				matrix_out[h + 1][0] = 0;	
			}
		}
		//codul este duplicat fiindca nu aveam incredere sa folosesc pointer
		//	ca sa fac swap intre matrici , am simtit ca am mai mult control asa.
		//	Eficienta este aceeasi ca si cum as fi folosit pointeri 
		else {
			if (stance == 'T') {
				for (i = 1; i <= w; i++) {
					matrix_aux[0][i] = matrix_aux[h][i];
					matrix_aux[h + 1][i] = matrix_aux[1][i];
				}
				for (i = 1; i <= h; i++) {
					matrix_aux[i][0] = matrix_aux[i][w];
					matrix_aux[i][w + 1] = matrix_aux[i][1];
				}
				matrix_aux[0][0] = matrix_aux[h][w];
				matrix_aux[h + 1][w + 1] = matrix_aux[1][1];
				matrix_aux[0][w + 1] = matrix_aux[h][1];
				matrix_aux[h + 1][0] = matrix_aux[1][w];
			}
			else {
				for (i = 1; i <= w; i++) {
					matrix_aux[0][i] = 0;
					matrix_aux[h + 1][i] = 0;
				}
				for (i = 1; i <= h; i++) {
					matrix_aux[i][0] = 0;
					matrix_aux[i][w + 1] = 0;
				}
				matrix_aux[0][0] = 0;
				matrix_aux[h + 1][w + 1] = 0;
				matrix_aux[0][w + 1] = 0;
				matrix_aux[h + 1][0] = 0;
			}
		}
		//o sa adun vecinii in suma asta (fiecare vecin e 1 sau 0)
		vec_sum = 0; 
		if (switcher == 1) {
			switcher = (-1) * switcher;	
				#pragma omp parallel for private(i,j,vec_sum) shared(matrix_out , matrix_aux,w,h) default(none) 
				for (i = 1; i < h + 1; i++) {
					for (j = 1; j < w + 1; j++) {
						//verificam vecinii fiecarei pozitii , si updatam matricea
						vec_sum = 0;
						vec_sum = 			matrix_out[i - 1][j - 1] + matrix_out[i][j - 1] + matrix_out[i + 1][j - 1];
						vec_sum = vec_sum + matrix_out[i - 1][j] 	 						+ matrix_out[i + 1][j];
						vec_sum = vec_sum + matrix_out[i - 1][j + 1] + matrix_out[i][j + 1] + matrix_out[i + 1][j + 1];	
						if (matrix_out[i][j] == 0) {	
							//un individ se creaza daca are 3 vecini
							if (vec_sum == 3) {
								matrix_aux[i][j] = 1;
							}
							else 
								matrix_aux[i][j] = 0;
						}
						else {
							//un individ dispare daca nu avem doar 2 sau 3 vecini
							if (vec_sum != 2 && vec_sum != 3) matrix_aux[i][j] = 0;	
							else matrix_aux[i][j] = 1;
						}
					}
				}
		}
		else {
			switcher = (-1)*switcher;
				#pragma omp parallel for private(i,j,vec_sum) shared(matrix_out , matrix_aux,w,h) default(none)
				for (i = 1; i < h + 1; i++) {
					for (j = 1; j < w + 1; j++) {
						vec_sum = 0;
						vec_sum = 			matrix_aux[i - 1][j - 1] + matrix_aux[i][j - 1] + matrix_aux[i + 1][j - 1];
						vec_sum = vec_sum + matrix_aux[i - 1][j] 	 						+ matrix_aux[i + 1][j];
						vec_sum = vec_sum + matrix_aux[i - 1][j + 1] + matrix_aux[i][j + 1] + matrix_aux[i + 1][j + 1];

						if (matrix_aux[i][j] == 0) {
							if (vec_sum == 3) {	
								matrix_out[i][j] = 1;
							}
							else 
								matrix_out[i][j] = 0;
						}
						else {
							if (vec_sum != 2 && vec_sum != 3) matrix_out[i][j] = 0;
							else matrix_out[i][j] = 1;
						}
					}
				}
		}
	}

	int haux = 0 , waux = 0, OK = 0;
	//eliminam portiunea de matrice plina de 0 , nu prea vrem sa avem biti redundanti
	if (switcher == 1) {
		for (i = 1; i <= h  ; i++) {
			OK = 0;
			for (j = 1; j <= w  ; j++)
				if (matrix_out[i][j] == 1) {
					OK = 1;
					if (j > waux ) waux = j;
				}
			if (OK == 1) {
				if (i > haux) haux = i;
			}
		}
	}
	else {
		for (i = 1; i <= h  ; i++) {
			OK = 0;
			for (j = 1; j <= w  ; j++)
				if (matrix_aux[i][j] == 1) {
					OK = 1;
					if (j > waux) waux = j;
				}
			if (OK == 1) {
				if (i > haux) haux = i;
			}
		}
	}
	//afisarea matricei finale
	fprintf(file_out, "%c %d %d %d %d\n",stance , waux , haux , w , h);
	if (switcher == 1) {
		for (i = 1; i <= haux  ; i++) {
			for (j = 1; j <= waux  ; j++)
				fprintf(file_out, "%d ", matrix_out[i][j]);
			fprintf(file_out, "\n");
		}
	}
	else {
		for (i = 1; i <= haux  ; i++) {
			for (j = 1; j <= waux  ; j++)
				fprintf(file_out, "%d ", matrix_aux[i][j]);
			fprintf(file_out, "\n");
		}
	}
	return 0;
}