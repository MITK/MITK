package actions;

import java.util.HashMap;
import java.util.List;
import java.util.Map;

import model.Connection;

import org.eclipse.gef.EditPart;
import org.eclipse.gef.Request;
import org.eclipse.gef.commands.Command;
import org.eclipse.gef.commands.CompoundCommand;
import org.eclipse.gef.ui.actions.SelectionAction;
import org.eclipse.ui.IWorkbenchPart;

public class ChangeEvent extends SelectionAction {
	
	private static final String CHANGE_EVENT_REQUEST = "Change event";  //$NON-NLS-1$
	
	public static final String CHANGE_EVENT = "Change event";   //$NON-NLS-1$
	
	private Map map = null;
	
	Request request;

	/**
	 * constructor to change an event
	 * @param part the IWorkbenchPart
	 */
	public ChangeEvent(IWorkbenchPart part) {
		super(part);
		request = new Request(CHANGE_EVENT_REQUEST);
		setId(CHANGE_EVENT);
		setText("Change event");
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
			// add connection to map
			else {
				Connection con = (Connection) part.getModel();
				map = new HashMap();
				map.put("1", con);
			}
		}
		return true;
	}

	private Command getCommand() {
		List editparts = getSelectedObjects();
		request.setExtendedData(map);
		CompoundCommand cc = new CompoundCommand();
		cc.setDebugLabel("Change event");//$NON-NLS-1$
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
		execute(getCommand());
	}
}