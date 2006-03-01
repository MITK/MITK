/*
 * Created on 05.07.2005
 */
package model;

import java.util.ArrayList;
import java.util.List;

import org.eclipse.jface.viewers.ICellEditorValidator;
import org.eclipse.ui.views.properties.IPropertyDescriptor;
import org.eclipse.ui.views.properties.PropertyDescriptor;
import org.eclipse.ui.views.properties.TextPropertyDescriptor;
import org.jdom.Attribute;
import org.jdom.Comment;
import org.jdom.Element;

import stateMachines.StateMachinesEditor;
import stateMachinesList.StateMachinesList;

import dom.ReadDOMTree;

/**
 * @author Daniel
 */
public class StateMachinesDiagram extends ModelElement {
	
	private static IPropertyDescriptor[] descriptors;
	
	public static final String NAME_PROP = "StateMachinesDiagram.Name";
	
	public static final String COMMENT_PROP = "StateMachinesDiagram.Comment";

	/** Property ID to use when a child is added to this diagram. */
	public static final String CHILD_ADDED_PROP = "StateMachinesDiagram.ChildAdded";

	/** Property ID to use when a child is removed from this diagram. */
	public static final String CHILD_REMOVED_PROP = "StateMachinesDiagram.ChildRemoved";

	/** Property ID to use when the EditPolicies should be deactivated in debug mode. */
	public static final String DEACTIVATE_POLICY_PROP = "StateMachinesDiagram.Deactivate";
	
	/** Property ID to use when the EditPolicies should be activated in debug mode. */
	public static final String ACTIVATE_POLICY_PROP = "StateMachinesDiagram.Activate";
	
	private static final long serialVersionUID = 1;

	private List states = new ArrayList();
	
	private Element stateMachine, stateMachine2;
	
	private int maxID = 0;
	
	private double zoom = 1.0;
	
	private StateMachinesEditor editor1;
	
	private Comment comment = null;
	
	private String oldMachineName = null;
	
	private List allStateIDs = new ArrayList();
	
	/*
	 * Initializes the property descriptors array.
	 * 
	 * @see #getPropertyDescriptors()
	 * @see #getPropertyValue(Object)
	 * @see #setPropertyValue(Object, Object)
	 */
	static {
		descriptors = new IPropertyDescriptor[] {
				new TextPropertyDescriptor(NAME_PROP, "Statemachine name"),
				new TextPropertyDescriptor(COMMENT_PROP, "Comment")
				};
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
		if (id.equals(NAME_PROP)) {
			return getStateMachineName();
		}
		else if (id.equals(COMMENT_PROP)) {
			return getStateMachineComment();
		}
		return super.getPropertyValue(id);
	}
	
	/**
	 * Sets the properties based on the String provided by the PropertySheet
	 * 
	 * @see org.eclipse.ui.views.properties.IPropertySource#setPropertyValue(java.lang.Object,
	 *      java.lang.Object)
	 */
	public void setPropertyValue(Object id, Object value) {
		if (id.equals(NAME_PROP)) {
			String newName = (String) value;
			changeName(newName);
		}
		else if (id.equals(COMMENT_PROP)) {
			String newComment = (String) value;
			changeComment(newComment);
		}
		else
			super.setPropertyValue(id, value);
	}
	
	public StateMachinesDiagram(Element machine) {
		stateMachine2 = machine;
		oldMachineName = stateMachine2.getAttributeValue("NAME");
		stateMachine = (Element) machine.clone();
		List statesList = stateMachine.getChildren("state", stateMachine.getNamespace());
        for (int i = 0; i < statesList.size(); i++) {
        	Element ele1 = (Element) statesList.get(i);
        	List attributes = ele1.getAttributes();
        	boolean startState = false;
        	boolean endState = false;
        	for (int j = 0; j < attributes.size(); j++) {
    			Attribute attr = (Attribute) attributes.get(j);
    			if (attr.getName().equals("START_STATE")) {
    				StartState state = new StartState();
    				state.setParent(this);
    				state.setStateElement(ele1);
        			this.addChildAtCreation(state);
        			startState = true;
    			}
    			if (attr.getName().equals("END_STATE")) {
    				EndState state = new EndState();
    				state.setParent(this);
    				state.setStateElement(ele1);
        			this.addChildAtCreation(state);
        			endState = true;
    			}
    			if (attr.getName().equals("ID")) {
    				if (Integer.parseInt(attr.getValue()) > maxID) {
    					maxID = Integer.parseInt(attr.getValue());
    				}
    			}
    		}
        	if (!startState && !endState) {
        		NormalState state = new NormalState();
        		state.setParent(this);
        		state.setStateElement(ele1);
    			this.addChildAtCreation(state);
        	}
        }
		for (int j = 0; j < states.size(); j++) {
			//create all transitions
			States state = (States) states.get(j);
			List transitions = state.getAllTransitions();
			for (int k = 0; k < transitions.size(); k++) {
				Element trans = (Element) transitions.get(k);
				new Connection(trans, state, this);
			}
		}
	}
	
