package stateMachinesList;

import model.StateMachinesDiagram;

import org.eclipse.core.resources.IContainer;
import org.eclipse.core.resources.IFile;
import org.eclipse.core.runtime.Path;
import org.eclipse.ui.PlatformUI;
import org.jdom.Element;

import stateMachines.StateMachinesEditor;
import dom.DOMGetInstance;
import dom.ReadDOMTree;

public class DeleteStateMachine {

	private String machine;
	private IContainer container;
	private IFile file;
	private StateMachinesDiagram diagram;
	private Element machineElement;
	
	public DeleteStateMachine(String machine, IContainer container) {
		this.machine = machine;
		this.container = container;
	}
	
	public boolean execute(){
		ReadDOMTree tree = DOMGetInstance.getInstance();
		diagram = tree.getStateMachinesDiagram(machine);
		if (!(diagram == null)) {
			StateMachinesEditor editor1 = diagram.getEditor();
			PlatformUI.getWorkbench().getActiveWorkbenchWindow().getActivePage().closeEditor(editor1, true);
		}
		machineElement = tree.removeStateMachine1(machine);
		StateMachinesList.viewer.remove(machine);
		for (int i = 0; i < StateMachinesList.stateMachinesList.size(); i++) {
			file = (IFile) StateMachinesList.stateMachinesList.get(i);
			if (file.getName().equals(machine + ".states")) {
				StateMachinesList.stateMachinesList.remove(i);
				return true;
			}
		}
		return false;
	}
	
	public void undo(){
		ReadDOMTree tree = DOMGetInstance.getInstance();
		tree.addStateMachine1(machineElement);
		StateMachinesList.viewer.add(machine);
		file = container.getFile(new Path(machine + ".states"));
		StateMachinesList.stateMachinesList.add(file);
	}
	
	public void undoForDispose(){
		ReadDOMTree tree = DOMGetInstance.getInstance();
		tree.addStateMachine1(machineElement);
	}
	
	public void redo(){
		execute();
	}
}
