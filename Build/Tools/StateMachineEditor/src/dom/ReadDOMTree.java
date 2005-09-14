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
import org.jdom.input.SAXBuilder;
import org.jdom.output.Format;
import org.jdom.output.XMLOutputter;

public class ReadDOMTree {
	private List allMachines = new ArrayList();
	private Document doc = null;
	
	public List getStateMachines() {
		return doc.getRootElement().getChildren("stateMachine", doc.getRootElement().getNamespace());
	}
	
	public Element getRoot() {
		return doc.getRootElement();
	}	

	public void addStateMachine(StateMachinesDiagram machine, Element stateMachine) {
		stateMachine.setNamespace(doc.getRootElement().getNamespace());
		doc.getRootElement().addContent(stateMachine);
		allMachines.add(machine);
	}

	public void removeStateMachine(StateMachinesDiagram machine, Element stateMachine) {
		doc.getRootElement().removeContent(stateMachine);
		allMachines.remove(machine);
	}
	
	/*public StateMachinesDiagram create() {
		List stateMachinesList = doc.getRootElement().getChildren("stateMachine", doc.getRootElement().getNamespace());
        for (int i = 0; i < stateMachinesList.size(); i++) {
        	Element ele1 = (Element) stateMachinesList.get(i);
        	StateMachinesDiagram machine = new StateMachinesDiagram(ele1);
        	allMachines.add(machine);
        }
        return machine;
	}*/
	public ReadDOMTree(String file) {
        String filename = file;
        try {
            // Build the document with SAX and Xerces, no validation
            SAXBuilder builder = new SAXBuilder();
            // Create the document
            doc = builder.build(new File(filename));
            /*List stateMachinesList = doc.getRootElement().getChildren("stateMachine", doc.getRootElement().getNamespace());
            for (int i = 0; i < stateMachinesList.size(); i++) {
            	Element ele1 = (Element) stateMachinesList.get(i);
            	StateMachinesDiagram machine = new StateMachinesDiagram(ele1);
            	allMachines.add(machine);
            }*/
        } catch (Exception e) {
            e.printStackTrace();
        }
	}
   
	public ReadDOMTree() {
		// create new DOMTree
		 Element root = new Element("mitkInteraktionStates");
		 
		 root.setAttribute("STYLE", "User001");
		 doc = new Document(root);
	}

	public void writeTree(String file) {
		Format format = Format.getPrettyFormat();
		format.setEncoding("utf-8");
		XMLOutputter xmlOut = new XMLOutputter(format);
		try {
			xmlOut.output(doc, new FileOutputStream(file));
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