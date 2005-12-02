package commands;

import java.util.Vector;

import model.Action;
import model.Connection;

import org.eclipse.gef.commands.Command;
import org.jdom.Comment;
import org.jdom.Element;

import dialog.ActionAndEventInfo;


public class ChangeActionCommand extends Command {
		
	private final Connection con;
	private Action oldAction;
	private Action newAction = new Action();
	private String name = null;
	private ActionAndEventInfo info;
	
	/**
	 * Instantiate a command that changes an action
	 * @param con connection on which to change the action
	 * @param act the action to change
	 * @param info object which holds comment, name, id and parameters from the new action
	 */
	public ChangeActionCommand(Connection con, Action act, ActionAndEventInfo info){
		if (con == null) {
			throw new IllegalArgumentException();
		}
		setLabel("Change action");
		this.con = con;
		this.oldAction = act;
		this.info = info;
	}
	
	/**
	 * removes the old action and adds the new one
	 * 
	 * @see org.eclipse.gef.commands.Command#execute()
	 */
	public void execute(){
		con.removeAction(oldAction, oldAction.getActionId());
		name = info.getActionEventName();
		Element act = new Element("action");
		if (!(info.getActionEventComment().equals("") && !(info.getActionEventComment() == null))) {
			Comment actionComment = new Comment(info.getActionEventComment());
			act.addContent(0, actionComment);
		}
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
					parameter.addContent(paraComment);
				}
				act.addContent(parameter);
			}
		}
		act.setNamespace(con.getElement().getNamespace());
		act.setAttribute("ID", name);
		newAction.setActionElement(act);
		con.addAction(newAction, name);
	}
	
	/* (non-Javadoc)
	 * @see org.eclipse.gef.commands.Command#undo()
	 */
	public void undo(){
		con.removeAction(newAction, name);
		con.addAction(oldAction, oldAction.getActionId());
	}
	
	/* (non-Javadoc)
	 * @see org.eclipse.gef.commands.Command#redo()
	 */
	public void redo(){
		con.removeAction(oldAction, oldAction.getActionId());
		con.addAction(newAction, name);
	}
}