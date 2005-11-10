package actions;

import org.eclipse.ui.actions.RetargetAction;

public class ChangeRetargetAction extends RetargetAction {
	/**
	 * Constructor for IncrementRetargetAction.
	 * @param actionID
	 * @param label
	 */
	public ChangeRetargetAction() {
		super(ChangeAction.CHANGE_ACTION, "Change action");
	}
}
