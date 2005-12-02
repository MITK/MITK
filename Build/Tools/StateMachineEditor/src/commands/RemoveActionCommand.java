package commands;

import model.Action;
import model.Connection;

import org.eclipse.gef.commands.Command;

public class RemoveActionCommand extends Command {
	
	private final Connection con;
	private Action act;
	
	/**
	 * Instantiate a command that removes an action
	 * @param con connection on which to remove the action
	 * @param act action to be removed
	 */
	public RemoveActionCommand(Connection con, Action act){
		if (con == null) {
			throw new IllegalArgumentException();
		}
		setLabel("Remove action");
		this.con = con;
		this.act = act;
	}
	
	/**
	 * removes the action from the connection
	 * 
	 * @see org.eclipse.gef.commands.Command#execute()
	 */
	public void execute(){
		con.removeAction(act, act.getActionId());		
	}
	
	/* (non-Javadoc)
	 * @see org.eclipse.gef.commands.Command#undo()
	 */
	public void undo(){
		con.addAction(act, act.getActionId());
	}
	
	/* (non-Javadoc)
	 * @see org.eclipse.gef.commands.Command#redo()
	 */
	public void redo(){
		con.removeAction(act, act.getActionId());
	}
}
