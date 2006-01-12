/*
 * Created on 05.07.2005
 */
package parts;

import java.beans.PropertyChangeEvent;
import java.beans.PropertyChangeListener;
import java.util.List;

import org.eclipse.draw2d.ChopboxAnchor;
import org.eclipse.draw2d.ColorConstants;
import org.eclipse.draw2d.ConnectionAnchor;
import org.eclipse.draw2d.IFigure;
import org.eclipse.draw2d.geometry.Rectangle;

import org.eclipse.gef.ConnectionEditPart;
import org.eclipse.gef.EditPolicy;
import org.eclipse.gef.GraphicalEditPart;
import org.eclipse.gef.NodeEditPart;
import org.eclipse.gef.Request;
import org.eclipse.gef.commands.Command;
import org.eclipse.gef.editparts.AbstractGraphicalEditPart;
import org.eclipse.gef.editpolicies.GraphicalNodeEditPolicy;
import org.eclipse.gef.requests.CreateConnectionRequest;
import org.eclipse.gef.requests.ReconnectRequest;

import model.Connection;
import model.RoundLabel;
import model.StartState;
import model.NormalState;
import model.EndState;
import model.ModelElement;
import model.States;
import commands.ConnectionCreateCommand;
import commands.ConnectionReconnectCommand;

/**
 * @author Daniel
 */
