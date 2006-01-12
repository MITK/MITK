package wizard;

import org.eclipse.jface.viewers.IStructuredSelection;
import org.eclipse.jface.wizard.Wizard;
import org.eclipse.swt.SWT;
import org.eclipse.swt.widgets.FileDialog;
import org.eclipse.swt.widgets.Shell;
import org.eclipse.ui.INewWizard;
import org.eclipse.ui.IWorkbench;
import org.eclipse.ui.IWorkbenchPage;
import org.eclipse.ui.PartInitException;
import org.eclipse.ui.PlatformUI;
import org.eclipse.core.runtime.*;
import org.eclipse.jface.operation.*;

import java.io.File;
import java.lang.reflect.InvocationTargetException;

import org.eclipse.jface.dialogs.MessageDialog;
import org.eclipse.jface.viewers.ISelection;
import org.eclipse.core.resources.*;
import org.eclipse.core.runtime.CoreException;
import org.eclipse.ui.*;

import dom.DOMGetInstance;
import dom.ReadDOMTree;

import stateMachinesList.StateMachinesList;

/**
 * This is a new wizard. Its role is to create a new statemachine
 * in the provided container. If the container resource
 * (a folder or a project) is selected in the workspace 
 * when the wizard is opened, it will accept it as the target
 * container. The wizard creates one statemachine with the extension
 * ".states". If an editor is registered for the same extension, it will
 * be able to open it.
 */

public class StateMachinesWizard extends Wizard implements INewWizard {
	private StateMachinesWizardPage page;
	private ISelection selection;

	/**
	 * Constructor for StateMachinesWizard.
	 */
	public StateMachinesWizard() {
		super();
		setNeedsProgressMonitor(true);
	}
	
	/**
	 * Adding the page to the wizard.
	 */

	public void addPages() {
		page = new StateMachinesWizardPage(selection);
		addPage(page);
	}

	/**
	 * This method is called when 'Finish' button is pressed in
	 * the wizard. We will create an operation and run it
	 * using wizard as execution context.
	 */
	public boolean performFinish() {
		IWorkbenchPage page1 = PlatformUI.getWorkbench().getActiveWorkbenchWindow().getActivePage(); 
		try {
			page1.showView("StatemachinesList");
		} catch (PartInitException e1) {
			e1.printStackTrace();
		}
		final String containerName = page.getContainerName();
		final String fileName = page.getFileName();
		IRunnableWithProgress op = new IRunnableWithProgress() {
			public void run(IProgressMonitor monitor) throws InvocationTargetException {
				try {
					doFinish(containerName, fileName, monitor);
				} catch (CoreException e) {
					throw new InvocationTargetException(e);
				} finally {
					monitor.done();
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
	 * statemachine.
	 */

	private void doFinish(
		String containerName,
		String fileName,
		IProgressMonitor monitor)
		throws CoreException {
		// create a new file
		monitor.beginTask("Creating " + fileName, 2);
		IWorkspaceRoot root = ResourcesPlugin.getWorkspace().getRoot();
		IResource resource = root.findMember(new Path(containerName));
		if (!resource.exists() || !(resource instanceof IContainer)) {
			throwCoreException("Container \"" + containerName + "\" does not exist.");
		}
		
		IContainer container = (IContainer) resource;
		final IFile file = container.getFile(new Path(fileName));
		DOMGetInstance.setContainer(container);
		monitor.worked(1);
		monitor.setTaskName("Opening file for editing...");
		getShell().getDisplay().asyncExec(new Runnable() {
			public void run() {
				ReadDOMTree tree = DOMGetInstance.getInstance();
				// tree already exists, add statemachine to tree
				if (!(tree == null)) {
					String file1 = file.getName().toString();
					int length = file1.length();
					String filename = file1.subSequence(0, length-7).toString();
					StateMachinesList.addToStateMachinesList2(file, filename);
				}
				// tree does not exist, create a new tree and add a statemachine
				else {
					DOMGetInstance.reset();
					StateMachinesList.reset();
					Shell shell = PlatformUI.getWorkbench().getActiveWorkbenchWindow().getShell();
					FileDialog fileDialog = new FileDialog(shell, SWT.SAVE);
					fileDialog.setFilterExtensions(new String[] {"*.xml"});
					fileDialog.open();
					String file2 = fileDialog.getFilterPath().toString() + File.separator + fileDialog.getFileName().toString();
					if (!(fileDialog.getFileName() == null) && !(fileDialog.getFileName().equals(""))) {
						DOMGetInstance.createNew(file2);
						String file1 = file.getName().toString();
						int length = file1.length();
						String filename = file1.subSequence(0, length-7).toString();
						StateMachinesList.addToStateMachinesList2(file, filename);
					}
				}
			}
		});
		monitor.worked(1);
	}

	private void throwCoreException(String message) throws CoreException {
		IStatus status =
			new Status(IStatus.ERROR, "GetStateMachines", IStatus.OK, message, null);
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