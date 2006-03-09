package stateMachinesList;

import java.io.File;
import java.util.ArrayList;
import java.util.List;

import javax.swing.JOptionPane;

import model.StateMachinesDiagram;

import org.eclipse.core.resources.IFile;
import org.eclipse.core.runtime.IProgressMonitor;
import org.eclipse.core.runtime.Path;
import org.eclipse.jface.action.Action;
import org.eclipse.jface.action.GroupMarker;
import org.eclipse.jface.action.IMenuListener;
import org.eclipse.jface.action.IMenuManager;
import org.eclipse.jface.action.MenuManager;
import org.eclipse.jface.action.Separator;
import org.eclipse.jface.dialogs.InputDialog;
import org.eclipse.jface.resource.ImageDescriptor;
import org.eclipse.jface.viewers.DoubleClickEvent;
import org.eclipse.jface.viewers.IDoubleClickListener;
import org.eclipse.jface.viewers.ISelectionChangedListener;
import org.eclipse.jface.viewers.IStructuredSelection;
import org.eclipse.jface.viewers.ListViewer;
import org.eclipse.jface.viewers.SelectionChangedEvent;
import org.eclipse.jface.viewers.ViewerSorter;
import org.eclipse.jface.window.Window;
import org.eclipse.swt.SWT;
import org.eclipse.swt.graphics.Image;
import org.eclipse.swt.widgets.Composite;
import org.eclipse.swt.widgets.FileDialog;
import org.eclipse.swt.widgets.Menu;
import org.eclipse.swt.widgets.Shell;
import org.eclipse.ui.ISaveablePart;
import org.eclipse.ui.ISharedImages;
import org.eclipse.ui.IWorkbenchActionConstants;
import org.eclipse.ui.IWorkbenchPage;
import org.eclipse.ui.PartInitException;
import org.eclipse.ui.PlatformUI;
import org.eclipse.ui.ide.IDE;
import org.eclipse.ui.part.ViewPart;
import org.jdom.Element;

import dom.DOMGetInstance;
import dom.ReadDOMTree;

public class StateMachinesList extends ViewPart implements ISaveablePart, IDoubleClickListener {
	
	Action deleteItemAction, renameItemAction, copyItemAction;
	static Action redoAction, undoAction;
	static List stateMachinesList = new ArrayList();
	static Composite parent;
	static ListViewer viewer = null;
	static IFile machineName;
	static String fileName;
	public static List allNames = new ArrayList();
	private static List undoList = new ArrayList();
	private static List redoList = new ArrayList();
	private static List undoNewStateMachine = new ArrayList();
	
	public static final String DELETE = "delete statemachine";
	public static final String RENAME = "rename statemachine";
	public static final String COPY = "copy statemachine";
	public static final String DOUBLE_CLICK_ACTION = "Double_Click_ActionId";
	private static boolean dirty = false;
	private boolean isSaved = false;
	private boolean workbenchIsClosing = false;
	private static StateMachinesList view;
	private static boolean debugMode = false;
	
	/**
	 * the constructor for this view
	 */
	public StateMachinesList() {
		super();
		view = this;
	}
	
	/**
	 * hides this view, asks for save if it is modified 
	 */
	public static void closeView() {
		IWorkbenchPage page1 = PlatformUI.getWorkbench().getActiveWorkbenchWindow().getActivePage(); 
		page1.hideView(view);
	}
	
	/**
	 * adds a statemachine to this view at startup
	 * @param file statemachine name with .states extension
	 * @param fileName the name of the statemachine
	 */
	public static void addToStateMachinesList(IFile file, String fileName) {
		stateMachinesList.add(file);
		viewer.add(fileName);
		allNames.add(fileName);
	}
	
	/**
	 * adds a newly created statemachine to this view
	 * @param file statemachine name with .states extension
	 * @param fileName the name of the statemachine
	 */
	public static void addToStateMachinesList2(IFile file, String fileName) {
		ReadDOMTree tree = DOMGetInstance.getInstance();
		Element machine = new Element("stateMachine");
		machine.setAttribute("NAME", fileName);
		tree.addStateMachine1(machine);
		undoNewStateMachine.add(machine);
		stateMachinesList.add(file);
		viewer.add(fileName);
		allNames.add(fileName);
		dirty = true;
	}
	
