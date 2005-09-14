/*
 * Created on 05.07.2005
 */
package parts;

import org.eclipse.gef.commands.Command;
import org.eclipse.gef.editpolicies.ComponentEditPolicy;
import org.eclipse.gef.requests.GroupRequest;

import model.States;
import model.StateMachinesDiagram;
import commands.StatesDeleteCommand;

/**
 * @author Daniel
 */
class StateMachinesComponentEditPolicy extends ComponentEditPolicy {

	/*
	 * (non-Javadoc)
	 * 
	 * @see org.eclipse.gef.editpolicies.ComponentEditPolicy#createDeleteCommand(org.eclipse.gef.requests.GroupRequest)
	 */
	protected Command createDeleteCommand(GroupRequest deleteRequest) {
		Object parent = getHost().getParent().getModel();
		Object child = getHost().getModel();
		if (parent instanceof StateMachinesDiagram && child instanceof States) {
			return new StatesDeleteCommand((StateMachinesDiagram) parent,
					(States) child);
		}
		return super.createDeleteCommand(deleteRequest);
	}
}
