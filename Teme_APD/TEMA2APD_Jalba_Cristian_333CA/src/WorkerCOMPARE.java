import java.io.FileNotFoundException;
import java.io.IOException;
import java.io.RandomAccessFile;
import java.math.BigDecimal;
import java.text.DecimalFormat;
import java.util.ArrayList;
import java.util.Collections;
import java.util.Comparator;
import java.util.HashMap;
import java.util.LinkedList;
import java.util.List;
import java.util.RandomAccess;
import java.util.StringTokenizer;

class Triplet implements Comparable{
	String doc1;
	String doc2;
	Double grad_similiritate;
	static double epsilon = 0.0001;
	public Triplet (String doc1 , String doc2 , Double grad_similiritate) {
		this.grad_similiritate = grad_similiritate;
		this.doc1 = doc1;
		this.doc2 = doc2;
	}
	public String getNumeDoc1 (){
		return doc1;
	}
	public String getNumeDoc2 (){
		return doc2;
	}
	public double getXFactorSimilitudine(){
		return grad_similiritate;
	}
	public int compareTo(Double grad_similiritate) {
		// TODO Auto-generated method stub
		if ((this.grad_similiritate - grad_similiritate) > 0)	return -1;
		else if (Math.abs(this.grad_similiritate - grad_similiritate) < epsilon){
			return 0;
		}
		else {
			return 1;
		}

	}
	@Override
	public int compareTo(Object arg0) {
		// TODO Auto-generated method stub
		return 0;
	}	
}

class clasaLoadStoreLista {
	static LinkedList<Triplet> lista_triplete = new LinkedList<Triplet>();
	static LinkedList<Double> vector_similitudini = new LinkedList<Double>(); 
	
	public static synchronized void adaugare_lista_finala (Double grad_similaritate , PartialSolution ps) {
		clasaLoadStoreLista.lista_triplete.add(new Triplet(ps.nume_doc1, ps.nume_doc2, grad_similaritate));
	}
	//metoda folosita doar pt debugging
	public static void nr_total_cuv_din_text (String filemname) throws IOException {
		RandomAccessFile fisier = new RandomAccessFile(filemname, "r");
		long size_fisier = fisier.length();
		byte[] vector_cuvinte = new byte[(int)size_fisier];
		fisier.read(vector_cuvinte, 0, (int)size_fisier);
		String delims =  ";:_/?~\\.,><~`[]{}()!@#$%^&-+'=*\"| \t\n";
		StringTokenizer toneker = new StringTokenizer(new String(vector_cuvinte), delims);
		System.out.println("nr total de cuvinte este: " + toneker.countTokens() + " pt fisierul " + filemname);
	}
}

public class WorkerCOMPARE extends Thread {
	WorkPool wp;
	
	static public void afisare_lista_triplete (String nume_fisier_scriere) throws IOException {
		RandomAccessFile fisier_scriere = null ;
		try {
			fisier_scriere = new RandomAccessFile (nume_fisier_scriere , "rw");
		}
		catch (FileNotFoundException e){
			e.printStackTrace();
		}
		for (Triplet tuplu : clasaLoadStoreLista.lista_triplete) {
			DecimalFormat df = new DecimalFormat();
			df.setMaximumFractionDigits(4);
			fisier_scriere.write((tuplu.doc1 +";"+ tuplu.doc2 +";"+ df.format(tuplu.grad_similiritate) + "\n").getBytes());
		}
	}
	
	public static void sortListaTriplete () {
		
		Collections.sort(clasaLoadStoreLista.lista_triplete, new Comparator<Triplet>() {
	        @Override
	        public int compare(Triplet  tuplu1, Triplet  tuplu2) {
	        	if (tuplu1.grad_similiritate.compareTo(tuplu2.grad_similiritate) == 0){
	            	if (tuplu1.doc1.compareTo(tuplu2.doc1) == 0){
	            		return tuplu1.doc2.compareTo(tuplu2.doc2);
	            	}
	            	return tuplu1.doc1.compareTo(tuplu2.doc1);
	            }
	            else return  -tuplu1.grad_similiritate.compareTo(tuplu2.grad_similiritate);
	        }
	    });
		
	}
	
	public double numara_cuvinte_din_document (HashMap<String, Integer> document) {
		double nr_cuvinte = 0;
		for (String key : document.keySet()) {
			nr_cuvinte += document.get(key);
		}
		return nr_cuvinte;
	}
	
	public double numara_aparitie_cuvant_in_doc (HashMap<String, Integer> document , String cuvant){
		if (document.containsKey(cuvant)){
			return document.get(cuvant);
		}
		else return 0;
	}
	
	public WorkerCOMPARE(WorkPool workpool) {
		this.wp = workpool;		
	}

	void processPartialSolution(PartialSolution ps) throws IOException {
	
		double grad_similaritate = 0;
		for (String cuvant1 : ps.getDoc1().keySet()){
			if (ps.getDoc2().containsKey(cuvant1)) {
				grad_similaritate += ( (numara_aparitie_cuvant_in_doc(ps.getDoc1(), cuvant1) / 
						numara_cuvinte_din_document(ps.getDoc1())) ) *
							(numara_aparitie_cuvant_in_doc(ps.getDoc2(), cuvant1) / 
								numara_cuvinte_din_document(ps.getDoc2()));
			}
		}
		grad_similaritate *= 100;
		clasaLoadStoreLista.adaugare_lista_finala(grad_similaritate, ps);
	}
	public void run() {
		System.out.println("Thread-ul worker COMPARE" + this.getName() + " a pornit...");
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
		System.out.println("Thread-ul worker COMPARE" + this.getName() + " s-a terminat...");
	}

}