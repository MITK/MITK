package actions;

import java.util.List;

import model.Connection;

import org.eclipse.gef.EditPart;
import org.eclipse.gef.Request;
import org.eclipse.gef.commands.Command;
import org.eclipse.gef.commands.CompoundCommand;
import org.eclipse.gef.ui.actions.SelectionAction;
import org.eclipse.ui.IWorkbenchPart;

public class AddAction extends SelectionAction {
	
	private static final String ADD_ACTION_REQUEST = "Add action";  //$NON-NLS-1$
	
	public static final String ADD_ACTION = "Add action";   //$NON-NLS-1$
	
	Request request;

	public AddAction(IWorkbenchPart part) {
		super(part);
		request = new Request(ADD_ACTION_REQUEST);
		setId(ADD_ACTION);
		setText("Add action");
	}

	protected boolean calculateEnabled() {
		return canPerformAction();
	}

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
		}
		return true;
	}

	private Command getCommand() {
		List editparts = getSelectedObjects();
		CompoundCommand cc = new CompoundCommand();
		cc.setDebugLabel("Add action");//$NON-NLS-1$
		for (int i=0; i < editparts.size(); i++) {
			EditPart part = (EditPart)editparts.get(i);
			cc.add(part.getCommand(request));
		}
		return cc;
	}

	public void run() {
		execute(getCommand());
	}
}
