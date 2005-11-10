package stateMachinesList;

import java.util.List;

import model.StateMachinesDiagram;

import org.eclipse.core.resources.IContainer;
import org.eclipse.core.resources.IFile;
import org.eclipse.core.runtime.Path;
import org.eclipse.ui.PlatformUI;
import org.jdom.Element;

import stateMachines.StateMachinesEditor;

import dom.DOMGetInstance;
import dom.ReadDOMTree;

public class CopyStateMachine {
	
	private String machine;
	private String name;
	private IContainer container;
	private IFile file;
	private StateMachinesDiagram diagram;
	private Element machineElement;
	
	public CopyStateMachine(String machine, String name, IContainer container) {
		this.machine = machine;
		this.name = name;
		this.container = container;
	}
	
	public void execute(){
		Element machineEle = null;
		StateMachinesList.allNames.add(name);
		StateMachinesList.viewer.add(name);
		ReadDOMTree tree = DOMGetInstance.getInstance();
		List allMachineEles = tree.getStateMachines();
		for (int i = 0; i < allMachineEles.size(); i++) {
			machineEle = (Element) allMachineEles.get(i);
			if (machineEle.getAttributeValue("NAME").equals(machine)) {
				break;
			}
		}
		Element machineEle1 = (Element) machineEle.clone();
		machineEle1.setAttribute("NAME", name);
		tree.addStateMachine1(machineEle1);
		file = container.getFile(new Path(name + ".states"));
		StateMachinesList.stateMachinesList.add(file);
	}
	
	public void undo(){
		ReadDOMTree tree = DOMGetInstance.getInstance();
		diagram = tree.getStateMachinesDiagram(name);
		if (!(diagram == null)) {
			StateMachinesEditor editor1 = diagram.getEditor();
			PlatformUI.getWorkbench().getActiveWorkbenchWindow().getActivePage().closeEditor(editor1, true);
		}
		machineElement = tree.removeStateMachine1(name);
		StateMachinesList.viewer.remove(name);
		StateMachinesList.stateMachinesList.remove(file);
	}
	
	public void undoForDispose(){
		ReadDOMTree tree = DOMGetInstance.getInstance();
		tree.removeStateMachine1(name);
	}
	
	public void redo(){
		StateMachinesList.allNames.add(name);
		StateMachinesList.viewer.add(name);
		ReadDOMTree tree = DOMGetInstance.getInstance();
		tree.addStateMachine1(machineElement);
		StateMachinesList.stateMachinesList.add(file);
	}
}
