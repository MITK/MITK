package dom;

public class DOMGetInstance {
	private static ReadDOMTree tree = null;
	public static ReadDOMTree getFromXML(String filename) {
		tree = new ReadDOMTree(filename, false);
		return tree;
	}
	
	public static ReadDOMTree getInstance() {
		return tree;
	}
	
	public static ReadDOMTree createNew(String filename) {
		tree = new ReadDOMTree(filename, true);
		return tree;
	}

	public static void reset() {
		tree = null;
	}
}
