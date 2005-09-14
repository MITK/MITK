/*
 * Created on 05.07.2005
 */
package parts;

import org.eclipse.gef.EditPart;
import org.eclipse.gef.EditPartFactory;

import model.States;
import model.StateMachinesDiagram;

/**
 * @author Daniel
 */
public class StateMachinesTreeEditPartFactory implements EditPartFactory {

	/*
	 * (non-Javadoc)
	 * 
	 * @see org.eclipse.gef.EditPartFactory#createEditPart(org.eclipse.gef.EditPart,
	 *      java.lang.Object)
	 */
	public EditPart createEditPart(EditPart context, Object model) {
		if (model instanceof States) {
			return new StateMachinesTreeEditPart((States) model);
		}
		if (model instanceof StateMachinesDiagram) {
			return new DiagramTreeEditPart((StateMachinesDiagram) model);
		}
		return null; // will not show an entry for the corresponding model
						// instance
	}

}
