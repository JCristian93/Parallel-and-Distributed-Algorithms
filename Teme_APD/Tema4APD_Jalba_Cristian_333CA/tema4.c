#include "mpi.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define dimensiune_mesaj 256

int main(int argc, char *argv[]) {
	int  numtasks ,rank, len, rc , i, j , k , l ,m; 
	char hostname[MPI_MAX_PROCESSOR_NAME];
	rc = MPI_Init(&argc,&argv);
  
   	if (rc != MPI_SUCCESS) {
   		printf ("Error starting MPI program. Terminating.\n");
   	 	MPI_Abort(MPI_COMM_WORLD, rc);
   	}
   	MPI_Comm_size(MPI_COMM_WORLD,&numtasks);
   	MPI_Comm_rank(MPI_COMM_WORLD,&rank);
   	MPI_Get_processor_name(hostname, &len);
   	MPI_Status status;
   	MPI_Request request;

    typedef struct mesaj
    {
      int sursa;
      int destinatie;
      int parinte;
      char info [dimensiune_mesaj];
    } Mesaj_prim;

    Mesaj_prim p[4] , part[4];
  
    MPI_Status stat;
    MPI_Datatype structura_mesaj , oldtypes[4];
    int blockcounts[4];
    MPI_Aint offsets[4] , extent;
  
    offsets[0] = 0;
    oldtypes[0] = MPI_INT;
    blockcounts[0] = 1;
    offsets[1] = 1;
    oldtypes[1] = MPI_INT;
    blockcounts[1] = 1;
    offsets[2] = 2;
    oldtypes[2] = MPI_INT;
    blockcounts[2] = 1;
    offsets[3] = 3;
    oldtypes[3] = MPI_CHAR;
    blockcounts[3] = dimensiune_mesaj;
    
    MPI_Type_struct(4, blockcounts , offsets , oldtypes , &structura_mesaj);
    MPI_Type_commit(&structura_mesaj);
    
   	if (rank == 0) {
   		//0 va actiona ca initiator 
   		FILE * fp;
      	char * line = NULL;
        size_t len = 0;
        ssize_t read;
        fp = fopen(argv[1], "r");
        int *matrice_locala_adiacenta = (int *) malloc (numtasks * numtasks * sizeof (int));
       	for (i = 0 ; i < numtasks * numtasks ; i++) {
       		matrice_locala_adiacenta[i] = -1;		
       	}
        if (fp == NULL)
           exit(EXIT_FAILURE);

        while ((read = getline(&line, &len, fp)) != -1) {
           	char * char_buff;
           	char_buff = (strtok (line , " -\n"));
           	if (char_buff != NULL && atoi(char_buff) == rank) {
           		matrice_locala_adiacenta[rank * numtasks] = atoi (strtok (NULL , " -\n"));
           		int count = 0;
           		while ((char_buff = strtok (NULL , " -\n")) != NULL) {
           			count++;
           			matrice_locala_adiacenta[rank * numtasks + count] = atoi(char_buff); 
           		}
           	}
           	break;
        }
        fclose(fp);
        //in punctul acesta cunoastem doar vecinii radacinii , asa ca trimitem mesaj de ecou la toti si asteptam reply
        int *mesaj_ecou = (int *) malloc (numtasks * numtasks * sizeof(int));
        for (i = 0 ; i < numtasks * numtasks ; i++) {
       		mesaj_ecou[i] = -1;		
       	} 
       	//trimit ecoul la toti vecinii , apoi ei trebuie sa faca la fel pana se ajunge la frunze 
       	//frunzele trimit inapoi numele lor si nodurile intermediare recompun mesajul de ecou ce se va intoarce catre radacina
       	for (i = 0 ; i < numtasks ; i ++) {
       		if (matrice_locala_adiacenta[i] != -1) {
       			MPI_Isend (mesaj_ecou , numtasks * numtasks , MPI_INT , matrice_locala_adiacenta[i] , 42 , MPI_COMM_WORLD , &request);
       		}
       	}
   		for (i = 0 ; i < numtasks ; i ++) {
       		if (matrice_locala_adiacenta[i] != -1) {
       			MPI_Recv (mesaj_ecou , numtasks * numtasks , MPI_INT , matrice_locala_adiacenta[i] , MPI_ANY_TAG , MPI_COMM_WORLD , &status);
       			for (j = 0 ; j < numtasks * numtasks ; j++) {
       				if (mesaj_ecou[j] != -1) matrice_locala_adiacenta[j] = mesaj_ecou[j];
       			}
       		}
       	}
      
      int arbore_acoperire[numtasks][numtasks];
      for (i = 0 ; i < numtasks ; i++) {
          for (j = 0 ; j < numtasks ; j++) {
              arbore_acoperire[i][j] = -1;
          }
      }
      for (i = 1 ; i < numtasks ; i++) {
          arbore_acoperire[i][matrice_locala_adiacenta[i * numtasks + numtasks - 1]] = 0;
          arbore_acoperire[matrice_locala_adiacenta[i * numtasks + numtasks - 1]][i] = 0;
      }
      
      int tabela_rutare[numtasks*numtasks][numtasks*numtasks];      
      for (i = 0 ; i < numtasks * numtasks; i++) {
          for (j = 0 ; j < 3 ; j++) {
              tabela_rutare[i][j] = -1;
          }
      }
      //calculez tabela de rutare
      //pornesc cu DFS de la nodul destinatie si caut ultimul nod pana la sursa
      for (i = 0 ; i < numtasks ; i++) {
        for (j = 0 ; j < numtasks ; j++) {
            tabela_rutare[i*numtasks + j][0] = i;
            tabela_rutare[i*numtasks + j][1] = j;
          if (i == j) {
            tabela_rutare[i*numtasks + j][2] = i;
          }
          else {
            //sursa e i , destinatia e j
            int ok = 0 , nod_curent = i , facem_negru = 1;
            int culori[numtasks]; //0 e alb , 1 , e gri , 2 e negru
            for (k = 0 ; k < numtasks ; k++) {
              culori[k] = 0;
            }
            while (ok == 0) {
              culori[nod_curent] = 1;
              facem_negru = 1;
              for (k = 0 ; k < numtasks ; k++) {
                if (arbore_acoperire[nod_curent][k] == 0) {
                  if (k == j) {
                    tabela_rutare[i*numtasks + j][2] = nod_curent;
                    ok = 1;
                    break;
                  }
                  else if (culori[k] == 0) {
                    nod_curent = k;
                    facem_negru = 0;
                    break;
                  }
                }
              }
              if (facem_negru == 1) {
                culori[nod_curent] = 2;
                for (k = 0 ; k < numtasks ; k++) {
                  if (arbore_acoperire[nod_curent][k] == 0) {
                   if (culori[k] == 1) {
                    nod_curent = k;
                    break;
                   } 
                  }
                }
              }  
            }

          }
        }
      }
        
      FILE * fpout;
      fpout = fopen ("output_etapa_1" , "a");
      for (i = 0 ; i < numtasks ; i++) {
          for (j = 0 ; j < numtasks ; j++) {
            fprintf(fpout, "%d ", arbore_acoperire[i][j]);
          }
          fprintf(fpout, "\n");
        }
      fprintf(fpout, "\n");
      fprintf(fpout, "%d\n", rank);
      for (i = 0 ; i < numtasks * numtasks; i++) {
        if (tabela_rutare[i][0] != -1) {
          if (tabela_rutare[i][1] == rank){
            fprintf(fpout, "%d ", tabela_rutare[i][2]);
            fprintf(fpout, "%d ", tabela_rutare[i][0]);
            fprintf(fpout , "\n");
          }
        }
      }
      fclose (fpout);
        //acum ca am topologia gata , o trimit la fiecare nod in parte , trimit si tabela de rutare
      for (i = 0 ; i < numtasks ; i++) {
        if (matrice_locala_adiacenta[i] != -1) {
         MPI_Isend (arbore_acoperire , numtasks * numtasks , MPI_INT , matrice_locala_adiacenta[i] , 43 , MPI_COMM_WORLD , &request);        
        }
      }
      //INCEPE ETAPA 2
      FILE * fpin;
      fpin = fopen ("mesaje_input_etapa_2" , "r");
      int numar_mesaje;
      line = NULL;
      len = 0;
      int ret = fscanf (fpin , "%d" , &numar_mesaje);
      if (ret < 0)
          printf("Fisierul de input pt etapa 2 e corupt sau gol\n");
      Mesaj_prim messages[numar_mesaje];
      i=0;
      fseek (fpin , 2 , SEEK_SET);
      while ((read = getline(&line, &len, fpin)) > 4) {
          char * char_buff;
          char * pointer = line;
          while (*pointer != ' '){
            pointer++;
          }
          pointer++;
          while (*pointer != ' '){
            pointer++;
          }
          pointer++;
          strcpy(messages[i].info , pointer);
          char_buff = (strtok (line , " "));
          messages[i].sursa = atoi(char_buff);
          messages[i].parinte = rank;
          char_buff = (strtok (NULL , " "));
          if (strcmp (char_buff , "B") == 0) {
            messages[i].destinatie = -1;
          }
          else {
            messages[i].destinatie = atoi (char_buff);
          }
          i++;
      }
      if (line)
            free(line);
      fclose(fpin);
      //dam mesajul de broadcast pentru a semnaliza inceputul etapei
   	  FILE * fpcolectiv2;
      fpcolectiv2 = fopen ("output_etapa_2" , "a");
      int parinte;  
      Mesaj_prim broadcast;
      broadcast.sursa = rank;
      broadcast.destinatie = -1;
      broadcast.parinte = rank;
      strcpy (broadcast.info , "Initiere etapa 2");
      for (i = 0 ; i < numtasks ; i++){
        if (arbore_acoperire[0][i] == 0)
          MPI_Isend (&broadcast , 1 , structura_mesaj , i , 17 , MPI_COMM_WORLD , &request);
      }
      for (i = 0 ; i < numar_mesaje ; i++) {
          if (messages[i].sursa == rank && messages[i].destinatie != -1) {
            for (j = 0 ; j < numtasks * numtasks; j++) {
              if (tabela_rutare[j][0] != -1) {
                if (tabela_rutare[j][1] == rank){
                  if (tabela_rutare[j][0] == messages[i].destinatie) {
                    messages[i].parinte = rank;
                    MPI_Isend (&messages[i] , 1, structura_mesaj , tabela_rutare[j][2] , j + 180 , MPI_COMM_WORLD , &request);
                    break;
                  }
                }
              }
            } 
          }
          else {
            for (j = 0 ; j < numtasks ; j++) {
              //atentie aici , ca trimit la ABSOLUT toti vecinii , nu iert pe cineva , nu am parinte
              if (matrice_locala_adiacenta[rank * numtasks + j] == 0) {
                messages[i].parinte = rank;
                MPI_Isend (&messages[i] , 1 , structura_mesaj , j , j + 210 , MPI_COMM_WORLD , &request);
              }
            }
          }
      }
        int conditie_ascultare = 1 , timeout = numtasks * numar_mesaje * 0.02;
        //daca se depaseste timeout-ul si nu a primit un mesaj inseamna ca trebuie sa incheie
        while (conditie_ascultare == 1) {
          Mesaj_prim new_message;
          new_message.sursa = -2;
          new_message.destinatie = -2;
          conditie_ascultare = 0;
          for (i = 0 ; i < numtasks ; i++) {
            MPI_Irecv (&new_message , 1 , structura_mesaj , MPI_ANY_SOURCE , MPI_ANY_TAG , MPI_COMM_WORLD , &request);
            parinte = new_message.parinte;   
            new_message.parinte = rank;
            if (new_message.sursa != -2 && new_message.destinatie != -2) {
              conditie_ascultare = 1;
              if (new_message.destinatie == -1) {
                for (j = 0; j < numtasks ; j++) {
                  if (matrice_locala_adiacenta[rank*numtasks + j] == 0 && j != parinte) {
                    fprintf(fpcolectiv2, "sunt nodul %d mesaj de broadcast de la %d\n", rank , parinte);
                    MPI_Isend (&new_message , 1 , structura_mesaj , j , j + 110 , MPI_COMM_WORLD , &request);
                  }
                }
              }
              else if (new_message.sursa == rank) {
                fprintf (fpcolectiv2 , "sunt nodul %d \nam primit de la %d mesaj cu sursa %d\n iar mesajul este:\n%s\n",
                                                      rank , parinte , new_message.sursa , new_message.info);
              }
              else {
                //daca nu e broadcast folosesc tabela de rutare sa stiu sa trimit direct la nodul bun
                for (j = 0 ; j < numtasks * numtasks ; j++) {
                  if (tabela_rutare[j][1] == rank && tabela_rutare[j][0] == new_message.destinatie) {
                    fprintf (fpcolectiv2 , "sunt nodul %d \nam primit de la %d si trimit catre %d mesajul:\n%s\n", 
                                    rank , new_message.sursa , tabela_rutare[j][2] , new_message.info);
                    MPI_Isend(&new_message , 1, structura_mesaj , tabela_rutare[j][2] , j + 300 , MPI_COMM_WORLD , &request);
                  }
                }
              }
            }
          }
          //daca a ramas 0 , pun wait(timeout) si mai ascult inca o data
          if (conditie_ascultare == 0) {
            sleep(timeout);
            for (i = 0 ; i < numtasks ; i++) {
            MPI_Irecv (&new_message , 1 , structura_mesaj , MPI_ANY_SOURCE , MPI_ANY_TAG , MPI_COMM_WORLD , &request);
            parinte = new_message.parinte;
            new_message.parinte = rank;  
            if (new_message.sursa != -2 && new_message.destinatie != -2) {
              conditie_ascultare = 1;
              if (new_message.destinatie == -1) {
                for (j = 0; j < numtasks ; j++) {
                  if (matrice_locala_adiacenta[rank*numtasks + j] == 0 && j != parinte) {
                    fprintf(fpcolectiv2, "sunt nodul %d mesaj de broadcast de la %d\n", rank , parinte);
                    MPI_Isend (&new_message , 1 , structura_mesaj , j , j + 110 , MPI_COMM_WORLD , &request);
                  }
                }
              }
              else if (new_message.sursa == rank) {
                fprintf (fpcolectiv2 , "sunt nodul %d \nam primit de la %d mesaj cu sursa %d\n iar mesajul este:\n%s\n",
                                                      rank , parinte , new_message.sursa , new_message.info);
              }
              else {
                //daca nu e broadcast folosesc tabela de rutare sa stiu sa trimit direct la nodul bun
                for (j = 0 ; j < numtasks * numtasks ; j++) {
                  if (tabela_rutare[j][1] == rank && tabela_rutare[j][0] == new_message.destinatie) {
                    fprintf (fpcolectiv2 , "sunt nodul %d \nam primit de la %d si trimit catre %d mesajul:\n%s\n", 
                                    rank , new_message.sursa , tabela_rutare[j][2] , new_message.info);
                    MPI_Isend(&new_message , 1, structura_mesaj , tabela_rutare[j][2] , j + 300 , MPI_COMM_WORLD , &request);
                  }
                }
              }
            }
          }
          }
        }
        //acum ca a trecut timeout-ul si nu am primit mesaje , e timpul sa dau broadcast 
        Mesaj_prim broadcast_final;
        broadcast_final.sursa = rank;
        broadcast_final.destinatie = -1;
        broadcast_final.parinte = rank;
        strcpy (broadcast.info , "Incheiati etapa 2");
              
        for (i = 0 ; i < numtasks ; i++) {
          if (matrice_locala_adiacenta[rank * numtasks + i] == 0)
            MPI_Isend (&broadcast_final , 1 , structura_mesaj , i , j + 400 , MPI_COMM_WORLD , &request);
        }
    }
   	else {	
   		FILE * fp;
      	char * line = NULL;
        size_t len = 0;
        ssize_t read;
        fp = fopen(argv[1], "r");
        int *matrice_locala_adiacenta = (int *) malloc (numtasks * numtasks * sizeof (int));
       	int count = 0;
       	for (i = 0 ; i < numtasks * numtasks ; i++) {
       		matrice_locala_adiacenta[i] = -1;		
       	}
        if (fp == NULL)
           exit(EXIT_FAILURE);
        while ((read = getline(&line, &len, fp)) != -1) {
           	char * char_buff;
           	char_buff = (strtok (line , " -\n"));
           	if (char_buff != NULL && atoi(char_buff) == rank) {
           		matrice_locala_adiacenta[rank * numtasks] = atoi (strtok (NULL , " -\n"));
           		count = 0;
           		while ((char_buff = strtok (NULL , " -\n")) != NULL) {
           			count++;
           			matrice_locala_adiacenta[rank * numtasks + count] = atoi(char_buff); 
           		}
           		break;
           	}
        }
        fclose(fp);
        int *mesaj_ecou , ok = 0 , *mesaj_sonda;
        mesaj_ecou = (int *) malloc (numtasks * numtasks * sizeof (int));
        mesaj_sonda = (int *) malloc (numtasks * numtasks * sizeof (int));
        for (i = 0 ; i < numtasks * numtasks ; i++) {
        	mesaj_sonda[i] = -1;
        }
        MPI_Recv (mesaj_ecou , numtasks * numtasks , MPI_INT , MPI_ANY_SOURCE , MPI_ANY_TAG , MPI_COMM_WORLD , &status);
        int parinte = status.MPI_SOURCE;
        for (i = 0 ; i < numtasks ; i++) {
        	if (matrice_locala_adiacenta[rank * numtasks + i] != -1 && matrice_locala_adiacenta[rank * numtasks + i] != parinte) {
        		MPI_Isend (mesaj_sonda , numtasks * numtasks , MPI_INT , matrice_locala_adiacenta[rank * numtasks + i] , 42 + i , MPI_COMM_WORLD , &request);
        		ok = 1;
        	}
        }
        if (ok == 0) {	
        	for (i = 0 ; i < numtasks * numtasks; i++) {
        		if (matrice_locala_adiacenta[i] != -1) mesaj_ecou[i] = matrice_locala_adiacenta[i];
        	}
          mesaj_ecou[rank * numtasks + numtasks - 1] = parinte;
        	if (ok == 0)	MPI_Isend (mesaj_ecou , numtasks * numtasks , MPI_INT , parinte , 42 , MPI_COMM_WORLD , &request);
        }
        else {
        	//daca nu suntem frunza trebuie sa asteptam reply de la copii si sa le combinam , apoi sa adaugam vecinii proprii si sa trimitem la parinte
        	int *ecou_buffer , verif_ecou = 0;
        	ecou_buffer = (int *) malloc (numtasks * numtasks * sizeof (int));
        	for (i = 0 ; i < numtasks ; i++) {
        		if (matrice_locala_adiacenta[rank * numtasks + i] != -1 && matrice_locala_adiacenta[rank * numtasks + i] != parinte) {
        			MPI_Recv (ecou_buffer , numtasks * numtasks , MPI_INT , matrice_locala_adiacenta[rank * numtasks + i] , MPI_ANY_TAG , MPI_COMM_WORLD , &status);
     				for (j = 0 ; j < numtasks * numtasks ; j++) {
        				if (ecou_buffer[j] != -1) {
        					verif_ecou = 1;
        					mesaj_ecou[j] = ecou_buffer[j];
        				}
        			}
        			if (verif_ecou == 1) {
        				mesaj_ecou[rank * numtasks + i] = i;
        			}
        			verif_ecou = 0;
        		}
        	}
        	for (i = 0 ; i < numtasks * numtasks; i++) {
        		if (matrice_locala_adiacenta[i] != -1) mesaj_ecou[i] = matrice_locala_adiacenta[i];
        	}
          mesaj_ecou[rank * numtasks + numtasks - 1] = parinte;
        	MPI_Isend (mesaj_ecou , numtasks * numtasks , MPI_INT , parinte , 42 , MPI_COMM_WORLD , &request);
        }
        //matrice_locala_adiacenta e defapt arbore minim de acoperire
        MPI_Recv (matrice_locala_adiacenta , numtasks * numtasks , MPI_INT , MPI_ANY_SOURCE, 43 , MPI_COMM_WORLD , &status);
        parinte = status.MPI_SOURCE;
        
        //calculez tabela de rutare si pentru noduri
        
        int tabela_rutare[numtasks*numtasks][numtasks*numtasks];      
        for (i = 0 ; i < numtasks * numtasks; i++) {
          for (j = 0 ; j < 3 ; j++) {
              tabela_rutare[i][j] = -1;
          }
        }

        for (i = 0 ; i < numtasks ; i++) {
          for (j = 0 ; j < numtasks ; j++) {
            tabela_rutare[i*numtasks + j][0] = i;
            tabela_rutare[i*numtasks + j][1] = j;
          if (i == j) {
            tabela_rutare[i*numtasks + j][2] = i;
          }
          else {
            //sursa e i , destinatia e j
            int ok = 0 , nod_curent = i , facem_negru = 1;
            int culori[numtasks]; //0 e alb , 1 , e gri , 2 e negru
            for (k = 0 ; k < numtasks ; k++) {
              culori[k] = 0;
            }
            while (ok == 0) {
              culori[nod_curent] = 1;
              facem_negru = 1;
              for (k = 0 ; k < numtasks ; k++) {
                if (matrice_locala_adiacenta[nod_curent*numtasks + k] == 0) {
                  if (k == j) {
                    tabela_rutare[i*numtasks + j][2] = nod_curent;
                    ok = 1;
                    break;
                  }
                  else if (culori[k] == 0) {
                    nod_curent = k;
                    facem_negru = 0;
                    break;
                  }
                }
              }
              if (facem_negru == 1) {
                culori[nod_curent] = 2;
                for (k = 0 ; k < numtasks ; k++) {
                  if (matrice_locala_adiacenta[nod_curent*numtasks + k] == 0) {
                   if (culori[k] == 1) {
                    nod_curent = k;
                    break;
                   } 
                  }
                }
              }  
            }

          }
        }
      }

        FILE * fpout;
        //fiecare buncar are propriul fisier in care afiseaza
        fpout = fopen ("output_etapa_1" , "a");
	      fprintf(fpout, "\n");
        fprintf(fpout, "%d\n", rank); 
        for (i = 0 ; i < numtasks * numtasks; i++) {
          if (tabela_rutare[i][0] != -1) {
            if (tabela_rutare[i][1] == rank){
              fprintf(fpout, "%d ", tabela_rutare[i][2]);
              fprintf(fpout, "%d ", tabela_rutare[i][0]);
              fprintf(fpout , "\n");
            }
          }
        }
        
        fclose (fpout);
        if (line)
       	    free(line);
        for (i = 0 ; i < numtasks ; i++) {
          if (matrice_locala_adiacenta[rank * numtasks + i] == 0 &&  i != parinte) {
            MPI_Isend (matrice_locala_adiacenta , numtasks * numtasks , MPI_INT , i , 43 , MPI_COMM_WORLD , &request);
          }
        }
        //AM TERMINAT ETAPA 1 , incepe ETAPA 2
        //INCEPE ETAPA 2
        Mesaj_prim broadcast;
        MPI_Recv (&broadcast , 1 , structura_mesaj , MPI_ANY_SOURCE , 17 , MPI_COMM_WORLD , &status);
        parinte = status.MPI_SOURCE;
        
        FILE *fpin;
        fpin = fopen ("mesaje_input_etapa_2" , "r");
        int numar_mesaje;
        line = NULL;
        len = 0;
        int ret = fscanf (fpin , "%d" , &numar_mesaje);
        if (ret < 0)
            printf("Fisierul de input pt etapa 2 e corupt sau gol\n");
        Mesaj_prim messages[numar_mesaje];
        i=0;
        fseek (fpin , 2 , SEEK_SET);
        while ((read = getline(&line, &len, fpin)) > 4) {
            char * char_buff;
            char * pointer = line;
            while (*pointer != ' '){
              pointer++;
            }
            pointer++;
            while (*pointer != ' '){
            pointer++;
            }
            pointer++;
            strcpy(messages[i].info , pointer);
            char_buff = (strtok (line , " "));
            messages[i].sursa = atoi(char_buff);
            char_buff = (strtok (NULL , " "));
            if (strcmp (char_buff , "B") == 0) {
              messages[i].destinatie = -1;
            }
            else {
              messages[i].destinatie = atoi (char_buff);
            }
            i++;
        }
        if (line)
              free(line);
        fclose(fpin);
        
        //dam mesajul de broadcast de la initiator mai departe pt a semnala inceputul etapei
        for (i = 0 ; i < numtasks ; i++){
          if (matrice_locala_adiacenta[rank * numtasks + i] == 0 && i != parinte)
            MPI_Isend (&broadcast , 1 , structura_mesaj , i , 17 , MPI_COMM_WORLD , &request);
        }
        FILE * fpcolectiv2;
        fpcolectiv2 = fopen ("output_etapa_2" , "a");
        //acum incepe etapa in care fiecare proces verifica daca are un mesaj de transmis si il transmite
        //apoi urmeaza while de listening 
        //apoi trimiterea mesajului de broadcast pt a specifica ca s-a terminat etapa din punctul de vedere al procesului
        //daca un proces vede ca a primit broadcast de la toate celelalte noduri , atunci stie ca poate sa se opreasca si el
        for (i = 0 ; i < numar_mesaje ; i++) {
          if (messages[i].sursa == rank && messages[i].destinatie != -1) {
            for (j = 0 ; j < numtasks * numtasks; j++) {
              if (tabela_rutare[j][0] != -1) {
                if (tabela_rutare[j][1] == rank){
                  if (tabela_rutare[j][0] == messages[i].destinatie) {
                    messages[i].parinte = rank;
                    MPI_Isend (&messages[i] , 1, structura_mesaj , tabela_rutare[j][2] , j + 180 , MPI_COMM_WORLD , &request);
                    break;
                  }
                }
              }
            } 
          }
          else {
            for (j = 0 ; j < numtasks ; j++) {
              //atentie aici , ca trimit la ABSOLUT toti vecinii , nu iert pe cineva , nu am parinte
              if (matrice_locala_adiacenta[rank * numtasks + j] == 0) {
                messages[i].parinte = rank;
                MPI_Isend (&messages[i] , 1 , structura_mesaj , j , j + 210 , MPI_COMM_WORLD , &request);
              }
            }
          }
        }
        int conditie_ascultare = 1 , timeout = numtasks * numar_mesaje * 0.02;
        //daca se depaseste timeout-ul si nu a primit un mesaj inseamna ca trebuie sa incheie
        while (conditie_ascultare == 1) {
          Mesaj_prim new_message;
          new_message.sursa = -2;
          new_message.destinatie = -2;
          conditie_ascultare = 0;
          for (i = 0 ; i < numtasks ; i++) {
            MPI_Irecv (&new_message , 1 , structura_mesaj , MPI_ANY_SOURCE , MPI_ANY_
            parinte = new_message.parinte;   
            new_message.parinte = rank;
            if (new_message.sursa != -2 && new_message.destinatie != -2) {
              conditie_ascultare = 1;
              if (new_message.destinatie == -1) {
                for (j = 0; j < numtasks ; j++) {
                  if (matrice_locala_adiacenta[rank*numtasks + j] == 0 && j != parinte) {
                    fprintf(fpcolectiv2, "sunt nodul %d mesaj de broadcast de la %d\n", rank , parinte);
                    MPI_Isend (&new_message , 1 , structura_mesaj , j , j + 110 , MPI_COMM_WORLD , &request);
                  }
                }
              }
              else if (new_message.sursa == rank) {
                fprintf (fpcolectiv2 , "sunt nodul %d \nam primit de la %d mesaj cu sursa %d\n iar mesajul este:\n%s\n",
                                                      rank , parinte , new_message.sursa , new_message.info);
              }
              else {
                //daca nu e broadcast folosesc tabela de rutare sa stiu sa trimit direct la nodul bun
                for (j = 0 ; j < numtasks * numtasks ; j++) {
                  if (tabela_rutare[j][1] == rank && tabela_rutare[j][0] == new_message.destinatie) {
                    fprintf (fpcolectiv2 , "sunt nodul %d \nam primit de la %d si trimit catre %d mesajul:\n%s\n", 
                                    rank , new_message.sursa , tabela_rutare[j][2] , new_message.info);
                    MPI_Isend(&new_message , 1, structura_mesaj , tabela_rutare[j][2] , j + 300 , MPI_COMM_WORLD , &request);
                  }
                }
              }
            }
          }
          //daca a ramas 0 , pun wait(timeout) si mai ascult inca o data
          if (conditie_ascultare == 0) {
            sleep(timeout);
            for (i = 0 ; i < numtasks ; i++) {
            MPI_Irecv (&new_message , 1 , structura_mesaj , MPI_ANY_SOURCE , MPI_ANY_TAG , MPI_COMM_WORLD , &request);
            parinte = new_message.parinte;
            new_message.parinte = rank;   
            if (new_message.sursa != -2 && new_message.destinatie != -2) {
              conditie_ascultare = 1;
              if (new_message.destinatie == -1) {
                for (j = 0; j < numtasks ; j++) {
                  if (matrice_locala_adiacenta[rank*numtasks + j] == 0 && j != parinte) {
                    fprintf(fpcolectiv2, "sunt nodul %d mesaj de broadcast de la %d\n", rank , parinte);
                    MPI_Isend (&new_message , 1 , structura_mesaj , j , j + 110 , MPI_COMM_WORLD , &request);
                  }
                }
              }
              else if (new_message.sursa == rank) {
                fprintf (fpcolectiv2 , "sunt nodul %d \nam primit de la %d mesaj cu sursa %d\n iar mesajul este:\n%s\n",
                                                      rank , parinte , new_message.sursa , new_message.info);
              }
              else {
                //daca nu e broadcast folosesc tabela de rutare sa stiu sa trimit direct la nodul bun
                for (j = 0 ; j < numtasks * numtasks ; j++) {
                  if (tabela_rutare[j][1] == rank && tabela_rutare[j][0] == new_message.destinatie) {
                    fprintf (fpcolectiv2 , "sunt nodul %d \nam primit de la %d si trimit catre %d mesajul:\n%s\n", 
                                    rank , new_message.sursa , tabela_rutare[j][2] , new_message.info);
                    MPI_Isend(&new_message , 1, structura_mesaj , tabela_rutare[j][2] , j + 300 , MPI_COMM_WORLD , &request);
                  }
                }
              }
            }
          }
          }
        }
        //acum ca a trecut timeout-ul si nu am primit mesaje , e timpul sa dau broadcast 
        Mesaj_prim broadcast_final;
        broadcast_final.sursa = rank;
        broadcast_final.destinatie = -1;
        broadcast_final.parinte = rank;
        strcpy (broadcast.info , "Incheiati etapa 2");
              
        for (i = 0 ; i < numtasks ; i++) {
          if (i != rank && matrice_locala_adiacenta[rank * numtasks + i] == 0)
            MPI_Isend (&broadcast_final , 1 , structura_mesaj , i , j + 400 , MPI_COMM_WORLD , &request);
        }
    }

   	MPI_Finalize();
}
