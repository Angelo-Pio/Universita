import java.security.KeyStore.Entry;
import java.util.*;
import java.util.concurrent.ConcurrentLinkedQueue;

public class GraphServices {

	
	/**
	 *
	 */
	private static final int MAX_VALUE = 10000;

	public static <V> void bfs(Graph<V> g) {
            
        for ( Node<V> v : g.getNodes()) {
            if(v.stato == Node.Stato.UNEXPLORED){

                bfsFromNode(v, g);
            }
        }
            
	}

	public static <V> void sssp(Graph<V> g, Node<V> source) {

        MinHeap<Node<V>> pqueue = new MinHeap<Node<V>>();

		HashMap<Node<V>, HeapEntry<Node<V>>> dist = new HashMap<Node<V>, HeapEntry<Node<V>>>();


		final int INFINITY = 100000; // = "Infinito"
		// (NB.: deve essere maggiore della somma di tutti i pesi del grafo, altrimenti e' scorretto)

		// Inizializzazione

		for(Node<V> u : g.getNodes()) {
			HeapEntry<Node<V>> hu = pqueue.insert(u == source ? 0 : INFINITY, u);
			dist.put(u, hu);
		}

		// Ciclo principale

		while (!pqueue.isEmpty()) {
			HeapEntry<Node<V>> hu = pqueue.removeMin();
			Node<V> u = hu.getValue();

			for(Edge<V> e : g.getOutEdges(u)) {

				Node<V> v = e.getTarget();
				if (dist.get(u).getKey() + e.getWeight() < dist.get(v).getKey()) {
					pqueue.replaceKey(dist.get(v), dist.get(u).getKey() + e.getWeight());
				}

			}

		}

		for(Node<V> u : g.getNodes()) {
			System.out.println(u + " " + dist.get(u).getKey());
		}

        
    }
	
	public static <V> void mst(Graph<V> G) {

		Partition<V> part = new Partition<>(G.getNodes());
		MinHeap<Edge<V>> Q = new MinHeap<>();

		for (Edge<V> e : G.getEdges()) {
			Q.insert(e.getWeight(), e);
		}
		LinkedList<Edge<V>> T = new LinkedList<>();

		while (!Q.isEmpty() ) {
			HeapEntry<Edge<V>> entry = Q.removeMin();
			Edge<V> e = entry.getValue();

			Node<V> s = e.getSource();
			Node<V> t = e.getTarget();
			
			List<Node<V>> C_s = part.find(s.map);
			List<Node<V>> C_t = part.find(t.map);

			if(!C_s.equals(C_t)){
				part.union(s.map,t.map);
				T.add(e);
			}

		}

		printMST(T);

	
	}

	public static<V> void mst_prim(Graph<V> g){

		LinkedList<Edge<V>> T = new LinkedList<>(); 
		MinHeap<Node<V>> Q = new MinHeap<>();
		HashMap< Node<V> , HeapEntry<Node<V> > > cloud  = new HashMap<>();



		// Inizializza una pq Q di HeapEntry(chiave,nodo)
		Iterator<Node<V>> nodes = g.getNodes().iterator();
		Node<V> n = nodes.next();
		cloud.put(n, Q.insert(0, n));

		while(nodes.hasNext()){
			n = nodes.next();
			cloud.put(n, Q.insert(MAX_VALUE, n));
		}


		// Finchè Q non è vuota
		while (!Q.isEmpty()) {
			// Prendi il minimo
			HeapEntry<Node<V>> entry = Q.removeMin();
			Node<V> u = entry.getValue();
			
			u.stato = Node.Stato.EXPLORED;

			T.add(u.getConnectEdge());
			// Scansiona tutti gli archi (u,v) per cui v è in Q
			for (Edge<V> e : g.getOutEdges(u)) {

				Node<V> v = e.getTarget();
				if(v.stato == Node.Stato.UNEXPLORED ){
					// Se il peso di (u,v) < etichetta di v
					int label = cloud.get(v).getKey();
					if(e.getWeight() < label){
						// Cambia la chiave di v in Q ed il suo arco associato
						Q.replaceKey(cloud.get(v), label);
						v.setConnectEdge(e);

					}

				}
			}
		}




		printMST(T);



	}

	private static <V> void printMST(LinkedList<Edge<V>> T) {
		for (Edge<V> e : T) {
			System.out.println(e.getSource().getElement() + " " + e.getTarget().getElement());
		}
	}

    private static <V> void bfsFromNode(Node<V> node, Graph<V> g) {

        ArrayDeque<Node<V>> Q = new ArrayDeque<>();
        
        node.stato = Node.Stato.EXPLORED;
        Q.add(node);
        
        while(! Q.isEmpty()){
                Node<V> u = Q.remove();
                System.out.println(u.getElement());
                for (Edge<V> e : g.getOutEdges(u)) {
                    Node<V> v = e.getTarget();
                    if(v.stato == Node.Stato.UNEXPLORED){
                        v.stato = Node.Stato.EXPLORED;
                        Q.add(v);
                    }
                }
            
           
        }
        
        
    }
}




