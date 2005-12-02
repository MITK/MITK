/*
 * Created on 05.07.2005
 */
package parts;

import java.beans.PropertyChangeEvent;
import java.beans.PropertyChangeListener;
import java.util.List;

import org.eclipse.gef.EditPart;
import org.eclipse.gef.EditPolicy;
import org.eclipse.gef.RootEditPart;
import org.eclipse.gef.editparts.AbstractTreeEditPart;
import org.eclipse.gef.editpolicies.RootComponentEditPolicy;

import model.ModelElement;
import model.StateMachinesDiagram;

/**
 * @author Daniel
 */
class DiagramTreeEditPart extends AbstractTreeEditPart implements
		PropertyChangeListener {

	/**
	 * Create a new instance of this edit part using the given model element.
	 * 
	 * @param model
	 *            a non-null StateMachinesDiagram instance
	 */
	DiagramTreeEditPart(StateMachinesDiagram model) {
		super(model);
	}

	/**
	 * Upon activation, attach to the model element as a property change
	 * listener.
	 */
	public void activate() {
		if (!isActive()) {
			super.activate();
			((ModelElement) getModel()).addPropertyChangeListener(this);
		}
	}

	/*
	 * (non-Javadoc)
	 * 
	 * @see org.eclipse.gef.examples.stateMachines.parts.StateMachinesTreeEditPart#createEditPolicies()
	 */
	protected void createEditPolicies() {
		// If this editpart is the root content of the viewer, then disallow
		// removal
		if (getParent() instanceof RootEditPart) {
			installEditPolicy(EditPolicy.COMPONENT_ROLE,
					new RootComponentEditPolicy());
		}
	}

	/**
	 * Upon deactivation, detach from the model element as a property change
	 * listener.
	 */
	public void deactivate() {
		if (isActive()) {
			super.deactivate();
			((ModelElement) getModel()).removePropertyChangeListener(this);
		}
	}

	/**
	 * @return the StateMAchinesDiagram model
	 */
	private StateMachinesDiagram getCastedModel() {
		return (StateMachinesDiagram) getModel();
	}

	/**
	 * Convenience method that returns the EditPart corresponding to a given
	 * child.
	 * 
	 * @param child
	 *            a model element instance
	 * @return the corresponding EditPart or null
	 */
	private EditPart getEditPartForChild(Object child) {
		return (EditPart) getViewer().getEditPartRegistry().get(child);
	}

	/*
	 * (non-Javadoc)
	 * 
	 * @see org.eclipse.gef.editparts.AbstractEditPart#getModelChildren()
	 */
	protected List getModelChildren() {
		return getCastedModel().getChildren(); // a list of states
	}

	/*
	 * (non-Javadoc)
	 * 
	 * @see java.beans.PropertyChangeListener#propertyChange(java.beans.PropertyChangeEvent)
	 */
	public void propertyChange(PropertyChangeEvent evt) {
		String prop = evt.getPropertyName();
		if (StateMachinesDiagram.CHILD_ADDED_PROP.equals(prop)) {
			// add a child to this edit part
			// causes an additional entry to appear in the tree of the outline
			// dialog
			addChild(createChild(evt.getNewValue()), -1);
		} else if (StateMachinesDiagram.CHILD_REMOVED_PROP.equals(prop)) {
			// remove a child from this edit part
			// causes the corresponding edit part to disappear from the tree in
			// the outline dialog
			removeChild(getEditPartForChild(evt.getNewValue()));
		} else {
			refreshVisuals();
		}
	}
}