	/**
	 * deletes all new statemachines if StateMachinesList is not saved.
	 * New statemachines with an open modified Editor can still be saved
	 * when the editor closes.
	 */
	private void undoCreateNewStateMachine() {
		ReadDOMTree tree = DOMGetInstance.getInstance();
		for (int i = 0; i < undoNewStateMachine.size(); i++) {
			Element machineEle = (Element) undoNewStateMachine.get(i);
			tree.removeStateMachine1(machineEle.getAttributeValue("NAME"));
		}
	}
	
	/**
	 * changes the name of a statemachine in the view
	 * @param oldName old name to remove from list
	 * @param newName new name to add to list
	 */
	public static void changeStateMachineName(String oldName, String newName) {
		for (int i = 0; i < stateMachinesList.size(); i++) {
			IFile name = (IFile) stateMachinesList.get(i);
			if (name.getName().equals(oldName + ".states")) {
				stateMachinesList.remove(i);
				final IFile file1 = DOMGetInstance.getContainer().getFile(new Path(newName + ".states"));
				stateMachinesList.add(file1);
				viewer.remove(oldName);
				viewer.add(newName);
				break;
			}
		}
	}
	
	/* (non-Javadoc)
	 * @see org.eclipse.ui.IWorkbenchPart#createPartControl(org.eclipse.swt.widgets.Composite)
	 */
	public void createPartControl(Composite parent) {
		viewer = new ListViewer(parent);
		viewer.setSorter(new ViewerSorter());
		for (int i = 0; i < stateMachinesList.size(); i++) {
			machineName = (IFile) stateMachinesList.get(i);
			fileName = machineName.getName();
			int length = fileName.length();
			fileName = fileName.subSequence(0, length-7).toString();
			viewer.add(fileName);
		}
		viewer.addDoubleClickListener(this);
		createActions();
		createContextMenu();	
	}

	/* (non-Javadoc)
	 * @see org.eclipse.ui.IWorkbenchPart#setFocus()
	 */
	public void setFocus() {
		viewer.getControl().setFocus();
	}
	
	/**
	 * clears all lists in this view
	 */
	public static void reset() {
		dirty = false;
		for (int i = 0; i < stateMachinesList.size(); i++) {
			machineName = (IFile) stateMachinesList.get(i);
			fileName = machineName.getName();
			int length = fileName.length();
			fileName = fileName.subSequence(0, length-7).toString();
			viewer.remove(fileName);
		}
		stateMachinesList.clear();
		allNames.clear();
		undoList.clear();
		redoList.clear();
		undoAction.setEnabled(false);
		undoAction.setText("Undo");
		redoAction.setEnabled(false);
		redoAction.setText("Redo");
	}

	/* (non-Javadoc)
	 * @see org.eclipse.jface.viewers.IDoubleClickListener#doubleClick(org.eclipse.jface.viewers.DoubleClickEvent)
	 */
	public void doubleClick(DoubleClickEvent event) {
		// opens the selected statemachine editor if not in debug-mode
		if (!(debugMode)) {
			fileName = (String) ((IStructuredSelection)viewer.getSelection()).getFirstElement();
			for (int i = 0; i < stateMachinesList.size(); i++) {
				machineName = (IFile) stateMachinesList.get(i);
				if (machineName.getName().equals(fileName + ".states")) {
					break;
				}
			}
			try {
				Element machine = null;
				ReadDOMTree tree = DOMGetInstance.getInstance();
				List allMachines = tree.getStateMachines();
				for (int i = 0; i < allMachines.size(); i++) {
					machine = (Element) allMachines.get(i);
					if (machine.getAttributeValue("NAME").equals(fileName)) {
						break;
					}
				}
				if (!(machine == null)) {
					StateMachinesDiagram diagram = new StateMachinesDiagram(machine);
					tree.addDiagram(diagram);
					IDE.openEditor(PlatformUI.getWorkbench().getActiveWorkbenchWindow().getActivePage(), machineName, true);
				}
			} catch (PartInitException e) {
				e.printStackTrace();
			}
		}
	}
	
