package dialog;

import java.util.Vector;

public class ActionAndEventInfo {
	String actionEventComment;
	String actionEventName;
	Vector actionParameter;
	
	/**
	 * constructor for an info object containing comment, name and parameter
	 * @param comment the comment on the action or event
	 * @param name the name of the action or event
	 * @param parameter the parameter for the action
	 */
	ActionAndEventInfo(String comment, String name, Vector parameter) {
		actionEventComment = comment;
		actionEventName = name;
		actionParameter = parameter;
	}
	
	/**
	 * @return the comment for the action or event
	 */
	public String getActionEventComment() {
		return actionEventComment;
	}
	
	/**
	 * @return the name for the action or event
	 */
	public String getActionEventName(){
		return actionEventName;
	}
	
	/**
	 * @return the parameter for the action
	 */
	public Vector getActionParameter() {
		return actionParameter;
	}
}
