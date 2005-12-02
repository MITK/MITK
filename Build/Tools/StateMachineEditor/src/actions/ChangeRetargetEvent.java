package actions;

import org.eclipse.ui.actions.RetargetAction;

public class ChangeRetargetEvent extends RetargetAction {
	/**
	 * Constructor for ChangeRetargetEvent.
	 */
	public ChangeRetargetEvent() {
		super(ChangeEvent.CHANGE_EVENT, "Change event");
	}
}