/*
 * Created on 05.07.2005
 */
package model;

import java.io.IOException;
import java.io.InputStream;
import java.util.ArrayList;
import java.util.List;

import org.eclipse.swt.graphics.Image;

import org.eclipse.jface.viewers.ICellEditorValidator;
import org.eclipse.ui.IWorkbenchWindow;
import org.eclipse.ui.PlatformUI;
import org.eclipse.ui.views.properties.IPropertyDescriptor;
import org.eclipse.ui.views.properties.PropertyDescriptor;
import org.eclipse.ui.views.properties.TextPropertyDescriptor;

import org.eclipse.draw2d.geometry.Dimension;
import org.eclipse.draw2d.geometry.Point;
import org.jdom.Comment;
import org.jdom.Element;

import stateMachines.StateMachinesEditor;
import stateMachines.StateMachinesPlugin;



/**
 * Abstract prototype of a state. Has a size (width and height), a location (x
 * and y position) and a list of incoming and outgoing connections. Use
 * subclasses to instantiate a specific state.
 * 
 * @see model.StartState
 * @see model.NormalState
 * @see model.EndState
 * @author Daniel
 */
public abstract class States extends ModelElement {
	
	/**
	 * @return the owner StateMachinesDiagram
	 */
	public abstract StateMachinesDiagram getParent();
	
	/**
	 * sets the owner diagram of this state
	 * @param parent the owner StateMachinesDiagram
	 */
	public abstract void setParent(StateMachinesDiagram parent);
	
	/**
	 * @return the jDOM element of this state
	 */
	public abstract Element getStateElement();
	
	/**
	 * @return the comment of this state
	 */
	public abstract Comment getStateComment();
	
	/**
	 * removes the comment from this state
	 */
	public abstract void removeStateComment();
	
	/**
	 * sets the jDOM element of this state
	 * @param state the jDOM element of this state
	 */
	public abstract void setStateElement(Element state);
	/**
	 * A static array of property descriptors. There is one IPropertyDescriptor
	 * entry per editable property.
	 * 
	 * @see #getPropertyDescriptors()
	 * @see #getPropertyValue(Object)
	 * @see #setPropertyValue(Object, Object)
	 */
	private static IPropertyDescriptor[] descriptors;

	/**
	 * ID for the Height property value (used for by the corresponding property
	 * descriptor).
	 */
	private static final String HEIGHT_PROP = "States.Height";

	/** Property ID to use when the location of this state is modified. */
	public static final String LOCATION_PROP = "States.Location";

	private static final long serialVersionUID = 1;

	/** Property ID to use when the size of this state is modified. */
	public static final String SIZE_PROP = "States.Size";
	
	/** Property ID to use when the states name is modified. */
	public static final String STATE_NAME_PROP = "States.stateName";
	
	/** Property ID to use when the comment is modified. */
	public static final String COMMENT_PROP = "States.stateComment";

	/** Property ID to use when the states ID is modified. */
	public static final String STATE_ID_PROP = "States.stateId";
	
	/** Property ID to use when the list of outgoing connections is modified. */
	public static final String SOURCE_CONNECTIONS_PROP = "States.SourceConn";

	/** Property ID to use when the list of incoming connections is modified. */
	public static final String TARGET_CONNECTIONS_PROP = "States.TargetConn";
	
	/** Property ID to use when a state is active in debug mode. */
	public static final String ACTIVE_PROP = "States.Active";
	
	/** Property ID to use when a state is inactive in debug mode. */
	public static final String INACTIVE_PROP = "States.Inactive";

	/** Property ID to use when the EditPolicies should be deactivated in debug mode. */
	public static final String DEACTIVATE_POLICY_PROP = "States.Deactivate";
	
	/** Property ID to use when the EditPolicies should be activated in debug mode. */
	public static final String ACTIVATE_POLICY_PROP = "States.Activate";
	
