package actions;

import org.eclipse.ui.actions.RetargetAction;

public class RemoveRetargetAction extends RetargetAction {
	/**
	 * Constructor for RemoveRetargetAction.
	 */
	public RemoveRetargetAction() {
		super(RemoveAction.REMOVE_ACTION, "Remove action");
	}
}
