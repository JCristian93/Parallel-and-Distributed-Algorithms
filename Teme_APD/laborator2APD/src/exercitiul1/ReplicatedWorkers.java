package exercitiul1;

import java.io.File;
import java.io.FileNotFoundException;
import java.util.ArrayList;
import java.util.Scanner;

/**
 * Clasa ce reprezinta o solutie partiala pentru problema de rezolvat. Aceste
 * solutii partiale constituie task-uri care sunt introduse in workpool.
 */
class PartialSolution {
	
	private ArrayList<String> sir;
	private String deCautat;
	
	
	public PartialSolution(ArrayList<String> sir, String deCautat) {
		this.sir = sir;
		this.deCautat = deCautat;
	}
	
	
	public boolean process() {
		return sir.get(0).equals(deCautat);
	}
	
	
	public int numberOfWords() {
		return sir.size();
	}
	
	
	public String toString() {
		return sir.size() + "";
	}
	
	
	public ArrayList<String> getSir() {
		return sir;
	}
	
	public String getDeCautat() {
		return deCautat;
	}
	
}

/**
 * Clasa ce reprezinta un thread worker.
 */
class Worker extends Thread {
	WorkPool wp;
	int counter;

	public Worker(WorkPool workpool) {
		this.wp = workpool;
	}

	/**
	 * Procesarea unei solutii partiale. Aceasta poate implica generarea unor
	 * noi solutii partiale care se adauga in workpool folosind putWork().
	 * Daca s-a ajuns la o solutie finala, aceasta va fi afisata.
	 */
	void processPartialSolution(PartialSolution ps) {
		if (ps.numberOfWords() == 1) {
			if (ps.process() == true)
				counter ++;
		}
		else {
			ArrayList<String> sir = ps.getSir();
			String deCautat = ps.getDeCautat();
			
			ArrayList<String> sirStart = new ArrayList<String>(sir.subList(0, sir.size()/2));
			ArrayList<String> sirEnd = new ArrayList<String>(sir.subList(sir.size()/2, sir.size()));
			
			wp.putWork(new PartialSolution(sirStart, deCautat));
			wp.putWork(new PartialSolution(sirEnd, deCautat));
		}
	}
	
	public void run() {
		System.out.println("Thread-ul worker " + this.getName() + " a pornit...");
		while (true) {
			PartialSolution ps = wp.getWork();
			if (ps == null)
				break;
			
			processPartialSolution(ps);
		}
		System.out.println("Thread-ul worker " + this.getName() + " s-a terminat...");
	}

	
}


public class ReplicatedWorkers {
	
	public static void main(String args[]) {
		int cores = Runtime.getRuntime().availableProcessors();
		ArrayList<String> sir = new ArrayList<String>();
		
		//initializare workeri si workpool
		WorkPool wp = new WorkPool(cores);
		Worker[] workers = new Worker[cores];
		for (int i = 0; i < cores; i ++)
			workers[i] = new Worker(wp);
		
		//citire text din fisier
		try {
			Scanner in = new Scanner(new File("test.txt"));
			
			while (in.hasNext())
				sir.add(in.next());
			
			in.close();
		}
		catch (FileNotFoundException e) {
			e.printStackTrace();
		}
		
		PartialSolution initial = new PartialSolution(sir, "the");
		wp.putWork(initial);
		
		
		for (int i = 0; i < cores; i ++)
			workers[i].start();
		
		for (int i = 0; i < cores; i ++)
			try {
				workers[i].join();
			}
			catch (InterruptedException e) {
				e.printStackTrace();
			}
		
		
		int counter = 0;
		for (int i = 0; i < cores; i ++)
			counter += workers[i].counter;
		
		System.out.println("Numarul de aparitii gasite: " + counter);
	}
	
}


