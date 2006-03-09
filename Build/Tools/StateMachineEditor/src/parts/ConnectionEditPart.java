/*
 * Created on 05.07.2005
 */
package parts;

import java.beans.PropertyChangeEvent;
import java.beans.PropertyChangeListener;

import org.eclipse.draw2d.ColorConstants;
import org.eclipse.draw2d.ConnectionLocator;
import org.eclipse.draw2d.IFigure;
import org.eclipse.draw2d.Label;
import org.eclipse.draw2d.PolygonDecoration;
import org.eclipse.draw2d.PolylineConnection;
import org.eclipse.draw2d.PositionConstants;

import org.eclipse.gef.EditPolicy;
import org.eclipse.gef.commands.Command;
import org.eclipse.gef.editparts.AbstractConnectionEditPart;
import org.eclipse.gef.editpolicies.ConnectionEditPolicy;
import org.eclipse.gef.editpolicies.ConnectionEndpointEditPolicy;
import org.eclipse.gef.requests.GroupRequest;

import model.Connection;
import model.ModelElement;
import commands.ConnectionDeleteCommand;

/**
 * @author Daniel
 */
class ConnectionEditPart extends AbstractConnectionEditPart implements
		PropertyChangeListener {
	
	private Label actionName = new Label("action");
	private Label eventName = new Label("event");
	
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
		installEditPolicy(EditPolicy.COMPONENT_ROLE, new ActionAndEventEditPolicy());
		// Selection handle edit policy.
		// Makes the connection show a feedback, when selected by the user.
		installEditPolicy(EditPolicy.CONNECTION_ENDPOINTS_ROLE,
				new ConnectionEndpointEditPolicy());
		// Allows the removal of the connection model element
		installEditPolicy(EditPolicy.CONNECTION_ROLE,
				new ConnectionEditPolicy() {
					protected Command getDeleteCommand(GroupRequest request) {
						return new ConnectionDeleteCommand(getCastedModel());
					}
				});
	}

	/*
	 * (non-Javadoc)
	 * 
	 * @see org.eclipse.gef.editparts.AbstractGraphicalEditPart#createFigure()
	 */
	protected IFigure createFigure() {
		PolylineConnection connection = (PolylineConnection) super.createFigure();
		connection.setTargetDecoration(new PolygonDecoration()); // arrow at
																	// target
																	// endpoint
		ConnectionLocator midpointLocator = new ConnectionLocator(connection, PositionConstants.RIGHT);
		actionName.setOpaque(true);
		//actionName.setBackgroundColor(ColorConstants.green);
		actionName.setForegroundColor(ColorConstants.black);
		actionName.setText(getCastedModel().getAction()); // actionName of this connection
		// add action on midpoint
		midpointLocator.setRelativePosition(PositionConstants.RIGHT);
		connection.add(actionName, midpointLocator);
		ConnectionLocator midpointLocator2 = new ConnectionLocator(connection, PositionConstants.RIGHT);
		eventName.setOpaque(true);
		//eventName.setBackgroundColor(ColorConstants.red);
		eventName.setForegroundColor(ColorConstants.black);
		eventName.setText(getCastedModel().getEvent()); // eventName of this connection
		// add event on midpoint
		midpointLocator2.setRelativePosition(PositionConstants.LEFT);
		connection.add(eventName, midpointLocator2);
		
		return connection;
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
	 * @return the Connection model
	 */
	private Connection getCastedModel() {
		return (Connection) getModel();
	}

	/*
	 * (non-Javadoc)
	 * 
	 * @see java.beans.PropertyChangeListener#propertyChange(java.beans.PropertyChangeEvent)
	 */
	public void propertyChange(PropertyChangeEvent event) {
		String property = event.getPropertyName();
		if (Connection.EVENT_PROP.equals(property)) {
			eventName.setText(getCastedModel().getEvent());
		}
		else if (Connection.ACTION_PROP.equals(property)) {
			actionName.setText(getCastedModel().getAction());
		} else if (Connection.ACTIVE_PROP.equals(property)) {
			PolylineConnection connection = (PolylineConnection) getFigure();
			connection.setForegroundColor(ColorConstants.red);
		} else if (Connection.INACTIVE_PROP.equals(property)) {
			PolylineConnection connection = (PolylineConnection) getFigure();
			connection.setForegroundColor(ColorConstants.black);
		} else if (Connection.DEACTIVATE_POLICY_PROP.equals(property)) {
			this.removeEditPolicy(EditPolicy.COMPONENT_ROLE);
			this.removeEditPolicy(EditPolicy.CONNECTION_ENDPOINTS_ROLE);
			this.removeEditPolicy(EditPolicy.CONNECTION_ROLE);
		} else if (Connection.ACTIVATE_POLICY_PROP.equals(property)) {
			createEditPolicies();
		}
	}

}
