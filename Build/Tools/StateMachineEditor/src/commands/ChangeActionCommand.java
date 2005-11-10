package commands;

import java.util.Vector;

import model.Action;
import model.Connection;

import org.eclipse.gef.commands.Command;
import org.jdom.Comment;
import org.jdom.Element;

import dialog.ActionInfo;


public class ChangeActionCommand extends Command {
		
	private final Connection con;
	private Action oldAction;
	private Action newAction = new Action();
	private String name = null;
	private ActionInfo info;
	
	public ChangeActionCommand(Connection con, Action act, ActionInfo info){
		if (con == null) {
			throw new IllegalArgumentException();
		}
		setLabel("Change action");
		this.con = con;
		this.oldAction = act;
		this.info = info;
	}
	
	public void execute(){
		con.removeAction(oldAction, oldAction.getAction());
		name = info.getActionName();
		Element act = new Element("action");
		if (!(info.getActionComment().equals("") && !(info.getActionComment() == null))) {
			Comment actionComment = new Comment(info.getActionComment());
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
	
	public void undo(){
		con.removeAction(newAction, name);
		con.addAction(oldAction, oldAction.getAction());
	}
	
	public void redo(){
		con.removeAction(oldAction, oldAction.getAction());
		con.addAction(newAction, name);
	}
}

