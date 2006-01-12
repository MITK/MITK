package debug;

import javax.swing.JOptionPane;

import org.eclipse.jface.action.IAction;
import org.eclipse.jface.viewers.ISelection;
import org.eclipse.ui.IWorkbenchPage;
import org.eclipse.ui.IWorkbenchWindow;
import org.eclipse.ui.IWorkbenchWindowActionDelegate;
import org.eclipse.ui.PartInitException;
import org.eclipse.ui.PlatformUI;

import dom.DOMGetInstance;

import stateMachinesList.StateMachinesList;

public class DebugServer implements IWorkbenchWindowActionDelegate {
	
	public DebugServer() {
	}
	
	public void dispose() {
	}
	
	public void init(IWorkbenchWindow window) {
	}
	
	public void run(IAction action) {
		if (!(DOMGetInstance.getInstance() == null)) {
			StateMachinesList.setDebugMode(true);
			IWorkbenchPage page1 = PlatformUI.getWorkbench().getActiveWorkbenchWindow().getActivePage(); 
			try {
				page1.showView("DebugStateMachinesList");
			} catch (PartInitException e1) {
				e1.printStackTrace();
			}
			new DebugServerThread().start();
		}
		else {
			JOptionPane.showMessageDialog(null,
					"You have to open a statemachine xml file first!", "Error Message",
					JOptionPane.ERROR_MESSAGE);
		}
	}
	
	public void selectionChanged(IAction action, ISelection selection) {
	}
}