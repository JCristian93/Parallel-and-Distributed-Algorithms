package exercitiul2;

import java.util.ArrayList;

/**
 * Clasa ce reprezinta o solutie partiala pentru problema de rezolvat. Aceste
 * solutii partiale constituie task-uri care sunt introduse in workpool.
 */
class PartialSolution {
	
	private ArrayList<Integer> vector;
	private int counter;
	
	
	public PartialSolution(ArrayList<Integer> vector, int counter) {
		this.vector = vector;
		this.counter = counter;
	}
	
	
	public boolean process(int size) {
		for (int i = 0; i < size - 1; i ++)
			for (int j = i + 1; j < size; j ++) {
				if (vector.get(i) == vector.get(j))
					return false;
		
				if (Math.abs(vector.get(i) - vector.get(j)) == Math.abs(i - j))
					return false;
			}
			
		return true;
	}
	
	
	public String toString() {
		return vector.toString();
	}
	
	
	public ArrayList<Integer> getVector() {
		return vector;
	}
	
	public int getCounter() {
		return counter;
	}
	
}

/**
 * Clasa ce reprezinta un thread worker.
 */
class Worker extends Thread {
	WorkPool wp;
	int solutii;

	public Worker(WorkPool workpool) {
		this.wp = workpool;
	}

	/**
	 * Procesarea unei solutii partiale. Aceasta poate implica generarea unor
	 * noi solutii partiale care se adauga in workpool folosind putWork().
	 * Daca s-a ajuns la o solutie finala, aceasta va fi afisata.
	 */
	void processPartialSolution(PartialSolution ps) {
		ArrayList<Integer> vector = ps.getVector();
		int counter = ps.getCounter();
		
		
		if (counter < 4 && ps.process(counter)) {
			ArrayList<Integer> newVector0 = new ArrayList<Integer>(vector);
			newVector0.set(counter, 0);
			wp.putWork(new PartialSolution(newVector0, counter + 1));
			
			ArrayList<Integer> newVector1 = new ArrayList<Integer>(vector);
			newVector1.set(counter, 1);
			wp.putWork(new PartialSolution(newVector1, counter + 1));
			
			ArrayList<Integer> newVector2 = new ArrayList<Integer>(vector);
			newVector2.set(counter, 2);
			wp.putWork(new PartialSolution(newVector2, counter + 1));
			
			ArrayList<Integer> newVector3 = new ArrayList<Integer>(vector);
			newVector3.set(counter, 3);
			wp.putWork(new PartialSolution(newVector3, counter + 1));
		}
		else {
			if (ps.process(4) == true) {
				System.out.println("Solutie finala: " + ps.getVector());
				solutii ++;
			}
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
		
		//initializare workeri si workpool
		WorkPool wp = new WorkPool(cores);
		Worker[] workers = new Worker[cores];
		for (int i = 0; i < cores; i ++)
			workers[i] = new Worker(wp);
		
		//generare prima asezare de dame
		ArrayList<Integer> vector = new ArrayList<Integer>();
		vector.add(0);
		vector.add(0);
		vector.add(0);
		vector.add(0);
		
		
		PartialSolution initial = new PartialSolution(vector, 0);
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
			counter += workers[i].solutii;
		
		System.out.println("Numarul de solutii finale gasite: " + counter);
	}
	
}