	/**
	 * ID for the Width property value (used for by the corresponding property
	 * descriptor).
	 */
	private static final String WIDTH_PROP = "States.Width";

	/**
	 * ID for the X property value (used for by the corresponding property
	 * descriptor).
	 */
	private static final String XPOS_PROP = "States.xPos";

	/**
	 * ID for the Y property value (used for by the corresponding property
	 * descriptor).
	 */
	private static final String YPOS_PROP = "States.yPos";
	
	
	
	public static StateMachinesDiagram parent = null;
	
	/*
	 * Initializes the property descriptors array.
	 * 
	 * @see #getPropertyDescriptors()
	 * @see #getPropertyValue(Object)
	 * @see #setPropertyValue(Object, Object)
	 */
	static {
		//String[] value = {"TRUE", "FALSE"};
		descriptors = new IPropertyDescriptor[] {
				new TextPropertyDescriptor(XPOS_PROP, "X"), // id and description pair
				new TextPropertyDescriptor(YPOS_PROP, "Y"),
				new TextPropertyDescriptor(WIDTH_PROP, "Width"),
				new TextPropertyDescriptor(HEIGHT_PROP, "Height"),
				new TextPropertyDescriptor(STATE_ID_PROP, "ID"),
				new TextPropertyDescriptor(STATE_NAME_PROP, "Statename"),
				new TextPropertyDescriptor(COMMENT_PROP, "Comment")
				};
		((PropertyDescriptor) descriptors[descriptors.length - 3]).setCategory("State");
		((PropertyDescriptor) descriptors[descriptors.length - 2]).setCategory("State");
		((PropertyDescriptor) descriptors[descriptors.length - 1]).setCategory("Comment");
		// use a custom cell editor validator for all four array entries
		for (int i = 0; i < descriptors.length - 3; i++) {
			if (((PropertyDescriptor) descriptors[i]).getId().equals(XPOS_PROP)) {
				((PropertyDescriptor) descriptors[i]).setCategory("Location");
			}
			else if (((PropertyDescriptor) descriptors[i]).getId().equals(YPOS_PROP)) {
				((PropertyDescriptor) descriptors[i]).setCategory("Location");
			}
			else if (((PropertyDescriptor) descriptors[i]).getId().equals(WIDTH_PROP)) {
				((PropertyDescriptor) descriptors[i]).setCategory("Size");
			}
			else if (((PropertyDescriptor) descriptors[i]).getId().equals(HEIGHT_PROP)) {
				((PropertyDescriptor) descriptors[i]).setCategory("Size");
			}
			((PropertyDescriptor) descriptors[i])
					.setValidator(new ICellEditorValidator() {
						public String isValid(Object value) {
							int intValue = -1;
							try {
								intValue = Integer.parseInt((String) value);
							} catch (NumberFormatException exc) {
									return "Not a number";
							}
							return (intValue >= 0) ? null
									: "Value must be >=  0";
						}
					});
			((PropertyDescriptor) descriptors[descriptors.length - 3])
			.setValidator(new ICellEditorValidator() {
				public String isValid(Object value) {
					int intValue = -1;
					try {
						intValue = Integer.parseInt((String) value);
					} catch (NumberFormatException exc) {
							return "Not a number";
					}
					IWorkbenchWindow ui = PlatformUI.getWorkbench().getActiveWorkbenchWindow();
					StateMachinesEditor activeEditor = (StateMachinesEditor) ui.getActivePage().getActiveEditor();
					if (activeEditor.getDiagram().containsStateID((String)value)) {
						return "ID already in use";
					}
					return (intValue >= 0) ? null
							: "Value must be >=  0";
				}
			});
		}
	} // static
	
	protected static Image createImage(String name) {
		InputStream stream = StateMachinesPlugin.class
				.getResourceAsStream(name);
		Image image = new Image(null, stream);
		try {
			stream.close();
		} catch (IOException ioe) {
		}
		return image;
	}

	/** Location of this state. */
	private Point location = new Point(0, 0);

