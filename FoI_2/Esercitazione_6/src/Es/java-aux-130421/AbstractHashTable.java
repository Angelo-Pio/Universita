import java.util.Random;
import java.util.ArrayList;

public abstract class AbstractHashTable {
	private int capacity; // dim. tabella
	private int n = 0; // numero di entry nella tabella
	private int prime; // numero primo
	private long a, b; // coefficienti per MAD
	private double maxLambda; // fattore di carico massimo
	
	// La classe Entry --> coppie (chiave, valore)
	class Entry {	
		private String key;
		private int value;
		public Entry(String k, int v) {
			key = k;
			value = v;
		}
		public String getKey() { // Restituisce chiave
			return key;
		}
		public int getValue() { // Restituisce valore
			return value;
		}
		public void setValue(int v) { // Aggiorna valore 
			value = v;
			return;
		}
		public String toString() {
			return "(" + getKey() + ", " + Integer.toString(getValue()) + ")";
		}
	}
	
	// Costruttori di AbstractHashTable
	public AbstractHashTable(int cap, int p, double lambda) {
		capacity = cap;
		prime = p;
		maxLambda = lambda;
		Random gen = new Random();
		a = gen.nextInt(prime) + 1;
		b = gen.nextInt(prime);
		createTable();
	}
	public AbstractHashTable(int cap, int p) {
		this(cap, p, 0.5); // massimo fattore di carico predefinito
	}
	public AbstractHashTable(int cap) {
		this(cap, 109345121); // primo predefinito
	}
	public AbstractHashTable() {
		this(5); // capacità predefinita
	}
	
	// Metodi ausiliari comuni a tutte le classi
	
	// metodo che implementa la funzione hash (hash code + compressione)
	// Si ricordi che ogni oggetto Java implementa hashcode, 
	// a cominciare dalle stringhe
	protected int hashFunction(String k) { 
		int hashvalue = k.hashCode();
		return 	(int)(Math.abs((a*hashvalue+b)%prime)%capacity);
	}

	protected void checkSize (){
        long loadFactor = size()/getCapacity();
        if(!(loadFactor < getMaxLambda())){
            resize(getCapacity()*2 -1);
        }
    }

	// metodo che aggiorna la dimensione della tabella hash	(N)
	protected void resize(int newCap) { 	

		Iterable<Entry> it = entrySet();
		capacity = newCap;
		createTable();

		for (Entry entry : it) {
			if(entry == null){
				continue;
			}
			put(entry.getKey(),entry.getValue());
		}


		return;
	}
		
	// Metodi pubblici comuni a tutte le classi
	
	// restituisce true se la tabella è vuota
	public boolean isEmpty() { // restituisce true se tabella vuota
		return n == 0 ? true : false;
	}
	
	// restituisce il numero di chiavi presenti nella tabella
	public int size() { 
		return n;
	}
	
	// restituisce la capacità della tabella
	public int getCapacity() { 
		return capacity;
	}
	
	// incrementa il numero n di chiavi presenti
	public void incSize() { 
		n++;
		return;
	}
	
	// decrementa il numero n di chiavi presenti
	public void decSize() { 
		n--;
		return;
	}
	
	// restituisce valore max. per il fattore di carico (si effettua resize se superato)
	public double getMaxLambda() { 
		return maxLambda;
	}
	
	// Stampa una rappresentazione delle coppire presenti secondo
    // il formato [(k1, v1),(k2,v2), ... ,(kn, vn)]
	public void print() {
		System.out.printf("[");

		Iterable<Entry> it = entrySet(); 

		for (Entry entry : it) {
			if(entry == null){
				System.out.printf("(null)");
			}else{
				System.out.printf(entry.toString());
			}
			System.out.printf(",");
		}

		System.out.printf("]\n");
	}
	
	// Metodi astratti da implementare nelle sottoclassi
	// Descrizioni più dettagliate nelle classi ChainHashTable e OpenHashTable
	protected abstract void createTable(); // inizializza la tabella hash
	public abstract int get(String k); // restituisce il valore associato alla chiave k
	public abstract int put(String k, int value); // inserisce un coppia
	public abstract int remove(String k); // rimuove la coppia con chiave k
	public abstract Iterable<Entry> entrySet(); // restituisce un Iterable contentente tutte le coppie presenti
		
}