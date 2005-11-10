package commands;

import model.Action;
import model.Connection;

import org.eclipse.gef.commands.Command;

public class RemoveActionCommand extends Command {
	
	private final Connection con;
	private Action act;
	
	public RemoveActionCommand(Connection con, Action act){
		if (con == null) {
			throw new IllegalArgumentException();
		}
		setLabel("Remove action");
		this.con = con;
		this.act = act;
	}
	
	public void execute(){
		con.removeAction(act, act.getAction());		
	}
	
	public void undo(){
		con.addAction(act, act.getAction());
	}
	
	public void redo(){
		con.removeAction(act, act.getAction());
	}
}