	/** Size of this state. */
	private Dimension size = new Dimension(100, 50);

	/** List of outgoing Connections. */
	private List sourceConnections = new ArrayList();

	/** List of incoming Connections. */
	private List targetConnections = new ArrayList();

	/** Name of this state */
	private String stateName = new String("Statename");
	
	/** Comment of this state */
	private String commentText = new String("");
	
	/** ID of this state */
	private String stateId = new String("0");

	/**
	 * Add an incoming or outgoing connection to this state.
	 * 
	 * @param conn a non-null connection instance
	 * @throws IllegalArgumentException
	 *             if the connection is null or has not distinct endpoints
	 */
	void addConnection(Connection conn, Element transition) {
		if (conn == null) {
			throw new IllegalArgumentException();
		}
		if (conn.getSource() == this) {
			sourceConnections.add(conn);
			if (transition.getParent() == null) {
				this.getStateElement().addContent(transition);
			}
			firePropertyChange(SOURCE_CONNECTIONS_PROP, null, conn);
		}
		if (conn.getTarget() == this) {
			targetConnections.add(conn);
			firePropertyChange(TARGET_CONNECTIONS_PROP, null, conn);
		}
	}

	/**
	 * Return a pictogram (small icon) describing this model element. Children
	 * should override this method and return an appropriate Image.
	 * 
	 * @return a 16x16 Image or null
	 */
	public abstract Image getIcon();

	/**
	 * Return the Location of this state.
	 * 
	 * @return a non-null location instance
	 */
	public Point getLocation() {
		return location.getCopy();
	}

	/**
	 * Returns an array of IPropertyDescriptors for this state.
	 * <p>
	 * The returned array is used to fill the property dialog, when the edit-part
	 * corresponding to this model element is selected.
	 * </p>
	 * 
	 * @see #descriptors
	 * @see #getPropertyValue(Object)
	 * @see #setPropertyValue(Object, Object)
	 */
	public IPropertyDescriptor[] getPropertyDescriptors() {
		return descriptors;
	}

	/**
	 * Return the property value for the given propertyId, or null.
	 * <p>
	 * The property dialog uses the IDs from the IPropertyDescriptors array to
	 * obtain the value of the corresponding properties.
	 * </p>
	 * 
	 * @see #descriptors
	 * @see #getPropertyDescriptors()
	 */
	public Object getPropertyValue(Object propertyId) {
		if (XPOS_PROP.equals(propertyId)) {
			return Integer.toString(location.x);
		}
		if (YPOS_PROP.equals(propertyId)) {
			return Integer.toString(location.y);
		}
		if (HEIGHT_PROP.equals(propertyId)) {
			return Integer.toString(size.height);
		}
		if (WIDTH_PROP.equals(propertyId)) {
			return Integer.toString(size.width);
		}
		if (STATE_NAME_PROP.equals(propertyId)) {
			return stateName;
		}
		if (COMMENT_PROP.equals(propertyId)) {
			return commentText;
		}
		if (STATE_ID_PROP.equals(propertyId)) {
			return stateId;
		}
		return super.getPropertyValue(propertyId);
	}

	/**
	 * Return the Size of this state.
	 * 
	 * @return a non-null Dimension instance
	 */
	public Dimension getSize() {
		return size.getCopy();
	}
	
	public String getStateName() {
		return stateName;
	}
	
	public String getComment() {
		return commentText;
	}
	
	public String getStateId() {
		return stateId;
	}

	/**
	 * Return a List of outgoing Connections.
	 */
	public List getSourceConnections() {
		return new ArrayList(sourceConnections);
	}

	/**
	 * Return a List of incoming Connections.
	 */
	public List getTargetConnections() {
		return new ArrayList(targetConnections);
	}
	
	/**
	 * @return all jDOM elements of type transition
	 */
	public List getAllTransitions() {
		List allTransitions = this.getStateElement().getChildren("transition", this.getStateElement().getNamespace());
		return allTransitions;
	}

