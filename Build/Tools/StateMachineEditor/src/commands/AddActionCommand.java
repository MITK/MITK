package commands;

import java.util.Vector;

import model.Action;
import model.Connection;

import org.eclipse.gef.commands.Command;
import org.jdom.Comment;
import org.jdom.Element;

import dialog.ActionAndEventInfo;


public class AddActionCommand extends Command {
	
	private final Connection con;
	private Action action = new Action();
	private String name = null;
	private ActionAndEventInfo info;
	
	
	/**
	 * Instantiate a command that adds an action
	 * @param con connection on which to add the action
	 * @param info object which holds comment, name, id and parameters from the new action
	 */
	public AddActionCommand(Connection con, ActionAndEventInfo info) {
		if (con == null) {
			throw new IllegalArgumentException();
		}
		setLabel("Add action");
		this.con = con;
		this.info = info;
	}
	
	/* (non-Javadoc)
	 * @see org.eclipse.gef.commands.Command#canExecute()
	 */
	public boolean canExecute() {
		return true;
	}
	
	/** 
	 * creates a new action element and adds it to connection
	 * 
	 * @see org.eclipse.gef.commands.Command#execute()
	 */
	public void execute() {
		name = info.getActionEventName();
		Element act = new Element("action");
		Vector paraVector = info.getActionParameter();
		for (int i = 0; i < paraVector.size(); i++) {
			Vector para = (Vector) paraVector.get(i);
			if (!(para.get(1) == null) && !(para.get(1).toString().equals(""))) {
				for (int j = 0; j < para.size(); j++) {
					if (para.get(j) == null) {
						para.set(j, "");
					}
				}
				Element parameter = new Element(para.get(1).toString());
				parameter.setNamespace(con.getElement().getNamespace());
				parameter.setAttribute("NAME", para.get(0).toString());
				parameter.setAttribute("VALUE", para.get(2).toString());
				if (!(para.get(3).toString().equals(""))) {
					Comment paraComment = new Comment(para.get(3).toString());
					parameter.addContent(0, paraComment);
				}
				act.addContent(parameter);
			}
		}
		act.setNamespace(con.getElement().getNamespace());
		act.setAttribute("ID", name);
		if (!(info.getActionEventComment().equals("") && !(info.getActionEventComment() == null))) {
			Comment actionComment = new Comment(info.getActionEventComment());
			act.addContent(0, actionComment);
		}
		action.setActionElement(act);
		con.addAction(action, name);
	}
	
	/* (non-Javadoc)
	 * @see org.eclipse.gef.commands.Command#undo()
	 */
	public void undo() {
		con.removeAction(action, name);
	}
	
	/* (non-Javadoc)
	 * @see org.eclipse.gef.commands.Command#redo()
	 */
	public void redo() {
		con.addAction(action, name);
	}
}
