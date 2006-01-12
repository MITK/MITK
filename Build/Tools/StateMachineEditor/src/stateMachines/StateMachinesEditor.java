/*
 * Created on 05.07.2005
 */
package stateMachines;

import java.io.File;
import java.util.ArrayList;
import java.util.EventObject;
import java.util.List;

import model.StateMachinesDiagram;

import org.eclipse.core.resources.IFile;
import org.eclipse.core.runtime.IProgressMonitor;
import org.eclipse.gef.ContextMenuProvider;
import org.eclipse.gef.DefaultEditDomain;
import org.eclipse.gef.EditPartViewer;
import org.eclipse.gef.GraphicalViewer;
import org.eclipse.gef.dnd.TemplateTransferDragSourceListener;
import org.eclipse.gef.dnd.TemplateTransferDropTargetListener;
import org.eclipse.gef.editparts.ScalableFreeformRootEditPart;
import org.eclipse.gef.editparts.ZoomManager;
import org.eclipse.gef.palette.PaletteRoot;
import org.eclipse.gef.requests.CreationFactory;
import org.eclipse.gef.requests.SimpleFactory;
import org.eclipse.gef.ui.actions.ActionRegistry;
import org.eclipse.gef.ui.actions.ZoomInAction;
import org.eclipse.gef.ui.actions.ZoomOutAction;
import org.eclipse.gef.ui.palette.PaletteViewer;
import org.eclipse.gef.ui.palette.PaletteViewerProvider;
import org.eclipse.gef.ui.palette.FlyoutPaletteComposite.FlyoutPreferences;
import org.eclipse.gef.ui.parts.ContentOutlinePage;
import org.eclipse.gef.ui.parts.GraphicalEditorWithFlyoutPalette;
import org.eclipse.gef.ui.parts.GraphicalViewerKeyHandler;
import org.eclipse.gef.ui.parts.ScrollingGraphicalViewer;
import org.eclipse.gef.ui.parts.TreeViewer;
import org.eclipse.jface.action.IAction;
import org.eclipse.jface.util.TransferDropTargetListener;
import org.eclipse.swt.SWT;
import org.eclipse.swt.widgets.Composite;
import org.eclipse.swt.widgets.Control;
import org.eclipse.swt.widgets.FileDialog;
import org.eclipse.swt.widgets.Shell;
import org.eclipse.ui.IActionBars;
import org.eclipse.ui.IEditorInput;
import org.eclipse.ui.IEditorPart;
import org.eclipse.ui.IFileEditorInput;
import org.eclipse.ui.PlatformUI;
import org.eclipse.ui.actions.ActionFactory;
import org.eclipse.ui.part.IPageSite;
import org.eclipse.ui.views.contentoutline.IContentOutlinePage;

import actions.AddAction;
import actions.ChangeAction;
import actions.ChangeEvent;
import actions.RemoveAction;

import parts.StateMachinesEditPartFactory;
import parts.StateMachinesTreeEditPartFactory;
import stateMachinesList.StateMachinesList;
import debug.DebugEventsList;
import dom.DOMGetInstance;
import dom.ReadDOMTree;

/**
 * @author Daniel
 */
public class StateMachinesEditor extends GraphicalEditorWithFlyoutPalette {
	
	private boolean saved = false;
	
	/** This is the root of the editor's model. */
	private StateMachinesDiagram diagram;

	/** Palette component, holding the tools and states. */
	private static PaletteRoot PALETTE_MODEL;

	/** Create a new StateMachinesEditor instance. This is called by the Workspace. */
	public StateMachinesEditor() {
		setEditDomain(new DefaultEditDomain(this));
	}

