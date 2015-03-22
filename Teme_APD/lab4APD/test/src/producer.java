import java.util.Vector;
import java.util.logging.Level;
import java.util.logging.Logger;

public class producer {

    public static void main(String args[]) {
        Vector sharedQueue = new Vector();	//bufferul
        int size = 4;
        Thread prodThread = new Thread(new Producer(sharedQueue, size), "Producatorul");
        Thread consThread = new Thread(new Consumer(sharedQueue, size), "Consumatorul");
        prodThread.start();
        consThread.start();
    }
}

class Producer implements Runnable {

    private final Vector sharedQueue;
    private final int SIZE;

    public Producer(Vector sharedQueue, int size) {
        this.sharedQueue = sharedQueue;
        this.SIZE = size;
    }

    @Override
    public void run() {
        for (int i = 0; i < 10; i++) {
            System.out.println("s-a produs: " + i);
            try {
                produce(i);
            } catch (InterruptedException ex) {
               
            }

        }
    }

    private void produce(int i) throws InterruptedException {

        //asteapta daca bufferul e plin
    	synchronized (sharedQueue) {
    		if (sharedQueue.size() == SIZE) {
            
                System.out.println("bufferul e plin " + Thread.currentThread().getName()
                                    + " asteapta , poz: " + sharedQueue.size());

                sharedQueue.wait();
            }
        

        //produc un nou element si notific consumator
            sharedQueue.add(i);
            sharedQueue.notifyAll();
        }
    }
}

class Consumer implements Runnable {

    private final Vector sharedQueue;
    private final int SIZE;

    public Consumer(Vector sharedQueue, int size) {
        this.sharedQueue = sharedQueue;
        this.SIZE = size;
    }

    @Override
    public void run() {
        while (true) {
            try {
                System.out.println("s-a consumat: " + consume());
                Thread.sleep(50);	//daca nu fac sleep , se vor amesteca threadurile 
                					//si bufferul va fi si citit si scris in acelasi timp
            } catch (InterruptedException ex) {
             
            }

        }
    }

    private int consume() throws InterruptedException {
        //asteapta daca bufferul e gol
    	synchronized (sharedQueue) {
    		if (sharedQueue.isEmpty()) {
            
                System.out.println("bufferul e gol " + Thread.currentThread().getName()
                                    + " asteapta , poz: " + sharedQueue.size());

                sharedQueue.wait();
            }
        
            sharedQueue.notifyAll();
            return (Integer) sharedQueue.remove(0);
        }
    }
}