	public void setEditor(StateMachinesEditor editor) {
		editor1 = editor;
	}
	
	public double getZoom() {
		return zoom;
	}
	
	public void setZoom(double zoom) {
		this.zoom = zoom;
	}
	
	/**
	 * Add a state to this diagram.
	 * 
	 * @param s a non-null states instance
	 * @return true, if the state was added, false otherwise
	 */
	public boolean addChild(States s) {
		if (s != null && states.add(s)) {
			Element eleState = s.getStateElement();
			eleState.setNamespace(stateMachine.getNamespace());
			stateMachine.addContent(eleState);
			firePropertyChange(CHILD_ADDED_PROP, null, s);
			return true;
		}
		return false;
	}
	
	public int getMaxID() {
		maxID = maxID + 1;
		return maxID;
	}
	
	/**
	 * Add a state to this diagram.
	 * 
	 * @param s a non-null states instance
	 * @return true, if the state was added, false otherwise
	 */
	public boolean addChildAtCreation(States s) {
		if (s != null && states.add(s)) {
			firePropertyChange(CHILD_ADDED_PROP, null, s);
			return true;
		}
		return false;
	}

	/**
	 * Return a List of States in this diagram. The returned List should not be
	 * modified.
	 */
	public List getChildren() {
		return states;
	}
	
	/**
	 * Remove a state from this diagram.
	 * 
	 * @param s a non-null states instance;
	 * @return true, if the state was removed, false otherwise
	 */
	public boolean removeChild(States s) {
		if (s != null && states.remove(s)) {
			Element eleState = s.getStateElement();
			String id = eleState.getAttributeValue("ID");
			removeStateID(id);
			stateMachine.removeContent(eleState);
			firePropertyChange(CHILD_REMOVED_PROP, null, s);
			return true;
		}
		return false;
	}
	
	public void addToRoot(ReadDOMTree tree) {
		Element stateMachine1 = (Element) stateMachine.clone();
		tree.addStateMachine(this, stateMachine1);
		stateMachine2 = stateMachine1;
	}
	
	public void removeFromRoot(ReadDOMTree tree) {
		tree.removeStateMachine(this, stateMachine2);
	}
	
	public void changeName(String name) {
		String oldName = stateMachine.getAttributeValue("NAME");
		stateMachine.setAttribute("NAME", name);
		if (!(editor1 == null)) {
			editor1.setPartName1(name);
		}
		StateMachinesList.changeStateMachineName(oldName, name);
		firePropertyChange(NAME_PROP, null, name);
	}
	
	public void changeToOldName() {
		String oldValue = stateMachine.getAttributeValue("NAME");
		StateMachinesList.changeStateMachineName(oldValue, oldMachineName);
	}
	
	public String getStateMachineName() {
		return stateMachine.getAttributeValue("NAME");
	}

	public StateMachinesEditor getEditor() {
		return editor1;
	}
	
	public Element getStateMachinesElement() {
		return stateMachine;
	}
	
	public void changeComment(String machineComment) {
		if (hasComment()) {
			if (!(machineComment.equals("") && !(machineComment == null))) {
				comment.setText(machineComment);
			}
			else if (machineComment.equals("") || machineComment == null) {
				stateMachine.removeContent(comment);
			}
		}
		else {
			comment = new Comment(machineComment);
			stateMachine.addContent(0, comment);
		}
		firePropertyChange(COMMENT_PROP, null, machineComment);
	}
	
	public String getStateMachineComment() {
		if (hasComment()) {
			return comment.getText();
		}
		return "";
	}
	
	public boolean hasComment() {
		List machineComment = stateMachine.getContent();
		for (int i = 0; i < machineComment.size(); i++) {
			Object o = machineComment.get(i);
			if (o instanceof Comment) {
				comment = (Comment) o;
				return true;
			}
		}
		return false;
	}
	
	public void addStateID(String id) {
		allStateIDs.add(id);
	}
	
	public void removeStateID (String id) {
		allStateIDs.remove(id);
	}
	
	public boolean containsStateID (String id) {
		if (allStateIDs.contains(id)) {
			return true;
		}
		return false;
	}
	
	/**
	 * Deactivates the editpolicies for add states. Has to be called after the editor is opened.
	 * Calls the methods to deactivate policies of states and connections as well.
	 */
	public void setDebugMode() {
		for (int i = 0; i < states.size(); i++) {
			States state = (States) states.get(i);
			state.deactivatePolicy();
		}
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
	 * Reactivates the editpolicies for add states.
	 */
	public void resetFromDebugMode() {
		for (int i = 0; i < states.size(); i++) {
			States state = (States) states.get(i);
			state.activatePolicy();
		}
		for (int i = 0; i < descriptors.length; i++) {
			((PropertyDescriptor) descriptors[i]).setValidator(null);
		}
		firePropertyChange(ACTIVATE_POLICY_PROP, null, null);
	}
}
