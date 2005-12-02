package parts;

import java.util.Map;

import model.Action;
import model.Connection;

import org.eclipse.gef.Request;
import org.eclipse.gef.commands.Command;
import org.eclipse.gef.editpolicies.ComponentEditPolicy;


import commands.AddActionCommand;
import commands.ChangeActionCommand;
import commands.ChangeEventCommand;
import commands.RemoveActionCommand;
import dialog.ActionDialog;
import dialog.ActionAndEventInfo;
import dialog.EventDialog;

public class ActionAndEventEditPolicy extends ComponentEditPolicy {
	
	private static final String CHANGE_ACTION_REQUEST = "Change action";  //$NON-NLS-1$
	private static final String ADD_ACTION_REQUEST = "Add action";  //$NON-NLS-1$
	private static final String REMOVE_ACTION_REQUEST = "Remove action";  //$NON-NLS-1$
	private static final String CHANGE_EVENT_REQUEST = "Change event";  //$NON-NLS-1$
	
	/* (non-Javadoc)
	 * @see org.eclipse.gef.EditPolicy#getCommand(org.eclipse.gef.Request)
	 */
	public Command getCommand(Request request) {
		if (CHANGE_ACTION_REQUEST.equals(request.getType())) {
			// map contains the connection to which the action belongs
			Map map = request.getExtendedData();
			ActionDialog dlg = new ActionDialog((Action) map.get("1"));
			dlg.setVisible(true);
			ActionAndEventInfo info = dlg.getInfo();
			if (info == null || info.getActionEventName() == null) {
				return null;
			}
			return getChangeActionCommand((Action) map.get("1"), info);
		}
		if (ADD_ACTION_REQUEST.equals(request.getType())) {
			ActionDialog dlg = new ActionDialog();
			dlg.setVisible(true);
			ActionAndEventInfo info = dlg.getInfo();
			if (info == null || info.getActionEventName() == null) {
				return null;
			}
			return getAddActionCommand(info);
		}
		if (REMOVE_ACTION_REQUEST.equals(request.getType())) {
			// map contains the connection to which the action belongs
			Map map = request.getExtendedData();
			return getRemoveActionCommand((Action) map.get("1"));
		}
		if (CHANGE_EVENT_REQUEST.equals(request.getType())) {
			// map contains the connection to which the event belongs
			Map map = request.getExtendedData();
			EventDialog dlg = new EventDialog((Connection)map.get("1"));
			dlg.setVisible(true);
			ActionAndEventInfo info = dlg.getInfo();
			if (info == null || info.getActionEventName() == null) {
				return null;
			}
			return getChangeEventCommand(info);
		}
		return super.getCommand(request);
	}
	
	/**
	 * creates a new command to change an action
	 * @param act the action which should be changed
	 * @param info the info object with the new values
	 * @return the command
	 */
	protected Command getChangeActionCommand(Action act, ActionAndEventInfo info){
		Connection con = (Connection) getHost().getModel();
		ChangeActionCommand command = new ChangeActionCommand(con, act, info);
		return command;
	}

	/**
	 * creates a new command to add an action
	 * @param info the info object with the new values
	 * @return the command
	 */
	protected Command getAddActionCommand(ActionAndEventInfo info){
		Connection con = (Connection) getHost().getModel();
		AddActionCommand command = new AddActionCommand(con, info);
		return command;
	}
	
	/**
	 * creates a new command to remove an action
	 * @param act the action which should be changed
	 * @return the command
	 */
	protected Command getRemoveActionCommand(Action act){
		Connection con = (Connection) getHost().getModel();
		RemoveActionCommand command = new RemoveActionCommand(con, act);
		return command;
	}
	
	/**
	 * creates a new command to change an event
	 * @param info the info object with the new values
	 * @return the command
	 */
	protected Command getChangeEventCommand(ActionAndEventInfo info){
		Connection con = (Connection) getHost().getModel();
		ChangeEventCommand command = new ChangeEventCommand(con, info);
		return command;
	}
}
