package dom;

import java.io.File;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.IOException;
import java.util.ArrayList;
import java.util.List;

import model.StateMachinesDiagram;

import org.jdom.Document;
import org.jdom.Element;
import org.jdom.Namespace;
import org.jdom.input.SAXBuilder;
import org.jdom.output.Format;
import org.jdom.output.XMLOutputter;

public class ReadDOMTree {
	private List allMachines = new ArrayList();
	private Document doc = null;
	private String filename;
	
	/**
	 * adds an open editor to the list allMachines
	 * @param diagram StateMAchinesDiagram of an open editor
	 */
	public void addDiagram(StateMachinesDiagram diagram) {
		allMachines.add(diagram);
	}
	
	/**
	 * removes an open editor from list allMachines
	 * @param diagram StateMachinesDiagram of an open editor
	 */
	public void removeDiagram(StateMachinesDiagram diagram) {
		allMachines.remove(diagram);
	}
	
	/**
	 * @return a list of all currently open editors
	 */
	public List getAllDiagrams() {
		return allMachines;
	}
	
	/**
	 * @return a list of all statemachines in the DOMTree
	 */
	public List getStateMachines() {
		return doc.getRootElement().getChildren("stateMachine", doc.getRootElement().getNamespace());
	}

	/**
	 * adds a statemachine with an open editor to the DOMTree
	 * @param machine the stateMachinesDiagram with an open editor
	 * @param stateMachine the DOM element of the statemachine
	 */
	public void addStateMachine(StateMachinesDiagram machine, Element stateMachine) {
		stateMachine.setNamespace(doc.getRootElement().getNamespace());
		doc.getRootElement().addContent(stateMachine);
		allMachines.add(machine);
	}
	
	/**
	 * adds a statemachine with no open editor to the DOMTree
	 * @param stateMachine the DOM element of the statemachine
	 */
	public void addStateMachine1(Element stateMachine) {
		stateMachine.setNamespace(doc.getRootElement().getNamespace());
		doc.getRootElement().addContent(stateMachine);
	}

	/**
	 * removes a statemachine with an open editor from DOMTree
	 * @param machine the stateMachinesDiagram with an open editor
	 * @param stateMachine the DOM element of the statemachine
	 */
	public void removeStateMachine(StateMachinesDiagram machine, Element stateMachine) {
		doc.getRootElement().removeContent(stateMachine);
		allMachines.remove(machine);
	}
	
	
	/**
	 * removes a statemachine with no open editor from DOMTree
	 * @param stateMachine the DOM element of the statemachine
	 * @return the DOM element of the removed statemachine
	 */
	public Element removeStateMachine1(String stateMachine) {
		List stateMachines = getStateMachines();
		for (int i = 0; i < stateMachines.size(); i++) {
			Element ele = (Element) stateMachines.get(i);
			if (ele.getAttributeValue("NAME").equals(stateMachine)) {
				doc.getRootElement().removeContent(ele);
				return ele;
			}
		}
		return null;
	}
	
	/**
	 * returns a diagram from the statemachine with the specified name
	 * @param machineName name of the statemachine with the diagram
	 * @return a StateMachinesDiagram with an open editor
	 */
	public StateMachinesDiagram getStateMachinesDiagram(String machineName) {
		StateMachinesDiagram machine = null;
		for (int j = 0; j < allMachines.size(); j++) {
			machine = (StateMachinesDiagram) allMachines.get(j);
			if (machine.getStateMachineName().equals(machineName)) {
				return machine;
			}
		}
		return null;
	}
	
	/**
	 * constructor to build the DOMTree from the xml file
	 * @param file the xml file
	 * @param newTree boolean value to show if it has to create a new xml file
	 * 			true = has to create
	 */
	public ReadDOMTree(String file, boolean newTree) {
        filename = file;
        if (!(newTree)) {
        	try {
            	// Build the document with SAX and Xerces, no validation
            	SAXBuilder builder = new SAXBuilder();
            	// Create the document
            	doc = builder.build(new File(filename));
        	} catch (Exception e) {
        		e.printStackTrace();
        	}
        }
        // create new xml file
        else {
        	Element root = new Element("mitkInteraktionStates");
   		 	Namespace xmlns = Namespace.getNamespace(
   		 			"", "http://tempuri.org/StateMachine.xsd");
   		 	root.setNamespace(xmlns);
   		 	root.setAttribute("STYLE", "User001");
   		 	doc = new Document(root);
        }
	}

	/**
	 * saves the DOMTree on file location
	 * @param file file location to save the xml file
	 */
	public void writeTree(String file) {
		Format format = Format.getPrettyFormat();
		format.setEncoding("utf-8");
		XMLOutputter xmlOut = new XMLOutputter(format);
		try {
			xmlOut.output(doc, new FileOutputStream(file));
			filename = file;
		} catch (FileNotFoundException e) {
			e.printStackTrace();
		} catch (IOException e) {
			e.printStackTrace();
		}
	}
	
	/**
	 * saves the DOMTree on the same location where it has been loaded
	 */
	public void writeTree() {
		Format format = Format.getPrettyFormat();
		format.setEncoding("utf-8");
		XMLOutputter xmlOut = new XMLOutputter(format);
		try {
			xmlOut.output(doc, new FileOutputStream(filename));
		} catch (FileNotFoundException e) {
			e.printStackTrace();
		} catch (IOException e) {
			e.printStackTrace();
		}
	}

	/**
	 * @return returns a list of all names from the statemachine in the DOMTree
	 */
	public List getStateMachineNames() {
		List stateMachines = doc.getRootElement().getChildren("stateMachine", doc.getRootElement().getNamespace());
		List stateMachineNames = new ArrayList();
		for (int i = 0; i < stateMachines.size(); i++) {
			Element machine = (Element) stateMachines.get(i);
			stateMachineNames.add(machine.getAttributeValue("NAME"));
		}
		return stateMachineNames;
	}
}