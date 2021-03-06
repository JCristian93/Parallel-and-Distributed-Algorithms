import java.io.BufferedWriter;
import java.io.FileWriter;
import java.io.IOException;
import java.util.ArrayList;
import java.util.List;


public class lab4ex1 extends Thread implements Runnable {
	
	int [] vectoras;
	static int cores;
	static List<Thread> threads;
	private static lab4ex1 obj;
	public lab4ex1 () {
		vectoras = new int[100000];
	}
	public static void main(String[] args) throws InterruptedException {
		cores = Runtime.getRuntime().availableProcessors();
		lab4ex1 obj = new lab4ex1();
		threads = new ArrayList<Thread>();	//lista de threads
		for (int i = 0 ; i < cores ; i++) {
			Thread t = new Thread(obj);
			t.start();
			threads.add(t);
		}
		for (int i = 0; i < threads.size(); i++)
	    {
	        ((Thread)threads.get(i)).join();
	    }
		obj.afisare_vectoras();
	}
	@Override
	public void run() {
		// TODO Auto-generated method stub
		Thread t = Thread.currentThread();
		for (int i = (int) (t.getId()%cores) ; i < 100000 ; i+= cores) 
			vectoras[i] = i * i;
	}
	public void afisare_vectoras() {
		for (int j = 0 ; j < 100 ; j++) {
			System.out.println(vectoras[j] + " ");
		}
	}
}