	/**
	 * creates the context menu for this view
	 */
	private void createContextMenu() {
		//create menu manager
		MenuManager menuMgr = new MenuManager();
		menuMgr.setRemoveAllWhenShown(true);
		menuMgr.addMenuListener(new IMenuListener() {
			public void menuAboutToShow(IMenuManager mgr) {
				fillContextMenu(mgr);
			}
		});
		
		// Create menu
		Menu menu = menuMgr.createContextMenu(viewer.getControl());
		viewer.getControl().setMenu(menu);
		
		// Register menu for extension
		getSite().registerContextMenu(menuMgr, viewer);
	}
	
	/**
	 * fills the context menu with actions
	 * @param mgr the IMenuManager
	 */
	private void fillContextMenu(IMenuManager mgr) {
		mgr.add(new GroupMarker(IWorkbenchActionConstants.MB_ADDITIONS));
		mgr.add(undoAction);
		mgr.add(redoAction);
		mgr.add(new Separator());
		mgr.add(deleteItemAction);
		mgr.add(copyItemAction);
		mgr.add(new Separator("Additions"));
	}
	
	/**
	 * creates the actions for the context menu
	 */
	private void createActions() {
		undoAction = new Action("Undo") {
			public void run() {
				undo();
			}
		};
		undoAction.setEnabled(false);
		Image image = PlatformUI.getWorkbench().getSharedImages().getImage(ISharedImages.IMG_TOOL_UNDO);
		undoAction.setImageDescriptor(ImageDescriptor.createFromImage(image));
		
		
		redoAction = new Action("Redo") {
			public void run() {
				redo();
			}
		};
		redoAction.setEnabled(false);
		Image imag = PlatformUI.getWorkbench().getSharedImages().getImage(ISharedImages.IMG_TOOL_REDO);
		redoAction.setImageDescriptor(ImageDescriptor.createFromImage(imag));
		
		
		deleteItemAction = new Action("Delete") {
			public void run() {
				deleteItem();
			}
		};
		deleteItemAction.setEnabled(false);
		Image ima = PlatformUI.getWorkbench().getSharedImages().getImage(ISharedImages.IMG_TOOL_DELETE);
		deleteItemAction.setImageDescriptor(ImageDescriptor.createFromImage(ima));

		copyItemAction = new Action("Copy") {
			public void run() {
				copyItem();
			}
		};
		copyItemAction.setEnabled(false);
		Image i = PlatformUI.getWorkbench().getSharedImages().getImage(ISharedImages.IMG_TOOL_COPY);
		copyItemAction.setImageDescriptor(ImageDescriptor.createFromImage(i));
		
		// Add selection listener.
		viewer.addSelectionChangedListener(new ISelectionChangedListener() {
			public void selectionChanged(SelectionChangedEvent event) {
				updateActionEnablement();
			}
		});
	}
	
	/**
	 * updates whether an actionitem is selectable
	 */
	private void updateActionEnablement() {
		if (debugMode) {
			deleteItemAction.setEnabled(false);
			copyItemAction.setEnabled(false);
		}
		else {
			IStructuredSelection sel = (IStructuredSelection)viewer.getSelection();
			deleteItemAction.setEnabled(sel.size() > 0 && sel.size() < 2);
			copyItemAction.setEnabled(sel.size() > 0 && sel.size() < 2);
		}
	}
	
	/**
	 * the undo functionality for this view
	 */
	private void undo() {
		// undo delete statemachine
		if (undoList.get(undoList.size()-1) instanceof DeleteStateMachine) {
			DeleteStateMachine delete = (DeleteStateMachine) undoList.get(undoList.size()-1);
			delete.undo();
			redoAction.setText("Redo " + DELETE);
			redoList.add(delete);
			redoAction.setEnabled(true);
			undoList.remove(undoList.size()-1);
		}
		// undo create statemachine
		else if (undoList.get(undoList.size()-1) instanceof CopyStateMachine) {
			CopyStateMachine copy = (CopyStateMachine) undoList.get(undoList.size()-1);
			copy.undo();
			redoAction.setText("Redo " + COPY);
			redoList.add(copy);
			redoAction.setEnabled(true);
			undoList.remove(undoList.size()-1);
		}
		if (undoList.size() == 0) {
			dirty = false;
			undoAction.setEnabled(false);
			undoAction.setText("Undo");
		}
		else if (undoList.get(undoList.size()-1) instanceof DeleteStateMachine){
			undoAction.setText("Undo " + DELETE);
		}
		else if (undoList.get(undoList.size()-1) instanceof CopyStateMachine){
			undoAction.setText("Undo " + COPY);
		}
	}
	
