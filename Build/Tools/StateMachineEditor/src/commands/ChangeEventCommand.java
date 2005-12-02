package commands;

import model.Connection;

import org.eclipse.gef.commands.Command;

import dialog.ActionAndEventInfo;


public class ChangeEventCommand extends Command {
		
	private final Connection con;
	private String oldEvent;
	private String newEvent;
	
	/**
	 * Instantiate a command that changes an event
	 * @param con connection on which to change the event
	 * @param info object which holds comment, name and id from the new event
	 */
	public ChangeEventCommand(Connection con, ActionAndEventInfo info){
		if (con == null) {
			throw new IllegalArgumentException();
		}
		setLabel("Change event");
		this.con = con;
		newEvent = info.getActionEventName();
	}
	
	/**
	 * changes the event on the connection
	 * 
	 * @see org.eclipse.gef.commands.Command#execute()
	 */
	public void execute(){
		oldEvent = con.getEvent();
		con.setEvent(newEvent);
	}
	
	/* (non-Javadoc)
	 * @see org.eclipse.gef.commands.Command#undo()
	 */
	public void undo(){
		con.setEvent(oldEvent);
	}
	
	/* (non-Javadoc)
	 * @see org.eclipse.gef.commands.Command#redo()
	 */
	public void redo(){
		con.setEvent(newEvent);
	}
}