	/**
	 * Remove an incoming or outgoing connection from this state.
	 * 
	 * @param conn
	 *            a non-null connection instance
	 * @throws IllegalArgumentException
	 *             if the parameter is null
	 */
	void removeConnection(Connection conn, Element transition) {
		if (conn == null) {
			throw new IllegalArgumentException();
		}
		if (conn.getSource() == this) {
			sourceConnections.remove(conn);
			if (!(transition.getParent() == null)) {
				this.getStateElement().removeContent(transition);
			}
			firePropertyChange(SOURCE_CONNECTIONS_PROP, null, conn);
		} 
		if (conn.getTarget() == this) {
			targetConnections.remove(conn);
			firePropertyChange(TARGET_CONNECTIONS_PROP, null, conn);
		}
	}

	/**
	 * Set the Location of this state.
	 * 
	 * @param newLocation
	 *            a non-null Point instance
	 * @throws IllegalArgumentException
	 *             if the parameter is null
	 */
	public void setLocation(Point newLocation) {
		if (newLocation == null) {
			throw new IllegalArgumentException();
		}
		Element state = getStateElement();
		location.setLocation(newLocation);
		state.setAttribute("X_POS", Integer.toString(location.x));
		state.setAttribute("Y_POS", Integer.toString(location.y));
		firePropertyChange(LOCATION_PROP, null, location);
	}

	/**
	 * Set the property value for the given property id. If no matching id is
	 * found, the call is forwarded to the superclass.
	 * <p>
	 * The property dialog uses the IDs from the IPropertyDescriptors array to set
	 * the values of the corresponding properties.
	 * </p>
	 * 
	 * @see #descriptors
	 * @see #getPropertyDescriptors()
	 */
	public void setPropertyValue(Object propertyId, Object value) {
		//Element state = getStateElement();
		if (XPOS_PROP.equals(propertyId)) {
			int x = Integer.parseInt((String) value);
			setLocation(new Point(x, location.y));
			//state.setAttribute("X_POS", value.toString());
		} else if (YPOS_PROP.equals(propertyId)) {
			int y = Integer.parseInt((String) value);
			setLocation(new Point(location.x, y));
			//state.setAttribute("Y_POS", value.toString());
		} else if (HEIGHT_PROP.equals(propertyId)) {
			int height = Integer.parseInt((String) value);
			setSize(new Dimension(size.width, height));
			//state.setAttribute("HEIGHT", Integer.toString(height));
		} else if (WIDTH_PROP.equals(propertyId)) {
			int width = Integer.parseInt((String) value);
			setSize(new Dimension(width, size.height));
			//state.setAttribute("WIDTH", Integer.toString(width));
		} else if (STATE_NAME_PROP.equals(propertyId)) {
			String name = (String) value;
			setStateName(name);
			//state.setAttribute("NAME", name);
		} else if (COMMENT_PROP.equals(propertyId)) {
			String commentText = (String) value;
			setComment(commentText);
			/*if (!(commentText.equals("")) && !(commentText == null)) {
				Comment comment = getStateComment();
				comment.setText(commentText);
			}*/
		} else if (STATE_ID_PROP.equals(propertyId)) {
			String id = (String)value;
			setStateId(id);
			//state.setAttribute("ID", id);
			List connToChange = this.getTargetConnections();
			for (int i = 0; i < connToChange.size(); i++) {
				Connection conn1 = (Connection) connToChange.get(i);
				conn1.setNextStateID(id);
			}
		} else {
			super.setPropertyValue(propertyId, value);
		}
	}

	/**
	 * Set the Size of this state. Will not modify the size if newSize is null.
	 * 
	 * @param newSize
	 *            a non-null Dimension instance or null
	 */
	public void setSize(Dimension newSize) {
		Element state = getStateElement();
		if (newSize != null) {
			size.setSize(newSize);
			state.setAttribute("WIDTH", Integer.toString(size.width));
			state.setAttribute("HEIGHT", Integer.toString(size.height));
			firePropertyChange(SIZE_PROP, null, size);
		}
	}

