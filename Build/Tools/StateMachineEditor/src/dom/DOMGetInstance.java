package dom;

public class DOMGetInstance {
	private static ReadDOMTree tree = null;
	public static ReadDOMTree getInstance(String filename) {
		if (tree == null) {
			tree = new ReadDOMTree(filename);
		}
		return tree;
	}
	
	public static ReadDOMTree getInstance() {
		return tree;
	}
	
	public static ReadDOMTree createNew() {
		tree = new ReadDOMTree();
		return tree;
	}

	public static void reset() {
		tree = null;
	}
}
