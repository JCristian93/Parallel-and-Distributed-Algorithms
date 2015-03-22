import java.awt.font.NumericShaper;
import java.io.File;
import java.io.FileNotFoundException;
import java.io.IOException;
import java.io.RandomAccessFile;
import java.util.HashMap;
import java.util.Hashtable;

/**
 * Clasa ce reprezinta o solutie partiala pentru problema de rezolvat. Aceste
 * solutii partiale constituie task-uri care sunt introduse in workpool.
 */
class PartialSolution {
	int start;
	int finish;
	int dim;
	String nume_file;
	//astea contin munca unui workerMAP pe un fragment si va fi stocata in REDUCE
	HashMap<String, Integer> index_partial_cuvinte = new HashMap<String , Integer>();
	
	//asteam 2 sunt hash-urile ce vor fi folosite in COMPARE .
	//fiecare va contine toate cuvintele din documentul respectiv , iar in COMPARE se va uita la numarul de a
	//aparitii al fiecarui cuvant 
	HashMap<String, Integer> document1 = new HashMap<String , Integer>();
	HashMap<String, Integer> document2 = new HashMap<String , Integer>();
	String nume_doc1 ,nume_doc2;
	public String toString() {
		return nume_file;
	}
	
	public PartialSolution(int start , int dim ,int finish, String nume_file, HashMap<String, Integer> index_partial_cuvinte){
		this.start = start;
		this.dim = dim;
		this.nume_file = nume_file;
		this.finish = finish;
		this.index_partial_cuvinte = index_partial_cuvinte;
	}	
	public PartialSolution(HashMap<String, Integer> document1 ,HashMap<String, Integer> document2 ,
								String nume_doc1, String nume_doc2){
		this.start = -2;
		this.dim = -2;
		this.nume_file = "APD";
		this.finish = -2;
		this.document1 = document1;
		this.document2 = document2;
		this.nume_doc1 = nume_doc1;
		this.nume_doc2 = nume_doc2;
	}
	public HashMap<String, Integer> getHash () {
		return index_partial_cuvinte;
	}
	public HashMap<String, Integer> getDoc1 () {
		return this.document1;
	}
	public HashMap<String, Integer> getDoc2 () {
		return this.document2;
	}
	public String getFileName () {
		return this.nume_file;
	}
}

public class ReplicatedWorkers {

	public static void main(String args[]) throws IOException, InterruptedException {
		int nr_threads = Integer.parseInt(args[0]);	

		int dim_fragmente = 0 , numar_documente = 0;
		double prag_sim;
		String [] nume_fisiere = null;
		//fac aici o citire dupa care in workpoolMAP fac citirile de fragmente
		RandomAccessFile fisier_citire = new RandomAccessFile(args[1], "r");
		fisier_citire.seek(0);
		try {
			dim_fragmente = Integer.parseInt(fisier_citire.readLine());
			prag_sim = Double.parseDouble(fisier_citire.readLine());
			numar_documente = Integer.parseInt(fisier_citire.readLine());
			nume_fisiere = new String[numar_documente];
			for (int i = 0 ; i < numar_documente ; i++) {
				nume_fisiere[i] = fisier_citire.readLine();
			}
			
		}catch (FileNotFoundException e){
			e.printStackTrace();
		}		
		fisier_citire.close();
		//CELE 3 WORKPOOLS in care bag diverse task-uri
		WorkPool workpoolMAP = new WorkPool(nr_threads);
		WorkPool workpoolREDUCE = new WorkPool(nr_threads);
		WorkPool workpoolCOMPARE = new WorkPool(nr_threads);
		int l = 0;
		{
			File file = new File(nume_fisiere[l]);
			RandomAccessFile tester = new RandomAccessFile(nume_fisiere[l],"r");
			//adaugam taskuri in workpooluri
			System.out.println("DIM: " + file.length() + " " + dim_fragmente);
			for (int j = 0 ; j < numar_documente ; j++) {	
				int i;
				for (i = 0 ; i < file.length()/dim_fragmente ; i++){
					PartialSolution ps = new PartialSolution(i*dim_fragmente, dim_fragmente,
						(i+1)*dim_fragmente -1, nume_fisiere[j], null);
					workpoolMAP.putWork(ps);
				}
				if (i*dim_fragmente != (int)(new File(nume_fisiere[j])).length()) {
					PartialSolution psfinal;
					if (i*dim_fragmente < (int)(new File(nume_fisiere[j])).length() )	{
						psfinal = new PartialSolution((i)*dim_fragmente, dim_fragmente,
							(int)(new File(nume_fisiere[j])).length() -1, nume_fisiere[j], null);
					}
					else psfinal = new PartialSolution((i)*dim_fragmente, dim_fragmente,
						(int)(new File(nume_fisiere[j])).length() , nume_fisiere[j], null); 
				}
			}
			WorkerMAP[] peon = new WorkerMAP[nr_threads]; 
			for (int i = 0 ; i < nr_threads ; i++) {
				peon[i] = new WorkerMAP(workpoolMAP, nume_fisiere);
				peon[i].start();
			}
			try {
				for (int i = 0 ; i < nr_threads ; i++) {
					peon[i].join();	
				}
			}
			catch (InterruptedException e) {
				e.printStackTrace();
			}
			for (int i = 0 ; i < nr_threads ; i++){
				for (int j = 0 ; j < numar_documente ; j++){
					//punem toate hash-urile adunate de peoni in workpool-ul pt overseeri
					workpoolREDUCE.putWork(new PartialSolution(-1, -1, -1, nume_fisiere[j] ,
							peon[i].getHash(nume_fisiere[j])));
				}
			}
			//e esential sa initializez hash-ul final inainte de a incepe sa lucrez cu el
			WorkerREDUCE.initialize_index_cuvinte_total(nume_fisiere);
			WorkerREDUCE[] overseer = new WorkerREDUCE[nr_threads];
			for (int i = 0 ; i < nr_threads ; i++){
				overseer[i] = new WorkerREDUCE(workpoolREDUCE);
				overseer[i].start();
			}
			for (int i = 0 ; i < nr_threads ; i++) {
					
				try {
					overseer[i].join();								
				}
				catch (InterruptedException e) {
					e.printStackTrace();
				}
			}
			for (int i = 0 ; i < numar_documente ; i++){
				for (int j = i + 1 ; j < numar_documente ; j++){
						workpoolCOMPARE.putWork(new PartialSolution(WorkerREDUCE.getFinalHash().get(nume_fisiere[i]),
							WorkerREDUCE.getFinalHash().get(nume_fisiere[j]) , nume_fisiere[i] , nume_fisiere[j]));
				}
			}
			
			WorkerCOMPARE[] master = new WorkerCOMPARE[nr_threads];
			for (int i = 0 ; i < nr_threads ; i++) {
				master[i] = new WorkerCOMPARE(workpoolCOMPARE);
				master[i].start();
			}

			for (int i = 0 ; i < nr_threads ; i++) {
				
				try {
					master[i].join();
				}
				catch (InterruptedException e) {
					e.printStackTrace();
				}
			}
			WorkerCOMPARE.sortListaTriplete();
			WorkerCOMPARE.afisare_lista_triplete(args[2]);		
		}
	}
}
	