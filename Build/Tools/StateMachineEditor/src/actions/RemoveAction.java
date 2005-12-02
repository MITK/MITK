package actions;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

import model.Action;
import model.Connection;

import org.eclipse.gef.EditPart;
import org.eclipse.gef.Request;
import org.eclipse.gef.commands.Command;
import org.eclipse.gef.commands.CompoundCommand;
import org.eclipse.gef.ui.actions.SelectionAction;
import org.eclipse.ui.IWorkbenchPart;

import stateMachines.StateMachinesEditorContextMenuProvider;

public class RemoveAction extends SelectionAction {
	
	private static final String REMOVE_ACTION_REQUEST = "Remove action";  //$NON-NLS-1$
	
	public static final String REMOVE_ACTION = "Remove action";   //$NON-NLS-1$
	
	Request request;
	
	public static List actionList = new ArrayList();

	/**
	 * constructor to remove an action
	 * @param part the IWorkbenchPart
	 */
	public RemoveAction(IWorkbenchPart part) {
		super(part);
		request = new Request(REMOVE_ACTION_REQUEST);
		setId(REMOVE_ACTION);
		setText("Remove action");
	}

	/* (non-Javadoc)
	 * @see org.eclipse.gef.ui.actions.WorkbenchPartAction#calculateEnabled()
	 */
	protected boolean calculateEnabled() {
		return canPerformAction();
	}

	/**
	 * @return true if selected item instance of connection
	 */
	private boolean canPerformAction() {
		if (getSelectedObjects().isEmpty()) {
			return false;
		}
		List parts = getSelectedObjects();
		for (int i=0; i<parts.size(); i++){
			Object o = parts.get(i);
			if (!(o instanceof EditPart)) {
				return false;
			}
			EditPart part = (EditPart)o;
			if (!(part.getModel() instanceof Connection)) {
				return false;
			}
			if (part.getModel() instanceof Connection) {
				Connection con = (Connection) part.getModel();
				actionList = con.getAllActions();
			}
		}
		return true;
	}

	private Command getCommand(Action act) {
		// add action to map
		Map map = new HashMap();
		map.put("1", act);
		request.setExtendedData(map);
		List editparts = getSelectedObjects();
		CompoundCommand cc = new CompoundCommand();
		cc.setDebugLabel("Remove action");//$NON-NLS-1$
		for (int i=0; i < editparts.size(); i++) {
			EditPart part = (EditPart)editparts.get(i);
			cc.add(part.getCommand(request));
		}
		return cc;
	}

	/* (non-Javadoc)
	 * @see org.eclipse.jface.action.IAction#run()
	 */
	public void run() {
		Action act = (StateMachinesEditorContextMenuProvider.getAction());
		execute(getCommand(act));
	}
	
	/**
	 * @return List with all actions
	 */
	public List actionList() {
		return actionList;
	}
}