	/**
	 * Configure the graphical viewer before it receives contents.
	 * <p>
	 * This is the place to choose an appropriate RootEditPart and
	 * EditPartFactory for your editor. The RootEditPart determines the behavior
	 * of the editor's "work-area". For example, GEF includes zoomable and
	 * scrollable root edit parts. The EditPartFactory maps model elements to
	 * edit parts (controllers).
	 * </p>
	 * 
	 * @see org.eclipse.gef.ui.parts.GraphicalEditor#configureGraphicalViewer()
	 */
	protected void configureGraphicalViewer() {
		super.configureGraphicalViewer();
		ScrollingGraphicalViewer viewer = (ScrollingGraphicalViewer)getGraphicalViewer();

		ScalableFreeformRootEditPart root = new ScalableFreeformRootEditPart();
		
		// adding all actions to the editor
		List zoomLevels = new ArrayList(3);
		zoomLevels.add(ZoomManager.FIT_ALL);
		zoomLevels.add(ZoomManager.FIT_WIDTH);
		zoomLevels.add(ZoomManager.FIT_HEIGHT);
		root.getZoomManager().setZoomLevelContributions(zoomLevels);

		IAction zoomIn = new ZoomInAction(root.getZoomManager());
		IAction zoomOut = new ZoomOutAction(root.getZoomManager());
		IAction changeAction = new ChangeAction(this);
		IAction addAction = new AddAction(this);
		IAction removeAction = new RemoveAction(this);
		IAction changeEvent = new ChangeEvent(this);
		getActionRegistry().registerAction(zoomIn);
		getActionRegistry().registerAction(zoomOut);
		getActionRegistry().registerAction(changeAction);
		getActionRegistry().registerAction(addAction);
		getActionRegistry().registerAction(removeAction);
		getActionRegistry().registerAction(changeEvent);
		getSite().getKeyBindingService().registerAction(zoomIn);
		getSite().getKeyBindingService().registerAction(zoomOut);
		getSite().getKeyBindingService().registerAction(changeAction);
		getSite().getKeyBindingService().registerAction(addAction);
		getSite().getKeyBindingService().registerAction(removeAction);
		getSite().getKeyBindingService().registerAction(changeEvent);
		getSelectionActions().add(changeAction.getId());
		getSelectionActions().add(addAction.getId());
		getSelectionActions().add(removeAction.getId());
		getSelectionActions().add(changeEvent.getId());
		
		viewer.setEditPartFactory(new StateMachinesEditPartFactory());
		viewer.setKeyHandler(new GraphicalViewerKeyHandler(viewer));
		viewer.setRootEditPart(root);

		// configure the context menu provider
		ContextMenuProvider cmProvider = new StateMachinesEditorContextMenuProvider(
				viewer, getActionRegistry());
		viewer.setContextMenu(cmProvider);
		getSite().registerContextMenu(cmProvider, viewer);
	}
	
	/*
	 * (non-Javadoc)
	 * 
	 * @see org.eclipse.gef.ui.parts.GraphicalEditor#commandStackChanged(java.util.EventObject)
	 */
	
	public void commandStackChanged(EventObject event) {
		firePropertyChange(IEditorPart.PROP_DIRTY);
		super.commandStackChanged(event);
	}

	/*
	 * (non-Javadoc)
	 * 
	 * @see org.eclipse.gef.ui.parts.GraphicalEditorWithFlyoutPalette#createPaletteViewerProvider()
	 */
	
	protected PaletteViewerProvider createPaletteViewerProvider() {
		return new PaletteViewerProvider(getEditDomain()) {
			protected void configurePaletteViewer(PaletteViewer viewer) {
				super.configurePaletteViewer(viewer);
				// create a drag source listener for this palette viewer
				// together with an appropriate transfer drop target listener,
				// this will enable
				// model element creation by dragging a
				// CombinatedTemplateCreationEntries
				// from the palette into the editor
				// @see StateMachinesEditor#createTransferDropTargetListener()
				viewer.addDragSourceListener(new TemplateTransferDragSourceListener(
								viewer));
			}
		};
	}

	/**
	 * Create a transfer drop target listener. When using a
	 * CombinedTemplateCreationEntry tool in the palette, this will enable model
	 * element creation by dragging from the palette.
	 * 
	 * @see #createPaletteViewerProvider()
	 */
	
