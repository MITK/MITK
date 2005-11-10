package actions;

import org.eclipse.ui.actions.RetargetAction;

public class AddRetargetAction extends RetargetAction {
	/**
	 * Constructor for IncrementRetargetAction.
	 * @param actionID
	 * @param label
	 */
	public AddRetargetAction() {
		super(AddAction.ADD_ACTION, "Add action");
	}
}