	/**
	 * Set the states name.
	 * 
	 * @param newName the name that is displayed in the states label.
	 */
	public void setStateName(String newName) {
		Element state = getStateElement();
		stateName = newName;
		state.setAttribute("NAME", newName);
		firePropertyChange(STATE_NAME_PROP, null, stateName);
	}
	
	/**
	 * Set the backgroundcolor of this state to orange 
	 * if it is the active state in debug mode.
	 */
	public void setActive() {
		firePropertyChange(ACTIVE_PROP, null, null);
	}
	
	/**
	 * Set the backgroundcolor of this state to its normal color 
	 * if it is not the active state in debug mode anymore.
	 */
	public void setInactive() {
		firePropertyChange(INACTIVE_PROP, null, null);
	}
	
	/**
	 * Deactivates the editpolicies for remove states and create connections.
	 */
	public void deactivatePolicy() {
		for (int i = 0; i < sourceConnections.size(); i++) {
			Connection con = (Connection) sourceConnections.get(i);
			con.deactivatePolicy();
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
	 * Reactivates the editpolicies for remove states and create connections.
	 */
	public void activatePolicy() {
		for (int i = 0; i < sourceConnections.size(); i++) {
			Connection con = (Connection) sourceConnections.get(i);
			con.activatePolicy();
		}
		for (int i = 0; i < descriptors.length; i++) {
			((PropertyDescriptor) descriptors[i]).setValidator(null);
		}
		for (int i = 0; i < descriptors.length - 3; i++) {
			((PropertyDescriptor) descriptors[i])
					.setValidator(new ICellEditorValidator() {
						public String isValid(Object value) {
							int intValue = -1;
							try {
								intValue = Integer.parseInt((String) value);
							} catch (NumberFormatException exc) {
									return "Not a number";
							}
							return (intValue >= 0) ? null
									: "Value must be >=  0";
						}
					});
			((PropertyDescriptor) descriptors[descriptors.length - 3])
			.setValidator(new ICellEditorValidator() {
				public String isValid(Object value) {
					int intValue = -1;
					try {
						intValue = Integer.parseInt((String) value);
					} catch (NumberFormatException exc) {
							return "Not a number";
					}
					IWorkbenchWindow ui = PlatformUI.getWorkbench().getActiveWorkbenchWindow();
					StateMachinesEditor activeEditor = (StateMachinesEditor) ui.getActivePage().getActiveEditor();
					if (activeEditor.getDiagram().containsStateID((String)value)) {
						return "ID already in use";
					}
					return (intValue >= 0) ? null
							: "Value must be >=  0";
				}
			});
		}
		firePropertyChange(ACTIVATE_POLICY_PROP, null, null);
	}
	
	/**
	 * Set the states comment.
	 * 
	 * @param newComment the new comment for the state
	 */
	public void setComment(String newComment) {
		if (!(newComment.equals("") && !(newComment == null))) {
			Comment comment = getStateComment();
			commentText = newComment;
			comment.setText(commentText);
		}
		else if (newComment.equals("") || newComment == null) {
			commentText = newComment;
			removeStateComment();
		}
		firePropertyChange(COMMENT_PROP, null, commentText);
	}
	
	/**
	 * Set the states id.
	 * 
	 * @param newId the new id for the state
	 */
	public void setStateId(String newId) {
		Element state = getStateElement();
		stateId = newId;
		String oldID = state.getAttributeValue("ID");
		parent = this.getParent();
		if (this.getParent().containsStateID(oldID)) {
			this.getParent().removeStateID(oldID);
		}
		this.getParent().addStateID(newId);
		state.setAttribute("ID", newId);
		firePropertyChange(STATE_ID_PROP, null, newId);
	}
}