	private TransferDropTargetListener createTransferDropTargetListener() {
		return new TemplateTransferDropTargetListener(getGraphicalViewer()) {
			protected CreationFactory getFactory(Object template) {
				return new SimpleFactory((Class) template);
			}
		};
	}
	
	/*
	 * (non-Javadoc)
	 * 
	 * @see org.eclipse.ui.ISaveablePart#doSave(org.eclipse.core.runtime.IProgressMonitor)
	 */
	
	public void doSave(IProgressMonitor monitor) {
		saved = true;
		getCommandStack().markSaveLocation();
		ReadDOMTree tree = DOMGetInstance.getInstance();
		diagram.removeFromRoot(tree);
		diagram.addToRoot(tree);
		tree.writeTree();
	}
	
	/**
	 * @return the StateMachinesDiagram for this editor
	 */
	public StateMachinesDiagram getDiagram() {
		return diagram;
	}

	/*
	 * (non-Javadoc)
	 * 
	 * @see org.eclipse.ui.ISaveablePart#doSaveAs()
	 */
	
	public void doSaveAs() {
		saved = true;
		Shell shell = PlatformUI.getWorkbench().getActiveWorkbenchWindow().getShell();
		FileDialog fileDialog = new FileDialog(shell, SWT.SAVE);
		fileDialog.setFilterExtensions(new String[] {"*.xml"});
		fileDialog.open();
		String file2 = fileDialog.getFilterPath().toString() + File.separator + fileDialog.getFileName().toString();
		getCommandStack().markSaveLocation();
		ReadDOMTree tree = DOMGetInstance.getInstance();
		diagram.removeFromRoot(tree);
		diagram.addToRoot(tree);
		tree.writeTree(file2);
	}

	public Object getAdapter(Class type) {
		if (type == ZoomManager.class)
			return getGraphicalViewer().getProperty(ZoomManager.class.toString());
		if (type == IContentOutlinePage.class)
			return new StateMachinesOutlinePage(new TreeViewer());
		return super.getAdapter(type);
	}

	StateMachinesDiagram getModel() {
		return diagram;
	}

	/*
	 * (non-Javadoc)
	 * 
	 * @see org.eclipse.gef.ui.parts.GraphicalEditorWithFlyoutPalette#getPalettePreferences()
	 */
	
	protected FlyoutPreferences getPalettePreferences() {
		return StateMachinesEditorPaletteFactory.createPalettePreferences();
	}

	/*
	 * (non-Javadoc)
	 * 
	 * @see org.eclipse.gef.ui.parts.GraphicalEditorWithFlyoutPalette#getPaletteRoot()
	 */
	
	protected PaletteRoot getPaletteRoot() {
		if (PALETTE_MODEL == null)
			PALETTE_MODEL = StateMachinesEditorPaletteFactory.createPalette();
		return PALETTE_MODEL;
	}

	/**
	 * Opens an editor for the selected diagram "stateMachineName"
	 * @param stateMachineName Name of statemachine in this diagram
	 */
	
	private void openEditor(String stateMachineName) {
		ReadDOMTree tree = DOMGetInstance.getInstance();
		// open diagram in editor
		List allDiagrams = tree.getAllDiagrams();
		for (int j = 0; j < allDiagrams.size(); j++) {
			StateMachinesDiagram dia = (StateMachinesDiagram) allDiagrams.get(j);
			if (dia.getStateMachineName().equals(stateMachineName)) {
				diagram = dia;
				diagram.setEditor(this);
				break;
			}
		}
		setPartName1(stateMachineName);
	}
	
	public void setPartName1(String name) {
		setPartName(name);
	}
	
	/**
	 * Set up the editor's inital content (after creation).
	 * 
	 * @see org.eclipse.gef.ui.parts.GraphicalEditorWithFlyoutPalette#initializeGraphicalViewer()
	 */
	
