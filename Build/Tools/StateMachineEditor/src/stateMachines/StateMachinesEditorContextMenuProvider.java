/*
 * Created on 05.07.2005
 */
package stateMachines;

import java.util.ArrayList;
import java.util.List;

import model.Action;

import org.eclipse.jface.action.IAction;
import org.eclipse.jface.action.IMenuListener;
import org.eclipse.jface.action.IMenuManager;
import org.eclipse.jface.action.MenuManager;
import org.eclipse.ui.actions.ActionFactory;

import org.eclipse.gef.ContextMenuProvider;
import org.eclipse.gef.EditPartViewer;
import org.eclipse.gef.ui.actions.ActionRegistry;
import org.eclipse.gef.ui.actions.GEFActionConstants;

import stateMachinesList.StateMachinesList;

import dom.DOMGetInstance;
import dom.ReadActionAndEventDOMTree;

import actions.AddAction;
import actions.ChangeAction;
import actions.ChangeEvent;
import actions.RemoveAction;

/**
 * @author Daniel
 */
public class StateMachinesEditorContextMenuProvider extends ContextMenuProvider implements IMenuListener {

	/** The editor's action registry. */
	private ActionRegistry actionRegistry;
	private static Action action;
	private List actions = new ArrayList();
	
	private MenuManager submenu = null;
	
	/**
	 * Instantiate a new menu context provider for the specified EditPartViewer
	 * and ActionRegistry.
	 * 
	 * @param viewer
	 *            the editor's graphical viewer
	 * @param registry
	 *            the editor's action registry
	 * @param editor 
	 * @throws IllegalArgumentException
	 *             if registry is <tt>null</tt>.
	 */
	public StateMachinesEditorContextMenuProvider(EditPartViewer viewer,
			ActionRegistry registry) {
		super(viewer);
		if (registry == null) {
			throw new IllegalArgumentException();
		}
		setActionRegistry(registry);
	}

	/**
	 * Called when the context menu is about to show. Actions, whose state is
	 * enabled, will appear in the context menu.
	 * 
	 * @see org.eclipse.gef.ContextMenuProvider#buildContextMenu(org.eclipse.jface.action.IMenuManager)
	 */
	public void buildContextMenu(IMenuManager menu) {
		
		// Add standard action groups to the menu
		GEFActionConstants.addStandardActionGroups(menu);
			
		IAction action;
		if (!(StateMachinesList.isDebugMode())) {	
			action = getActionRegistry().getAction(ActionFactory.UNDO.getId());
			menu.appendToGroup(GEFActionConstants.GROUP_UNDO, action);
			
			action = getActionRegistry().getAction(ActionFactory.REDO.getId());
			menu.appendToGroup(GEFActionConstants.GROUP_UNDO, action);
			
			action = getActionRegistry().getAction(ActionFactory.SAVE.getId());
			menu.appendToGroup(GEFActionConstants.GROUP_SAVE, action);
			
			action = getActionRegistry().getAction(ActionFactory.DELETE.getId());
			if (action.isEnabled())
				menu.appendToGroup(GEFActionConstants.GROUP_EDIT, action);
			
			action = getActionRegistry().getAction(ChangeEvent.CHANGE_EVENT);
			if (action.isEnabled())	
				menu.appendToGroup(GEFActionConstants.GROUP_ADD, action);
			
			action = getActionRegistry().getAction(AddAction.ADD_ACTION);
			if (action.isEnabled())	
				menu.appendToGroup(GEFActionConstants.GROUP_ADD, action);
			
			action = getActionRegistry().getAction(RemoveAction.REMOVE_ACTION);
			if (action.isEnabled()) {
				actions = RemoveAction.actionList;
				if (!(actions.isEmpty())) {
					for (int i = 0; i < actions.size(); i++) {
						Action act = (Action) actions.get(i);
						ReadActionAndEventDOMTree actionTree = DOMGetInstance.getActionAndEventInstance();
						submenu = new MenuManager(actionTree.getActionName(act.getActionId()), Integer.toString(i));
						submenu.addMenuListener(listener);
						action = getActionRegistry().getAction(RemoveAction.REMOVE_ACTION);					
						submenu.add(action);
						action = getActionRegistry().getAction(ChangeAction.CHANGE_ACTION);
						submenu.add(action);
						if (!submenu.isEmpty()) {
							menu.appendToGroup(GEFActionConstants.GROUP_REST, submenu);
						}
					}
				}
			}
		}
	}
	
	
	
	/**
	 * registers the selected action in the context menu
	 */
	IMenuListener listener = new IMenuListener() {
		public void menuAboutToShow(IMenuManager manager) {
			action = (Action) actions.get(Integer.parseInt(manager.getId()));
		}
	};
	
	/**
	 * returns the selected action from the context menu
	 */
	public static Action getAction() {
		return action;
	}
	
	/**
	 * @return the editor's action registry
	 */
	private ActionRegistry getActionRegistry() {
		return actionRegistry;
	}

	/**
	 * @param registry sets the editor's action registry
	 */
	private void setActionRegistry(ActionRegistry registry) {
		actionRegistry = registry;
	}
}