/*
 * Created on 05.07.2005
 */
package model;

import java.util.ArrayList;
import java.util.List;

import org.eclipse.draw2d.Graphics;
import org.eclipse.jface.viewers.ICellEditorValidator;
import org.eclipse.ui.views.properties.IPropertyDescriptor;
import org.eclipse.ui.views.properties.PropertyDescriptor;
import org.eclipse.ui.views.properties.TextPropertyDescriptor;
import org.jdom.Comment;
import org.jdom.Element;

import dom.DOMGetInstance;
import dom.ReadActionAndEventDOMTree;

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
	
	/** property id for changes on the connection comment. */
	public static final String COMMENT_PROP = "Connection.comment";
	
	/** property id for changes on the connection name. */
	public static final String NAME_PROP = "Connection.name";
	
	/** property id for changes on the connection event. */
	public static final String EVENT_PROP = "Connection.event";
	
	/** property id for changes on the connection actions. */
	public static final String ACTION_PROP = "Connection.action";
	
	/** Property ID to use when a connection is active in debug mode. */
	public static final String ACTIVE_PROP = "Connection.active";
	
	/** Property ID to use when a connection is inactive in debug mode. */
	public static final String INACTIVE_PROP = "Connection.inactive";
	
	/** Property ID to use when the EditPolicies should be deactivated in debug mode. */
	public static final String DEACTIVATE_POLICY_PROP = "Connection.deactivatePolicy";
	
	/** Property ID to use when the EditPolicies should be deactivated in debug mode. */
	public static final String ACTIVATE_POLICY_PROP = "Connection.activatePolicy";

	private static final long serialVersionUID = 1;

	/** True, if the connection is attached to its endpoints. */
	private boolean isConnected;
	
	private String action = new String("");
	
	private String actionPropertyValue = new String("");
	
	private String event = new String("event");
	
	private String name = new String("name");
	
	private String comment = new String("");

	/** Connection's source endpoint. */
	private States source;

	/** Connection's target endpoint. */
	private States target;
	
	/** List with all jDOM action elements. */
	private List allActions = new ArrayList();
	
	private Element transition1;
	private Comment comment1;
	private boolean hasComment = false;
	
	/** List with all action names. */
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
				new TextPropertyDescriptor(NAME_PROP, "Transition name"),
				new PropertyDescriptor(EVENT_PROP, "Event"),
				new PropertyDescriptor(ACTION_PROP, "Action")
				};
		((PropertyDescriptor) descriptors[0]).setCategory("Comment");
		((PropertyDescriptor) descriptors[1]).setCategory("Transition name");
	}
	
	
	/**
	 * Create a connection between two distinct states.
	 * 
	 * @param source
	 *            a source endpoint for this connection (non null)
	 * @param target
	 *            a target endpoint for this connection (non null)
	 * @param transition JDOMElement
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
		reconnect(source, target);
	}
	
	/**
	 * Create a connection between two distinct states.
	 * 
	 * @param transition JDOMElement
	 * @param parentState source state
	 * @param diagram parent StateMachinesDiagram
	 */
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
        ReadActionAndEventDOMTree actionTree = DOMGetInstance.getActionAndEventInstance();
        for (int k = 0; k < allActions.size(); k++) {
			Action action = (Action) allActions.get(k);
			String act = action.getActionId();
			actions.add(actionTree.getActionName(act));
			// Fill action value in Propertysheet
			this.actionPropertyValue = this.actionPropertyValue + actionTree.getActionName(act);
			if (k < allActions.size() - 1) {
				this.actionPropertyValue = this.actionPropertyValue + "; ";
			}
		}
		if (allActions.size() > 1) {
			Action action = (Action) allActions.get(0);
			String act = action.getActionId();
			action1 = "<" + actionTree.getActionName(act) + ">" + "\n" + "<...>";
		}
		else if (allActions.size() == 1) {
			Action action = (Action) allActions.get(0);
			String act = action.getActionId();
			action1 = "<" + actionTree.getActionName(act) + ">";
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
				break;
			}
		}
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
			return actionPropertyValue;
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
	
	/**
	 * set the action of this connection
	 * @param newAction the new action
	 */
	public void setAction(String newAction) {
		ReadActionAndEventDOMTree actionTree = DOMGetInstance.getActionAndEventInstance();
		action = newAction;
		if (!(actionTree.getActionName(newAction) == null)) {
			action = actionTree.getActionName(newAction);
		}
		firePropertyChange(ACTION_PROP, null, action);
	}
	
	/**
	 * set the event of this connection
	 * @param newEvent the new event
	 */
	public void setEvent(String newEvent) {
		ReadActionAndEventDOMTree eventTree = DOMGetInstance.getActionAndEventInstance();
		event = newEvent;
		if ((eventTree.getEventId(newEvent) == null)) {
			transition1.setAttribute("EVENT_ID", newEvent);
		}
		else {
			transition1.setAttribute("EVENT_ID", eventTree.getEventId(newEvent));
		}
		if (!(eventTree.getEventName(newEvent) == null)) {
			event = eventTree.getEventName(newEvent);
		}
		firePropertyChange(EVENT_PROP, null, event);
	}
	
	/**
	 * Set the backgroundcolor of this connection to orange 
	 * if it is the active state in debug mode.
	 */
	public void setActive() {
		firePropertyChange(ACTIVE_PROP, null, null);
	}
	
	/**
	 * Set the backgroundcolor of this connection to its normal color 
	 * if it is not the active state in debug mode anymore.
	 */
	public void setInactive() {
		firePropertyChange(INACTIVE_PROP, null, null);
	}
	
	/**
	 * Deactivates the editpolicies for remove connections.
	 */
	public void deactivatePolicy() {
		for (int i = 0; i < descriptors.length; i++) {
		((PropertyDescriptor) descriptors[i]).setValidator(new ICellEditorValidator() 
				{
					public String isValid(Object value) {
						return "no changes possible during debug mode";
					}
				});
		}
		firePropertyChange(DEACTIVATE_POLICY_PROP, null, null);
	}
	
	/**
	 * Reactivates the editpolicies for remove connections.
	 */
	public void activatePolicy() {
		for (int i = 0; i < descriptors.length; i++) {
			((PropertyDescriptor) descriptors[i]).setValidator(null);
		}
		firePropertyChange(ACTIVATE_POLICY_PROP, null, null);
	}
	
	/**
	 * set the name of this connection
	 * @param newName the new name
	 */
	public void setName(String newName) {
		name = newName;
		transition1.setAttribute("NAME", newName);
		firePropertyChange(NAME_PROP, null, name);
	}
	
	/**
	 * set the comment of this connection
	 * @param newComment the new comment
	 */
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
	
	/**
	 * set the id of the target connection
	 * @param id the id of the target connection
	 */
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
			setAction("<" + actionName + ">");
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

	/**
	 * @return the action name
	 */
	public String getAction() {
		return action;
	}
	
	/**
	 * @return the event name
	 */
	public String getEvent() {
		return event;
	}
	
	/**
	 * @return the connection name
	 */
	public String getName() {
		return name;
	}
	
	/**
	 * @return the connection comment
	 */
	public String getComment() {
		return comment;
	}
	
	/**
	 * @return a list with all action names
	 */
	public List getActionList() {
		return actions;
	}
	
	/**
	 * @return a list with all action elements
	 */
	public List getAllActions() {
		return allActions;
	}
	
	/**
	 * @return this connection element
	 */
	public Element getElement() {
		return transition1;
	}
	
	/**
	 * adds an action to this connection
	 * @param act the action element
	 * @param act1 the action name
	 */
	public void addAction(Action act, String act1) {
		ReadActionAndEventDOMTree actionTree = DOMGetInstance.getActionAndEventInstance();
		if (allActions.size() == 0) {
			this.setAction("<" + actionTree.getActionName(act1) + ">");
			this.actionPropertyValue = actionTree.getActionName(act1);
		}
		else if (allActions.size() == 1) {
			this.setAction(this.getAction() + "\n" + "<...>");
			this.actionPropertyValue = actionPropertyValue + "; " + actionTree.getActionName(act1);
		}
		else {
			this.actionPropertyValue = actionPropertyValue + "; " + actionTree.getActionName(act1);
		}
		allActions.add(act);
		actions.add(actionTree.getActionName(act1));
		transition1.addContent(act.getActionElement());
	}
	
	/**
	 * removes an action from this connection
	 * @param act the action element
	 * @param act1 the action name
	 */
	public void removeAction(Action act, String act1) {
		ReadActionAndEventDOMTree actionTree = DOMGetInstance.getActionAndEventInstance();
		allActions.remove(act);
		actions.remove(actionTree.getActionName(act1));
		if (actions.size() == 0) {
			this.setAction("");
			this.actionPropertyValue = "";
		}
		else if (actions.size() == 1) {
			this.setAction("<" + actions.get(0).toString() + ">");
			this.actionPropertyValue = actions.get(0).toString();
		}
		else {
			this.setAction("<" + actions.get(0).toString() + ">" + "\n" + "<...>");
			this.actionPropertyValue = actions.get(0).toString();
			for (int i = 1; i < allActions.size(); i++) {
				this.actionPropertyValue = this.actionPropertyValue + "; " + actions.get(i).toString();
			}
		}
		transition1.removeContent(act.getActionElement());
	}
}
