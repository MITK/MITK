/*
 * Created on 05.07.2005
 */
package parts;

import org.eclipse.gef.EditPart;
import org.eclipse.gef.EditPartFactory;

import model.Connection;
import model.States;
import model.StateMachinesDiagram;

/**
 * @author Daniel
 */
public class StateMachinesEditPartFactory implements EditPartFactory {

	/*
	 * (non-Javadoc)
	 * 
	 * @see org.eclipse.gef.EditPartFactory#createEditPart(org.eclipse.gef.EditPart,
	 *      java.lang.Object)
	 */
	public EditPart createEditPart(EditPart context, Object modelElement) {
		// get EditPart for model element
		EditPart part = getPartForElement(modelElement);
		// store model element in EditPart
		part.setModel(modelElement);
		return part;
	}

	/**
	 * Maps an object to an EditPart.
	 * 
	 * @throws RuntimeException
	 *             if no match was found (programming error)
	 */
	private EditPart getPartForElement(Object modelElement) {
		if (modelElement instanceof StateMachinesDiagram) {
			return new DiagramEditPart();
		}
		if (modelElement instanceof States) {
			return new StateMachinesEditPart();
		}
		if (modelElement instanceof Connection) {
			return new ConnectionEditPart();
		}
		throw new RuntimeException("Can't create part for model element: "
				+ ((modelElement != null) ? modelElement.getClass().getName()
						: "null"));
	}

}