	/**
	 * the redo functionality for this view
	 */
	private void redo() {
		// redo delete statemachine
		if (redoList.get(redoList.size()-1) instanceof DeleteStateMachine) {
			DeleteStateMachine delete = (DeleteStateMachine) redoList.get(redoList.size()-1);
			delete.redo();
			undoAction.setText("Undo " + DELETE);
			undoList.add(delete);
			undoAction.setEnabled(true);
			redoList.remove(redoList.size()-1);
		}
		// redo copy statemachine
		else if (redoList.get(redoList.size()-1) instanceof CopyStateMachine) {
			CopyStateMachine copy = (CopyStateMachine) redoList.get(redoList.size()-1);
			copy.redo();
			undoAction.setText("Undo " + COPY);
			undoList.add(copy);
			undoAction.setEnabled(true);
			redoList.remove(redoList.size()-1);
		}
		if (redoList.size() == 0) {
			redoAction.setEnabled(false);
			redoAction.setText("Redo");
		}
		else if (redoList.get(redoList.size()-1) instanceof DeleteStateMachine){
			redoAction.setText("Redo " + DELETE);
		}
		else if (redoList.get(redoList.size()-1) instanceof CopyStateMachine){
			redoAction.setText("Redo " + COPY);
		}
	}
	
	/**
	 * Remove item from list.
	 */
	private void deleteItem() {
		fileName = (String) ((IStructuredSelection)viewer.getSelection()).getFirstElement();
		DeleteStateMachine delete = new DeleteStateMachine(fileName, DOMGetInstance.getContainer());
		if (delete.execute()) {
			dirty = true;
			redoList.clear();
			redoAction.setEnabled(false);
			redoAction.setText("Redo");
			undoList.add(delete);
			undoAction.setText("Undo " + DELETE);
			undoAction.setEnabled(true);
		}
	}
	
	/**
	 * makes a copy of the selected statemachine and saves it under an entered name
	 */
	private void copyItem() {
		String machine = (String) ((IStructuredSelection)viewer.getSelection()).getFirstElement();
		String name = promptForValue("Enter name for copy:", null);
		boolean exist = false;
		for (int j = 0; j < allNames.size(); j++) {
			if (allNames.get(j).toString().equals(name)) {
				exist = true;
				break;
			}
		}
		if (!(name == null) && !(name.equals("")) && exist == false) {
			dirty = true;
			redoList.clear();
			redoAction.setEnabled(false);
			redoAction.setText("Redo");
			CopyStateMachine copy = new CopyStateMachine(machine, name, DOMGetInstance.getContainer());
			undoList.add(copy);
			copy.execute();
			undoAction.setEnabled(true);
			undoAction.setText("Undo " + COPY);
		}
		else if (name == null || name.equals("")) {
			JOptionPane.showMessageDialog(null, "You have to enter a name for the new statemachine!",
					"Error Message", JOptionPane.ERROR_MESSAGE);
		}
		else if (exist == true) {
			JOptionPane.showMessageDialog(null, "Name already exists!",
					"Error Message", JOptionPane.ERROR_MESSAGE);
		}
	}
	
	/**
	 * input dialog
	 * @param text the text which the dialog should show
	 * @param oldValue the old value which should be changed
	 * @return the new value
	 */
	private String promptForValue(String text, String oldValue) {
		InputDialog dlg = new InputDialog(getSite().getShell(), 
			"List View", text, oldValue, null);
		if (dlg.open() != Window.OK)
			return null;
		return dlg.getValue();
	}
	
	/* (non-Javadoc)
	 * @see org.eclipse.ui.ISaveablePart#doSave(org.eclipse.core.runtime.IProgressMonitor)
	 */
	public void doSave(IProgressMonitor monitor) {
		isSaved = true;
		ReadDOMTree tree = DOMGetInstance.getInstance();
		tree.writeTree();
		undoList.clear();
		redoList.clear();
		undoAction.setEnabled(false);
		undoAction.setText("Undo");
		redoAction.setEnabled(false);
		redoAction.setText("Redo");
		dirty = false;
	}
	
