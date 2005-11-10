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
	
	public void addDiagram(StateMachinesDiagram diagram) {
		allMachines.add(diagram);
	}
	
	public void removeDiagram(StateMachinesDiagram diagram) {
		allMachines.remove(diagram);
	}
	
	public List getAllDiagrams() {
		return allMachines;
	}
	
	public List getStateMachines() {
		return doc.getRootElement().getChildren("stateMachine", doc.getRootElement().getNamespace());
	}

	public void addStateMachine(StateMachinesDiagram machine, Element stateMachine) {
		stateMachine.setNamespace(doc.getRootElement().getNamespace());
		doc.getRootElement().addContent(stateMachine);
		allMachines.add(machine);
	}
	
	public void addStateMachine1(Element stateMachine) {
		stateMachine.setNamespace(doc.getRootElement().getNamespace());
		doc.getRootElement().addContent(stateMachine);
	}

	public void removeStateMachine(StateMachinesDiagram machine, Element stateMachine) {
		doc.getRootElement().removeContent(stateMachine);
		allMachines.remove(machine);
	}
	
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
        else {
        	Element root = new Element("mitkInteraktionStates");
   		 	Namespace xmlns = Namespace.getNamespace(
   		 			"", "http://tempuri.org/StateMachine.xsd");
   		 	root.setNamespace(xmlns);
   		 	root.setAttribute("STYLE", "User001");
   		 	doc = new Document(root);
        }
	}

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