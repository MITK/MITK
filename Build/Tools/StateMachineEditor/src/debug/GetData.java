package debug;

import org.eclipse.jface.action.IAction;
import org.eclipse.jface.viewers.ISelection;
import org.eclipse.ui.IWorkbenchWindow;
import org.eclipse.ui.IWorkbenchWindowActionDelegate;

import stateMachinesList.StateMachinesList;

public class GetData  implements IWorkbenchWindowActionDelegate {

	public GetData() {
		super();
	}

	public void dispose() {
		
	}

	public void init(IWorkbenchWindow window) {
		
	}

	public void run(IAction action) {
		if (StateMachinesList.isDebugMode()) {
			DebugStateMachinesList.addStateMachineToList();
		}
	}

	public void selectionChanged(IAction action, ISelection selection) {
		
	}

}
