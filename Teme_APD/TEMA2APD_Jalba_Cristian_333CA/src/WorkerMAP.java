
import java.io.IOException;
import java.io.RandomAccessFile;
import java.util.HashMap;


public class WorkerMAP extends Thread {
	WorkPool wp;
	HashMap<String, HashMap<String, Integer>> index_cuvinte = new HashMap<String , HashMap<String, Integer>>();
	static char [] delimitatori = ";:_/?~\\.,><~`[]{}()!@#$%^&-+'=*\"| \t\n".toCharArray();
	public WorkerMAP(WorkPool workpool , String[] nume_fisiere) {
		this.wp = workpool;
		for (String nume : nume_fisiere) {
			index_cuvinte.put(nume, new HashMap<String , Integer>());
		}
	}
	public HashMap<String, Integer> getHash (String nume_hash) {
		return index_cuvinte.get(nume_hash);
	}
	static public boolean isDelim (char caracter) {
		
		for (char t : delimitatori) {
			if (t == caracter) return true;
		}
		return false;
	}
	/**
	 * Procesarea unei solutii partiale. Aceasta poate implica generarea unor
	 * noi solutii partiale care se adauga in workpool folosind putWork().
	 * Daca s-a ajuns la o solutie finala, aceasta va fi afisata.
	 * @throws IOException 
	 */
	void processPartialSolution(PartialSolution ps) throws IOException {
		//fac citirea din fisier
		try {	
			RandomAccessFile fisier = new RandomAccessFile(ps.nume_file, "r");
			fisier.seek(ps.start);	
			if (ps.start < fisier.length()) {
			byte character_buf = fisier.readByte();	
			byte [] cuvant = new byte[10000000];
			boolean ok = true , termina_cuvant = false;
			fisier.seek(ps.start);
			
			if(ps.start == 0);
			else
			{
				if( isDelim((char) (character_buf & 0xFF))) {
					fisier.seek(ps.start);
				}
				else {
					fisier.seek(ps.start -1);
					character_buf = fisier.readByte();
					if( isDelim((char) (character_buf & 0xFF)) ) {
						fisier.seek(ps.start);
					}
					else {
						while( !isDelim((char) (character_buf & 0xFF)) ) {
							character_buf = fisier.readByte();
							
						}
						fisier.seek( fisier.getFilePointer() - 1 );
					}
				}
			}
			//System.out.println(ps.start);
			if (ps.start >= ps.finish) ok = false;
			int contor = 0;
			char caracter_aux;
			
			contor = -1;
			
			fisier.seek(ps.finish);
			if (fisier.getFilePointer() < fisier.length()) character_buf = fisier.readByte();
			while (!isDelim((char)(character_buf & 0xFF)) && fisier.getFilePointer() < fisier.length()) {
				character_buf = fisier.readByte();
			}
			long save_the_poor_pointer = fisier.getFilePointer() - 1;	
			fisier.seek(ps.start);
			cuvant = new byte [(int) (save_the_poor_pointer - ps.start)];
			fisier.read(cuvant);
			for (int i = 0 ; i < cuvant.length ; i++){
				if (isDelim((char)(cuvant[i] & 0xFF))){
					cuvant[i] = ' ';
				}
			}
			String frag = new String (cuvant);
			frag = frag.toLowerCase();								
			String[] multe_cuvinte;
			multe_cuvinte = frag.split(" ");
			
			for(String key : multe_cuvinte) {
				if (key.length() > 0) {
					if(index_cuvinte.containsKey(ps.nume_file)) {
						if(index_cuvinte.get(ps.nume_file).containsKey(key))
							index_cuvinte.get(ps.nume_file).put(key, index_cuvinte.get(ps.nume_file).get(key) + 1);
						else
							index_cuvinte.get(ps.nume_file).put(key, 1);
					}
					else {
						index_cuvinte.get(ps.nume_file).put(key, 1);
					}
				}
			}
		}}	
		catch (IOException e) {
			e.printStackTrace();
		}
	}
	
	public void run() {
		System.out.println("Thread-ul worker MAP" + this.getName() + " a pornit...");
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
		System.out.println("Thread-ul worker MAP" + this.getName() + " s-a terminat...");
	}

	
}