//CIOTEC MARIAN-STEFAN 333CA

import java.io.*;
import java.util.*;

class FileStatistics {
	//hastable care contine aocieri de tipul <cuvant, numar aparitii>
	Hashtable<String, Integer> file_hash;
	//numarul de cuvinte din fisier
	int no_of_words;
	
	public FileStatistics() {
		file_hash = new Hashtable<String, Integer>();
		no_of_words = 0;
	}
}

class FileInfo implements Comparable<FileInfo> {
	String name;
	double similarity;
	
	public FileInfo(String name, double similarity) {
		this.name = name;
		this.similarity = similarity;
	}
	public int compareTo(FileInfo compareFileInfo) {
		double sim = compareFileInfo.similarity;
		if ((sim - this.similarity) > 0) {
			return 1;
		}
		else if((sim - this.similarity) < 0) {
			return -1;
		}
		return 0;
	}
}
interface PartialSolution {}

class PartialReduceSolution implements PartialSolution {
	//fisierul de verificat importriva plagiatului
	String plagiat_file;
	//identificatorul fisierului de verificat impotriva plagiatului
	int id_plagiat_file;
	//fisierul cu care se verifica
	String verify_file;
	//identificatorul fisierului cu care se verifica
	int id_verify_file;
	
	public PartialReduceSolution(String plagiat_file, int id_plagiat_file, String verify_file, int id_verify_file) {
		this.plagiat_file = plagiat_file;
		this.id_plagiat_file = id_plagiat_file;
		this.verify_file = verify_file;
		this.id_verify_file = id_verify_file;	
	}
}

class PartialMapSolution implements PartialSolution {
	//fisierul din care thread-ul va citi
	String file_name;
	//identificatorul fisierului
	int file_number;
	//pozitia din care incepe sa citeasca
	long start_pos;
	//pozitia pana unde citeste
	long finish_pos;
	
	public PartialMapSolution(String file_name, int file_number, long start_pos, long finish_pos) {
		this.file_name = file_name;
		this.file_number = file_number;
		this.start_pos = start_pos;
		this.finish_pos = finish_pos;
	}
	public String getFile() {
		return file_name;
	}
	public long getStartPos() {
		return start_pos;
	}
	public long getFinishPos() {
		return finish_pos;
	}
	public int getNumber() {
		return file_number;
	}
}

/**
 * Clasa ce reprezinta un thread worker.
 */
class Worker extends Thread {
	WorkPool wp;

	public Worker(WorkPool workpool) {
		this.wp = workpool;
	}

	/**
	 * Procesarea unei solutii partiale. Aceasta poate implica generarea unor
	 * noi solutii partiale care se adauga in workpool folosind putWork().
	 * Daca s-a ajuns la o solutie finala, aceasta va fi afisata.
	 * @throws IOException 
	 */
	void processPartialSolution(PartialSolution ps) throws IOException {
		//daca are loc procesarea pt map
		if(ps instanceof PartialMapSolution) {
			byte[] data;
			byte b;
			RandomAccessFile file = new RandomAccessFile(((PartialMapSolution) ps).getFile(), "r");
			long start = ((PartialMapSolution) ps).getStartPos();
			long end = ((PartialMapSolution) ps).getFinishPos();
			//daca nu este pozitia de start in fisier
			if(start != 0) {
				//ma pozitionez inainte cu un caracter
				start -= 1;
				//fragmentul prelucrat incepe in mijlocul unui cuvant
				//determin inceputul framentului
				do {
					file.seek(start);
					b = file.readByte();
					start += 1;
				} while(((b >= 65) && (b <= 90)) || ((b >= 97) && (b <= 122)) && (start != file.length() - 1));
			}
			if(end != file.length()-1) {
				//fragmentul prelucrat se termina in mijlocul cuvantului
				do {
					file.seek(end);
					b = file.readByte();
					end += 1;
				}while(((b >= 65) && (b <= 90)) || ((b >= 97) && (b <= 122)) && (end != file.length() - 1));
			}
			int len = (int) (end - start);
			if(len > 0) {
				data = new byte[len];
				//citesc len bytes
				file.seek(start);
				file.read(data, 0, len);
				//se sincronizeaza doar modificarea asupra varibilelor care retin informatii despre fisiere
				synchronized (GlobalVars.lock[((PartialMapSolution) ps).getNumber()]) {
					Hashtable<String, Integer> thread_hash = 
							GlobalVars.file_stat[((PartialMapSolution) ps).getNumber()].file_hash;
					String str = new String(data);
					String del = String.valueOf(GlobalVars.delims);
					StringTokenizer st = new StringTokenizer(str,del);
					while(st.hasMoreElements()) {
						String word = (String) st.nextElement();
						word = word.toLowerCase();
						if(thread_hash.containsKey(word)) {
							thread_hash.put(word, thread_hash.get(word) + 1);
						}
						else {
							thread_hash.put(word, 1);
						}
						//actualizare nmar cuvinte din fisier
						GlobalVars.file_stat[((PartialMapSolution) ps).getNumber()].no_of_words++;
					}
				}
			}
			file.close();
		}
		//daca are loc procesarea pentru reduce
		else if(ps instanceof PartialReduceSolution) {
			int id_p = ((PartialReduceSolution) ps).id_plagiat_file;
			int id_v = ((PartialReduceSolution) ps).id_verify_file;
			int words_p = GlobalVars.file_stat[id_p].no_of_words;
			int words_v = GlobalVars.file_stat[id_v].no_of_words;
			Hashtable<String, Integer> hash_p = GlobalVars.file_stat[id_p].file_hash;
			Hashtable<String, Integer> hash_v = GlobalVars.file_stat[id_v].file_hash;
			Enumeration<String> e = hash_p.keys();
			while(e.hasMoreElements()) {
				String key = (String) e.nextElement();
				if(hash_v.containsKey(key)) {
					double f_key_p = ((double) hash_p.get(key) / (double) words_p);
					double f_key_v = ((double) hash_v.get(key) / (double) words_v);
					GlobalVars.sim[id_v] += (f_key_p * f_key_v) * 100;
				}
			}	
		}
	}
	
