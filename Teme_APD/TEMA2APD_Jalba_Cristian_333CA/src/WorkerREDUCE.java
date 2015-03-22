import java.io.IOException;
import java.util.HashMap;
import java.util.Iterator;
import java.util.Map;

class setterREDUCE {
	static public synchronized void adaugare_in_hash_comun (PartialSolution ps , Map.Entry<String, Integer> element_hash) {
		if (WorkerREDUCE.index_cuvinte_total.get(ps.getFileName()).containsKey(element_hash.getKey())){
			WorkerREDUCE.index_cuvinte_total.get(ps.getFileName()).put(element_hash.getKey(), 
				WorkerREDUCE.index_cuvinte_total.get(ps.getFileName()).get(element_hash.getKey()) + element_hash.getValue());
		}
		else {
			//daca nu am inregistrarea cuvantului acela , o adaug
			WorkerREDUCE.index_cuvinte_total.get(ps.getFileName()).put(element_hash.getKey(), element_hash.getValue());
		}
	}
}

public class WorkerREDUCE extends Thread {
	WorkPool wp;

	public static HashMap<String, HashMap<String, Integer>> index_cuvinte_total = 
				new HashMap<String , HashMap<String, Integer>>();
	
	public static void initialize_index_cuvinte_total(String [] nume_fisiere) {
		for (String nume : nume_fisiere) {
			//initializez hash-ul mare sa am pt fiecare fisier ceva
			index_cuvinte_total.put(nume, new HashMap<String , Integer>());
		}
	}
	
	public WorkerREDUCE(WorkPool workpool) {
		this.wp = workpool;		
	}

	public static void afis_hash () {
		for (String nume_fis : index_cuvinte_total.keySet()){
			System.out.println("Afisam ce contine documentul: " + nume_fis);
			for (String key : index_cuvinte_total.get(nume_fis).keySet()){
				System.out.println(key);
				System.out.println(index_cuvinte_total.get(nume_fis).get(key));
			}
		}
	}
	
	public static HashMap<String, HashMap<String, Integer>> getFinalHash () {
		return index_cuvinte_total;
	}
	//adaugam sincronizat in hash-ul comun

	void processPartialSolution(PartialSolution ps) throws IOException {
		//ignor toate campurile mai putin numele fisierului si hashtable-ul
		//creez un iterator pentru hash-ul solutiei partiale
		Iterator <Map.Entry<String,Integer>> iterator =  ps.getHash().entrySet().iterator();	
		while (iterator.hasNext()){
			Map.Entry<String, Integer> element_hash = iterator.next();
			//probabil va fi nevoie sa fac si verificari daca intrarile sunt nule , dar am auzit ca 
			//Hashtable nu are intrari nule , poate nu e cazul
			setterREDUCE.adaugare_in_hash_comun(ps, element_hash);
		}
	}
	
	public void run() {
		System.out.println("Thread-ul worker REDUCE" + this.getName() + " a pornit...");
		while (true) {
			PartialSolution ps = wp.getWork();
			if (ps == null)
				break;
			
			try {
				processPartialSolution(ps);
			} catch (IOException e) {
				e.printStackTrace();
			}
		}
		System.out.println("Thread-ul worker REDUCE" + this.getName() + " s-a terminat...");
	}

}