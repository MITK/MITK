package dom;

import java.io.File;

import org.eclipse.core.resources.IContainer;

public class DOMGetInstance {
	private static ReadDOMTree tree = null;
	private static String filename1 = null;
	private static IContainer container1 = null;
	
	/**
	 * creates a DOMTree from an specified xml file
	 * @param filename the xml file with the statemachines
	 * @return the ReadDOMTree instance
	 */
	public static ReadDOMTree getFromXML(String filename) {
		filename1 = filename;
		tree = new ReadDOMTree(filename, false);
		return tree;
	}
	
	/**
	 * @return the ReadDOMTree instance
	 */
	public static ReadDOMTree getInstance() {
		return tree;
	}
	
	/**
	 * creates a new xml file with the specified name
	 * @param filename the name for the new xml file
	 * @return the ReadDOMTree instance
	 */
	public static ReadDOMTree createNew(String filename) {
		filename1 = filename;
		tree = new ReadDOMTree(filename, true);
		return tree;
	}

	/**
	 * resets the ReadDOMTree instance
	 */
	public static void reset() {
		tree = null;
	}
	
	private static ReadActionAndEventDOMTree actionAndEventTree = null;
	
	/**
	 * creates a new ReadActionAndEventDOMTree instance from mitkEventAndActionConstants.xml
	 * which has to be located in the same folder as the xml file with the statemachines
	 * @return the ReadActionAndEventDOMTree instance
	 */
	public static ReadActionAndEventDOMTree getActionAndEventInstance() {
		if (actionAndEventTree == null) {
			File file = new File(filename1);
			file.getAbsolutePath();
			File file1 = new File(file.getParentFile() + File.separator + "mitkEventAndActionConstants.xml");
			actionAndEventTree = new ReadActionAndEventDOMTree(file1);
		}
		return actionAndEventTree;
	}
	
	/**
	 * sets the file container
	 * @param container the file container
	 */
	public static void setContainer(IContainer container) {
		container1 = container;
	}
	
	public static IContainer getContainer() {
		return container1;
	}
	
	public static String getFilename() {
		File file = new File(filename1);
		String filename = file.getName();
		return filename;
	}
}
