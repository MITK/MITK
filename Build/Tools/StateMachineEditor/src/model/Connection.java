/*
 * Created on 05.07.2005
 */
package model;

import java.util.ArrayList;
import java.util.List;

import org.eclipse.draw2d.Graphics;
import org.eclipse.ui.views.properties.IPropertyDescriptor;
import org.eclipse.ui.views.properties.PropertyDescriptor;
import org.eclipse.ui.views.properties.TextPropertyDescriptor;
import org.jdom.Comment;
import org.jdom.Element;

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
	
	public static final String COMMENT_PROP = "Connection.comment";
	
	public static final String NAME_PROP = "Connection.name";
	
	public static final String EVENT_PROP = "Connection.event";
	
	public static final String ACTION_PROP = "Connection.action";

	private static final long serialVersionUID = 1;

	/** True, if the connection is attached to its endpoints. */
	private boolean isConnected;
	
	private String action = new String("action");
	
	private String actionPropertyValue = new String("");
	
	private String event = new String("event");
	
	private String name = new String("name");
	
	private String comment = new String("");

	/** Connection's source endpoint. */
	private States source;

	/** Connection's target endpoint. */
	private States target;
	
	private List allActions = new ArrayList();
	
	private Element transition1;
	private Comment comment1;
	private boolean hasComment = false;
	
	private List actions = new ArrayList();
	
	/*
	 * Initializes the property descriptors array.
	 * 
	 * @see #getPropertyDescriptors()
	 * @see #getPropertyValue(Object)
	 * @see #setPropertyValue(Object, Object)
	 */
	static {
		descriptors = new IPropertyDescriptor[] {
				new TextPropertyDescriptor(COMMENT_PROP, "Comment"),
				new TextPropertyDescriptor(NAME_PROP, "Name"),
				new TextPropertyDescriptor(EVENT_PROP, "Event"),
				new PropertyDescriptor(ACTION_PROP, "Action")
				};
		((PropertyDescriptor) descriptors[0]).setCategory("Comment");
	}
	
	
	/**
	 * Create a connection between two distinct states.
	 * 
	 * @param source
	 *            a source endpoint for this connection (non null)
	 * @param target
	 *            a target endpoint for this connection (non null)
	 * @param transitions JDOMElement
	 */
	public Connection(States source, States target, Element transition) {
        transition1 = transition;
        List transContent = transition1.getContent();
		for (int i = 0; i < transContent.size(); i++) {
			Object o = transContent.get(i);
			if (o instanceof Comment) {
				hasComment = true;
				comment1 = (Comment) o;
				this.setComment(comment1.getText());
			}
		}
		/*if (!(hasComment)) {
			comment1 = new Comment("");
			transition1.addContent(0, comment1);
		}*/
		reconnect(source, target);
	}
	
	public Connection(Element transition, States parentState, StateMachinesDiagram diagram) {
		List actionList = transition.getChildren("action", transition.getNamespace());
        for (int i = 0; i < actionList.size(); i++) {
        	Element ele1 = (Element) actionList.get(i);
        	Action action = new Action();
        	action.setActionElement(ele1);
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
			actions.add(act);
			// Fill action value in Propertysheet
			this.actionPropertyValue = this.actionPropertyValue + act;
			if (k < allActions.size() - 1) {
				this.actionPropertyValue = this.actionPropertyValue + "; ";
			}
		}
		if (allActions.size() > 1) {
			Action action = (Action) allActions.get(0);
			String act = action.getAction();
			action1 = act + "\n" + "...";
		}
		else if (allActions.size() == 1) {
			Action action = (Action) allActions.get(0);
			String act = action.getAction();
			action1 = act;
		}
		else action1 = "";
		this.setAction(action1);
        this.setName(transition1.getAttributeValue("NAME"));
        this.setEvent(transition1.getAttributeValue("EVENT_ID"));
        List transComment = transition1.getContent();
		for (int i = 0; i < transComment.size(); i++) {
			Object o = transComment.get(i);
			if (o instanceof Comment) {
				hasComment = true;
				comment1 = (Comment) o;
				this.setComment(comment1.getText());
			}
		}
		/*if (!(hasComment)) {
			comment1 = new Comment("");
			transition1.addContent(0, comment1);
		}*/
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
	 * Returns the String for the Property Sheet
	 * 
	 * @see org.eclipse.ui.views.properties.IPropertySource#getPropertyValue(java.lang.Object)
	 */
	public Object getPropertyValue(Object id) {
		if (id.equals(EVENT_PROP)) {
			return event;
		}
		else if (id.equals(ACTION_PROP)) {
			return actionPropertyValue;//action;
		}
		else if (id.equals(NAME_PROP)) {
			return name;
		}
		else if (id.equals(COMMENT_PROP)) {
			return comment;
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
	
	public void setComment(String newComment) {
		comment = newComment;
		if (!(comment.equals("") && !(comment == null))) {
			if (!(hasComment)) {
				hasComment = true;
				comment1 = new Comment("");
				transition1.addContent(0, comment1);
			}
			comment1.setText(comment);
		}
		else if (hasComment) {
			hasComment = false;
			transition1.removeContent(comment1);
		}
		firePropertyChange(COMMENT_PROP, null, comment);
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
		else if (id.equals(COMMENT_PROP)) {
			String newComment = (String) value;
			setComment(newComment);
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
	
	public String getComment() {
		return comment;
	}
	
	public List getActionList() {
		return actions;
	}
	
	public List getAllActions() {
		return allActions;
	}
	
	public Element getElement() {
		return transition1;
	}
	
	public void addAction(Action act, String act1) {
		if (allActions.size() == 0) {
			this.setAction(act1);
			this.actionPropertyValue = act1;
		}
		else if (allActions.size() == 1) {
			this.setAction(this.getAction() + "\n" + "...");
			this.actionPropertyValue = actionPropertyValue + "; " + act1;
		}
		else {
			this.actionPropertyValue = actionPropertyValue + "; " + act1;
		}
		allActions.add(act);
		actions.add(act1);
		transition1.addContent(act.getActionElement());
	}
	
	public void removeAction(Action act, String act1) {
		allActions.remove(act);
		actions.remove(act1);
		if (actions.size() == 0) {
			this.setAction("");
			this.actionPropertyValue = "";
		}
		else if (actions.size() == 1) {
			this.setAction(actions.get(0).toString());
			this.actionPropertyValue = actions.get(0).toString();
		}
		else {
			this.setAction(actions.get(0).toString() + "\n" + "...");
			this.actionPropertyValue = actions.get(0).toString();
			for (int i = 1; i < allActions.size(); i++) {
				this.actionPropertyValue = this.actionPropertyValue + "; " + actions.get(i).toString();
			}
		}
		transition1.removeContent(act.getActionElement());
	}

}