	protected void initializeGraphicalViewer() {
		super.initializeGraphicalViewer();
		GraphicalViewer viewer = getGraphicalViewer();
		// set the contents of this editor
		viewer.setContents(getModel()); 
		// listen for dropped parts
		viewer.addDropTargetListener(createTransferDropTargetListener());
	}

	/*
	 * (non-Javadoc)
	 * 
	 * @see org.eclipse.ui.ISaveablePart#isSaveAsAllowed()
	 */
	
	public boolean isSaveAsAllowed() {
		return true;
	}

	/*
	 * (non-Javadoc)
	 * 
	 * @see org.eclipse.ui.part.EditorPart#setInput(org.eclipse.ui.IEditorInput)
	 */
	protected void setInput(IEditorInput input) {
		super.setInput(input);
		IFile file = ((IFileEditorInput) input).getFile();
		// get filename without .states extension
		String file1 = file.getName().toString();
		int length = file1.length();
		String filename = file1.subSequence(0, length-7).toString();
		openEditor(filename);
	}
	
	public void dispose() {
		if (!saved) {
			diagram.changeToOldName();
		}
		if (StateMachinesList.isDebugMode()) {
			DebugEventsList.closeView();
		}
		ReadDOMTree tree = DOMGetInstance.getInstance();
		tree.removeDiagram(diagram);
	}

	/**
	 * Creates an outline pagebook for this editor.
	 */
	
	public class StateMachinesOutlinePage extends ContentOutlinePage {
		/**
		 * Create a new outline page for the GetDOMStateMachines editor.
		 * 
		 * @param viewer
		 *            a viewer (TreeViewer instance) used for this outline page
		 * @throws IllegalArgumentException
		 *             if editor is null
		 */
		
		public StateMachinesOutlinePage(EditPartViewer viewer) {
			super(viewer);
		}

		/*
		 * (non-Javadoc)
		 * 
		 * @see org.eclipse.ui.part.IPage#createControl(org.eclipse.swt.widgets.Composite)
		 */
		
		public void createControl(Composite parent) {
			// create outline viewer page
			getViewer().createControl(parent);
			// configure outline viewer
			getViewer().setEditDomain(getEditDomain());
			getViewer().setEditPartFactory(
					new StateMachinesTreeEditPartFactory());
			// configure & add context menu to viewer
			ContextMenuProvider cmProvider = new StateMachinesEditorContextMenuProvider(
					getViewer(), getActionRegistry());
			getViewer().setContextMenu(cmProvider);
			getSite().registerContextMenu("stateMachines.outline.contextmenu",
					cmProvider, getSite().getSelectionProvider());
			// hook outline viewer
			getSelectionSynchronizer().addViewer(getViewer());
			// initialize outline viewer with model
			getViewer().setContents(getModel());
			// show outline viewer
		}

		/*
		 * (non-Javadoc)
		 * 
		 * @see org.eclipse.ui.part.IPage#dispose()
		 */
		
		public void dispose() {
			// unhook outline viewer
			getSelectionSynchronizer().removeViewer(getViewer());
			// dispose
			super.dispose();
		}

		/*
		 * (non-Javadoc)
		 * 
		 * @see org.eclipse.ui.part.IPage#getControl()
		 */
		
		public Control getControl() {
			return getViewer().getControl();
		}

		/**
		 * @see org.eclipse.ui.part.IPageBookViewPage#init(org.eclipse.ui.part.IPageSite)
		 */
		
		public void init(IPageSite pageSite) {
			super.init(pageSite);
			ActionRegistry registry = getActionRegistry();
			IActionBars bars = pageSite.getActionBars();
			String id = ActionFactory.UNDO.getId();
			bars.setGlobalActionHandler(id, registry.getAction(id));
			id = ActionFactory.REDO.getId();
			bars.setGlobalActionHandler(id, registry.getAction(id));
			id = ActionFactory.DELETE.getId();
			bars.setGlobalActionHandler(id, registry.getAction(id));
		}
	}
}