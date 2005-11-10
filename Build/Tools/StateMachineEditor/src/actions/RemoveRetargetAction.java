package actions;

import org.eclipse.ui.actions.RetargetAction;

public class RemoveRetargetAction extends RetargetAction {
	/**
	 * Constructor for IncrementRetargetAction.
	 * @param actionID
	 * @param label
	 */
	public RemoveRetargetAction() {
		super(RemoveAction.REMOVE_ACTION, "Remove action");
	}
}
