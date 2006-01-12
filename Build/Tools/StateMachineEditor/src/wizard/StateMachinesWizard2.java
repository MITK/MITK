package wizard;

import org.eclipse.jface.viewers.IStructuredSelection;
import org.eclipse.jface.wizard.Wizard;
import org.eclipse.ui.INewWizard;
import org.eclipse.ui.IWorkbench;
import org.eclipse.ui.PartInitException;
import org.eclipse.core.runtime.*;
import org.eclipse.jface.operation.*;

import java.lang.reflect.InvocationTargetException;
import java.util.List;

import org.eclipse.jface.dialogs.MessageDialog;
import org.eclipse.jface.viewers.ISelection;
import org.eclipse.core.resources.*;
import org.eclipse.core.runtime.CoreException;
import org.eclipse.ui.*;
import org.jdom.Element;

import dom.DOMGetInstance;
import dom.ReadDOMTree;

import stateMachinesList.StateMachinesList;

/**
 * This is a new wizard. Its role is to create new statemachines  
 * from the *.xml file in the provided container. If the container resource
 * (a folder or a project) is selected in the workspace 
 * when the wizard is opened, it will accept it as the target
 * container. The wizard creates statemachines with the extension
 * ".states". If an editor is registered for the same extension, it will
 * be able to open it.
 */

public class StateMachinesWizard2 extends Wizard implements INewWizard {
	private String file;
	private StateMachinesWizardPage2 page;
	private ISelection selection;
	private ReadDOMTree tree = null;

	/**
	 * Constructor for StateMachinesWizard2.
	 */
	
	public StateMachinesWizard2() {
		super();
		setNeedsProgressMonitor(true);
	}
	
	public StateMachinesWizard2(String file2) {
		super();
		setNeedsProgressMonitor(true);
		file = file2;
	}
	
	/**
	 * Adding the page to the wizard.
	 */

	public void addPages() {
		page = new StateMachinesWizardPage2(selection);
		addPage(page);
	}

	/**
	 * This method is called when 'Finish' button is pressed in
	 * the wizard. We will create an operation and run it
	 * using wizard as execution context.
	 */
	public boolean performFinish() {
		IWorkbenchPage page1 = PlatformUI.getWorkbench().getActiveWorkbenchWindow().getActivePage(); 
		StateMachinesList.closeView();
		try {
			page1.showView("StatemachinesList");
		} catch (PartInitException e1) {
			e1.printStackTrace();
		}
		DOMGetInstance.reset();
		StateMachinesList.reset();
		final String containerName = page.getContainerName();
		IRunnableWithProgress op = new IRunnableWithProgress() {
			public void run(IProgressMonitor monitor) throws InvocationTargetException {
				tree = DOMGetInstance.getFromXML(file);
				List filenames = tree.getStateMachineNames();
				for (int i = 0; i < filenames.size(); i++) {
				try {					
					String filename = filenames.get(i).toString() + ".states";
					doFinish(containerName, filename, monitor);				
				} catch (CoreException e) {
					throw new InvocationTargetException(e);
				}
			finally {
					monitor.done();
				}
			}
		}
		};
		try {
			getContainer().run(true, false, op);
		} catch (InterruptedException e) {
			return false;
		} catch (InvocationTargetException e) {
			MessageDialog.openError(getShell(), "Error", "Project does not exist!");
			return false;
		}
		return true;
	}
	
	/**
	 * The worker method. It will find the container and create the
	 * statemachines.
	 */

	private void doFinish(
		String containerName,
		String fileName,
		IProgressMonitor monitor)
		throws CoreException {
		// create a file
		monitor.beginTask("Creating " + fileName, 2);
		IWorkspaceRoot root = ResourcesPlugin.getWorkspace().getRoot();
		IResource resource = root.findMember(new Path(containerName));
		if (!resource.exists() || !(resource instanceof IContainer)) {
			throwCoreException("Container \"" + containerName + "\" does not exist.");
		}
		IContainer container = (IContainer) resource;
		final IFile file1 = container.getFile(new Path(fileName));
		DOMGetInstance.setContainer(container);
		monitor.worked(1);
		monitor.setTaskName("Opening file for editing...");
		getShell().getDisplay().asyncExec(new Runnable() {
			public void run() {
				// create StateMachinesList view from xml file
				List allMachines = tree.getStateMachines();
				for (int i = 0; i < allMachines.size(); i++) {
					Element machine = (Element) allMachines.get(i);
					String file2 = file1.getName().toString();
					int length = file2.length();
					String filename = file2.subSequence(0, length-7).toString();
					if (filename.equals(machine.getAttributeValue("NAME"))) {
						StateMachinesList.addToStateMachinesList(file1, filename);
						break;
					}
				}
			}
		});
		monitor.worked(1);
	}

	private void throwCoreException(String message) throws CoreException {
		IStatus status =
			new Status(IStatus.ERROR, "Container", IStatus.OK, message, null);
		throw new CoreException(status);
	}

	/**
	 * We will accept the selection in the workbench to see if
	 * we can initialize from it.
	 * @see IWorkbenchWizard#init(IWorkbench, IStructuredSelection)
	 */
	
	public void init(IWorkbench workbench, IStructuredSelection selection) {
		this.selection = selection;
	}
}