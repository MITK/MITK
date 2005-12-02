package actions;

import org.eclipse.ui.actions.RetargetAction;

public class ChangeRetargetAction extends RetargetAction {
	/**
	 * Constructor for ChangeRetargetAction.
	 */
	public ChangeRetargetAction() {
		super(ChangeAction.CHANGE_ACTION, "Change action");
	}
}
