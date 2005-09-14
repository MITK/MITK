/*
 * Created on 05.07.2005
 */
package model;

import java.util.ArrayList;
import java.util.List;

import org.eclipse.draw2d.Graphics;
import org.eclipse.ui.views.properties.IPropertyDescriptor;
import org.eclipse.ui.views.properties.TextPropertyDescriptor;
import org.jdom.Element;

import dom.Action;

/**
 * @author Daniel
 */
public class Connection extends ModelElement {
	
	private static IPropertyDescriptor[] descriptors;
	
	/**
	 * Used for indicating that a Connection with solid line style should be
	 * created.
	 * 
	 * @see parts.StateMachinesEditPart#createEditPolicies()
	 */
	public static final Integer SOLID_CONNECTION = new Integer(
			Graphics.LINE_SOLID);
	
	public static final String NAME_PROP = "Connection.name";
	
	public static final String EVENT_PROP = "Connection.event";
	
	public static final String ACTION_PROP = "Connection.action";

	private static final long serialVersionUID = 1;

	/** True, if the connection is attached to its endpoints. */
	private boolean isConnected;
	
	private String action = new String("action");
	
	private String event = new String("event");
	
	private String name = new String("name");

	/** Connection's source endpoint. */
	private States source;

	/** Connection's target endpoint. */
	private States target;
	
	private List allActions = new ArrayList();
	private Element transition1;
	
	/*
	 * Initializes the property descriptors array.
	 * 
	 * @see #getPropertyDescriptors()
	 * @see #getPropertyValue(Object)
	 * @see #setPropertyValue(Object, Object)
	 */
	static {
		descriptors = new IPropertyDescriptor[] {
				new TextPropertyDescriptor(NAME_PROP, "Name"),
				new TextPropertyDescriptor(EVENT_PROP, "Event"),
				new TextPropertyDescriptor(ACTION_PROP, "Action")};
	}
	/**
	 * Create a connection between two distinct states.
	 * 
	 * @param source
	 *            a source endpoint for this connection (non null)
	 * @param target
	 *            a target endpoint for this connection (non null)
	 * @param transitions DOMObject
	 * @throws IllegalArgumentException
	 *             if any of the parameters are null
	 */
	public Connection(States source, States target, Element transition) {
        transition1 = transition;
		reconnect(source, target);
	}
	
	public Connection(Element transition, States parentState, StateMachinesDiagram diagram) {
		List actionList = transition.getChildren("action", transition.getNamespace());
        for (int i = 0; i < actionList.size(); i++) {
        	Element ele1 = (Element) actionList.get(i);
        	Action action = new Action(ele1);
        	allActions.add(action);
        }	
        transition1 = transition;
        List allStates = diagram.getChildren();
        for (int i = 0; i < allStates.size(); i++) {
        	States state = (States) allStates.get(i);
        	if (state.getStateId().equals(transition1.getAttributeValue("NEXT_STATE_ID"))) {
        		target = state;
        		break;
        	}
        }
        String action1 = "";
		for (int k = 0; k < allActions.size(); k++) {
			Action action = (Action) allActions.get(k);
			String act = action.getAction();
			action1 = action1 + act;
			if (k < allActions.size() - 1) {
				action1 = action1 + "\n";
			}
		}
		this.setAction(action1);
        this.setName(transition1.getAttributeValue("NAME"));
        this.setEvent(transition1.getAttributeValue("EVENT_ID"));
		reconnect(parentState, target);
	}

	/**
	 * Disconnect this connection from the states it is attached to.
	 */
	public void disconnect() {
		if (isConnected) {
			source.removeConnection(this, transition1);
			target.removeConnection(this, transition1);
			isConnected = false;
		}
	}

	/**
	 * Returns the descriptor for the properties
	 * 
	 * @see org.eclipse.ui.views.properties.IPropertySource#getPropertyDescriptors()
	 */
	public IPropertyDescriptor[] getPropertyDescriptors() {
		return descriptors;
	}

	/**
	 * Returns the lineStyle as String for the Property Sheet
	 * 
	 * @see org.eclipse.ui.views.properties.IPropertySource#getPropertyValue(java.lang.Object)
	 */
	public Object getPropertyValue(Object id) {
		if (id.equals(EVENT_PROP)) {
			return event;
		}
		else if (id.equals(ACTION_PROP)) {
			return action;
		}
		else if (id.equals(NAME_PROP)) {
			return name;
		}
		return super.getPropertyValue(id);
	}

	/**
	 * Returns the source endpoint of this connection.
	 * 
	 * @return a non-null States instance
	 */
	public States getSource() {
		return source;
	}

	/**
	 * Returns the target endpoint of this connection.
	 * 
	 * @return a non-null States instance
	 */
	public States getTarget() {
		return target;
	}

	/**
	 * Reconnect this connection. The connection will reconnect with the states
	 * it was previously attached to.
	 */
	public void reconnect() {
		if (source.equals(target)) {
			source.addConnection(this, transition1);
			transition1.setAttribute("NEXT_STATE_ID", target.getStateId());
		}
		else {
			source.addConnection(this, transition1);
			target.addConnection(this, transition1);
			transition1.setAttribute("NEXT_STATE_ID", target.getStateId());
		}
		isConnected = true;
	}
		
	

	/**
	 * Reconnect to a different source and/or target state. The connection will
	 * disconnect from its current attachments and reconnect to the new source
	 * and target.
	 * 
	 * @param newSource
	 *            a new source endpoint for this connection (non null)
	 * @param newTarget
	 *            a new target endpoint for this connection (non null)
	 * @throws IllegalArgumentException
	 *             if any of the paramers are null
	 */
	public void reconnect(States newSource, States newTarget) {
		if (newSource == null || newTarget == null) {
			throw new IllegalArgumentException();
		}
		disconnect();
		this.source = newSource;
		this.target = newTarget;
		reconnect();
	}
	
	public void setAction(String newAction) {
		action = newAction;
		//transition1.changeAction(newAction);
		firePropertyChange(ACTION_PROP, null, action);
	}
	
	public void setEvent(String newEvent) {
		event = newEvent;
		transition1.setAttribute("EVENT_ID", newEvent);
		firePropertyChange(EVENT_PROP, null, event);
	}
	
	public void setName(String newName) {
		name = newName;
		transition1.setAttribute("NAME", newName);
		firePropertyChange(NAME_PROP, null, name);
	}
	
	public void setNextStateID(String id) {
		transition1.setAttribute("NEXT_STATE_ID", id);
	}

	/**
	 * Sets the properties based on the String provided by the PropertySheet
	 * 
	 * @see org.eclipse.ui.views.properties.IPropertySource#setPropertyValue(java.lang.Object,
	 *      java.lang.Object)
	 */
	public void setPropertyValue(Object id, Object value) {
		if (id.equals(EVENT_PROP)) {
			String eventName = (String) value;
			setEvent(eventName);
		}
		else if (id.equals(ACTION_PROP)) {
			String actionName = (String) value;
			setAction(actionName);
		}
		else if (id.equals(NAME_PROP)) {
			String newName = (String) value;
			setName(newName);
		}
		else
			super.setPropertyValue(id, value);
	}

	public String getAction() {
		return action;
	}
	
	public String getEvent() {
		return event;
	}
	
	public String getName() {
		return name;
	}

}