	/* (non-Javadoc)
	 * @see org.eclipse.ui.ISaveablePart#doSaveAs()
	 */
	public void doSaveAs() {
		Shell shell = PlatformUI.getWorkbench().getActiveWorkbenchWindow().getShell();
		FileDialog fileDialog = new FileDialog(shell, SWT.SAVE);
		fileDialog.setFilterExtensions(new String[] {"*.xml"});
		fileDialog.open();
		String file2 = fileDialog.getFilterPath().toString() + File.separator + fileDialog.getFileName().toString();
		isSaved = true;
		ReadDOMTree tree = DOMGetInstance.getInstance();
		tree.writeTree(file2.toString());
		undoList.clear();
		redoList.clear();
		undoAction.setEnabled(false);
		undoAction.setText("Undo");
		redoAction.setEnabled(false);
		redoAction.setText("Redo");
		dirty = false;
	}

	/* (non-Javadoc)
	 * @see org.eclipse.ui.ISaveablePart#isDirty()
	 */
	public boolean isDirty() {
		if (dirty) {
			return true;
		}
		return false;
	}

	/* (non-Javadoc)
	 * @see org.eclipse.ui.ISaveablePart#isSaveAsAllowed()
	 */
	public boolean isSaveAsAllowed() {
		return true;
	}

	/* (non-Javadoc)
	 * @see org.eclipse.ui.ISaveablePart#isSaveOnCloseNeeded()
	 */
	public boolean isSaveOnCloseNeeded() {
		// when eclipse is closing, close this view and ask separate for save, then return false
		if (PlatformUI.getWorkbench().isClosing() && (workbenchIsClosing == false)) {
			workbenchIsClosing = true;
			IWorkbenchPage page1 = PlatformUI.getWorkbench().getActiveWorkbenchWindow().getActivePage(); 
			page1.hideView(this);
			return false;
		}
		else if (workbenchIsClosing) {
			return false;
		}
		if (dirty) {
			return true;
		}
		return false;
	}
	
	/* (non-Javadoc)
	 * @see org.eclipse.ui.IWorkbenchPart#dispose()
	 */
	public void dispose() {
		// close all editors when closing this view if eclipse remains open
		if (!PlatformUI.getWorkbench().isClosing()) {
			PlatformUI.getWorkbench().getActiveWorkbenchWindow().getActivePage().closeAllEditors(true);
		}
		// if eclipse is closing and this view is dirty => ask for save
		if (PlatformUI.getWorkbench().isClosing()) {
			if (dirty) {
				int result = JOptionPane.showConfirmDialog(null,"'StateMachinesList' has been modified. Save changes?", "Save Resource", JOptionPane.YES_NO_OPTION);
				if (result == JOptionPane.YES_OPTION) {
					doSave(null);
				}
			}
		}
		// if not saved undo every change, reset every list and close
		if (!(isSaved)) {
			int listSize = undoList.size();
			for(int i = 0; i < listSize; i++) {
				if (undoList.get(i) instanceof DeleteStateMachine) {
					DeleteStateMachine delete = (DeleteStateMachine) undoList.get(i);
					delete.undoForDispose();
				}
				else if (undoList.get(i) instanceof CopyStateMachine) {
					CopyStateMachine copy = (CopyStateMachine) undoList.get(i);
					copy.undoForDispose();
				}
			}
			undoCreateNewStateMachine();
		}
		isSaved = false;
		workbenchIsClosing = false;
		stateMachinesList.clear();
		allNames.clear();
		undoList.clear();
		redoList.clear();
		undoAction.setEnabled(false);
		undoAction.setText("Undo");
		redoAction.setEnabled(false);
		redoAction.setText("Redo");
		// reset the DOMTree
		if (!PlatformUI.getWorkbench().isClosing()) {
			DOMGetInstance.reset();
		}
	}
	
	public static void setDebugMode(boolean debugMode1) {
		debugMode = debugMode1;
		//PlatformUI.getWorkbench().getActiveWorkbenchWindow().getActivePage().closeAllEditors(true);
		if (debugMode) {
			undoAction.setEnabled(false);
			redoAction.setEnabled(false);
		}
		else {
			if (undoList.size() > 0) {
				undoAction.setEnabled(true);
			}
			if (redoList.size() > 0) {
				redoAction.setEnabled(true);
			}
		}
	}
	
	public static boolean isDebugMode() {
		return debugMode;
	}
}