class StateMachinesEditPart extends AbstractGraphicalEditPart implements
		PropertyChangeListener, NodeEditPart {

	private ConnectionAnchor anchor;

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
	 * @see org.eclipse.gef.editparts.AbstractEditPart#createEditPolicies()
	 */
	protected void createEditPolicies() {
		// allow removal of the associated model element
		installEditPolicy(EditPolicy.COMPONENT_ROLE,
				new StateMachinesComponentEditPolicy());
		// allow the creation of connections and
		// the reconnection of connections between States instances
		installEditPolicy(EditPolicy.GRAPHICAL_NODE_ROLE,
				new GraphicalNodeEditPolicy() {
					/*
					 * (non-Javadoc)
					 * 
					 * @see org.eclipse.gef.editpolicies.GraphicalNodeEditPolicy#getConnectionCompleteCommand(org.eclipse.gef.requests.CreateConnectionRequest)
					 */
					protected Command getConnectionCompleteCommand(
							CreateConnectionRequest request) {
						ConnectionCreateCommand cmd = (ConnectionCreateCommand) request
								.getStartCommand();
						cmd.setTarget((States) getHost().getModel());
						return cmd;
					}

					/*
					 * (non-Javadoc)
					 * 
					 * @see org.eclipse.gef.editpolicies.GraphicalNodeEditPolicy#getConnectionCreateCommand(org.eclipse.gef.requests.CreateConnectionRequest)
					 */
					protected Command getConnectionCreateCommand(
							CreateConnectionRequest request) {
						States source = (States) getHost().getModel();
						ConnectionCreateCommand cmd = new ConnectionCreateCommand(
								source);
						request.setStartCommand(cmd);
						return cmd;
					}

					/*
					 * (non-Javadoc)
					 * 
					 * @see org.eclipse.gef.editpolicies.GraphicalNodeEditPolicy#getReconnectSourceCommand(org.eclipse.gef.requests.ReconnectRequest)
					 */
					protected Command getReconnectSourceCommand(
							ReconnectRequest request) {
						Connection conn = (Connection) request
								.getConnectionEditPart().getModel();
						States newSource = (States) getHost().getModel();
						ConnectionReconnectCommand cmd = new ConnectionReconnectCommand(
								conn);
						cmd.setNewSource(newSource);
						return cmd;
					}

					/*
					 * (non-Javadoc)
					 * 
					 * @see org.eclipse.gef.editpolicies.GraphicalNodeEditPolicy#getReconnectTargetCommand(org.eclipse.gef.requests.ReconnectRequest)
					 */
					protected Command getReconnectTargetCommand(
							ReconnectRequest request) {
						Connection conn = (Connection) request
								.getConnectionEditPart().getModel();
						States newTarget = (States) getHost().getModel();
						ConnectionReconnectCommand cmd = new ConnectionReconnectCommand(
								conn);
						cmd.setNewTarget(newTarget);
						return cmd;
					}
				});
	}

	/*
	 * (non-Javadoc)
	 * 
	 * @see org.eclipse.gef.editparts.AbstractGraphicalEditPart#createFigure()
	 */
	protected IFigure createFigure() {
		IFigure f = createFigureForModel();
		f.setOpaque(true); // non-transparent figure
		if (getModel() instanceof StartState) {
			f.setBackgroundColor(ColorConstants.green);
			f.setForegroundColor(ColorConstants.black);
		}
		else if (getModel() instanceof NormalState){
			f.setBackgroundColor(ColorConstants.yellow);
			f.setForegroundColor(ColorConstants.black);
		}
		else if (getModel() instanceof EndState){
			f.setBackgroundColor(ColorConstants.orange);
			f.setForegroundColor(ColorConstants.black);
		}
		else {
			// if States gets extended the conditions above must be updated
			throw new IllegalArgumentException();
		}
		return f;
	}

	/**
	 * Return a IFigure depending on the instance of the current model element.
	 * This allows this EditPart to be used for all subclasses of States.
	 */
	private IFigure createFigureForModel() {
		if (getModel() instanceof StartState) {
			return new RoundLabel();
		} 
		else if (getModel() instanceof NormalState) {
			return new RoundLabel();
		} 
		else if (getModel() instanceof EndState) {
			return new RoundLabel();
		} 
		else {
			// if States gets extended the conditions above must be updated
			throw new IllegalArgumentException();
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

	private States getCastedModel() {
		return (States) getModel();
	}

	protected ConnectionAnchor getConnectionAnchor() {
		if (anchor == null) {
			if (getModel() instanceof StartState) {
				anchor = new ChopboxAnchor(getFigure());
			}
			else if (getModel() instanceof NormalState) {
				anchor = new ChopboxAnchor(getFigure());
			}
			else if (getModel() instanceof EndState) {
				anchor = new ChopboxAnchor(getFigure());
			}
			else {
				// if States gets extended the conditions above must be updated
				throw new IllegalArgumentException("unexpected model");
			}
		}
		return anchor;
	}

	/*
	 * (non-Javadoc)
	 * 
	 * @see org.eclipse.gef.editparts.AbstractGraphicalEditPart#getModelSourceConnections()
	 */
	protected List getModelSourceConnections() {
		return getCastedModel().getSourceConnections();
	}

	/*
	 * (non-Javadoc)
	 * 
	 * @see org.eclipse.gef.editparts.AbstractGraphicalEditPart#getModelTargetConnections()
	 */
	protected List getModelTargetConnections() {
		return getCastedModel().getTargetConnections();
	}

	/*
	 * (non-Javadoc)
	 * 
	 * @see org.eclipse.gef.NodeEditPart#getSourceConnectionAnchor(org.eclipse.gef.ConnectionEditPart)
	 */
	public ConnectionAnchor getSourceConnectionAnchor(
			ConnectionEditPart connection) {
		return getConnectionAnchor();
	}

	/*
	 * (non-Javadoc)
	 * 
	 * @see org.eclipse.gef.NodeEditPart#getSourceConnectionAnchor(org.eclipse.gef.Request)
	 */
	public ConnectionAnchor getSourceConnectionAnchor(Request request) {
		return getConnectionAnchor();
	}

	/*
	 * (non-Javadoc)
	 * 
	 * @see org.eclipse.gef.NodeEditPart#getTargetConnectionAnchor(org.eclipse.gef.ConnectionEditPart)
	 */
	public ConnectionAnchor getTargetConnectionAnchor(
			ConnectionEditPart connection) {
		return getConnectionAnchor();
	}

	/*
	 * (non-Javadoc)
	 * 
	 * @see org.eclipse.gef.NodeEditPart#getTargetConnectionAnchor(org.eclipse.gef.Request)
	 */
	public ConnectionAnchor getTargetConnectionAnchor(Request request) {
		return getConnectionAnchor();
	}

	/*
	 * (non-Javadoc)
	 * 
	 * @see java.beans.PropertyChangeListener#propertyChange(java.beans.PropertyChangeEvent)
	 */
	public void propertyChange(PropertyChangeEvent evt) {
		String prop = evt.getPropertyName();
		if (States.SIZE_PROP.equals(prop) || States.LOCATION_PROP.equals(prop) 
				|| States.STATE_NAME_PROP.equals(prop)) {
			refreshSourceConnections();
			refreshTargetConnections();
			refreshVisuals();
		} else if (States.SOURCE_CONNECTIONS_PROP.equals(prop)) {
			refreshSourceConnections();
			refreshTargetConnections();
			refreshVisuals();
		} else if (States.TARGET_CONNECTIONS_PROP.equals(prop)) {
			refreshSourceConnections();
			refreshTargetConnections();
			refreshVisuals();
		} else if (States.ACTIVE_PROP.equals(prop)) {
			RoundLabel label = (RoundLabel)getFigure();
			label.setBackgroundColor(ColorConstants.red);
		} else if (States.INACTIVE_PROP.equals(prop)) {
			RoundLabel label = (RoundLabel)getFigure();
			if (getModel() instanceof StartState) {
				label.setBackgroundColor(ColorConstants.green);
			}
			else if (getModel() instanceof NormalState){
				label.setBackgroundColor(ColorConstants.yellow);
			}
			else if (getModel() instanceof EndState){
				label.setBackgroundColor(ColorConstants.orange);
			}
		} else if (States.DEACTIVATE_POLICY_PROP.equals(prop)) {
			this.removeEditPolicy(EditPolicy.COMPONENT_ROLE);
			this.removeEditPolicy(EditPolicy.GRAPHICAL_NODE_ROLE);
		} else if (States.ACTIVATE_POLICY_PROP.equals(prop)) {
			createEditPolicies();
		}
	}

	protected void refreshVisuals() {
		// notify parent container of changed position & location
		// if this line is removed, the XYLayoutManager used by the parent
		// container
		// (the Figure of the StateMachinesDiagramEditPart), will not know the bounds
		// of this figure
		// and will not draw it correctly.
		RoundLabel label = (RoundLabel)getFigure();
		label.setText(getCastedModel().getStateName());
		Rectangle bounds = new Rectangle(getCastedModel().getLocation(),
				getCastedModel().getSize());
		((GraphicalEditPart) getParent()).setLayoutConstraint(this,
				getFigure(), bounds);
	}
}