	public void run() {
		while (true) {
			PartialSolution ps = wp.getWork();
			if (ps == null)
				break;
			
			try {
				processPartialSolution(ps);
			} catch (FileNotFoundException e) {
				// TODO Auto-generated catch block
				e.printStackTrace();
			} catch (IOException e) {
				// TODO Auto-generated catch block
				e.printStackTrace();
			}
		}
	}
}

public class ReplicatedWorkers {

	public static void main(String args[]) throws IOException, InterruptedException {
		long size, no_of_chunks, last_chunk;
		String plagiat_file;
		int id_plagiat_file = 0;
		String [] verify_file;
		//dimensiunea fragmentelor in care se impart fiserele
		int chunk;
		//gradul de similaritate
		double similarity;
		//numarul de thread-uri
		int nt;
		//numarul de documente de indexat
		int nd;
		//numele fisierelor de input si de output
		String input_file, output_file;
		//extragere argumente linia de comanda
		nt = Integer.parseInt(args[0]);
		input_file = args[1];
		output_file = args[2];
		//alocare workpool si workeri map
		WorkPool map_wp = new WorkPool(nt);
		Worker map_worker[] = new Worker[nt];
		for(int i = 0; i < nt; i++) {
			map_worker[i] = new Worker(map_wp);
		}
		//citire date fisier input
		BufferedReader input_br = new BufferedReader(new FileReader(input_file));
		plagiat_file = input_br.readLine();
		chunk = Integer.parseInt(input_br.readLine());
		similarity = Double.parseDouble(input_br.readLine());
		nd = Integer.parseInt(input_br.readLine());
		GlobalVars.file_stat = new FileStatistics[nd];
		GlobalVars.lock = new Object[nd];
		for(int i = 0; i < GlobalVars.file_stat.length; i++) {
			GlobalVars.file_stat[i] = new FileStatistics();
			GlobalVars.lock[i] = new Object();
		}
		verify_file = new String[nd];
		for(int i = 0; i < nd; i++) {
			verify_file[i] = input_br.readLine();
		}
		input_br.close();
		//creare task-uri MAP
		for(int i = 0; i < nd; i++) {
			if(plagiat_file.equals(verify_file[i])) {
				id_plagiat_file = i;
			}
			size = (new File(verify_file[i]).length());
			no_of_chunks = size / chunk;
			last_chunk = size % chunk;
			for(int j = 0; j < no_of_chunks; j++){
				//adaugare in map workpool de catre thread-ul master
				map_wp.putWork(new PartialMapSolution(verify_file[i],i,0+j*chunk, (chunk-1)+(j*chunk)));
			}
			map_wp.putWork(new PartialMapSolution(verify_file[i],i,no_of_chunks*chunk, (no_of_chunks*chunk)+last_chunk-1));
		}
		//prelucrare MAP
		for(int i = 0; i < nt; i++){
			map_worker[i].start();
		}
		for(int i = 0; i < nt; i++) {
			map_worker[i].join();
		}
		//alocare workpool reduce
		WorkPool reduce_wp = new WorkPool(nt);
		Worker reduce_worker[] = new Worker[nt];
		for(int i = 0; i < nt; i++) {
			reduce_worker[i] = new Worker(reduce_wp);
		}
		GlobalVars.sim = new double[nd];
		//similitudinea intre fisierul de verificat si el insusi e de 100%
		GlobalVars.sim[id_plagiat_file] = 100;
		//creare task-uri REDUCE
		for(int i = 0; i < nd; i++) {
			if(!plagiat_file.equals(verify_file[i])) {
				reduce_wp.putWork(new PartialReduceSolution(plagiat_file, id_plagiat_file, verify_file[i], i));
				GlobalVars.sim[i] = 0;
			}
		}
		
		//prelucrare REDUCE
		for(int i = 0; i < nt; i++) {
			reduce_worker[i].start();
		}
		for(int i = 0; i < nt; i++) {
			reduce_worker[i].join();
		}
		//retinere informatii fisiere
		FileInfo[] file_info = new FileInfo[nd];
		for(int i = 0; i < nd; i++) {
			file_info[i] = new FileInfo(verify_file[i], GlobalVars.sim[i]);
		}
		//sortare dupa similitudine
		Arrays.sort(file_info);
		//afisare in fisierul de output
		BufferedWriter out = new BufferedWriter(new FileWriter(output_file));
		out.write("Rezultate pentru: (" + plagiat_file + ")");
		out.write("\n"); out.write("\n");
		for(int i = 0; i < nd; i++) {
			if(!plagiat_file.equals(file_info[i].name)) {
				if(file_info[i].similarity >= similarity) {
					out.write(file_info[i].name + " " + "(" + 
								String.format("%.3f", file_info[i].similarity) + ")");
					out.write("\n");
				}
			}
		}
		out.close();
	}
}


